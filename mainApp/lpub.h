/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2024 Trevor SANDY. All rights reserved.
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

#include "lpub_qtcompat.h"

#include "lc_global.h"
#include "lc_math.h"      // placed here to avoid having to always place this in .cpp files calling lpub.h

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

#include "application.h"
#include "lpub_object.h"

// Set to enable file watcher
#ifndef WATCHER
#define WATCHER
#endif

// Set to enable PageSize debug logging
#ifndef PAGE_SIZE_DEBUG
//#define PAGE_SIZE_DEBUG
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
class SeparatorComboBox;

class WaitingSpinnerWidget;
class MpdComboDelegate;

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
class UpdateCheck;

class LGraphicsView;
class LGraphicsScene;
class PageBackgroundItem;

class Pointer;
class PagePointer;

class lcHttpReply;
class lcHttpManager;
class lcPreferences;
class lcPiecesLibrary;
class lcModel;
class lcPartSelectionWidget;
class lcView;
class lcPreview;

class ColourPartListWorker;
class DialogExportPages;
class FadeStepsMeta;
class FadeStepsMeta;
class HighlightStepMeta;
class LDrawColor;
class LDrawColourParts;
class LDrawFile;
class MetaItem;
class Page;
class PageMeta;
class PageNumberItem;
class PartWorker;
class PageSizeData;
class Placement;
class PlacementFooter;
class PlacementHeader;
class PliPartGroupMeta;
class PliSubstituteParts;
class Preferences;
class ProgressDialog;
class Render;
class Range;
class Steps;
class Where;

class Gui : public QMainWindow
{

  Q_OBJECT

public:
  Gui();
  ~Gui();


  static int      stepPageNum;      // the number displayed on the page
  static int      saveStepPageNum;  // saved instance of the number displayed on the page
  static int      saveContStepNum;  // saved continuous step number for steps before displayPage, subModel exit and stepGroup end
  static int      saveGroupStepNum; // saved step group step number when pli per step is false
  static int      saveDisplayPageNum; // saved display page number when counting pages
  static int      saveMaxPages;     // saved page count when count (actually parse) build mods requested
  static int      firstStepPageNum; // the first Step page number - used to specify frontCover page
  static int      lastStepPageNum;  // the last Step page number - used to specify backCover page

  static QList<Where> parsedMessages; // previously parsed messages within the current session
  static QStringList  messageList;    // message list used when exporting or continuous processing

  static int      displayPageNum;       // what page are we displaying
  static int      prevDisplayPageNum;   // previous displayed page - used to roll back after encountering an error.
  static int      prevMaxPages;         // previous page count - used by continuousPageDialog to roll back after encountering an error.
  static int      processOption;        // export Option
  static int      pageDirection;        // page processing direction
  static int      savePrevStepPosition; // indicate the previous step position between current and previous steps  - used for fade/highlight steps and to roll back after exporting or an error.
  static bool     resetCache;           // reset model, fade and highlight parts
  static QString  saveFileName;         // user specified output file Name [commandline only]

  static QList<Where> topOfPages;       // topOfStep list of modelName and lineNumber for each page

  static RendererData savedRendererData;// store current renderer data when temporarily switching renderer;
  static int          saveRenderer;     // saved renderer when temporarily switching to Native renderer
  static bool         saveProjection;   // saved projection when temporarily switching to Native renderer
  static QString      pageRangeText;    // page range parameters

  static int          pa;               // page adjustment
  static int          sa;               // step number adustment
  static int          maxPages;

  static int          pageProcessRunning; // indicate page processing stage - 0=none, 1=writeToTmp,2-find/drawPage...
  static int          pageProcessParent; // the page process that triggers another process - e.g. drawPage -> writeToTmp
  qreal           exportPixelRatio;     // export resolution pixel density

  bool            submodelIconsLoaded;  // load submodel images

  static bool     buildModJumpForward;  // parse build mods in countPage call - special case for jump forward

  static bool      doFadeStep;
  static bool      doHighlightStep;

  static bool      m_fadeStepsSetup;    // enable fade previous steps locally
  static bool      m_highlightStepSetup;// enable highlight current step locally
  static int       m_exportMode;        // export mode
  static int       m_saveExportMode;    // saved export mode used when exporting BOM
  static QString   m_saveDirectoryName; // user specified output directory name [commandline only]

  bool             m_previewDialog;
  ProgressDialog  *m_progressDialog;    // general use progress dialogue

  static bool      m_partListCSIFile;   // processing part list CSI file
  static bool      suspendFileDisplay;  // when true, the endMacro() call will not call displayPage()
  void            *noData;

  FadeStepsMeta      *fadeStepsMeta;     // propagate fade step settings

  HighlightStepMeta *highlightStepMeta;  // propagate highlight step settings

  FitMode          fitMode;              // how to fit the scene into the view

  static QString  AttributeNames[];      // Pointer arrow attribute names
  static QString  PositionNames[];       // Pointer arrow position names
  static void     set_divider_pointers(  // Process step_group or callout divider pointers and pointer attributes
          Meta &curMeta,
          Where &current,
          Range *range,
          LGraphicsView *view,
          DividerType dividerType,
          int stepNum,
          Rc rct);
  Range *newRange(
          Steps  *steps,
          bool    calledOut);

  static Where &topOfPage();
  static Where &bottomOfPage();

  static bool processPageRange(const QString &range);

  void    changePageNum(int offset)
  {
    displayPageNum += offset;
  }
  void  displayPage();

  bool continuousPageDialog(PageDirection d);

  /* We need to send ourselves these, to eliminate recursion and the model
   * changing under foot */
  void drawPage(DrawPageFlags &dpFlags);  // this is the workhorse for preparing a
                                          // page for viewing.  It depends heavily
                                          // on the next two functions

  /*--------------------------------------------------------------------*
   * These are the work horses for back annotating user changes into    *
   * the LDraw files                                                    *
   *--------------------------------------------------------------------*/

  QStringList fileList()
  {
    return lpub->ldrawFile.subFileOrder();
  }
  QStringList modelList()
  {
      return lpub->ldrawFile.getSubModels();
  }
  QStringList modelContents(const QString &modelName)
  {
      return lpub->ldrawFile.contents(modelName);
  }
  QStringList contents(const QString &modelName)
  {
      return lpub->ldrawFile.contents(modelName);
  }
  QStringList smiContents(const QString &modelName)
  {
      return lpub->ldrawFile.smiContents(modelName);
  }
  bool changedSinceLastWrite(QString &modelName)
  {
      return lpub->ldrawFile.changedSinceLastWrite(modelName);
  }
  int subFileSize(const QString &modelName)
  {
    return lpub->ldrawFile.size(modelName);
  }
  // Only used to return fade or highlight content size
  int configuredSubFileSize(const QString &modelName)
  {
    return lpub->ldrawFile.configuredSubFileSize(modelName);
  }
  int numSteps(const QString &modelName)
  {
    return lpub->ldrawFile.numSteps(modelName);
  }
  int numParts()
  {
    return lpub->ldrawFile.getPartCount();
  }

