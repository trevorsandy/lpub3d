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
#include "jsoncommandtranslator.h"

#include "command.h"

namespace {
static const QLatin1String PREAMBLE("preamble");
static const QLatin1String COMMAND("command");
static const QLatin1String DESCRIPTION("description");
static const QLatin1String MODIFIED("modified");
}


Command JsonCommandTranslator::fromJsonObject(const QJsonObject &object)
{
    Command command;

    command.preamble = object.value(PREAMBLE).toString();
    command.command = object.value(COMMAND).toString();
    command.description = object.value(DESCRIPTION).toString();
    command.modified = object.value(MODIFIED).toInt();

    return command;
}

QJsonObject JsonCommandTranslator::toJsonObject(const Command &command)
{
    QJsonObject object;

    object.insert(PREAMBLE, command.preamble);
    object.insert(COMMAND, command.command);
    object.insert(DESCRIPTION, command.description);
    object.insert(MODIFIED, command.modified);

    return object;
}
