#include <DnsResolver.h>
#include <Browser.h>
#include <URL.h>
#include <QDebug>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    
    QThread::currentThread()->setObjectName("MAIN THREAD");

    BROWSER Browser;
    Browser.RUN();
    
    engine.rootContext()->setContextProperty("Main_Url", &Browser.Give_Url_address());
    engine.loadFromModule("Awax_browser", "Main");  
    
    if (engine.rootObjects().isEmpty()){return -1;}

    return app.exec();
}
