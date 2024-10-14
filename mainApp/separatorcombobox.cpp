/****************************************************************************
**
** Copyright (C) 2020 - 2024 Trevor SANDY. All rights reserved.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/****************************************************************************
 *
 * The editwindow is used to display the LDraw file to the user.  The Gui
 * portion of the program (see lpub.h) decides what files and line numbers
 * are displayed.  The edit window has as little responsibility as is
 * possible.  It does work the syntax highlighter implemented in
 * highlighter.(h,cpp)
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#include <QPainter>
#include <QStandardItem>
#include <QStandardItemModel>
#include "separatorcombobox.h"

QSize ComboBoxDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // increase the height of the separator to 5 pixels
    QString type = index.data( Qt::AccessibleDescriptionRole ).toString();
    if ( type == QLatin1String( "separator" ) )
        return QSize( 5, 5 );
    return QItemDelegate::sizeHint( option, index );
}

void ComboBoxDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // override the paint method in order to draw a dark, horizontal line:
    QString type = index.data( Qt::AccessibleDescriptionRole ).toString();
    if ( type == QLatin1String( "separator" ) ) {
        QItemDelegate::paint( painter, option, index );
        int y = ( option.rect.top() + option.rect.bottom() ) / 2;
        painter->setPen(  option.palette.color( QPalette::Active, QPalette::Dark ) );
        painter->drawLine( option.rect.left(), y, option.rect.right(), y );
    } else
    // Paint the parent item as if it was enabled, otherwise it would be grayed. We also adjust the rectangle
    // of the child item by the width of four spaces to make the indent.

    // The width of "text " may sometimes be slightly smaller than the sum of widths of " " and "text" because of font kerning.
    // The longest item may sometimes appear truncated (with "..." at the end). To prevent this, we disable the elide mode.
    if ( type == QLatin1String( "parent" ) ) {
        QStyleOptionViewItem parentOption = option;
        parentOption.state |= QStyle::State_Enabled;
        QItemDelegate::paint( painter, parentOption, index );
    } else if ( type == QLatin1String( "child" ) ) {
        QStyleOptionViewItem childOption = option;
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
        int indent = option.fontMetrics.horizontalAdvance( QString( 4, QChar( ' ' ) ) );
#else
        int indent = option.fontMetrics.width( QString( 4, QChar( ' ' ) ) );
#endif
        childOption.rect.adjust( indent, 0, 0, 0 );
        childOption.textElideMode = Qt::ElideNone;
        QItemDelegate::paint( painter, childOption, index );
    } else {
        QItemDelegate::paint( painter, option, index );
    }
}

SeparatorComboBox::SeparatorComboBox(QWidget *parent)
    : QComboBox(parent)
{
    comboBoxDelegate = new ComboBoxDelegate(this);
    setItemDelegate(comboBoxDelegate);
}

void SeparatorComboBox::addSeparator()
{
    insertSeparator( count() );
}

void SeparatorComboBox::addParentItem( const QString& text ) const
{
    QStandardItem* item = new QStandardItem( text );
    item->setFlags( item->flags() & ~( Qt::ItemIsEnabled | Qt::ItemIsSelectable ) );
    item->setData( "parent", Qt::AccessibleDescriptionRole );

    QFont font = item->font();
    font.setBold( true );
    item->setFont( font );

    QStandardItemModel* itemModel = (QStandardItemModel*)model();
    itemModel->appendRow( item );
}

void SeparatorComboBox::addChildItem( const QString& text, const QVariant& data ) const
{
    QStandardItem* item = new QStandardItem( text + QString( 4, QChar( ' ' ) ) );
    item->setData( data, Qt::UserRole );
    item->setData( "child", Qt::AccessibleDescriptionRole );

    QStandardItemModel* itemModel = (QStandardItemModel*)model();
    itemModel->appendRow( item );
}

