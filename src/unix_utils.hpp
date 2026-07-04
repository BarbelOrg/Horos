#pragma once
#include <filesystem>

std::optional<std::filesystem::path> GetHomePath();
std::filesystem::path GetConfigPath();
