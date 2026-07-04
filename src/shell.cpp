#include "shell.hpp"

#include <cstdlib>
#include <pwd.h>
#include <string_view>
#include <unistd.h>

static std::optional<Shell> ParseShellPath(std::string_view path)
{
    auto pos = path.find_last_of('/');
    auto name = (pos == std::string_view::npos) ? path : path.substr(pos + 1);

    if (name == "bash")
        return Shell::Bash;
    if (name == "zsh")
        return Shell::Zsh;
    if (name == "fish")
        return Shell::Fish;

    return std::nullopt;
}

std::optional<Shell> GetShell()
{
#ifdef _WIN32
    return Shell::Windows
#else
    // 1st we try via the SHELL env var (most if not all systems)
    if (auto envShell = std::getenv("SHELL"))
    {
        if (auto shell = ParseShellPath(envShell))
        {
            return shell;
        }
    }

    // Else we fall back on /etc/passwd (edge cases)
    if (auto pwd = getpwuid(getuid()))
    {
        if (pwd->pw_shell)
        {
            if (auto shell = ParseShellPath(pwd->pw_shell))
            {
                return shell;
            }
        }
    }

    return std::nullopt;
#endif
}