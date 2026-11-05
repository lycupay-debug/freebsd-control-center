/**
 * Responsibility: command-line read-only diagnostic for FCC FreeBSD platform adapters.
 * Dependencies: FCC platform library and C++ standard I/O.
 * Collaboration: validates backend behavior independently from the Qt presentation.
 * Side effects: performs only the adapters' read-only device opens and ioctls.
 * Constraint: never changes brightness, volume, mute or system configuration.
 */
#include "fcc/platform/audio_backend.h"
#include "fcc/platform/brightness_backend.h"
#include <iostream>

int main() {
    const fcc::platform::BrightnessBackend brightness;
    const auto brightnessReport = brightness.discover();
    for (const auto &device : brightnessReport.devices) {
        std::cout << "brightness " << device.path << " name=" << device.name
                  << " type=" << device.type << " current=" << device.current
                  << " range=";
        if (device.minimum && device.maximum)
            std::cout << *device.minimum << ".." << *device.maximum;
        else
            std::cout << "not-reported";
        std::cout << " levels=" << device.levels.size() << '\n';
    }
    for (const auto &issue : brightnessReport.issues)
        std::cerr << "brightness issue [" << issue.subject << "]: " << issue.detail << '\n';

    const fcc::platform::AudioBackend audio;
    const auto audioReport = audio.discover();
    for (const auto &device : audioReport.devices) {
        std::cout << "audio " << device.path << " name=" << device.name
                  << " default=" << (device.isDefault ? "yes" : "no")
                  << " master=" << device.masterVolume.left << ":" << device.masterVolume.right << '\n';
    }
    for (const auto &issue : audioReport.issues)
        std::cerr << "audio issue [" << issue.subject << "]: " << issue.detail << '\n';
    return brightnessReport.available() || audioReport.available() ? 0 : 1;
}