  // Only used to read fade or highlight content
  QString readConfiguredLine(const Where &here)
  {
    return lpub->ldrawFile.readConfiguredLine(here.modelName,here.lineNumber);
  }

  bool isSubmodel(const QString &modelName)
  {
    return lpub->ldrawFile.isSubmodel(modelName);
  }
  bool isDisplayModel(const QString &modelName)
  {
    return lpub->ldrawFile.isDisplayModel(modelName);
  }
  bool isMpd()
  {
    return lpub->ldrawFile.isMpd();
  }
  bool isOlder(const QString &fileName,const QDateTime &lastModified)
  {
    bool older = lpub->ldrawFile.older(fileName,lastModified);
    return older;
  }
  bool isOlder(const QStringList &parsedStack,const QDateTime &lastModified)
  {
    bool older = lpub->ldrawFile.older(parsedStack,lastModified);
    return older;
  }
  bool isMirrored(QStringList &argv)
  {
    return lpub->ldrawFile.mirrored(argv);
  }
  bool isUnofficialPart(const QString &name)
  {
    return lpub->ldrawFile.isUnofficialPart(name) == UNOFFICIAL_PART;
  }
  bool isUnofficialSubPart(const QString &name)
  {
    return lpub->ldrawFile.isUnofficialPart(name) == UNOFFICIAL_SUBPART;
  }
  bool modified(const QString &fileName, bool reset = false)
  {
    return lpub->ldrawFile.modified(fileName, reset);
  }
  bool modified(const QStringList &parsedStack, bool reset = false)
  {
    return lpub->ldrawFile.modified(parsedStack, reset);
  }
  bool modified(const QVector<int> &parsedIndexes, bool reset = false)
  {
    return lpub->ldrawFile.modified(parsedIndexes, reset);
  }

  void insertGeneratedModel(const QString &name, QStringList &csiParts)
  {
    QDateTime date;
    lpub->ldrawFile.insert(name,
                           csiParts,
                           date,
                           UNOFFICIAL_SUBMODEL,
                           false /*displayModel*/,
                           true  /*generated*/,
                           false /*includeFile*/,
                           false /*dataFile*/,
                           QString(),
                           QFileInfo(name).completeBaseName());
    writeToTmp();
  }

  // Only used to insert fade or highlight content
  void insertConfiguredSubFile(const QString &name,
                                     QStringList &content);

  void clearPrevStepPositions()
  {
    lpub->ldrawFile.clearPrevStepPositions();
  }

  QString getSubmodelName(int index, bool lower = true)
  {
      return lpub->ldrawFile.getSubmodelName(index, lower);
  }

  int getLineTypeRelativeIndex(int submodelIndx, int lineTypeIndx)
  {
      return lpub->ldrawFile.getLineTypeRelativeIndex(submodelIndx,lineTypeIndx);
  }

  int getLineTypeIndex(int submodelIndx, int lineTypeIndx)
  {
      return lpub->ldrawFile.getLineTypeIndex(submodelIndx,lineTypeIndx);
  }

  int getSubmodelIndex(const QString &fileName)
  {
      return lpub->ldrawFile.getSubmodelIndex(fileName);
  }

  QVector<int> getSubmodelIndexes(const QString &fileName)
  {
      return lpub->ldrawFile.getSubmodelIndexes(fileName);
  }

  int getSubmodelInstances(const QString &fileName, bool isMirrored)
  {
      return lpub->ldrawFile.instances(fileName, isMirrored);
  }

  bool ldcadGroupMatch(const QString &name, const QStringList &lids)
  {
      return lpub->ldrawFile.ldcadGroupMatch(name,lids);
  }

  void updateViewerStep(const QString     &stepKey,
                        const QStringList &contents)
  {
      lpub->ldrawFile.updateViewerStep(stepKey, contents);
  }

  void insertViewerStep(const QString     &stepKey,
                        const QStringList &rotatedViewerContents,
                        const QStringList &rotatedContents,
                        const QStringList &unrotatedContents,
                        const QString     &filePath,
                        const QString     &imagePath,
                        const QString     &csiKey,
                        bool               multiStep,
                        bool               calledOut,
                        int                viewType)
  {
      lpub->ldrawFile.insertViewerStep(stepKey,
                                 rotatedViewerContents,
                                 rotatedContents,
                                 unrotatedContents,
                                 filePath,
                                 imagePath,
                                 csiKey,
                                 multiStep,
                                 calledOut,
                                 viewType);
  }

  QStringList getViewerStepRotatedContents(const QString &stepKey)
  {
      return lpub->ldrawFile.getViewerStepRotatedContents(stepKey);
  }

  QStringList getViewerStepUnrotatedContents(const QString &stepKey)
  {
      return lpub->ldrawFile.getViewerStepUnrotatedContents(stepKey);
  }

  QString getViewerStepFilePath(const QString &stepKey)
  {
      return lpub->ldrawFile.getViewerStepFilePath(stepKey);
  }

  QString getViewerStepImagePath(const QString &stepKey)
  {
      return lpub->ldrawFile.getViewerStepImagePath(stepKey);
  }

  QString getViewerConfigKey(const QString &stepKey)
  {
      return lpub->ldrawFile.getViewerConfigKey(stepKey);
  }

  QString getViewerStepKeyFromRange(const Where &here, const Where &top, const Where &bottom)
  {
      return lpub->ldrawFile.getViewerStepKeyFromRange(here.modelIndex, here.lineNumber, top.modelIndex, top.lineNumber, bottom.modelIndex, bottom.lineNumber);
  }

  QString getViewerStepKeyWhere(const Where &here)
  {
      return lpub->ldrawFile.getViewerStepKeyWhere(here.modelIndex, here.lineNumber);
  }

  int getViewerStepPartCount(const QString &stepKey)
  {
      return lpub->ldrawFile.getViewerStepPartCount(stepKey);
  }

  bool isViewerStepMultiStep(const QString &stepKey)
  {
      return lpub->ldrawFile.isViewerStepMultiStep(stepKey);
  }

  bool isViewerStepCalledOut(const QString &stepKey)
  {
      return lpub->ldrawFile.isViewerStepCalledOut(stepKey);
  }

  bool viewerStepContentExist(const QString &stepKey)
  {
      return lpub->ldrawFile.viewerStepContentExist(stepKey);
  }

  bool deleteViewerStep(const QString &stepKey)
  {
      return lpub->ldrawFile.deleteViewerStep(stepKey);
  }

  void clearViewerSteps() {
      lpub->ldrawFile.clearViewerSteps();
  }

  bool viewerStepModified(const QString &stepKey, bool reset = false)
  {
      return lpub->ldrawFile.viewerStepModified(stepKey, reset);
  }

  void setViewerStepModified(const QString &stepKey)
  {
      lpub->ldrawFile.setViewerStepModified(stepKey);
  }

  QString getViewerStepKey(const int stepIndex)
  {
      return lpub->ldrawFile.getViewerStepKey(stepIndex);
  }

