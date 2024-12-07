
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
 * This file describes a dialog that users can use to inspect LPub3D commands,
 * update command description details, and manage the command snippet system.  
 *
 * Please see lpub.h for an overall description of how the files in LPub3D
 * make up the LPub3D program.
 *
 ***************************************************************************/

#include <QDesktopWidget>
#include <QTabWidget>
#include <QListView>
#include <QTableView>
#include <QHeaderView>
#include <QPlainTextEdit>
#include <QStringListModel>
#include <QPushButton>
#include <QDir>
#include <QThread>
#include <QAction>
#include <QMenu>
#include <QLabel>
#include <QSettings>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QClipboard>
#include <QDialogButtonBox>
#include <QSortFilterProxyModel>

#include "commandsdialog.h"
#include "version.h"
#include "meta.h"
#include "lpub_object.h"
#include "commonmenus.h"

#include <jsonfile.h>
#include <commands/jsoncommandtranslatorfactory.h>
#include <commands/commandcollection.h>
#include <commands/commandstablemodel.h>
#include <commands/commandstextedit.h>
#include <commands/filterlineedit.h>
#include <commands/snippets/jsonsnippettranslatorfactory.h>
#include <commands/snippets/snippetcollection.h>
#include <commands/snippets/snippetstablemodel.h>

#include "messageboxresizable.h"

