/**
 * Responsibility: enumerate /dev/backlight endpoints and read FreeBSD backlight ioctls.
 * Dependencies: sys/backlight.h, POSIX directory/file APIs, shared platform types.
 * Collaboration: implements BrightnessBackend; callers own presentation and policy.
 * Side effects: opens endpoints O_RDONLY and calls GETINFO/GETSTATUS only.
 * Constraints: duplicate symlink targets are collapsed; malformed levels are rejected.
 */
#include "fcc/platform/brightness_backend.h"
#include <sys/backlight.h>
#include <sys/ioctl.h>
#include <algorithm>
#include <cerrno>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <memory>
#include <set>
#include <string>
#include <unistd.h>
namespace fcc::platform {
namespace {
struct DirCloser { void operator()(DIR *dir) const noexcept { if (dir) closedir(dir); } };
struct FdCloser { int fd{-1}; ~FdCloser() { if (fd >= 0) close(fd); } };
ProbeError classifyErrno(int value) { return value == EACCES || value == EPERM ? ProbeError::PermissionDenied : ProbeError::IoError; }
std::string errorText(const char *operation, const std::string &path, int error) {
    return std::string(operation) + " " + path + ": " + std::strerror(error);
}
}
ProbeReport<BrightnessDevice> BrightnessBackend::discover() const {
    ProbeReport<BrightnessDevice> report;
    constexpr const char *directory = "/dev/backlight";
    std::unique_ptr<DIR, DirCloser> dir(opendir(directory));
    if (!dir) {
        const int error = errno;
        report.issues.push_back({error == ENOENT ? ProbeError::Unavailable : classifyErrno(error), directory, errorText("opendir", directory, error)});
        return report;
    }
    std::set<std::string> visited;
    while (dirent *entry = readdir(dir.get())) {
        if (entry->d_name[0] == '.') continue;
        const std::string path = std::string(directory) + "/" + entry->d_name;
        char resolved[PATH_MAX]{};
        const std::string identity = realpath(path.c_str(), resolved) ? resolved : path;
        if (!visited.insert(identity).second) continue;
        FdCloser fd{open(path.c_str(), O_RDONLY | O_CLOEXEC)};
        if (fd.fd < 0) {
            const int error = errno;
            report.issues.push_back({classifyErrno(error), path, errorText("open", path, error)});
            continue;
        }
        backlight_info info{};
        backlight_props props{};
        if (ioctl(fd.fd, BACKLIGHTGETINFO, &info) < 0) {
            const int error = errno;
            report.issues.push_back({classifyErrno(error), path, errorText("BACKLIGHTGETINFO", path, error)});
            continue;
        }
        if (ioctl(fd.fd, BACKLIGHTGETSTATUS, &props) < 0) {
            const int error = errno;
            report.issues.push_back({classifyErrno(error), path, errorText("BACKLIGHTGETSTATUS", path, error)});
            continue;
        }
        if (props.nlevels > BACKLIGHTMAXLEVELS) {
            report.issues.push_back({ProbeError::Unsupported, path, "driver returned an invalid brightness level count"});
            continue;
        }
        BrightnessDevice device;
        device.path = path; device.name = std::string(info.name, strnlen(info.name, sizeof(info.name)));
        device.type = info.type == BACKLIGHT_TYPE_PANEL ? "panel" :
                      info.type == BACKLIGHT_TYPE_KEYBOARD ? "keyboard" : "unknown";
        device.current = props.brightness;
        if (props.nlevels > 0) {
            device.levels.assign(props.levels, props.levels + props.nlevels);
            const auto bounds = std::minmax_element(device.levels.begin(), device.levels.end());
            device.minimum = *bounds.first; device.maximum = *bounds.second;
        }
        report.devices.push_back(std::move(device));
    }
    if (report.devices.empty() && report.issues.empty())
        report.issues.push_back({ProbeError::Unavailable, directory, "no backlight endpoints were found"});
    return report;
}
} // namespace fcc::platform
