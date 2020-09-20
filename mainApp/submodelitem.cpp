/****************************************************************************
**
** Copyright (C) 2018 - 2019 Trevor SANDY. All rights reserved.
**
** This file may be used under the terms of the
** GNU General Public License (GPL) version 3.0
** which accompanies this distribution, and is
** available at http://www.gnu.org/licenses/gpl.html
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/****************************************************************************
 *
 * This class provides a simple mechanism for displaying a Submodel image
 * on the page.
 *
 ***************************************************************************/
#include <QMenu>
#include <QGraphicsSceneContextMenuEvent>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QTextStream>

#include "submodelitem.h"
#include "step.h"
#include "ranges.h"
#include "callout.h"
#include "resolution.h"
#include "render.h"
#include "paths.h"
#include "ldrawfiles.h"
#include "placementdialog.h"
#include "metaitem.h"
#include "color.h"
#include "lpub.h"
#include "commonmenus.h"
#include "lpub_preferences.h"
#include "ranges_element.h"
#include "range_element.h"
#include "dependencies.h"

const Where &SubModel::topOfStep()
{
  if (step == nullptr || steps == nullptr) {
    return gui->topOfPage();
  } else {
    if (step) {
      return step->topOfStep();
    } else {
      return steps->topOfSteps();
    }
  }
}

const Where &SubModel::bottomOfStep()
{
  if (step == nullptr || steps == nullptr) {
    return gui->bottomOfPage();
  } else {
    if (step) {
      return step->bottomOfStep();
    } else {
      return steps->bottomOfSteps();
    }
  }
}
const Where &SubModel::topOfSteps()
{
  return steps->topOfSteps();
}
const Where &SubModel::bottomOfSteps()
{
  return steps->bottomOfSteps();
}
const Where &SubModel::topOfCallout()
{
  return step->callout()->topOfCallout();
}
const Where &SubModel::bottomOfCallout()
{
  return step->callout()->bottomOfCallout();
}

/****************************************************************************
 * Submodel Image routines
 ***************************************************************************/

/* PLI - SM cross-reference
   SubModelPart          // PliPart             [New]
   SMGraphicsPixmapItem   // SMGraphicsPixmapItem [New]
   SubModel              // Pli
   SubModelItem          // PliBackgroundItem
*/

 // SubModelPart
SubModelPart::~SubModelPart()
{
  leftEdge.clear();
  rightEdge.clear();
}

float SubModelPart::maxMargin()
{
  float margin1 = qMax(instanceMeta.margin.valuePixels(XX),
                       csiMargin.valuePixels(XX));
  return margin1;
}

 // SubModel

SubModel::SubModel()
{
  relativeType = SubModelType;
  steps = nullptr;
  step = nullptr;
  meta = nullptr;
  background = nullptr;
  imageOutOfDate = false;
  shared         = false;
  displayInstanceCount = false;
}

void SubModel::setSubModel(
  QString &_file,
  Meta    &_meta)
{
  QString color = "0";
  subModelMeta  = _meta.LPub.subModel;

  // setup 3DViewer entry
  switch (parentRelativeType) {
  case CalloutType:
      top     = topOfCallout();
      bottom  = bottomOfCallout();
      callout = true;
      break;
  default:
      if (step) {
          top = topOfStep();
          bottom = bottomOfStep();
      } else {
          top = topOfSteps();
          bottom = bottomOfSteps();
      }
      multistep = parentRelativeType == StepGroupType;
      break;
  }

  QFileInfo fileInfo(_file);
  QString type = fileInfo.fileName().toLower();
  QString key = QString("%1_%2_%3")            // partial key
                        .arg(fileInfo.completeBaseName())
                        .arg(PREVIEW_SUBMODEL_SUFFIX)
                        .arg(color);

  if ( ! parts.contains(key)) {
      SubModelPart *part = new SubModelPart(type,color);
      part->instanceMeta = subModelMeta.instance;
      part->csiMargin    = subModelMeta.part.margin;
      parts.insert(key,part);
  }
}

void SubModel::clear()
{
  parts.clear();
}

bool SubModel::rotateModel(QString ldrName, QString subModel, const QString color, bool noCA)
{
   subModel = subModel.toLower();
   QStringList rotatedModel = QStringList()
           << QString("1 %1 0 0 0 1 0 0 0 1 0 0 0 1 %2").arg(color).arg(subModel);
   QString addLine = "1 color 0 0 0 1 0 0 0 1 0 0 0 1 foo.ldr";
   FloatPairMeta cameraAngles = noCA ? FloatPairMeta() : subModelMeta.cameraAngles;

   // create the Submodel ldr file and rotate its parts - camera angles not applied for Native renderer
   if ((renderer->rotateParts(
            addLine,
            subModelMeta.rotStep,
            rotatedModel,
            ldrName,
            step ? step->top.modelName : gui->topOfPage().modelName,
            cameraAngles
            )) != 0) {
       emit gui->messageSig(LOG_ERROR,QString("Failed to create and rotate Submodel ldr file: %1.")
                                             .arg(ldrName));
       return false;
   }
   return true;
}

int SubModel::pageSizeP(Meta *meta, int which){
  int _size;

  // flip orientation for landscape
  if (meta->LPub.page.orientation.value() == Landscape){
      which == 0 ? _size = 1 : _size = 0;
    } else {
      _size = which;
    }
  return meta->LPub.page.size.valuePixels(_size);
}