CommandsDialog::CommandsDialog(QWidget *parent) :
    QDialog(parent),
    commandEdit(false)
{
  setWindowTitle(tr ("%1 Meta Commands").arg(VER_PRODUCTNAME_STR));

  setWhatsThis(lpubWT(WT_DIALOG_META_COMMANDS, windowTitle()));

  QVBoxLayout *layout = new QVBoxLayout();
  setLayout(layout);

  tabWidget = new QTabWidget(this);
  layout->addWidget(tabWidget);
  
  QWidget     *widget;
  QGridLayout *widgetLayout;

  readOnlyPalette = QApplication::palette();
  if (Preferences::darkTheme)
      readOnlyPalette.setColor(QPalette::Base,QColor(Preferences::themeColors[THEME_DARK_PALETTE_MIDLIGHT]));
  else
      readOnlyPalette.setColor(QPalette::Base,QColor(Preferences::themeColors[THEME_DEFAULT_PALETTE_LIGHT]).lighter(140));

  /*
   * Commands Tab
   */

  widget  = new QWidget(this);
  widget->setObjectName(tr("Meta Commands"));
  widgetLayout = new QGridLayout(widget);
  widget->setLayout(widgetLayout);

  commandCollection = lpub->commandCollection;

  commandFilterEdit = new FilterLineEdit(commandCollection, widget);
  commandFilterEdit->setWhatsThis(lpubWT(WT_CONTROL_COMMANDS_FILTER, tr("Filter Meta Commands")));
  widgetLayout->addWidget(commandFilterEdit,0,0);
  connect(commandFilterEdit, &FilterLineEdit::filterChanged,
          this,              &CommandsDialog::commandFilterEditChanged);

  commandTableView = new QTableView(widget);
  commandTableView->setWhatsThis(lpubWT(WT_CONTROL_COMMANDS_VIEW, tr("View Meta Commands")));
  commandTableView->setToolTip(tr("Select command"));
  commandTableView->setSelectionMode(QAbstractItemView::SingleSelection);
  commandTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
  commandTableView->setCornerButtonEnabled(false);
  commandTableView->setShowGrid(false);
  commandTableView->setWordWrap(false);
  QHeaderView *commandHorizontalHeader;
  commandHorizontalHeader = commandTableView->horizontalHeader();
  commandHorizontalHeader->setHighlightSections(false);
  commandHorizontalHeader->setStretchLastSection(true);
  QHeaderView *commandVerticalHeader;
  commandVerticalHeader = commandTableView->verticalHeader();
  commandVerticalHeader->setVisible(false);
  commandVerticalHeader->setDefaultSectionSize(22);
  commandsProxyTableModel = new QSortFilterProxyModel(widget);
  commandsTableModel = new CommandsTableModel(commandCollection, commandTableView);
  commandsProxyTableModel->setSourceModel(commandsTableModel);
  commandTableView->setModel(commandsProxyTableModel);
  commandTableView->setContextMenuPolicy (Qt::CustomContextMenu);
  widgetLayout->addWidget(commandTableView,1,0);
  connect(commandTableView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
          this,                               SLOT(   currentCommandChanged(QModelIndex,QModelIndex)));

  connect(commandTableView,                   SIGNAL(customContextMenuRequested(QPoint)),
          this,                               SLOT(  customMenuRequested(QPoint)));

  if (!lpub->commandTextEdit)
      commandTextEdit = new CommandsTextEdit(widget);
  else {
      commandTextEdit = lpub->commandTextEdit;
      commandTextEdit->setParent(widget);
  }
  commandTextEdit->setParent(widget);
  commandTextEdit->setToolTip(tr("Edit the command adding default value(s) or descriptive content."));
  commandTextEdit->setWhatsThis(lpubWT(WT_CONTROL_COMMANDS_TEXT_EDIT, tr("Edit And Insert Meta Commands")));
  commandTextEdit->setReadOnly(true);
  commandTextEdit->setPalette(readOnlyPalette);
  widgetLayout->addWidget(commandTextEdit,2,0);
  connect(commandTextEdit,     &CommandsTextEdit::textChanged,
          this,                &CommandsDialog::commandTextChanged);

  commandUsageLabel = new QLabel(tr("To insert a command in the command editor, select the command and click "
                                     "<b><i>Insert Before</i></b> or <b><i>Insert After</i></b>.<br>"
                                     "Additionally, you can use<b><i>Move Up</i></b> or <b><i>Move Down</i></b> "
                                     "position the command as desired."));
  commandUsageLabel->hide();
  commandUsageLabel->setWordWrap(true);
  widgetLayout->addWidget(commandUsageLabel,3,0,1,2);

  editCommandButton = new QPushButton(tr("Edit"),widget);
  editCommandButton->setWhatsThis(lpubWT(WT_CONTROL_COMMANDS_EDIT, tr("Edit Meta Command")));
  editCommandButton->setToolTip(tr("Enable the text editor to make the command editable"));
  editCommandButton->setCheckable(true);
  widgetLayout->addWidget(editCommandButton,0,1);
  connect(editCommandButton, &QPushButton::clicked,
          this,              &CommandsDialog::editCommandButtonClicked);

  resetCommandButton = new QPushButton(tr("Reset"),widget);
  resetCommandButton->setWhatsThis(lpubWT(WT_CONTROL_COMMANDS_RESET, tr("Reset Meta Command")));
  resetCommandButton->setEnabled(false);
  resetCommandButton->setToolTip(tr("Reset the command command in the text editor to its initial state."));
  widgetLayout->addWidget(resetCommandButton,1,1,1,1,Qt::AlignTop);
  connect(resetCommandButton,  &QPushButton::clicked,
          this,                &CommandsDialog::resetCommandButtonClicked);

  QVBoxLayout *lineEditLayout = new QVBoxLayout(widget);
  widgetLayout->addLayout(lineEditLayout,2,1);

  insertCommandBeforeButton = new QPushButton(tr("Insert Before"), widget);
  insertCommandBeforeButton->setWhatsThis(lpubWT(WT_CONTROL_COMMANDS_INSERT_BEFORE, tr("Insert Before Current Line")));
  insertCommandBeforeButton->setToolTip(tr("Insert your command before the command editor cursor position."));
  insertCommandBeforeButton->setEnabled(false);
  insertCommandBeforeButton->setVisible(false);
  lineEditLayout->addWidget(insertCommandBeforeButton,0,Qt::AlignTop);
  connect(insertCommandBeforeButton, &QPushButton::clicked,
          this,                &CommandsDialog::applyCommandEdit);

  insertCommandAfterButton = new QPushButton(tr("Insert After"), widget);
  insertCommandAfterButton->setWhatsThis(lpubWT(WT_CONTROL_COMMANDS_INSERT_AFTER, tr("Insert After Current Line")));
  insertCommandAfterButton->setToolTip(tr("Insert your command after the command editor cursor position."));
  insertCommandAfterButton->setEnabled(false);
  insertCommandAfterButton->setVisible(false);
  lineEditLayout->addWidget(insertCommandAfterButton,0,Qt::AlignTop);
  connect(insertCommandAfterButton, &QPushButton::clicked,
          this,                     &CommandsDialog::applyCommandEdit);

  moveLineUpButton = new QPushButton(tr("Move Up"), widget);
  moveLineUpButton->setWhatsThis(lpubWT(WT_CONTROL_COMMANDS_MOVE_LINE_UP, tr("Move Current Line Up")));
  moveLineUpButton->setToolTip(tr("Move the line at the command editor cursor position up by one line."));
  moveLineUpButton->setEnabled(false);
  moveLineUpButton->setVisible(false);
  lineEditLayout->addWidget(moveLineUpButton,0,Qt::AlignTop);
  connect(moveLineUpButton, &QPushButton::clicked,
          this,             &CommandsDialog::moveLine);

  moveLineDownButton = new QPushButton(tr("Move Down"), widget);
  moveLineDownButton->setWhatsThis(lpubWT(WT_CONTROL_COMMANDS_MOVE_LINE_DOWN, tr("Move Current Line Down")));
  moveLineDownButton->setToolTip(tr("Move the line at the command editor cursor position down by one line."));
  moveLineDownButton->setEnabled(false);
  moveLineDownButton->setVisible(false);
  lineEditLayout->addWidget(moveLineDownButton,0,Qt::AlignTop);
  connect(moveLineDownButton, &QPushButton::clicked,
          this,               &CommandsDialog::moveLine);

  lineEditLayout->addStretch(1);

  tabWidget->addTab(widget,widget->objectName());

  /*
   * Snippet Tab
   */

  widget  = new QWidget(this);
  widget->setObjectName(tr("Command Snippets"));
  widgetLayout = new QGridLayout(widget);
  widget->setLayout(widgetLayout);

  snippetCollection = lpub->snippetCollection;

  snippetFilterEdit = new FilterLineEdit(snippetCollection, widget);
  snippetFilterEdit->setWhatsThis(lpubWT(WT_CONTROL_COMMANDS_FILTER,tr("Filter Command Snippets")));
  widgetLayout->addWidget(snippetFilterEdit,0,0);
  connect(snippetFilterEdit, &FilterLineEdit::filterChanged,
          this,              &CommandsDialog::snippetFilterEditChanged);

  snippetTableView = new QTableView(widget);
  snippetTableView->setWhatsThis(lpubWT(WT_CONTROL_SNIPPETS_LIST,tr("List Command Snippets")));
  snippetTableView->setToolTip(tr("Select snippet"));
  snippetTableView->setSelectionMode(QAbstractItemView::SingleSelection);
  snippetTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
  snippetTableView->setCornerButtonEnabled(false);
  snippetTableView->setShowGrid(false);
  snippetTableView->setWordWrap(false);
  QHeaderView *snippetHorizontalHeader;
  snippetHorizontalHeader = snippetTableView->horizontalHeader();
  snippetHorizontalHeader->setHighlightSections(false);
  snippetHorizontalHeader->setStretchLastSection(true);
  QHeaderView *snippetVerticalHeader;
  snippetVerticalHeader = snippetTableView->verticalHeader();
  snippetVerticalHeader->setVisible(false);
  snippetVerticalHeader->setDefaultSectionSize(22);
  snippetsProxyTableModel = new QSortFilterProxyModel(widget);
  snippetsTableModel = new SnippetsTableModel(snippetCollection, snippetTableView);
  snippetsProxyTableModel->setSourceModel(snippetsTableModel);
  snippetTableView->setModel(snippetsProxyTableModel);
  snippetTableView->setContextMenuPolicy (Qt::CustomContextMenu);
  widgetLayout->addWidget(snippetTableView,1,0);
  connect(snippetTableView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
          this,                               SLOT(  currentSnippetChanged(QModelIndex,QModelIndex)));

  connect(snippetTableView,                   SIGNAL(customContextMenuRequested(QPoint)),
          this,                               SLOT(  customMenuRequested(QPoint)));

  if (!lpub->snippetTextEdit)
      snippetTextEdit = new CommandsTextEdit(widget);
  else {
      snippetTextEdit = lpub->snippetTextEdit;
      snippetTextEdit->setParent(widget);
  }
  snippetTextEdit->setToolTip(tr("Create shortcuts that trigger LPUB command insert or autocomplete.<br>"
                                 "Place the '$|' marker where the cursor will be after the command is inserted."));
  snippetTextEdit->setWhatsThis(lpubWT(WT_CONTROL_SNIPPETS_EDIT, tr("Edit Command Snippet")));
  snippetTextEdit->setReadOnly(true);
  snippetTextEdit->setPalette(readOnlyPalette);
  widgetLayout->addWidget(snippetTextEdit,2,0);
  connect(snippetTextEdit,     &CommandsTextEdit::textChanged,
          this,                &CommandsDialog::snippetTextChanged);

  snippetUsageLabel = new QLabel(tr("To apply a snippet in the command editor, type the snippet trigger - e.g. "
                                     "'1' for <i>0 STEP</i>, then <b><i>CTRL+Space</i></b> to trigger the snippet insert dialog.<br>"
                                     "When the desired snippet is highlighted, hit <b><i>ENTER</i></b> to insert the snippet."));
  snippetUsageLabel->hide();
  snippetUsageLabel->setWordWrap(true);
  widgetLayout->addWidget(snippetUsageLabel,3,0,1,2);

  addSnippetButton = new QPushButton(tr("Add"),widget);
  addSnippetButton->setWhatsThis(lpubWT(WT_CONTROL_SNIPPETS_ADD, tr("Add Command Snippet")));
  addSnippetButton->setToolTip(tr("Add a new command snippet"));
  widgetLayout->addWidget(addSnippetButton,0,1);
  connect(addSnippetButton,    &QPushButton::clicked,
          this,                &CommandsDialog::addSnippetButtonClicked);

  removeSnippetButton = new QPushButton(tr("Remove"),widget);
  removeSnippetButton->setWhatsThis(lpubWT(WT_CONTROL_SNIPPETS_REMOVE, tr("Remove Command Snippet")));
  removeSnippetButton->setToolTip(tr("Remove the selected command snippet"));
  widgetLayout->addWidget(removeSnippetButton,1,1,1,1,Qt::AlignTop);
  connect(removeSnippetButton, &QPushButton::clicked,
          this,                &CommandsDialog::removeSnippetButtonClicked);

  tabWidget->addTab(widget,widget->objectName());

  QDialogButtonBox *buttonBox = new QDialogButtonBox(this);

  buttonBox->addButton(QDialogButtonBox::Ok);
  connect(buttonBox,SIGNAL(accepted()),SLOT(accept()));
  buttonBox->addButton(QDialogButtonBox::Cancel);
  connect(buttonBox,SIGNAL(rejected()),SLOT(cancel()));

  layout->addWidget(buttonBox);

  setSizeGripEnabled(true);

  setMinimumSize(650,450);

  readSettings();
}

