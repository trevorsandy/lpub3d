
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

/****************************************************************************
 *
 * This file describes a set of classes that implement a parse tree for
 * all the meta-commands that LPub supports.  Action metas such as STEP,
 * ROTSTEP, CALLOUT BEGIN, etc. return special return codes.  Configuration
 * metas that imply no action, but specify data for later use, retain
 * the onfiguration information, and return a generic OK return code.
 *
 * The top of tree is the Meta class that is the interface to the traverse
 * function that walks the LDraw model higherarchy.  Meta also tracks
 * locations in files like topOfModel, bottomOfModel, bottomOfSteps,topOfRange,
 * bottomOfRange, topOfStep, bottomOfStep, etc.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#include "version.h"
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets>
#else
#include <QtGui>
#endif
#include <QStringList>
#include "meta.h"
#include "lpub.h"

/* The token map translates known keywords to values 
 * used by LPub to identify things like placement and such
 */

QHash<QString, int> tokenMap;

bool AbstractMeta::reportErrors = false;

void AbstractMeta::init(
    BranchMeta *parent,
    QString name)
{
  preamble           = parent->preamble + name + " ";
  parent->list[name] = this;
}

void AbstractMeta::doc(QStringList &out, QString preamble)
{
  out << preamble;
}

QString LeafMeta::format(
    bool local,
    bool global,
    QString suffix)
{
  QString result;
  
  if (local) {
      result = "LOCAL ";
    } else if (global) {
      result = "GLOBAL ";
    }
  
  return preamble + result + suffix;
}  

BranchMeta::~BranchMeta()
{
  list.clear();
}

Rc BranchMeta::parse(QStringList &argv, int index, Where &here)
{
  Rc rc;
  int offset;
  int size = argv.size();

  if (index < size) {

      /* Find out if the current argv explicitly matches any of the
     * keywords known to be valid at this point in the meta command */

      QHash<QString, AbstractMeta *>::iterator i = list.find(argv[index]);

      if (i != list.end()) {

          /* We found a match */

          offset = 1;
          rc = OkRc;

          if (size - index > 1) {
              if (i.value()) {
                  if (argv[index+offset] == "LOCAL") {
                      i.value()->pushed = true;
                      offset++;
                    } else if (argv[index+offset] == "GLOBAL") {
                      i.value()->global = true;
                      offset++;
                    }
                  if (index + offset >= size) {
                      rc = FailureRc;
                    }
                }
            }

          /* Now parse the rest of the argvs */

          if (rc == OkRc) {
              return i.value()->parse(argv,index+offset,here);
            }
        } else if (size - index > 1) {

          /* Failed an explicit match.  Lets try to see if the value
       * matches any of the keywords through regular expressions */
          bool local  = argv[index] == "LOCAL";
          bool global = argv[index] == "GLOBAL";

          // LPUB CALLOUT Vertical
          // LPUB CALLOUT LOCAL Vertical

          offset = local || global;

          if (index + offset < size) {
              for (i = list.begin(); i != list.end(); i++) {
                  QRegExp rx(i.key());
                  if (argv[index + offset].contains(rx)) {

                      /* Now parse the rest of the argvs */

                      i.value()->pushed = local;
                      i.value()->global = global;
                      return i.value()->parse(argv,index+offset,here);
                    }
                }
            }
        }
    }

  // syntax error
  return FailureRc;
}

/* 
 * Find out if the match string matches the syntax graph up until this
 * point
 */

bool BranchMeta::preambleMatch(QStringList &argv, int index, QString &match)
{
  QHash<QString, AbstractMeta *>::iterator i = list.find(argv[index]);
  if (i == list.end() || index == argv.size()) {
      return false;
    } else {
      return i.value()->preambleMatch(argv,index,match);
    }
}

/*
 * Output documentation information for this node in the syntax
 * graph
 */

void BranchMeta::doc(QStringList &out, QString preamble)
{
  QString key;
  QStringList keys = list.keys();
  keys.sort();
  foreach(key, keys) {
      list[key]->doc(out, preamble + " " + key);
    }
}

void BranchMeta::pop()
{
  QString key;
  foreach (key,list.keys()) {
      list[key]->pop();
    }
}

/* ------------------ */
void RcMeta::init(BranchMeta *parent, const QString name, Rc _rc)
{ 
  AbstractMeta::init(parent,name);
  rc = _rc;
}
Rc RcMeta::parse(QStringList &argv, int index, Where &here)
{
  if (index != argv.size()) {
    }
  _here[pushed] = here;
  return rc;
}

void RcMeta::doc(QStringList &out, QString preamble)
{
  out << preamble;
}

/* ------------------ */

void IntMeta::init(
    BranchMeta *parent,
    const QString name,
    Rc _rc)
{
  AbstractMeta::init(parent,name);
  rc = _rc;
}
Rc IntMeta::parse(QStringList &argv, int index,Where &here)
{
  if (index == argv.size() - 1) {
      bool ok;
      int v;
      v = argv[index].toInt(&ok);
      if (ok) {
          if (v < _min || v > _max) {
              return RangeErrorRc;
            }
          _value[pushed] = v;
          _here[pushed] = here;
          return rc;
        }
    }

  if (reportErrors) {
      QMessageBox::warning(NULL,
                           QMessageBox::tr("LPub3D"),
                           QMessageBox::tr("Expected a whole number but got \"%1\" %2") .arg(argv[index]) .arg(argv.join(" ")));
    }

  return FailureRc;
}
QString IntMeta::format(bool local, bool global)
{
  QString foo;
  foo.arg(_value[pushed],0,base);
  
  return LeafMeta::format(local,global,foo);
}

void IntMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " <integer>";
}

/* ------------------ */

void FloatMeta::init(
    BranchMeta *parent,
    const QString name,
    Rc _rc)
{
  AbstractMeta::init(parent,name);
  rc = _rc;
}
Rc FloatMeta::parse(QStringList &argv, int index,Where &here)
{
  int size = argv.size();
  if (index == size - 1) {
      bool ok;
      float v = argv[index].toFloat(&ok);
      if (ok) {
          if (v < _min || v > _max) {
              return RangeErrorRc;
            }
          _value[pushed] = v;
          _here[pushed] = here;
          return rc;
        }
    }
  if (reportErrors) {
      QMessageBox::warning(NULL,
                           QMessageBox::tr("LPub3D"),
                           QMessageBox::tr("Expected a floating point number but got \"%1\" %2") .arg(argv[index]) .arg(argv.join(" ")));
    }

  return FailureRc;
}
QString FloatMeta::format(bool local, bool global)
{
  QString foo;
  foo = QString("%1") .arg(value(),_fieldWidth,'f',_precision);
  return LeafMeta::format(local,global,foo);
}
void FloatMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " <float>";
}

/* ------------------ */

QString UnitMeta::format(bool local, bool global)
{
  QString foo;
  foo = QString("%1") .arg(valueInches(),_fieldWidth,'f',_precision);
  return LeafMeta::format(local,global,foo);
}

int UnitMeta::valuePixels()
{
  float t = _value[pushed];
  return int(t*resolution());
}


/* ------------------ */
int UnitsMeta::valuePixels(int which)
{
  float t = _value[pushed][which];
  float r = resolution();

  return int(t*r);
}

QString UnitsMeta::format(bool local, bool global)
{
  QString foo;
  foo = QString("%1 %2")
      .arg(valueInches(0),_fieldWidth,'f',_precision)
      .arg(valueInches(1),_fieldWidth,'f',_precision);
  return LeafMeta::format(local,global,foo);
}

/* ------------------ */

void FloatPairMeta::init(
    BranchMeta *parent,
    const QString name,
    Rc _rc)
{
  AbstractMeta::init(parent,name);
  rc = _rc;
}
Rc FloatPairMeta::parse(QStringList &argv, int index,Where &here)
{
  if (argv.size() - index == 2) {
      bool ok[2];
      float v0 = argv[index  ].toFloat(&ok[0]);
      float v1 = argv[index+1].toFloat(&ok[1]);
      if (ok[0] && ok[1]) {
          if (v0 < _min || v0 > _max ||
              v1 < _min || v1 > _max) {
              return RangeErrorRc;
            }
          _value[pushed][0] = v0;
          _value[pushed][1] = v1;
          _here[pushed] = here;
          return rc;
        }
    }

  if (reportErrors) {
      QMessageBox::warning(NULL,
                           QMessageBox::tr("LPub3D"),
                           QMessageBox::tr("Expected two floating point numbers but got \"%1\" \"%2\" %3") .arg(argv[index]) .arg(argv[index+1]) .arg(argv.join(" ")));
    }

  return FailureRc;
}
QString FloatPairMeta::format(bool local, bool global)
{
  QString foo = QString("%1 %2")
      .arg(_value[pushed][0],_fieldWidth,'f',_precision)
      .arg(_value[pushed][1],_fieldWidth,'f',_precision);
  return LeafMeta::format(local,global,foo);
}
void FloatPairMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " <float> <float>";
}

