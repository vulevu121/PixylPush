#include "MainWindow.h"
#include <QApplication>
#include "C:/SmtpClient-for-Qt-1.1/src/SmtpMime"

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication a(argc, argv);
    a.setAttribute(Qt::AA_EnableHighDpiScaling);

    a.setOrganizationName("Pixyl");
    a.setOrganizationDomain("gopixyl.com");
    a.setApplicationName("PixylPush");

    QSettings *settings = new QSettings("Pixyl", "PixylPush");
    settings->setValue("exePath", a.applicationFilePath());
    settings->setValue("version", a.applicationVersion());

    a.setStyle("Fusion");

    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53,53,53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25,25,25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53,53,53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53,53,53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));

    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);

    a.setPalette(darkPalette);
    a.setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");

    MainWindow w;
    w.show();


    return a.exec();

}
