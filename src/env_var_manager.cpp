#include "env_var_manager.hpp"

#include "bash_like.hpp"
#include "fish.hpp"
#include "shell.hpp"
#include "string_utils.hpp"
#include "unix_utils.hpp"

#include <QDebug>
#include <ctre.hpp>
#include <fast_io.h>
#include <fast_io_device.h>
#include <flux.hpp>
#include <flux/core/assert.hpp>
#include <fmt/format.h>
#include <glaze/json/write.hpp>
#include <glaze/util/glaze_fast_float.hpp>

#ifdef __linux
constexpr auto kSeperator = ':';
#else
constexpr auto kSeperator = ';';
#endif

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
            auto obuf = fast_io::obuf_file(GetEnvShPathForShell(shell).value(),
                                           fast_io::open_mode::out | fast_io::open_mode::trunc);
            for (const auto &[key, value] : vars)
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
        auto ec = glz::write_file_json<glz::opts{.prettify = true}>(
            vars, (GetConfigPath() / "horos" / filename).c_str(), std::string());
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

bool EnvVarManager::AddPathLike(const std::string &name, const std::string &value)
{
    if (!vars.contains(name))
    {
        auto base = std::string();
        if (auto envVal = std::getenv(name.c_str()))
            base = envVal;
        vars[name] = base;
    }

    if (shell == Shell::Fish)
        throw std::runtime_error("AddPathLike: fish not implemented");

    auto &current = vars[name];
    auto valueStr = current.empty() ? value : fmt::format("{}{}{}", current, kSeperator, value);

    return Set(name, valueStr);
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

bool EnvVarManager::Rename(const std::string &oldName, const std::string &newName)
{
    if (oldName == newName)
        return true;
    if (!vars.contains(oldName))
        return false;
    if (vars.contains(newName))
        return false; // don't silently clobber an existing var
    if (!IsValidKey(newName))
    {
        qCritical() << "Invalid env var key: " << QString::fromUtf8(newName.data(), newName.size());
        return false;
    }

    auto value = vars[oldName];
    Remove(oldName);
    Set(newName, value);
    return true;
}

bool EnvVarManager::IsPathLike(std::string_view str)
{
    return str.contains(kSeperator);
}

bool EnvVarManager::RemovePathLike(const std::string &name, const std::string &entry)
{
    // TODO have it works with overrides
    if (!vars.contains(name))
        return false;

    auto joined = flux::split_string(flux::ref(vars[name]), kSeperator)
                      .map([](auto &&r) { return std::string_view(r); })
                      .filter([&entry](auto &&r) { return r != entry; })
                      .fold(
                          [](std::string acc, std::string_view piece) {
                              if (!acc.empty())
                                  acc += kSeperator;
                              acc += piece;
                              return acc;
                          },
                          std::string());

    return Set(name, joined);
}

void EnvVarManager::Clear()
{
    vars.clear();
}
