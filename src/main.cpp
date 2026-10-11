#include <Browser.h>
#include <QQmlContext>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include<URL.h>
// #include <QTextStream>
// #include <cstdlib>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    URL url;
    engine.rootContext()->setContextProperty("Main_Url", &url);
    engine.loadFromModule("Awax_browser", "Main");
    if (engine.rootObjects().isEmpty()){return -1;}

    return app.exec();
}