// Command items

void CommandsDialog::commandFilterEditChanged()
{
  QRegExp regExp(commandFilterEdit->text(),
                 commandFilterEdit->caseSensitivity(),
                 commandFilterEdit->patternSyntax());

  commandsProxyTableModel->setFilterRegExp(regExp);
}

void CommandsDialog::currentCommandChanged(const QModelIndex &current, const QModelIndex &)
{
  enableCommandEditButtons(true);
  resetCommandButton->setEnabled(false);
  commandTextEdit->setCommandText(commandsProxyTableModel->data(current, Qt::EditRole).toString());
  commandTextEdit->setReadOnly(true);
  commandTextEdit->setPalette(readOnlyPalette);
}

void CommandsDialog::commandTextChanged()
{
  if (commandTextEdit->isReadOnly())
      return;

  const QModelIndex &modelIndex = commandTableView->selectionModel()->currentIndex();
  if (modelIndex.isValid()) {
    const QString commandStr = commandsProxyTableModel->data(modelIndex, Qt::DisplayRole).toString();
    QString description = commandTextEdit->toPlainText();

    QRegExp rx("^([^\\[\\(<\\\n]*)");
    if (description != commandStr && commandStr.contains(rx)) {
      Command command = commandCollection->command(rx.cap(1).trimmed());
      command.description = commandTextEdit->toPlainText();
      command.modified = Command::Pending;
      commandCollection->update(command);
    }
  }
}

