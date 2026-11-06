/**
 * Responsibility: unit-test pure OSS volume decoding used by the FreeBSD backend.
 * Dependencies: Qt Test and the FCC platform backend library.
 * Collaboration: protects byte layout and rejects invalid driver values.
 * Side effects: none; tests do not open or modify hardware devices.
 */
#include "fcc/platform/audio_backend.h"
#include <QtTest/QtTest>
class PlatformValueTests final : public QObject {
    Q_OBJECT
private slots:
    void decodesStereoVolume() {
        const auto value = fcc::platform::decodeOssVolume(73 | (61 << 8));
        QVERIFY(value.has_value()); QCOMPARE(value->left, 73); QCOMPARE(value->right, 61);
    }
    void acceptsZeroAndMaximum() {
        const auto zero = fcc::platform::decodeOssVolume(0);
        QVERIFY(zero.has_value()); QCOMPARE(zero->left, 0); QCOMPARE(zero->right, 0);
        const auto maximum = fcc::platform::decodeOssVolume(100 | (100 << 8));
        QVERIFY(maximum.has_value()); QCOMPARE(maximum->left, 100); QCOMPARE(maximum->right, 100);
    }
    void encodesValidStereoVolume() {
        const auto encoded = fcc::platform::encodeOssVolume({60, 86});
        QVERIFY(encoded.has_value()); QCOMPARE(*encoded, 60 | (86 << 8));
        QVERIFY(fcc::platform::isValidOssVolume({0, 100}));
        QVERIFY(!fcc::platform::isValidOssVolume({-1, 50}));
        QVERIFY(!fcc::platform::isValidOssVolume({50, 101}));
        QVERIFY(!fcc::platform::encodeOssVolume({101, 0}).has_value());
    }
    void rejectsOutOfRangeChannels() {
        QVERIFY(!fcc::platform::decodeOssVolume(101 | (40 << 8)).has_value());
        QVERIFY(!fcc::platform::decodeOssVolume(40 | (255 << 8)).has_value());
    }
    void refusesUnsafeMixerWriteRequests() {
        const fcc::platform::AudioBackend backend;
        QVERIFY(!backend.setMasterVolume("/dev/mixer32", {50, 50}).success);
        QVERIFY(!backend.setMasterVolume("/dev/mixer0/../mixer1", {50, 50}).success);
        QVERIFY(!backend.setMasterVolume("/dev/mixer0", {101, 50}).success);
    }
    void muteRequiresAdvertisedCapability() {
        // A successful READ_MUTE ioctl must not override a missing capability bit.
        QVERIFY(!fcc::platform::supportsOssMute(0));
        QVERIFY(!fcc::platform::supportsOssMute(0x01002831));
        QVERIFY(fcc::platform::supportsOssMute(0x01002831 | (1 << 28)));
    }
};
QTEST_GUILESS_MAIN(PlatformValueTests)
#include "test_platform_values.moc"