  /* Build Modifications */

  void insertBuildMod(const QString      &buildModKey,
                      const QVector<int> &modAttributes,
                      int                 stepIndex)
  {
      lpub->ldrawFile.insertBuildMod(buildModKey,
                               modAttributes,
                               stepIndex);
  }

  int getBuildModBeginLineNumber(const QString &buildModKey)
  {
      return lpub->ldrawFile.getBuildModBeginLineNumber(buildModKey);
  }

  int getBuildModActionLineNumber(const QString &buildModKey)
  {
      return lpub->ldrawFile.getBuildModActionLineNumber(buildModKey);
  }

  int getBuildModEndLineNumber(const QString &buildModKey)
  {
      return lpub->ldrawFile.getBuildModEndLineNumber(buildModKey);
  }

  int getBuildModActionPrevIndex(const QString &buildModKey, const int stepIndex, const int action)
  {
      return lpub->ldrawFile.getBuildModActionPrevIndex(buildModKey, stepIndex, action);
  }

  int getBuildModAction(const QString &buildModKey, const int stepIndex)
  {
      return lpub->ldrawFile.getBuildModAction(buildModKey, stepIndex);
  }

  int getBuildModAction(const QString &buildModKey, const int stepIndex, const int defaultIndex)
  {
      return lpub->ldrawFile.getBuildModAction(buildModKey, stepIndex, defaultIndex);
  }

  int setBuildModAction(const QString &buildModKey, int stepIndex, int modAction)
  {
      return lpub->ldrawFile.setBuildModAction(buildModKey, stepIndex, modAction);
  }

  QMap<int, int> getBuildModActions(const QString &buildModKey)
  {
      return lpub->ldrawFile.getBuildModActions(buildModKey);
  }

  int getBuildModDisplayPageNumber(const QString &buildModKey)
  {
      return lpub->ldrawFile.getBuildModDisplayPageNumber(buildModKey);
  }

  int setBuildModDisplayPageNumber(const QString &buildModKey, int displayPageNum)
  {
      return lpub->ldrawFile.setBuildModDisplayPageNumber(buildModKey, displayPageNum);
  }

  void setBuildModSubmodelStack(const QString &buildModKey, const QStringList &submodelStack)
  {
      lpub->ldrawFile.setBuildModSubmodelStack(buildModKey, submodelStack);
  }

  int setBuildModStepPieces(const QString &buildModKey, int pieces)
  {
      return lpub->ldrawFile.setBuildModStepPieces(buildModKey, pieces);
  }

  int getBuildModStepPieces(const QString &buildModKey)
  {
      return lpub->ldrawFile.getBuildModStepPieces(buildModKey);
  }

  int getBuildModStepIndex(const QString &buildModKey)
  {
      return lpub->ldrawFile.getBuildModStepIndex(buildModKey);
  }

  int getBuildModStepIndex(const Where &here)
  {
      return lpub->ldrawFile.getBuildModStepIndex(getSubmodelIndex(here.modelName), here.lineNumber);
  }

  int getBuildModStepAction(const Where &here)
  {
      return lpub->ldrawFile.getBuildModStepAction(here.modelName, here.lineNumber);
  }

  QList<QVector<int> > getBuildModStepActions(const Where &here)
  {
      return lpub->ldrawFile.getBuildModStepActions(here.modelName, here.lineNumber);
  }

  void setBuildModStepKey(const QString &buildModKey, const QString &modStepKey)
  {
      lpub->ldrawFile.setBuildModStepKey(buildModKey, modStepKey);
  }

  void setBuildModRendered(const QString &buildModKey, const QString &colorModel)
  {
      lpub->ldrawFile.setBuildModRendered(buildModKey, colorModel);
  }

  bool setBuildModNextStepIndex(const Where &here)
  {
      return lpub->ldrawFile.setBuildModNextStepIndex(here.modelName, here.lineNumber);
  }

  void setLoadBuildMods(bool b)
  {
      lpub->ldrawFile.setLoadBuildMods(b);
  }

  void setBuildModNavBackward()
  {
      lpub->ldrawFile.setBuildModNavBackward();
  }

  int getBuildModNextStepIndex()
  {
      return lpub->ldrawFile.getBuildModNextStepIndex();
  }

  int getBuildModPrevStepIndex()
  {
      return lpub->ldrawFile.getBuildModPrevStepIndex();
  }

  void clearBuildModAction(const QString &buildModKey,const int stepIndex)
  {
      lpub->ldrawFile.clearBuildModAction(buildModKey, stepIndex);
  }

  void clearBuildModRendered(const QString &buildModKey, const QString &colorModel)
  {
      lpub->ldrawFile.clearBuildModRendered(buildModKey, colorModel);
  }

  QString getBuildModKey(const Where &here)
  {
      return lpub->ldrawFile.getBuildModKey(here.modelName, here.lineNumber);
  }

  // This function returns the equivalent of the ViewerStepKey
  QString getBuildModStepKey(const QString &buildModKey)
  {
      return lpub->ldrawFile.getBuildModStepKey(buildModKey);
  }

  QString getBuildModStepKeyModelName(const QString &buildModKey)
  {
      return lpub->ldrawFile.getBuildModStepKeyModelName(buildModKey);
  }

  int getBuildModStepKeyLineNum(const QString &buildModKey)
  {
      return lpub->ldrawFile.getBuildModStepKeyLineNum(buildModKey);
  }

  int getBuildModStepKeyStepNum(const QString &buildModKey)
  {
      return lpub->ldrawFile.getBuildModStepKeyStepNum(buildModKey);
  }

  int getBuildModStepKeyModelIndex(const QString &buildModKey)
  {
      return lpub->ldrawFile.getBuildModStepKeyModelIndex(buildModKey);
  }

  bool getBuildModStepIndexWhere(int stepIndex, Where &here)
  {
      return lpub->ldrawFile.getBuildModStepIndexWhere(stepIndex, here.modelName, here.modelIndex, here.lineNumber);
  }

  bool getBuildModRendered(const QString &buildModKey, const QString &colorModel)
  {
      return lpub->ldrawFile.getBuildModRendered(buildModKey, colorModel);
  }

  bool buildModContains(const QString &buildModKey)
  {
      return lpub->ldrawFile.buildModContains(buildModKey);
  }

  int getBuildModStepLineNumber(int stepIndex, bool bottom = true)
  {
      return lpub->ldrawFile.getBuildModStepLineNumber(stepIndex, bottom);
  }

  QStringList getBuildModsList()
  {
      return lpub->ldrawFile.getBuildModsList();
  }

  QStringList getPathsFromViewerStepKey(const QString &stepKey)
  {
      return lpub->ldrawFile.getPathsFromViewerStepKey(stepKey);
  }

  QStringList getPathsFromBuildModKeys(const QStringList &buildModKeys)
  {
      return lpub->ldrawFile.getPathsFromBuildModKeys(buildModKeys);
  }

  int buildModsCount()
  {
      return lpub->ldrawFile.buildModsCount();
  }