void CommandsDialog::resetCommandButtonClicked()
{
  const QModelIndex &modelIndex = commandTableView->selectionModel()->currentIndex();

  if (modelIndex.isValid()) {
    const QString commandStr = commandsProxyTableModel->data(modelIndex, Qt::DisplayRole).toString();
    const QString description = commandTextEdit->toPlainText();

    QRegExp rx("^([^\\[\\(<\\\n]*)");
    if (description != commandStr && commandStr.contains(rx)) {
      Command command = commandCollection->command(rx.cap(1).trimmed());
      command.description = command.command;
      command.modified = Command::Pending;
      commandCollection->update(command);

      commandTextEdit->setCommandText(command.description);

      commandTextEdit->setReadOnly(true);
      commandTextEdit->setPalette(readOnlyPalette);
      resetCommandButton->setEnabled(false);
    }
  }
}

void CommandsDialog::editCommandButtonClicked(bool clicked)
{
  if (clicked) {
    editCommandButton->setStyleSheet(QString("QPushButton { color : %1; }").arg(QColor(Qt::green).name()));
    commandTextEdit->setPalette(QApplication::palette());
    resetCommandButton->setEnabled(clicked);
    commandTextEdit->setReadOnly(false);
  } else {
    editCommandButton->setStyleSheet(QString("QPushButton { color : %1; }").arg(QApplication::palette().text().color().name()));
    commandTextEdit->setPalette(readOnlyPalette);
    resetCommandButton->setEnabled(false);
    commandTextEdit->setReadOnly(true);
  }


}

