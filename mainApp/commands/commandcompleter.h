/*
 * Copyright 2013-2014 Christian Loose <christian.loose@hamburg.de>
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
#ifndef COMMANDCOMPLETER_H
#define COMMANDCOMPLETER_H

#include <QObject>

class QCompleter;
class CommandCollection;


class CommandCompleter : public QObject
{
    Q_OBJECT
public:
    explicit CommandCompleter(CommandCollection *collection, QWidget *parentWidget);

    void performCompletion(const QString &text, const QRect &popupRect);

    bool isPopupVisible() const;
    void hidePopup();

private:
    CommandCollection *commandCollection;
    QCompleter *completer;
};

#endif // COMMANDCOMPLETER_H