int SubModel::createSubModelImage(
  QString  &partialKey,
  QString  &type,
  QString  &color,
  QPixmap  *pixmap)
{
  int rc = 0;

  float modelScale   = subModelMeta.modelScale.value();
  int stepNumber     = subModelMeta.showStepNum.value() ?
                       subModelMeta.showStepNum.value() : 1;
  QString unitsName  = resolutionType() ? "DPI" : "DPCM";
  bool noCA          = subModelMeta.rotStep.value().type == "ABS";
  float camDistance  = subModelMeta.cameraDistance.value();
  bool  useImageSize = subModelMeta.imageSize.value(0) > 0;

  // assemble name key - create unique file when a value that impacts the image changes
  QString keyPart1 = QString("%1").arg(partialKey); /*baseName + @submodel + colour (0) */
  QString keyPart2 = QString("%1_%2_%3_%4_%5_%6_%7_%8")
                             .arg(stepNumber)
                             .arg(useImageSize ? double(subModelMeta.imageSize.value(0)) :
                                                 pageSizeP(meta, 0))
                             .arg(double(resolution()))
                             .arg(resolutionType() == DPI ? "DPI" : "DPCM")
                             .arg(double(modelScale))
                             .arg(double(subModelMeta.cameraFoV.value()))
                             .arg(noCA ? 0.0 : double(subModelMeta.cameraAngles.value(0)))
                             .arg(noCA ? 0.0 : double(subModelMeta.cameraAngles.value(1)));

  // append target vector if specified
  if (subModelMeta.target.isPopulated())
      keyPart2.append(QString("_%1_%2_%3")
                     .arg(double(subModelMeta.target.x()))
                     .arg(double(subModelMeta.target.y()))
                     .arg(double(subModelMeta.target.z())));

  // append rotstep if specified
  if (subModelMeta.rotStep.isPopulated())
      keyPart2.append(QString("_%1")
                     .arg(renderer->getRotstepMeta(subModelMeta.rotStep,true)));

  QString key = QString("%1_%2").arg(keyPart1).arg(keyPart2);

  imageName = QDir::toNativeSeparators(QDir::currentPath() + QDir::separator() +
                                       Paths::submodelDir + QDir::separator() + key.toLower() + ".png");

  // define ldr file name
  QStringList ldrNames = QStringList() << QDir::toNativeSeparators(QDir::currentPath() + QDir::separator() +
                                                                   Paths::tmpDir + QDir::separator() + "submodel.ldr");

  // Check if png file date modified is older than model file (on the stack) date modified
  imageOutOfDate = false;

  QFile part(imageName);

  if (part.exists()) {
      QDateTime lastModified = QFileInfo(imageName).lastModified();
      if (meta->LPub.multiStep.pli.perStep.value() == true){
          QStringList parsedStack = step->submodelStack();
          parsedStack << step->parent->modelName();
          if ( ! isOlder(parsedStack,lastModified)) {
              imageOutOfDate = true;
              if (imageOutOfDate && ! part.remove()) {
                  emit gui->messageSig(LOG_ERROR,QString("Failed to remove out of date SubModel Preview PNG file."));
              }
          }
      } else {
          if ( ! isOlder(type,lastModified)) {
              imageOutOfDate = true;
              if (imageOutOfDate && ! part.remove()) {
                  emit gui->messageSig(LOG_ERROR,QString("Failed to remove out of date SubModel Preview PNG file."));
              }
          }
      }
  }

  // Populate viewerStepKey variable
  viewerStepKey = QString("%1;%2;%3")
                           .arg(QString("%1_%2")
                                        .arg(gui->getSubFileIndex(QFileInfo(type).fileName()))
                                        .arg(PREVIEW_SUBMODEL_SUFFIX))
                           .arg(bottom.lineNumber)
                           .arg(stepNumber);

  emit gui->messageSig(LOG_DEBUG,QString("DEBUG - SubModel Preview viewerStepKey [%1] - modelName [%2]")
                                         .arg(viewerStepKey).arg(QFileInfo(type).fileName()));

  // Viewer submodel does not yet exist in repository
  bool addViewerStepContent = !gui->viewerStepContentExist(viewerStepKey);

  // We are processing again the current submodel Key so Submodel must have been updated in the viewer
  bool viewerUpdate = viewerStepKey == gui->getViewerStepKey();

  // Generate 3DViewer Submodel entry - TODO move to after Generate and renderer Submodel file
  if (! gui->exportingObjects()) {
      if ((addViewerStepContent || imageOutOfDate || viewerUpdate)) {

          FloatPairMeta cameraAngles = noCA ? FloatPairMeta() : subModelMeta.cameraAngles;
          QString addLine = "1 color 0 0 0 1 0 0 0 1 0 0 0 1 foo.ldr";

          // set submodel - unrotated
          QStringList subModel = QStringList()
                  << QString("1 %1 0 0 0 1 0 0 0 1 0 0 0 1 %2").arg(color).arg(type.toLower());

          // set rotated submodel
          QStringList rotatedSubmodel = subModel;

          // rotate submodel for 3DViewer without camera angles - this routine returns a list
          if ((rc = renderer->rotateParts(addLine,subModelMeta.rotStep,rotatedSubmodel,cameraAngles,false)) != 0)
              emit gui->messageSig(LOG_ERROR,QString("Failed to rotate viewer Submodel"));

          // add ROTSTEP command
          rotatedSubmodel.prepend(renderer->getRotstepMeta(subModelMeta.rotStep));

          // header and closing meta
          renderer->setNativeHeaderAndNoFileMeta(rotatedSubmodel,top.modelName,false/*pliPart*/,false/*finalModel*/);

          // consolidate submodel subfiles into single file
          if ((rc = renderer->createNativeModelFile(rotatedSubmodel,false,false) != 0))
              emit gui->messageSig(LOG_ERROR,QString("Failed to consolidate Viewer Submodel subfiles"));

          // store rotated and unrotated (csiParts). Unrotated parts are used to generate LDView pov file
          if (!subModelMeta.target.isPopulated())
              keyPart2.append(QString("_0_0_0"));
          if (!subModelMeta.rotStep.isPopulated())
              keyPart2.append(QString("_0_0_0_REL"));
          QString stepKey = QString("%1;%3").arg(keyPart1).arg(keyPart2);
          gui->insertViewerStep(viewerStepKey,rotatedSubmodel,subModel,ldrNames.first(),stepKey,multistep,callout);
      }

      // set viewer display options
      viewerOptions                 = new ViewerOptions();
      viewerOptions->ImageType      = Options::SMP;
      viewerOptions->ViewerStepKey  = viewerStepKey;
      viewerOptions->StudLogo       = subModelMeta.studLogo.value();
      viewerOptions->ImageFileName  = imageName;
      viewerOptions->Resolution     = resolution();
      viewerOptions->PageWidth      = pageSizeP(meta, 0);
      viewerOptions->PageHeight     = pageSizeP(meta, 1);
      viewerOptions->UsingViewpoint = gApplication->mPreferences.mNativeViewpoint <= 6;
      viewerOptions->ZFar           = CAMERA_ZFAR_NATIVE_DEFAULT;
      viewerOptions->ZNear          = CAMERA_ZNEAR_NATIVE_DEFAULT;
      viewerOptions->FoV            = CAMERA_FOV_NATIVE_DEFAULT;
      viewerOptions->CameraDistance = camDistance > 0 ? camDistance : renderer->ViewerCameraDistance(*meta,subModelMeta.modelScale.value());
      viewerOptions->NativeCDF      = meta->LPub.nativeCD.factor.value();
      viewerOptions->CameraName     = subModelMeta.cameraName.value();
      viewerOptions->RotStep        = xyzVector(float(subModelMeta.rotStep.value().rots[0]),float(subModelMeta.rotStep.value().rots[1]),float(subModelMeta.rotStep.value().rots[2]));
      viewerOptions->RotStepType    = subModelMeta.rotStep.value().type;
      viewerOptions->Latitude       = noCA ? 0.0 : subModelMeta.cameraAngles.value(0);
      viewerOptions->Longitude      = noCA ? 0.0 : subModelMeta.cameraAngles.value(1);
      viewerOptions->ModelScale     = subModelMeta.modelScale.value();
      viewerOptions->Target         = xyzVector(subModelMeta.target.x(),subModelMeta.target.y(),subModelMeta.target.z());
  }

  // Generate and renderer Submodel file
  if ( ! part.exists() || imageOutOfDate) {

      QElapsedTimer timer;
      timer.start();

      // generate Submodel file
      if (! rotateModel(ldrNames.first(),type,color,noCA)) {
          emit gui->messageSig(LOG_ERROR,QString("Failed to create and rotate Submodel ldr file: %1.")
                               .arg(ldrNames.first()));
          return -1;
      }

      // feed DAT to renderer
      if ((renderer->renderPli(ldrNames,imageName,*meta,SUBMODEL,0) != 0)) {
          emit gui->messageSig(LOG_ERROR,QString("%1 Submodel render failed for [%2] %3 %4 %5 on page %6")
                                                 .arg(Render::getRenderer())
                                                 .arg(imageName)
                                                 .arg(callout ? "called out," : "simple,")
                                                 .arg(multistep ? "step group" : "single step")
                                                 .arg(step ? step->stepNumber.number : 0)
                                                 .arg(gui->stepPageNum));
          imageName = QString(":/resources/missingimage.png");
          rc = -1;
      }

      if (!rc) {
          emit gui->messageSig(LOG_INFO,
                               QString("%1 Submodel render call took %2 milliseconds "
                                       "to render %3 for %4 %5 %6 on page %7.")
                               .arg(Render::getRenderer())
                               .arg(timer.elapsed())
                               .arg(imageName)
                               .arg(callout ? "called out," : "simple,")
                               .arg(multistep ? "step group" : "single step")
                               .arg(step ? step->stepNumber.number : 0)
                               .arg(gui->stepPageNum));
      }
  }

  pixmap->load(imageName);

  if (! gui->exportingObjects()) {
      viewerOptions->ImageWidth  = pixmap->width();
      viewerOptions->ImageHeight = pixmap->height();
  }

  return rc;
}