  bool deleteBuildMod(const QString &buildModKey = QString())
  {
      return lpub->ldrawFile.deleteBuildMod(buildModKey.isEmpty() ? getBuildModsList().last() : buildModKey);
  }

  void deleteBuildMods(const int &stepIndex)
  {
      lpub->ldrawFile.deleteBuildMods(stepIndex);
  }

  QString getBuildModificationKey()
  {
      return buildModificationKey;
  }

  QString getBuildModClearStepKey()
  {
      return buildModClearStepKey;
  }

  void setBuildModClearStepKey(const QString &text);

  void enableVisualBuildModActions();

  void enableVisualBuildModEditAction();

  bool setBuildModificationKey(Rc key = OkRc);

  int setBuildModForNextStep(Where topOfNextStep,
                             Where topOfSubmodel = Where());

  void setUndoRedoBuildModAction(bool Undo = true);

  /* End Build Modifications */

  void skipHeader(Where &current)
  {
      lpub->ldrawFile.skipHeader(current.modelName, current.lineNumber);
  }

  int getStepIndex(const Where &here)
  {
      return lpub->ldrawFile.getStepIndex(here.modelName, here.lineNumber);
  }

  void getTopOfStep(Where &here)
  {
      lpub->ldrawFile.getTopOfStep(here.modelName, here.modelIndex, here.lineNumber);
  }

  void setExportedFile(const QString &fileName)
  {
      exportedFile = fileName;
  }

  static bool suppressColourMeta()
  {
    return false; // Preferences::preferredRenderer == RENDERER_NATIVE;
  }

  void writeSmiContent(QStringList *content, const QString &fileName);
  QStringList getModelFileContent(QStringList *content, const QString &fileName);
  bool installRenderer(const int which);

  QString topLevelFile();
  QString readLine(const Where &here);
  void insertLine (const Where &here, const QString &line, QUndoCommand *parent = nullptr);
  void appendLine (const Where &here, const QString &line, QUndoCommand *parent = nullptr);
  void replaceLine(const Where &here, const QString &line, QUndoCommand *parent = nullptr);
  void deleteLine (const Where &here, QUndoCommand *parent = nullptr);
  void beginMacro (QString name);
  void endMacro();

  static void scanPast(    Where &here, const QRegExp &lineRx);
  static bool stepContains(Where &here, const QString &value);
  static bool stepContains(Where &here, const QRegExp &lineRx, QString &result, int capGrp = 0, bool displayModel = false);
  static bool stepContains(Where &here, QRegExp &lineRx, bool displayModel = false);

  static QString elapsedTime(const qint64 &duration);

  void getRequireds();
  void initialize();
  void initiaizeVisualEditor();

  void displayFile(
      LDrawFile *ldrawFile,
      const Where &here,
      bool editModelFile = false,
      bool displayStartPage = false,
      bool cycleSilent = false);
  void displayParmsFile(const QString &fileName);

  LGraphicsView *pageview()
  {
      return KpageView;
  }

  LGraphicsScene *pagescene()
  {
      return KpageScene;
  }

  void setSceneTheme()
  {
      KpageView->setSceneTheme();
  }

  void enableNavigationActions(bool enable);

  static Step *getCurrentStep()
  {
      return lpub->currentStep;
  }

  bool getSelectedLine(int modelIndex, int lineIndex, int source, int &lineNumber);

  static QString getCurFile()
  {
      return curFile;
  }

  void clearWorkingFiles(const QStringList &filePaths);
  void addEditLDrawIniFileAction();

  static QMap<int, PageSizeData> &getPageSizes()
  {
      return pageSizes;
  }

  static PageSizeData &getPageSize(int i)
  {
      return pageSizes[i];
  }

  int includePub(Meta &meta, int &lineNumber, bool &includeFileFound)
  {
      return include(meta, lineNumber, includeFileFound);
  }

  EditWindow  *getEditModeWindow()
  {
      return editModeWindow;
  }

  static void remove_group(
      QStringList  in,     // csiParts
      QVector<int> tin,    // typeIndexes
      QString      group,  // steps->meta.LPub.remove.group.value()
      QStringList  &out,   // newCSIParts
      QVector<int> &tiout, // newTypeIndexes
      Meta         *meta);

  static void remove_parttype(
      QStringList   in,     // csiParts
      QVector<int>  tin,    // typeIndexes
      QString       model,  // part type
      QStringList  &out,    // newCSIParts
      QVector<int> &tiout); // newTypeIndexes

  static void remove_partname(
      QStringList   in,     // csiParts
      QVector<int>  tin,    // typeIndexes
      QString       name,   // partName
      QStringList  &out,    // newCSIParts
      QVector<int> &tiout); // newCSIParts

  static void restorePreferredRenderer();

  static void setNativeRenderer();

  bool compareVersionStr(const QString &first, const QString &second);

  void createOpenWithActions(int maxPrograms = 0);

  void saveCurrent3DViewerModel(const QString &modelFile);

  QStringList get3DViewerPOVLightList() const;

  /******************************
   *
   * Shortcut mangement
   *
   */

  QAction *getAct(const QString &objectName);

public slots:
#ifndef QT_NO_CLIPBOARD
  void updateClipboard();
#endif

  // Native viewer functions
  void enable3DActions(bool enable);
  void halt3DViewer(bool enable);
  void UpdateVisualEditUndoRedo(const QString& UndoText, const QString& RedoText);

  void SetRotStepAngles(QVector<float>& Angles, bool display);
  void SetRotStepAngleX(float AngleX, bool display);
  void SetRotStepAngleY(float AngleY, bool display);
  void SetRotStepAngleZ(float AngleZ, bool display);
  void SetRotStepType(QString& RotStepType, bool display);
  void SetRotStepCommand();

  void ShowStepRotationStatus();
  void SetActiveModel(const QString &modelName);
  void SelectedPartLines(QVector<TypeLine> &indexes, PartSource source);
  void openFolderSelect(const QString &absoluteFilePath);
  void clearBuildModRange();
  void clearVisualEditUndoRedoText();
  void PreviewPiece(const QString &type, int colorCode, bool dockable, QRect parentRect, QPoint position);
  bool PreviewPiece(const QString &type, int colorCode);
  void setStepForLine();
  void togglePreviewWidget(bool);
  void updatePreview();
  void enableVisualBuildModification();
  int setupFadeOrHighlight(bool setupFadeSteps, bool setupHighlightStep);

  bool VisualEditorRotateTransform()
  {
    return EnableBuildModRotateAct->isChecked();
  }

  void SetVisualEditorRotateTransform(bool b)
  {
      EnableBuildModRotateAct->setChecked(b);
      EnableRotstepRotateAct->setChecked(!b);
  }

  QDockWidget *getPreviewDockWindow()
  {
      return previewDockWindow;
  }

  static void setPageProcessRunning(int p)
  {
      if (pageProcessRunning != p)
        pageProcessParent = pageProcessRunning;
      pageProcessRunning = p;
  }

