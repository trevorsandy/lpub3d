/****************************************************************************  
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2016 Trevor SANDY. All rights reserved.
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

/***************************************************************************
 *
 *  Program structure, organization and file naming conventions.
 *
 *    LPub is conceptually written in layers.  The lower level layers
 *  interact with the real LDraw files, and the highest layers interact
 *  with Qt, and therefore the computer system, its mouse, keyboard,
 *  graphics display and file system.
 *
 *  Fundamental concept files:
 *    lpub.(h,cpp) - the application (outermost) layer of code.  When
 *      LPub first starts up, this outer layer of code is in charge.
 *      main.cpp is the actual start of program, but that simply 
 *      creates and destroys an implementation of LPub's application
 *      defined in lpub.(h,cpp)
 *  
 *    ldrawfiles(h,cpp) - knows how to read and write MPD files and 
 *      retain the contents in a list of files.  For non-MPD files, 
 *      this layer reads in top level file, and any submodel files 
 *      that can be found in the same directory as the top level file. 
 *      After being loaded, the rest of LPub does not care if the 
 *      model came from MPD or not.  The rest of LPub only interacts
 *      with the model through this layer of code.
 *
 *    color(h,cpp) - knows about LDraw color numbers and their RGB values.
 *
 *    paths(h,cpp) - a place to put the names of external dependencies like
 *      the path for the LDraw file system, the path to ldglite, etc.
 *
 *    render(h,cpp) - provides access to the renderer which means either
 *      LDGLite or LDView. You can set your preferred renderer. (red. Jaco)
 *
 *  The next layer has to do with the parsing of the LDraw files and knowing
 *  what to do with them.  At the lowest level, LPub's parsing is line based
 *  as specified in the LDraw file specification.  At the higher layers, the
 *  recognition of the meaning of the lines implies how LPub should respond to
 *  them.  LDraw type 1 through 5 lines are obviously model content and eventually
 *  get fed to the renderer.  LDraw type 0 lines provide meta-commands like STEP.
 *  LPub specific meta-commands come in two flavors:  configuration metas, and
 *  action metas.  configuration metas values are retained, and action metas
 *  potentially use the values from configuration metas to do their jobs.
 *
 *  This and higher layers are built around some fundamental concepts
 *  provided by LPub. These include:
 *    Step - literally, this means the LDraw defined STEP metacommand.
 *           It also means an MLCad ROTSTEP metacommand.  For LPub it means
 *           one or more LDraw type 1 through type 5 lines (part, line,
 *           triangle, and quads) followed by STEP, ROTSTEP or end of file
 *           (an implied step).  Empty STEPs: ones not preceded by the addition
 *           of type 1 through type 5 lines are ignored.
 *           For LPub, conceptually step means something that needs to
 *           be displayed in your building instructions:
 *              - a step number
 *              - the model after all previous type1 through 5 lines 
 *                are added together
 *              - any of the type1 lines since the previous step (e.g. "in
 *                this step"), that are submodels.  These are either
 *                called out (see callout below), or represent one or more
 *                building instruction pages.
 *              - the list of part type/colors for parts list images
 *   MultiStep - a collection of steps in the current model/submodel.
 *          These steps are displayed in rows (horizontally), or columns
 *          vertically.  A multi-step can contain multiple rows or columns.
 *   Callout - a collection of all the steps in a submodel, being displayed
 *          on the current page.  These steps are displayed in rows or columns.
 *          A callout can contain multiple rows or columns.
 *   Pointer - a visual indicator starting at a callout, and ending at the
 *          assembly image to which the callout belongs.
 *   ranges.h - the interal representation for both multi-steps and callouts.
 *          A ranges is a list of one or more individual range(s).
 *   Range - one row or column in a multi-step or callout. A range contains
 *          one or more steps.
 *   Step again - a step can contain one or more callouts.  Callouts contain
 *          one or more ranges, which contain one or more range-s, which
 *          contain one or more steps.  Callouts in callouts in callouts
 *          can happen, and LPub has to deal with it.
 *
 *   LPub is page oriented.  As it walks through your LDraw file creating
 *   building instructions, it reads and internalizes a page worth of the
 *   lines in your model.  It keeps what is needed to draw the page in a
 *   tree of data structures.  A page has a page number, one or more
 *   assembly step images, possibly a parts list image, zero or more
 *   callouts, with zero or more pointers.
 *
 *   Once LPub recognizes a page boundary (end of multistep, STEP or 
 *   end of file (implied step), it converts the tree for that
 *   page into graphical representations and displays them so the user
 *   can interact with them.
 *
 *   The cornerstone of this page oriented process is line by line parsing
 *   and recognition of the lines in your ldraw file.  There are two functions
 *   that do this parsing.  findPage traverses the model higherarchy, counting
 *   pages.  One issue is that you do not know the page number at start of
 *   step, because non-callout submodels result in pages.  findPage is lightweight
 *   mechanism for scanning through the design, and finding the page of interest.
 *   at each page boundary, if the current page number is not the desired page
 *   and the current page is before the desired page, the state of the parse is
 *   saved.  When we hit the page we want to display, the saved state is passed
 *   to drawPage.  drawPage ignores any non-callout submodels (those were taken
 *   care of by findPage) gathers up any callouts, and at end of page, converts
 *   the results to Qt GraphicsItems.  After drawPage draws the page, it returns
 *   to findPage, which traverses the rest of the model hierarchy counting the
 *   total number of pages.
 *
 *   findPage and drawPage present a bit of a maintainability dilema, because
 *   for a few things, there is redundnant code.  This is small, though, and
 *   having findPage as a separate function, makes optimizations there easier.
 *
 *   findPage and drawPage know about all LDraw line types and deals with 
 *   types 1 through 5 directly.  They depends on a whole set of classes to 
 *   parse and possibly retain information from meta-commands (type 0 lines).  
 *   findPage and drawPage both deal with "0 GHOST" meta-commands themselves, 
 *   but all other metacommand parsing is done by the Meta class and its 
 *   associated meta subclasses.
 *
 *   findPage and drawPage's interface to the Meta class is through the 
 *   parse function.  meta.parse() is handed an ldraw type 0 line, and the
 *   file and lineNumber that the line came from.  For configuration meta-commands
 *   the meta classes retain the filename/linenumber information for use
 *   in implementing backannotation of user changes into the LDraw files.
 *   meta.parse() provides a return code indicating what it saw.  Some 
 *   meta-commands (like LDraw's STEP, MLCad's ROTSTEP, and LPub's CALLOUT BEGIN) 
 *   are action meta-commands.  Each action meta-command has its own specific
 *   return code.  Configuration meta-commands provide a return code that
 *   says the line parsed ok, or failed parsing.
 *
 *   Meta is composed of other classes, and is described and implemented
 *   in meta.(h,cpp).  Some of those classes are LPubMeta (which is composed
 *   of a whole bunch of other classes), and MLCadMeta (which is also composed
 *   of other classes.
 *
 *   The LPubMeta class is composed of major lpub concepts like, page (PageMeta),
 *   callout (CalloutMeta), multi-step (MultiStepMeta), and parts list (PliMeta).
 *   These are all composed of lower level classes.  There are mid-layer
 *   abstractions like the concept of a number which has font, color
 *   and margins, or LPub concepts like placement, background, border, and
 *   divider (the visual thing that sits between rows or columns of steps).
 * 
 *   Then there are the bottom layer classes like, an integer
 *   number, a floating point number, a pair of floating point numbers, units,
 *   booleans, etc.  Units are like floating points, except their internal
 *   values can be converted to pixels (which is the cornerstone concept of
 *   supporting dots per inch or dots per centimeter.  
 * 
 *   These are all derived from an abstract class called LeafMeta. Leaf
 *   metas provide the handling of the "rest of the meta-command", typically
 *   parsing the actual values of a specific configuration meta-command, or
 *   returning a special return code for action meta-commands.  Every leaf
 *   has the knowledge of where it was defined in your LDraw model (modelName,
 *   lineNumber).  See the Where class in where.(h,cpp).  The
 *   Where information (filename, linenumber) is used to implement backannotation
 *   of user changes into the LDraw files.
 *
 *   All metas that are not derived from LeafMeta are derived from
 *   BranchMeta.  A BranchMeta represent non-terminal nodes in the meta-command
 *   syntax graph.  A BranchMeta contains a list of keywords, each of which
 *   is associated with an instance of a meta that handles that keyword.
 *   The list of keyword/metas can contain either Leaf or Branch Metas,
 *   both of which are derived from AbstractMeta.
 *
 *   So, in the big picture Meta contains all the values associated with all
 *   possible meta-commands, or if not specified, their default values.
 *   Meta is handed around to various layers of the process for converting
 *   the page's contents to graphical entities.
 *
 *   When findPage hits end of page, for the page being displayed, it returns
 *   a tree of the page's contents, and the configuration information (Meta)
 *   to its caller.
 *
 *   There are only a few callers of findPage, the most important being
 *   drawPage (not the detailed one findPage calls, but a highlevel one
 *   that takes no parameters.  Draw page converts the LDraw file structure 
 *   tree (ranges, range, step) and the configuration tree (Meta) into 
 *   graphical entities. drawPage is a member of LPub (therefore lpub.h, 
 *   and implemented in traverse.cpp.
 *
 *   The LDraw structure tree is composed of classes including:
 *     ranges.h  - ranges.(h,cpp), ranges_element.(h,cpp)
 *       By itself, ranges.h represents multi-step.  Single step per page
 *       is represented by ranges.h that contains one range, that contains
 *       one step.  ranges.h is the top of structure tree provided by
 *       traverse.
 *     Range   - range.(h,cpp), range_element.(h,cpp)
 *     Step    - step.(h,cpp)
 *     Callout - callout.(h,cpp)
 *     Pli     - pli.(h,cpp)
 *
 *   These classes all represent things visible to the reader of your
 *   building instructions.  They each have to be placed on the page.
 *   They are all derived from the Placement class, which describes
 *   what they are placed next to, and how they are placed relative
 *   to that thing.  See placement.(h,cpp).
 *
 *   Each of these items (ranges.h, Range, Step, Callout, Pli, etc.)
 *   knows how to size itself (in pixels).  Once sizes are known, these
 *   things can be placed relative to the things they are relative to.
 *   At the top level, one or more things are placed relative to the page.
 *
 *   Once placed, graphical representations of these things are created.
 *   In particular, they are converted to graphical "items", added to
 *   the page's "scene", which is displayed in a "view" which is in
 *   a big window in the LPub application.  The items/scene/view concepts
 *   are defined by Qt and are very powerful abstactions.  LPub's
 *   "scene" components are implemented in files name *item.(h,cpp)
 *
 *     csiitem(h,cpp) an assembly image
 *     ranges_item(h,cpp)
 *     numberitem.(h,cpp)
 *     calloutbackgrounditem.(h,cpp)
 *     pointeritem.(h,cpp)
 *     backgrounditem.(h,cpp)
 *     
 *   In the case of PLIs, the "item" implementation is in pli.(h,cpp)
 *
 *   Once the page components are sized, placed and put into the scene,
 *   Qt displays the view to the user for editing.  The various graphical
 *   "items" can have menus, and can handle mouse activity, like press,
 *   move, and release.  Each of these activities is translated into
 *   an event, which is handed to the associated item.
 *
 *   Events typically imply a user change to the LDraw files.  The file
 *   metaitem.(h,cpp) provides methods for backannotating this editing
 *   activity to the LDraw files.  Some of these events imply the need
 *   for a user to change some configuration.  Configuration is maintained
 *   in Meta's, thus the name meta-item.  Meta's not only retain the values
 *   for configuration, but what file/line last set the current value.
 *
 *   Certain of these metas' values are changed via graphical dialogs. These
 *   dialogs contain push buttons, check boxes, radio buttons, and such
 *   that let the user change values.  Some dialogs use Qt dialogs like,
 *   font or color.  After the user changes some value(s) and hits OK,
 *   The values are backannotated into the LDraw file.  At this point
 *   LPub invokes traverse to count pages, gathers up the content of the
 *   current page, produces a new scene that is displayed for the user.
 *
 *   The graphical representations for configuration metas are implemented
 *   in metagui.(h,cpp).  Each of these is used in at least two ways:
 *     Global - values that are specified before the first step of the
 *       entire model (think of them as defaults).
 *       projectglobals.cpp
 *       pageglobals.cpp
 *       multistepglobals.cpp
 *       assemglobals.cpp
 *       pliglobals.cpp
 *       calloutglobals.cpp
 *       fadestepglobals.cpp
 *     Local - values that are specified in something other than the first
 *       global step and are invoked individually in small dialogs
 *       backgrounddialog.(h,cpp)
 *       borderdialog.(h,cpp)
 *       dividerdialog.(h,cpp)
 *       placementdialog.(h,cpp)
 *       pairdialog.(h,cpp) - single integer, floating point
 *                            pair of floating point
 *       scaledialog.(h,cpp) - single and pair dialogs for values retained
 *                             in either inches or centimeters.  Quite
 *                             possibly this could be combined with pairdialog
 *                             and eliminated.
 *
 *   placementdialog lets the user access LPub's placement concept of
 *   things placed relative to things with margins in between.
 *
 *   The can also use the mouse to drag things around on the page, to change
 *   placement.  The implementations of these movements and their backannotation
 *   into the ldraw files are implemented in *item.cpp
 *
 *   This only leaves a few source files undescribed.  Part of the 
 *   LPub gui presented to the user is a texual display of the LDraw
 *   file.  It is displayed using editwindow.(h,cpp) using a Qt QTextEdit
 *   object.  The syntax highlighing that goes with that is implemented
 *   in highlighter.(h,cpp).
 *
 *   Like gui edits, the manual edits in the editWindow have to be
 *   back annotated into the ldraw files.  It is important to note that
 *   the text displayed in editWindow is only a copy of what is in ldrawFiles.
 *   User changes in these files cause an event that tells you position
 *   (i.e. how many characters from start of file), number of characters
 *   added, and number of characters deleted.  It annoyingly does not
 *   tell you the actual characters added or the characters deleted, just
 *   position and how many.
 *
 *   editWindow extracts the contents of textEdit and picks out the new
 *   characters that were added.  It then signals Gui that at this position
 *   (in the file you told me to display), this many characters were deleted,
 *   and these characters were added.
 *
 *   The Gui code examines the ldrawFiles version of the file being displayed
 *   by the editWindow, and calculates the characters that were deleted.
 *   With position, characters added and characters deleted, we can perform
 *   the edit, and undo it.
 *
 *   Most of the user activity maps to changes of the LDraw file(s).  All these
 *   changes are funneled through Qt's undo/redo facility.  So, new meta
 *   commands are inserted into the LDraw file, changes to an existing meta
 *   are replaced in the LDraw file, and removal of metas (think undo),
 *   means lines are deleted in the LDraw file(s).
 *
 *   Each of these activities is implemented as a Qt command, which works
 *   in conjuction with the undo redo facility.  These commands are
 *   implemented in commands.(h,cpp).
 *
 *   This leaves only a few miscellanous files unexplained.  commonmenis.(h,cpp)
 *   could just as easily been named metamenus.(h.cpp).  These implement
 *   popup menu elements (and their help information) for commonly used
 *   meta commands.
 *
 *   resolution.(h,cpp) contain some variables that define the resolution
 *   (e.g. 72) per units (inches vs. centimeters).  Most of the resolution
 *   dependence is implemented in meta.(h,cpp), but a few other situations
 *   require the knowledge of resolution.
 */
