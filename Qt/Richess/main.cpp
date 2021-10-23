#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <QQuickView>
#include <QObject>

#include <QQmlEngine>
#include <QQmlContext>

#include "Client.h"

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    QQuickView view;

    Client client;
    client.init();

    qmlRegisterType<Client>("Client", 1, 0, "Client");
    qRegisterMetaType<Client*>("Client");

    view.resize(1024, 600);
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.engine()->rootContext()->setContextProperty("client", &client);
    view.setSource(QUrl("qrc:/main.qml"));
    view.show();

    return app.exec();
}
