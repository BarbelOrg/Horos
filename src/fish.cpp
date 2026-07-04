#include "fish.hpp"
#include "unix_utils.hpp"

#include <QDebug>
#include <QProcess>
#include <QtLogging>
#include <fast_io.h>
#include <fast_io_device.h>
#include <flux.hpp>
#include <iostream>
#include <ranges>

using namespace std;

static optional<filesystem::path> GetFishVariablesPath()
{
    auto vars = GetConfigPath() / "fish" / "fish_variables";
    if (!exists(vars) || !is_regular_file(vars))
        return nullopt;
    return vars;
}

std::vector<EnvVar> FishGetAll()
{
    auto varsPath = GetFishVariablesPath();
    if (!varsPath)
        return {};

    auto vars = std::vector<EnvVar>();
    auto file = fast_io::native_file_loader(*varsPath);

    auto prefix = std::string_view("SETUVAR --export ");
    auto delimiter = ':';

    for (const auto line : file | std::views::split('\n'))
    {
        auto lineStr = std::string_view(line.data(), line.size());
        if (!lineStr.starts_with(prefix))
            continue;

        auto payload = lineStr.substr(prefix.size());
        auto delimPos = payload.find(delimiter);
        if (delimPos == std::string_view::npos)
            continue;

        auto key = payload.substr(0, delimPos);
        auto value = payload.substr(delimPos + 1);

        vars.push_back(EnvVar{.name = QString::fromUtf8(key.data(), key.size()),
                              .value = QString::fromUtf8(value.data(), value.size()),
                              .managed = false});
    }

    return vars;
}

void FishSet(const QString &name, const QString &value)
{
    QProcess process;
    process.setProgram("fish");
    process.setArguments({
        "-c",
        "set -Ux -- $argv[1] $argv[2]",
        name,
        value
    });
    process.start();
    if (!process.waitForFinished(5000))
    {
        qCritical() << "Fish timeout to set " << name;
    }
    if (process.exitCode() != 0)
    {
        qCritical() << "fish set failed:" << process.readAllStandardError();
    }
}

void FishDelete(const QString &name)
{
    auto proc = QProcess();
    proc.setProgram(QStringLiteral("fish"));
    proc.setArguments({
        QStringLiteral("-c"),
        QStringLiteral("set -e -U -- $argv[1]"),
        name,
    });
    proc.start();
    if (!proc.waitForFinished(5000))
    {
        qCritical() << "fish erase timed out for" << name;
    }
}