void CommandsDialog::copyToClipboard() {

  QAction *action = qobject_cast<QAction *>(sender());

  static const QLatin1String COMMAND("command");
  const bool commands = action->data().toString() == COMMAND;

  const QModelIndex &modelIndex = commands ? commandTableView->selectionModel()->currentIndex() :
                                             snippetTableView->selectionModel()->currentIndex();

  if (modelIndex.isValid()) {
    const QString data = commands ? commandsProxyTableModel->data(modelIndex, Qt::DisplayRole).toString() :
                                    snippetsProxyTableModel->data(modelIndex, Qt::DisplayRole).toString();

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(data, QClipboard::Clipboard);

    if (clipboard->supportsSelection())
      clipboard->setText(data, QClipboard::Selection);

#if defined(Q_OS_LINUX)
    QThread::msleep(1); //workaround for copied text not being available...
#endif
  }
}

void CommandsDialog::customMenuRequested(QPoint pos) {

  QAction *copyToClipboardAct = new QAction(QIcon(":/resources/copytoclipboard.png"),tr("Copy to Clipboard"), this);
  copyToClipboardAct->setObjectName("copyToClipboardAct.6");
  copyToClipboardAct->setShortcut(QKeySequence::Copy);
  lpub->actions.insert(copyToClipboardAct->objectName(), Action(QStringLiteral("Edit.Copy To Clipboard"), copyToClipboardAct));

  QMenu *menu = new QMenu(this);
#ifndef QT_NO_CLIPBOARD
  menu->addAction(copyToClipboardAct);
#endif
  if (sender() == commandTableView) {
    copyToClipboardAct->setData(QString("command").toLatin1());
    menu->popup(commandTableView->viewport()->mapToGlobal(pos));
  } else {
    copyToClipboardAct->setData(QString("snippet").toLatin1());
    menu->popup(snippetTableView->viewport()->mapToGlobal(pos));
  }

  connect (copyToClipboardAct, &QAction::triggered,
           this,               &CommandsDialog::copyToClipboard);
}

