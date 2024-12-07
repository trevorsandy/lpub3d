/****************************************************************************
**
** Copyright (C) 2022 - 2024 Trevor SANDY. All rights reserved.
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
 * This file describes a dialog that users can use to inspect LPUB commands,
 * update command description details, and manage the command snippet system.  
 *
 * Please see lpub.h for an overall description of how the files in LPub3D
 * make up the LPub3D program.
 *
 ***************************************************************************/

#ifndef COMMANDSDIALOG_H
#define COMMANDSDIALOG_H

#include <QDialog>

class Meta;
class QLabel;
class QTabWidget;
class QTableView;
class QListView;
class QPlainTextEdit;
class QSortFilterProxyModel;
class SnippetCollection;
class SnippetsTableModel;
class CommandCollection;
class CommandsTableModel;
class CommandsTextEdit;
class FilterLineEdit;
class CommandsDialog : public QDialog
{
  Q_OBJECT
public:
    
  CommandsDialog(QWidget *parent);
  virtual ~CommandsDialog() {}

  static void showCommandsDialog(QWidget *parent = nullptr);

  static void setCommandEdit(bool b);

  static void showCommandEditButtons(bool b);

  static void enableCommandEditButtons(bool b);

private:
  void readSettings();
  void writeSettings();
  QPalette readOnlyPalette;

  QTabWidget  *tabWidget;

  FilterLineEdit        *commandFilterEdit;
  CommandsTableModel    *commandsTableModel;
  QSortFilterProxyModel *commandsProxyTableModel;

  CommandCollection *commandCollection;

  QTableView       *commandTableView;
  CommandsTextEdit *commandTextEdit;
  QPushButton      *editCommandButton;
  QPushButton      *resetCommandButton;

  FilterLineEdit        *snippetFilterEdit;
  SnippetsTableModel    *snippetsTableModel;
  QSortFilterProxyModel *snippetsProxyTableModel;

  SnippetCollection *snippetCollection;

  QTableView       *snippetTableView;
  CommandsTextEdit *snippetTextEdit;
  QPushButton      *addSnippetButton;
  QPushButton      *removeSnippetButton;

  QPushButton      *insertCommandBeforeButton;
  QPushButton      *insertCommandAfterButton;
  QPushButton      *moveLineUpButton;
  QPushButton      *moveLineDownButton;
  QLabel           *commandUsageLabel;
  QLabel           *snippetUsageLabel;

  bool              commandEdit;

signals:
  void commandSnippet(const QString &, bool);
  void moveLineUp();
  void moveLineDown();

private slots:
  void customMenuRequested(QPoint pos);
  void currentCommandChanged(const QModelIndex &current, const QModelIndex &previous);
  void commandFilterEditChanged();
  void commandTextChanged();
  void resetCommandButtonClicked();
  void editCommandButtonClicked(bool clicked);

  void currentSnippetChanged(const QModelIndex &current, const QModelIndex &previous);
  void snippetFilterEditChanged();
  void snippetTextChanged();
  void addSnippetButtonClicked();
  void removeSnippetButtonClicked();
  void copyToClipboard();
  void closeEvent(QCloseEvent *event) override;
  bool maybeSave();
  void applyCommandEdit();
  void moveLine();

public slots:
  void accept() override;
  void cancel();
};
#endif // COMMANDSDIALOG_H
