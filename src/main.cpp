#include <DnsResolver.h>
#include <URL.h>
#include <QDebug>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    URL url;
    DnsResolver dns;

    QObject::connect(&url, &URL::navigationRequested,
                     &dns, [&dns](const ParsedUrl &parsed) {
                         dns.resolve(parsed.host);
                     });

    engine.rootContext()->setContextProperty("Main_Url", &url);
    engine.loadFromModule("Awax_browser", "Main");
    
    if (engine.rootObjects().isEmpty()){return -1;}

    return app.exec();
}
