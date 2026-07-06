#pragma once
#include "env_var_manager.hpp"
#include <QAbstractListModel>
#include <QString>
#include <QVector>
#include <QtQmlIntegration/qqmlintegration.h>

class EnvVarModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        ValueRole
    };

    explicit EnvVarModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void refresh();

    Q_INVOKABLE bool addVariable(const QString& name, const QString& value);
    Q_INVOKABLE bool setValue(int row, const QString& value);
    Q_INVOKABLE bool removeVariable(int row);

    Q_INVOKABLE bool renameVariable(int row, const QString& newName);
    Q_INVOKABLE bool addPathLike(const QString& name, const QString& value);
    Q_INVOKABLE bool removePathLike(const QString& name, const QString& entry);
    Q_INVOKABLE bool isPathLike(int row) const;
    Q_INVOKABLE void clear();

private:
    EnvVarManager manager;
    QVector<QString> orderedNames;

    void rebuildOrder();
};