  static void revertPageProcess()
  {
      pageProcessRunning = pageProcessParent;
  }

  static void insertPageSize(int i, const PageSizeData &pgSizeData)
  {
      pageSizes.insert(i,pgSizeData);
  }

  static void removePageSize(int i)
  {
      pageSizes.remove(i);
  }

  QString getViewerStepKey()
  {
      return lpub->viewerStepKey;
  }

  static int GetBOMs()
  {
      return boms;
  }

  static int GetBOMOccurrence()
  {
      return bomOccurrence;
  }

  bool OkToRaiseVisualEditorOrPreview()
  {
      bool commandEditIsVisible = false;
      if (Preferences::editorTabLock) {
          commandEditIsVisible = !commandEditDockWindow->visibleRegion().isEmpty();
          commandEditIsVisible &= !commandEditDockWindow->isFloating();
      }

      return !commandEditIsVisible;
  }

  void RaiseVisualEditDockWindow()
  {
      if (visualEditDockWindow &&
          visualEditDockWindow->visibleRegion().isEmpty() &&
          OkToRaiseVisualEditorOrPreview())
          visualEditDockWindow->raise();
  }

  void RaisePreviewDockWindow()
  {
      if (previewDockWindow &&
          previewDockWindow->visibleRegion().isEmpty() &&
          OkToRaiseVisualEditorOrPreview())
          previewDockWindow->raise();
  }

  QString GetPliIconsPath(QString& key);

  void setPliIconPath(QString& key, QString& value);

  int GetImageWidth();
  int GetImageHeight();

  // Native Viewer and Visual Editor convenience calls
  lcView*                GetActiveView();
  lcModel*               GetActiveModel();
  Project*               GetActiveProject();
  lcPartSelectionWidget* GetPartSelectionWidget();
  lcPiecesLibrary*       GetPiecesLibrary();
  lcPreferences&         GetPreferences();
  QToolBar*              GetToolsToolBar();
  QDockWidget*           GetTimelineToolBar();
  QDockWidget*           GetPropertiesToolBar();
  QDockWidget*           GetPartsToolBar();
  QDockWidget*           GetColorsToolBar();
  QMenu*                 GetCameraMenu();
  QMenu*                 GetToolsMenu();
  QMenu*                 GetViewpointMenu();
  QMenu*                 GetProjectionMenu();
  QMenu*                 GetShadingMenu();
  bool                   GetSubmodelIconsLoaded();
  int                    GetLPubStepPieces();
  void                   SetSubmodelIconsLoaded(bool);
  bool                   ReloadPiecesLibrary();
  bool                   ReloadUnofficialPiecesLibrary();
  void                   ReloadVisualEditor();
  void                   LoadColors();
  void                   LoadDefaults();
  static void            UpdateAllViews();
  void                   UnloadOfficialPiecesLibrary();
  void                   UnloadUnofficialPiecesLibrary();
  // End native viewer calls

  void loadBLCodes();

  void loadLastOpenedFile();

  /* The undoStack needs access to these */

  void canRedoChanged(bool);
  void canUndoChanged(bool);
  void cleanChanged(bool);

  /* The edit window sends us this */

  void contentsChange(const QString &fileName,
                            bool isUndo,
                            bool isRedo,
                            int  position,
                            int  charsRemoved,
                      const QString &charsAdded);

  /* This must be called from the Gui thread */
  void parseError(const QString &errorMsg,
                  const Where &here,
                  Preferences::MsgKey msgKey,
                  bool option,
                  bool override,
                  int icon,
                  const QString &title,
                  const QString &type);

  void statusBarMsg(QString msg);
  void statusMessage(LogType logType, const QString &statusMessage, int msgBox = 0);
  void showExportedFile();
  void showLine(const Where &here, int type = LINE_HIGHLIGHT);
  void openDropFile(QString &fileName);
  void enableApplyLightAction();

  /* Fade color processing */
  static QString createColourEntry(
    const QString &colourCode,
    const PartType partType,
    const QString &highlightStepColour = "",
    const QString &fadeStepsColour = "",
    const bool fadeStepsUseColour = Preferences::fadeStepsUseColour,
    const int fadeStepsOpacity = Preferences::fadeStepsOpacity);

  static bool colourEntryExist(
    const QStringList &colourEntries,
    const QString &code,
    const PartType partType,
    const bool fadeStepsUseColour = Preferences::fadeStepsUseColour);

  static bool isLDrawColourPart(const QString &fileName)
  {
     return LDrawColourParts::isLDrawColourPart(fileName);
  }

  static void deployBanner(bool b);
  static bool loadBanner(const int &type, const QString &bannerPath);
  static void setExporting(bool b) { m_exportingContent = b; m_abort = m_exportingContent && !b ? true : m_abort; if (!b) { m_exportingObjects = b; }; if (b) { m_countWaitForFinished = b; } }
  static void setExportingObjects(bool b) { m_exportingContent = m_exportingObjects = b; }
  static void setCountWaitForFinished(bool b) { m_countWaitForFinished = b; }
  static bool exporting() { return m_exportingContent; }
  static bool exportingImages() { return m_exportingContent && !m_exportingObjects; }
  static bool exportingObjects() { return m_exportingContent && m_exportingObjects; }
  static bool countWaitForFinished() { return m_countWaitForFinished; }
  static void cancelExporting() { m_exportingContent = m_exportingObjects = false; m_abort = m_exportingContent ? true : m_abort; }
  static void setAbortProcess(int b) { m_abort = b; }
  static bool abortProcess() { return m_abort; }

  static int exportMode() { return m_exportMode; }
  static QString saveDirectoryName () { return m_saveDirectoryName; }

  static bool ContinuousPage() { return m_contPageProcessing; }
  static void setContinuousPage(bool b) { m_contPageProcessing = b; m_abort = m_contPageProcessing && !b ? true : m_abort; }
  static void cancelContinuousPage() { m_contPageProcessing = false; m_abort = m_contPageProcessing ? true : m_abort; }
  void setContinuousPageAct(PageActType p = SET_DEFAULT_ACTION);
  void setPageContinuousIsRunning(bool b = true, PageDirection d = DIRECTION_NOT_SET);

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
  void fadeStepsSetup();
  void highlightStepSetup();
  void generateCoverPages();
  void ldrawColorPartsLoad();
  void insertFinalModelStep();
  void deleteFinalModelStep(bool force = false);

  void countPages();
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
  void groupActionTriggered();
  void useImageSize();
  void useSystemEditor();
  void recountParts();
  void autoCenterSelection();
  void resetViewerImage(bool=false);

  void ldrawSearchDirectories();
  void showDefaultCameraProperties();
  void restoreLightAndViewpointDefaults();
  void applyCameraSettings();
  void applyLightSettings();
  void applyBuildModification();
  void removeBuildModification();
  void createBuildModification();
  void loadBuildModification();
  void updateBuildModification();
  void deleteBuildModification();
  bool saveBuildModification();
  void deleteBuildModificationAction();

