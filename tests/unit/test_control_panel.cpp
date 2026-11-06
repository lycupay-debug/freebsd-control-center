/**
 * Responsibility: test popup positioning and native transient dismissal behavior.
 * Dependencies: Qt Test and the FCC control-panel widget.
 * Collaboration: protects the tray-to-popup interaction contract.
 * Side effects: creates only offscreen test windows; no desktop or device changes.
 */
#include "fcc/gui/control_center/control_panel.h"
#include <QApplication>
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
    void positionsOnOffsetMonitor() {
        const QRect screen(1366, 0, 1920, 1080);
        const QPoint position = fcc::gui::ControlPanel::popupPosition(
            QPoint(3270, 1070), QSize(400, 300), screen);
        QCOMPARE(position, QPoint(2886, 770));
    }
};
QTEST_MAIN(ControlPanelTests)
#include "test_control_panel.moc"