int SubModel::generateSubModelItem()
{
  QString key;

  widestPart = 0;
  tallestPart = 0;

  foreach(key,parts.keys()) { // There's only one
    SubModelPart *part;

    part = parts[key];

    if (gui->isSubmodel(part->type)) {

        if (part->color == "16") {
            part->color = "0";
        }

        // Create Image
        QFileInfo info(part->type);

        QPixmap *pixmap = new QPixmap();

        if (pixmap == nullptr) {
            return -1;
        }

        if (createSubModelImage(key,part->type,part->color,pixmap)) {
            QString imageName = QDir::toNativeSeparators(Paths::partsDir + QDir::separator() + key + ".png");
            emit gui->messageSig(LOG_ERROR, QMessageBox::tr("Failed to create Submodel image for %1")
                                 .arg(imageName));
            return -1;
        }

        QImage image = pixmap->toImage();

        part->pixmap = new SMGraphicsPixmapItem(this,part,*pixmap,parentRelativeType,part->type, part->color);

        delete pixmap;

        part->pixmapWidth  = image.width();
        part->pixmapHeight = image.height();

        part->width  = image.width();

        int textWidth = 0, textHeight = 0;

        if (displayInstanceCount) {

            QString descr;

            descr = QString("%1x") .arg(part->instanceMeta.number,0,10);

            QString font = subModelMeta.instance.font.valueFoo();
            QString color = subModelMeta.instance.color.value();

            part->instanceText =
                    new SMInstanceTextItem(this,part,descr,font,color,parentRelativeType);

            part->instanceText->size(textWidth,textHeight);

            part->textHeight = textHeight;

            // if text width greater than image width
            // the bounding box is wider

            if (textWidth > part->width) {
                part->width = textWidth;
            }
        }

        part->partTopMargin = 0;

        part->topMargin = part->csiMargin.valuePixels(YY);
        getLeftEdge(image,part->leftEdge);
        getRightEdge(image,part->rightEdge);

        if (displayInstanceCount) {
            part->partBotMargin = part->instanceMeta.margin.valuePixels(YY);

            /* Lets see if we can slide the text up in the bottom left corner of part image */

            int overlap;
            bool overlapped = false;

            for (overlap = 1; overlap < textHeight && ! overlapped; overlap++) {
                if (part->leftEdge[part->leftEdge.size() - overlap] < textWidth) {
                    overlapped = true;
                }
            }

            int hMax = textHeight + part->instanceMeta.margin.valuePixels(YY);
            for (int h = overlap; h < hMax; h++) {
                part->leftEdge << 0;
                part->rightEdge << textWidth;
            }
        } else {
            part->partBotMargin = part->csiMargin.valuePixels(YY);
        }

        part->height = part->leftEdge.size();

        if (part->width > widestPart) {
            widestPart = part->width;
        }
        if (part->height > tallestPart) {
            tallestPart = part->height;
        }
    } else {
        delete parts[key];
        parts.remove(key);
    }
  }

  sortedKeys = parts.keys(); // Only one

  return 0;
}