  void clearPLICache();
  void clearBOMCache();
  void clearCSICache();
  void clearSMICache(const QString &key = QString());
  void clearTempCache();
  void clearAllCaches();
  void clearCustomPartCache(bool silent = false);
  void clearStepCSICache(QString &pngName);
  void clearStepCache(Step *step, int option);
  void clearPageCache(PlacementType relativeType, Page *page, int option);
  void clearStepGraphicsItems(Step *step, int option);
  void clearAndReloadModelFile(bool fileReload = false, bool savePrompt = false, bool keepWork = false);
  void clearAndRedrawModelFile();
  void reloadCurrentModelFile();
  void reloadModelFileAfterColorFileGen();
  void reloadCurrentPage(bool prompt = false);
  void cyclePageDisplay(const int inputPageNum, bool silent = true, bool global = false);
  void loadTheme();
  void restartApplication(bool changeLibrary = false, bool prompt = false);
  void resetModelCache(QString file = QString(), bool commandLine = false);
  bool removeDir(int &count,const QString &dirName);

  void fileChanged(const QString &path);

  void processFadeColourParts(bool overwrite, bool setup);
  void processHighlightColourParts(bool overwrite, bool setup);
  void loadLDSearchDirParts(bool Process = false, bool OnDemand = false, bool Update = false);
  bool loadFile(const QString &file);
  bool loadFile(const QString &file, bool console);
  void openWith(const QString &filePath);

  void showRenderDialog();

  void TogglePdfExportPreview();
  void TogglePrintToFilePreview();
  void TogglePrintPreview(ExportMode m);

  void editModelFile(bool saveBefore, bool subModel = false);

signals:
  /* tell the editor to display this file */
  void displayFileSig(LDrawFile *ldrawFile, const QString &subFile, const StepLines &lineScope);
  void parseErrorSig(const QString &,
                     const Where &,
                     Preferences::MsgKey = Preferences::ParseErrors,
                     bool = false/*option*/,
                     bool = false/*override*/,
                     int = 0     /*NoIcon*/,
                     const QString &  = "",/*title*/
                     const QString &  = ""/*type*/);
  void displayModelFileSig(LDrawFile *ldrawFile, const QString &subFile);
  void displayParmsFileSig(const QString &fileName);
  void highlightSelectedLinesSig(QVector<int> &indexes, bool clear);
  void setSelectedPiecesSig(QVector<int> &indexes);
  void showLineSig(int lineNumber, int type);
  void setSubFilesSig(const QStringList &subFiles);
  void setLineScopeSig(const StepLines& lineScope);
  void previewModelSig(const QString &);
  void clearEditorWindowSig();
  void setTextEditHighlighterSig();
  void updateAllViewsSig();
  void clearViewerWindowSig();
  void setExportingSig(bool);
  void setExportingObjectsSig(bool);
  void setContinuousPageSig(bool);
  void hidePreviewDialogSig();
  void showExportedFileSig(int);
  void visualEditorVisibleSig(bool);
  void setGeneratingBomSig(bool);
  void countPagesSig();

  // cache management
  void clearStepCacheSig(Step *, int);
  void clearPageCacheSig(PlacementType, Page*, int);
  void clearAndReloadModelFileSig(bool, bool, bool);
  void clearCustomPartCacheSig(bool);
  void clearAllCachesSig();
  void clearSMICacheSig();
  void clearPLICacheSig();
  void clearBOMCacheSig();
  void clearCSICacheSig();
  void clearTempCacheSig();

  void reloadCurrentPageSig(bool);

  void restartApplicationSig(bool changeLibrary, bool prompt);

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

  void messageSig(LogType logType, const QString &message, int msgBox = 0);

  void requestEndThreadNowSig();
  void loadFileSig(const QString &file);

  void enableLPubFadeOrHighlightSig(bool, bool, bool);
  void operateHighlightParts(bool, bool);
  void operateFadeParts(bool, bool);
  void setPliIconPathSig(QString &,QString &);

  void fileLoadedSig(bool);
  void consoleCommandFromOtherThreadSig(int, int *);

public:
  ParmsWindow           *parmsWindow;                 // the parameter file editor

  // multi-thread worker classes
  PartWorker             partWorkerLDSearchDirs;      // part worker to process search directories and fade and or highlight color parts
  PartWorker            *partWorkerCustomColour;      // part worker to process color part fade and or highlight
  ColourPartListWorker  *colourPartListWorker;        // create static color parts list in separate thread
  static QMap<int, PageSizeData>  pageSizes;          // page size and orientation object

protected:
  // capture camera rotation from Visual Editor module
  QVector<float>         mStepRotation;
  float                  mRotStepAngleX;
  float                  mRotStepAngleY;
  float                  mRotStepAngleZ;
  QString                mRotStepType;
  QMap<QString, QString> mPliIconsPath;        // used to set an icon image in the Visual Editor timeline view
  QVector<int>           mBuildModRange;       // begin and end range of modified parts from Visual Editor
  QFutureWatcher<int>    futureWatcher;        // watch the countPage future

  int                    mViewerZoomLevel;

  SceneObject            selectedItemObj;

private:
  LGraphicsScene        *KpageScene;         // top of displayed page's graphics items
  LGraphicsView         *KpageView;          // the visual representation of the scene
  LGraphicsScene         KexportScene;       // top of displayed export and print page's graphics items
  LGraphicsView          KexportView;        // the visual representation of the export and print view

  Meta                  &meta = getMetaRef();   // meta command container
  Where                  current;            // current line being parsed by drawPage

  QElapsedTimer          displayPageTimer;   // measure elapsed time for slow functions

  lcPreview             *preview;
  EditWindow            *editWindow;         // the sub file editable by the user
  EditWindow            *editModeWindow;     // the model file editable by the user in
  QProgressBar          *progressBar;        // left side progress bar
  QProgressBar          *progressBarPerm;    // Right side progress bar
  QLabel                *progressLabel;
  QLabel                *progressLabelPerm;  // 
  WaitingSpinnerWidget  *waitingSpinner;     // waiting spinner animation
  PliSubstituteParts     pliSubstituteParts; // internal list of PLI/BOM substitute parts
  
  QMutex                 pageMutex;          // recursive drawPage, buildModNextStep, and findPage mutex,
  QMutex                 writeMutex;         // non-recursive write to temp working directory and countPage mutex 

  QTimer                 updateTimer;        // keep UI responsive when exporting or using continuous page processing

  static QString         curFile;                // the file name for MPD, or top level file
  static QString         curSubFile;             // whats being displayed in the edit window
  static bool            m_exportingContent;     // indicate export/printing underway
  static bool            m_exportingObjects;     // indicate exporting non-image object file content
  static bool            m_contPageProcessing;   // indicate continuous page processing underway
  static bool            m_countWaitForFinished; // indicate wait for countPage to finish on exporting 'return to saved page'
  static bool            m_abort;                // set to true when response to critcal error is abort