/* ------------------ */
void StringMeta::init(
    BranchMeta *parent,
    QString name,
    Rc _rc,
    QString _delim)
{
  AbstractMeta::init(parent,name);
  rc = _rc;
  delim = _delim;
}
Rc StringMeta::parse(QStringList &argv, int index,Where &here)
{
  if (argv.size() - index == 1) {
      _value[pushed] = argv[index].replace("\\""","""");
      _here[pushed] = here;
      return rc;
    }

  if (reportErrors) {
      QMessageBox::warning(NULL,
                           QMessageBox::tr("LPub3D"),
                           QMessageBox::tr("Expected a string after \"%1\"") .arg(argv.join(" ")));
    }

  return FailureRc;
}
QString StringMeta::format(bool local, bool global)
{
  QString foo = delim + _value[pushed] + delim;
  return LeafMeta::format(local,global,foo);
}

void StringMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " <\"string\">";
}

/* ------------------ */

void StringListMeta::init(
    BranchMeta *parent,
    QString name,
    Rc _rc,
    QString _delim)
{
  AbstractMeta::init(parent,name);
  rc = _rc;
  delim = _delim;
}
Rc StringListMeta::parse(QStringList &argv, int index,Where &here)
{
  _value[pushed].clear();
  for (int i = index; i < argv.size(); i++) {
      _value[pushed] << argv[i];
    }
  _here[pushed] = here;
  return rc;
}
QString StringListMeta::format(bool local, bool global)
{
  QString foo;
  for (int i = 0; i < _value[pushed].size() ; i++) {
      foo += delim + _value[pushed][i] + delim + " ";
    }
  return LeafMeta::format(local,global,foo);
}

void StringListMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " <\"string\"> <\"string\"> .....";
}

/* ------------------ */

Rc BoolMeta::parse(QStringList &argv, int index,Where &here)
{
  QRegExp rx("^(TRUE|FALSE)$");
  if (index == argv.size() - 1 && argv[index].contains(rx)) {
      _value[pushed] = argv[index] == "TRUE";
      _here[pushed] = here;
      return OkRc;
    }
  
  if (reportErrors) {
      QMessageBox::warning(NULL,
                           QMessageBox::tr("LPub3D"),
                           QMessageBox::tr("Expected TRUE or FALSE \"%1\" %2") .arg(argv[index]) .arg(argv.join(" ")));
    }

  return FailureRc;
}
QString BoolMeta::format(bool local, bool global)
{
  QString foo (_value[pushed] ? "TRUE" : "FALSE");
  return LeafMeta::format(local,global,foo);
}

void BoolMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " <TRUE|FALSE>";
}

/* ------------------ */ 

QString placementOptions[][3] = 
{
  { "TOP_LEFT",    "",      "OUTSIDE" },
  { "TOP",         "LEFT",  "OUTSIDE" },
  { "TOP",         "CENTER","OUTSIDE" },
  { "TOP",         "RIGHT", "OUTSIDE" },
  { "TOP_RIGHT",   "",      "OUTSIDE" },

  { "LEFT",        "TOP",   "OUTSIDE" },
  { "TOP_LEFT",    "",      "INSIDE"  },
  { "TOP",         "",      "INSIDE"  },
  { "TOP_RIGHT",   "",      "INSIDE"  },
  { "RIGHT",       "TOP",   "OUTSIDE" },

  { "LEFT",        "CENTER","OUTSIDE" },
  { "LEFT",        "",      "INSIDE"  },
  { "CENTER",      "",      "INSIDE"  },
  { "RIGHT",       "",      "INSIDE"  },
  { "RIGHT",       "CENTER","OUTSIDE" },

  { "LEFT",        "BOTTOM", "OUTSIDE"},
  { "BOTTOM_LEFT", "",       "INSIDE" },
  { "BOTTOM",      "",       "INSIDE" },
  { "BOTTOM_RIGHT","",       "INSIDE" },
  { "RIGHT",       "BOTTOM", "OUTSIDE"},

  { "BOTTOM_LEFT", "",       "OUTSIDE" },
  { "BOTTOM",      "LEFT",   "OUTSIDE" },
  { "BOTTOM",      "CENTER", "OUTSIDE" },
  { "BOTTOM",      "RIGHT",  "OUTSIDE" },
  { "BOTTOM_RIGHT","",       "OUTSIDE" }
};

int placementDecode[][3] = 
{
  { TopLeft,     Center, Outside },
  { Top,         Left,   Outside },
  { Top,         Center, Outside },
  { Top,         Right,  Outside },
  { TopRight,    Center, Outside },

  { Left,        Top,    Outside },
  { TopLeft,     Center, Inside  },
  { Top,         Center, Inside  },
  { TopRight,    Center, Inside  },
  { Right,       Top,    Outside },

  { Left,        Center, Outside },
  { Left,        Center, Inside  },
  { Center,      Center, Inside  },
  { Right,       Center, Inside  },
  { Right,       Center, Outside },

  { Left,        Bottom, Outside },
  { BottomLeft,  Center, Inside  },
  { Bottom,      Center, Inside  },
  { BottomRight, Center, Inside  },
  { Right,       Bottom, Outside },
  
  { BottomLeft,  Center, Outside },
  { Bottom,      Left,   Outside },
  { Bottom,      Center, Outside },
  { Bottom,      Right,  Outside },
  { BottomRight, Center, Outside }
};

QString relativeNames[] = 
{
  "PAGE","ASSEM","MULTI_STEP","STEP_NUMBER","PLI","CALLOUT","PAGE_NUMBER",
  "DOCUMENT_TITLE","MODEL_ID","DOCUMENT_AUTHOR","PUBLISH_URL","MODEL_DESCRIPTION",
  "PUBLISH_DESCRIPTION","PUBLISH_COPYRIGHT","PUBLISH_EMAIL","LEGO_DISCLAIMER",
  "MODEL_PIECES","APP_PLUG","MODEL_CATEGORY","DOCUMENT_LOGO","DOCUMENT_COVER_IMAGE",
  "APP_PLUG_IMAGE","PAGE_HEADER","PAGE_FOOTER","ROTATE_ICON"
};

PlacementMeta::PlacementMeta() : LeafMeta()
{
  _value[0].placement     = PlacementEnc(placementDecode[TopLeftInsideCorner][0]);
  _value[0].justification = PlacementEnc(placementDecode[TopLeftInsideCorner][1]);
  _value[0].relativeTo    = PageType;
  _value[0].preposition   = PrepositionEnc(placementDecode[TopLeftInsideCorner][2]);
  _value[0].rectPlacement = TopLeftInsideCorner;
  _value[0].offsets[0] = 0;
  _value[0].offsets[1] = 0;
}

void PlacementMeta::setValue(
    RectPlacement placement,
    PlacementType relativeTo)
{
  _value[pushed].placement     = PlacementEnc(placementDecode[placement][0]);
  _value[pushed].justification = PlacementEnc(placementDecode[placement][1]);
  _value[pushed].relativeTo    = relativeTo;
  _value[pushed].preposition   = PrepositionEnc(placementDecode[placement][2]);
  _value[pushed].rectPlacement = placement;
}

Rc PlacementMeta::parse(QStringList &argv, int index,Where &here)
{
  RectPlacement  _placementR;
  PlacementType  _relativeTo;
  float _offsets[2];
  Rc rc = FailureRc;
  QString foo;
  int argc = argv.size();
  QString relativeTos = "^(PAGE|ASSEM|MULTI_STEP|STEP_NUMBER|PLI|CALLOUT|PAGE_NUMBER|"
                        "DOCUMENT_TITLE|MODEL_ID|DOCUMENT_AUTHOR|PUBLISH_URL|MODEL_DESCRIPTION|"
                        "PUBLISH_DESCRIPTION|PUBLISH_COPYRIGHT|PUBLISH_EMAIL|LEGO_DISCLAIMER|"
                        "MODEL_PIECES|APP_PLUG|MODEL_CATEGORY|DOCUMENT_LOGO|DOCUMENT_COVER_IMAGE|"
                        "APP_PLUG_IMAGE|PAGE_HEADER|PAGE_FOOTER|ROTATE_ICON)$";

  _placementR    = _value[pushed].rectPlacement;
  _relativeTo    = _value[pushed].relativeTo;
  _offsets[0]    = 0;
  _offsets[1]    = 0;

  //debug logging
  //   for(int i=0;i<argv.size();i++){
  //       int size = argv.size();
  //       int incr = i;
  //       int result = size - incr;
  //       logNotice() << "\nPAGE ARGV pos:(" << i+1 << ") index:(" << i << ") [" << size << " - " << incr << " = " << result << "] " << argv[i];}
  //       logNotice() << "\nWHERE: " << here.modelName << ", Line: " << here.lineNumber
  //                   << "\nSIZE: " << argv.size() << ", INDEX: " << index
  //                      ;
  //       logInfo()   << "\nSTART - Value at index: " << argv[index]
  //                      ;
  //end debug logging only

  if (argv[index] == "OFFSET") {
      index++;
      if (argc - index == 2) {
          bool ok[2];
          argv[index  ].toFloat(&ok[0]);
          argv[index+1].toFloat(&ok[1]);
          if (ok[0] && ok[1]) {
              _value[pushed].offsets[0] = argv[index  ].toFloat(&ok[0]);
              _value[pushed].offsets[1] = argv[index+1].toFloat(&ok[1]);
              _here[pushed] = here;
              return OkRc;
            }
        }
    }

  QString placement, justification, preposition, relativeTo;

  QRegExp rx("^(TOP|BOTTOM)$");
  if (argv[index].contains(rx)) {
      placement = argv[index++];

      if (index < argc) {
          rx.setPattern("^(LEFT|CENTER|RIGHT)$");
          if (argv[index].contains(rx)) {
              justification = argv[index++];
              rc = OkRc;
            } else {
              rx.setPattern(relativeTos);
              if (argv[index].contains(rx)) {
                  rc = OkRc;
                }
            }
        }
    } else {
      rx.setPattern("^(LEFT|RIGHT)$");
      if (argv[index].contains(rx)) {
          placement = argv[index++];

          if (index < argc) {
              rx.setPattern("^(TOP|CENTER|BOTTOM)$");
              if (argv[index].contains(rx)) {
                  justification = argv[index++];
                  rc = OkRc;
                } else {
                  rx.setPattern(relativeTos);
                  if (argv[index].contains(rx)) {
                      rc = OkRc;
                    }
                }
            }
        } else {
          rx.setPattern("^(TOP_LEFT|TOP_RIGHT|BOTTOM_LEFT|BOTTOM_RIGHT|CENTER)$");
          if (argv[index].contains(rx)) {
              placement = argv[index++];
              rc = OkRc;
            } else {
              return rc;
            }
        }
    }

  if (rc == OkRc && index < argv.size()) {
      rx.setPattern(relativeTos);
      if (argv[index].contains(rx)) {
          relativeTo = argv[index++];
          if (index < argc) {
              rx.setPattern("^(INSIDE|OUTSIDE)$");
              if (argv[index].contains(rx)) {
                  preposition = argv[index++];
                  rc = OkRc;
                }
              if (argc - index == 2) {
                  bool ok[2];
                  argv[index  ].toFloat(&ok[0]);
                  argv[index+1].toFloat(&ok[1]);
                  if (ok[0] && ok[1]) {
                      _offsets[0] = argv[index  ].toFloat(&ok[0]);
                      _offsets[1] = argv[index+1].toFloat(&ok[1]);
                      rc = OkRc;
                    }
                }
            } else {
              rc = OkRc;
            }
        }

      int i;

      if (preposition == "INSIDE" && justification == "CENTER") {
          justification = "";
        }

      for (i = 0; i < NumSpots; i++) {
          if (placementOptions[i][0] == placement &&
              placementOptions[i][1] == justification &&
              placementOptions[i][2] == preposition) {
              break;
            }
        }

      if (i == NumSpots) {
          return FailureRc;
        }
      _placementR = RectPlacement(i);

      _relativeTo = PlacementType(tokenMap[relativeTo]);
      setValue(_placementR,_relativeTo);
      _value[pushed].offsets[0] = _offsets[0];
      _value[pushed].offsets[1] = _offsets[1];
      _here[pushed] = here;
    }
  return rc;
}

QString placementNames[] =
{
  "TOP_LEFT", "TOP", "TOP_RIGHT", "RIGHT",
  "BOTTOM_RIGHT", "BOTTOM", "BOTTOM_LEFT", "LEFT", "CENTER"
};

QString prepositionNames[] =
{
  "INSIDE", "OUTSIDE"
};

QString PlacementMeta::format(bool local, bool global)
{
  //debug logging
  //qDebug() << " \nPLACEMENT META FORMAT: "
  //           << " \nPUSHED VALUES: "
  //           << " \nPlacement: "      << _value[pushed].placement
  //           << " \nJustification: "  << _value[pushed].justification
  //           << " \nRelativeTo: "     << _value[pushed].relativeTo
  //           << " \nPreposition(*): " << _value[pushed].preposition
  //           << " \nOffset[0]: "      << _value[pushed].offsets[0]
  //           << " \nOffset[0]: "      << _value[pushed].offsets[1]
  //           << " \nNAMES: "
  //           << " \nPlacement: "      << placementNames  [_value[pushed].placement]
  //           << " \nJustification: "  << placementNames  [_value[pushed].justification]
  //           << " \nRelativeTo: "     << relativeNames   [_value[pushed].relativeTo]
  //           << " \nPreposition(*): " << prepositionNames[_value[pushed].preposition]
  //              ;
  //end debug logging
  QString foo;
  
  if (_value[pushed].preposition == Inside) {
      switch (_value[pushed].placement) {
        case Top:
        case Bottom:
        case Right:
        case Left:
          foo = placementNames[_value[pushed].placement] + " "
              + relativeNames [_value[pushed].relativeTo] + " "
              + prepositionNames[_value[pushed].preposition];
          break;
        default:
          foo = placementNames[_value[pushed].placement] + " "
              + relativeNames [_value[pushed].relativeTo] + " "
              + prepositionNames[_value[pushed].preposition];
        }
    } else {

      switch (_value[pushed].placement) {
        case Top:
        case Bottom:
        case Right:
        case Left:
          foo = placementNames[_value[pushed].placement] + " "
              + placementNames[_value[pushed].justification] + " "
              + relativeNames [_value[pushed].relativeTo] + " "
              + prepositionNames[_value[pushed].preposition];
          break;
        default:
          foo = placementNames[_value[pushed].placement] + " "
              + relativeNames [_value[pushed].relativeTo] + " "
              + prepositionNames[_value[pushed].preposition];
        }
    }
  if (_value[pushed].offsets[0] || _value[pushed].offsets[1]) {
      QString bar = QString(" %1 %2")
          .arg(_value[pushed].offsets[0])
          .arg(_value[pushed].offsets[1]);
      foo += bar;
    }
  return LeafMeta::format(local,global,foo);
}

void PlacementMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " (TOP|BOTTOM) (LEFT|CENTER|RIGHT) (PAGE|ASSEM (INSIDE|OUTSIDE)|MULTI_STEP|STEP_NUMBER|PLI|CALLOUT)";
  out << preamble + " (LEFT|RIGHT) (TOP|CENTER|BOTTOM) (PAGE|ASSEM (INSIDE|OUTSIDE)|MULTI_STEP|STEP_NUMBER|PLI|CALLOUT)";
  out << preamble + " (TOP_LEFT|TOP_RIGHT|BOTTOM_LEFT|BOTTOM_RIGHT) (PAGE|ASSEM (INSIDE|OUTIDE)|MULTI_STEP|STEP_NUMBER|PLI|ROTATE_ICON|CALLOUT)";
}
/* ------------------ */ 

Rc BackgroundMeta::parse(QStringList &argv, int index,Where &here)
{
  Rc rc = FailureRc;
  if (argv.size() - index == 1) {
      if (argv[index] == "TRANS" || argv[index] == "TRANSPARENT") {
          _value[pushed].type = BackgroundData::BgTransparent;
          rc = OkRc;
        } else if (argv[index] == "SUBMODEL_BACKGROUND_COLOR") {
          _value[pushed].type = BackgroundData::BgSubmodelColor;
          rc = OkRc;
        } else {
          _value[pushed].type = BackgroundData::BgImage;
          _value[pushed].string = argv[index];
          _value[pushed].stretch = false;
          rc = OkRc;
        }
    } else if (argv.size() - index == 2) {
      if (argv[index] == "COLOR") {
          _value[pushed].type = BackgroundData::BgColor;
          _value[pushed].string = argv[index+1];
          rc = OkRc;
        } else if (argv[index] == "PICTURE") {
          _value[pushed].type = BackgroundData::BgImage;
          _value[pushed].string = argv[index+1];
          _value[pushed].stretch = false;
          rc = OkRc;
        }
    } else if (argv.size() - index == 3) {
      if (argv[index] == "PICTURE" && argv[index+2] == "STRETCH") {
          _value[pushed].type = BackgroundData::BgImage;
          _value[pushed].string = argv[index+1];
          _value[pushed].stretch = true;
          rc = OkRc;
        }
    } else if (argv.size() - index == 9){

      if (argv[index] == "GRADIENT"){

          bool ok[6];
          bool pass = true;
          argv[index+1].toInt(&ok[0]);
          argv[index+2].toInt(&ok[1]);
          argv[index+3].toInt(&ok[2]);
          argv[index+4].toFloat(&ok[3]);
          argv[index+5].toFloat(&ok[4]);
          argv[index+6].toFloat(&ok[5]);

          const QStringList _gpoints = argv[index+7].split("|");
          QVector<QPointF> gpoints;
          Q_FOREACH(const QString &gpoint, _gpoints){
              bool ok[2];
              int x = gpoint.section(',',0,0).toFloat(&ok[0]);
              int y = gpoint.section(',',1,1).toFloat(&ok[1]);
              if (ok[0] && ok[1])
                  gpoints << QPointF(x, y);
              else if (pass)
                  pass = false;
            }

          const QStringList _gstops  = argv[index+8].split("|");
          QVector<QPair<qreal,QColor> > gstops;
          Q_FOREACH(const QString &_gstop, _gstops){
              bool ok[2];
              qreal point  = _gstop.section(',',0,0).toFloat(&ok[0]);
              unsigned int rgba = _gstop.section(',',1,1).toUInt(&ok[1],16);
              if (ok[0] && ok[1])
                  gstops.append(qMakePair(point, QColor::fromRgba(rgba)));
              else if (pass)
                  pass = false;
            }

          if (ok[0] && ok[1] && ok[2] &&
              ok[3] && ok[4] && ok[5] && pass) {

              int _gmode   = argv[index+1].toInt(&ok[0]);
              int _gspread = argv[index+2].toInt(&ok[1]);
              int _gtype   = argv[index+3].toInt(&ok[2]);

              _value[pushed].type = BackgroundData::BgGradient;

              switch (_gmode){
              case 0:
                  _value[pushed].gmode = BackgroundData::LogicalMode;
              break;
              case 1:
                  _value[pushed].gmode = BackgroundData::StretchToDeviceMode;
              break;
              case 2:
                  _value[pushed].gmode = BackgroundData::ObjectBoundingMode;
              break;
                }
              switch (_gspread){
              case 0:
                  _value[pushed].gspread = BackgroundData::PadSpread;
              break;
              case 1:
                  _value[pushed].gspread = BackgroundData::RepeatSpread;
              break;
              case 2:
                  _value[pushed].gspread = BackgroundData::ReflectSpread;
              break;
                }
              switch (_gtype){
              case 0:
                  _value[pushed].gtype = BackgroundData::LinearGradient;
              break;
              case 1:
                  _value[pushed].gtype = BackgroundData::RadialGradient;
              break;
              case 2:
                  _value[pushed].gtype = BackgroundData::ConicalGradient;
              break;
              case 3:
              break;
                }
              _value[pushed].gsize[0] = argv[index+4].toInt(&ok[3]);
              _value[pushed].gsize[1] = argv[index+5].toInt(&ok[4]);
              _value[pushed].gangle   = argv[index+6].toInt(&ok[5]);
              _value[pushed].gpoints  = gpoints;
              _value[pushed].gstops   = gstops;
              rc = OkRc;
            }
        }
    }
  if (rc == OkRc) {
      _here[pushed] = here;
      return rc;
    } else {
      
      if (reportErrors) {
          QMessageBox::warning(NULL,
                               QMessageBox::tr("LPub3D"),
                               QMessageBox::tr("Malformed background \"%1\"") .arg(argv.join(" ")));
        }

      return FailureRc;
    }
}

QString BackgroundMeta::format(bool local, bool global)
{
  QString foo;
  switch (_value[pushed].type) {
    case BackgroundData::BgTransparent:
      foo = "TRANSPARENT";
      break;
    case BackgroundData::BgSubmodelColor:
      foo = "SUBMODEL_BACKGROUND_COLOR";
      break;
    case BackgroundData::BgColor:
      foo = "COLOR \"" + _value[pushed].string + "\"";
      break;
    case BackgroundData::BgGradient:
      {
        QString points;
        const QVector<QPointF> _points = _value[pushed].gpoints;
        Q_FOREACH(const QPointF &point, _points){
            points += QString("%1,%2|")
                .arg(point.x())
                .arg(point.y());
          }
        points = points.remove(points.size()-1,1);

        QString stops;
        const QVector<QPair<qreal,QColor> > _gstops = _value[pushed].gstops;
        typedef QPair<qreal,QColor> _gstop;
        Q_FOREACH(const _gstop &gstop, _gstops){
//            if (gstop.second.name() != "#000000")
//            gstop.second.name().replace("#","0xff");
            stops += QString("%1,%2|")
                .arg(gstop.first)
                .arg(gstop.second.name().replace("#","0xff"));
          }
        stops = stops.remove(stops.size()-1,1);

        foo = QString("GRADIENT %1 %2 %3 %4 %5 %6 \"%7\" \"%8\"")
            .arg(_value[pushed].gmode)
            .arg(_value[pushed].gspread)
            .arg(_value[pushed].gtype)
            .arg(_value[pushed].gsize[0])
            .arg(_value[pushed].gsize[1])
            .arg(_value[pushed].gangle)
            .arg(points)
            .arg(stops);
      }
      break;
    case BackgroundData::BgImage:
      foo = "PICTURE \"" + _value[pushed].string + "\"";
      if (_value[pushed].stretch) {
          foo += " STRETCH";
        }
      break;
    }
  return LeafMeta::format(local,global,foo);
}

void BackgroundMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " (TRANSPARENT|SUBMODEL_BACKGROUND_COLOR|COLOR <\"color\">|"
                    "GRADIENT <mode spread type size[0] size[1] angle \"points\" \"stops\">|"
                    "PICTURE (STRETCH) <\"picture\">)";
}

QString BackgroundMeta::text()
{
  BackgroundData background = value();
  switch (background.type) {
    case BackgroundData::BgTransparent:
      return "Transparent";
      break;
    case BackgroundData::BgImage:
      return "Picture " + background.string;
      break;
    case BackgroundData::BgColor:
      return "Color " + background.string;
      break;
    case BackgroundData::BgGradient:
      return "Gradient " + background.string;
      break;
    default:
      break;
    }
  return "Submodel level color";
}

Rc BorderMeta::parse(QStringList &argv, int index,Where &here)
{
  Rc rc = FailureRc;

  if (argv[index] == "NONE" && argv.size() - index >= 1) {
      _value[pushed].type = BorderData::BdrNone;
      bool ok;
      argv[index+1].toInt(&ok);
      if (ok){
         _value[pushed].line = setBorderLine(argv[index+1]);
         index++;
         rc = OkRc;
        }
    } else if (argv[index] == "SQUARE" && argv.size() - index >= 4) {
      bool ok[2];
      argv[index+1].toInt(&ok[0]);
      argv[index+3].toFloat(&ok[1]);
      if (ok[0] && ok[1]) {
          _value[pushed].type      = BorderData::BdrSquare;
          _value[pushed].line      = setBorderLine(argv[index+1]);
          _value[pushed].color     = argv[index+2];
          _value[pushed].thickness = argv[index+3].toFloat(&ok[1]);
          index += 4;
          rc = OkRc;
        }
    } else if (argv[index] == "ROUND" && argv.size() - index >= 5) {
      bool ok[3];
      argv[index+1].toInt(&ok[0]);
      argv[index+3].toFloat(&ok[1]);
      argv[index+4].toInt(&ok[2]);
      if (ok[0] && ok[1] && ok[2]) {
          _value[pushed].type      = BorderData::BdrRound;
          _value[pushed].line      = setBorderLine(argv[index+1]);
          _value[pushed].color     = argv[index+2];
          _value[pushed].thickness = argv[index+3].toFloat(&ok[0]);
          _value[pushed].radius    = argv[index+4].toInt(&ok[0]);
          index += 5;
          rc = OkRc;
        }
    }

  if (rc == OkRc && argv.size() - index == 3) {
      if (argv[index] == "MARGINS") {
          bool ok[2];
          argv[index + 1].toFloat(&ok[0]);
          argv[index + 2].toFloat(&ok[1]);
          if (ok[0] && ok[1]) {
              _value[pushed].margin[0] = argv[index + 1].toFloat(&ok[0]);
              _value[pushed].margin[1] = argv[index + 2].toFloat(&ok[1]);
              index += 3;
            } else {
              rc = FailureRc;
            }
        } else {
          rc = FailureRc;
        }
    }
  if (rc == OkRc) {
      _here[pushed] = here;
    }
  return rc;
}

QString BorderMeta::format(bool local, bool global)
{
  QString foo;
  switch (_value[pushed].type) {
    case BorderData::BdrNone:
      foo = QString("NONE %1")
          .arg(_value[pushed].line);
      break;
    case BorderData::BdrSquare:
      foo = QString("SQUARE %1 %2 %3")
          .arg(_value[pushed].line)
          .arg(_value[pushed].color)
          .arg(_value[pushed].thickness);
      break;
    case BorderData::BdrRound:
      foo = QString("ROUND %1 %2 %3 %4")
          .arg(_value[pushed].line)
          .arg(_value[pushed].color)
          .arg(_value[pushed].thickness)
          .arg(_value[pushed].radius);
      break;
    }

  QString bar = QString(" MARGINS %1 %2")
      .arg(_value[pushed].margin[0])
      .arg(_value[pushed].margin[1]);
  foo += bar;
  return LeafMeta::format(local,global,foo);
}

void BorderMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " (NONE <line> |SQUARE <line> <color> <thickness>|ROUND <line> <color> <thickness> <radius>) MARGINS <x> <y>";
}

QString BorderMeta::text()
{
  BorderData border = valueInches();
  QString thickness;
  switch (border.type) {
    case BorderData::BdrNone:
      return "No Border";
      break;
    case BorderData::BdrSquare:
      thickness = QString("%1")
          .arg(border.thickness,4,'f',3);
      return "Square Corners, thickess " + thickness + " " + units2abbrev();
      break;
    default:
      break;
    }
  thickness = QString("%1") .arg(border.thickness,4,'f',3);
  return "Round Corners, thickess " + thickness + " " + units2abbrev();
}

/* ------------------ */ 

PointerMeta::PointerMeta() : LeafMeta()
{
  _value[0].placement = TopLeft;
  _value[0].loc       = 0;
  _value[0].x         = 0.5;
  _value[0].y         = 0.5;
  _value[0].base      = 0.125;
}

/*
 * (TopLeft|TopRight|BottomRight|BottomLeft) <x> <y> (<base>)
 *
 * (Top|Right|Bottom|Left) <loc> <x> <y> (<base>)
 */

Rc PointerMeta::parse(QStringList &argv, int index,Where &here)
{
  float _loc = 0, _x = 0, _y = 0, _base = -1;
  int   n_tokens = argv.size() - index;
  QString foo1 = argv[index];
  QString foo2 = argv[index+1];
  bool    fail = true;

  if (argv.size() - index > 0) {
      QRegExp rx("^(TOP_LEFT|TOP_RIGHT|BOTTOM_LEFT|BOTTOM_RIGHT)$");
      if (argv[index].contains(rx) && n_tokens == 4) {
          _loc = 0;
          bool ok[3];
          _x    = argv[index+1].toFloat(&ok[0]);
          _y    = argv[index+2].toFloat(&ok[1]);
          _base = argv[index+3].toFloat(&ok[2]);
          fail  = ! (ok[0] && ok[1] && ok[2]);
        }
      if (argv[index].contains(rx) && n_tokens == 3) {
          _loc = 0;
          bool ok[2];
          _x    = argv[index+1].toFloat(&ok[0]);
          _y    = argv[index+2].toFloat(&ok[1]);
          fail  = ! (ok[0] && ok[1]);
        }
      rx.setPattern("^(TOP|BOTTOM|LEFT|RIGHT|CENTER)$");
      if (argv[index].contains(rx) && n_tokens == 5) {
          _loc = 0;
          bool ok[4];
          _loc  = argv[index+1].toFloat(&ok[0]);
          _x    = argv[index+2].toFloat(&ok[1]);
          _y    = argv[index+3].toFloat(&ok[2]);
          _base = argv[index+4].toFloat(&ok[3]);
          fail  = ! (ok[0] && ok[1] && ok[2] && ok[3]);
        }
      if (argv[index].contains(rx) && n_tokens == 4) {
          _loc = 0;
          bool ok[3];
          _loc  = argv[index+1].toFloat(&ok[0]);
          _x    = argv[index+2].toFloat(&ok[1]);
          _y    = argv[index+3].toFloat(&ok[2]);
          fail  = ! (ok[0] && ok[1] && ok[2]);
        }
    }
  if ( ! fail) {
      _value[pushed].placement = PlacementEnc(tokenMap[argv[index]]);
      _value[pushed].loc       = _loc;
      _value[pushed].x         = _x;
      _value[pushed].y         = _y;
      if (_base > 0) {
          _value[pushed].base = _base;
        } else if (_value[pushed].base == 0) {
          _value[pushed].base = 1.0/8;
        }
      _here[0] = here;
      _here[1] = here;
      return CalloutPointerRc;
    } else {

      if (reportErrors) {
          QMessageBox::warning(NULL,
                               QMessageBox::tr("LPub3D"),
                               QMessageBox::tr("Malformed callout arrow \"%1\"") .arg(argv.join(" ")));
        }
      return FailureRc;
    }
}

QString PointerMeta::format(bool local, bool global)
{
  QString foo;
  switch(_value[pushed].placement) {
    case TopLeft:
    case TopRight:
    case BottomRight:
    case BottomLeft:
      foo = QString("%1 %2 %3 %4")
          .arg(placementNames[_value[pushed].placement])
          .arg(_value[pushed].x,0,'f',3)
          .arg(_value[pushed].y,0,'f',3)
          .arg(_value[pushed].base);
      break;
    default:
      foo = QString("%1 %2 %3 %4 %5")
          .arg(placementNames[_value[pushed].placement])
          .arg(_value[pushed].loc,0,'f',3)
          .arg(_value[pushed].x,  0,'f',3)
          .arg(_value[pushed].y,  0,'f',3)
          .arg(_value[pushed].base);
      break;
    }
  return LeafMeta::format(local,global,foo);
}

void PointerMeta::doc(QStringList &out, QString preamble)
{
  out << preamble +  " (TOP_LEFT|TOP_RIGHT|BOTTOM_LEFT|BOTTOM_RIGHT) <floatX> <floatY> <intBase>";
  out << preamble + " (TOP|BOTTOM|LEFT|RIGHT) <floatLoc> <floatX> <floatY> <intBase>";
}

/* ------------------ */ 

FreeFormMeta::FreeFormMeta() : LeafMeta()
{
  _value[0].mode = false;
}
Rc FreeFormMeta::parse(QStringList &argv, int index,Where &here)
{
  Rc rc = FailureRc;
  if (argv.size() - index == 1 && argv[index] == "FALSE") {
      _value[pushed].mode = false;
      rc = OkRc;
    } else if (argv.size() - index == 2) {
      _value[pushed].mode = true;
      QRegExp rx("^(STEP_NUMBER|ASSEM|PLI|ROTATE_ICON)$");
      if (argv[index].contains(rx)) {
          rx.setPattern("^(LEFT|RIGHT|TOP|BOTTOM|CENTER)$");
          if (argv[index+1].contains(rx)) {
              _value[pushed].base = PlacementEnc(tokenMap[argv[index]]);
              _value[pushed].justification = PlacementEnc(tokenMap[argv[index+1]]);
              rc = OkRc;
            }
        }
    }
  if (rc == OkRc) {
      _here[pushed] = here;
    }
  return rc;
}
QString FreeFormMeta::format(bool local, bool global)
{
  QString foo;
  if (_value[pushed].mode) {
      foo = relativeNames[_value[pushed].base] + " " +
          placementNames[_value[pushed].justification];
    } else {
      foo = "FALSE";
    }
  return LeafMeta::format(local,global,foo);
}
void FreeFormMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " (FALSE|(STEP_NUMBER|ASSEM|PLI|ROTATE_ICON) (LEFT|RIGHT|TOP|BOTTOM|CENTER))";
}

/* ------------------ */ 

ConstrainMeta::ConstrainMeta()
{
  _value[0].type = ConstrainData::PliConstrainArea;
  _default = true;
}
Rc ConstrainMeta::parse(QStringList &argv, int index,Where &here)
{
  Rc rc = FailureRc;
  bool ok;
  QRegExp rx;
  switch(argv.size() - index) {
    case 1:
      rx.setPattern("^(AREA|SQUARE)$");
      if (argv[index].contains(rx)) {
          _value[pushed].type = ConstrainData::PliConstrain(tokenMap[argv[index]]);
          rc = OkRc;
        }
      break;
    case 2:
      argv[index+1].toFloat(&ok);
      if (ok) {
          rx.setPattern("^(WIDTH|HEIGHT|COLS)$");
          if (argv[index].contains(rx)) {
              _value[pushed].type = ConstrainData::PliConstrain(tokenMap[argv[index]]);
              _value[pushed].constraint = argv[index+1].toFloat(&ok);
              rc = OkRc;
            }
        }
      break;
    }
  if (rc == OkRc) {
      _here[pushed] = here;
      _default = false;
    }
  return rc;
}
QString ConstrainMeta::format(bool local, bool global)
{
  QString foo;
  switch (_value[pushed].type) {
    case ConstrainData::PliConstrainArea:
      foo = "AREA";
      break;
    case ConstrainData::PliConstrainSquare:
      foo = "SQUARE";
      break;
    case ConstrainData::PliConstrainWidth:
      foo = QString("WIDTH %1") .arg(_value[pushed].constraint);
      break;
    case ConstrainData::PliConstrainHeight:
      foo = QString("HEIGHT %1") .arg(_value[pushed].constraint);
      break;
    default:
      foo = QString("COLS %1") .arg(_value[pushed].constraint);
      break;
    }
  return LeafMeta::format(local,global,foo);
}
void ConstrainMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " (AREA|SQUARE|(WIDTH|HEIGHT|COLS) <integer>)";
}

/* ------------------ */ 

AllocMeta::AllocMeta() : LeafMeta()
{
  type[0] = Vertical;
}
Rc AllocMeta::parse(QStringList &argv, int index, Where &here)
{
  QRegExp rx("^(HORIZONTAL|VERTICAL)$");
  if (argv.size() - index == 1 && argv[index].contains(rx)) {
      type[pushed] = AllocEnc(tokenMap[argv[index]]);
      _here[pushed] = here;
      return OkRc;
    }
  if (reportErrors) {
      QMessageBox::warning(NULL,
                           QMessageBox::tr("LPub3D"),
                           QMessageBox::tr("Expected HORIZONTAL or VERTICAL got \"%1\" in \"%2\"") .arg(argv[index]) .arg(argv.join(" ")));
    }
  return FailureRc;
}
QString AllocMeta::format(bool local, bool global)
{
  QString foo;
  if (type[pushed] == Horizontal) {
      foo = "HORIZONTAL";
    } else {
      foo = "VERTICAL";
    }
  return LeafMeta::format(local,global,foo);
}
void AllocMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " (HORIZONTAL|VERTICAL)";
}

/* ------------------ */ 

PageOrientationMeta::PageOrientationMeta() : LeafMeta()
{
  type[0] = Landscape;
}
Rc PageOrientationMeta::parse(QStringList &argv, int index, Where &here)
{
  QRegExp rx("^(PORTRAIT|LANDSCAPE)$");
  if (argv.size() - index == 1 && argv[index].contains(rx)) {
      type[pushed] = OrientationEnc(tokenMap[argv[index]]);
      _here[pushed] = here;
      return OkRc;
    }
  if (reportErrors) {
      QMessageBox::warning(NULL,
                           QMessageBox::tr("LPub3D"),
                           QMessageBox::tr("Expected PORTRAIT or LANDSCAPE got \"%1\" in \"%2\"") .arg(argv[index]) .arg(argv.join(" ")));
    }
  return FailureRc;
}
QString PageOrientationMeta::format(bool local, bool global)
{
  QString foo;
  if (type[pushed] == Portrait) {
      foo = "PORTRAIT";
    } else {
      foo = "LANDSCAPE";
    }
  return LeafMeta::format(local,global,foo);
}
void PageOrientationMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " (PORTRAIT|LANDSCAPE)";
}

/* ------------------ */

SepMeta::SepMeta() : LeafMeta()
{
  _value[0].thickness = DEFAULT_THICKNESS;
  _value[0].color = "black";
  _value[0].margin[0] = DEFAULT_MARGIN;
  _value[0].margin[1] = DEFAULT_MARGIN;
}
Rc SepMeta::parse(QStringList &argv, int index,Where &here)
{
  bool ok[3];
  if (argv.size() - index == 4) {
      argv[index  ].toFloat(&ok[0]);
      argv[index+2].toFloat(&ok[1]);
      argv[index+3].toFloat(&ok[2]);

      if (ok[0] && ok[1] && ok[2]) {
          _value[pushed].thickness = argv[index].toFloat(&ok[0]);
          _value[pushed].color     = argv[index+1];
          _value[pushed].margin[0] = argv[index+2].toFloat(&ok[0]);
          _value[pushed].margin[1] = argv[index+3].toFloat(&ok[0]);
          _here[pushed] = here;
          return OkRc;
        }
    }
  if (reportErrors) {
      QMessageBox::warning(NULL,
                           QMessageBox::tr("LPub3D"),
                           QMessageBox::tr("Malformed separator \"%1\"") .arg(argv.join(" ")));
    }
  return FailureRc;
}
QString SepMeta::format(bool local, bool global)
{
  QString foo = QString("%1 %2 %3 %4")
      .arg(_value[pushed].thickness)
      .arg(_value[pushed].color)
      .arg(_value[pushed].margin[0])
      .arg(_value[pushed].margin[1]);
  return LeafMeta::format(local,global,foo);
}
void SepMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " <intThickness> <color> <marginX> <marginY>";
}

/* ------------------ */ 

/*
 *  INSERT (
 *   PICTURE "name" (SCALE x) |
 *   TEXT "text" "font" color  |
 *   ARROW HDX HDY TLX TLY HD HFX HFY |
 *   BOM
 *   MODEL)
 *   OFFSET x y
 */

Rc InsertMeta::parse(QStringList &argv, int index, Where &here)
{ 
  InsertData insertData;
  Rc rc = OkRc;

  bool unsupportedInsertCoverPage = false;

  if (argv.size() - index == 1) {
      if (argv[index] == "PAGE") {
          return InsertPageRc;
        } else if (argv[index] == "COVER_PAGE") {
          unsupportedInsertCoverPage = true;
          rc = FailureRc;
        }
    } else if (argv.size() - index == 2) {
      if (argv[index] == "COVER_PAGE") {
          return InsertCoverPageRc;
        }
    }

  if (argv.size() - index > 1 && argv[index] == "PICTURE") {
      insertData.type = InsertData::InsertPicture;
      insertData.picName = argv[++index];
      ++index;
      if (argv.size() - index >= 2 && argv[index] == "SCALE") {
          bool good;
          insertData.picScale = argv[++index].toFloat(&good);
          ++index;

          if (! good) {
              rc = FailureRc;
            }
        }
    } else if (argv.size() - index > 3 && argv[index] == "TEXT") {
      insertData.type       = InsertData::InsertText;
      insertData.text       = argv[++index];
      insertData.textFont   = argv[++index];
      insertData.textColor  = argv[++index];
      ++index;
    } else if (argv[index] == "ROTATE_ICON"){
      insertData.type = InsertData::InsertRotateIcon;
      ++index;
    } else if (argv.size() - index >= 8 && argv[index] == "ARROW") {
      insertData.type = InsertData::InsertArrow;
      bool good, ok;
      insertData.arrowHead.setX(argv[++index].toFloat(&good));
      insertData.arrowHead.setY(argv[++index].toFloat(&ok));
      good &= ok;
      insertData.arrowTail.setX(argv[++index].toFloat(&ok));
      good &= ok;
      insertData.arrowTail.setY(argv[++index].toFloat(&ok));
      good &= ok;
      insertData.haftingDepth = argv[++index].toFloat(&ok);
      good &= ok;
      insertData.haftingTip.setX(argv[++index].toFloat(&ok));
      good &= ok;
      insertData.haftingTip.setY(argv[++index].toFloat(&ok));
      good &= ok;
      if ( ! good) {
          rc = FailureRc;
        }
      ++index;

    } else if (argv[index] == "BOM") {
      insertData.type = InsertData::InsertBom;
      ++index;
    } else if (argv[index] == "MODEL"){
      insertData.type = InsertData::InsertFinalModel;
      ++index;
    }

  if (rc == OkRc) {
      if (argv.size() - index == 3 && argv[index] == "OFFSET") {
          bool ok[2];
          insertData.offsets[0] = argv[++index].toFloat(&ok[0]);
          insertData.offsets[1] = argv[++index].toFloat(&ok[1]);
          if ( ! ok[0] || ! ok[1]) {
              rc = FailureRc;
            }
        } else if (argv.size() - index > 0) {
          rc = FailureRc;
        }
    }

  if (rc == OkRc) {
      _value = insertData;
      _here[0] = here;

      return InsertRc;
    } else {
      QString coverPageMsg = "";
      if (unsupportedInsertCoverPage){
         coverPageMsg = "LPub3D does not support the LPub meta \n0 !LPUB INSERT COVER_PAGE.\n"
                        "Please update to\n 0 !LPUB INSERT COVER_PAGE FRONT (or BACK) accordingly.";
        }
      if (reportErrors) {
          QMessageBox::warning(NULL,
                               QMessageBox::tr("LPub3D"),
                               QMessageBox::tr("Malformed Insert metacommand \"%1\"\n"
                                               "%2") .arg(argv.join(" ")).arg(coverPageMsg));
        }
      return FailureRc;
    }
}

QString InsertMeta::format(bool local, bool global)
{
  QString foo;
  switch (_value.type) {
    case InsertData::InsertPicture:
      foo += " PICTURE \"" + _value.picName + "\"";
      if (_value.picScale) {
          foo += QString(" SCALE %1") .arg(_value.picScale);
        }
      break;
    case InsertData::InsertText:
      foo += QString("TEXT \"%1\" \"%2\" \"%3\"") .arg(_value.text)
          .arg(_value.textFont)
          .arg(_value.textColor);
      break;
    case InsertData::InsertRotateIcon:
      foo += " ROTATE_ICON";
      break;
    case InsertData::InsertArrow:
      foo += " ARROW";
      foo += QString(" %1") .arg(_value.arrowHead.rx());
      foo += QString(" %1") .arg(_value.arrowHead.ry());
      foo += QString(" %1") .arg(_value.arrowTail.rx());
      foo += QString(" %1") .arg(_value.arrowTail.ry());
      foo += QString(" %1") .arg(_value.haftingDepth);
      foo += QString(" %1") .arg(_value.haftingTip.rx());
      foo += QString(" %1") .arg(_value.haftingTip.ry());
      break;
    case InsertData::InsertBom:
      foo += " BOM";
      break;
    case InsertData::InsertFinalModel:
      foo += " MODEL";
      break;
    case InsertData::InsertCoverPage:
      break;
    }

  if (_value.offsets[0] || _value.offsets[1]) {
      foo += QString(" OFFSET %1 %2") .arg(_value.offsets[0])
          .arg(_value.offsets[1]);
    }

  return LeafMeta::format(local,global,foo);
}

void InsertMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " <placement> PICTURE \"name\"|ARROW x y x y |BOM|TEXT|MODEL|ROTATE_ICON \"\" \"\"";
}

/* ------------------ */

Rc AlignmentMeta::parse(QStringList &argv, int index, Where &here)
{
  if (argv.size() - index == 1) {
      if (argv[index] == "LEFT") {
          _value[pushed] = Qt::AlignLeft;
        } else if (argv[index] == "CENTER") {
          _value[pushed] = Qt::AlignCenter;
        } else if (argv[index] == "RIGHT") {
          _value[pushed] = Qt::AlignRight;
        }
      _here[pushed] = here;
      return OkRc;
    } else {
      return FailureRc;
    }
}

QString AlignmentMeta::format(bool local, bool global)
{
  QString foo;
  
  switch (_value[pushed]) {
    case Qt::AlignLeft:
      foo = " LEFT";
      break;
    case Qt::AlignCenter:
      foo = " CENTER";
      break;
    default:
      foo = " RIGHT";
    }
  
  return LeafMeta::format(local,global,foo);
}

void AlignmentMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + "(LEFT|CENTER|RIGHT)";
}

/* ------------------ */ 

void TextMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent,name);
  font.init(     this,"FONT");
  color.init(    this,"COLOR");
  alignment.init(this,"ALIGNMENT");
}

/* ------------------ */

Rc ArrowHeadMeta::parse(QStringList &argv, int index, Where &here)
{
  if (argv.size() - index == 4) {
      qreal head[4];
      bool  good, ok;

      head[0] = argv[index  ].toFloat(&good);
      head[1] = argv[index+1].toFloat(&ok);
      good &= ok;
      head[2] = argv[index+2].toFloat(&ok);
      good &= ok;
      head[3] = argv[index+3].toFloat(&ok);
      good &= ok;

      if ( ! good) {
          return FailureRc;
        }

      _value[pushed][0] = head[0];
      _value[pushed][1] = head[1];
      _value[pushed][2] = head[2];
      _value[pushed][3] = head[3];
      _here[pushed] = here;

      return OkRc;
    }
  return FailureRc;
}

QString ArrowHeadMeta::format(bool local, bool global)
{
  QString foo;
  foo  = QString(" %1 %2 %3 %4") .arg(_value[pushed][0]);
  foo += QString(" %1 %2 %3 %4") .arg(_value[pushed][1]);
  foo += QString(" %1 %2 %3 %4") .arg(_value[pushed][2]);
  foo += QString(" %1 %2 %3 %4") .arg(_value[pushed][3]);
  return LeafMeta::format(local,global,foo);
}

void ArrowHeadMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + "TipX HaftingInsideX HaftingOutsideX HaftingOutsideY";
}

/* ------------------ */ 

Rc ArrowEndMeta::parse(QStringList &argv, int index, Where &here)
{
  if (argv.size() - index == 1) {
      if (argv[0] == "SQUARE") {
          _value[pushed] = false;
        } else if (argv[0] == "ROUND") {
          _value[pushed] = true;
        } else {
          return FailureRc;
        }
      _here[pushed] = here;
      return OkRc;
    }
  return FailureRc;
}

QString ArrowEndMeta::format(bool local, bool global)
{
  QString foo;
  
  if (_value[pushed]) {
      foo = "ROUND";
    } else {
      foo = "SQUARE";
    }
  return LeafMeta::format(local,global,foo);
}

void ArrowEndMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + "(SQUARE|ROUND)";
}

/* ------------------ */ 

CalloutCsiMeta::CalloutCsiMeta() : BranchMeta()
{
}

void CalloutCsiMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  placement.init(this,"PLACEMENT");
  margin.init   (this,"MARGINS");
}

/* ------------------ */ 

CalloutPliMeta::CalloutPliMeta() : BranchMeta()
{
  placement.setValue(TopOutside,CsiType);
  perStep.setValue(true);
}

void CalloutPliMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  placement.init(this,"PLACEMENT");
  margin.init(   this,"MARGINS");
  perStep.init  (this,"PER_STEP");
}

/* ------------------ */ 

void PliBeginMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  ignore.init(this, "IGN",    PliBeginIgnRc);
  sub.init   (this, "SUB");
}

/* ------------------ */ 

NumberMeta::NumberMeta() : BranchMeta()
{
  color.setValue("black");
  // font - default
}

void NumberMeta::init(
    BranchMeta *parent,
    QString name)
{
  AbstractMeta::init(parent, name);
  color.init    (this, "FONT_COLOR");
  font.init     (this, "FONT",OkRc, "\"");
  margin.init   (this, "MARGINS");
}

NumberPlacementMeta::NumberPlacementMeta() : NumberMeta()
{
  placement.setValue(TopLeftInsideCorner,PageType);
  color.setValue("black");
  // font - default
}

void NumberPlacementMeta::init(
    BranchMeta *parent,
    QString name)
{
  AbstractMeta::init(parent, name);
  placement.init(this, "PLACEMENT");
  color.init    (this, "FONT_COLOR");
  font.init     (this, "FONT",OkRc, "\"");
  margin.init   (this, "MARGINS");
}

/* ------------------ */

PageAttributeTextMeta::PageAttributeTextMeta() : BranchMeta()
{
  placement.setValue(TopLeftInsideCorner,PageType);
  display.setValue(Preferences::displayAllAttributes);
  textColor.setValue("black");
  placement.value().placement     = TopLeft;
  placement.value().justification = Center;
  placement.value().preposition   = Inside;
  placement.value().relativeTo    = PageType;
  placement.value().offsets[0]    = 0.0;
  placement.value().offsets[1]    = 0.0;
}

void PageAttributeTextMeta::init(
    BranchMeta *parent,
    QString name)
{
  AbstractMeta::init(parent, name);
  textFont.init     	(this, "FONT",OkRc, "\"");
  textColor.init    	(this, "COLOR");
  margin.init   	(this, "MARGINS");
  placement.init        (this, "PLACEMENT");
  content.init          (this, "CONTENT");
  display.init          (this, "DISPLAY");
}

/* ------------------ */

PageAttributePictureMeta::PageAttributePictureMeta() : BranchMeta()
{
  placement.setValue(TopLeftInsideCorner,PageType);
  display.setValue(Preferences::displayAllAttributes);
  picScale.setRange(-10000.0,10000.0);
  picScale.setFormats(7,4,"99999.9");
  picScale.setValue(1.0);
  margin.setValuesInches(0.0f,0.0f);
  placement.value().placement     = TopLeft;
  placement.value().justification = Center;
  placement.value().preposition   = Inside;
  placement.value().relativeTo    = PageType;
  placement.value().offsets[0]    = 0.0;
  placement.value().offsets[1]    = 0.0;
  tile.setValue(false);
  stretch.setValue(false);
}

void PageAttributePictureMeta::init(
    BranchMeta *parent,
    QString name)
{
  AbstractMeta::init(parent, name);
  placement.init        (this, "PLACEMENT");
  margin.init   	(this, "MARGINS");
  picScale.init         (this, "SCALE");
  file.init             (this, "FILE");
  display.init          (this, "DISPLAY");
  stretch.init          (this, "STRETCH");
  tile.init             (this, "TILE");
}

/* ------------------ */

PageHeaderMeta::PageHeaderMeta() : BranchMeta()
{
  placement.setValue(TopInside,PageType);
  size.setValuesInches(8.5f,0.3f);
  size.setRange(.1,1000);
  size.setFormats(6,4,"9.9999");
}

void PageHeaderMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  placement.init        (this, "PLACEMENT");
  size.init             (this, "SIZE");
}

/* ------------------ */

PageFooterMeta::PageFooterMeta() : BranchMeta()
{
  placement.setValue(BottomInside,PageType);
  size.setValuesInches(8.5f,0.3f);
  size.setRange(.1,1000);
  size.setFormats(6,4,"9.9999");
}

void PageFooterMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  placement.init        (this, "PLACEMENT");
  size.init             (this, "SIZE");
}

/* ------------------ */

FadeStepMeta::FadeStepMeta() : BranchMeta()
{
  fadeColor.setValue(Preferences::fadeStepColor); // inherited from properties
  fadeStep.setValue(Preferences::enableFadeStep); // inherited from properties
}

void FadeStepMeta::init(
    BranchMeta *parent,
    QString name)
{
  AbstractMeta::init(parent, name);
  fadeColor.init    (this, "FADE_COLOR");
  fadeStep.init     (this, "FADE");
}

/* ------------------ */

void RemoveMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  group.init(   this,"GROUP",RemoveGroupRc);
  parttype.init(this,"PART", RemovePartRc);
  partname.init(this,"NAME", RemoveNameRc);
}

/* ------------------ */

PartMeta::PartMeta() : BranchMeta()
{
}

void PartMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent,name);
  margin.init(this,"MARGINS");
}

/* ------------------ */ 

Rc SubMeta::parse(QStringList &argv, int index,Where &here)
{
  Rc rc = FailureRc;
  int argc = argv.size() - index;

  if (argc == 1) {
      _value.part = argv[index];
      _value.color = "";
      _value.type = rc = PliBeginSub1Rc;
    } else if (argc == 2) {
      _value.part  = argv[index];
      _value.color = argv[index+1];
      _value.type = rc = PliBeginSub2Rc;
    }
  if (rc != FailureRc) {
      _here[0] = here;
      _here[1] = here;
    }
  return rc;
}

QString SubMeta::format(bool local, bool global)
{
  QString foo;
  
  if (_value.type == PliBeginSub1Rc) {
      foo = _value.part;
    } else {
      foo = _value.color + " " + _value.part;
    }
  return LeafMeta::format(local,global,foo);
}

void SubMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " <part> <color>";
  out << preamble + " <part>";
}

/* ------------------ */ 

void PartBeginMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  ignore.init(this, "IGN",   PartBeginIgnRc);
}

/* ------------------ */ 

void PartIgnMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  begin.init(this, "BEGIN");
  end  .init(this, "END", PartEndRc);
}

/* ------------------ */ 

Rc RotStepMeta::parse(QStringList &argv, int index,Where &here)
{
  if (index + 4 == argv.size()) {
      bool ok[3];
      argv[index+0].toFloat(&ok[0]);
      argv[index+1].toFloat(&ok[1]);
      argv[index+2].toFloat(&ok[2]);
      ok[0] &= ok[1] & ok[2];
      QRegExp rx("^(ABS|REL|ADD)$");
      if (ok[0] && argv[index+3].contains(rx)) {
          _value.rots[0] = argv[index+0].toFloat(&ok[0]);
          _value.rots[1] = argv[index+1].toFloat(&ok[1]);
          _value.rots[2] = argv[index+2].toFloat(&ok[2]);
          _value.type = argv[index+3];
          _here[0] = here;
          _here[1] = here;
          return RotStepRc;
        }
    } else if (argv.size()-index == 1 && argv[index] == "END") {
      _value.type.clear();
      _value.rots[0] = 0;
      _value.rots[1] = 0;
      _value.rots[2] = 0;
      _here[0] = here;
      _here[1] = here;
      return RotStepRc;
    }
  if (reportErrors) {
      QMessageBox::warning(NULL,
                           QMessageBox::tr("LPub3D"),
                           QMessageBox::tr("Malformed rotation step \"%1\"") .arg(argv.join(" ")));
    }
  return FailureRc;
}
QString RotStepMeta::format(bool local, bool global)
{
  QString foo = QString("%1 %2 %3 %4")
      .arg(_value.rots[0]) .arg(_value.rots[1])
      .arg(_value.rots[2]) .arg(_value.type);
  return LeafMeta::format(local,global,foo);
}

void RotStepMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " <rotX> <rotY> <rotZ> <ABS|REL|ADD>";
}

/* ------------------ */ 
Rc BuffExchgMeta::parse(QStringList &argv, int index,Where &here)
{
  if (index + 2 == argv.size()) {
      QRegExp b("^[A-Z]$");
      QRegExp t("^(STORE|RETRIEVE)$");
      if (argv[index].contains(b) && argv[index+1].contains(t)) {
          _value.buffer = argv[index];
          _here[0] = here;
          _here[1] = here;
          if (argv[index+1] == "RETRIEVE") {
              return BufferLoadRc;
            } else {
              return BufferStoreRc;
            }
        }
    }
  if (reportErrors) {
      QMessageBox::warning(NULL,
                           QMessageBox::tr("LPub3D"),
                           QMessageBox::tr("Malformed buffer exchange \"%1\"") .arg(argv.join(" ")));
    }
  return FailureRc;
}
QString BuffExchgMeta::format(bool local, bool global)
{
  QString foo = _value.buffer + " " + _value.type;
  return LeafMeta::format(local,global,foo);
}

void BuffExchgMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + " <bufferName> <STORE|RETRIEVE>";
}

/* ------------------ */

PliSortMeta::PliSortMeta() : BranchMeta()
{
  sortOption.setValue("Part Size");
}

void PliSortMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  sortOption.init    (this, "SORT_OPTION");
}

/* ------------------ */

PliAnnotationMeta::PliAnnotationMeta() : BranchMeta()
{
  titleAnnotation.setValue           (true);
  freeformAnnotation.setValue        (false);
  titleAndFreeformAnnotation.setValue(false);
  display.setValue                   (false);
}

void PliAnnotationMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  titleAnnotation.init            (this, "USE_TITLE");
  freeformAnnotation.init         (this, "USE_FREE_FORM");
  titleAndFreeformAnnotation.init (this, "USE_TITLE_AND_FREE_FORM");
  display.init                    (this, "DISPLAY");
}

/* ------------------ */

RotateIconMeta::RotateIconMeta() : BranchMeta()
{
  placement.setValue(RightOutside,CsiType);   // right outside single step
  BorderData borderData;
  borderData.type = BorderData::BdrRound;
  borderData.line = BorderData::BdrLnSolid;
  borderData.color = "Black";
  borderData.thickness = DEFAULT_THICKNESS;
  borderData.radius = 10;
  borderData.margin[0] = DEFAULT_MARGIN;
  borderData.margin[1] = DEFAULT_MARGIN;
  border.setValueInches(borderData);
  background.setValue(BackgroundData::BgTransparent);
  margin.setValuesInches(0.0f,0.0f);
  BorderData arrowData;
  arrowData.type = BorderData::BdrSquare;
  arrowData.line = BorderData::BdrLnSolid;
  arrowData.color = "Blue";
  arrowData.thickness = DEFAULT_THICKNESS;
  arrowData.margin[0] = DEFAULT_MARGIN;
  arrowData.margin[1] = DEFAULT_MARGIN;
  arrow.setValueInches(arrowData);
  display.setValue(true);
  size.setValuesInches(0.52f,0.52f);
  size.setRange(1,1000);
  size.setFormats(6,4,"9.9999");
  picScale.setRange(-10000.0,10000.0);
  picScale.setFormats(7,4,"99999.9");
  picScale.setValue(1.0);
  subModelColor.setValue("#ffffff");
  subModelColor.setValue("#ffffcc");
  subModelColor.setValue("#ffcccc");
  subModelColor.setValue("#ccccff");
}

void RotateIconMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  size            .init(this,"SIZE");
  arrow           .init(this,"ARROW");
  placement       .init(this,"PLACEMENT");
  border          .init(this,"BORDER");
  background      .init(this,"BACKGROUND");
  margin          .init(this,"MARGINS");
  display         .init(this,"DISPLAY");
  picScale        .init(this,"SCALE");
}

/*---------------------------------------------------------------
 * The Top Level LPub Metas
 *---------------------------------------------------------------*/

PageMeta::PageMeta() : BranchMeta()
{
  size.setValuesInches(8.5f,11.0f);
  size.setRange(1,1000);
  size.setFormats(6,4,"9.9999");
  orientation.setValue(Portrait);

  BorderData borderData;
  borderData.type = BorderData::BdrNone;
  borderData.line = BorderData::BdrLnNone;
  borderData.color = "Black";
  borderData.thickness = 0;
  borderData.radius = 0;
  borderData.margin[0] = DEFAULT_MARGIN;
  borderData.margin[1] = DEFAULT_MARGIN;
  border.setValueInches(borderData);

  background.setValue(BackgroundData::BgSubmodelColor);
  dpn.setValue(true);
  togglePnPlacement.setValue(false);
  number.placement.setValue(BottomRightInsideCorner,PageType);
  number.color.setValue("black");
  number.font.setValuePoints("Arial,24,-1,255,75,0,0,0,0,0");
  instanceCount.placement.setValue(TopLeftOutsideCorner,PageNumberType); //fooWas LeftBottomOutside,PageNumberType
  instanceCount.color.setValue("black");
  instanceCount.font.setValuePoints("Arial,48,-1,255,75,0,0,0,0,0");

  subModelColor.setValue("#ffffff");
  subModelColor.setValue("#ffffcc");
  subModelColor.setValue("#ffcccc");
  subModelColor.setValue("#ccccff");

  /* PAGE ATTRIBUTE FORMAT
   *
   * Front Cover Default Attribute Placements
   *************************************
   *   Logo (Top Left of Page)         *
   *                                   *
   *   ModelName                       *  (Top of Title)
   *   Title                           *  (Left Center of Page)
   *   Author                          *  (Bottom of Title)
   *   Pieces                          *  (Bottom of Author)
   *   Model Description               *  (Bottom of Pieces)
   *   Publsiher Description           *  (Bottom of Model Description)
   *                                   *
   *************************************


   * Header/Footer Default Attribute Placements
   * ***********************************
   * URL (Top Left of Page)            Email (Top Right of Page)
   * ***********************************
   * *                                 *
   * *                                 *
   * *                                 *
   * *                                 *
   * ***********************************
   * Copyright (Bottom Left of Page)   Author (Bottom Right of Page)
   * ***********************************


   * Back Cover Default Attribute Placements
   * **********************************
   *               Logo               *  (Top of Page)
   *  Title                           *  (Top of Author)
   *  Author                          *  (Top of Copyright)
   *  Copyright                       *  (Top of URL)
   *  URL                             *  (Top of LEGO Disclaimer)
   *  Email                           *
   *  LEGO Disclaimer                 *  (Center of Page)
   *  LPub3D Plug                     *  (Below LEGO Disclaimer)
   *  LPub3D Plug Image               *  (Right of LPub3D Plug)
   *                                  *
   ************************************


   * Page Attribute relative Tos
   *
   *  FRONT COVER PAGE
   *  documentLogoFront.placement  (BottomLeftOutside,  PageHeaderType)
   *  modelName.placement          (TopLeftOutside,	PageTitleType)
   *  titleFront.placement         (LeftInside,         PageType)
   *  authorFront.placement        (BottomLeftOutside,	PageTitleType)
   *  pieces.placement             (BottomLeftOutside,	PageAuthorType)
   *  modelDesc.placement          (BottomLeftOutside,	PagePiecesType)
   *  publishDesc.placement        (BottomLeftOutside,	PageModelDescType)
   *
   *  BACK COVER PAGE
   *  documentLogoBack.placement   (BottomOutside,	PageHeaderType)
   *  titleBack.placement          (CenterCenter,	PageType)
   *  authorBack.placement         (BottomOutside,      PageTitleType)
   *  copyrightBack.placement      (BottomOutside,      PageAuthorType)
   *  urlBack.placement            (BottomOutside,      PageCopyrightType)
   *  emailBack.placement          (BottomOutside,      PageURLType)
   *  disclaimer.placement         (BottomOutside,      PageEmailType)
   *  plug.placement               (BottomOutside,	PageDisclaimerType)
   *  plugImage.placement          (BottomOutside,	PagePlugType)
   *
   *  HEADER
   *  url.placement                (TopLeftInsideCorner,	PageType)
   *  email.placement		   (TopRightInsideCorner,	PageType)
   *
   *  FOOTER
   *  copyright.placement	   (BottomLeftInsideCorner,      PageType)
   *  author.placement		   (LeftBottomOutside,     PageNumberType)
   *
   *  NOT PLACED
   *  category                     (TopLeftInsideCorner,       PageType)
   */

  //model titleFront text
  titleFront.placement.setValue(LeftInside,PageType);
  titleFront.type = PageTitleType;
  titleFront.textFont.setValuePoints("Arial,32,-1,255,75,0,0,0,0,0");
  titleFront.setValue(LDrawFile::_file);

  //model titleBack text
  titleBack.placement.setValue(CenterCenter,PageType);
  titleBack.type = PageTitleType;
  titleBack.textFont.setValuePoints("Arial,18,-1,255,75,0,0,0,0,0");
  titleBack.setValue(LDrawFile::_file);

  //model id/name text
  modelName.placement.setValue(TopLeftOutside,PageTitleType);
  modelName.type = PageModelNameType;
  modelName.textFont.setValuePoints("Arial,20,-1,255,75,0,0,0,0,0");
  modelName.setValue(LDrawFile::_name);

  //publisher authorFront text
  authorFront.placement.setValue(BottomLeftOutside,PageTitleType);
  authorFront.type = PageAuthorType;
  authorFront.textFont.setValuePoints("Arial,20,-1,255,75,0,0,0,0,0");
  authorFront.setValue(QString("Model instructions by %1").arg(Preferences::defaultAuthor));

  //publisher authorBack text
  authorBack.placement.setValue(BottomOutside,PageTitleType);
  authorBack.type = PageAuthorType;
  authorBack.textFont.setValuePoints("Arial,18,-1,255,75,0,0,0,0,0");
  authorBack.setValue(QString("Model instructions by %1").arg(Preferences::defaultAuthor));

  //publisher author text
  author.placement.setValue(LeftBottomOutside,PageNumberType);
  author.type = PageAuthorType;
  author.textFont.setValuePoints("Arial,18,-1,255,75,0,0,0,0,0");
  author.setValue(Preferences::defaultAuthor);

  //model number of pieces text
  pieces.placement.setValue(BottomLeftOutside,PageAuthorType);
  pieces.type = PagePiecesType;
  pieces.textFont.setValuePoints("Arial,20,-1,255,75,0,0,0,0,0");
  pieces.setValue(QString("%1 Pieces").arg(QString::number(LDrawFile::_pieces)));

  //model description text
  modelDesc.placement.setValue(BottomLeftOutside,PagePiecesType);
  modelDesc.type = PageModelDescType;
  modelDesc.textFont.setValuePoints("Arial,18,-1,255,75,0,0,0,0,0");
  modelDesc.setValue(LDrawFile::_description);

  //publisher description text
  publishDesc.placement.setValue(BottomLeftOutside,PageModelDescType);
  publishDesc.type = PagePublishDescType;
  publishDesc.textFont.setValuePoints("Arial,18,-1,255,75,0,0,0,0,0");
  publishDesc.setValue(Preferences::publishDescription);

  //publisher url text
  url.placement.setValue(TopLeftInsideCorner,PageType);
  url.type = PageURLType;
  url.textFont.setValuePoints("Arial,18,-1,255,75,0,0,0,0,0");
  url.setValue(Preferences::defaultURL);

  //publisher urlBack text
  urlBack.placement.setValue(BottomOutside,PageCopyrightType);
  urlBack.type = PageURLType;
  urlBack.textFont.setValuePoints("Arial,18,-1,255,75,0,0,0,0,0");
  urlBack.setValue(Preferences::defaultURL);

  //publisher email text
  email.placement.setValue(TopRightInsideCorner,PageType);
  email.type = PageEmailType;
  email.textFont.setValuePoints("Arial,18,-1,255,75,0,0,0,0,0");
  email.setValue(Preferences::defaultEmail);

  //publisher emailBack text
  emailBack.placement.setValue(BottomOutside,PageURLType);
  emailBack.type = PageEmailType;
  emailBack.textFont.setValuePoints("Arial,18,-1,255,75,0,0,0,0,0");
  emailBack.setValue(Preferences::defaultEmail);

  //publisher copyright text
  copyright.placement.setValue(BottomLeftInsideCorner,PageType);
  copyright.type = PageCopyrightType;
  copyright.textFont.setValuePoints("Arial,18,-1,255,75,0,0,0,0,0");
  copyright.setValue(Preferences::copyright + " by " + Preferences::defaultAuthor);

  //publisher copyrightBack text
  copyrightBack.placement.setValue(BottomOutside,PageAuthorType);
  copyrightBack.type = PageCopyrightType;
  copyrightBack.textFont.setValuePoints("Arial,18,-1,255,75,0,0,0,0,0");
  copyrightBack.setValue(Preferences::copyright);

  //plug text
  plug.placement.setValue(BottomOutside,PageDisclaimerType);
  plug.type = PagePlugType;
  plug.textFont.setValuePoints("Arial,16,-1,255,75,0,0,0,0,0");
  plug.setValue(Preferences::plug);

  //disclaimer text
  disclaimer.placement.setValue(BottomOutside,PageEmailType);
  disclaimer.type = PageDisclaimerType;
  disclaimer.textFont.setValuePoints("Arial,18,-1,255,75,0,0,0,0,0");
  disclaimer.setValue(Preferences::disclaimer);

  //publisher cover IMAGE
  coverImage.placement.setValue(CenterCenter,PageType);
  coverImage.type = PageCoverImageType;

  //publisher documentLogoFront IMAGE
  documentLogoFront.placement.setValue(BottomLeftOutside,PageHeaderType);
  documentLogoFront.type = PageDocumentLogoType;
  documentLogoFront.file.setValue(Preferences::documentLogoFile);

  //publisher documentLogoBack IMAGE
  documentLogoBack.placement.setValue(BottomOutside,PageHeaderType);
  documentLogoBack.picScale.setValue(0.5);
  documentLogoBack.type = PageDocumentLogoType;
  documentLogoBack.file.setValue(Preferences::documentLogoFile);

  //plug IMAGE
  plugImage.placement.setValue(BottomOutside,PagePlugType);
  plugImage.type = PagePlugImageType;
  plugImage.file.setValue(Preferences::plugImage);

  //model category text (NOT USED)
  category.placement.setValue(BottomLeftOutside,PageType);
  category.type = PageCategoryType;
  category.textFont.setValuePoints("Arial,18,-1,255,75,0,0,0,0,0");
  category.setValue(LDrawFile::_category);
}

void PageMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  size.init               (this, "SIZE");
  orientation.init        (this, "ORIENTATION");
  margin.init             (this, "MARGINS");
  border.init             (this, "BORDER");
  background.init         (this, "BACKGROUND");
  dpn.init                (this, "DISPLAY_PAGE_NUMBER");
  togglePnPlacement.init  (this, "TOGGLE_PAGE_NUMBER_PLACEMENT");
  number.init             (this, "NUMBER");
  instanceCount.init      (this, "SUBMODEL_INSTANCE_COUNT");
  subModelColor.init      (this, "SUBMODEL_BACKGROUND_COLOR");

  pageHeader.init         (this, "PAGE_HEADER");
  pageFooter.init         (this, "PAGE_FOOTER");

  titleFront.init         (this, "DOCUMENT_TITLE_FRONT");
  titleBack.init          (this, "DOCUMENT_TITLE_BACK");
  modelName.init          (this, "MODEL_ID");
  modelDesc.init          (this, "MODEL_DESCRIPTION");
  pieces.init             (this, "MODEL_PIECES");
  authorFront.init        (this, "DOCUMENT_AUTHOR_FRONT");
  authorBack.init         (this, "DOCUMENT_AUTHOR_BACK");
  author.init             (this, "DOCUMENT_AUTHOR");
  publishDesc.init        (this, "PUBLISH_DESCRIPTION");
  url.init                (this, "PUBLISH_URL");
  urlBack.init            (this, "PUBLISH_URL_BACK");
  email.init              (this, "PUBLISH_EMAIL");
  emailBack.init          (this, "PUBLISH_EMAIL_BACK");
  copyrightBack.init      (this, "PUBLISH_COPYRIGHT_BACK");
  copyright.init          (this, "PUBLISH_COPYRIGHT");
  documentLogoFront.init  (this, "DOCUMENT_LOGO_FRONT");
  documentLogoBack.init   (this, "DOCUMENT_LOGO_BACK");
  coverImage.init         (this, "DOCUMENT_COVER_IMAGE");
  disclaimer.init         (this, "LEGO_DISCLAIMER");
  plug.init               (this, "APP_PLUG");
  plugImage.init          (this, "APP_PLUG_IMAGE");
  category.init           (this, "MODEL_CATEGORY" );
}

/* ------------------ */ 

AssemMeta::AssemMeta() : BranchMeta()
{
  placement.setValue(CenterCenter,PageType);
  modelScale.setRange(-10000.0,10000.0);
  modelScale.setFormats(7,4,"99999.9");
  modelScale.setValue(1.0);
  ldgliteParms.setValue("-fh"); // change removed -w1 duplicate on 01-25-16 v1.3.3 r578
  ldviewParms.setValue("");
  l3pParms.setValue("-q4 -sw2");
  povrayParms.setValue("+A");
  showStepNumber.setValue(true);
}
void AssemMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  margin.init        (this,"MARGINS");
  placement.init     (this,"PLACEMENT");
  modelScale.init    (this,"MODEL_SCALE");
  ldviewParms.init   (this,"LDGLITE_PARMS");
  ldgliteParms.init  (this,"LDVIEW_PARMS");
  l3pParms .init     (this,"L3P_PARMS");
  povrayParms .init  (this,"POVRAY_PARMS");
  showStepNumber.init(this,"SHOW_STEP_NUMBER");
}

/* ------------------ */

PliMeta::PliMeta() : BranchMeta()
{
  placement.setValue(RightTopOutside,StepNumberType);
  BorderData borderData;
  borderData.type = BorderData::BdrSquare;
  borderData.line = BorderData::BdrLnSolid;
  borderData.color = "Black";
  borderData.thickness = DEFAULT_THICKNESS;
  borderData.radius = 15;
  borderData.margin[0] = DEFAULT_MARGIN;
  borderData.margin[1] = DEFAULT_MARGIN;
  border.setValueInches(borderData);
  background.setValue(BackgroundData::BgColor,"#ffffff");
  margin.setValuesInches(0.0f,0.0f);
  // instance - default
  // annotate - default
  modelScale.setRange(-10000.0,10000.0);
  modelScale.setFormats(7,4,"99999.9");
  modelScale.setValue(1.0);
  angle.setValues(23,-45);
  angle.setRange(-360.0,360.0);
  angle.setFormats(6,4,"#999.9");
  show.setValue(true);
  ldgliteParms.setValue("-fh -w1");
  ldviewParms.setValue("");
  l3pParms.setValue("-q4 -sw2");
  povrayParms.setValue("+A");
  includeSubs.setValue(false);
  subModelColor.setValue("#ffffff");
  subModelColor.setValue("#ffffcc");
  subModelColor.setValue("#ffcccc");
  subModelColor.setValue("#ccccff");
  part.margin.setValuesInches(0.05f,0.03f);
  instance.font.setValuePoints("Arial,36,-1,255,75,0,0,0,0,0");
  instance.margin.setValuesInches(0.0f,0.0f);
  //annotate.font.setValuePoints("Arial,36,-1,255,75,0,0,0,0,0");   // Rem at revision 226 11/06/15
  annotate.font.setValuePoints("Arial,24,-1,5,50,0,0,0,0,0");
  annotate.color.setValue("#3a3938");                               // Add at revision 285 01/07/15
  annotate.margin.setValuesInches(0.0f,0.0f);
  margin.setValuesInches(DEFAULT_MARGIN,DEFAULT_MARGIN);
  pack.setValue(true);
  sort.setValue(false);
  sortBy.setValue(SortOptionName[PartSize]);
}

void PliMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  placement       .init(this,"PLACEMENT");
  constrain       .init(this,"CONSTRAIN");
  border          .init(this,"BORDER");
  background      .init(this,"BACKGROUND");
  margin          .init(this,"MARGINS");
  instance        .init(this,"INSTANCE_COUNT");
  annotate        .init(this,"ANNOTATE");
  modelScale      .init(this,"MODEL_SCALE");
  angle           .init(this,"VIEW_ANGLE");
  show            .init(this,"SHOW");
  ldviewParms     .init(this,"LDVIEW_PARMS");
  ldgliteParms    .init(this,"LDGLITE_PARMS");
  l3pParms        .init(this,"L3P_PARMS");
  povrayParms     .init(this,"POVRAY_PARMS");
  includeSubs     .init(this,"INCLUDE_SUBMODELS");
  subModelColor   .init(this,"SUBMODEL_BACKGROUND_COLOR");
  part            .init(this,"PART");
  begin           .init(this,"BEGIN");
  end             .init(this,"END",           PliEndRc);
  sort            .init(this,"SORT");
  sortBy          .init(this,"SORT_BY");
  annotation      .init(this,"ANNOTATION");
}

/* ------------------ */ 

BomMeta::BomMeta() : PliMeta()
{
  placement.setValue(CenterCenter,PageType);
  BorderData borderData;
  borderData.type = BorderData::BdrSquare;
  borderData.line = BorderData::BdrLnSolid;
  borderData.color = "Black";
  borderData.thickness = DEFAULT_THICKNESS;
  borderData.radius = 15;
  borderData.margin[0] = DEFAULT_MARGIN;
  borderData.margin[1] = DEFAULT_MARGIN;
  border.setValueInches(borderData);
  background.setValue(BackgroundData::BgColor,"#ffffff");
  margin.setValuesInches(0.0f,0.0f);
  // instance - default
  // annotate - default
  modelScale.setRange(-10000.0,10000.0);
  modelScale.setFormats(7,4,"99999.9");
  modelScale.setValue(1.0);
  angle.setValues(23,-45);
  angle.setRange(-360.0,360.0);
  angle.setFormats(6,4,"#999.9");
  show.setValue(true);
  ldgliteParms.setValue("-fh -w1");
  ldviewParms.setValue("");
  l3pParms.setValue("-q4 -sw2");
  povrayParms.setValue("+A");
  includeSubs.setValue(false);
  subModelColor.setValue("#ffffff");
  subModelColor.setValue("#ffffcc");
  subModelColor.setValue("#ffcccc");
  subModelColor.setValue("#ccccff");
  part.margin.setValuesInches(0.05f,0.03f);
  instance.font.setValuePoints("Arial,24,-1,255,75,0,0,0,0,0");
  instance.margin.setValuesInches(0.0f,0.0f);
  //instance.font.setValuePoints("Arial,24,-1,255,75,0,0,0,0,0");   // Rem at revision 226 11/06/15
  annotate.font.setValuePoints("Arial,18,-1,5,50,0,0,0,0,0");
  annotate.color.setValue("#3a3938");                               // Add at revision 285 01/07/15
  annotate.margin.setValuesInches(0.0f,0.0f);
  pack.setValue(false);
  sort.setValue(true);
  sortBy.setValue("Part Size");
  sortBy.setValue(SortOptionName[PartColour]);
}

void BomMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  placement       .init(this,"PLACEMENT");
  constrain       .init(this,"CONSTRAIN");
  border          .init(this,"BORDER");
  background      .init(this,"BACKGROUND");
  margin          .init(this,"MARGINS");
  instance        .init(this,"INSTANCE_COUNT");
  annotate        .init(this,"ANNOTATE");
  modelScale      .init(this,"MODEL_SCALE");
  angle           .init(this,"VIEW_ANGLE");
  show            .init(this,"SHOW");
  ldviewParms     .init(this,"LDVIEW_PARMS");
  ldgliteParms    .init(this,"LDGLITE_PARMS");
  l3pParms        .init(this,"L3P_PARMS");
  povrayParms     .init(this,"POVRAY_PARMS");
  includeSubs     .init(this,"INCLUDE_SUBMODELS");
  subModelColor   .init(this,"SUBMODEL_BACKGROUND_COLOR");
  part            .init(this,"PART");
  begin           .init(this,"BEGIN");
  begin.ignore.rc = BomBeginIgnRc;
  end             .init(this,"END",BomEndRc);
  sort            .init(this,"SORT");
  sortBy          .init(this,"SORT_BY");
  annotation      .init(this,"ANNOTATION");
}

/* ------------------ */ 

Rc CalloutBeginMeta::parse(QStringList &argv, int index,Where &here)
{
  Rc rc = FailureRc;
  int argc = argv.size() - index;

  if (argc == 0) {
      rc = CalloutBeginRc;
      mode = Unassembled;
    } else if (argc == 1 && argv[index] == "ASSEMBLED") {
      rc = CalloutBeginRc;
      mode = Assembled;
    } else if (argc == 1 && argv[index] == "ROTATED") {
      rc = CalloutBeginRc;
      mode = Rotated;
    }
  if (rc != FailureRc) {
      _here[0] = here;
      _here[1] = here;
    }
  return rc;
}

QString CalloutBeginMeta::format(bool local, bool global)
{
  QString foo;

  if (mode == Assembled) {
      foo = "ASSEMBLED";
    } else if (mode == Rotated) {
      foo = "ROTATED";
    }
  return LeafMeta::format(local,global,foo);
}

void CalloutBeginMeta::doc(QStringList &out, QString preamble)
{
  out << preamble;
  out << preamble + " WHOLE";
}

CalloutMeta::CalloutMeta() : BranchMeta()
{
  stepNum.font.setValuePoints("Arial,36,-1,255,75,0,0,0,0,0");
  stepNum.color.setValue("black");
  // stepNum.font - default
  stepNum.placement.setValue(LeftTopOutside,PartsListType);
  sep.setValueInches("Black",DEFAULT_THICKNESS,DEFAULT_MARGINS);
  BorderData borderData;
  borderData.type = BorderData::BdrSquare;
  borderData.line = BorderData::BdrLnSolid;
  borderData.color = "Black";
  borderData.thickness = DEFAULT_THICKNESS;
  borderData.radius = 15;
  borderData.margin[0] = DEFAULT_MARGIN;
  borderData.margin[1] = DEFAULT_MARGIN;
  border.setValueInches(borderData);
  // subModelFont - default
  instance.font.setValuePoints("Arial,24,-1,255,75,0,0,0,0,0");
  instance.color.setValue("black");
  // instance - default
  instance.placement.setValue(RightBottomOutside, CalloutType);
  background.setValue(BackgroundData::BgSubmodelColor);
  subModelColor.setValue("#ffffff");
  subModelColor.setValue("#ffffcc");
  subModelColor.setValue("#ffcccc");
  subModelColor.setValue("#ccccff");
  subModelFontColor.setValue("black");
  placement.setValue(RightOutside,CsiType);
  // freeform
  alloc.setValue(Vertical);
  pli.placement.setValue(TopLeftOutside,CsiType);
  pli.perStep.setValue(true);
  // Rotate Icon
  rotateIcon.placement.setValue(RightOutside,CsiType);
}

void CalloutMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  margin     .init(this,      "MARGINS");
  stepNum    .init(this,      "STEP_NUMBER");
  sep        .init(this,      "SEPARATOR");
  border     .init(this,      "BORDER");
  subModelFont.init (this,    "SUBMODEL_FONT");
  instance   .init(this,      "INSTANCE_COUNT");
  background .init(this,      "BACKGROUND");
  subModelColor.init(this,    "SUBMODEL_BACKGROUND_COLOR");
  subModelFontColor.init(this,"SUBMODEL_FONT_COLOR");
  placement  .init(this,      "PLACEMENT");
  freeform   .init(this,      "FREEFORM");
  alloc      .init(this,      "ALLOC");
  pointer    .init(this,      "POINTER");

  begin      .init(this,      "BEGIN",   CalloutBeginRc);
  divider    .init(this,      "DIVIDER", CalloutDividerRc);
  end        .init(this,      "END",     CalloutEndRc);
  csi        .init(this,      "ASSEM");
  pli        .init(this,      "PLI");
  rotateIcon .init(this,      "ROTATE_ICON");
}

/* ------------------ */ 

MultiStepMeta::MultiStepMeta() : BranchMeta()
{
  stepNum.placement.setValue(LeftTopOutside,PartsListType);
  stepNum.color.setValue("black");
  // stepNum.font - default
  placement.setValue(CenterCenter,PageType);
  sep.setValue("black",DEFAULT_THICKNESS,DEFAULT_MARGINS);
  // subModelFont - default
  subModelFontColor.setValue("black");
  // freeform
  alloc.setValue(Vertical);
  pli.placement.setValue(LeftTopOutside,CsiType);
  pli.perStep.setValue(true);
  // Rotate Icon
  rotateIcon.placement.setValue(RightOutside,CsiType);
}

void MultiStepMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  margin   .init(this,    "MARGINS");
  stepNum  .init(this,    "STEP_NUMBER");
  placement.init(this,    "PLACEMENT");
  sep      .init(this,    "SEPARATOR");
  subModelFont.init (this,"SUBMODEL_FONT");
  subModelFontColor.init(this,
                         "SUBMODEL_FONT_COLOR");
  freeform .init(this,    "FREEFORM");
  alloc    .init(this,    "ALLOC");
  csi      .init(this,    "ASSEM");
  pli      .init(this,    "PLI");
  rotateIcon .init(this,  "ROTATE_ICON");

  begin    .init(this,    "BEGIN",  StepGroupBeginRc);
  divider  .init(this,    "DIVIDER",StepGroupDividerRc);
  end      .init(this,    "END",    StepGroupEndRc);
}

/* ------------------ */ 

void ResolutionMeta::init(
    BranchMeta *parent,
    const QString name)
{
  AbstractMeta::init(parent,name);
}

Rc ResolutionMeta::parse(QStringList &argv, int index, Where &here)
{
  int tokens = argv.size();
  tokens -= index;
  if (tokens == 0) {
      return FailureRc;
    }
  if (tokens == 2 && argv[index+1] == "DPI") {
      _here[0] = here;
      setResolution(argv[index].toFloat());
      setResolutionType(DPI);
    } else if (tokens == 2 && argv[index+1] == "DPCM") {
      _here[0] = here;
      setResolution(argv[index].toFloat());
      setResolutionType(DPCM);
    } else {
      return FailureRc;
    }
  return ResolutionRc;
}

QString ResolutionMeta::format(bool local, bool global)
{
  QString res;
  res = QString("%1 DPI") .arg(resolution(),0,'f',0);
  return LeafMeta::format(local,global,res);
} 

void ResolutionMeta::doc(QStringList &out, QString preamble)
{
  out << preamble + "<integer> (DPI|DPCM)";
}

/* ------------------ */

void NoStepMeta::init(
    BranchMeta *parent,
    const QString name,
    Rc _rc)
{
  AbstractMeta::init(parent,name);
  rc = _rc;
}
Rc NoStepMeta::parse(QStringList &argv, int index,Where & /* here */ )
{
  if (index == argv.size()) {
      return rc;
    } else {

      if (reportErrors) {
          QMessageBox::warning(NULL,
                               QMessageBox::tr("LPub3D"),
                               QMessageBox::tr("Unexpected token \"%1\" %2") .arg(argv[index]) .arg(argv.join(" ")));
        }

      return FailureRc;
    }
}
QString NoStepMeta::format(bool local, bool global)
{
  return LeafMeta::format(local,global,"");
}

void NoStepMeta::doc(QStringList &out, QString preamble)
{
  out << preamble;
}

/* ------------------ */

LPubMeta::LPubMeta() : BranchMeta()
{
  rotateIcon.placement.setValue(RightOutside,CsiType);
  stepNumber.placement.setValue(BottomLeftOutside,PageHeaderType);      // TopLeftInsideCorner,PageType
  stepNumber.color.setValue("black");
  // stepNumber - default
}

void LPubMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  page                   .init(this,"PAGE");
  assem                  .init(this,"ASSEM");
  stepNumber             .init(this,"STEP_NUMBER");
  callout                .init(this,"CALLOUT");
  multiStep              .init(this,"MULTI_STEP");
  pli                    .init(this,"PLI");
  bom                    .init(this,"BOM");
  remove                 .init(this,"REMOVE");
  reserve                .init(this,"RESERVE", ReserveSpaceRc);
  partSub                .init(this,"PART");
  resolution             .init(this,"RESOLUTION");
  insert                 .init(this,"INSERT");
  include                .init(this,"INCLUDE", IncludeRc);
  nostep                 .init(this,"NOSTEP", NoStepRc);\
  fadeStep               .init(this,"FADE_STEP");
  rotateIcon             .init(this,"ROTATE_ICON");
  reserve.setRange(0.0,1000000.0);
}

/* ------------------ */ 

void MLCadMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  MLCadSB .init(this,"SKIP_BEGIN", MLCadSkipBeginRc);
  MLCadSE .init(this,"SKIP_END",   MLCadSkipEndRc);
  MLCadGrp.init(this,"BTG",        MLCadGroupRc);
}

Rc MLCadMeta::parse(QStringList &argv, int index,Where &here)
{
  Rc rc;

  QHash<QString, AbstractMeta *>::iterator i = list.find(argv[index]);
  if (i == list.end() || index == argv.size()) {
      rc = OkRc;
    } else {
      rc = i.value()->parse(argv,index+1,here);
    }
  return rc;
}

/* ------------------ */ 

void LSynthMeta::init(BranchMeta *parent, QString name)
{
  AbstractMeta::init(parent, name);
  begin      .init(this, "BEGIN",       SynthBeginRc);
  end        .init(this, "END",         SynthEndRc);
  show       .init(this, "SHOW");
  hide       .init(this, "HIDE");
  inside     .init(this, "INSIDE");
  outside    .init(this, "OUTSIDE");
  cross      .init(this, "CROSS");
  synthesized.init(this, "SYNTHESIZED");
}

/* ------------------ */ 

Meta::Meta() : BranchMeta()
{
  QString empty;
  init(NULL,empty);
}

Meta::~Meta()
{
}

void Meta::init(BranchMeta * /* unused */, QString /* unused */)
{
  preamble = "0 ";

  LPub   .init(this,"!LPUB");
  step   .init(this,"STEP",    StepRc);
  clear  .init(this,"CLEAR",   ClearRc);
  rotStep.init(this,"ROTSTEP");
  bfx    .init(this,"BUFEXCHG");
  MLCad  .init(this,"MLCAD");
  LSynth .init(this,"SYNTH");

  if (tokenMap.size() == 0) {
      tokenMap["TOP_LEFT"]     = TopLeft;
      tokenMap["TOP"]          = Top;
      tokenMap["TOP_RIGHT"]    = TopRight;
      tokenMap["RIGHT"]        = Right;
      tokenMap["BOTTOM_RIGHT"] = BottomRight;
      tokenMap["BOTTOM"]       = Bottom;
      tokenMap["BOTTOM_LEFT"]  = BottomLeft;
      tokenMap["LEFT"]         = Left;
      tokenMap["CENTER"]       = Center;

      tokenMap["INSIDE"]       = Inside;
      tokenMap["OUTSIDE"]      = Outside;

      tokenMap["PAGE"]         = PageType;
      tokenMap["ASSEM"]        = CsiType;
      tokenMap["MULTI_STEP"]   = StepGroupType;
      tokenMap["STEP_GROUP"]   = StepGroupType;
      tokenMap["STEP_NUMBER"]  = StepNumberType;
      tokenMap["PLI"]          = PartsListType;
      tokenMap["PAGE_NUMBER"]  = PageNumberType;
      tokenMap["CALLOUT"]      = CalloutType;

      tokenMap["DOCUMENT_TITLE"]    	= PageTitleType;
      tokenMap["MODEL_ID"]    		= PageModelNameType;
      tokenMap["DOCUMENT_AUTHOR"]    	= PageAuthorType;
      tokenMap["PUBLISH_URL"]    	= PageURLType;
      tokenMap["MODEL_DESCRIPTION"]     = PageModelDescType;
      tokenMap["PUBLISH_DESCRIPTION"]	= PagePublishDescType;
      tokenMap["PUBLISH_COPYRIGHT"]     = PageCopyrightType;
      tokenMap["PUBLISH_EMAIL"]    	= PageEmailType;
      tokenMap["LEGO_DISCLAIMER"]    	= PageDisclaimerType;
      tokenMap["MODEL_PIECES"]    	= PagePiecesType;
      tokenMap["APP_PLUG"]    		= PagePlugType;
      tokenMap["MODEL_CATEGORY"]    	= PageCategoryType;
      tokenMap["DOCUMENT_LOGO"]    	= PageDocumentLogoType;
      tokenMap["DOCUMENT_COVER_IMAGE"]  = PageCoverImageType;
      tokenMap["APP_PLUG_IMAGE"]    	= PagePlugImageType;
      tokenMap["PAGE_HEADER"]    	= PageHeaderType;
      tokenMap["PAGE_FOOTER"]    	= PageFooterType;

      tokenMap["AREA"]         = ConstrainData::PliConstrainArea;
      tokenMap["SQUARE"]       = ConstrainData::PliConstrainSquare;
      tokenMap["WIDTH"]        = ConstrainData::PliConstrainWidth;
      tokenMap["HEIGHT"]       = ConstrainData::PliConstrainHeight;
      tokenMap["COLS"]         = ConstrainData::PliConstrainColumns;

      tokenMap["HORIZONTAL"]   = Horizontal;
      tokenMap["VERTICAL"]     = Vertical;

      tokenMap["PORTRAIT"]     = Portrait;
      tokenMap["LANDSCAPE"]    = Landscape;

      tokenMap["SORT_BY"]       = SortByType;
      tokenMap["ANNOTATION"]    = AnnotationType;
      tokenMap["ROTATE_ICON"]   = RotateIconType;
    }
}

Rc Meta::parse(
    QString  &line,
    Where    &here,
    bool           reportErrors)
{
  QStringList argv;
  
  QRegExp bgt("^\\s*0\\s+(MLCAD)\\s+(BTG)\\s+(.*)$");

  AbstractMeta::reportErrors = reportErrors;

  if (line.contains(bgt)) {
      argv << "MLCAD" << "BTG" << bgt.cap(3);
    } else {

      /* Parse the input line into argv[] */

      split(line,argv);

      if (argv.size() > 0) {
          argv.removeFirst();
        }
      if (argv.size()) {
          if (argv[0] == "LPUB") {
              argv[0] = "!LPUB";
            }

          if (argv[0] == "PLIST") {
              return  LPub.pli.parse(argv,1,here);
            }
        }
    }

  if (argv.size() > 0 && list.contains(argv[0])) {

      /* parse it up */

      Rc rc = this->BranchMeta::parse(argv,0,here);

      if (rc == FailureRc) {
          if (reportErrors) {
              QMessageBox::warning(NULL,QMessageBox::tr("LPub3D"),
                                   QMessageBox::tr("Parse failed %1:%2\n%3")
                                   .arg(here.modelName) .arg(here.lineNumber) .arg(line));
            }
        }
      return rc;
    }
  return OkRc;
}

bool Meta::preambleMatch(
    QString &line,
    QString &preamble)
{
  QStringList argv;

  /* Parse the input line into argv[] */

  split(line,argv);

  if (argv.size() > 0) {
      argv.removeFirst();
    }

  if (argv.size() && list.contains(argv[0])) {
      return BranchMeta::preambleMatch(argv,0,preamble);
    } else {
      return false;
    }
}

void Meta::pop()
{
  BranchMeta::pop();
}

void Meta::doc(QStringList &out)
{
  QString key;
  QStringList keys = list.keys();
  keys.sort();
  foreach(key, keys) {
      list[key]->doc(out, "0 " + key);
    }
}
