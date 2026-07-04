#pragma once
#include "env_var.hpp"

#include <filesystem>

std::vector<EnvVar> FishGetAll();

void FishDelete(const QString &name);
void FishSet(const QString &name, const QString &value);

// TODO
// static std::optional<QStringList> GetPathlike(const QString& name);
// static bool AddToPathLike(const QString& name, const QString& value);
// static bool RemoveFromPathLike(const QString& name);
