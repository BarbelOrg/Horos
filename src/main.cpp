#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QIcon>
#include <QQuickStyle>
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDebug>
#include <glaze/glaze.hpp>
#include <CLI/CLI.hpp>

#include "shell.hpp"
#include "env_var_model.hpp"

#include <fast_io.h>
#include <fast_io_device.h>

static int RunGUI(int argc, char** argv)
{
    QGuiApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/resources/horos.svg"));
    app.setDesktopFileName("horos");

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

static int RunCLI(int argc, char** argv, EnvVarManager& envManager)
{
    auto app = CLI::App("Environment variable editor", "horos");

    auto listCmd = app.add_subcommand("list", "List all managed environment variables");

    auto getCmd = app.add_subcommand("get", "Get the value of one managed environment variable");
    auto getKey = std::string();
    getCmd->add_option("key", getKey, "Environment variable key");

    auto setCmd = app.add_subcommand("set", "Set an environment variable");
    auto setKey = std::string();
    auto setValue = std::string();
    setCmd->add_option("key", setKey, "Environment variable key")->required();
    setCmd->add_option("value", setValue, "Environment variable value")->required();

    auto addPath = app.add_subcommand("add-path", "Add a value to a PATH-like environment variable");
    auto addPathKey = std::string();
    auto addPathValue = std::string();
    addPath->add_option("key", addPathKey, "Environment variable key")->required();
    addPath->add_option("value", addPathValue, "Environment variable value")->required();

    auto removePath = app.add_subcommand("remove-path", "Remove an entry from a PATH-like environment variable");
    auto removePathKey = std::string();
    auto removePathValue = std::string();
    removePath->add_option("key", removePathKey, "Environment variable key")->required();
    removePath->add_option("value", removePathValue, "Environment variable value")->required();

    auto removeCmd = app.add_subcommand("remove", "Remove one environment variable");
    auto removeKey = std::string();
    removeCmd->add_option("key", removeKey, "Environment variable key");

    auto clearCmd = app.add_subcommand("clear", "Clear all environment variables");

    CLI11_PARSE(app, argc, argv);

    if (*listCmd)
    {
        for (const auto& [k, v] : envManager.Vars())
        {
            fast_io::io::println(k, "=", v);
        }
    }
    else if (*getCmd)
    {
        const auto& vars = envManager.Vars();
        auto it = vars.find(getKey);
        if (it != vars.end())
        {
            fast_io::io::println(it->second);
        }
        else
        {
            fast_io::io::println("Variable not found");
        }
    }
    else if (*setCmd)
    {
        if (envManager.Set(setKey, setValue))
        {
            fast_io::io::println("Updated ", setKey, " to ", setValue);
        }
        else
        {
            fast_io::io::println("Created env var ", setKey, " with value ", setValue);
        }
    }
    else if (*addPath)
    {
        if (envManager.AddPathLike(addPathKey, addPathValue))
        {
            fast_io::io::println("Added path-like entry ", addPathKey, " to ", addPathValue);
        }
        else
        {
            fast_io::io::println("Created path-like entry ", addPathKey, " with value ", addPathValue);
        }
    }
    else if (*removePath)
    {
        if (envManager.RemovePathLike(removePathKey, removePathValue))
        {
            fast_io::io::println("Removed path-like entry ", removePathKey, " from ", removePathValue);
        }
        else
        {
            fast_io::io::println("Path-like entry ", removePathKey, " not found or does not contain ", removePathValue);
        }
    }
    else if (*removeCmd)
    {
        if (envManager.Remove(removeKey))
        {
            fast_io::io::println("Removed env var ", removeKey);
        }
        else
        {
            fast_io::io::println("Env var ", removeKey, " not found");
        }
    }
    else if (*clearCmd)
    {
        envManager.Clear();
        fast_io::io::println("Cleared all env vars");
    }

    envManager.Save();

    return 0;
}

int main(int argc, char *argv[])
{
    // GUI mode (no args)
    if (argc == 1)
    {
        return RunGUI(argc, argv);
    }
    // CLI mode
    else
    {
        auto manager = EnvVarManager::LoadForCurrentShell();
        return RunCLI(argc, argv, manager);
    }
}