int SubModel::sizeSubModel(Meta *_meta, PlacementType _parentRelativeType, bool _perStep)
{
  int rc;

  parentRelativeType = _parentRelativeType;
  perStep = _perStep;

  if (parts.size() == 0) {
    return 1;
  }

  meta = _meta;

  rc = generateSubModelItem();
  if (rc != 0) {
    return rc;
  }

  ConstrainData constrainData = subModelMeta.constrain.value();

  return resizeSubModel(meta,constrainData);
}

int SubModel::sizeSubModel(ConstrainData::PliConstrain constrain, unsigned height)
{
  if (parts.size() == 0) {
    return 1;
  }

  if (meta) {
    ConstrainData constrainData;
    constrainData.type = constrain;
    constrainData.constraint = height;

    return resizeSubModel(meta,constrainData);
  }
  return 1;
}

int SubModel::resizeSubModel(
  Meta *meta,
  ConstrainData &constrainData)
{

  switch (parentRelativeType) {
    case StepGroupType:
      placement = meta->LPub.multiStep.subModel.placement;
    break;
    default:
      placement = meta->LPub.subModel.placement;
    break;
  }

  // Fill the Submodel image using constraint
  //   Constrain Height
  //   Constrain Width
  //   Constrain Columns
  //   Constrain Area
  //   Constrain Square

  int cols, height;
  int subModelWidth,subModelHeight;

  if (constrainData.type == ConstrainData::PliConstrainHeight) {
    int cols;
    int rc;
    rc = placeSubModel(sortedKeys,
                  10000000,
                  int(constrainData.constraint),
                  cols,
                  subModelWidth,
                  subModelHeight);
    if (rc == -2) {
      constrainData.type = ConstrainData::PliConstrainArea;
    }
  } else if (constrainData.type == ConstrainData::PliConstrainColumns) {
      if (parts.size() <= constrainData.constraint) {
        placeCols(sortedKeys);
        subModelWidth = Placement::size[0];
        subModelHeight = Placement::size[1];
        cols = parts.size();
      } else {
      int bomCols = int(constrainData.constraint);

      int maxHeight = 0;
      for (int i = 0; i < parts.size(); i++) {
        maxHeight += parts[sortedKeys[i]]->height + parts[sortedKeys[i]]->csiMargin.valuePixels(1);
      }

      maxHeight += maxHeight;

      if (bomCols) {
        for (height = maxHeight/(4*bomCols); height <= maxHeight; height++) {
          int rc = placeSubModel(sortedKeys,10000000,
                            height,
                            cols,
                            subModelWidth,
                            subModelHeight);
          if (rc == 0 && cols == bomCols) {
            break;
          }
            }
      }
    }
  } else if (constrainData.type == ConstrainData::PliConstrainWidth) {

    int height = 0;
    for (int i = 0; i < parts.size(); i++) {
      height += parts[sortedKeys[i]]->height;
    }

    int cols;
    int good_height = height;

    for ( ; height > 0; height -= 4) {

      int rc = placeSubModel(sortedKeys,10000000,
                        height,
                        cols,
                        subModelWidth,
                        subModelHeight);
      if (rc) {
        break;
      }

      int w = 0;

      for (int i = 0; i < parts.size(); i++) {
        int t;
        t = parts[sortedKeys[i]]->left + parts[sortedKeys[i]]->width;
        if (t > w) {
          w = t;
        }
      }
      if (w < constrainData.constraint) {
        good_height = height;
      }
    }
    placeSubModel(sortedKeys,10000000,
             good_height,
             cols,
             subModelWidth,
             subModelHeight);
  } else if (constrainData.type == ConstrainData::PliConstrainArea) {

    int height = 0;
    for (int i = 0; i < parts.size(); i++) {
      height += parts[sortedKeys[i]]->height;
    }

    int cols;
    int min_area = height*height;
    int good_height = height;

    // step by 1/10 of inch or centimeter

    int step = int(toPixels(0.1f,DPI));

    for ( ; height > 0; height -= step) {

      int rc = placeSubModel(sortedKeys,10000000,
                        height,
                        cols,
                        subModelWidth,
                        subModelHeight);

      if (rc) {
        break;
      }

      int h = 0;
      int w = 0;

      for (int i = 0; i < parts.size(); i++) {
        int t;
        t = parts[sortedKeys[i]]->bot + parts[sortedKeys[i]]->height;
        if (t > h) {
          h = t;
        }
        t = parts[sortedKeys[i]]->left + parts[sortedKeys[i]]->width;
        if (t > w) {
          w = t;
        }
      }
      if (w*h < min_area) {
        min_area = w*h;
        good_height = height;
      }
    }
    placeSubModel(sortedKeys,10000000,
             good_height,
             cols,
             subModelWidth,
             subModelHeight);
  } else if (constrainData.type == ConstrainData::PliConstrainSquare) {

    int height = 0;
    for (int i = 0; i < parts.size(); i++) {
      height += parts[sortedKeys[i]]->height;
    }

    int cols;
    int min_delta = height;
    int good_height = height;
    int step = int(toPixels(0.1f,DPI));

    for ( ; height > 0; height -= step) {

      int rc = placeSubModel(sortedKeys,10000000,
                        height,
                        cols,
                        subModelWidth,
                        subModelHeight);

      if (rc) {
        break;
      }

      int h = subModelWidth;
      int w = subModelHeight;

      int delta = 0;
      if (w < h) {
        delta = h - w;
      } else if (h < w) {
        delta = w - h;
      }
      if (delta < min_delta) {
        min_delta = delta;
        good_height = height;
      }
    }
    placeSubModel(sortedKeys,10000000,
             good_height,
             cols,
             subModelWidth,
             subModelHeight);
  }

  size[0] = subModelWidth;
  size[1] = subModelHeight;

  return 0;
}

void SubModel::positionChildren(
  int height,
  qreal scaleX,
  qreal scaleY)
{
  QString key;

  foreach (key, sortedKeys) {
    SubModelPart *part = parts[key];
    if (part == nullptr) {
      continue;
    }
    float x,y;

    x = part->left;
    y = height - part->bot;

    part->pixmap->setParentItem(background);
    part->pixmap->setPos(
      x/scaleX,
      (y - part->height + part->partTopMargin)/scaleY);
    part->pixmap->setTransformationMode(Qt::SmoothTransformation);

    bool includeInstance = subModelMeta.showInstanceCount.value() && part->instanceMeta.number > 1;
    if (includeInstance) {
        part->instanceText->setParentItem(background);
        part->instanceText->setPos(
                    x/scaleX,
                    (y - part->textHeight)/scaleY);
    }
  }
}

