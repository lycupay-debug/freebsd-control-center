/**
 * Responsibility: test popup positioning, dismissal and explicit audio-write requests.
 * Dependencies: Qt Test and the FCC control-panel widget.
 * Collaboration: protects the tray-to-popup and UI-to-service request contract.
 * Side effects: creates offscreen test windows; does not access or mutate hardware.
 */
#include "fcc/gui/control_center/control_panel.h"
#include <QApplication>
#include <QComboBox>
#include <QPushButton>
#include <QSignalSpy>
#include <QSlider>
#include <QTest>
class ControlPanelTests final : public QObject {
    Q_OBJECT
private slots:
    void clampsPopupToAvailableGeometry() {
        const QRect screen(0, 0, 1366, 768);
        const QPoint topLeft = fcc::gui::ControlPanel::popupPosition(QPoint(2, 2), QSize(400, 300), screen);
        QCOMPARE(topLeft, QPoint(0, 2));
        const QPoint bottomRight = fcc::gui::ControlPanel::popupPosition(QPoint(1364, 766), QSize(400, 300), screen);
        QCOMPARE(bottomRight, QPoint(966, 466));
    }
    void popupClosesOnEscape() {
        fcc::gui::ControlPanel panel;
        panel.openAt(QPoint(400, 300), QRect(0, 0, 1366, 768));
        QVERIFY(panel.isVisible());
        QTest::keyClick(&panel, Qt::Key_Escape);
        QTRY_VERIFY(!panel.isVisible());
    }
    void audioApplyEmitsSelectedDeviceAndValue() {
        fcc::gui::ControlPanel panel;
        panel.setAudioDevices({QStringLiteral("Default mixer")},
                              {QStringLiteral("/dev/mixer0")}, {60}, {75},
                              QStringLiteral("/dev/mixer0"));
        auto *left = panel.findChild<QSlider *>(QStringLiteral("fccAudioLeft"));
        auto *right = panel.findChild<QSlider *>(QStringLiteral("fccAudioRight"));
        auto *apply = panel.findChild<QPushButton *>(QStringLiteral("fccApplyVolume"));
        QVERIFY(left != nullptr);
        QVERIFY(right != nullptr);
        QVERIFY(apply != nullptr);
        QSignalSpy spy(&panel, &fcc::gui::ControlPanel::volumeApplyRequested);
        left->setValue(55);
        right->setValue(65);
        QCOMPARE(spy.size(), 0); // preview changes never issue hardware writes
        apply->click();
        QCOMPARE(spy.size(), 1);
        QCOMPARE(spy.at(0).at(0).toString(), QStringLiteral("/dev/mixer0"));
        QCOMPARE(spy.at(0).at(1).toInt(), 55);
        QCOMPARE(spy.at(0).at(2).toInt(), 65);
    }
    void selectedMixerUsesItsOwnReadbackVolume() {
        fcc::gui::ControlPanel panel;
        panel.setAudioDevices({QStringLiteral("Default"), QStringLiteral("Headphones")},
                              {QStringLiteral("/dev/mixer0"), QStringLiteral("/dev/mixer1")},
                              {60, 86}, {60, 86}, QStringLiteral("/dev/mixer1"));
        auto *left = panel.findChild<QSlider *>(QStringLiteral("fccAudioLeft"));
        auto *right = panel.findChild<QSlider *>(QStringLiteral("fccAudioRight"));
        auto *device = panel.findChild<QComboBox *>(QStringLiteral("fccAudioDevice"));
        QVERIFY(left != nullptr);
        QVERIFY(right != nullptr);
        QVERIFY(device != nullptr);
        QCOMPARE(left->value(), 86);
        QCOMPARE(right->value(), 86);
        device->setCurrentIndex(0);
        QCOMPARE(left->value(), 60);
        QCOMPARE(right->value(), 60);
        device->setCurrentIndex(1);
        QCOMPARE(left->value(), 86);
        QCOMPARE(right->value(), 86);
        panel.setAudioVolume(54, 74);
        device->setCurrentIndex(0);
        device->setCurrentIndex(1);
        QCOMPARE(left->value(), 54);
        QCOMPARE(right->value(), 74);
    }
    void switchingMixerLoadsItsOwnStereoValues() {
        fcc::gui::ControlPanel panel;
        panel.setAudioDevices({QStringLiteral("Default"), QStringLiteral("Headphones")},
                              {QStringLiteral("/dev/mixer0"), QStringLiteral("/dev/mixer1")},
                              {60, 86}, {62, 91}, QStringLiteral("/dev/mixer0"));
        auto *combo = panel.findChild<QComboBox *>(QStringLiteral("fccAudioDevice"));
        auto *left = panel.findChild<QSlider *>(QStringLiteral("fccAudioLeft"));
        auto *right = panel.findChild<QSlider *>(QStringLiteral("fccAudioRight"));
        QVERIFY(combo != nullptr);
        QVERIFY(left != nullptr);
        QVERIFY(right != nullptr);
        combo->setCurrentIndex(1);
        QCOMPARE(left->value(), 86);
        QCOMPARE(right->value(), 91);
    }
    void noMixerDisablesVolumeWrite() {
        fcc::gui::ControlPanel panel;
        panel.setAudioDevices({}, {}, {}, {}, {});
        auto *left = panel.findChild<QSlider *>(QStringLiteral("fccAudioLeft"));
        auto *right = panel.findChild<QSlider *>(QStringLiteral("fccAudioRight"));
        auto *apply = panel.findChild<QPushButton *>(QStringLiteral("fccApplyVolume"));
        QVERIFY(left != nullptr);
        QVERIFY(right != nullptr);
        QVERIFY(apply != nullptr);
        QVERIFY(!left->isEnabled());
        QVERIFY(!right->isEnabled());
        QVERIFY(!apply->isEnabled());
    }
    void positionsOnOffsetMonitor() {
        const QRect screen(1366, 0, 1920, 1080);
        const QPoint position = fcc::gui::ControlPanel::popupPosition(
            QPoint(3270, 1070), QSize(400, 300), screen);
        QCOMPARE(position, QPoint(2886, 770));
    }
};
QTEST_MAIN(ControlPanelTests)
#include "test_control_panel.moc"