#ifndef GUI_H
#define GUI_H

#include <QtGlobal>
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets/QApplication>
#else
#include <QApplication>
#endif
#include <QMainWindow>
#include <QMenuBar>
#include <QAction>
#include <QToolBar>
#include <QStatusBar>
#include <QDockWidget>
#include <QSettings>
#include <QGraphicsView>
#include <QDateTime>
#include <QFileSystemWatcher>
#include <QComboBox>
#include <QPrinter>
#include <QFile>
#include <QProgressBar>
#include <QElapsedTimer>
 
#include "color.h"
#include "ranges.h"
#include "ldrawfiles.h"
#include "where.h"
#include "aboutdialog.h"
#include "version.h"
#include "threadworkers.h"
#include "FadeStepColorParts.h"
#include "plisubstituteparts.h"
#include "dialogexportpages.h"

//** 3D
#include "lc_math.h"
#include "lc_library.h"
#include "lc_mainwindow.h"
#include "progress_dialog.h"
#include "QsLog.h"



class QString;
class QSplitter;
class QGraphicsScene;
class QFrame;
class QFileDialog;
class QResizeEvent;
class QLineEdit;
class QUndoStack;
class QUndoCommand;

class EditWindow;
class ParmsWindow;

class Meta;
class Pli;

class InsertLineCommand;
class DeleteLineCommand;
class ReplaceLineCommand;
class ContentChangesCommand;

