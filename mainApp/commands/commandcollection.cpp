/*
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
#include "commandcollection.h"

#include <QSharedPointer>


CommandCollection::CommandCollection(QObject *parent) :
    QObject(parent)
{
}

int CommandCollection::count() const
{
    return commands.count();
}

int CommandCollection::insert(const Command &command)
{
    QMap<QString, Command>::iterator it = commands.insert(command.preamble, command);
    emit collectionChanged(CommandCollection::ItemAdded, command);
    return std::distance(commands.begin(), it);
}

void CommandCollection::update(const Command &command)
{
    commands.insert(command.preamble, command);
    emit collectionChanged(CommandCollection::ItemChanged, command);
}

void CommandCollection::remove(const Command &command)
{
    commands.remove(command.preamble);
    emit collectionChanged(CommandCollection::ItemDeleted, command);
}

const QString CommandCollection::name() const
{
    return QStringLiteral("commands");
}

bool CommandCollection::contains(const QString &preamble) const
{
    return commands.contains(preamble);
}

const Command CommandCollection::command(const QString &preamble) const
{
    return commands.value(preamble);
}

const Command &CommandCollection::at(int offset) const
{
    return (commands.begin() + offset).value();
}

QStringList CommandCollection::preambleList() const
{
   QStringList preambleList;
   foreach (const Command &command, commands.values()) {
       preambleList << command.preamble;
   }
   return preambleList;
}

QSharedPointer<CommandCollection> CommandCollection::userModifiedCommands() const
{
    QSharedPointer<CommandCollection> userModifiedCommands = QSharedPointer<CommandCollection>::create();

    foreach (Command command, commands.values()) {
        if (command.modified == Command::True) {
            userModifiedCommands->insert(command);
        }
    }

    return userModifiedCommands;
}
