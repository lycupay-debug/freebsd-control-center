/**
 * Responsibility: FCC composition root and P0 desktop/platform capability display.
 * Dependencies: Qt Widgets/Concurrent and platform backend interfaces.
 * Collaboration: starts tray/window UI; invokes read-only platform probes asynchronously.
 * Side effects: creates a GUI/tray and performs read-only backlight/mixer queries.
 * Constraint: run as normal desktop user; no device write or privilege escalation.
 */
#include "fcc/platform/audio_backend.h"
#include "fcc/platform/brightness_backend.h"
#include <QAction>
#include <QApplication>
#include <QFutureWatcher>
#include <QLabel>
#include <QMenu>
#include <QStyle>
#include <QSystemTrayIcon>
#include <QVBoxLayout>
#include <QWidget>
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
    QApplication::setApplicationName("FreeBSD Control Center");
    QApplication::setApplicationVersion("0.1.0");
    QApplication::setOrganizationName("FCC");
    QWidget window;
    window.setWindowTitle("FreeBSD Control Center - P0");
    window.resize(520, 300);
    auto *layout = new QVBoxLayout(&window);
    auto *title = new QLabel("FreeBSD Control Center", &window);
    auto *status = new QLabel("Qt Widgets initialized.\nReading hardware state (read-only)…", &window);
    title->setStyleSheet("font-size: 20px; font-weight: 600;");
    status->setWordWrap(true);
    layout->addWidget(title);
    layout->addWidget(status);

    QFutureWatcher<QString> probeWatcher;
    QObject::connect(&probeWatcher, &QFutureWatcher<QString>::finished, &window,
        [&probeWatcher, status]() { status->setText(probeWatcher.result()); });
    probeWatcher.setFuture(QtConcurrent::run(&probePlatform));

    const bool trayAvailable = QSystemTrayIcon::isSystemTrayAvailable();
    QSystemTrayIcon tray;
    if (trayAvailable) {
        tray.setIcon(app.style()->standardIcon(QStyle::SP_ComputerIcon));
        tray.setToolTip("FreeBSD Control Center");
        auto *menu = new QMenu(&window);
        auto *showAction = menu->addAction("Show FCC");
        auto *quitAction = menu->addAction("Quit");
        QObject::connect(showAction, &QAction::triggered, &window, [&window]() {
            window.show(); window.raise(); window.activateWindow();
        });
        QObject::connect(quitAction, &QAction::triggered, &app, &QApplication::quit);
        tray.setContextMenu(menu);
        QObject::connect(&tray, &QSystemTrayIcon::activated, &window,
            [&window](QSystemTrayIcon::ActivationReason reason) {
                if (reason == QSystemTrayIcon::Trigger) {
                    window.show(); window.raise(); window.activateWindow();
                }
            });
        tray.show();
    }
    window.show();
    return app.exec();
}