class PlacementNum;
class AbstractStepsElement;
class GlobalFadeStep;
class UpdateCheck;

class LGraphicsView;
class PageBackgroundItem;

enum traverseRc { HitEndOfPage = 1 };

enum FitMode { FitNone, FitWidth, FitVisible, FitTwoPages, FitContinuousScroll };

enum ExportOption { EXPORT_ALL_PAGES, EXPORT_PAGE_RANGE, EXPORT_CURRENT_PAGE };
enum ExportType { EXPORT_PDF, EXPORT_PNG, EXPORT_JPG, EXPORT_BMP };

void clearPliCache();
void clearCsiCache();
void clearCsi3dCache();
void clearAndRedrawPage();

class Gui : public QMainWindow
{
  Q_OBJECT

public:
  Gui();
  ~Gui();

  int             displayPageNum;  // what page are we displaying
  int             stepPageNum;     // the number displayed on the page
  int             saveStepPageNum;
  int             firstStepPageNum;
  int             lastStepPageNum; 
  QList<Where>    topOfPages;

  int             boms;            // the number of pli BOMs in the document
  int             bomOccurrence;   // the acutal occurenc of each pli BOM

  int             exportType;      // export Type
  int             exportOption;    // export Option
  QString         pageRangeText;    // page range parameters

