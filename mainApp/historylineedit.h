/**
 * \file
 *
 * \author Mattia Basaglia
 * \author Trevor SANDY
 *
 * \copyright Copyright (C) 2012-2015 Mattia Basaglia
 * \copyright Copyright (C) 2020-2020 Trevor SANDY
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef HISTORY_LINE_EDIT_H
#define HISTORY_LINE_EDIT_H

#include <QLineEdit>
#include <QCompleter>

/**
 * \brief Line edit providing a history of entered text
 */
class HistoryLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    explicit HistoryLineEdit(QWidget *parent = nullptr);
	
    ~HistoryLineEdit()override{}

    /**
     * \brief Number of available lines
     */
    int lineCount() const { return lines.size(); }

    /**
     * \brief Stored history
     */
    QStringList history() const { return lines; }

    /**
     * \brief Overwrite the line history
     */
    void setHistory(const QStringList& history);

    /**
     * \brief Sets the completer used on a per-word completion
     *
     * Unlike setCompleter(), this suggests completion at every entered word
     *
     * If \c completer is null it will remove the current completer
     */
    void setWordCompleter(QCompleter* completer);

    /**
     * \brief Sets a prefix that is ignored by the word completer
     */
    void setWordCompleterPrefix(const QString& prefix);

    /**
     * \brief Sets the minimum number of characters required to display the word completer
     */
    void setWordCompleterMinChars(int min_chars);

    /**
     * \brief Sets the maximum number of suggestions that the completer should show.
     *
     * If more than this many suggestions are found the completer isn't shown
     */
    void setWordCompleterMaxSuggestions(int max);

public slots:
    /**
     * \brief Executes the current line
     */
    void execute();

signals:
    /**
     * \brief Emitted when some text is executed
     */
    void lineExecuted(QString);

protected:
    void keyPressEvent(QKeyEvent *) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *) Q_DECL_OVERRIDE;

    void previous_line();
    void next_line();

    /**
     * \brief Current word being edited (used to fire the completer)
     */
    QString current_word() const;

private slots:
    /**
     * \brief Autocompletes the current word
     */
    void autocomplete(const QString& completion);

private:
    /**
     * \brief Returns the index of the character starting the currently edited word
     */
    int word_start() const;

    int         current_line;
    QStringList lines;
    QString     unfinished;

    QCompleter* completer;
    QString     completion_prefix;
    int         completion_minchars;
    int         completion_max;
};

#endif // HISTORY_LINE_EDIT_H
