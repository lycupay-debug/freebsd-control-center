/**
 * Responsibility: FCC entry point and minimal Qt Widgets/tray capability probe.
 * Dependencies: Qt 6 Widgets only; no hardware or privileged interfaces.
 * Collaboration: future core/services are composed here; device access stays in
 * service/platform adapters and never enters UI widgets.
 * Side effects: creates a GUI and optional tray icon; writes no system settings.
 * Constraint: run as normal desktop user in an active graphical session.
 */
#include <QAction>
#include <QApplication>
#include <QLabel>
#include <QMenu>
#include <QStyle>
#include <QSystemTrayIcon>
#include <QVBoxLayout>
#include <QWidget>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("FreeBSD Control Center");
    QApplication::setApplicationVersion("0.1.0");
    QApplication::setOrganizationName("FCC");

    QWidget window;
    window.setWindowTitle("FreeBSD Control Center - P0");
    window.resize(440, 220);
    auto *layout = new QVBoxLayout(&window);
    auto *title = new QLabel("FreeBSD Control Center", &window);
    auto *status = new QLabel(&window);
    title->setStyleSheet("font-size: 20px; font-weight: 600;");
    status->setWordWrap(true);

    const bool trayAvailable = QSystemTrayIcon::isSystemTrayAvailable();
    status->setText(QStringLiteral(
        "Qt Widgets initialized.\nSystem tray available: %1\n"
        "Brightness/audio backends: not implemented.")
        .arg(trayAvailable ? QStringLiteral("yes") : QStringLiteral("no")));
    layout->addWidget(title);
    layout->addWidget(status);

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