  bool             m_cancelPrinting; // cancel print job
  ProgressDialog  *m_progressDialog; // general use progress dialog
  QLabel          *m_progressDlgMessageLbl;
  QProgressBar    *m_progressDlgProgressBar;

  void            *noData;
  /**Fade Step variables**/
  FadeStepMeta    *fadeMeta;             // propagate fade color and fade bool
  FitMode          fitMode;              // how to fit the scene into the view

  Where &topOfPage();
  Where &bottomOfPage();

  void    changePageNum(int offset)
  {
    displayPageNum += offset;
  }
  void    displayPage();

  /* We need to send ourselved these, to eliminate resursion and the model
   * changing under foot */
  void drawPage(                   // this is the workhorse for preparing a
    LGraphicsView *view,           // page for viewing.  It depends heavily
    QGraphicsScene *scene,         // on the next two functions
    bool            printing);

  /*--------------------------------------------------------------------*
   * These are the work horses for back annotating user changes into    *
   * the LDraw files                                                    *
   *--------------------------------------------------------------------*/

  QStringList fileList()
  {
    return ldrawFile.subFileOrder();
  }
  int subFileSize(const QString &modelName)
  {
    return ldrawFile.size(modelName);
  }
  int numSteps(const QString &modelName)
  {
    return ldrawFile.numSteps(modelName);
  }
  QString readLine(const Where &here);