int SubModel::addSubModel(
  int       submodelLevel,
  QGraphicsItem *parent)
{
  if (parts.size()) {
    background =
      new SubModelBackgroundItem(
            this,
            size[0],
            size[1],
            parentRelativeType,
            submodelLevel,
            parent);

    if ( ! background) {
      return -1;
    }

    background->size[0] = size[0];
    background->size[1] = size[1];

    positionChildren(size[1],1.0,1.0);
  } else {
    background = nullptr;
  }
  return 0;
}

void SubModel::setPos(float x, float y)
{
  if (background) {
    background->setPos(x,y);
  }
}
void SubModel::setFlag(QGraphicsItem::GraphicsItemFlag flag, bool value)
{
  if (background) {
    background->setFlag(flag,value);
  }
}

 /*
 * Single step per page                        case 3 top/bottom of step
 * step in step group subModel per step = true case 3 top/bottom of step
 * step in callout                             case 3 top/bottom of step
 * step group global subModel                  case 2 topOfSteps/bottomOfSteps
 */
bool SubModel::autoRange(Where &top, Where &bottom)
{
  if (! perStep) {
    top = topOfSteps();
    bottom = bottomOfSteps();
    return steps->list.size() && perStep;
  } else {
    top = topOfStep();
    bottom = bottomOfStep();
    return false;
  }
}

int SubModel::placeSubModel(
  QList<QString> &keys,
  int    xConstraint,
  int    yConstraint,
  int   &cols,
  int   &width,
  int   &height)
{

  // Place the first row
  BorderData borderData;
  borderData = subModelMeta.border.valuePixels();
  int left = 0;
  int nPlaced = 0;
  int tallest = 0;
  int topMargin = int(borderData.margin[1]+borderData.thickness);
  int botMargin = topMargin;

  cols = 0;

  width = 0;
  height = 0;

  QString key;

  for (int i = 0; i < keys.size(); i++) {
    parts[keys[i]]->placed = false;
    if (parts[keys[i]]->height > yConstraint) {
      yConstraint = parts[keys[i]]->height;
      // return -2;
    }
  }

  QList< QPair<int, int> > margins;

  while (nPlaced < keys.size()) {

    int i;
    SubModelPart *part = nullptr;

    for (i = 0; i < keys.size(); i++) {
      QString key = keys[i];
      part = parts[key];
      if ( ! part->placed && left + part->width < xConstraint) {
        break;
      }
    }

    if (i == keys.size()) {
      return -1;
    }

    /* Start new col */

    SubModelPart *prevPart = parts[keys[i]];

    cols++;

    int width = prevPart->width /* + partMarginX */;
    int widest = i;

    prevPart->left = left;
    prevPart->bot  = 0;
    prevPart->placed = true;
    prevPart->col = cols;
    nPlaced++;

    QPair<int, int> margin;

    margin.first = qMax(prevPart->instanceMeta.margin.valuePixels(XX),
                        prevPart->csiMargin.valuePixels(XX));

    tallest = qMax(tallest,prevPart->height);

    int right = left + prevPart->width;
    int bot = prevPart->height;

    botMargin = qMax(botMargin,prevPart->csiMargin.valuePixels(YY));

    topMargin = qMax(topMargin,part->topMargin);

    left += width;

    part = parts[keys[widest]];

    margin.second = part->csiMargin.valuePixels(XX);

    margins.append(margin);
  }
  width = left;

  int margin;
  int totalCols = margins.size();
  int lastMargin = 0;
  for (int col = 0; col < totalCols; col++) {
    lastMargin = margins[col].second;
    if (col == 0) {
      int bmargin = int(borderData.thickness + borderData.margin[0]);
      margin = qMax(bmargin,margins[col].first);
    } else {
      margin = qMax(margins[col].first,margins[col].second);
    }
    for (int i = 0; i < parts.size(); i++) {
      if (parts[keys[i]]->col >= col+1) {
        parts[keys[i]]->left += margin;
      }
    }
    width += margin;
  }
  if (lastMargin < borderData.margin[0]+borderData.thickness) {
    lastMargin = int(borderData.margin[0]+borderData.thickness);
  }
  width += lastMargin;

  height = tallest;

  for (int i = 0; i < parts.size(); i++) {
    parts[keys[i]]->bot += botMargin;
  }

  height += botMargin + topMargin;

  return 0;
}

void SubModel::placeCols(
  QList<QString> &keys)
{
  QList< QPair<int, int> > margins;

  // Place the first row
  BorderData borderData;
  borderData = subModelMeta.border.valuePixels();

  float topMargin = parts[keys[0]]->topMargin;
  topMargin = qMax(borderData.margin[1]+borderData.thickness,topMargin);
  float botMargin = parts[keys[0]]->csiMargin.valuePixels(YY);
  botMargin = qMax(borderData.margin[1]+borderData.thickness,botMargin);

  int height = 0;
  int width;

  SubModelPart *part = parts[keys[0]];

  float borderMargin = borderData.thickness + borderData.margin[XX];

  width = int(qMax(borderMargin, part->maxMargin()));

  for (int i = 0; i < keys.size(); i++) {
    part = parts[keys[i]];
    part->left = width;
    part->bot  = int(botMargin);
    part->col = i;

    width += part->width;

      if (part->height > height) {
      height = part->height;
      }

      if (i < keys.size() - 1) {
        SubModelPart *nextPart = parts[keys[i+1]];
      width += int(qMax(part->maxMargin(),nextPart->maxMargin()));
      }
  }
  part = parts[keys[keys.size()-1]];
  width += int(qMax(part->maxMargin(),borderMargin));

  size[0] = width;
  size[1] = int(topMargin + height + botMargin);
}

