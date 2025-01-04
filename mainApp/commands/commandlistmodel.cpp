/*
 * Copyright 2013-2014 Christian Loose <christian.loose@hamburg.de>
 * Copyright (C) 2022 - 2025 Trevor SANDY. All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "commandlistmodel.h"

#include <QFont>
#include <QIcon>

CommandListModel::CommandListModel(QObject *parent) :
    QAbstractListModel(parent)
{
}

int CommandListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return commands.count();
}

QVariant CommandListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() > rowCount())
        return QVariant();

    if (index.row() < commands.count()) {
        const Command command = commands.at(index.row());

        switch (role) {
            case Qt::DecorationRole:
                return QIcon(":/resources/commandcommand.png");

            case Qt::DisplayRole:
                return command.description;

            case Qt::EditRole:
                return command.description;

            case Qt::ToolTipRole:
                return command.command.toHtmlEscaped();
        }
    }

    return QVariant();
}

void CommandListModel::commandCollectionChanged(CommandCollection::CollectionChangedType changedType, const Command &command)
{
    switch (changedType) {
    case CommandCollection::ItemAdded:
        {
            QList<Command>::iterator it = std::lower_bound(commands.begin(), commands.end(), command);
            int row = std::distance(commands.begin(), it);
            beginInsertRows(QModelIndex(), row, row);
            commands.insert(it, command);
            endInsertRows();
        }
        break;
    case CommandCollection::ItemChanged:
        {
            int row = commands.indexOf(command);
            commands.replace(row, command);
        }
        break;
    case CommandCollection::ItemDeleted:
        {
            int row = commands.indexOf(command);
            beginRemoveRows(QModelIndex(), row, row);
            commands.removeAt(row);
            endRemoveRows();
        }
        break;
    default:
        break;
    }
}