  bool isSubmodel(const QString &modelName)
  {
    return ldrawFile.isSubmodel(modelName);
  }
  bool isMpd()
  {
    return ldrawFile.isMpd();
  }
  bool isOlder(const QStringList &foo,const QDateTime &lastModified)
  {
    bool older = ldrawFile.older(foo,lastModified);
    return older;
  }
  bool isMirrored(QStringList &argv)
  {
    return ldrawFile.mirrored(argv);
  }
  bool isUnofficialPart(const QString &name)
  {
    return ldrawFile.isUnofficialPart(name);
  }

  void insertGeneratedModel(const QString &name,
                                  QStringList &csiParts) {
    QDateTime date;
    ldrawFile.insert(name,csiParts,date,false,true);
    writeToTmp();
  }

  LDrawFile getLDrawFile()
  {
      return ldrawFile;
  }
  void insertLine (const Where &here, const QString &line, QUndoCommand *parent = 0);
  void appendLine (const Where &here, const QString &line, QUndoCommand *parent = 0);
  void replaceLine(const Where &here, const QString &line, QUndoCommand *parent = 0);
  void deleteLine (const Where &here, QUndoCommand *parent = 0);
  QString topLevelFile();
  void beginMacro (QString name);
  void endMacro   ();

  bool InitializeApp(int argc, char *argv[], const char* LibraryInstallPath, const char* LDrawPath);

  void displayFile(LDrawFile *ldrawFile, const QString &modelName);
  void displayParmsFile(const QString &fileName);
  void halt3DViewer(bool b);
  QString elapsedTime(const qint64 &time);

  int             maxPages;
  
  LGraphicsView *pageview()
  {
    return KpageView;
  }
  //**3D  
  void UpdateStepRotation();

  lcVector3 GetStepRotationStatus() const
  {
      return mModelStepRotation;
  }

  lcVector3 GetExistingRotStep() const
  {
      return mExistingRotStep;
  }

  void ResetStepRotation()
  {
      mRotStepAngleX = mExistingRotStep[0];
      mRotStepAngleY = mExistingRotStep[1];
      mRotStepAngleZ = mExistingRotStep[2];
      UpdateStepRotation();
  }

  void SetExistingRotStep(lcVector3 rotStep)
  {
      mExistingRotStep = rotStep;
  }

  void SetRotStepAngleX(float AngleX)
  {
      mRotStepAngleX = AngleX;
  }

  void SetRotStepAngleY(float AngleY)
  {
      mRotStepAngleY = AngleY;
  }

  void SetRotStepAngleZ(float AngleZ)
  {
      mRotStepAngleZ = AngleZ;
  }

  QString getCurFile()
  {
      return curFile;
  }

  //**


public slots:

  /* The undoStack needs access to these */

  void canRedoChanged(bool);
  void canUndoChanged(bool);
  void cleanChanged(bool);

  /* The edit window sends us these */

  void contentsChange(const QString &fileName,int position, int charsRemoved, const QString &charsAdded);

  void parseError(QString errorMsg,Where &here)
  {
    showLine(here);
    if (1) {
      statusBarMsg(QString("%1 %2:%3") .arg(errorMsg) .arg(here.modelName) .arg(here.lineNumber));
    } else {
      QMessageBox::warning(this,tr("LPub3D"),tr(errorMsg.toLatin1()));
    }
  }

  void statusMessage(bool status, QString message){
      if (status){
          statusBarMsg(message);
      }else{
          QMessageBox::warning(this,tr("LPub3D"),tr(message.toLatin1()));
      }
  }

  void statusBarMsg(QString msg);

  void showLine(const Where &topOfStep)
  {
    displayFile(&ldrawFile,topOfStep.modelName);
    showLineSig(topOfStep.lineNumber);
  }

