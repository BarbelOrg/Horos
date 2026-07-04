#pragma once
#include <optional>

enum class Shell
{
    Bash,
    Zsh,
    Fish,
    Windows
};

std::optional<Shell> GetShell();
