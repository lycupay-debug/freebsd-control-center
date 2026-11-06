/**
 * Responsibility: compose the tray application, transient control panel and probes.
 * Dependencies: Qt Widgets/Concurrent, FCC GUI surface and FreeBSD platform adapters.
 * Collaboration: owns service lifetime and routes panel requests to app actions.
 * Side effects: creates a tray/popup; performs read-only device discovery asynchronously.
 * Constraint: normal-user process; no hardware writes, shell commands or privilege escalation.
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
#include <QScreen>
#include <QStyle>
#include <QSystemTrayIcon>
#include <QVBoxLayout>
#include <QtConcurrent/QtConcurrentRun>
#include <QStringList>

namespace {
QString probePlatform() {
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
    } else {
        lines << QStringLiteral("Brightness: unavailable");
    }
    for (const auto &issue : brightnessReport.issues)
        lines << QStringLiteral("Brightness diagnostic: %1").arg(QString::fromStdString(issue.detail));

    const fcc::platform::AudioBackend audio;
    const auto audioReport = audio.discover();
    if (audioReport.available()) {
        for (const auto &device : audioReport.devices) {
            lines << QStringLiteral("Audio: %1%2, master L/R %3/%4")
                .arg(QString::fromStdString(device.name))
                .arg(device.isDefault ? QStringLiteral(" [default]") : QString())
                .arg(device.masterVolume.left).arg(device.masterVolume.right);
            lines << QStringLiteral("Audio mute: %1")
                .arg(device.muteSupported ? QStringLiteral("advertised by device")
                                          : QStringLiteral("unsupported by device"));
        }
    } else {
        lines << QStringLiteral("Audio: unavailable");
    }
    for (const auto &issue : audioReport.issues)
        lines << QStringLiteral("Audio diagnostic: %1").arg(QString::fromStdString(issue.detail));
    return lines.join(QLatin1Char('\n'));
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
    QFutureWatcher<QString> probeWatcher;
    const auto refresh = [&panel, &probeWatcher]() {
        if (probeWatcher.isRunning()) return;
        panel.setStatusText(QStringLiteral("Reading hardware state (read-only)…"));
        probeWatcher.setFuture(QtConcurrent::run(&probePlatform));
    };
    QObject::connect(&probeWatcher, &QFutureWatcher<QString>::finished, &panel,
        [&panel, &probeWatcher]() { panel.setStatusText(probeWatcher.result()); });
    QObject::connect(&panel, &fcc::gui::ControlPanel::refreshRequested, &panel, refresh);

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

    QSystemTrayIcon tray;
    tray.setIcon(app.style()->standardIcon(QStyle::SP_ComputerIcon));
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

    tray.show();
    refresh();
    if (QSystemTrayIcon::isSystemTrayAvailable()) openPanel();
    else if (auto *screen = QGuiApplication::primaryScreen())
        panel.openAt(screen->availableGeometry().center(), screen->availableGeometry());
    return app.exec();
}