  // cancel printing
  void cancelPrinting(){m_cancelPrinting = true;}

  // left side progress bar
  void progressBarInit();
  void progressBarSetText(const QString &progressText);
  void progressBarSetRange(int minimum, int maximum);
  void progressBarSetValue(int value);
  void progressBarReset();
  // right side progress bar
  void progressBarPermInit();
  void progressBarPermSetText(const QString &progressText);
  void progressBarPermSetRange(int minimum, int maximum);
  void progressBarPermSetValue(int value);
  void progressBarPermReset();

  void removeProgressStatus(){
      statusBar()->removeWidget(progressBar);
      statusBar()->removeWidget(progressLabel);
  }

  void removeProgressPermStatus(){
      statusBar()->removeWidget(progressBarPerm);
      statusBar()->removeWidget(progressLabelPerm);
  }

  void preferences();
  void fadeStepSetup();
  void generateCoverPages();
  void insertFinalModel();

  void pageSetup();
  void assemSetup();
  void pliSetup();
  void bomSetup();
  void calloutSetup();
  void multiStepSetup();
  void projectSetup();
  
  void fitWidth();
  void fitVisible();
  void actualSize();

  void twoPages();
  void continuousScroll();

  
  void fitWidth(  LGraphicsView *view);
  void fitVisible(LGraphicsView *view);
  void actualSize(LGraphicsView *view);

  void twoPages(LGraphicsView *view);
  void continuousScroll(LGraphicsView *view);

  void clearPLICache();
  void clearCSICache();
  void clearCSI3DCache();
  void clearImageModelCaches();
  void clearFadeCache();
  bool removeDir(int &count,const QString &dirName);

  void clearAndRedrawPage();

  void fileChanged(const QString &path);

  void processFadeColourParts();
  void loadFile(const QString &file);

signals:       

    /* tell the editor to display this file */

  void displayFileSig(LDrawFile *ldrawFile, const QString &subFile);
  void displayParmsFileSig(const QString &fileName);  
  void showLineSig(int lineNumber);
  void enable3DActionsSig();

  void halt3DViewerSig(bool b);

  // right side progress bar
 void progressBarInitSig();
 void progressMessageSig(const QString &text);
 void progressRangeSig(const int &min, const int &max);
 void progressSetValueSig(const int &value);
 void progressResetSig();
 void removeProgressStatusSig();

   // right side progress bar
  void progressBarPermInitSig();
  void progressPermMessageSig(const QString &text);
  void progressPermRangeSig(const int &min, const int &max);
  void progressPermSetValueSig(const int &value);
  void progressPermResetSig();
  void removeProgressPermStatusSig();

  void messageSig(bool  status, QString message);

  void requestEndThreadNowSig();
  void loadFileSig(const QString &file);

public:
  Page                  page;            // the abstract version of page contents

  // multi-thread worker classes
//  PartWorker            partWorkerLDSearchDirs; // part worker to process search directories and fade color parts
  PartWorker             partWorkerLdgLiteSearchDirs;      // part worker to process temp directory parts
  PartWorker            *partWorkerFadeColour;    // part worker to process colour part fade
  ColourPartListWorker  *colourPartListWorker;    // create static colour parts list in separate thread
  ParmsWindow           *parmsWindow;             // the parametrer file editor

private:    
  QGraphicsScene        *KpageScene;      // top of displayed page's graphics items
  LGraphicsView         *KpageView;       // the visual representation of the scene
  LDrawFile              ldrawFile;       // contains MPD or all files used in model
  QString                curFile;         // the file name for MPD, or top level file
  QElapsedTimer          timer;           // measure elapsed time for slow functions
  QString                curSubFile;      // whats being displayed in the edit window
  EditWindow            *editWindow;      // the sub file editable by the user
  QProgressBar          *progressBar;        // left side progress bar
  QProgressBar          *progressBarPerm;    // Right side progress bar
  QLabel                *progressLabel;
  QLabel                *progressLabelPerm;  //
  UpdateCheck           *libraryDownload;     // download request

  FadeStepColorParts     fadeStepColorParts; // internal list of color parts to be processed for fade step.
  PliSubstituteParts     pliSubstituteParts; // internal list of PLI/BOM substitute parts

#ifdef WATCHER
  QFileSystemWatcher watcher;      // watch the file system for external
                                   // changes to the ldraw files currently
                                   // being edited
#endif

  LDrawColor      ldrawColors;     // provides maps from ldraw color to RGB

  QUndoStack     *undoStack;       // the undo/redo stack
  int             macroNesting;

  void countPages();

  void skipHeader(Where &current);

