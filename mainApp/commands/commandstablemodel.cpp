/*
 * Copyright 2013-2014 Christian Loose <christian.loose@hamburg.de>
 * Copyright (C) 2022 - 2023 Trevor SANDY. All rights reserved.
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
#include "commandstablemodel.h"

#include <QIcon>
#include <QMessageBox>

#include "commandcollection.h"
#include "application.h"

CommandsTableModel::CommandsTableModel(CommandCollection *collection, QObject *parent) :
    QAbstractTableModel(parent),
    commandCollection(collection)
{
}

int CommandsTableModel::rowCount(const QModelIndex &) const
{
    return commandCollection->count();
}

int CommandsTableModel::columnCount(const QModelIndex &) const
{
    return 1;
}

Qt::ItemFlags CommandsTableModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags itemFlags = QAbstractTableModel::flags(index);
    // set all items read-only
    if (index.isValid())
        itemFlags & ~Qt::ItemIsEditable;
    return itemFlags;
}

QVariant CommandsTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    const Command command = commandCollection->at(index.row());

    QString commandIcon = QStringLiteral(":/resources/command16.png");
    if (Application::instance()->getTheme() == THEME_DARK)
        commandIcon = QStringLiteral(":/resources/command_dark16.png");

    switch (role) {
        case Qt::DecorationRole:
            return QIcon(commandIcon);

        case Qt::DisplayRole:
            return command.command;

        case Qt::EditRole:
            return command.description;

        case Qt::ToolTipRole:
            return command.preamble.toHtmlEscaped();
    }

    return QVariant();
}

bool CommandsTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    Command command = commandCollection->at(index.row());

    command.command = value.toString();

    replaceCommand(command, index);
    return true;
}

QVariant CommandsTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section)

    if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
        return QVariant();

    return tr("Command");
}

QModelIndex CommandsTableModel::createCommand()
{
    Command command;
    beginInsertRows(QModelIndex(), 0, 0);
    int row = commandCollection->insert(command);
    endInsertRows();

    return index(row, 0);
}

void CommandsTableModel::removeCommand(const QModelIndex &index)
{
    beginRemoveRows(QModelIndex(), index.row(), index.row());
    Command command = commandCollection->at(index.row());
    commandCollection->remove(command);
    endRemoveRows();
}

void CommandsTableModel::replaceCommand(const Command &command, const QModelIndex &index)
{
    const int row = index.row();

    Command previousCommand = commandCollection->at(index.row());
    commandCollection->remove(previousCommand);

    int insertedRow = commandCollection->insert(command);

    if (index.row() == insertedRow) {
        emit dataChanged(index.sibling(row, 0), index);
    } else {
        if (row < insertedRow)
            beginMoveRows(QModelIndex(), row, row, QModelIndex(), insertedRow+1);
        else
            beginMoveRows(QModelIndex(), row, row, QModelIndex(), insertedRow);
        endMoveRows();
    }
}


