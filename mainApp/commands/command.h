/*
 * Copyright 2022 Trevor SANDY <trevor.sandy@gmail.com>
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
#ifndef COMMAND_H
#define COMMAND_H

#include <QString>


struct Command
{
    enum Status
    {
        False,
        True,
        Pending
    };

    QString preamble;
    QString command;    
    QString description;
    int modified;

    Command(): modified(false)  {}

    bool operator<(const Command &rhs) const
    {
        return preamble < rhs.preamble;
    }

    bool operator ==(const Command &rhs) const
    {
        return preamble == rhs.preamble;
    }
};

#endif // COMMAND_H
