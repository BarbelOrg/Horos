#include "bash_like.hpp"

#include "unix_utils.hpp"

#include <ctre.hpp>
#include <fast_io.h>
#include <fast_io_device.h>
#include <filesystem>
#include <fmt/core.h>

using namespace std;

optional<filesystem::path> GetRcPathForShell(Shell shell)
{
    switch (shell)
    {
    case Shell::Bash: {
        auto bashrc = GetHomePath().value() / ".bashrc";
        if (filesystem::exists(bashrc) && filesystem::is_regular_file(bashrc))
            return bashrc;

        auto bashProfile = GetHomePath().value() / ".bashprofile";
        if (filesystem::exists(bashProfile) && filesystem::is_regular_file(bashProfile))
            return bashProfile;

        return nullopt;
    }
    case Shell::Zsh: {
        auto zshrc = GetHomePath().value() / ".zshrc";
        if (filesystem::exists(zshrc) && filesystem::is_regular_file(zshrc))
            return zshrc;

        auto zprofile = GetHomePath().value() / ".zprofile";
        if (filesystem::exists(zprofile) && filesystem::is_regular_file(zprofile))
            return zprofile;

        return nullopt;
    }
    default: {
        return nullopt;
    }
    }
}

optional<filesystem::path> GetEnvShPathForShell(Shell shell)
{
    switch (shell)
    {
    case Shell::Bash: {
        return GetConfigPath() / "horos" / "env.sh";
    }
    case Shell::Zsh: {
        return GetConfigPath() / "horos" / "env.zsh";
    }
    default: {
        return nullopt;
    }
    }
}

bool EnvIsSourced(Shell shell)
{
    if (shell == Shell::Fish)
        return true;

    auto rc = GetRcPathForShell(shell);
    auto file = fast_io::native_file_loader(*rc);
    auto fileStr = string_view(file.data(), file.size());

    auto source = std::format("source {}", GetEnvShPathForShell(shell).value().string());
    return fileStr.contains(source);
}

void EnsureEnvIsSourced(Shell shell)
{
    constexpr auto kStartMarker = string_view("# Added by horos, do not edit");
    constexpr auto kEndMarker = string_view("# End of horos additions");

    auto source = std::format("source {}", GetEnvShPathForShell(shell).value().string());

    if (!EnvIsSourced(shell))
    {
        auto file = fast_io::obuf_file(*GetRcPathForShell(shell), fast_io::open_mode::app);
        fast_io::io::print(file, "\n", kStartMarker, "\n", source, "\n", kEndMarker, "\n");
    }
}

QList<EnvVar> BashLikeGetAll(Shell shell)
{
    auto path = GetRcPathForShell(shell);
    if (!path)
        throw runtime_error("Could not find rc file for shell");

    auto vars = QList<EnvVar>();
    auto file = fast_io::native_file_loader(*path);

    for (const auto line : file | views::split('\n'))
    {
        // Find export {} = {}
        auto lineStr = string_view(line.data(), line.size());
        if (auto [_, k, v] = ctre::match<R"(^\s*(?:export\s+)?(\w+)\s*=\s*(.*)$)">(lineStr))
        {
            vars.push_back(EnvVar{.name = QString::fromUtf8(k.data(), k.size()),
                                  .value = QString::fromUtf8(v.data(), v.size()),
                                  .managed = false});
        }
    }

    return vars;
}

bool BashLikeSet(const QString &name, const QString &value, Shell shell)
{
    // todo juste tout reecrire a chaque fois
}

bool BashLikeDelete(const QString &name, Shell shell)
{
}