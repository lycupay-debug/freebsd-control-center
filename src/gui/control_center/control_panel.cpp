/**
 * Responsibility: implement popup layout, positioning and user-request signals.
 * Dependencies: Qt Widgets only.
 * Collaboration: emits requests to the app composition root; it owns no services.
 * Side effects: transient top-level popup; Qt handles Escape and outside-click dismissal.
 */
#include "fcc/gui/control_center/control_panel.h"
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <algorithm>
namespace fcc::gui {
ControlPanel::ControlPanel(QWidget *parent)
    : QFrame(parent, Qt::Popup | Qt::FramelessWindowHint)
{
    setObjectName(QStringLiteral("fccControlPanel"));
    setWindowTitle(QStringLiteral("FreeBSD Control Center"));
    setMinimumWidth(360);
    setFrameShape(QFrame::StyledPanel);
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(18, 16, 18, 16);
    layout->setSpacing(10);
    auto *title = new QLabel(QStringLiteral("Control Center"), this);
    title->setObjectName(QStringLiteral("fccPanelTitle"));
    title->setStyleSheet(QStringLiteral("font-size: 18px; font-weight: 600;"));
    status_ = new QLabel(QStringLiteral("Reading system state…"), this);
    status_->setObjectName(QStringLiteral("fccPlatformStatus"));
    status_->setWordWrap(true);
    auto *refresh = new QPushButton(QStringLiteral("Refresh system state"), this);
    auto *notifications = new QPushButton(QStringLiteral("Notification Center"), this);
    auto *quit = new QPushButton(QStringLiteral("Quit"), this);
    layout->addWidget(title);
    layout->addWidget(status_);
    layout->addWidget(refresh);
    layout->addWidget(notifications);
    layout->addWidget(quit);
    connect(refresh, &QPushButton::clicked, this, &ControlPanel::refreshRequested);
    connect(notifications, &QPushButton::clicked, this, &ControlPanel::notificationCenterRequested);
    connect(quit, &QPushButton::clicked, this, &ControlPanel::quitRequested);
    setStyleSheet(QStringLiteral(
        "QFrame#fccControlPanel { background: palette(window); color: palette(window-text); "
        "border: 1px solid palette(mid); border-radius: 12px; }"
        "QPushButton { min-height: 30px; text-align: left; padding: 4px 8px; }"));
    adjustSize();
}
void ControlPanel::setStatusText(const QString &text) { status_->setText(text); adjustSize(); }
QPoint ControlPanel::popupPosition(const QPoint &anchor, const QSize &popupSize,
                                   const QRect &availableGeometry)
{
    const int left = availableGeometry.left();
    const int top = availableGeometry.top();
    const int maxX = std::max(left, availableGeometry.right() - popupSize.width() + 1);
    const int maxY = std::max(top, availableGeometry.bottom() - popupSize.height() + 1);
    int x = anchor.x() - popupSize.width() / 2;
    const bool anchorInLowerHalf = anchor.y() > availableGeometry.center().y();
    int y = anchorInLowerHalf ? anchor.y() - popupSize.height() : anchor.y();
    x = std::clamp(x, left, maxX);
    y = std::clamp(y, top, maxY);
    return {x, y};
}
void ControlPanel::openAt(const QPoint &anchor, const QRect &availableGeometry)
{
    adjustSize();
    move(popupPosition(anchor, size(), availableGeometry));
    show();
    raise();
    activateWindow();
}
} // namespace fcc::gui