// Snippet items

void CommandsDialog::snippetFilterEditChanged()
{
  const static QString TIP(tr("Add a new command snippet"));
  const static QString NOFILTER(tr(" - reset filter to enable"));
  const bool canAdd = snippetFilterEdit->text().isEmpty();
  addSnippetButton->setEnabled(canAdd);
  addSnippetButton->setToolTip(canAdd ? TIP : TIP + NOFILTER);

  QRegExp regExp(snippetFilterEdit->text(),
                 snippetFilterEdit->caseSensitivity(),
                 snippetFilterEdit->patternSyntax());

  snippetsProxyTableModel->setFilterRegExp(regExp);
}

void CommandsDialog::currentSnippetChanged(const QModelIndex &current, const QModelIndex &)
{
  const int row = current.row();
  const QModelIndex &modelIndex = snippetsProxyTableModel->index(row, 0, QModelIndex());

  if (modelIndex.isValid()) {
    const QString trigger = snippetsProxyTableModel->data(modelIndex, Qt::EditRole).toString();
    const Snippet snippet = snippetCollection->snippet(trigger);
    const bool canEdit = snippet.builtIn != Snippet::True;

    // update text edit for snippet content
    QString formattedSnippet(snippet.snippet);
    formattedSnippet.insert(snippet.cursorPosition, "$|");

    snippetTextEdit->setCommandText(formattedSnippet);

    removeSnippetButton->setEnabled(canEdit);
    snippetTextEdit->setReadOnly(!canEdit);
    snippetTextEdit->setPalette(canEdit ? QApplication::palette() : readOnlyPalette);
  }
}

void CommandsDialog::snippetTextChanged()
{
  const QModelIndex &viewIndex = snippetTableView->selectionModel()->currentIndex();
  const int row = viewIndex.row();
  const QModelIndex &modelIndex = snippetsProxyTableModel->index(row, 0, QModelIndex());

  if (modelIndex.isValid()) {
      QString trigger = snippetsProxyTableModel->data(modelIndex, Qt::DisplayRole).toString();

      Snippet snippet = snippetCollection->snippet(trigger);
      if (snippet.builtIn != Snippet::True) {
        snippet.snippet = snippetTextEdit->toPlainText();

        // find cursor marker
        int pos = snippet.snippet.indexOf("$|");
        if (pos >= 0) {
          snippet.cursorPosition = pos;
        snippet.snippet.remove(pos, 2);
      }

      snippetCollection->update(snippet);
    }
  }
}

void CommandsDialog::addSnippetButtonClicked()
{
  SnippetsTableModel *snippetModel = qobject_cast<SnippetsTableModel*>(snippetsProxyTableModel->sourceModel());

  const QModelIndex &sourceIndex = snippetModel->createSnippet();

  QModelIndex modelIndex = snippetsProxyTableModel->mapFromSource(sourceIndex);

  if (!modelIndex.isValid()) {
      if (!snippetFilterEdit->text().isEmpty()) {
          const QString filterText = snippetFilterEdit->text();
          snippetFilterEdit->clear();
          modelIndex = snippetsProxyTableModel->mapFromSource(sourceIndex);
          if (modelIndex.isValid())
              QMessageBox::information(this, tr("Add Snippet"), tr("View is filtered, reset filter to add snippet.."));
          snippetFilterEdit->setText(filterText);
      }
      if (!modelIndex.isValid())
          emit lpub->messageSig(LOG_WARNING,tr("Invalid row %1 and column %2, cannot add snippet")
                                             .arg(modelIndex.row()).arg(modelIndex.column()),LOG_SHOW_DIALOG);
      return;
  }

  const int row = modelIndex.row();
  QModelIndex topLeft = snippetModel->index(row, 0, QModelIndex());
  QModelIndex bottomRight = snippetModel->index(row, 1, QModelIndex());
  QItemSelection selection(topLeft, bottomRight);

  snippetTableView->selectionModel()->select(selection, QItemSelectionModel::SelectCurrent);
  snippetTableView->setCurrentIndex(topLeft);
  snippetTableView->scrollTo(topLeft);

  snippetTableView->edit(modelIndex);
}

