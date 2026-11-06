/**
 * Responsibility: implement popup layout, positioning and user-request signals.
 * Dependencies: Qt Widgets only.
 * Collaboration: emits requests to the app composition root; owns no services.
 * Side effects: a user must explicitly activate Apply before a volume write is requested.
 */
#include "fcc/gui/control_center/control_panel.h"
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSlider>
#include <QVBoxLayout>
#include <algorithm>
namespace fcc::gui {
ControlPanel::ControlPanel(QWidget *parent)
    : QFrame(parent, Qt::Popup | Qt::FramelessWindowHint)
{
    setObjectName(QStringLiteral("fccControlPanel"));
    setWindowTitle(QStringLiteral("FreeBSD Control Center"));
    setMinimumWidth(380);
    setFrameShape(QFrame::StyledPanel);
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(18, 16, 18, 16);
    layout->setSpacing(9);
    auto *title = new QLabel(QStringLiteral("Control Center"), this);
    title->setObjectName(QStringLiteral("fccPanelTitle"));
    title->setStyleSheet(QStringLiteral("font-size: 18px; font-weight: 600;"));
    status_ = new QLabel(QStringLiteral("Reading system state…"), this);
    status_->setObjectName(QStringLiteral("fccPlatformStatus"));
    status_->setWordWrap(true);
    auto *audioTitle = new QLabel(QStringLiteral("Output volume"), this);
    audioDevice_ = new QComboBox(this);
    audioDevice_->setObjectName(QStringLiteral("fccAudioDevice"));
    leftVolumeLabel_ = new QLabel(QStringLiteral("Left: unavailable"), this);
    rightVolumeLabel_ = new QLabel(QStringLiteral("Right: unavailable"), this);
    leftSlider_ = new QSlider(Qt::Horizontal, this);
    leftSlider_->setObjectName(QStringLiteral("fccAudioLeft"));
    rightSlider_ = new QSlider(Qt::Horizontal, this);
    rightSlider_->setObjectName(QStringLiteral("fccAudioRight"));
    for (auto *slider : {leftSlider_, rightSlider_}) {
        slider->setRange(0, 100);
        slider->setEnabled(false);
    }
    applyVolume_ = new QPushButton(QStringLiteral("Apply volume"), this);
    applyVolume_->setObjectName(QStringLiteral("fccApplyVolume"));
    applyVolume_->setEnabled(false);
    auto *refresh = new QPushButton(QStringLiteral("Refresh system state"), this);
    auto *notifications = new QPushButton(QStringLiteral("Notification Center"), this);
    auto *quit = new QPushButton(QStringLiteral("Quit"), this);
    layout->addWidget(title);
    layout->addWidget(status_);
    layout->addSpacing(4);
    layout->addWidget(audioTitle);
    layout->addWidget(audioDevice_);
    layout->addWidget(leftVolumeLabel_);
    layout->addWidget(leftSlider_);
    layout->addWidget(rightVolumeLabel_);
    layout->addWidget(rightSlider_);
    layout->addWidget(applyVolume_);
    layout->addSpacing(4);
    layout->addWidget(refresh);
    layout->addWidget(notifications);
    layout->addWidget(quit);
    connect(refresh, &QPushButton::clicked, this, &ControlPanel::refreshRequested);
    connect(notifications, &QPushButton::clicked, this, &ControlPanel::notificationCenterRequested);
    connect(quit, &QPushButton::clicked, this, &ControlPanel::quitRequested);
    connect(audioDevice_, &QComboBox::currentIndexChanged, this,
            [this](int) { updateSelectedAudioDevice(); });
    connect(leftSlider_, &QSlider::valueChanged, this,
            [this](int) { updateVolumeLabel(); });
    connect(rightSlider_, &QSlider::valueChanged, this,
            [this](int) { updateVolumeLabel(); });
    connect(applyVolume_, &QPushButton::clicked, this, [this]() {
        const QString path = audioDevice_->currentData().toString();
        if (!path.isEmpty() && leftSlider_->isEnabled())
            emit volumeApplyRequested(path, leftSlider_->value(), rightSlider_->value());
    });
    setStyleSheet(QStringLiteral(
        "QFrame#fccControlPanel { background: palette(window); color: palette(window-text); "
        "border: 1px solid palette(mid); border-radius: 12px; }"
        "QPushButton { min-height: 30px; text-align: left; padding: 4px 8px; }"));
    adjustSize();
}
void ControlPanel::setStatusText(const QString &text)
{
    status_->setText(text);
    adjustSize();
}
void ControlPanel::appendStatusLine(const QString &text)
{
    status_->setText(status_->text() + QLatin1Char('\n') + text);
    adjustSize();
}
void ControlPanel::setAudioDevices(const QStringList &labels, const QStringList &paths,
                                   const QList<int> &leftVolumes, const QList<int> &rightVolumes,
                                   const QString &preferredPath)
{
    const QString previousPath = audioDevice_->currentData().toString();
    const QString selectedPath = previousPath.isEmpty() ? preferredPath : previousPath;
    audioPaths_ = paths;
    leftVolumes_ = leftVolumes;
    rightVolumes_ = rightVolumes;
    const QSignalBlocker blocker(audioDevice_);
    audioDevice_->clear();
    for (qsizetype i = 0; i < paths.size() && i < labels.size(); ++i)
        audioDevice_->addItem(labels.at(i), paths.at(i));
    int index = audioDevice_->findData(selectedPath);
    if (index < 0) index = audioDevice_->findData(preferredPath);
    if (index < 0 && audioDevice_->count() > 0) index = 0;
    audioDevice_->setCurrentIndex(index);
    updateSelectedAudioDevice();
    adjustSize();
}
void ControlPanel::setAudioVolume(int left, int right)
{
    confirmedLeft_ = std::clamp(left, 0, 100);
    confirmedRight_ = std::clamp(right, 0, 100);
    const int index = audioDevice_->currentIndex();
    if (index >= 0 && index < leftVolumes_.size() && index < rightVolumes_.size()) {
        leftVolumes_[index] = confirmedLeft_;
        rightVolumes_[index] = confirmedRight_;
    }
    leftSlider_->setValue(confirmedLeft_);
    rightSlider_->setValue(confirmedRight_);
    updateVolumeLabel();
}
void ControlPanel::restoreAudioVolume()
{
    leftSlider_->setValue(confirmedLeft_);
    rightSlider_->setValue(confirmedRight_);
    updateVolumeLabel();
}
void ControlPanel::updateSelectedAudioDevice()
{
    const int index = audioDevice_->currentIndex();
    const bool available = index >= 0 && index < audioPaths_.size() &&
        index < leftVolumes_.size() && index < rightVolumes_.size();
    leftSlider_->setEnabled(available);
    rightSlider_->setEnabled(available);
    applyVolume_->setEnabled(available);
    if (available) {
        confirmedLeft_ = std::clamp(leftVolumes_.at(index), 0, 100);
        confirmedRight_ = std::clamp(rightVolumes_.at(index), 0, 100);
        leftSlider_->setValue(confirmedLeft_);
        rightSlider_->setValue(confirmedRight_);
    } else {
        confirmedLeft_ = 0;
        confirmedRight_ = 0;
        leftSlider_->setValue(0);
        rightSlider_->setValue(0);
    }
    updateVolumeLabel();
}
void ControlPanel::setVolumeBusy(bool busy)
{
    const bool available = audioDevice_->currentIndex() >= 0;
    audioDevice_->setEnabled(available && !busy);
    leftSlider_->setEnabled(available && !busy);
    rightSlider_->setEnabled(available && !busy);
    applyVolume_->setEnabled(available && !busy);
    applyVolume_->setText(busy ? QStringLiteral("Applying…") : QStringLiteral("Apply volume"));
}
void ControlPanel::updateVolumeLabel()
{
    if (audioDevice_->currentIndex() < 0) {
        leftVolumeLabel_->setText(QStringLiteral("Left: unavailable"));
        rightVolumeLabel_->setText(QStringLiteral("Right: unavailable"));
        return;
    }
    leftVolumeLabel_->setText(QStringLiteral("Left: %1%").arg(leftSlider_->value()));
    rightVolumeLabel_->setText(QStringLiteral("Right: %1%").arg(rightSlider_->value()));
}
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
