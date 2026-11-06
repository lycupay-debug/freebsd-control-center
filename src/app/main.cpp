/**
 * Responsibility: compose the tray application, transient control panel and platform services.
 * Dependencies: Qt Widgets/Concurrent and FreeBSD platform adapters.
 * Collaboration: owns async probe/write lifetimes and routes explicit UI requests.
 * Side effects: reads devices during refresh; writes master volume only after explicit Apply.
 * Constraint: normal-user process; no shell commands or privilege escalation.
 */
#include "fcc/gui/control_center/control_panel.h"
#include "fcc/platform/audio_backend.h"
#include "fcc/platform/brightness_backend.h"
#include <QAction>
#include <QApplication>
#include <QCursor>
#include <QDialog>
#include <QFutureWatcher>
#include <QGuiApplication>
#include <QLabel>
#include <QMenu>
#include <QPainter>
#include <QPixmap>
#include <QPen>
#include <QRectF>
#include <QIcon>
#include <QScreen>
#include <QStyle>
#include <QSystemTrayIcon>
#include <QVBoxLayout>
#include <QtConcurrent/QtConcurrentRun>
#include <QStringList>
namespace {
struct ProbeSnapshot {
    QString status;
    QStringList audioLabels;
    QStringList audioPaths;
    QList<int> audioLeftVolumes;
    QList<int> audioRightVolumes;
    QString defaultAudioPath;
};
ProbeSnapshot probePlatform()
{
    ProbeSnapshot snapshot;
    QStringList lines;
    const fcc::platform::BrightnessBackend brightness;
    const auto brightnessReport = brightness.discover();
    if (brightnessReport.available()) {
        for (const auto &device : brightnessReport.devices) {
            lines << QStringLiteral("Brightness: %1 (%2), current %3, range %4")
                .arg(QString::fromStdString(device.name), QString::fromStdString(device.type))
                .arg(device.current)
                .arg(device.minimum && device.maximum
                    ? QStringLiteral("%1..%2").arg(*device.minimum).arg(*device.maximum)
                    : QStringLiteral("not reported"));
        }
    } else lines << QStringLiteral("Brightness: unavailable");
    for (const auto &issue : brightnessReport.issues)
        lines << QStringLiteral("Brightness diagnostic: %1").arg(QString::fromStdString(issue.detail));
    const fcc::platform::AudioBackend audio;
    const auto audioReport = audio.discover();
    if (audioReport.available()) {
        for (const auto &device : audioReport.devices) {
            const QString path = QString::fromStdString(device.path);
            const QString label = QStringLiteral("%1%2 — %3/%4")
                .arg(QString::fromStdString(device.name))
                .arg(device.isDefault ? QStringLiteral(" [default]") : QString())
                .arg(device.masterVolume.left).arg(device.masterVolume.right);
            snapshot.audioPaths << path;
            snapshot.audioLabels << label;
            snapshot.audioLeftVolumes << device.masterVolume.left;
            snapshot.audioRightVolumes << device.masterVolume.right;
            if (device.isDefault || snapshot.defaultAudioPath.isEmpty())
                snapshot.defaultAudioPath = path;
            lines << QStringLiteral("Audio: %1, master L/R %2/%3")
                .arg(QString::fromStdString(device.name))
                .arg(device.masterVolume.left).arg(device.masterVolume.right);
            lines << QStringLiteral("Audio mute: %1")
                .arg(device.muteSupported ? QStringLiteral("advertised by device")
                                          : QStringLiteral("unsupported by device"));
        }
    } else lines << QStringLiteral("Audio: unavailable");
    for (const auto &issue : audioReport.issues)
        lines << QStringLiteral("Audio diagnostic: %1").arg(QString::fromStdString(issue.detail));
    snapshot.status = lines.join(QLatin1Char('\n'));
    return snapshot;
}
}
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("FreeBSD Control Center"));
    QApplication::setApplicationVersion(QStringLiteral("0.1.0"));
    QApplication::setOrganizationName(QStringLiteral("FCC"));
    app.setQuitOnLastWindowClosed(false);
    fcc::gui::ControlPanel panel;
    QFutureWatcher<ProbeSnapshot> probeWatcher;
    QFutureWatcher<fcc::platform::VolumeWriteResult> volumeWatcher;
    const auto refresh = [&panel, &probeWatcher, &volumeWatcher]() {
        if (probeWatcher.isRunning() || volumeWatcher.isRunning()) return;
        panel.setVolumeBusy(true);
        panel.setStatusText(QStringLiteral("Reading system state (read-only)…"));
        probeWatcher.setFuture(QtConcurrent::run(&probePlatform));
    };
    QObject::connect(&probeWatcher, &QFutureWatcher<ProbeSnapshot>::finished, &panel,
        [&panel, &probeWatcher]() {
            const auto snapshot = probeWatcher.result();
            panel.setStatusText(snapshot.status);
            panel.setAudioDevices(snapshot.audioLabels, snapshot.audioPaths,
                                  snapshot.audioLeftVolumes, snapshot.audioRightVolumes, snapshot.defaultAudioPath);
            panel.setVolumeBusy(false);
        });
    QObject::connect(&panel, &fcc::gui::ControlPanel::refreshRequested, &panel, refresh);
    QObject::connect(&panel, &fcc::gui::ControlPanel::volumeApplyRequested, &panel,
        [&panel, &probeWatcher, &volumeWatcher](const QString &path, int left, int right) {
            if (probeWatcher.isRunning() || volumeWatcher.isRunning()) return;
            panel.setVolumeBusy(true);
            volumeWatcher.setFuture(QtConcurrent::run([path, left, right]() {
                const fcc::platform::AudioBackend backend;
                return backend.setMasterVolume(path.toStdString(), {left, right});
            }));
        });
    QObject::connect(&volumeWatcher, &QFutureWatcher<fcc::platform::VolumeWriteResult>::finished,
        &panel, [&panel, &volumeWatcher]() {
            const auto result = volumeWatcher.result();
            panel.setVolumeBusy(false);
            if (result.actual)
                panel.setAudioVolume(result.actual->left, result.actual->right);
            else
                panel.restoreAudioVolume();
            panel.appendStatusLine(QStringLiteral("Audio volume: %1")
                .arg(QString::fromStdString(result.detail)));
            if (!result.actual) refresh();
        });
    QDialog notificationCenter;
    notificationCenter.setWindowTitle(QStringLiteral("Notification Center"));
    notificationCenter.resize(360, 220);
    auto *notificationLayout = new QVBoxLayout(&notificationCenter);
    auto *notificationText = new QLabel(QStringLiteral("No notifications yet."), &notificationCenter);
    notificationText->setWordWrap(true);
    notificationLayout->addWidget(notificationText);
    QObject::connect(&panel, &fcc::gui::ControlPanel::notificationCenterRequested,
                     &notificationCenter, [&notificationCenter]() {
        notificationCenter.show(); notificationCenter.raise(); notificationCenter.activateWindow();
    });
    QObject::connect(&panel, &fcc::gui::ControlPanel::quitRequested, &app, &QApplication::quit);
    QPixmap trayPixmap(22, 22);
    trayPixmap.fill(Qt::transparent);
    {
        QPainter painter(&trayPixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        QPen pen(app.palette().color(QPalette::WindowText));
        pen.setWidthF(1.7);
        painter.setPen(pen);
        painter.drawRoundedRect(QRectF(3.0, 3.0, 16.0, 16.0), 3.0, 3.0);
        painter.drawLine(QPointF(7.0, 8.0), QPointF(15.0, 8.0));
        painter.drawLine(QPointF(7.0, 12.0), QPointF(15.0, 12.0));
        painter.drawLine(QPointF(7.0, 16.0), QPointF(12.0, 16.0));
    }
    QSystemTrayIcon tray;
    tray.setIcon(QIcon(trayPixmap));
    tray.setToolTip(QStringLiteral("FreeBSD Control Center"));
    QMenu trayMenu;
    QAction *showAction = trayMenu.addAction(QStringLiteral("Open Control Center"));
    QAction *notificationsAction = trayMenu.addAction(QStringLiteral("Notification Center"));
    trayMenu.addSeparator();
    QAction *quitAction = trayMenu.addAction(QStringLiteral("Quit"));
    tray.setContextMenu(&trayMenu);
    const auto openPanel = [&tray, &panel]() {
        const QRect trayGeometry = tray.geometry();
        const QPoint anchor = trayGeometry.isValid() && !trayGeometry.isEmpty()
            ? trayGeometry.center() : QCursor::pos();
        QScreen *screen = QGuiApplication::screenAt(anchor);
        if (!screen) screen = QGuiApplication::primaryScreen();
        if (screen) panel.openAt(anchor, screen->availableGeometry());
        else panel.show();
    };
    QObject::connect(showAction, &QAction::triggered, &panel, openPanel);
    QObject::connect(notificationsAction, &QAction::triggered, &notificationCenter,
        [&notificationCenter]() { notificationCenter.show(); notificationCenter.raise(); });
    QObject::connect(quitAction, &QAction::triggered, &app, &QApplication::quit);
    QObject::connect(&tray, &QSystemTrayIcon::activated, &panel,
        [&openPanel](QSystemTrayIcon::ActivationReason reason) {
            if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick)
                openPanel();
        });
    QObject::connect(&app, &QCoreApplication::aboutToQuit, &app, [&]() {
        if (probeWatcher.isRunning()) probeWatcher.waitForFinished();
        if (volumeWatcher.isRunning()) volumeWatcher.waitForFinished();
    });
    tray.show();
    refresh();
    if (QSystemTrayIcon::isSystemTrayAvailable()) openPanel();
    else if (auto *screen = QGuiApplication::primaryScreen())
        panel.openAt(screen->availableGeometry().center(), screen->availableGeometry());
    return app.exec();
}
