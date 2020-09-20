/****************************************************************************  
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2020 Trevor SANDY. All rights reserved.
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
 *  LPub specific meta-commands come in two flavours:  configuration metas, and
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
 *   ranges.h - the internal representation for both multi-steps and callouts.
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
 *   Certain of these metas' values are changed via graphical dialogues. These
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
 *       fadeandhighlightstepglobals.cpp
 *       Local - values that are specified in something other than the first
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
#include <QMainWindow>
#include <QMenuBar>
#include <QAction>
#include <QToolBar>
#include <QStatusBar>
#include <QDockWidget>
#include <QSettings>
#include <QDateTime>
#include <QFileSystemWatcher>
#include <QtPrintSupport>
#include <QFile>
#include <QProgressBar>
#include <QElapsedTimer>
#include <QPdfWriter>

#include "lgraphicsview.h"
#include "lgraphicsscene.h"

#include "version.h"
#include "ranges.h"
#include "render.h"
#include "where.h"
#include "color.h"
#include "ldrawfiles.h"
#include "threadworkers.h"
#include "lpub_preferences.h"
#include "ldrawcolourparts.h"
#include "plisubstituteparts.h"
#include "QsLog.h"

//3D Viewer
#include "lc_global.h"
#include "lc_math.h"
#include "lc_library.h"
#include "lc_mainwindow.h"

// Set to enable file watcher
#ifndef WATCHER
#define WATCHER
#endif

// PageSize default entry
#ifndef DEF_SIZE
#define DEF_SIZE 0
#endif

class QString;
class QSplitter;
class QFrame;
class QFileDialog;
class QResizeEvent;
class QLineEdit;
class QComboBox;
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
class GlobalHighlightStep;
class UpdateCheck;

class LGraphicsView;
class LGraphicsScene;
class PageBackgroundItem;

class Pointer;
class PagePointer;

class lcHttpReply;
class lcHttpManager;

class ColourPartListWorker;
class DialogExportPages;
class FadeStepMeta;
class FadeStepMeta;
class HighlightStepMeta;
class LDrawColor;
class LDrawColourParts;
class LDrawFile;
class MetaItem;
class Page;
class PageMeta;
class PageNumberItem;
class PartWorker;
class PgSizeData;
class PgSizeData;
class Placement;
class PlacementFooter;
class PlacementHeader;
class PliPartGroupMeta;
class PliSubstituteParts;
class Preferences;
class ProgressDialog;
class Render;
class Steps;
class Where;
enum traverseRc { HitEndOfPage = 1 };
enum Dimensions {Pixels = 0, Inches };
enum PAction { SET_DEFAULT_ACTION, SET_STOP_ACTION };
enum Direction { PAGE_PREVIOUS, PAGE_NEXT, DIRECTION_NOT_SET };
enum ExportOption { EXPORT_ALL_PAGES, EXPORT_PAGE_RANGE, EXPORT_CURRENT_PAGE };
enum ExportMode { PRINT_FILE   = -2,//-2
                  EXPORT_NONE  = -1,//-1
                  PAGE_PROCESS = 0, // 0
                  EXPORT_PDF,       // 1
                  EXPORT_PNG,       // 2
                  EXPORT_JPG,       // 3
                  EXPORT_BMP,       // 4
                  EXPORT_3DS_MAX,   // 5
                  EXPORT_COLLADA,   // 6
                  EXPORT_WAVEFRONT, // 7
                  EXPORT_STL,       // 8
                  EXPORT_POVRAY,    // 9
                  EXPORT_BRICKLINK, // 10
                  EXPORT_CSV,       // 11
                  EXPORT_ELEMENT,   // 12
                  EXPORT_HTML,      // 13
                  POVRAY_RENDER};   // 14

const QString nativeExportNames[] =
{
  "",              // PAGE_PROCESS
  "PDF",           // EXPORT_PDF
  "PNG",           // EXPORT_PNG
  "JPG",           // EXPORT_JPG
  "BMP",           // EXPORT_BMP
  "3DS MAX",       // EXPORT_3DS_MAX
  "COLLADA",       // EXPORT_COLLADA
  "WAVEFRONT",     // EXPORT_WAVEFRONT
  "STL",           // EXPORT_STL
  "POVRAY",        // EXPORT_POVRAY
  "BRICKLINK",     // EXPORT_BRICKLINK
  "CSV",           // EXPORT_CSV
  "ELEMENT",       // EXPORT_ELEMENT
  "HTML",          // EXPORT_HTML
  "POV-RAY RENDER" // RENDER_POVRAY
};

class Gui : public QMainWindow
{

  Q_OBJECT

public:
  Gui();
  ~Gui();

  int             displayPageNum;  // what page are we displaying
  int             stepPageNum;     // the number displayed on the page
  int             saveStepPageNum;
  int             saveContStepNum; // saved continuous step number for steps before displayed page, exited submodels and step group end
  int             firstStepPageNum;
  int             lastStepPageNum;
  int             savePrevStepPosition; // indicate the previous step position amongst current and previous steps.
  QList<Where>    topOfPages;
  QList<Where>    parsedMessages;       // previously parsed messages
  QVector<int>    buildModRange;    // begin and end range of modified parts from 3DViewer

  int             boms;            // the number of pli BOMs in the document
  int             bomOccurrence;   // the actual occurrence of each pli BOM

  int             exportMode;       // export Mode
  int             processOption;    // export Option
  int             pageDirection;    // continuous page processing direction
  qreal           exportPixelRatio; // export resolution pixel density
  QString         pageRangeText;    // page range parameters
  bool            submodelIconsLoaded; // load submodel images
  bool            resetCache;       // reset model, fade and highlight parts
  QString         saveFileName;      // user specified output file Name [commandline only]
  QString         saveDirectoryName; // user specified output directory name [commandline only]

  bool             m_previewDialog;
  ProgressDialog  *m_progressDialog; // general use progress dialogue
  QLabel          *m_progressDlgMessageLbl;
  QProgressBar    *m_progressDlgProgressBar;

  bool             m_partListCSIFile;   // processing part list CSI file

  void            *noData;

  FadeStepMeta      *fadeStepMeta;      // propagate fade step settings

  HighlightStepMeta *highlightStepMeta; // propagate highlight step settings

  FitMode          fitMode;             // how to fit the scene into the view

  Where &topOfPage();
  Where &bottomOfPage();

  static int pageSize(PageMeta  &, int which);          // Flip page size per orientation and return size in pixels

  void    changePageNum(int offset)
  {
    displayPageNum += offset;
  }
  void  displayPage();

  bool continuousPageDialog(Direction d);

  /* We need to send ourselves these, to eliminate recursion and the model
   * changing under foot */
  void drawPage(                   // this is the workhorse for preparing a
    LGraphicsView *view,           // page for viewing.  It depends heavily
    LGraphicsScene *scene,         // on the next two functions
    bool            printing);

  /*--------------------------------------------------------------------*
   * These are the work horses for back annotating user changes into    *
   * the LDraw files                                                    *
   *--------------------------------------------------------------------*/

  QStringList fileList()
  {
    return ldrawFile.subFileOrder();
  }
  bool changedSinceLastWrite(QString &modelName)
  {
      return ldrawFile.changedSinceLastWrite(modelName);
  }
  int subFileSize(const QString &modelName)
  {
    return ldrawFile.size(modelName);
  }
  int numSteps(const QString &modelName)
  {
    return ldrawFile.numSteps(modelName);
  }
  int numParts(){
    return ldrawFile.getPartCount();
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
  bool isOlder(const QString &fileName,const QDateTime &lastModified)
  {
    bool older = ldrawFile.older(fileName,lastModified);
    return older;
  }
  bool isOlder(const QStringList &parsedStack,const QDateTime &lastModified)
  {
    bool older = ldrawFile.older(parsedStack,lastModified);
    return older;
  }
  bool isMirrored(QStringList &argv)
  {
    return ldrawFile.mirrored(argv);
  }
  bool isUnofficialPart(const QString &name)
  {
    return ldrawFile.isUnofficialPart(name) == UNOFFICIAL_PART;
  }
  bool isUnofficialSubPart(const QString &name)
  {
    return ldrawFile.isUnofficialPart(name) == UNOFFICIAL_SUBPART;
  }
  bool loadAborted()
  {
    return ldrawFile._loadAborted;
  }

  void insertGeneratedModel(const QString &name,
                                  QStringList &csiParts) {
    QDateTime date;
    ldrawFile.insert(name,csiParts,date,false,true);
    writeToTmp();
  }

  void clearPrevStepPositions()
  {
    ldrawFile.clearPrevStepPositions();
  }

  LDrawFile getLDrawFile()
  {
      return ldrawFile;
  }

  QString getSubmodelName(int index)
  {
      return ldrawFile.getSubmodelName(index);
  }

  int getLineTypeRelativeIndex(int submodelIndx, int lineTypeIndx)
  {
      return ldrawFile.getLineTypeRelativeIndex(submodelIndx,lineTypeIndx);
  }

  int getLineTypeIndex(int submodelIndx, int lineTypeIndx)
  {
      return ldrawFile.getLineTypeIndex(submodelIndx,lineTypeIndx);
  }

  int getSubmodelIndex(const QString &fileName)
  {
      return ldrawFile.getSubmodelIndex(fileName);
  }

  int getSubmodelInstances(const QString &fileName, bool isMirrored)
  {
      return ldrawFile.instances(fileName, isMirrored);
  }

  bool ldcadGroupMatch(const QString &name, const QStringList &lids)
  {
      return ldrawFile.ldcadGroupMatch(name,lids);
  }

  void updateViewerStep(const QString     &stepKey,
                        const QStringList &contents)
  {
      ldrawFile.updateViewerStep(stepKey, contents);
  }

  void insertViewerStep(const QString     &stepKey,
                        const QStringList &rotatedContents,
                        const QStringList &unrotatedContents,
                        const QString     &filePath,
                        const QString     &csiKey,
                        bool               multiStep,
                        bool               calledOut)
  {
      ldrawFile.insertViewerStep(stepKey,
                                 rotatedContents,
                                 unrotatedContents,
                                 filePath,
                                 csiKey,
                                 multiStep,
                                 calledOut);
  }

  QStringList getViewerStepRotatedContents(const QString &stepKey)
  {
      return ldrawFile.getViewerStepRotatedContents(stepKey);
  }

  QStringList getViewerStepUnrotatedContents(const QString &stepKey)
  {
      return ldrawFile.getViewerStepUnrotatedContents(stepKey);
  }

  QString getViewerStepFilePath(const QString &stepKey)
  {
      return ldrawFile.getViewerStepFilePath(stepKey);
  }

  QString getViewerConfigKey(const QString &stepKey)
  {
      return ldrawFile.getViewerConfigKey(stepKey);
  }

  bool isViewerStepMultiStep(const QString &stepKey)
  {
      return ldrawFile.isViewerStepMultiStep(stepKey);
  }

  bool isViewerStepCalledOut(const QString &stepKey)
  {
      return ldrawFile.isViewerStepCalledOut(stepKey);
  }

  bool viewerStepContentExist(const QString &stepKey)
  {
      return ldrawFile.viewerStepContentExist(stepKey);
  }

  void clearViewerSteps(){
      ldrawFile.clearViewerSteps();
  }

  /* Build Modifications */
  void insertBuildMod(const QString      &buildModKey,
                      const QVector<int> &modAttributes,
                      int                 modAction,
                      int                 stepNumber)
  {
      ldrawFile.insertBuildMod(buildModKey,
                               modAttributes,
                               modAction,
                               stepNumber);
  }

  int getBuildModBeginLineNumber(const QString &buildModKey)
  {
      return ldrawFile.getBuildModBeginLineNumber(buildModKey);
  }

  int getBuildModActionLineNumber(const QString &buildModKey)
  {
      return ldrawFile.getBuildModActionLineNumber(buildModKey);
  }

  int getBuildModEndLineNumber(const QString &buildModKey)
  {
      return ldrawFile.getBuildModEndLineNumber(buildModKey);
  }

  int getBuildModStepNumber(const QString &buildModKey)
  {
      return ldrawFile.getBuildModStepNumber(buildModKey);
  }

  int getBuildModAction(const QString &buildModKey, int stepNumber)
  {
      int rc = ldrawFile.getBuildModAction(buildModKey, stepNumber);

      if (rc == OkRc) {
          rc = BuildModApplyRc;
          ldrawFile.setBuildModAction(buildModKey, stepNumber, rc);
      }

      return rc;
  }

  void setBuildModAction(const QString      &buildModKey,
                         int                 stepNumber,
                         int                 modAction)
  {
      ldrawFile.setBuildModAction(buildModKey, stepNumber, modAction);
  }

  bool hasBuildMods(){
      return ldrawFile.hasBuildMods();
  }

  QStringList getBuildModsList()
  {
      return ldrawFile.getBuildModsList();
  }

  int getBuildModNextIndex(const QString &buildModKeyPrefix)
  {
      return ldrawFile.getBuildModNextIndex(buildModKeyPrefix);
  }

  QString getBuildModModelName(const QString &buildModKey)
  {
      return ldrawFile.getBuildModModelName(buildModKey);
  }

  bool buildModContains(const QString &buildModKey)
  {
      return ldrawFile.buildModContains(buildModKey);
  }

  bool suppressColourMeta()
  {
    return false; //Preferences::usingNativeRenderer;
  }

  void insertLine (const Where &here, const QString &line, QUndoCommand *parent = nullptr);
  void appendLine (const Where &here, const QString &line, QUndoCommand *parent = nullptr);
  void replaceLine(const Where &here, const QString &line, QUndoCommand *parent = nullptr);
  void deleteLine (const Where &here, QUndoCommand *parent = nullptr);
  void normalizeHeader(const Where &here);
  void scanPast(Where &here, const QRegExp &lineRx);
  bool stepContains(Where &here, QRegExp &lineRx, QString &result, int capGrp = 0);
  bool stepContains(Where &here, QRegExp &lineRx);

  QString topLevelFile();

  void beginMacro (QString name);
  void endMacro   ();

  void getRequireds();
  void initialize();

  void displayFile(LDrawFile *ldrawFile, const QString &modelName, bool editModelFile = false, bool displayStartPage = false);
  void displayParmsFile(const QString &fileName);
  QString elapsedTime(const qint64 &duration);

  int             maxPages;
  
  LGraphicsView *pageview()
  {
      return KpageView;
  }

  LGraphicsScene *pagescene()
  {
      return KpageScene;
  }

  void setCurrentStep();
  void setCurrentStep(Step *step, Where here, int stepNumber);

  Step *getCurrentStep()
  {
      return currentStep;
  }

  bool getSelectedLine(int modelIndex, int lineIndex, int source, int &lineNumber);

  QString getCurFile()
  {
      return curFile;
  }

  bool extractStepKey(Where &here, int &stepNumber);

  /***********************************************************************
   * set Native renderer for fast processing
   **********************************************************************/

  void setPreferredRenderer(const QString &renderer = QString()){
      if (!Preferences::usingNativeRenderer || !renderer.isEmpty()) {
          saveRenderer   = Preferences::preferredRenderer;
          saveProjection = Preferences::perspectiveProjection;
          saveSingleCall = Preferences::enableLDViewSingleCall;
          if (renderer.isEmpty()) {
             Preferences::preferredRenderer   = RENDERER_NATIVE;
             Preferences::usingNativeRenderer = true;
          } else {
             Preferences::preferredRenderer   = renderer;
          }
          Preferences::perspectiveProjection  = true;
          Preferences::enableLDViewSingleCall = false;
          Render::setRenderer(Preferences::preferredRenderer);
      }
  }

  void restorePreferredRenderer(){
      if (!saveRenderer.isEmpty()) {
          Preferences::preferredRenderer      = saveRenderer;
          Preferences::perspectiveProjection  = saveProjection;
          Preferences::enableLDViewSingleCall = saveSingleCall;
          Preferences::usingNativeRenderer    = saveRenderer == RENDERER_NATIVE;
          Render::setRenderer(Preferences::preferredRenderer);
          saveRenderer = QString();
      }
  }

  float getDefaultCameraFoV(){
      return (Preferences::usingNativeRenderer ?
                  CAMERA_FOV_NATIVE_DEFAULT :
                  Preferences::preferredRenderer == RENDERER_LDVIEW && Preferences::perspectiveProjection ?
                  CAMERA_FOV_LDVIEW_P_DEFAULT :
                  CAMERA_FOV_DEFAULT);
  }

  float getDefaultFOVMinRange()
  {
      return (Preferences::usingNativeRenderer ?
                  CAMERA_FOV_NATIVE_MIN_DEFAULT :
                  CAMERA_FOV_MIN_DEFAULT);
  }

  float getDefaultFOVMaxRange()
  {
      return (Preferences::usingNativeRenderer ?
                  CAMERA_FOV_NATIVE_MAX_DEFAULT :
                  Preferences::preferredRenderer == RENDERER_LDVIEW && Preferences::perspectiveProjection ?
                  CAMERA_FOV_LDVIEW_P_MAX_DEFAULT :
                  CAMERA_FOV_MAX_DEFAULT);
  }

  float getDefaultCameraZNear(){
      return (Preferences::usingNativeRenderer ?
                  CAMERA_ZNEAR_NATIVE_DEFAULT :
                  CAMERA_ZNEAR_DEFAULT);
  }

  float getDefaultCameraZFar(){
      return (Preferences::usingNativeRenderer ?
                  CAMERA_ZFAR_NATIVE_DEFAULT :
                  CAMERA_ZFAR_DEFAULT);
  }

  bool compareVersionStr(const QString &first, const QString &second);

  // download components
  void downloadFile(QString URL, QString title);
  QByteArray getDownloadedFile() const
  {
      return mByteArray;
  }

  void createOpenWithActions(int maxPrograms = 0);

public slots:
  //**3D Viewer Manage Step Rotation
  void Disable3DActions();
  void Enable3DActions();

  void ShowStepRotationStatus();
  void SetRotStepMeta();
  void SetActiveModel(const QString &fileName,bool newSubmodel);
  void SelectedPartLines(QVector<TypeLine> &indexes, PartSource source);
  QStringList getViewerStepKeys(bool modelName = true, bool pliPart = false);

  void setViewerStepKey(const QString &stepKey, int notPliPart)
  {
      viewerStepKey = stepKey;
      currentStep   = nullptr;
      buildModRange = { 0, -1, 0 };
      if (notPliPart)
          setCurrentStep();
  }

  QString getViewerStepKey()
  {
      return viewerStepKey;
  }

  QString GetPliIconsPath(QString& key)
  {
      if (mPliIconsPath.contains(key))
          return mPliIconsPath.value(key);

      return QString();
  }

  void setPliIconPath(QString& key, QString& value)
  {
      if (!mPliIconsPath.contains(key)) {
          mPliIconsPath.insert(key,value);
          emit messageSig(LOG_NOTICE, QString("Icon Inserted: Key [%1] Value [%2]")
                                              .arg(key).arg(value));
          return;
      }
//      emit messageSig(LOG_DEBUG, QString("Icon Exist (Insert Ignored): Key [%1] Value [%2]")
//                                         .arg(key).arg(value));
  }

  lcVector3 GetRotStepMeta() const
  {
      return mStepRotation;
  }

  void SetRotStepAngleX(float AngleX, bool display)
  {
      mRotStepAngleX = AngleX;
      if (display)
          ShowStepRotationStatus();
  }

  void SetRotStepAngleY(float AngleY, bool display)
  {
      mRotStepAngleY = AngleY;
      if (display)
          ShowStepRotationStatus();
  }

  void SetRotStepAngleZ(float AngleZ, bool display)
  {
      mRotStepAngleZ = AngleZ;
      if (display)
          ShowStepRotationStatus();
  }

  void SetRotStepTransform(QString& Transform, bool display)
  {
      mRotStepTransform = Transform;
      if (display)
          ShowStepRotationStatus();
  }

  //**

  void loadBLCodes();

  void loadLastOpenedFile();

  /* The undoStack needs access to these */

  void canRedoChanged(bool);
  void canUndoChanged(bool);
  void cleanChanged(bool);

  /* The edit window sends us these */

  void contentsChange(const QString &fileName,int position, int charsRemoved, const QString &charsAdded);

  void showLineMessage(QString message, Where &here, Preferences::MsgKey = Preferences::ParseErrors);
  void parseError(QString errorMsg, Where &here, Preferences::MsgKey key = Preferences::ParseErrors)
  {
      showLineMessage(errorMsg,here,key);
  }

  void statusMessage(LogType logType, QString message);
  void statusBarMsg(QString msg);

  void showPrintedFile();
  void showLine(const Where &topOfStep)
  {
    if (! exporting()) {
        displayFile(&ldrawFile,topOfStep.modelName);
        showLineSig(topOfStep.lineNumber);
      }
  }

  /* Fade color processing */
  QString createColourEntry(
    const QString &colourCode,
    const PartType partType);

  bool colourEntryExist(
    const QStringList &colourEntries,
    const QString &code,
    const PartType partType);

  void openDropFile(QString &fileName);

  void deployExportBanner(bool b);
  void setExporting(bool b){ m_exportingContent = b; if (!b){ m_exportingObjects = b; } }
  void setExportingObjects(bool b){ m_exportingContent = m_exportingObjects = b; }
  bool exporting() { return m_exportingContent; }
  bool updateViewer() { return m_updateViewer; }
  bool exportingImages() { return m_exportingContent && !m_exportingObjects; }
  bool exportingObjects() { return m_exportingContent && m_exportingObjects; }
  void cancelExporting(){ m_exportingContent = m_exportingObjects = false; }

  void setContinuousPageAct(PAction p = SET_DEFAULT_ACTION);
  void setPageContinuousIsRunning(bool b = true, Direction d = DIRECTION_NOT_SET);
  void setContinuousPage(bool b){ m_contPageProcessing = b; }
  bool ContinuousPage() { return m_contPageProcessing; }
  void cancelContinuousPage(){ m_contPageProcessing = false; }

  // left side progress bar
  void progressBarInit();
  void progressBarSetText(const QString &progressText);
  void progressBarSetRange(int minimum, int maximum);
  void progressBarSetValue(int value);
  void progressBarReset();
  void progressStatusRemove();

  // right side progress bar
  void progressBarPermInit();
  void progressBarPermSetText(const QString &progressText);
  void progressBarPermSetRange(int minimum, int maximum);
  void progressBarPermSetValue(int value);
  void progressBarPermReset();
  void progressPermStatusRemove();

  void workerJobResult(int value);

  void preferences();
  void fadeStepSetup();
  void highlightStepSetup();
  void generateCoverPages();
  void insertFinalModel();
  void ldrawColorPartsLoad();

  void pageSetup();
  void assemSetup();
  void pliSetup();
  void bomSetup();
  void calloutSetup();
  void multiStepSetup();
  void subModelSetup();
  void projectSetup();

  void fitWidth();
  void fitVisible();
  void fitScene();
  void actualSize();
  void zoomIn();
  void zoomOut();
  void zoomSlider(int);
  void ViewerZoomSlider(int);
  void ResetViewerZoomSlider();
  void viewResolution();
  void sceneGuides();
  void sceneGuidesLine();
  void sceneGuidesPosition();
  void sceneRuler();
  void sceneRulerTracking();
  void snapToGrid();
  void hidePageBackground();
  void showCoordinates();
  void gridSize(int index);
  void gridSizeTriggered();
  void setTargetPosition();
  void enableBuildModification();
  void useImageSize();

  void showDefaultCameraProperties();
  void applyCameraSettings();
  void applyBuildModification();
  void removeBuildModification();
  void createBuildModification();

  void clearPLICache();
  void clearCSICache();
  void clearTempCache();
  void clearSubmodelCache(const QString &key = QString());
  void clearAllCaches();
  void clearCustomPartCache(bool silent = false);
  void clearStepCSICache(QString &pngName);
  void clearPageCSICache(PlacementType relativeType, Page *page);
  void clearPageCSIGraphicsItems(Step *step);
  void clearAndReloadModelFile();
  void clearAndRedrawModelFile();
  void reloadCurrentModelFile();
  void reloadModelFileAfterColorFileGen();
  void reloadCurrentPage();
  void reloadViewer();
  void loadTheme(bool restart = true);
  void restartApplication(bool changeLibrary = false);
  void resetModelCache(QString file = QString());

  bool removeDir(int &count,const QString &dirName);

  void fileChanged(const QString &path);

  void processFadeColourParts(bool overwriteCustomParts);
  void processHighlightColourParts(bool overwriteCustomParts);
  void loadLDSearchDirParts();
  bool loadFile(const QString &file);
  int processCommandLine();


  void showPovrayRenderDialog();

  void TogglePdfExportPreview();
  void TogglePrintToFilePreview();
  void TogglePrintPreview(ExportMode m);

  void DownloadFinished(lcHttpReply* Reply);

  void editModelFile(bool saveBefore);

signals:       

  /* tell the editor to display this file */

  void displayFileSig(LDrawFile *ldrawFile, const QString &subFile);
  void displayModelFileSig(LDrawFile *ldrawFile, const QString &subFile);
  void displayParmsFileSig(const QString &fileName);
  void highlightSelectedLinesSig(QVector<int> &indexes);
  void setSelectedPiecesSig(QVector<int> &indexes);
  void showLineSig(int lineNumber);
  void disableEditorActionsSig();

  void enable3DActionsSig();
  void disable3DActionsSig();
  void updateAllViewsSig();
  void clearViewerWindowSig();
  void setExportingSig(bool);
  void setExportingObjectsSig(bool);
  void setContinuousPageSig(bool);
  void hidePreviewDialogSig();
  void showPrintedFileSig(int);

  // right side progress bar
  void progressBarInitSig();
  void progressMessageSig(const QString &text);
  void progressRangeSig(const int &min, const int &max);
  void progressSetValueSig(const int &value);
  void progressResetSig();
  void progressStatusRemoveSig();

  // right side progress bar
  void progressBarPermInitSig();
  void progressPermMessageSig(const QString &text);
  void progressPermRangeSig(const int &min, const int &max);
  void progressPermSetValueSig(const int &value);
  void progressPermResetSig();
  void progressPermStatusRemoveSig();

  void messageSig(LogType logType, QString message);

  void requestEndThreadNowSig();
  void loadFileSig(const QString &file);
  void processCommandLineSig();

  void operateHighlightParts(bool overwriteCustomParts);
  void operateFadeParts(bool overwriteCustomParts);
  void setPliIconPathSig(QString &,QString &);

public:
  Page                  page;                         // the abstract version of page contents

  // multi-thread worker classes
  PartWorker             partWorkerLDSearchDirs;      // part worker to process search directories and fade and or highlight color parts
  PartWorker             partWorkerLdgLiteSearchDirs; // part worker to process temp directory parts
  PartWorker            *partWorkerCustomColour;      // part worker to process color part fade and or highlight
  ColourPartListWorker  *colourPartListWorker;        // create static color parts list in separate thread
  ParmsWindow           *parmsWindow;                 // the parameter file editor
  Preferences            lpub3dPreferences;           // lpub3D Preferences
  LDrawColourParts       ldrawColourParts;            // load the LDraw color parts list

protected:
  // capture camera rotation from 3DViewer module
  lcVector3              mStepRotation;
  float                  mRotStepAngleX;
  float                  mRotStepAngleY;
  float                  mRotStepAngleZ;
  QString                mRotStepTransform;
  QString                viewerStepKey;        // currently loaded CSI in 3DViewer
  QMap<QString, QString> mPliIconsPath;       // used to set an icon image in the 3DViewer timeline view

  QMap<int, PgSizeData>  pageSizes;           // page size and orientation object

  int                    mViewerZoomLevel;
  // download components
  lcHttpManager*         mHttpManager;
  lcHttpReply*           mHttpReply;
  QByteArray             mByteArray;
  QString                mTitle;
  SceneObject            selectedItemObj;

private:
  LGraphicsScene        *KpageScene;         // top of displayed page's graphics items
  LGraphicsView         *KpageView;          // the visual representation of the scene
  LDrawFile              ldrawFile;          // contains MPD or all files used in model
  QString                curFile;            // the file name for MPD, or top level file
  QString                pdfPrintedFile;     // the print preview produced pdf file
  QElapsedTimer          timer;              // measure elapsed time for slow functions
  QString                curSubFile;         // whats being displayed in the edit window
  EditWindow            *editWindow;         // the sub file editable by the user
  EditWindow            *editModeWindow;     // the model file editable by the user in
  QProgressBar          *progressBar;        // left side progress bar
  QProgressBar          *progressBarPerm;    // Right side progress bar
  QLabel                *progressLabel;
  QLabel                *progressLabelPerm;  //
  Step                  *currentStep;        // the current step as loaded in the 3DViewer

  PliSubstituteParts     pliSubstituteParts; // internal list of PLI/BOM substitute parts

  bool                   m_exportingContent; // indicate export/printing underway
  bool                   m_exportingObjects; // indicate exporting non-image object file content
  bool                   m_contPageProcessing;// indicate continuous page processing underway

  QString                saveRenderer;
  bool                   saveProjection;
  bool                   saveSingleCall;

  bool                   m_updateViewer;

  int                    m_workerJobResult;

  int                    numPrograms;

  bool                   okToInvokeProgressBar()
  {
    return               (Preferences::lpub3dLoaded && Preferences::modeGUI);
  }

#ifdef WATCHER
  QFileSystemWatcher watcher;                // watch the file system for external
                                             // changes to the ldraw files currently
                                             // being edited
  bool               changeAccepted;         // don't throw another message unless existing was accepted
#endif

  LDrawColor      ldrawColors;               // provides maps from ldraw color to RGB

  QUndoStack     *undoStack;                 // the undo/redo stack
  int             macroNesting;
  bool            previousPageContinuousIsRunning;// stop the continuous previous page action
  bool            nextPageContinuousIsRunning;    // stop the continuous next page action

  bool isUserSceneObject(const int so);

  void countPages();

  void skipHeader(Where &current);

  int findPage(                     // traverse the hierarchy until we get to the
    LGraphicsView   *view,          // page of interest, let traverse process the
    LGraphicsScene  *scene,         // page, and then finish by counting the rest
    Meta             meta,
    QString const   &addLine,
    FindPageOptions &opts
    /*
    int            &pageNum,        //maxPages
    QString const  &addLine,
    Where          &current,
    PgSizeData     &pageSize,
    bool            isMirrored,
    Meta            meta,
    bool            printing,
    int             contStepNumber,
    int             renderStepNumber = 0,
    QString         renderParentModel = ""
    */);

  int drawPage(// process the page of interest and any callouts
    LGraphicsView  *view,
    LGraphicsScene *scene,
    Steps          *steps,
    QString const   &addLine,
    DrawPageOptions &opts
    /*
    int            stepNum,
    QString const &addLine,
    Where         &current,
    QStringList   &csiParts,
    QStringList   &pliParts,
    bool           isMirrored,
    QHash<QString, QStringList> &bfx,
    QList<PliPartGroupMeta> &pliPartGroups,
    bool           printing,
    bool           bfxStore2,
    QStringList   &bfxParts,
    QStringList   &ldrStepFiles,
    QStringList   &csiKeys,
    bool           assembledCallout = false,
    bool           calledOut = false
    */);

  void attitudeAdjustment(); // reformat the LDraw file to fix LPub backward compatibility issues 
    
  void include(Meta &meta);

  int  createLDVieiwCsiImage(
            QPixmap            *pixmap,
            Meta               &meta);

  int addStepImageGraphics(Step    *step); //recurse the step's model - including callouts to add images.

  int addStepPliPartGroupsToScene(Step *step,LGraphicsScene *scene);  //recurse the step's PLI - including callout PLIs to add PartGroups

  int addPliPartGroupsToScene(
          Page           *page,
          LGraphicsScene *scene);

  int addGraphicsPageItems(        // this converts the abstract page into
    Steps          *steps,         // a graphics view
    bool            coverPage,
    bool            endOfSubmodel,
    LGraphicsView  *view,
    LGraphicsScene *scene,
    bool            printing);

  int addContentPageAttributes(
    Page                *page,
    PageBackgroundItem  *pageBg,
    PlacementHeader     *pageHeader,
    PlacementFooter     *pageFooter,
    PageNumberItem      *pageNumber,
    Placement           &plPage,
    bool                 endOfSubmodel = false);

  int addPliPerPageItems(
    Page                *page,
    PlacementHeader     *pageHeader,
    PlacementFooter     *pageFooter,
    PageNumberItem      *pageNumber,
    Placement           &plPage);

  QString getFilePath(const QString &fileName) const;

  int addCoverPageAttributes(
    Page                *page,
    PageBackgroundItem  *pageBg,
    PlacementHeader     *pageHeader,
    PlacementFooter     *pageFooter,
    Placement           &plPage);

  int getBOMParts(
    Where                    current,
    QString                 &addLine,
    QStringList             &csiParts,
    QList<PliPartGroupMeta> &bomPartGroups);

  int getBOMOccurrence(
          Where  current);

  bool generateBOMPartsFile(
          const QString &);

  void writeToTmp(
    const QString &fileName,
    const QStringList &);

  void writeToTmp();

  void addCsiTypeLine();

  QStringList configureModelSubFile(
    const QStringList &,
    const QString &,
    const PartType partType);         // fade and or highlight all parts in subfile

  QStringList configureModelStep(
    const QStringList &csiParts,
    const int         &stepNum,
    Where             &current);      // fade and or highlight parts in a step that are not current

  static bool installExportBanner(
    const int &type,
    const QString &printFile,
    const QString &imageFile);

  void openFolder(const QString &folder);
  bool processPageRange(const QString &range);

  void setSceneItemZValue(Page *page, LGraphicsScene *scene);
  void setSceneItemZValue(SceneObjectDirection direction);
  bool getSceneObjectWhere(QGraphicsItem *selectedItem, Where &itemTop);
  bool getSceneObjectStep(QGraphicsItem *selectedItem, int &stepNumber);
  bool getSceneObject(QGraphicsItem *selectedItem, Where &itemTop, int &stepNumber);

private slots:
    void open();
    void openWith();
    void save();
    void saveAs();
    void saveCopy();

    void bringToFront();
    void sendToBack();

    void openWithSetup();
    void openWorkingFolder();
    void openRecentFile();
    void clearRecentFiles();
    void updateCheck();
    void aboutDialog();

    void editModelFile();
    void refreshModelFile();
    void editTitleAnnotations();
    void editFreeFormAnnitations();
    void editLDrawColourParts();
    void editPliBomSubstituteParts();
    void editLdrawIniFile();
    void editLPub3DIniFile();
    void editExcludedParts();
    void editLdgliteIni();
    void editNativePovIni();
    void editLdviewIni();
    void editLdviewPovIni();
    void editPovrayIni();
    void editPovrayConf();
    void editLD2BLCodesXRef();
    void editLD2BLColorsXRef();
    void editLD2RBCodesXRef();
    void editLD2RBColorsXRef();
    void editAnnotationStyle();
    void editBLColors();
    void editBLCodes();
    void generateCustomColourPartsList(bool prompt = true);
    void viewLog();

    void toggleLCStatusBar(bool);
    void showLCStatusMessage();

    void loadUpdatedImages();

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
    void nextPageContinuous();
    void previousPage();
    void previousPageContinuous();
    void setPage();
    void firstPage();
    void lastPage();
    void setGoToPage(int index);
    void loadPages();

    void getExportPageSize(float &, float &, int d = Pixels);
    bool validatePageRange();

    void ShowPrintDialog();
    void Print(QPrinter* Printer);

    void exportAs(const QString &);
    void exportAsHtml();
    void exportAsCsv();
    void exportAsBricklinkXML();
    void exportAsPdf();
    bool exportAsDialog(ExportMode m);
    void exportAsPdfDialog();
    void exportAsPngDialog();
    void exportAsJpgDialog();
    void exportAsBmpDialog();
    void exportAsStlDialog();
    void exportAsPovDialog();
    void exportAs3dsDialog();
    void exportAsColladaDialog();
    void exportAsObjDialog();

    OrientationEnc getPageOrientation(bool nextPage = false);
    QPageLayout getPageLayout(bool nextPage = false);
    void checkMixedPageSizeStatus();

    void closeEvent(QCloseEvent *event);

    void mpdComboChanged(int index);
    void refreshLDrawUnoffParts();
    void refreshLDrawOfficialParts();
    void writeGeneratedColorPartsToTemp();

    void clearPage(
      LGraphicsView  *view,
      LGraphicsScene *scene,
      bool clearViewPageBg = false);
    
    void enableActions();
    void enableActions2();
    void disableActions();
    void disableActions2();

    void partsWidgetVisibilityChanged(bool);
    void coloursWidgetVisibilityChanged(bool);

    void exportToolBarVisibilityChanged(bool);
    void cacheToolBarVisibilityChanged(bool);
    void setupToolBarVisibilityChanged(bool);
    void editToolBarVisibilityChanged(bool);
    void editParamsToolBarVisibilityChanged(bool);
    void setFadeStepsFromCommandMeta();
    void setHighlightStepFromCommandMeta();

    /******************************************************************
     * File management functions
     *****************************************************************/

    void setCurrentFile(const QString &fileName);
    bool openFile(QString &fileName);
    bool maybeSave(bool prompt = true, int sender = SaveOnNone);
    bool saveFile(const QString &fileName);
    void closeFile();
    void updateOpenWithActions();
    void updateRecentFileActions();
    void closeModelFile();
    void enableWatcher();
    void disableWatcher();

private:
  /* Initialization stuff */

  void createActions();
  void create3DActions();
  void createMenus();
  void create3DMenus();
  void createToolBars();
  void create3DToolBars();
  void createStatusBar();
  void createDockWindows();
  void create3DDockWindows();
  void readSettings();
  void writeSettings();

  QDockWidget       *fileEditDockWindow; 
//** 3D
  QDockWidget       *viewerDockWindow;
//**

  // Menus
  QMenu    *fileMenu;
  QMenu    *openWithMenu;
  QMenu    *recentFileMenu;
  QMenu    *editMenu;
  QMenu    *viewMenu;
  QMenu    *toolsMenu;
  QMenu    *configMenu;
  QMenu    *helpMenu;

  QMenu    *editorMenu;
  QMenu    *cacheMenu;
  QMenu    *exportMenu;
  QMenu    *recentMenu;
  QMenu    *setupMenu;

  QMenu    *nextPageContinuousMenu;
  QMenu    *previousPageContinuousMenu;
  QMenu    *snapToGridMenu;
  QMenu    *sceneRulerTrackingMenu;
  QMenu    *zoomSliderMenu;
  QMenu    *sceneGuidesMenu;

  QMenu    *cameraMenu;
  QMenu    *buildModMenu;
  QMenu    *rotateActionMenu;

  // 3D Viewer Menus
  QMenu* ViewerMenu;
  QMenu* FileMenuViewer;
  QMenu* ViewerExportMenu;
  QMenu* ViewerZoomSliderMenu;

  // toolbars
  QToolBar *fileToolBar;
  QToolBar *undoredoToolBar;
  QToolBar *exportToolBar;
  QToolBar *editToolBar;
  QToolBar *cacheToolBar;
  QToolBar *setupToolBar;
  QToolBar *editParamsToolBar;
  QToolBar *zoomToolBar;
  QToolBar *navigationToolBar;
  QToolBar *mpdToolBar;
  QComboBox *mpdCombo;

  // file
  QAction  *openAct;
  QAction  *saveAct;
  QAction  *saveAsAct;
  QAction  *saveCopyAct;
  QAction  *closeFileAct;
  QAction  *printToFileAct;
  QAction  *printToFilePreviewAct;
  QAction  *exportAsPdfAct;
  QAction  *exportAsPdfPreviewAct;
  QAction  *exportPngAct;
  QAction  *exportJpgAct;
  QAction  *exportBmpAct;

  QAction  *exportStlAct;
  QAction  *exportPovAct;
  QAction  *export3dsAct;
  QAction  *exportColladaAct;
  QAction  *exportObjAct;
  QAction  *exportCsvAct;
  QAction  *exportBricklinkAct;
  QAction  *exportHtmlAct;

  QAction *bringToFrontAct;
  QAction *sendToBackAct;

  QAction  *povrayRenderAct;

  QAction  *clearRecentAct;
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
  QAction  *fitSceneAct;
  QAction  *actualSizeAct;

  QAction  *zoomInComboAct;
  QAction  *zoomOutComboAct;

  QSlider  *zoomSliderWidget;
  QWidgetAction* zoomSliderAct;

  QSlider  *viewerZoomSliderWidget;
  QWidgetAction *viewerZoomSliderAct;

  QAction  *sceneGuidesComboAct;
  QAction  *sceneGuidesSolidLineAct;
  QAction  *sceneGuidesDashLineAct;
  QAction  *sceneGuidesPosTLeftAct;
  QAction  *sceneGuidesPosTRightAct;
  QAction  *sceneGuidesPosBLeftAct;
  QAction  *sceneGuidesPosBRightAct;
  QAction  *sceneGuidesPosCentreAct;
  QAction  *sceneRulerComboAct;
  QAction  *sceneRulerTrackingTickAct;
  QAction  *sceneRulerTrackingLineAct;
  QAction  *sceneRulerTrackingNoneAct;
  QAction  *hideRulerPageBackgroundAct;
  QAction  *hideGridPageBackgroundAct;
  QAction  *showTrackingCoordinatesAct;
  QAction  *showGuidesCoordinatesAct;

  QAction  *defaultCameraPropertiesAct;

  QActionGroup* SceneGuidesPosGroup;
  QActionGroup* SceneGuidesLineGroup;
  QActionGroup* SceneRulerGroup;
  QActionGroup* GridStepSizeGroup;

  // view
  // navigation toolbar

  QAction  *firstPageAct;
  QAction  *lastPageAct;
  QAction  *nextPageAct;
  QAction  *previousPageAct;
  QAction  *nextPageComboAct;
  QAction  *nextPageContinuousAct;
  QAction  *previousPageComboAct;
  QAction  *previousPageContinuousAct;
  QLineEdit*setPageLineEdit;
  QComboBox*setGoToPageCombo;

  // manage Caches
  QAction *clearAllCachesAct;

  QAction *clearPLICacheAct;
  QAction *clearCSICacheAct;
  QAction *clearSubmodelCacheAct;
  QAction *clearTempCacheAct;
  QAction *clearCustomPartCacheAct;

  QAction *refreshLDrawUnoffPartsAct;
  QAction *refreshLDrawOfficialPartsAct;

  // config menu

  QAction *pageSetupAct;
  QAction *assemSetupAct;
  QAction *pliSetupAct;
  QAction *bomSetupAct;
  QAction *calloutSetupAct;
  QAction *multiStepSetupAct;
  QAction *subModelSetupAct;
  QAction *projectSetupAct;
  QAction *fadeStepSetupAct;
  QAction *highlightStepSetupAct;

  QAction *preferencesAct;

  QAction *editFreeFormAnnitationsAct;
  QAction *editTitleAnnotationsAct;
  QAction *editLDrawColourPartsAct;
  QAction *editPliBomSubstitutePartsAct;
  QAction *editExcludedPartsAct;
  QAction *editLdrawIniFileAct;
  QAction *editLPub3DIniFileAct;
  QAction *editLdgliteIniAct;
  QAction *editNativePOVIniAct;
  QAction *editLdviewIniAct;
  QAction *editLdviewPovIniAct;
  QAction *editPovrayIniAct;
  QAction *editPovrayConfAct;
  QAction *editAnnotationStyleAct;
  QAction *editLD2BLCodesXRefAct;
  QAction *editLD2BLColorsXRefAct;
  QAction *editLD2RBCodesXRefAct;
  QAction *editLD2RBColorsXRefAct;
  QAction *editBLColorsAct;
  QAction *editBLCodesAct;
  QAction *generateCustomColourPartsAct;
  QAction *editModelFileAct;

  QAction *snapGridActions[NUM_GRID_SIZES];
  QAction *snapToGridComboAct;

  QMenu* SnapXYMenu;
  QMenu* SnapZMenu;
  QMenu* SnapMenu;
  QMenu* SnapAngleMenu;
  QAction* MoveAction;
  QAction* AngleAction;

  QAction *applyCameraAct;
  QAction *applyBuildModAct;
  QAction *removeBuildModAct;
  QAction *createBuildModAct;
  QAction *setTargetPositionAct;
  QAction *useImageSizeAct;
  QAction *enableBuildModAct;
  QAction *enableRotstepRotateAct;

  // help

  QAction  *aboutAct;

  // Begin Jaco's code

  QAction  *onlineManualAct;

  // End Jaco's code

  QAction  *metaAct;
  QAction  *separatorAct;

  QList<QAction *> openWithActList;
  QAction  *openWithSetupAct;

  enum { MaxRecentFiles = 8 };
  QAction *recentFilesActs[MaxRecentFiles];

  QAction *updateAppAct;
  QAction *viewLogAct;
  QAction *openWorkingFolderAct;

  friend class PartWorker;
  friend class DialogExportPages;
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

class GlobalHighlightStep
{
private:
    LDrawFile   ldrawFile;       // contains MPD or all files used in model
public:
    Meta        meta;
    QString     topLevelFile;
    GlobalHighlightStep()
    {
        meta = gui->page.meta;

        topLevelFile = ldrawFile.topLevelFile();
        MetaItem mi; // examine all the globals and then return
        mi.sortedGlobalWhere(meta,topLevelFile,"ZZZZZZZ");
    }
};

extern class Gui *gui;
extern QHash<SceneObject, QString> soMap;

inline Preferences& lpub3DGetPreferences()
{
    return gui->lpub3dPreferences;
}

inline PartWorker& partWorkerLDSearchDirs()
{
    return gui->partWorkerLDSearchDirs;
}

void clearPliCache();
void clearCsiCache();
void clearTempCache();
void clearSubmodelCache();
void clearAndReloadModelFile();

#endif
