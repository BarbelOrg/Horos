#include "env_var_manager.hpp"

#include "bash_like.hpp"
#include "fish.hpp"
#include "shell.hpp"
#include "string_utils.hpp"
#include "unix_utils.hpp"

#include <ctre.hpp>
#include <QDebug>
#include <fast_io.h>
#include <fast_io_device.h>
#include <glaze/json/write.hpp>
#include <glaze/util/glaze_fast_float.hpp>

bool EnvVarManager::IsValidKey(std::string_view key)
{
    return ctre::match<R"(^[A-Za-z_][A-Za-z0-9_]*$)">(key);
}

EnvVarManager EnvVarManager::LoadForCurrentShell()
{
    return Load(GetShell().value());
}

EnvVarManager EnvVarManager::Load(Shell shell)
{
    auto out = EnvVarManager();
    out.shell = shell;

    auto filename = std::format("horos_{}.json", static_cast<int>(shell));
    auto path = GetConfigPath() / "horos" / filename;
    std::filesystem::create_directories(path.parent_path());
    if (!exists(path))
    {
        auto obuf = fast_io::obuf_file(path);
        fast_io::io::print(obuf, "{}");
    }

    auto ec = glz::read_file_json(out.vars, path.c_str(), std::string());
    if (ec)
        throw std::runtime_error("Failed to read env vars from json");

    return out;
}

void EnvVarManager::Save()
{
    // 1st write the exports
    {
        // For fish we just do directly in set and remove
        if (shell == Shell::Bash || shell == Shell::Zsh)
        {
            EnsureEnvIsSourced(shell);
            auto obuf = fast_io::obuf_file(GetEnvShPathForShell(shell).value(), fast_io::open_mode::out | fast_io::open_mode::trunc);
            for (const auto& [key, value] : vars)
            {
                if (!IsValidKey(key))
                {
                    qCritical() << "Invalid env var key: " << QString::fromUtf8(key.data(), key.size());
                    continue;
                }

                fast_io::io::print(obuf, "export ", key, "=", ShellSingleQuote(value), "\n");
            }
        }
    }

    // 2nd write the json
    {
        auto filename = std::format("horos_{}.json", static_cast<int>(shell));
        auto ec = glz::write_file_json<glz::opts{.prettify = true}>(vars, (GetConfigPath() / "horos" / filename).c_str(), std::string());
        if (ec)
            throw std::runtime_error("Failed to write env vars to json");
    }
}

bool EnvVarManager::Set(const std::string &name, const std::string &value)
{
    if (!IsValidKey(name))
    {
        qCritical() << "Invalid env var key: " << QString::fromUtf8(name.data(), name.size());
        return false;
    }

    auto found = vars.contains(name);
    vars[name] = value;

    if (shell == Shell::Fish)
    {
        FishSet(QString::fromStdString(name), QString::fromStdString(value));
    }

    return found;
}

bool EnvVarManager::Remove(const std::string &name)
{
    auto found = vars.contains(name);
    vars.erase(name);

    if (shell == Shell::Fish)
    {
        FishDelete(QString::fromStdString(name));
    }

    return found;
}
