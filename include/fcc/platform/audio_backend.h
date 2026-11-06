/**
 * Responsibility: read-only contract for FreeBSD OSS-compatible mixer discovery.
 * Dependencies: shared probe result types; no Qt or desktop dependencies.
 * Collaboration: audio service consumes stable mixer path and master-volume state.
 * Side effects: discovery is read-only; setMasterVolume performs an explicit, range-checked OSS write with readback and restoration attempt.
 */
#pragma once
#include "fcc/platform/probe_result.h"
#include <optional>
#include <string>
namespace fcc::platform {
struct StereoVolume { int left{0}; int right{0}; };
struct VolumeWriteResult {
    bool success{false};
    std::optional<StereoVolume> actual;
    bool originalRestored{false};
    std::string detail;
};
struct AudioDevice {
    std::string path, name;
    bool isDefault{false};
    StereoVolume masterVolume;
    bool muteSupported{false};
};
// Interpret optional OSS controls only when the device advertises their capability bit.
[[nodiscard]] bool supportsOssMute(int deviceMask) noexcept;
[[nodiscard]] bool isValidOssVolume(const StereoVolume &volume) noexcept;
[[nodiscard]] std::optional<int> encodeOssVolume(const StereoVolume &volume) noexcept;
[[nodiscard]] std::optional<StereoVolume> decodeOssVolume(int raw) noexcept;
class AudioBackend {
public:
    [[nodiscard]] ProbeReport<AudioDevice> discover() const;
    // Writes only a validated mixer master-volume endpoint and verifies readback.
    // On failed verification, it attempts to restore the value read before the write.
    [[nodiscard]] VolumeWriteResult setMasterVolume(const std::string &path,
                                                    StereoVolume requested) const;
};
} // namespace fcc::platform