void CommandsDialog::removeSnippetButtonClicked()
{
  const QModelIndex &viewIndex = snippetTableView->selectionModel()->currentIndex();
  const QModelIndex &modelIndex = snippetsProxyTableModel->mapToSource(viewIndex);
  if (!modelIndex.isValid()) {
    emit lpub->messageSig(LOG_WARNING,tr("No snippet selected."),LOG_SHOW_DIALOG);
    return;
  }

  SnippetsTableModel *snippetModel = qobject_cast<SnippetsTableModel*>(snippetsProxyTableModel->sourceModel());
  snippetModel->removeSnippet(modelIndex);
}

void CommandsDialog::moveLine()
{
  QPushButton *button = qobject_cast<QPushButton *>(sender());
  if (button == moveLineUpButton)
    emit moveLineUp();
  else
    emit moveLineDown();
}

void CommandsDialog::setCommandEdit(bool b)
{
  lpub->commandsDialog->commandEdit = b;
}

void CommandsDialog::showCommandEditButtons(bool b)
{
  lpub->commandsDialog->insertCommandBeforeButton->setVisible(b);
  lpub->commandsDialog->insertCommandAfterButton->setVisible(b);
  lpub->commandsDialog->moveLineUpButton->setVisible(b);
  lpub->commandsDialog->moveLineDownButton->setVisible(b);
  lpub->commandsDialog->commandUsageLabel->setVisible(b);
  lpub->commandsDialog->snippetUsageLabel->setVisible(b);
}

void CommandsDialog::enableCommandEditButtons(bool b)
{
  lpub->commandsDialog->insertCommandBeforeButton->setEnabled(b);
  lpub->commandsDialog->insertCommandAfterButton->setEnabled(b);
  lpub->commandsDialog->moveLineUpButton->setEnabled(b);
  lpub->commandsDialog->moveLineDownButton->setEnabled(b);
}

void CommandsDialog::showCommandsDialog(QWidget *parent)
{
  lpub->commandsDialog->setParent(parent);
  if (lpub->commandsDialog->commandEdit)
    lpub->commandsDialog->showCommandEditButtons(true);
  lpub->commandsDialog->show();
  lpub->commandsDialog->raise();
  lpub->commandsDialog->activateWindow();
}

void CommandsDialog::accept()
{
  // commit pending modifications
  for (int i = 0; i < commandCollection->count(); ++i) {
      Command command = commandCollection->at(i);
      if (command.modified == Command::Pending) {
          command.modified = Command::True;
          commandCollection->update(command);
      }
  }
  for (int i = 0; i < snippetCollection->count(); ++i) {
      Snippet snippet = snippetCollection->at(i);
      if (snippet.builtIn == Snippet::Pending) {
          snippet.builtIn = Snippet::False;
          snippetCollection->update(snippet);
      }
  }

  const QString userDataPath = QString("%1/extras").arg(Preferences::lpubDataPath);

  // path to user-updated command descriptions
  QSharedPointer<CommandCollection> userModifiedCommands = commandCollection->userModifiedCommands();
  JsonFile<Command>::save(QDir::toNativeSeparators(userDataPath + "/user-command-descriptions.json"), userModifiedCommands.data());

  // path to user-defined command snippets
  QSharedPointer<SnippetCollection> userDefinedSnippets = snippetCollection->userDefinedSnippets();
  JsonFile<Snippet>::save(QDir::toNativeSeparators(userDataPath + "/user-snippets.json"), userDefinedSnippets.data());

  writeSettings();
  QDialog::accept();
}

