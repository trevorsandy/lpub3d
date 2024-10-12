/*
 * Copyright 2013-2014 Christian Loose <christian.loose@hamburg.de>
 * Copyright (C) 2022 - 2024 Trevor SANDY. All rights reserved.
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
#include "commandcompleter.h"

#include <QApplication>
#include <QAbstractItemView>
#include <QCompleter>
#include <QClipboard>
#include <QScrollBar>

#include "command.h"
#include "commandcollection.h"
#include "commandlistmodel.h"


CommandCompleter::CommandCompleter(CommandCollection *collection, QWidget *parentWidget) :
    QObject(parentWidget),
    commandCollection(collection),
    completer(new QCompleter(this))
{
    completer->setWidget(parentWidget);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setCaseSensitivity(Qt::CaseSensitive);

    CommandListModel *model = new CommandListModel(completer);
    connect(collection, SIGNAL(collectionChanged(CommandCollection::CollectionChangedType,Command)),
            model,      SLOT(commandCollectionChanged(CommandCollection::CollectionChangedType,Command)));
    completer->setModel(model);
}

void CommandCompleter::performCompletion(const QString &text, const QRect &popupRect)
{
    const QString completionPrefix = text;

    if (completionPrefix != completer->completionPrefix()) {
        completer->setCompletionPrefix(completionPrefix);
        completer->popup()->setCurrentIndex(completer->completionModel()->index(0, 0));
    }

    QRect rect = popupRect;
    rect.setWidth(completer->popup()->sizeHintForColumn(0) +
            completer->popup()->verticalScrollBar()->sizeHint().width());
    completer->complete(rect);
}

bool CommandCompleter::isPopupVisible() const
{
    return completer->popup()->isVisible();
}

void CommandCompleter::hidePopup()
{
    completer->popup()->hide();
}