  static int             boms;                   // the number of pli BOMs in the document
  static int             bomOccurrence;          // the actual occurrence of each pli BOM
  static QStringList     bomParts;               // list of part strings configured for BOM setup
  static QList<PliPartGroupMeta> bomPartGroups;  // list of BOM part groups used for multi-page BOMs

  QString                exportedFile;           // the print preview produced pdf file
  QString                buildModClearStepKey;   // the step key indicating the step to start build mod clear actions
  QString                buildModificationKey;   // populated at buildMod change and cleared at buildMod create
  QStringList            programEntries;         // list of 'open with' programs populated on startup

  int                    numPrograms;

  int                    m_workerJobResult;

#ifdef WATCHER
  QFileSystemWatcher watcher;                // watch the file system for external
                                             // changes to the ldraw files currently
                                             // being edited
  bool               changeAccepted;         // don't throw another message unless existing was accepted
#endif

  QUndoStack     *undoStack;                 // the undo/redo stack
  int             macroNesting;
  bool            visualEditUndo;            // set the undo action enabled/disabled, text and status tip
  bool            visualEditRedo;            // set the redo action enabled/disabled, text and status tip
  QString         visualEditUndoRedoText;    // undo/redo text used to compare viewer change triggers

  bool     previousPageContinuousIsRunning;  // stop the continuous previous page action
  bool     nextPageContinuousIsRunning;      // stop the continuous next page action

  static bool okToInvokeProgressBar()
  {
    return (Preferences::lpub3dLoaded && Preferences::modeGUI && !Gui::exporting());
  }

  static bool isUserSceneObject(const int so);

  static int include(Meta &meta, int &lineNumber, bool &includeFileFound);

  static QStringList configureModelSubFile(
    const QStringList &,
    const QString &,
    const PartType partType);                // fade and or highlight all parts in subfile

  int findPage(                              // traverse the hierarchy until we get to the
    Meta             meta,                   // page of interest, let traverse process the
    QString const   &addLine,                // page, and then finish by counting the rest
    FindPageOptions &opts);

  int drawPage(                              // process the page of interest and any callouts
    Steps          *steps,
    QString const   &addLine,
    DrawPageOptions &opts);

  static void setSceneItemZValueDirection(
          QMap<Where, SceneObjectData> *selectedSceneItems,
          Meta &curMeta,
    const QString &line);

  static int addStepImageGraphics(Step    *step); //recurse the step's model - including callouts to add images.

  static int addStepPliPartGroupsToScene(Step *step,LGraphicsScene *scene);  //recurse the step's PLI - including callout PLIs to add PartGroups

  static int addPliPartGroupsToScene(
          Page           *page,
          LGraphicsScene *scene);

  static int addContentPageAttributes(
    Page                *page,
    PageBackgroundItem  *pageBg,
    PlacementHeader     *pageHeader,
    PlacementFooter     *pageFooter,
    PageNumberItem      *pageNumber,
    Placement           &plPage,
    bool                 endOfSubmodel = false);

  static int addPliPerPageItems(
    Page                *page,
    PlacementHeader     *pageHeader,
    PlacementFooter     *pageFooter,
    PageNumberItem      *pageNumber,
    Placement           &plPage);

  static int addCoverPageAttributes(
    Page                *page,
    PageBackgroundItem  *pageBg,
    PlacementHeader     *pageHeader,
    PlacementFooter     *pageFooter,
    Placement           &plPage);

  static int addGraphicsPageItems( // this converts the abstract page into
    Steps          *steps,         // a graphics view
    bool            coverPage,
    bool            endOfSubmodel,
    bool            printing);

  int getBOMParts(
    Where                    current,
    const QString           &addLine);

  int getBOMOccurrence(
          Where  current);

  bool generateBOMPartsFile(
          const QString &);

  void writeToTmp();

  QStringList writeToTmp(const QString &fileName, const QStringList &, bool = true);

  void attitudeAdjustment(); // reformat the LDraw file to fix LPub backward compatibility issues

  int whichFile(int option = 0);

  void openWithProgramAndArgs(QString &program, QStringList &arguments);

  void setSceneItemZValue(Page *page, LGraphicsScene *scene);
  void setSceneItemZValue(SceneObjectDirection direction);
  bool getSceneObjectWhere(QGraphicsItem *selectedItem, Where &itemTop);
  bool getSceneObjectStep(QGraphicsItem *selectedItem, int &stepNumber);
  bool getSceneObject(QGraphicsItem *selectedItem, Where &itemTop, int &stepNumber);

private slots:
    void finishedCountingPages();
    void pagesCounted();
    void open();
    void openWith();
    void save();
    void saveAs();
    void saveCopy();

    void bringToFront();
    void sendToBack();

    void fullScreenView();
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
    void editPliControlFile();
    void editPliBomSubstituteParts();
    void editLdrawIniFile();
    void editLPub3DIniFile();
    void editExcludedParts();
    void editStickerParts();
    void editLdgliteIni();
    void editNativePovIni();
    void editLdviewIni();
    void editLdviewPovIni();
    void editBlenderParameters();
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
    void enableLPubFadeOrHighlight(bool enableFadeSteps, bool enableHighlightStep ,bool waitForFinish);

    void viewLog();
    void loadStatus();

    void toggleLCStatusBar(bool);
    void showLCStatusMessage();
    void enableWindowFlags(bool);

    void pageProcessUpdate();

    void visitHomepage()
    {
        QDesktopServices::openUrl(QUrl(VER_HOMEPAGE_GITHUB_STR));
    }

    void openTicket()
    {
       QDesktopServices::openUrl(QUrl(VER_PUBLISHER_SUPPORT_STR));
    }
    // Begin Jaco's code

    void onlineManual();

    // End Jaco's code

    void commandsDialog();
    void exportMetaCommands();

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
    void removeBuildModFormat();
    void removeChildSubmodelFormat();
    void cycleEachPage();

    void nextPage();
    void nextPageContinuous();
    void previousPage();
    void previousPageContinuous();
    void pageLineEditReset();
    void enablePageLineReset(const QString &);
    void setPage();
    void firstPage();
    void lastPage();
    void setGoToPage(int index);
    void updateGoToPage(bool, bool);

    void consoleCommand(int, int *);
    void consoleCommandCurrentThread(int option, int *value);

    void getExportPageSize(float &, float &, int d = Pixels);
    bool validatePageRange();

    void ShowPrintDialog();
    void Print(QPrinter* Printer);
    void previewModel(QString const &);

    void importLDD();
    void importInventory();
    bool saveImport(const QString& FileName, Project *Importer);

    void exportAs(const QString &);
    void exportAsHtml();
    void exportAsHtmlSteps();
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

    void closeEvent(QCloseEvent *event) override;

    void mpdComboChanged(int index);
    void refreshLDrawUnoffParts();
    void refreshLDrawOfficialParts();
    void archivePartsOnDemand();
    void archivePartsOnLaunch();
    void writeGeneratedColorPartsToTemp();

    void clearPage(bool clearPageBg = false);

    void enableActions();
    void enableEditActions();
    void disableActions();
    void disableEditActions();