void SubModel::getLeftEdge(
  QImage     &image,
  QList<int> &edge)
{
  QImage alpha = image.alphaChannel();

  for (int y = 0; y < alpha.height(); y++) {
    int x;
    for (x = 0; x < alpha.width(); x++) {
      QColor c = alpha.pixel(x,y);
      if (c.blue()) {
        edge << x;
        break;
      }
    }
    if (x == alpha.width()) {
      edge << x - 1;
    }
  }
}

void SubModel::getRightEdge(
  QImage     &image,
  QList<int> &edge)
{
  QImage alpha = image.alphaChannel();

  for (int y = 0; y < alpha.height(); y++) {
    int x;
    for (x = alpha.width() - 1; x >= 0; x--) {
      QColor c = alpha.pixel(x,y);
      if (c.blue()) {
        edge << x;
        break;
      }
    }
    if (x < 0) {
      edge << 0;
    }
  }
}

bool SubModel::loadTheViewer(){
    if (! gui->exporting()) {
        if (! renderer->LoadViewer(viewerOptions)) {
            emit gui->messageSig(LOG_ERROR,QString("Could not load 3D Viewer with Submodel key: %1")
                                 .arg(viewerStepKey));
            return false;
        }
    }
    return true;
}

//-----------------------------------------
//-----------------------------------------
//-----------------------------------------

// SMInstanceTextItem

SMInstanceTextItem::SMInstanceTextItem(
  SubModel       *_subModel,
  SubModelPart   *_part,
  QString        &text,
  QString        &fontString,
  QString        &colorString,
  PlacementType _parentRelativeType) :
    isHovered(false),
    mouseIsDown(false)
{
  parentRelativeType = _parentRelativeType;
  QString toolTip(tr("Times used - right-click to modify"));
  setText(_subModel,_part,text,fontString,toolTip);
  QColor color(colorString);
  setDefaultTextColor(color);

  setFlag(QGraphicsItem::ItemIsSelectable,true);
  setFlag(QGraphicsItem::ItemIsFocusable, true);
  setAcceptHoverEvents(true);

  setData(ObjectId, SubModelInstanceObj);
  setZValue(SUBMODELINSTANCE_ZVALUE_DEFAULT);
}

void SMInstanceTextItem::contextMenuEvent(
    QGraphicsSceneContextMenuEvent *event)
{
  QMenu menu;

  QString pl = "Parts Count ";

  QAction *fontAction   = commonMenus.fontMenu(menu,pl);
  QAction *colorAction  = commonMenus.colorMenu(menu,pl);
  QAction *marginAction = commonMenus.marginMenu(menu,pl);

  QAction *selectedAction   = menu.exec(event->screenPos());

  if (selectedAction == nullptr) {
      return;
    }

  Where top = subModel->top;
  Where bottom = subModel->bottom;

  if (selectedAction == fontAction) {
      changeFont(top,bottom,&subModel->subModelMeta.instance.font,1,false);
    } else if (selectedAction == colorAction) {
      changeColor(top,bottom,&subModel->subModelMeta.instance.color,1,false);
    } else if (selectedAction == marginAction) {
      changeMargins(pl + " Margins",top,bottom,&subModel->subModelMeta.instance.margin,true,1,false);
    }
}

void SMInstanceTextItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    isHovered = !this->isSelected() && !mouseIsDown;
    QGraphicsItem::hoverEnterEvent(event);
}

void SMInstanceTextItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    isHovered = false;
    QGraphicsItem::hoverLeaveEvent(event);
}

void SMInstanceTextItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    mouseIsDown = true;
    QGraphicsItem::mousePressEvent(event);
    update();
}

void SMInstanceTextItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    mouseIsDown = false;
    QGraphicsItem::mouseReleaseEvent(event);
    update();
}

void SMInstanceTextItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QPen pen;
    pen.setColor(isHovered ? QColor(Preferences::sceneGuideColor) : Qt::black);
    pen.setWidth(0/*cosmetic*/);
    pen.setStyle(isHovered ? Qt::PenStyle(Preferences::sceneGuidesLine) : Qt::NoPen);
    painter->setPen(pen);
    painter->setBrush(Qt::transparent);
    painter->drawRect(this->boundingRect());
    QGraphicsTextItem::paint(painter,option,widget);
}

//-----------------------------------------
//-----------------------------------------

 // SMGraphicsPixmapItem
QString SMGraphicsPixmapItem::subModelToolTip(QString type)
{
  QString title = Pli::titleDescription(type);
  if (title == "") {
    Where here(type,0);
    title = gui->readLine(here);
    title = title.right(title.length() - 2);
  }
  QString toolTip;
  toolTip = type + " \"" + title + "\" - right-click to modify";
  return toolTip;
}

void SMGraphicsPixmapItem::contextMenuEvent(
  QGraphicsSceneContextMenuEvent *event)
{
  QMenu menu;
  QString partlbl = part->type.size() > 15 ?
                    part->type.left(12) + "..." +
                    part->type.right(3) : part->type;
  QString pl = QString("%1").arg(partlbl);
  QAction *marginAction = commonMenus.marginMenu(menu,pl);
  QAction *scaleAction  = commonMenus.scaleMenu(menu,pl);


  QAction *selectedAction   = menu.exec(event->screenPos());

  if (selectedAction == nullptr) {
    return;
  }

  Where top = subModel->top;
  Where bottom = subModel->bottom;

  if (selectedAction == marginAction) {
    changeMargins(pl+" Margins",
                  top,
                  bottom,
                  &subModel->subModelMeta.part.margin);
  } else if (selectedAction == scaleAction) {
    changeFloatSpin(
      pl,
      "Scale",
      top,
      bottom,
     &subModel->subModelMeta.modelScale,0);
  }
}

void SMGraphicsPixmapItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    isHovered = !this->isSelected() && !mouseIsDown;
    QGraphicsItem::hoverEnterEvent(event);
}

void SMGraphicsPixmapItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    isHovered = false;
    QGraphicsItem::hoverLeaveEvent(event);
}

void SMGraphicsPixmapItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    subModel->loadTheViewer();

    mouseIsDown = true;
    QGraphicsItem::mousePressEvent(event);
    update();
}

void SMGraphicsPixmapItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    mouseIsDown = false;
    QGraphicsItem::mouseReleaseEvent(event);
    update();
}

void SMGraphicsPixmapItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QPen pen;
    pen.setColor(isHovered ? QColor(Preferences::sceneGuideColor) : Qt::black);
    pen.setWidth(0/*cosmetic*/);
    pen.setStyle(isHovered ? Qt::PenStyle(Preferences::sceneGuidesLine) : Qt::NoPen);
    painter->setPen(pen);
    painter->setBrush(Qt::transparent);
    painter->drawRect(this->boundingRect());
    QGraphicsPixmapItem::paint(painter,option,widget);
}

//-----------------------------------------
//-----------------------------------------
//-----------------------------------------

// SubModelBackgroundItem

SubModelBackgroundItem::SubModelBackgroundItem(
  SubModel      *_subModel,
  int            width,
  int            height,
  PlacementType  _parentRelativeType,
  int            submodelLevel,
  QGraphicsItem *parent)
{
  subModel   = _subModel;
  grabHeight = height;

  grabber = nullptr;
  grabbersVisible = false;

  parentRelativeType = _parentRelativeType;

  QPixmap *pixmap = new QPixmap(width,height);

  QString toolTip;

  toolTip = QString("Submodel [%1 x %2 px] - right-click to modify")
                    .arg(width)
                    .arg(height);

  if (parentRelativeType == StepGroupType /* && subModel->perStep == false */) {
    placement = subModel->meta->LPub.multiStep.subModel.placement;
  } else {
    placement = subModel->subModelMeta.placement;
  }

  setBackground( pixmap,
                 SubModelType,
                 subModel->meta,
                 subModel->subModelMeta.background,
                 subModel->subModelMeta.border,
                 subModel->subModelMeta.margin,
                 subModel->subModelMeta.subModelColor,
                 submodelLevel,
                 toolTip);

  setData(ObjectId, SubModelBackgroundObj);
  setZValue(SUBMODELBACKGROUND_ZVALUE_DEFAULT);
  setPixmap(*pixmap);
  setParentItem(parent);
  if (parentRelativeType != SingleStepType && subModel->perStep) {
    setFlag(QGraphicsItem::ItemIsMovable,false);
  }
}

void SubModelBackgroundItem::placeGrabbers()
{
  if (grabbersVisible) {
      if (grabber) {
        scene()->removeItem(grabber);
        grabber = nullptr;
      }
    grabbersVisible = false;
    return;
  }

  QRectF rect = currentRect();
  point = QPointF(rect.left() + rect.width()/2,rect.bottom());
  if (grabber == nullptr) {
    grabber = new Grabber(BottomInside,this,myParentItem());
    grabber->setData(ObjectId, SubmodelGrabberObj);
    grabber->top        = subModel->top;
    grabber->bottom     = subModel->bottom;
    grabber->stepNumber = subModel->step ? subModel->step->stepNumber.number : 0;
    grabbersVisible     = true;
  }
  grabber->setPos(point.x()-grabSize()/2,point.y()-grabSize()/2);
}

void SubModelBackgroundItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  position = pos();
  positionChanged = false;
  // we only want to toggle the grabbers off on second left mouse click
  if (event->button() != Qt::LeftButton){
    grabbersVisible = false;
  }
  QGraphicsItem::mousePressEvent(event);
  placeGrabbers();
}

void SubModelBackgroundItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
  positionChanged = true;
  QGraphicsItem::mouseMoveEvent(event);
  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable)) {
    placeGrabbers();
  }
}

void SubModelBackgroundItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsItem::mouseReleaseEvent(event);

  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable)) {

    QPointF newPosition;

    // back annotate the movement of the SM into the LDraw file.
    newPosition = pos() - position;
    if (newPosition.x() || newPosition.y()) {
      positionChanged = true;
      PlacementData placementData = placement.value();
      placementData.offsets[0] += newPosition.x()/subModel->relativeToSize[0];
      placementData.offsets[1] += newPosition.y()/subModel->relativeToSize[1];
      placement.setValue(placementData);

      Where here, bottom;
      bool useBot;

      useBot = subModel->autoRange(here,bottom);

      if (useBot) {
        here = bottom;
      }
      changePlacementOffset(here,&placement,subModel->parentRelativeType);
    }
  }
}

