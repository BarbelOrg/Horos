#include "bash_like.hpp"
#include "env_var_manager.hpp"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <glaze/glaze.hpp>

#include "env_var_model.hpp"
#include "fish.hpp"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQuickStyle::setStyle("org.kde.desktop");

    qDebug() << "Current shell: " << static_cast<int>(GetShell().value());

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app, []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    qmlRegisterType<EnvVarModel>("horos", 1, 0, "EnvVarModel");

    engine.load(QUrl(QStringLiteral("qrc:/ui/Main.qml")));

    if (engine.rootObjects().isEmpty())
        return -1;

    return QGuiApplication::exec();
}
