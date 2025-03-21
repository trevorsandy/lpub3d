/*
 * Copyright 2014 Christian Loose <christian.loose@hamburg.de>
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
#ifndef JSONCOMMANDTRANSLATORFACTORY_H
#define JSONCOMMANDTRANSLATORFACTORY_H

#include <jsontranslatorfactory.h>
#include <jsontranslator.h>

#include "commands/command.h"
#include "commands/jsoncommandtranslator.h"


template <> class JsonTranslatorFactory<Command>
{
public:
    static JsonTranslator<Command> *create()
    {
        return new JsonCommandTranslator();
    }
};

#endif // JSONCOMMANDTRANSLATORFACTORY_H