    void importToolBarVisibilityChanged(bool);
    void exportToolBarVisibilityChanged(bool);
    void loadStatusToolBarVisibilityChanged(bool);
    void cacheToolBarVisibilityChanged(bool);
    void setupToolBarVisibilityChanged(bool);
    void editToolBarVisibilityChanged(bool);
    void editParamsToolBarVisibilityChanged(bool);
    void removeLPubFormatToolBarVisibilityChanged(bool);

    void getSubFileList();

    /******************************************************************
     * File management functions
     *****************************************************************/

    void setCurrentFile(const QString &fileName);
    bool openFile(const QString &fileName);
    bool maybeSave(bool prompt = true, int sender = SaveOnNone);
    bool saveFile(const QString &fileName);
    void closeFile();
    void reloadFromDisk();
    void restorePreviousPage();
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
  void readVisualEditorSettings(QSettings &Settings);
  void writeVisualEditorSettings(QSettings &Settings);
  bool createPreviewWidget();

  bool eventFilter(QObject *object, QEvent *event) override;

  QDockWidget       *commandEditDockWindow;
  QDockWidget       *visualEditDockWindow;
  QDockWidget       *previewDockWindow;

  QMenu *getMenu(const QString &objectName);

  QMenu *viewMenu;

  // Visual Editor Menus
  QMenu *CameraMenu;
  QMenu *LightMenu;
  QMenu *BuildModMenu;
  QMenu *SelectMenu;
  QMenu *FindAndReplaceMenu;
  QMenu *SnapXYMenu;
  QMenu *SnapZMenu;
  QMenu *SnapMenu;
  QMenu *SnapAngleMenu;
  QMenu *ViewerMenu;
  QMenu *FileMenuViewer;
  QMenu *ViewerExportMenu;
  QMenu *ViewerZoomSliderMenu;
  QMenu *RotateStepActionMenu;

  // main window menus
  QMenu  *openWithMenu;
  QMap<QString, QMenu *> menus;

  QToolBar *exportToolBar;

  QToolBar *getToolBar(const QString &objectName);

  QMap<QString, QToolBar *> toolbars;

  QWidgetAction *zoomSliderAct;
  QWidgetAction *viewerZoomSliderAct;

  QActionGroup  *SceneGuidesPosGroup;
  QActionGroup  *SceneGuidesLineGroup;
  QActionGroup  *SceneRulerGroup;
  QActionGroup  *GridStepSizeGroup;

  QAction *undoAct;
  QAction *redoAct;

  QAction *blenderRenderAct;
  QAction *blenderImportAct;
  QAction *povrayRenderAct;

  QAction *ApplyCameraAct;
  QAction *CreateBuildModAct;
  QAction *ApplyBuildModAct;
  QAction *RemoveBuildModAct;
  QAction *LoadBuildModAct;
  QAction *UpdateBuildModAct;
  QAction *DeleteBuildModAct;
  QAction *DeleteBuildModActionAct;
  QAction *EnableBuildModRotateAct;
  QAction *EnableRotstepRotateAct;
  QAction *UseImageSizeAct;
  QAction *AutoCenterSelectionAct;
  QAction *DefaultCameraPropertiesAct;

  QAction *ApplyLightAct;
  QAction *LightGroupAct;
  QAction *FindAndReplaceGroupAct;
  QAction *SelectGroupAct;
  QAction *ViewpointGroupAct;
  QAction *ViewpointZoomExtAct;
  QAction *TransformAct;
  QAction *MoveAct;
  QAction *AngleAct;
  QAction *ResetViewerImageAct;

  QAction *recentFilesSeparatorAct;
  QAction *recentFilesActs[MAX_RECENT_FILES];
  QList<QAction *> openWithActList;
  QAction *snapGridActions[NUM_GRID_SIZES];

  /******************************
   * Other controls
   */
  QLineEdit         *setPageLineEdit;

  SeparatorComboBox *mpdCombo;
  QComboBox         *setGoToPageCombo;

  QSlider           *zoomSliderWidget;
  QSlider           *viewerZoomSliderWidget;

  friend class PartWorker;
  friend class DialogExportPages;
};

/******************************
 * LDraw Search Directory Dialog
 */

class EditLineNumberArea;
class TextEditSearchDirs;
class LDrawSearchDirDialog : public QWidget
{
  Q_OBJECT
  public:
  explicit LDrawSearchDirDialog(QWidget *parent = nullptr) : QWidget(parent) {}
  virtual ~LDrawSearchDirDialog() {}

  void getLDrawSearchDirDialog();
  public slots:
  void buttonSetState();
  void buttonClicked();

  private:
  QDialog            *dialog;
  QLineEdit          *lineEditIniFile;
  TextEditSearchDirs *textEditSearchDirs;
  QPushButton        *pushButtonReset;
  QPushButton        *pushButtonMoveUp;
  QPushButton        *pushButtonMoveDown;
  QPushButton        *pushButtonOpenFolder;
  QPushButton        *pushButtonAddDirectory;
  QStringList         excludedSearchDirs;
};

class TextEditSearchDirs : public QPlainTextEdit
{
  Q_OBJECT
public:
  explicit TextEditSearchDirs(QWidget *parent = nullptr);
  void lineNumberAreaPaintEvent(QPaintEvent *event);
  int lineNumberAreaWidth();

protected:
  void resizeEvent(QResizeEvent *event) override;

private slots:
  void updateLineNumberAreaWidth(int newBlockCount);
  void highlightCurrentLine();
  void updateLineNumberArea(const QRect &, int);

  private:
  QWidget *lineNumberArea;
};

class EditLineNumberArea : public QWidget
{
public:
  EditLineNumberArea(TextEditSearchDirs *editor) : QWidget(editor) {
    searchDirEditor = editor;
  }

  QSize sizeHint() const Q_DECL_OVERRIDE {
    return QSize(searchDirEditor->lineNumberAreaWidth(), 0);
  }

protected:
  void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE {
    searchDirEditor->lineNumberAreaPaintEvent(event);
  }

private:
  TextEditSearchDirs *searchDirEditor;
};

/******************************
 * Meta Commands File Dialog
 */

class MetaCommandsFileDialog : public QFileDialog
{
    Q_OBJECT
public:
    explicit MetaCommandsFileDialog(QWidget *parent = nullptr,
                                    const QString &caption = QString(),
                                    const QString &dir = QString(),
                                    const QString &filter = QString())
        : QFileDialog(parent, caption, dir, filter) {}
    static QString getCommandsSaveFileName(bool &exportDirections,
                                           QWidget *parent,
                                           const QString &caption,
                                           const QString &dir,
                                           const QString &filter,
                                           QString *selectedFilter = nullptr,
                                           Options options = Options());
};

/*******************************/

extern const bool showMsgBox;
extern QHash<SceneObject, QString> soMap;
extern class Gui *gui;

inline PartWorker& partWorkerLDSearchDirs()
{
    return gui->partWorkerLDSearchDirs;
}
#endif
