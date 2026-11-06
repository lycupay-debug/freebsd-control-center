/**
 * Responsibility: compact transient control surface for the desktop tray.
 * Dependencies: Qt Widgets only; no platform/device APIs.
 * Collaboration: emits requests to the app composition root.
 * Side effects: displays user-selected volume changes only after explicit Apply.
 */
#pragma once
#include <QFrame>
#include <QList>
#include <QStringList>
class QComboBox;
class QLabel;
class QPushButton;
class QSlider;
namespace fcc::gui {
class ControlPanel final : public QFrame {
    Q_OBJECT
public:
    explicit ControlPanel(QWidget *parent = nullptr);
    void setStatusText(const QString &text);
    void appendStatusLine(const QString &text);
    void setAudioDevices(const QStringList &labels, const QStringList &paths,
                         const QList<int> &leftVolumes, const QList<int> &rightVolumes,
                         const QString &preferredPath);
    void setAudioVolume(int left, int right);
    void restoreAudioVolume();
    void setVolumeBusy(bool busy);
    void openAt(const QPoint &anchor, const QRect &availableGeometry);
    [[nodiscard]] static QPoint popupPosition(const QPoint &anchor,
                                               const QSize &popupSize,
                                               const QRect &availableGeometry);
signals:
    void refreshRequested();
    void notificationCenterRequested();
    void volumeApplyRequested(const QString &devicePath, int left, int right);
    void quitRequested();
private:
    void updateSelectedAudioDevice();
    void updateVolumeLabel();
    QLabel *status_{};
    QLabel *leftVolumeLabel_{};
    QLabel *rightVolumeLabel_{};
    QComboBox *audioDevice_{};
    QSlider *leftSlider_{};
    QSlider *rightSlider_{};
    QPushButton *applyVolume_{};
    QList<int> leftVolumes_;
    QList<int> rightVolumes_;
    QStringList audioPaths_;
    int confirmedLeft_{0};
    int confirmedRight_{0};
};
} // namespace fcc::gui
