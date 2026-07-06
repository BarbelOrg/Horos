#pragma once
#include "shell.hpp"
#include <QProcessEnvironment>

#include <string>
#include <vector>
#include <unordered_map>

class EnvVarManager
{
    Shell shell;
    std::unordered_map<std::string, std::string> vars;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
public:
    static EnvVarManager LoadForCurrentShell();
    static EnvVarManager Load(Shell shell);
    void Save();

    bool Set(const std::string& name, const std::string& value);
    bool Remove(const std::string& name);
    bool Rename(const std::string& oldName, const std::string& newName);

    static bool IsPathLike(std::string_view str);
    bool AddPathLike(const std::string& name, const std::string& value);
    bool RemovePathLike(const std::string& name, const std::string& entry);

    void Clear();

    bool IsValidKey(std::string_view key);

    const auto& Vars() const { return vars; }
};