  int findPage(                    // traverse the hierarchy until we get to the
    LGraphicsView  *view,          // page of interest, let traverse process the
    QGraphicsScene *scene,         // page, and then finish by counting the rest
    int           &pageNum,        // of the pages
    QString const &addLine,
    Where         &current,
    bool           mirrored,
    Meta           meta,
    bool           printing);

  int drawPage(// process the page of interest and any callouts
    LGraphicsView  *view,
    QGraphicsScene *scene,
    Steps          *steps,
    int            stepNum,
    QString const &addLine,
    Where         &current,
    QStringList   &csiParts,
    QStringList   &pliParts,
    bool           isMirrored,
    QHash<QString, QStringList> &bfx,
    bool           printing,
    bool           bfxStore2,
    QStringList   &bfxParts,
    QStringList   &ldrStepFiles,
    bool           supressRotateIcon = false,
    bool           calledOut = false);

  void attitudeAdjustment(); // reformat the LDraw file to fix LPub backward compatibility issues 
    
  void include(Meta &meta);

  int  createLDVieiwCsiImage(
            QPixmap            *pixmap,
            Meta               &meta);

  int addGraphicsPageItems(        // this converts the abstract page into
    Steps          *steps,         // a graphics view
    bool            coverPage,
    bool            modelDisplayPage,
    bool            endOfSubmodel,
    int             instances,
    LGraphicsView  *view,
    QGraphicsScene *scene,
    bool            printing);

  int getBOMParts(
    Where        current,
    QString     &addLine,
    QStringList &csiParts);

  int getBOMOccurrence(
          Where  current);

  void writeToTmp(
    const QString &fileName,
    const QStringList &);

  void writeToTmp();

  QStringList fadeSubFile(
     const QStringList &,
     const QString &color);      // fade all parts in subfile

  QStringList fadeStep(
     const QStringList &csiParts,
     const int &stepNum,
     Where        &current);      // fade parts in a step that are not current

  static bool installExportBanner(
    const int &type,
    const QString &printFile,
    const QString &imageFile);

private slots:
    void open();
    void save();
    void saveAs();

    void openRecentFile();
    void updateCheck();
    bool aboutDialog();

    bool printToPdfDialog();
    bool exportAsPngDialog();
    bool exportAsJpgDialog();
    bool exportAsBmpDialog();

    void editTitleAnnotations();
    void editFreeFormAnnitations();
    void editFadeColourParts();
    void editPliBomSubstituteParts();
    void editLdrawIniFile();
    void generateFadeColourPartsList();

    void toggleLCStatusBar();
    void showLCStatusMessage();

    // Begin Jaco's code

    void onlineManual();

    // End Jaco's code

    void meta();

    void redo();
    void undo();

    void insertCoverPage();
    void appendCoverPage();

    void insertNumberedPage();
    void appendNumberedPage();
    void deletePage();
    void addPicture();
    void addText();
    void addBom();
    void removeLPubFormatting();

    void nextPage();
    void prevPage();
    void setPage();
    void firstPage();
    void lastPage();
    void setGoToPage(int index);
    void loadPages();

    void zoomIn();
    void zoomOut();

    void zoomIn(LGraphicsView *view);
    void zoomOut(LGraphicsView *view);

    void GetPixelDimensions(float &, float &);
    void GetPagePixelDimensions(float &, float &, QPrinter::PaperSize &, QPrinter::Orientation &);

    void printToPdfFile();
    void exportAs(QString &);
    void exportAsPng();
    void exportAsJpg();
    void exportAsBmp();

    void closeEvent(QCloseEvent *event);

    void mpdComboChanged(int index);
    void refreshLDrawUnoffParts();
    void refreshLDrawOfficialParts();

    void clearPage(
      LGraphicsView  *view,
      QGraphicsScene *scene);
    
    void enableActions();
    void enableActions2();

    /******************************************************************
     * File management functions
     *****************************************************************/

    void setCurrentFile(const QString &fileName);
    void openFile(QString &fileName);
    bool maybeSave();
    bool saveFile(const QString &fileName);
    void closeFile();
    void updateRecentFileActions();

public:

  // Page size for pdf printing
  int bestPaperSizeOrientation(
    float widthMm,
    float heightMm,
    QPrinter::PaperSize &paperSize,
    QPrinter::Orientation &orientation);
	
private:
  /* Initialization stuff */

  void createActions();
  void createMenus();
  void createToolBars();
  void createStatusBar();
  void createDockWindows();
  void readSettings();
  void writeSettings();

  QDockWidget       *fileEditDockWindow; 
//** 3D
  QDockWidget       *modelDockWindow;
//**

  QMenu    *fileMenu;
  QMenu    *recentFileMenu;
  QMenu    *editMenu;
  QMenu    *viewMenu;
  QMenu    *toolsMenu;
  QMenu    *configMenu;
  QMenu    *helpMenu;

