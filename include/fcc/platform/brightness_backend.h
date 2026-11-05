/**
 * Responsibility: read-only contract for FreeBSD backlight discovery.
 * Dependencies: shared probe result types; no Qt or desktop dependencies.
 * Collaboration: brightness service consumes identity, raw levels and range.
 * Side effects: read-only device discovery/status ioctls; never changes brightness.
 */
#pragma once
#include "fcc/platform/probe_result.h"
#include <cstdint>
#include <string>
#include <optional>
#include <vector>
namespace fcc::platform {
struct BrightnessDevice {
    std::string path, name, type;
    std::uint32_t current{0};
    std::optional<std::uint32_t> minimum, maximum;
    std::vector<std::uint32_t> levels;
};
class BrightnessBackend { public: [[nodiscard]] ProbeReport<BrightnessDevice> discover() const; };
} // namespace fcc::platform
