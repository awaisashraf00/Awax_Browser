#include <Browser.h>
#include <QQmlContext>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
// #include <QTextStream>
// #include <cstdlib>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    // engine.rootContext()->setContextProperty("upper_control", &upper_controls);
    engine.loadFromModule("Awax_browser", "Main");
    if (engine.rootObjects().isEmpty()){return -1;}

    return app.exec();
}
