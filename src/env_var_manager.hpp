#pragma once
#include "shell.hpp"

#include <string>
#include <vector>
#include <unordered_map>

class EnvVarManager
{
    Shell shell;
    std::unordered_map<std::string, std::string> vars;
public:
    static EnvVarManager LoadForCurrentShell();
    static EnvVarManager Load(Shell shell);
    void Save();

    bool Set(const std::string& name, const std::string& value);
    bool Remove(const std::string& name);

    bool IsValidKey(std::string_view key);

    const auto& Vars() const { return vars; }
};
