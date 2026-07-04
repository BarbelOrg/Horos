#pragma once
#include "env_var.hpp"

#include "shell.hpp"
#include <QList>
#include <QString>
#include <optional>
#include <filesystem>

QList<EnvVar> BashLikeGetAll(Shell shell);

bool BashLikeSet(const QString &name, const QString &value, Shell shell);
bool BashLikeDelete(const QString &name, Shell shell);

std::optional<std::filesystem::path> GetRcPathForShell(Shell shell);
std::optional<std::filesystem::path> GetEnvShPathForShell(Shell shell);

bool EnvIsSourced(Shell shell);
void EnsureEnvIsSourced(Shell shell);