void CommandsDialog::cancel()
{
  for (int i = 0; i < commandCollection->count(); ++i) {
    Command command = commandCollection->at(i);
    if (command.modified == Command::Pending) {
        command.modified = Command::False;
        commandCollection->update(command);
    }
  }
  for (int i = 0; i < snippetCollection->count(); ++i) {
      Snippet snippet = snippetCollection->at(i);
      if (snippet.builtIn == Snippet::Pending)
          snippetCollection->remove(snippet);
  }
  writeSettings();
  QDialog::reject();
}

bool CommandsDialog::maybeSave()
{
  bool rc = false;

  bool commandEditModified = commandTextEdit->document()->isModified();
  bool snippetEditModified = snippetTextEdit->document()->isModified();

  if (commandEditModified || snippetEditModified) {

    const QString which = snippetEditModified && commandEditModified ? QLatin1String("Command") : snippetEditModified ? QLatin1String("Snippet") : QLatin1String("Command");

    QMessageBoxResizable box;
    box.setWindowIcon(QIcon());
    box.setIconPixmap (QPixmap(LPUB3D_MESSAGE_ICON));
    box.setTextFormat (Qt::RichText);
    box.setWindowTitle(tr ("%1 %2s").arg(VER_PRODUCTNAME_STR, which));
    box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    QString title = "<b>" + tr ("%1 changes detected").arg(which) + "</b>";
    QString text = tr("%1 text has been modified.<br>Do you want to save your changes?").arg(which);
    box.setText (title);
    box.setInformativeText (text);
    box.setStandardButtons (QMessageBox::No | QMessageBox::Yes);
    box.setDefaultButton   (QMessageBox::Yes);

    if (box.exec() == QMessageBox::Yes)
      rc = true;
  }
  return rc;
}

void CommandsDialog::applyCommandEdit()
{
  QPushButton *button = qobject_cast<QPushButton *>(sender());
  bool insertAfter = button == insertCommandAfterButton;

  const QModelIndex &modelIndex = commandTableView->selectionModel()->currentIndex();
  if (modelIndex.isValid()) {
    const QString command = commandsProxyTableModel->data(modelIndex, Qt::EditRole).toString();
    emit commandSnippet(command, insertAfter);
  } else {
    emit lpub->messageSig(LOG_WARNING,tr("No command selected."), LOG_SHOW_DIALOG);
  }
}

void CommandsDialog::writeSettings()
{
    QSettings Settings;
    Settings.beginGroup(COMMANDSWINDOW);
    Settings.setValue("Geometry", saveGeometry());
    Settings.setValue("Size", size());
    Settings.setValue("CurrentTab", tabWidget->currentIndex());
    Settings.setValue("SnippetHeader", snippetTableView->horizontalHeader()->saveState());
    Settings.endGroup();
}

void CommandsDialog::readSettings()
{
    QSettings Settings;
    Settings.beginGroup(COMMANDSWINDOW);
    restoreGeometry(Settings.value("Geometry").toByteArray());
    QSize size = Settings.value("Size", QDesktopWidget().availableGeometry(this).size()*0.5).toSize();
    resize(size);
    tabWidget->setCurrentIndex(Settings.value("CurrentTab").toInt());
    snippetTableView->horizontalHeader()->restoreState(Settings.value("SnippetHeader").toByteArray());
    Settings.endGroup();
}

void CommandsDialog::closeEvent(QCloseEvent *event)
{
  writeSettings();

  if (maybeSave())
    accept();
  else
    cancel();

  showCommandEditButtons(false);

  setCommandEdit(false);

  QDialog::closeEvent(event);
}
