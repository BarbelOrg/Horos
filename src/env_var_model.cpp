#include "env_var_model.hpp"
#include <algorithm>

EnvVarModel::EnvVarModel(QObject* parent)
    : QAbstractListModel(parent)
    , manager(EnvVarManager::LoadForCurrentShell())
{
    rebuildOrder();
}

void EnvVarModel::rebuildOrder()
{
    orderedNames.clear();
    for (const auto& [name, value] : manager.Vars())
        orderedNames.push_back(QString::fromStdString(name));
    std::sort(orderedNames.begin(), orderedNames.end());
}

int EnvVarModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return orderedNames.size();
}

QVariant EnvVarModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= orderedNames.size())
        return {};

    const QString& name = orderedNames[index.row()];

    switch (role) {
    case NameRole:
        return name;
    case ValueRole: {
        auto it = manager.Vars().find(name.toStdString());
        if (it == manager.Vars().end())
            return {};
        return QString::fromStdString(it->second);
    }
    default:
        return {};
    }
}

QHash<int, QByteArray> EnvVarModel::roleNames() const
{
    return {
        { NameRole, "name" },
        { ValueRole, "value" }
    };
}

void EnvVarModel::refresh()
{
    beginResetModel();
    manager = EnvVarManager::LoadForCurrentShell();
    rebuildOrder();
    endResetModel();
}

bool EnvVarModel::addVariable(const QString& name, const QString& value)
{
    if (!manager.IsValidKey(name.toStdString()))
        return false;

    const bool isNewRow = manager.Vars().find(name.toStdString()) == manager.Vars().end();

    if (isNewRow) {
        // figure out sorted insertion point so the view stays sorted
        int row = std::lower_bound(orderedNames.begin(), orderedNames.end(), name) - orderedNames.begin();
        beginInsertRows(QModelIndex(), row, row);
        manager.Set(name.toStdString(), value.toStdString());
        orderedNames.insert(row, name);
        endInsertRows();
    } else {
        manager.Set(name.toStdString(), value.toStdString());
        int row = orderedNames.indexOf(name);
        if (row >= 0)
            emit dataChanged(index(row), index(row), { ValueRole });
    }

    manager.Save();
    return true;
}

bool EnvVarModel::setValue(int row, const QString& value)
{
    if (row < 0 || row >= orderedNames.size())
        return false;

    const std::string name = orderedNames[row].toStdString();
    if (!manager.Set(name, value.toStdString()))
        return false;

    manager.Save();
    emit dataChanged(index(row), index(row), { ValueRole });
    return true;
}

bool EnvVarModel::removeVariable(int row)
{
    if (row < 0 || row >= orderedNames.size())
        return false;

    const std::string name = orderedNames[row].toStdString();

    beginRemoveRows(QModelIndex(), row, row);
    manager.Remove(name);
    orderedNames.remove(row);
    endRemoveRows();

    manager.Save();
    return true;
}

bool EnvVarModel::renameVariable(int row, const QString &newName)
{
    if (row < 0 || row >= orderedNames.size())
        return false;

    const QString oldName = orderedNames[row];
    if (oldName == newName)
        return true;
    if (!manager.IsValidKey(newName.toStdString()))
        return false;
    if (manager.Vars().contains(newName.toStdString()))
        return false; // don't clobber an existing var

    beginResetModel();
    bool ok = manager.Rename(oldName.toStdString(), newName.toStdString());
    if (ok)
        rebuildOrder();
    endResetModel();

    if (ok)
        manager.Save();
    return ok;
}

bool EnvVarModel::addPathLike(const QString& name, const QString& value)
{
    const bool isNewRow = !manager.Vars().contains(name.toStdString());

    if (isNewRow) {
        int row = std::lower_bound(orderedNames.begin(), orderedNames.end(), name) - orderedNames.begin();
        beginInsertRows(QModelIndex(), row, row);
        bool ok = manager.AddPathLike(name.toStdString(), value.toStdString());
        if (ok)
            orderedNames.insert(row, name);
        endInsertRows();
        if (!ok)
            return false;
    } else {
        if (!manager.AddPathLike(name.toStdString(), value.toStdString()))
            return false;
        int row = orderedNames.indexOf(name);
        if (row >= 0)
            emit dataChanged(index(row), index(row), { ValueRole });
    }

    manager.Save();
    return true;
}

bool EnvVarModel::removePathLike(const QString& name, const QString& entry)
{
    if (!manager.Vars().contains(name.toStdString()))
        return false;

    if (!manager.RemovePathLike(name.toStdString(), entry.toStdString()))
        return false;

    int row = orderedNames.indexOf(name);
    if (row >= 0)
        emit dataChanged(index(row), index(row), { ValueRole });

    manager.Save();
    return true;
}

bool EnvVarModel::isPathLike(int row) const
{
    if (row < 0 || row >= orderedNames.size())
        return false;
    auto it = manager.Vars().find(orderedNames[row].toStdString());
    if (it == manager.Vars().end())
        return false;
    return manager.IsPathLike(it->second);
}

void EnvVarModel::clear()
{
    beginResetModel();
    manager.Clear();
    rebuildOrder();
    endResetModel();
    manager.Save();
}