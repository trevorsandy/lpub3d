/*
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
#ifndef COMMANDCOLLECTION_H
#define COMMANDCOLLECTION_H

#include <QObject>
#include <QMap>
#include <jsoncollection.h>
#include "command.h"


class CommandCollection : public QObject, public JsonCollection<Command>
{
    Q_OBJECT
    Q_ENUMS(CollectionChangedType)

public:
    enum CollectionChangedType
    {
        ItemAdded,
        ItemChanged,
        ItemDeleted
    };

    explicit CommandCollection(QObject *parent = 0);

    int count() const;

    int insert(const Command& command);
    void update(const Command& command);
    void remove(const Command& command);

    const QString name() const;
    bool contains(const QString &preamble) const;
    const Command command(const QString &preamble) const;
    const Command &at(int offset) const;

    QStringList preambleList() const;
    QSharedPointer<CommandCollection> userModifiedCommands() const;

signals:
    void collectionChanged(CommandCollection::CollectionChangedType changedType, const Command &command);

private:
    QMap<QString, Command> commands;
};

#endif // COMMANDCOLLECTION_H