void SubModelBackgroundItem::contextMenuEvent(
  QGraphicsSceneContextMenuEvent *event)
{
  if (subModel) {
    QMenu menu;
    QString pl = "Submodel";
    QString whatsThis;

    QAction *constrainAction = commonMenus.constrainMenu(menu,pl);
    constrainAction->setWhatsThis(             "Change Shape:\n"
      "  You can change the shape of this Submodel.  One way, is\n"
      "  is to ask the computer to make the Submodel as small as\n"
      "  possible (area). Another way is to ask the computer to\n"
      "  make it as close to square as possible.  You can also pick\n"
      "  how wide you want it, and the computer will make it as\n"
      "  tall as is needed.  Another way is to pick how tall you\n"
      "  and it, and the computer will make it as wide as it needs.\n"
      "  The last way is to tell the computer how many columns it\n"
      "  can have, and then it will try to make all the columns the\n"
                                               "  same height\n");

    PlacementData placementData = subModel->placement.value();
    whatsThis = commonMenus.naturalLanguagePlacementWhatsThis(SubModelType,placementData,pl);
    QAction *cameraAnglesAction  = commonMenus.cameraAnglesMenu(menu,pl);
    QAction *scaleAction         = commonMenus.scaleMenu(menu, pl);
    QAction *cameraFoVAction     = commonMenus.cameraFoVMenu(menu,pl);
    QAction *placementAction     = commonMenus.placementMenu(menu, pl, whatsThis);
    QAction *backgroundAction    = commonMenus.backgroundMenu(menu,pl);
    QAction *borderAction        = commonMenus.borderMenu(menu,pl);
    QAction *marginAction        = commonMenus.marginMenu(menu,pl);
    QAction *subModelColorAction = commonMenus.subModelColorMenu(menu,pl);
    QAction *rotStepAction       = commonMenus.rotStepMenu(menu,pl);
    QAction *hideAction          = commonMenus.hideMenu(menu,pl);

    QAction *povrayRendererArgumentsAction = nullptr;
    QAction *rendererArgumentsAction = nullptr;
    bool usingPovray = Preferences::preferredRenderer == RENDERER_POVRAY;
    QString rendererName = QString("Add %1 Arguments")
                                   .arg(usingPovray ? "POV Generation":
                                                      QString("%1 Renderer").arg(Render::getRenderer()));
    if (!Preferences::usingNativeRenderer) {
        rendererArgumentsAction = menu.addAction(rendererName);
        rendererArgumentsAction->setWhatsThis("Add custom renderer arguments for this step");
        rendererArgumentsAction->setIcon(QIcon(":/resources/rendererarguments.png"));
        if (usingPovray) {
            povrayRendererArgumentsAction = menu.addAction(QString("Add %1 Renderer Arguments")
                                                                    .arg(Render::getRenderer()));
            povrayRendererArgumentsAction->setWhatsThis("Add POV-Ray custom renderer arguments for this step");
            povrayRendererArgumentsAction->setIcon(QIcon(":/resources/rendererarguments.png"));
        }
    }

    QAction *selectedAction   = menu.exec(event->screenPos());

    if (selectedAction == nullptr) {
      return;
    }

    Where top = subModel->top;
    Where bottom = subModel->bottom;

    if (selectedAction == constrainAction) {
        changeConstraint(pl+" Constraint",
                         top,
                         bottom,
                         &subModel->subModelMeta.constrain);
    } else if (selectedAction == placementAction) {
        if (subModel->perStep) {
            changePlacement(parentRelativeType,
                            subModel->perStep,
                            SubModelType,
                            pl+" Placement",
                            top,
                            bottom,
                            &subModel->placement);
        } else {
            changePlacement(parentRelativeType,
                            subModel->perStep,
                            SubModelType,
                            pl+" Placement",
                            top,
                            bottom,
                            &subModel->placement,true,1,0,false);
        }
    } else if (selectedAction == marginAction) {
      changeMargins(pl+" Margins",
                    top,
                    bottom,
                    &subModel->subModelMeta.margin);
    } else if (selectedAction == backgroundAction) {
      changeBackground(pl+" Background",
                       top,
                       bottom,
                       &subModel->subModelMeta.background);
    } else if (selectedAction == borderAction) {
      changeBorder(pl+" Border",
                   top,
                   bottom,
                   &subModel->subModelMeta.border);
    } else if (selectedAction == subModelColorAction) {
        changeSubModelColor(pl+" Background Color",
                         top,
                         bottom,
                         &subModel->subModelMeta.subModelColor);
    } else if (selectedAction == scaleAction){
          changeFloatSpin(pl+" Scale",
                          "Model Size",
                          top,
                          bottom,
                          &subModel->subModelMeta.modelScale);
    } else if (selectedAction == rotStepAction) {
        changeSubmodelRotStep(pl+" Rotate",
                              top,
                              bottom,
                              &subModel->subModelMeta.rotStep);
    } else if (selectedAction == cameraFoVAction) {
      changeFloatSpin(pl+" Camera Angle",
                      "Camera FOV",
                      top,
                      bottom,
                      &subModel->subModelMeta.cameraFoV);
    } else if (selectedAction == cameraAnglesAction) {
        changeCameraAngles(pl+" Camera Angles",
                        top,
                        bottom,
                        &subModel->subModelMeta.cameraAngles);
    } else if (selectedAction == hideAction) {
        hideSubmodel(top,
                     bottom,
                     &subModel->subModelMeta.show);
    } else if (selectedAction == rendererArgumentsAction) {
      StringMeta rendererArguments =
                 Render::getRenderer() == RENDERER_LDVIEW ? subModel->subModelMeta.ldviewParms :
                 Render::getRenderer() == RENDERER_LDGLITE ? subModel->subModelMeta.ldgliteParms :
                               /*POV scene file generator*/  subModel->subModelMeta.ldviewParms ;
      setRendererArguments(top,
                           bottom,
                           rendererName,
                           &rendererArguments);
    } else if (selectedAction == povrayRendererArgumentsAction) {
      setRendererArguments(top,
                           bottom,
                           Render::getRenderer(),
                           &subModel->subModelMeta.povrayParms);
    }
  }
}

/*
 * Code for resizing the SM - part of the resize class described in
 * resize.h
 */

void SubModelBackgroundItem::resize(QPointF grabbed)
{
  // recalculate corners Y

  point = grabbed;

  // Figure out desired height of SM

  if (subModel && subModel->parentRelativeType == CalloutType) {
    QPointF absPos = pos();
    absPos = mapToScene(absPos);
    grabHeight = int(grabbed.y() - absPos.y());
  } else {
    grabHeight = int(grabbed.y() - pos().y());
  }

  ConstrainData constrainData;
  constrainData.type = ConstrainData::PliConstrainHeight;
  constrainData.constraint = grabHeight;

  subModel->resizeSubModel(subModel->meta, constrainData);

  qreal width = subModel->size[0];
  qreal height = subModel->size[1];
  qreal scaleX = width/size[0];
  qreal scaleY = height/size[1];

  subModel->positionChildren(int(height),scaleX,scaleY);

  point = QPoint(int(pos().x()+width/2),int(pos().y()+height));
  grabber->setPos(point.x()-grabSize()/2,point.y()-grabSize()/2);

  resetTransform();
  setTransform(QTransform::fromScale(scaleX,scaleY),true);

  QList<QGraphicsItem *> kids = childItems();

  for (int i = 0; i < kids.size(); i++) {
    kids[i]->resetTransform();
    kids[i]->setTransform(QTransform::fromScale(1.0/scaleX,1.0/scaleY),true);
  }

  sizeChanged = true;
}

void SubModelBackgroundItem::change()
{
  ConstrainData constrainData;

  constrainData.type = ConstrainData::PliConstrainHeight;
  constrainData.constraint = int(grabHeight);

  subModel->subModelMeta.constrain.setValue(constrainData);

  Where top, bottom;
  bool useBot = subModel->autoRange(top,bottom);
  int append = 1;
  changeConstraint(top,bottom,&subModel->subModelMeta.constrain,append,useBot);
}

QRectF SubModelBackgroundItem::currentRect()
{
  if (subModel->parentRelativeType == CalloutType) {
    QRectF foo (pos().x(),pos().y(),size[0],size[1]);
    return foo;
  } else {
    return sceneBoundingRect();
  }
}
