#include "unix_utils.hpp"

std::optional<std::filesystem::path> GetHomePath()
{
    if (auto home = std::getenv("HOME"))
    {
        return home;
    }
    else
    {
        return std::nullopt;
    }
}

std::filesystem::path GetConfigPath()
{
    auto path = GetHomePath().value() / ".config";
    std::filesystem::create_directories(path);
    return path;
}