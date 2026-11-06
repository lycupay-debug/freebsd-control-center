/**
 * Responsibility: read-only contract for FreeBSD OSS-compatible mixer discovery.
 * Dependencies: shared probe result types; no Qt or desktop dependencies.
 * Collaboration: audio service consumes stable mixer path and master-volume state.
 * Side effects: opens mixer devices read-only and issues read ioctls only.
 */
#pragma once
#include "fcc/platform/probe_result.h"
#include <optional>
#include <string>
namespace fcc::platform {
struct StereoVolume { int left{0}; int right{0}; };
struct AudioDevice {
    std::string path, name;
    bool isDefault{false};
    StereoVolume masterVolume;
    bool muteSupported{false};
};
// Interpret optional OSS controls only when the device advertises their capability bit.
[[nodiscard]] bool supportsOssMute(int deviceMask) noexcept;
[[nodiscard]] std::optional<StereoVolume> decodeOssVolume(int raw) noexcept;
class AudioBackend { public: [[nodiscard]] ProbeReport<AudioDevice> discover() const; };
} // namespace fcc::platform
