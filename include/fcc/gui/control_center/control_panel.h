/**
 * Responsibility: compact, transient control-center surface for the desktop tray.
 * Dependencies: Qt Widgets; no platform or hardware APIs.
 * Collaboration: main composition root supplies status text and handles requests.
 * Side effects: creates a popup window; does not access or mutate system devices.
 */
#pragma once
#include <QFrame>
#include <QPoint>
#include <QRect>
#include <QSize>
class QLabel;
namespace fcc::gui {
class ControlPanel final : public QFrame {
    Q_OBJECT
public:
    explicit ControlPanel(QWidget *parent = nullptr);
    void setStatusText(const QString &text);
    void openAt(const QPoint &anchor, const QRect &availableGeometry);
    [[nodiscard]] static QPoint popupPosition(const QPoint &anchor,
                                               const QSize &popupSize,
                                               const QRect &availableGeometry);
signals:
    void refreshRequested();
    void notificationCenterRequested();
    void quitRequested();
private:
    QLabel *status_{};
};
} // namespace fcc::gui
