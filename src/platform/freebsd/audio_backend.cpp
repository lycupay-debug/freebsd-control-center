/**
 * Responsibility: enumerate FreeBSD mixer devices and read master volume via OSS ioctls.
 * Dependencies: sys/soundcard.h, sysctlbyname, POSIX read-only file APIs.
 * Collaboration: implements AudioBackend; device selection and UI stay above this layer.
 * Side effects: discovery opens O_RDONLY; explicit master-volume changes open one validated /dev/mixerN O_RDWR and verify readback, attempting restoration on failure.
 * Constraints: master-volume writes are explicit and readback-verified; mute writes are not exposed.
 */
#include "fcc/platform/audio_backend.h"
#include <sys/types.h>
#include <sys/soundcard.h>
#include <sys/sysctl.h>
#include <sys/ioctl.h>
#include <cerrno>
#include <charconv>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <string>
#include <unistd.h>
namespace fcc::platform {
namespace {
struct FdCloser { int fd{-1}; ~FdCloser() { if (fd >= 0) close(fd); } };
ProbeError classifyErrno(int value) { return value == EACCES || value == EPERM ? ProbeError::PermissionDenied : ProbeError::IoError; }
std::string pcmDescription(int unit) {
    char key[64]{}; std::snprintf(key, sizeof(key), "dev.pcm.%d.%%desc", unit);
    char value[256]{}; size_t size = sizeof(value);
    if (sysctlbyname(key, value, &size, nullptr, 0) == 0 && value[0] != '\0') return value;
    return "FreeBSD mixer " + std::to_string(unit);
}
bool defaultUnit(int unit) {
    int selected = -1; size_t size = sizeof(selected);
    return sysctlbyname("hw.snd.default_unit", &selected, &size, nullptr, 0) == 0 && selected == unit;
}
}
bool supportsOssMute(int deviceMask) noexcept {
    return (deviceMask & SOUND_MASK_MUTE) != 0;
}
bool isValidOssVolume(const StereoVolume &volume) noexcept {
    return volume.left >= 0 && volume.left <= 100 &&
           volume.right >= 0 && volume.right <= 100;
}
std::optional<int> encodeOssVolume(const StereoVolume &volume) noexcept {
    if (!isValidOssVolume(volume)) return std::nullopt;
    return volume.left | (volume.right << 8);
}
std::optional<StereoVolume> decodeOssVolume(int raw) noexcept {
    const int left = raw & 0xff, right = (raw >> 8) & 0xff;
    if (left > 100 || right > 100) return std::nullopt;
    return StereoVolume{left, right};
}
VolumeWriteResult AudioBackend::setMasterVolume(const std::string &path,
                                                       StereoVolume requested) const {
    VolumeWriteResult result;
    const std::string prefix = "/dev/mixer";
    if (path.compare(0, prefix.size(), prefix) != 0 || path.size() == prefix.size()) {
        result.detail = "refusing a path outside /dev/mixerN";
        return result;
    }
    int unit = -1;
    const char *first = path.data() + prefix.size();
    const char *last = path.data() + path.size();
    const auto parsed = std::from_chars(first, last, unit);
    if (parsed.ec != std::errc{} || parsed.ptr != last || unit < 0 || unit >= 32 ||
        path != prefix + std::to_string(unit)) {
        result.detail = "mixer unit must be a canonical integer in 0..31";
        return result;
    }
    const auto encoded = encodeOssVolume(requested);
    if (!encoded) {
        result.detail = "requested left/right volume must each be in 0..100";
        return result;
    }
    FdCloser fd{open(path.c_str(), O_RDWR | O_CLOEXEC)};
    if (fd.fd < 0) {
        const int error = errno;
        result.detail = std::string("open ") + path + ": " + std::strerror(error);
        return result;
    }
    int deviceMask = 0;
    if (ioctl(fd.fd, SOUND_MIXER_READ_DEVMASK, &deviceMask) < 0) {
        const int error = errno;
        result.detail = std::string("SOUND_MIXER_READ_DEVMASK: ") + std::strerror(error);
        return result;
    }
    if ((deviceMask & SOUND_MASK_VOLUME) == 0) {
        result.detail = "mixer does not advertise master-volume control";
        return result;
    }
    int originalRaw = 0;
    if (ioctl(fd.fd, SOUND_MIXER_READ_VOLUME, &originalRaw) < 0) {
        const int error = errno;
        result.detail = std::string("SOUND_MIXER_READ_VOLUME before write: ") + std::strerror(error);
        return result;
    }
    const auto original = decodeOssVolume(originalRaw);
    if (!original) {
        result.detail = "mixer returned an invalid original volume; refusing write";
        return result;
    }

    int writeRaw = *encoded;
    const int writeStatus = ioctl(fd.fd, SOUND_MIXER_WRITE_VOLUME, &writeRaw);
    const int writeError = writeStatus < 0 ? errno : 0;
    int readbackRaw = 0;
    const bool readbackOk = ioctl(fd.fd, SOUND_MIXER_READ_VOLUME, &readbackRaw) == 0;
    const auto readback = readbackOk ? decodeOssVolume(readbackRaw) : std::nullopt;
    if (writeStatus == 0 && readback && readback->left == requested.left &&
        readback->right == requested.right) {
        result.success = true;
        result.actual = *readback;
        result.detail = "master volume write verified by readback";
        return result;
    }

    // A failed ioctl/readback can still leave a changed device; restore the exact prior value.
    int restoreRaw = originalRaw;
    const bool restoreWriteOk = ioctl(fd.fd, SOUND_MIXER_WRITE_VOLUME, &restoreRaw) == 0;
    int restoredRaw = 0;
    const bool restoreReadOk = ioctl(fd.fd, SOUND_MIXER_READ_VOLUME, &restoredRaw) == 0;
    const auto restored = restoreReadOk ? decodeOssVolume(restoredRaw) : std::nullopt;
    result.originalRestored = restoreWriteOk && restored &&
        restored->left == original->left && restored->right == original->right;
    if (restored) result.actual = *restored;
    if (writeStatus < 0)
        result.detail = std::string("SOUND_MIXER_WRITE_VOLUME: ") + std::strerror(writeError);
    else if (!readback)
        result.detail = "volume write returned but readback failed or was invalid";
    else
        result.detail = "volume readback did not match requested value";
    result.detail += result.originalRestored ? "; original value restored and verified"
                                             : "; original value restoration could not be verified";
    return result;
}

ProbeReport<AudioDevice> AudioBackend::discover() const {
    ProbeReport<AudioDevice> report;
    int found = 0;
    for (int unit = 0; unit < 32; ++unit) {
        char path[64]{}; std::snprintf(path, sizeof(path), "/dev/mixer%d", unit);
        FdCloser fd{open(path, O_RDONLY | O_CLOEXEC)};
        if (fd.fd < 0) {
            if (errno == ENOENT) continue;
            const int error = errno;
            report.issues.push_back({classifyErrno(error), path, std::string("open ") + path + ": " + std::strerror(error)});
            continue;
        }
        ++found;
        int deviceMask = 0;
        if (ioctl(fd.fd, SOUND_MIXER_READ_DEVMASK, &deviceMask) < 0) {
            const int error = errno;
            report.issues.push_back({classifyErrno(error), path, std::string("SOUND_MIXER_READ_DEVMASK ") + path + ": " + std::strerror(error)});
            continue;
        }
        if ((deviceMask & (1 << SOUND_MIXER_VOLUME)) == 0) {
            report.issues.push_back({ProbeError::Unsupported, path, "mixer does not expose master volume"});
            continue;
        }
        int rawVolume = 0;
        if (ioctl(fd.fd, SOUND_MIXER_READ_VOLUME, &rawVolume) < 0) {
            const int error = errno;
            report.issues.push_back({classifyErrno(error), path, std::string("SOUND_MIXER_READ_VOLUME ") + path + ": " + std::strerror(error)});
            continue;
        }
        const auto volume = decodeOssVolume(rawVolume);
        if (!volume) {
            report.issues.push_back({ProbeError::Unsupported, path, "mixer returned a volume outside 0..100"});
            continue;
        }
        report.devices.push_back({
            path, pcmDescription(unit), defaultUnit(unit), *volume,
            supportsOssMute(deviceMask)
        });
    }
    if (found == 0 && report.issues.empty())
        report.issues.push_back({ProbeError::Unavailable, "/dev/mixer*", "no mixer devices were found"});
    return report;
}
} // namespace fcc::platform
