/**
 * Responsibility: enumerate FreeBSD mixer devices and read master volume via OSS ioctls.
 * Dependencies: sys/soundcard.h, sysctlbyname, POSIX read-only file APIs.
 * Collaboration: implements AudioBackend; device selection and UI stay above this layer.
 * Side effects: opens /dev/mixerN with O_RDONLY and issues DEVMASK/VOLUME read ioctls only.
 * Constraints: no volume/mute write operation is exposed in this P0 adapter.
 */
#include "fcc/platform/audio_backend.h"
#include <sys/types.h>
#include <sys/soundcard.h>
#include <sys/sysctl.h>
#include <sys/ioctl.h>
#include <cerrno>
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
std::optional<StereoVolume> decodeOssVolume(int raw) noexcept {
    const int left = raw & 0xff, right = (raw >> 8) & 0xff;
    if (left > 100 || right > 100) return std::nullopt;
    return StereoVolume{left, right};
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
        report.devices.push_back({path, pcmDescription(unit), defaultUnit(unit), *volume});
    }
    if (found == 0 && report.issues.empty())
        report.issues.push_back({ProbeError::Unavailable, "/dev/mixer*", "no mixer devices were found"});
    return report;
}
} // namespace fcc::platform
