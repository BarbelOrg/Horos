#pragma once
#include <QString>

enum class Origin
{
    Managed,
    Shell,
    Unknown
};

struct EnvVar
{
    QString name;
    QString value;
    bool managed;
};