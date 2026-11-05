/**
 * Responsibility: shared, non-throwing result and diagnostic types for OS probes.
 * Dependencies: C++ standard library only.
 * Collaboration: platform adapters return these values to services/composition.
 * Side effects: none; errors retain operation context without inventing defaults.
 */
#pragma once
#include <string>
#include <vector>
namespace fcc::platform {
enum class ProbeError { Unavailable, PermissionDenied, Unsupported, IoError };
struct ProbeIssue { ProbeError code; std::string subject; std::string detail; };
template <typename T> struct ProbeReport {
    std::vector<T> devices;
    std::vector<ProbeIssue> issues;
    [[nodiscard]] bool available() const noexcept { return !devices.empty(); }
};
} // namespace fcc::platform