  QMenu    *cacheMenu;
  QMenu    *exportMenu;


  QToolBar *fileToolBar;
  QToolBar *editToolBar;
  QToolBar *zoomToolBar;
  QToolBar *navigationToolBar;
  QToolBar *mpdToolBar;
  QComboBox *mpdCombo;

  // file

  QAction  *openAct;
  QAction  *saveAct;
  QAction  *saveAsAct;
  QAction  *printToPdfFileAct;
//  QAction  *printToFileAct;
  QAction  *exportPngAct;
  QAction  *exportJpgAct;
  QAction  *exportBmpAct;
  QAction  *exitAct;

  QAction  *undoAct;
  QAction  *redoAct;
  QAction  *insertCoverPageAct;
  QAction  *appendCoverPageAct;
  QAction  *insertNumberedPageAct;
  QAction  *appendNumberedPageAct;
  QAction  *deletePageAct;
  QAction  *addPictureAct;
  QAction  *addTextAct;
  QAction  *addBomAct;
  QAction  *removeLPubFormattingAct;

  // view
  // zoom toolbar
    
  QAction  *fitWidthAct;
  QAction  *fitVisibleAct;
  QAction  *actualSizeAct;

  QAction  *zoomInAct;
  QAction  *zoomOutAct;

  // view
  // navigation toolbar

  QAction  *firstPageAct;
  QAction  *lastPageAct;
  QAction  *nextPageAct;
  QAction  *previousPageAct;
  QLineEdit*setPageLineEdit;
  QComboBox*setGoToPageCombo;

  // manage Caches
  QAction  *clearImageModelCacheAct;

  QAction  *clearPLICacheAct;
  QAction  *clearCSICacheAct;
  QAction  *clearCSI3DCacheAct;
  QAction  *clearFadeCacheAct;

  QAction  *refreshLDrawUnoffPartsAct;
  QAction  *refreshLDrawOfficialPartsAct;

  // config menu

  QAction *pageSetupAct;
  QAction *assemSetupAct;
  QAction *pliSetupAct;
  QAction *bomSetupAct;
  QAction *calloutSetupAct;
  QAction *multiStepSetupAct;
  QAction *projectSetupAct;
  QAction *fadeStepSetupAct;    

  QAction *preferencesAct;

  QAction *editFreeFormAnnitationsAct;
  QAction *editTitleAnnotationsAct;
  QAction *editFadeColourPartsAct;
  QAction *editPliBomSubstitutePartsAct;
  QAction *editLdrawIniFileAct;
  QAction *generateFadeColourPartsAct;

  // help

  QAction  *aboutAct;

  // Begin Jaco's code

  QAction  *onlineManualAct;

  // End Jaco's code

  QAction  *metaAct;
  QAction  *separatorAct;

  enum { MaxRecentFiles = 8 };
  QAction *recentFilesActs[MaxRecentFiles];

  QAction *updateApp;

  // capture camera rotation from LeoCad module
protected:
  lcVector3 mExistingRotStep;
  lcVector3 mModelStepRotation;
  float mRotStepAngleX;
  float mRotStepAngleY;
  float mRotStepAngleZ;

};

extern class Gui *gui;

class LGraphicsView : public QGraphicsView
{
public:
  //LGraphicsView();
  LGraphicsView(QGraphicsScene *scene)
  {
    setScene(scene);
    pageBackgroundItem = NULL;
  }
  //~LGraphicsView();

  PageBackgroundItem *pageBackgroundItem;

protected:
    
  void resizeEvent(QResizeEvent * /* unused */)
  {
    if (pageBackgroundItem) {
      if (gui->fitMode == FitVisible) {
        gui->fitVisible(gui->pageview());
      } else if (gui->fitMode == FitWidth) {
        gui->fitWidth(gui->pageview());
      }
    }
  }
};
//custom type definition and meta-type declaration
//Q_DECLARE_METATYPE(LGraphicsView);

enum zValues {
  PageBackgroundZValue = 0,
  PageNumberZValue = 10,
  PagePLIZValue = 20,
  PageInstanceZValue = 30,
  AssemZValue = 30,
  StepGroupZValue = 30,
  CalloutPointerZValue = 45,
  CalloutBackgroundZValue = 50,
  CalloutAssemZValue = 55,
  CalloutInstanceZValue = 60,
};

class GlobalFadeStep
{
private:
    LDrawFile   ldrawFile;       // contains MPD or all files used in model
public:
    Meta        meta;
    QString     topLevelFile;
    GlobalFadeStep()
    {
        meta = gui->page.meta;

        topLevelFile = ldrawFile.topLevelFile();
        MetaItem mi; // examine all the globals and then return
        mi.sortedGlobalWhere(meta,topLevelFile,"ZZZZZZZ");
    }
};

#endif
