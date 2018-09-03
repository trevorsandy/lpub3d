#include "LDrawModelViewer.h"
#include <TCFoundation/TCMacros.h>
#include <TCFoundation/TCAutoreleasePool.h>
#include <TCFoundation/mystring.h>
#include <TCFoundation/TCImage.h>
#include <TCFoundation/TCJpegOptions.h>
#include <TCFoundation/TCAlertManager.h>
#include <TCFoundation/TCProgressAlert.h>
#include <TCFoundation/TCLocalStrings.h>
#include <TCFoundation/TCUserDefaults.h>
//#include <TCFoundation/TCWebClient.h>
#include <LDLoader/LDLMainModel.h>
#include <LDLoader/LDLError.h>
#include <LDLoader/LDLFindFileAlert.h>
#include <LDLoader/LDLPalette.h>
#include <LDLoader/LDLAutoCamera.h>
#include <LDLoader/LDLModelLine.h>
#include <LDLoader/LDLConditionalLineLine.h>
#include <LDExporter/LDPovExporter.h>
//#include <LDExporter/LDStlExporter.h>
#ifdef EXPORT_3DS
#include <LDExporter/LD3dsExporter.h>
#endif // EXPORT_3DS
#ifdef EXPORT_LDR
#include <LDExporter/LDLdrExporter.h>
#endif // EXPORT_LDR
#include "LDInputHandler.h"
#include "LDModelParser.h"
#include "LDPreferences.h"
#include "LDPartsList.h"
#include "LDViewPoint.h"
#include "LDUserDefaultsKeys.h"
#include <TRE/TREMainModel.h>
#include <TRE/TREGL.h>
#include <time.h>
#include <gl2ps/gl2ps.h>

#ifndef USE_STD_CHRONO
#ifdef COCOA
#include <Foundation/Foundation.h>
#define FRAME_TIME ((NSDate *&)frameTime)
#endif // COCOA
#endif // !USE_STD_CHRONO

#ifdef USE_STD_CHRONO
//#define TIME_MODEL_LOAD
#endif // USE_STD_CHRONO

#ifdef TIME_MODEL_LOAD
#include <iostream>
#include <ctime>
#endif // TIME_MODEL_LOAD

#ifdef WIN32
#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
#define new DEBUG_CLIENTBLOCK
#endif // _DEBUG
#endif // WIN32

#if (defined _AIX) && !defined (fmodf)
float fmodf(float x, float y)
{
	return (x-((int)(x/y))*y);
}
#endif

#define FONT_CHAR_WIDTH 8
#define FONT_IMAGE_WIDTH 128
#define FONT_IMAGE_HEIGHT 256
#define FONT_NUM_CHARACTERS 256
#define DEF_DISTANCE_MULT 1.0f

LDrawModelViewer::StandardSizeList LDrawModelViewer::standardSizes;

LDrawModelViewer::LDrawModelViewer(TCFloat width, TCFloat height)
	:mainTREModel(NULL),
	mainModel(NULL),
	whiteLightDirModel(NULL),
	blueLightDirModel(NULL),
	highlightModel(NULL),
	filename(NULL),
	programPath(NULL),
	width(width),
	height(height),
	scaleFactor(1.0f),
	pixelAspectRatio(1.0f),
	cullBackFaces(0),
	viewMode(VMExamine),
	examineMode(EMFree),
	xRotate(0.0f),
	yRotate(0.0f),
	zRotate(0.0f),
	rotationSpeed(0.0f),
	cameraXRotate(0.0f),
	cameraYRotate(0.0f),
	cameraZRotate(0.0f),
	zoomSpeed(0.0f),
	xPan(0.0f),
	yPan(0.0f),
	rotationMatrix(NULL),
	defaultRotationMatrix(NULL),
	defaultLatitude(30.0f),
	defaultLongitude(45.0f),
	examineLatitude(defaultLatitude),
	examineLongitude(defaultLongitude),
	clipAmount(0.0f),
	nextClipAmount(-1.0f),
	nextDistance(-1.0f),
	highlightLineWidth(1.0f),
	wireframeLineWidth(1.0f),
	anisoLevel(1.0f),
	clipZoom(false),
	fontListBase(0),
	backgroundR(0.0f),
	backgroundG(0.0f),
	backgroundB(0.0f),
	backgroundA(1.0f),
	defaultR(153),
	defaultG(153),
	defaultB(153),
	defaultColorNumber(-1),
	xTile(0),
	yTile(0),
	numXTiles(1),
	numYTiles(1),
	stereoMode(LDVStereoNone),
	stereoEyeSpacing(50.0f),
	cutawayMode(LDVCutawayNormal),
	cutawayAlpha(1.0f),
	cutawayLineWidth(1.0f),
	zoomMax(1.99f),
	curveQuality(2),
	textureFilterType(GL_LINEAR_MIPMAP_LINEAR),
	distanceMultiplier(DEF_DISTANCE_MULT),
	fontImage1x(NULL),
	fontImage2x(NULL),
	aspectRatio(1.0f),
	currentFov(45.0f),
	fov(45.0f),
	extraSearchDirs(NULL),
	seamWidth(0.5f),
	memoryUsage(2),
	lightVector(0.0f, 0.0f, 1.0f),
	preferences(NULL),
	mouseMode(LDVMouseNone),
	inputHandler(NULL),
	step(-1),
	commandLineStep(-1),
	mpdChildIndex(0),
	exporter(NULL),
	exportType(ETPov),
	highlightR(160),
	highlightG(224),
    highlightB(255)/*,
    cameraData(NULL)*/
{
#ifdef _LEAK_DEBUG
	strcpy(className, "LDrawModelViewer");
#endif
	flags.qualityLighting = false;
	flags.showsHighlightLines = false;
	flags.qualityStuds = false;
	flags.usesFlatShading = false;
	flags.usesSpecular = true;
	flags.drawWireframe = false;
	flags.useWireframeFog = false;
	flags.usePolygonOffset = true;
	flags.useLighting = true;
	flags.subduedLighting = false;
	flags.allowPrimitiveSubstitution = true;
	flags.useStipple = false;
	flags.fileIsPart = false;
	flags.sortTransparent = true;
	flags.needsSetup = true;
	flags.needsTextureSetup = true;
	flags.needsMaterialSetup = true;
	flags.needsLightingSetup = true;
	flags.needsReload = false;
	flags.needsReparse = false;
	flags.needsRecompile = false;
	flags.needsResize = true;
	flags.needsCalcSize = false;
	flags.needsRotationMatrixSetup = true;
	flags.needsViewReset = true;
	flags.needsResetStep = false;
	flags.needsResetMpd = false;
	flags.paused = false;
	flags.slowClear = false;
	flags.blackHighlights = false;
	flags.textureStuds = true;
	flags.oneLight = false;
	flags.drawConditionalHighlights = false;
	flags.showAllConditionalLines = false;
	flags.showConditionalControlPoints = false;
	flags.performSmoothing = true;
	flags.lineSmoothing = false;
	flags.constrainZoom = true;
	flags.edgesOnly = false;
	flags.hiResPrimitives = false;
	flags.processLDConfig = true;
	flags.skipValidation = false;
	flags.autoCenter = true;
	flags.forceZoomToFit = false;
	flags.defaultTrans = false;
	flags.bfc = true;
	flags.redBackFaces = false;
	flags.greenFrontFaces = false;
	flags.blueNeutralFaces = false;
	flags.defaultLightVector = true;
	flags.overrideModelCenter = false;
	flags.overrideModelSize = false;
	flags.overrideDefaultDistance = false;
	flags.checkPartTracker = true;
	flags.showLight = false;
	flags.drawLightDats = true;
	flags.optionalStandardLight = true;
	flags.noLightGeom = false;
	flags.updating = false;
	flags.saveAlpha = false;
	flags.showAxes = false;
	flags.axesAtOrigin = true;
	flags.showBoundingBox = false;
	flags.boundingBoxesOnly = false;
	flags.obi = false;
	flags.gl2ps = false;
	flags.povCameraAspect = TCUserDefaults::boolForKey(POV_CAMERA_ASPECT_KEY,
		false, false);
	flags.animating = false;
	flags.randomColors = false;
	flags.noUI = false;
	flags.keepRightSideUp = false;
	flags.texmaps = true;
//	TCAlertManager::registerHandler(LDLFindFileAlert::alertClass(), this,
//		(TCAlertCallback)&LDrawModelViewer::findFileAlertCallback);
	// Set 4:4:4 as the default sub-sample pattern for JPEG images.
	TCJpegOptions::setDefaultSubSampling(TCJpegOptions::SS444);
#ifndef USE_STD_CHRONO
#ifdef WIN32
	if (!QueryPerformanceFrequency(&hrpcFrequency))
	{
		hrpcFrequency.QuadPart = 0;
	}
#endif // WIN32
#ifdef _QT
	qtime.start();
#endif
#ifdef COCOA
	FRAME_TIME = nil;
#endif // COCOA
#endif // !USE_STD_CHRONO
	updateFrameTime(true);
}

LDrawModelViewer::~LDrawModelViewer(void)
{
}

void LDrawModelViewer::dealloc(void)
{
#ifndef USE_STD_CHRONO
#ifdef COCOA
	[FRAME_TIME release];
#endif // COCOA
#endif // !USE_STD_CHRONO
	TCAlertManager::unregisterHandler(this);
	TCObject::release(inputHandler);
	TCObject::release(mainTREModel);
	TCObject::release(mainModel);
	TCObject::release(whiteLightDirModel);
	TCObject::release(blueLightDirModel);
	TCObject::release(highlightModel);
	TCObject::release(exporter);
	mainTREModel = NULL;
	delete[] filename;
	filename = NULL;
	delete[] programPath;
	programPath = NULL;
	delete[] rotationMatrix;
	rotationMatrix = NULL;
	delete[] defaultRotationMatrix;
	defaultRotationMatrix = NULL;
	TCObject::release(fontImage1x);
	fontImage1x = NULL;
	TCObject::release(fontImage2x);
	fontImage2x = NULL;
	TCObject::release(extraSearchDirs);
	extraSearchDirs = NULL;
	/*delete cameraData;
	cameraData = NULL;*/
	TCObject::dealloc();
}

LDInputHandler *LDrawModelViewer::getInputHandler(void)
{
	if (!inputHandler)
	{
		inputHandler = new LDInputHandler(this);
	}
	return inputHandler;
}

void LDrawModelViewer::setFilename(const char* value)
{
	delete[] filename;
	filename = copyString(value);
	mpdName = "";
	if (filename != NULL)
	{
		char *mpdSpot = NULL;

#ifdef WIN32
		if (strlen(filename) > 2)
		{
			mpdSpot = strchr(&filename[2], ':');
		}
#else // WIN32
		mpdSpot = strchr(filename, ':');
#endif // WIN32
		if (mpdSpot != NULL)
		{
			mpdName = &mpdSpot[1];
			mpdSpot[0] = 0;
		}
	}
	flags.needsResetStep = true;
	flags.needsResetMpd = true;
	highlightPaths.clear();
}

void LDrawModelViewer::setProgramPath(const char *value)
{
	delete[] programPath;
	programPath = copyString(value);
	stripTrailingPathSeparators(programPath);
}

void LDrawModelViewer::setFileIsPart(bool value)
{
	flags.fileIsPart = value;
}

void LDrawModelViewer::applyTile(void)
{
	//if (1 || numXTiles > 1 || numYTiles > 1)
	{
		GLint tileLeft;
		GLint tileBottom;
		GLfloat xScale, yScale;
		GLfloat xOffset, yOffset;

		tileLeft = (int)(xTile * scale(width));
		tileBottom = (int)((numYTiles - yTile - 1) * scale(height));
		xScale = (GLfloat)((width * numXTiles) / width);
		yScale = (GLfloat)((height * numYTiles) / height);
		xOffset = (-2.0f * tileLeft) / (scale(width) * numXTiles) +
			(1 - 1.0f / numXTiles);
		yOffset = (-2.0f * tileBottom) / (scale(height) * numYTiles) +
			(1 - 1.0f / numYTiles);
		glScalef(xScale, yScale, 1.0f);
		treGlTranslatef(xOffset, yOffset, 0.0f);
	}
}

TCFloat LDrawModelViewer::getStereoWidthModifier(void)
{
	if (stereoMode == LDVStereoCrossEyed || stereoMode == LDVStereoParallel)
	{
		return 2.0f;
	}
	else
	{
		return 1.0f;
	}
}

void LDrawModelViewer::setFieldOfView(double lfov, TCFloat nClip, TCFloat fClip)
{
	GLdouble aspectWidth, aspectHeight;

//	printf("LDrawModelViewer::setFieldOfView(%.5f, %.5f, %.5f)\n", lfov, nClip,
//		fClip);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	applyTile();
	aspectWidth = width * numXTiles / getStereoWidthModifier();
	aspectHeight = height * numYTiles * pixelAspectRatio;
	gluPerspective(lfov, aspectWidth / aspectHeight, nClip, fClip);
	glMatrixMode(GL_MODELVIEW);
}

void LDrawModelViewer::perspectiveView(void)
{
	perspectiveView(true);
}

void LDrawModelViewer::setFov(TCFloat value)
{
	if (value != fov)
	{
		fov = value;
		flags.needsViewReset = true;
	}
}

TCFloat LDrawModelViewer::getHFov(void)
{
	TCFloat actualWidth = width / getStereoWidthModifier();

	return (TCFloat)(2.0 * rad2deg(atan(tan(deg2rad(fov / 2.0)) *
		(actualWidth * numXTiles) / height * numYTiles)));
}

void LDrawModelViewer::updateCurrentFov(void)
{
	TCFloat actualWidth = width / getStereoWidthModifier();

	currentFov = TCUserDefaults::floatForKey("HFOV", -1, false);
	if (currentFov == -1)
	{
		currentFov = fov;
		if (actualWidth * numXTiles < height * numYTiles)
		{
			// When the window is taller than it is wide, we want our current
			// FOV to be the horizontal FOV, so we need to calculate the
			// vertical FOV.
			//
			// From Lars Hassing:
			// Vertical FOV = 2*atan(tan(hfov/2)/(width/height))
			currentFov = (TCFloat)(2.0 * rad2deg(atan(tan(deg2rad(fov / 2.0)) *
				height * numYTiles / (actualWidth * numXTiles))));

			if (currentFov > 179.0f)
			{
				currentFov = 179.0f;
			}
			aspectRatio = height / actualWidth;
		}
		else
		{
			aspectRatio = actualWidth / height;
		}
	}
	else
	{
		fov = (TCFloat)(2.0 * rad2deg(atan(tan(deg2rad(currentFov / 2.0)) *
			height * numYTiles / (actualWidth * numXTiles))));
		if (actualWidth * numXTiles > height * numYTiles)
		{
			currentFov = fov;
			aspectRatio = actualWidth / height;
		}
		else
		{
			aspectRatio = height / actualWidth;
		}
	}
}

TCFloat LDrawModelViewer::getClipRadius(void)
{
	TCFloat clipRadius;

	if (flags.autoCenter)
	{
		clipRadius = clipSize / 1.45f;
	}
	else
	{
		// If we aren't centered, then just double the clip radius, and that
		// guarantees everything will fit.  Remember that the near clip plane
		// has a minimum distance, so even if it ends up initially behind the
		// camera, we clamp it to be in front.
		clipRadius = clipSize;
	}
	return clipRadius;
}

TCFloat LDrawModelViewer::getZDistance(void)
{
	if (flags.autoCenter)
	{
		TCFloat inverseMatrix[16];
		TCVector vector = camera.getPosition();

		camera.getFacing().getInverseMatrix(inverseMatrix);
		vector = vector.transformPoint(inverseMatrix);
		return vector[2];
	}
	else
	{
		return (center - camera.getPosition()).length();
	}
}

void LDrawModelViewer::perspectiveView(bool resetViewport)
{
	TCFloat nClip;
	TCFloat fClip;
	TCFloat clipRadius = getClipRadius();
	int actualWidth = (int)scale(width) / (int)getStereoWidthModifier();
	TCFloat zDistance = getZDistance();
	//TCFloat aspectAdjust = (TCFloat)tan(1.0f);

	if (flags.forceZoomToFit)
	{
		zoomToFit();
	}
	updateCurrentFov();
	if (resetViewport)
	{
		glViewport(0, 0, actualWidth, (GLsizei)scale(height));
		flags.needsResize = false;
	}
	//aspectRatio = (TCFloat)(1.0f / tan(1.0f / aspectRatio)) * aspectAdjust;
	aspectRatio = 1.0f;
	nClip = zDistance - clipRadius * aspectRatio + clipAmount * aspectRatio *
		clipRadius;
	if (nClip < clipSize / 1000.0f)
	{
		nClip = clipSize / 1000.0f;
	}
	fClip = zDistance + clipRadius * aspectRatio;
	setFieldOfView(currentFov, nClip, fClip);
	glLoadIdentity();
	glFogi(GL_FOG_MODE, GL_LINEAR);
	glFogf(GL_FOG_START, (GLfloat)nClip);
	glFogf(GL_FOG_END, (GLfloat)fClip);
}

void LDrawModelViewer::perspectiveViewToClipPlane(void)
{
	TCFloat nClip;
	TCFloat fClip;
	TCFloat zDistance = getZDistance();
//	TCFloat zDistance = (camera.getPosition()).length();
	TCFloat clipRadius = getClipRadius();
//	TCFloat distance = (camera.getPosition() - center).length();

	nClip = zDistance - clipSize / 2.0f;
//	fClip = distance - size * aspectRatio / 2.0f + clipAmount * aspectRatio *
//		size;
	fClip = zDistance - clipRadius * aspectRatio + clipAmount * aspectRatio *
		clipRadius;
	if (fClip < clipSize / 1000.0f)
	{
		fClip = clipSize / 1000.0f;
	}
	if (nClip < clipSize / 1000.0f)
	{
		nClip = clipSize / 1000.0f;
	}
	setFieldOfView(currentFov, nClip, fClip);
	glLoadIdentity();
	glFogi(GL_FOG_MODE, GL_LINEAR);
	glFogf(GL_FOG_START, (GLfloat)nClip);
	glFogf(GL_FOG_END, (GLfloat)fClip);
}

/*
void LDrawModelViewer::setProgressCallback(LDMProgressCallback callback,
										   void* userData)
{
	progressCallback = callback;
	progressUserData = userData;
}

void LDrawModelViewer::setErrorCallback(LDMErrorCallback callback,
										void* userData)
{
	errorCallback = callback;
	errorUserData = userData;
}
*/

bool LDrawModelViewer::skipCameraPositioning(void)
{
	return defaultRotationMatrix && (defaultRotationMatrix[12] != 0.0f ||
		defaultRotationMatrix[13] != 0.0f || defaultRotationMatrix[14] != 0.0f);
}

TCFloat LDrawModelViewer::calcDefaultDistance(void)
{
	if (flags.overrideDefaultDistance)
	{
		return defaultDistance;
	}
	// Note that the margin is on all sides, so it's on two edges per axis,
	// which is why we multiply by 2.
	TCFloat margin = getWideLineMargin() * 2.0f;
	TCFloat marginAdjust = 1.0f;

	if (margin != 0.0f)
	{
		int actualWidth = (int)scale(width) / (int)getStereoWidthModifier() *
			numXTiles;
		int actualHeight = (int)scale(height) * numYTiles;

		if (actualWidth < actualHeight)
		{
			marginAdjust = (actualHeight + margin) / actualHeight;
		}
		else
		{
			marginAdjust = (actualWidth + margin) / actualWidth;
		}
	}
	return (TCFloat)(size / 2.0 / sin(deg2rad(fov / 2.0))) * distanceMultiplier
		* marginAdjust;
/*
	double angle1 = deg2rad(90.0f - (currentFov / 2.0));
	double angle2 = deg2rad(currentFov / 4.0);
	double radius = size / 2.0;

	return (TCFloat)(radius * tan(angle1) + radius * tan(angle2)) *
		distanceMultiplier;
*/
}

void LDrawModelViewer::resetView(LDVAngle viewAngle)
{
	flags.needsViewReset = false;
	flags.autoCenter = true;
	if (!mainTREModel)
	{
		return;
	}
	if (clipAmount != 0.0f)
	{
		clipAmount = 0.0f;
		perspectiveView();
	}
	defaultDistance = calcDefaultDistance();
	if (!skipCameraPositioning())
	{
		// If the user specifies a rotation matrix that includes a translation,
		// then don't move the camera.
		camera.setPosition(TCVector(0.0f, 0.0f, defaultDistance));
	}
	camera.setFacing(LDLFacing());
	if (!rotationMatrix)
	{
		rotationMatrix = new TCFloat[16];
	}
	switch (viewAngle)
	{
	case LDVAngleDefault:
		setupDefaultViewAngle();
		break;
	case LDVAngleFront:
		setupFrontViewAngle();
		break;
	case LDVAngleBack:
		setupBackViewAngle();
		break;
	case LDVAngleLeft:
		setupLeftViewAngle();
		break;
	case LDVAngleRight:
		setupRightViewAngle();
		break;
	case LDVAngleTop:
		setupTopViewAngle();
		break;
	case LDVAngleBottom:
		setupBottomViewAngle();
		break;
	case LDVAngleIso:
		setupIsoViewAngle();
		break;
	}
	flags.needsRotationMatrixSetup = true;
	xPan = 0.0f;
	yPan = 0.0f;
	perspectiveView(true);
}

void LDrawModelViewer::setModelCenter(const TCFloat *value)
{
	if (value)
	{
		flags.overrideModelCenter = true;
		center = TCVector(value[0],value[1],value[2]);
		flags.needsSetup = true;
	}
}

void LDrawModelViewer::setPovCameraAspect(bool value, bool saveSetting)
{
	if (value != flags.povCameraAspect)
	{
		flags.povCameraAspect = value;
		if (saveSetting)
		{
			TCUserDefaults::setBoolForKey(value, POV_CAMERA_ASPECT_KEY, false);
		}
	}
}

void LDrawModelViewer::setModelSize(const TCFloat value)
{
	if (value)
	{
		flags.overrideModelSize = true;
		size = value;
		clipSize = value;
		flags.needsSetup = true;
	}
}

void LDrawModelViewer::setDefaultDistance(TCFloat value)
{
	if (value > 0)
	{
		defaultDistance = value;
		flags.overrideDefaultDistance = true;
	}
}

void LDrawModelViewer::setDefaultRotationMatrix(const TCFloat *value)
{
	if (value)
	{
		if (!defaultRotationMatrix || memcmp(defaultRotationMatrix, value,
			16 * sizeof(TCFloat)) != 0)
		{
			delete[] defaultRotationMatrix;
			defaultRotationMatrix = new TCFloat[16];
			memcpy(defaultRotationMatrix, value, 16 * sizeof(TCFloat));
			flags.needsSetup = true;
		}
	}
	else if (defaultRotationMatrix)
	{
		delete[] defaultRotationMatrix;
		defaultRotationMatrix = NULL;
		flags.needsSetup = true;
	}
}

void LDrawModelViewer::setDefaultLatLong(TCFloat latitude, TCFloat longitude)
{
	if (defaultLatitude != latitude || defaultLongitude != longitude)
	{
		defaultLatitude = latitude;
		defaultLongitude = longitude;
		flags.needsSetup = true;
	}
}

void LDrawModelViewer::setupDefaultViewAngle(void)
{
	if (viewMode == VMExamine && examineMode == EMLatLong)
	{
		examineLatitude = defaultLatitude;
		examineLongitude = defaultLongitude;
	}
	else if (defaultRotationMatrix)
	{
		memcpy(rotationMatrix, defaultRotationMatrix, 16 * sizeof(TCFloat));
	}
	else
	{
		setupIsoViewAngle();
	}
}

void LDrawModelViewer::setupIsoViewAngle(void)
{
	rotationMatrix[0] = (TCFloat)(sqrt(2.0) / 2.0);
	rotationMatrix[1] = (TCFloat)(sqrt(2.0) / 4.0);
	rotationMatrix[2] = (TCFloat)(-sqrt(1.5) / 2.0);
	rotationMatrix[3] = 0.0f;
	rotationMatrix[4] = 0.0f;
	rotationMatrix[5] = (TCFloat)(sin(M_PI / 3.0));
	rotationMatrix[6] = 0.5f;
	rotationMatrix[7] = 0.0f;
	rotationMatrix[8] = (TCFloat)(sqrt(2.0) / 2.0);
	rotationMatrix[9] = (TCFloat)(-sqrt(2.0) / 4.0);
	rotationMatrix[10] = (TCFloat)(sqrt(1.5) / 2.0);
	rotationMatrix[11] = 0.0f;
	rotationMatrix[12] = 0.0f;
	rotationMatrix[13] = 0.0f;
	rotationMatrix[14] = 0.0f;
	rotationMatrix[15] = 1.0f;
	examineLatitude = 30.0f;
	examineLongitude = 45.0f;
}

void LDrawModelViewer::setupFrontViewAngle(void)
{
	rotationMatrix[0] = 1.0f;
	rotationMatrix[1] = 0.0f;
	rotationMatrix[2] = 0.0f;
	rotationMatrix[3] = 0.0f;

	rotationMatrix[4] = 0.0f;
	rotationMatrix[5] = 1.0f;
	rotationMatrix[6] = 0.0f;
	rotationMatrix[7] = 0.0f;

	rotationMatrix[8] = 0.0f;
	rotationMatrix[9] = 0.0f;
	rotationMatrix[10] = 1.0f;
	rotationMatrix[11] = 0.0f;

	rotationMatrix[12] = 0.0f;
	rotationMatrix[13] = 0.0f;
	rotationMatrix[14] = 0.0f;
	rotationMatrix[15] = 1.0f;
	examineLatitude = 0.0f;
	examineLongitude = 0.0f;
}

void LDrawModelViewer::setupBackViewAngle(void)
{
	rotationMatrix[0] = -1.0f;
	rotationMatrix[1] = 0.0f;
	rotationMatrix[2] = 0.0f;
	rotationMatrix[3] = 0.0f;

	rotationMatrix[4] = 0.0f;
	rotationMatrix[5] = 1.0f;
	rotationMatrix[6] = 0.0f;
	rotationMatrix[7] = 0.0f;

	rotationMatrix[8] = 0.0f;
	rotationMatrix[9] = 0.0f;
	rotationMatrix[10] = -1.0f;
	rotationMatrix[11] = 0.0f;

	rotationMatrix[12] = 0.0f;
	rotationMatrix[13] = 0.0f;
	rotationMatrix[14] = 0.0f;
	rotationMatrix[15] = 1.0f;
	examineLatitude = 0.0f;
	examineLongitude = 180.0f;
}

void LDrawModelViewer::setupLeftViewAngle(void)
{
	rotationMatrix[0] = 0.0f;
	rotationMatrix[1] = 0.0f;
	rotationMatrix[2] = -1.0f;
	rotationMatrix[3] = 0.0f;

	rotationMatrix[4] = 0.0f;
	rotationMatrix[5] = 1.0f;
	rotationMatrix[6] = 0.0f;
	rotationMatrix[7] = 0.0f;

	rotationMatrix[8] = 1.0f;
	rotationMatrix[9] = 0.0f;
	rotationMatrix[10] = 0.0f;
	rotationMatrix[11] = 0.0f;

	rotationMatrix[12] = 0.0f;
	rotationMatrix[13] = 0.0f;
	rotationMatrix[14] = 0.0f;
	rotationMatrix[15] = 1.0f;
	examineLatitude = 0.0f;
	examineLongitude = 90.0f;
}

void LDrawModelViewer::setupRightViewAngle(void)
{
	rotationMatrix[0] = 0.0f;
	rotationMatrix[1] = 0.0f;
	rotationMatrix[2] = 1.0f;
	rotationMatrix[3] = 0.0f;

	rotationMatrix[4] = 0.0f;
	rotationMatrix[5] = 1.0f;
	rotationMatrix[6] = 0.0f;
	rotationMatrix[7] = 0.0f;

	rotationMatrix[8] = -1.0f;
	rotationMatrix[9] = 0.0f;
	rotationMatrix[10] = 0.0f;
	rotationMatrix[11] = 0.0f;

	rotationMatrix[12] = 0.0f;
	rotationMatrix[13] = 0.0f;
	rotationMatrix[14] = 0.0f;
	rotationMatrix[15] = 1.0f;
	examineLatitude = 0.0f;
	examineLongitude = -90.0f;
}

void LDrawModelViewer::setupTopViewAngle(void)
{
	rotationMatrix[0] = 1.0f;
	rotationMatrix[1] = 0.0f;
	rotationMatrix[2] = 0.0f;
	rotationMatrix[3] = 0.0f;

	rotationMatrix[4] = 0.0f;
	rotationMatrix[5] = 0.0f;
	rotationMatrix[6] = 1.0f;
	rotationMatrix[7] = 0.0f;

	rotationMatrix[8] = 0.0f;
	rotationMatrix[9] = -1.0f;
	rotationMatrix[10] = 0.0f;
	rotationMatrix[11] = 0.0f;

	rotationMatrix[12] = 0.0f;
	rotationMatrix[13] = 0.0f;
	rotationMatrix[14] = 0.0f;
	rotationMatrix[15] = 1.0f;
	examineLatitude = 90.0f;
	examineLongitude = 0.0f;
}

void LDrawModelViewer::setupBottomViewAngle(void)
{
	rotationMatrix[0] = 1.0f;
	rotationMatrix[1] = 0.0f;
	rotationMatrix[2] = 0.0f;
	rotationMatrix[3] = 0.0f;

	rotationMatrix[4] = 0.0f;
	rotationMatrix[5] = 0.0f;
	rotationMatrix[6] = -1.0f;
	rotationMatrix[7] = 0.0f;

	rotationMatrix[8] = 0.0f;
	rotationMatrix[9] = 1.0f;
	rotationMatrix[10] = 0.0f;
	rotationMatrix[11] = 0.0f;

	rotationMatrix[12] = 0.0f;
	rotationMatrix[13] = 0.0f;
	rotationMatrix[14] = 0.0f;
	rotationMatrix[15] = 1.0f;
	examineLatitude = -90.0f;
	examineLongitude = 0.0f;
}

/*
void LDrawModelViewer::ldlErrorCallback(LDLError *error)
{
//	static int errorCount = 0;

	if (error)
	{
		TCStringArray *extraInfo = error->getExtraInfo();

		if (getDebugLevel() > 0)
		{
			printf("Error on line %d in: %s\n", error->getLineNumber(),
				error->getFilename());
			indentPrintf(4, "%s\n", error->getMessage());
			indentPrintf(4, "%s\n", error->getFileLine());
		}
		if (extraInfo)
		{
			int i;
			int count = extraInfo->getCount();

			for (i = 0; i < count; i++)
			{
				indentPrintf(4, "%s\n", (*extraInfo)[i]);
			}
		}
	}
}

void LDrawModelViewer::progressAlertCallback(TCProgressAlert *alert)
{
	if (alert)
	{
		printf("Progress message from %s:\n%s (%f)\n", alert->getSource(),
			alert->getMessage(), alert->getProgress());
	}
}
*/

bool LDrawModelViewer::haveLightDats(void) const
{
	return flags.drawLightDats && mainTREModel &&
		mainTREModel->getLightLocations().size() > 0;
}

bool LDrawModelViewer::haveStandardLight(void)
{
	return flags.useLighting && (!haveLightDats() || !flags.optionalStandardLight);
}

bool LDrawModelViewer::forceOneLight(void) const
{
	return flags.oneLight || flags.usesSpecular || !flags.defaultLightVector ||
		(flags.bfc && (flags.redBackFaces | flags.greenFrontFaces |
		flags.blueNeutralFaces)) || haveLightDats();
}

int LDrawModelViewer::loadModel(bool resetViewpoint)
{
	int retValue = 0;

	debugLog1s(LOGFILE_KEY, "Load Started for %s.\n", filename);
	TCAlertManager::sendAlert(loadAlertClass(), this, _UC("ModelLoading"));
	if (filename && filename[0])
	{
		if (clipAmount != 0.0f && resetViewpoint)
		{
			clipAmount = 0.0f;
			perspectiveView();
		}
		if (loadLDLModel() && parseModel())
		{
			retValue = 1;
			if (resetViewpoint)
			{
				resetView();
			}
		}
	}
	// This shouldn't be necessary, but something is occasionally setting the
	// needsReload flag during loading.
	flags.needsReload = false;
	flags.needsLightingSetup = true;
	if (retValue)
	{
		debugLog1s(LOGFILE_KEY, "Successfully loaded %s.\n", filename);
		TCAlertManager::sendAlert(loadAlertClass(), this, _UC("ModelLoaded"));
	}
	else
	{
		TCObject::release(mainModel);
		mainModel = NULL;
		releaseTREModels();
		step = -1;
		TCAlertManager::sendAlert(loadAlertClass(), this,
			_UC("ModelLoadCanceled"));
		requestRedraw();
		debugLog1s(LOGFILE_KEY, "Failed to load %s.\n", filename);
	}
	return retValue;
}

void LDrawModelViewer::releaseTREModels(void)
{
	TCObject::release(mainTREModel);
	mainTREModel = NULL;
	TCObject::release(whiteLightDirModel);
	whiteLightDirModel = NULL;
	TCObject::release(blueLightDirModel);
	blueLightDirModel = NULL;
	TCObject::release(highlightModel);
	highlightModel = NULL;
}

bool LDrawModelViewer::calcSize(void)
{
	bool abort = false;
	LDLModel *curModel = getCurModel();

	TCProgressAlert::send("LDrawModelViewer", ls(_UC("CalculatingSizeStatus")),
		0.0f, &abort, this);
	if (!abort)
	{
		curModel->getBoundingBox(boundingMin, boundingMax);
		TCProgressAlert::send("LDrawModelViewer",
			ls(_UC("CalculatingSizeStatus")), 0.5f, &abort, this);
	}
	if (!abort)
	{
		if (!flags.overrideModelCenter)
		{
			center = (boundingMin + boundingMax) / 2.0f;
		}
		if (!flags.overrideModelSize)
		{
			size = curModel->getMaxRadius(center, true) * 2.0f;
			if (mainModel->getBBoxIgnoreUsed())
			{
				clipSize = curModel->getMaxRadius(center, false) * 2.0f;
				if (size == 0.0)
				{
					size = clipSize;
				}
			}
			else
			{
				clipSize = size;
			}
		}
		TCProgressAlert::send("LDrawModelViewer",
			ls(_UC("CalculatingSizeStatus")), 1.0f, &abort, this);
		flags.needsCalcSize = false;
	}
	return !abort;
}

bool LDrawModelViewer::loadLDLModel(void)
{
	TCObject::release(mainModel);
	mainModel = new LDLMainModel;
	mainModel->setAlertSender(this);

	// First, release the current TREModels, if they exist.
	releaseTREModels();
	mainModel->setLDConfig(m_ldConfig);
	mainModel->setLowResStuds(!flags.qualityStuds);
	mainModel->setGreenFrontFaces(flags.bfc && flags.greenFrontFaces);
	mainModel->setRedBackFaces(flags.bfc && flags.redBackFaces);
	mainModel->setBlueNeutralFaces(flags.bfc && flags.blueNeutralFaces);
	mainModel->setBlackEdgeLines(flags.blackHighlights);
	mainModel->setExtraSearchDirs(extraSearchDirs);
	mainModel->setProcessLDConfig(flags.processLDConfig);
	mainModel->setSkipValidation(flags.skipValidation);
	mainModel->setBoundingBoxesOnly(flags.boundingBoxesOnly);
	mainModel->setSeamWidth(seamWidth);
	mainModel->setCheckPartTracker(flags.checkPartTracker);
	mainModel->setTexmaps(flags.texmaps);
	if (flags.needsResetMpd)
	{
		mpdChildIndex = 0;
		flags.needsResetMpd = false;
	}
#ifdef TIME_MODEL_LOAD
	auto start = std::chrono::high_resolution_clock::now();
#endif // TIME_MODEL_LOAD
	if (mainModel->load(filename))
	{
#ifdef TIME_MODEL_LOAD
		auto end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> elapsed_seconds = end-start;
		std::cout << "\n\nModel load of " << filename << " took " <<
			elapsed_seconds.count() << "s\n\n\n";
#endif // TIME_MODEL_LOAD
		return calcSize();
	}
	else
	{
		return false;
	}
}

bool LDrawModelViewer::parseModel(void)
{
	LDModelParser *modelParser = NULL;
	bool retValue = false;
	LDLModel *model = NULL;

	if (!mainModel)
	{
		return false;
	}
	if (flags.randomColors)
	{
		// Even though they're "random", make them the same each time the model
		// is loaded.
		srand((unsigned)time(NULL));
	}
	// Note: this is the only mainModel setting that's applied here in
	// parseModel, but it needs to be here to make it so that a reload isn't
	// required when you change the setting.  The setting doesn't affect the
	// loading of the model at all, just how it reports colors while parsing.
	mainModel->setRandomColors(flags.randomColors);
	TCAlertManager::sendAlert(loadAlertClass(), this, _UC("ModelParsing"));
	releaseTREModels();
	if (flags.needsCalcSize && !calcSize())
	{
		return false;
	}
	modelParser = new LDModelParser(this);
	modelParser->setAlertSender(this);
	modelParser->setTexmapsFlag(getTexmaps());
	model = getCurModel();
	if (modelParser->parseMainModel(model))
	{
		mainTREModel = modelParser->getMainTREModel();
		mainTREModel->setTexturesAfterTransparentFlag(getTexturesAfterTransparent());
		mainTREModel->setTextureOffsetFactor(getTextureOffsetFactor());
		mainTREModel->retain();
		flags.needsRecompile = false;
		flags.needsReparse = false;
		retValue = true;
		initLightDirModels();
		TCProgressAlert::send("LDrawModelViewer", ls(_UC("Done")), 2.0f, this);
		if (commandLineStep > 0)
		{
			setStep(commandLineStep);
			flags.needsResetStep = false;
			commandLineStep = -1;
		}
		if (flags.needsResetStep)
		{
			setStep(getNumSteps());
			flags.needsResetStep = false;
		}
		else
		{
			mainTREModel->setStep(step);
		}
	}
	modelParser->release();
	highlightPathsChanged();
	if (retValue)
	{
		TCAlertManager::sendAlert(loadAlertClass(), this, _UC("ModelParsed"));
	}
	else
	{
		TCAlertManager::sendAlert(loadAlertClass(), this,
			_UC("ModelParseCanceled"));
	}
	return retValue;
}

void LDrawModelViewer::setShowsHighlightLines(bool value)
{
	if (value != flags.showsHighlightLines)
	{
		flags.showsHighlightLines = value;
		flags.needsReparse = true;
	}
}

void LDrawModelViewer::setDrawConditionalHighlights(bool value)
{
	if (flags.drawConditionalHighlights != value)
	{
		flags.drawConditionalHighlights = value;
		if (flags.showsHighlightLines)
		{
			flags.needsReparse = true;
		}
	}
}

void LDrawModelViewer::setPerformSmoothing(bool value)
{
	if (flags.performSmoothing != value)
	{
		flags.performSmoothing = value;
		flags.needsReparse = true;
	}
}

void LDrawModelViewer::setForceZoomToFit(bool value)
{
	if (flags.forceZoomToFit != value)
	{
		flags.forceZoomToFit = value;
	}
}

void LDrawModelViewer::setSaveAlpha(bool value)
{
	if (flags.saveAlpha != value)
	{
		flags.saveAlpha = value;
	}
}

void LDrawModelViewer::setSkipValidation(bool value)
{
	if (flags.skipValidation != value)
	{
		flags.skipValidation = value;
		if (!value)
		{
			// If they turn the flag on, there's no point reloading.  If, on the
			// other hand, they turn the flag off, we need to reload so that we
			// can work around any problems the model might have.
			flags.needsReload = true;
		}
	}
}

void LDrawModelViewer::setProcessLDConfig(bool value)
{
	if (flags.processLDConfig != value)
	{
		flags.processLDConfig = value;
		flags.needsReload = true;
	}
}

void LDrawModelViewer::setMemoryUsage(int value)
{
	if (value < 0 || value > 2)
	{
		value = 2;
	}
	if (value != memoryUsage)
	{
		memoryUsage = value;
		flags.needsReparse = true;
	}
}

void LDrawModelViewer::setLineSmoothing(bool value)
{
	if (flags.lineSmoothing != value)
	{
		flags.lineSmoothing = value;
		flags.needsReparse = true;
	}
}

void LDrawModelViewer::setSubduedLighting(bool value)
{
	if (value != flags.subduedLighting)
	{
		flags.subduedLighting = value;
		flags.needsLightingSetup = true;
	}
}

bool LDrawModelViewer::recompile(void)
{
	if (mainTREModel)
	{
		if (whiteLightDirModel != NULL)
		{
			// It crashes after an FSAA change if we simply recompile here.
			// I suspect it's related to the VBO/VAR extensions.  Deleting the
			// lightDirModel objects and recreating them causes the vertex
			// buffers to be reloaded from scratch.
			whiteLightDirModel->release();
			whiteLightDirModel = NULL;
			blueLightDirModel->release();
			blueLightDirModel = NULL;
			initLightDirModels();
		}
		if (highlightModel != NULL)
		{
			// See comment above about crash.
			highlightModel->release();
			highlightModel = NULL;
			highlightPathsChanged();
		}
		mainTREModel->recompile();
		flags.needsRecompile = false;
		TCProgressAlert::send("LDrawModelViewer", ls(_UC("Done")), 2.0f, this);
	}
	return true;
}

void LDrawModelViewer::uncompile(void)
{
	if (fontListBase)
	{
		glDeleteLists(fontListBase, 128);
		fontListBase = 0;
	}
	if (mainTREModel)
	{
		mainTREModel->uncompile();
	}
	if (whiteLightDirModel)
	{
		whiteLightDirModel->uncompile();
	}
	if (blueLightDirModel)
	{
		blueLightDirModel->uncompile();
	}
	if (highlightModel)
	{
		highlightModel->uncompile();
	}
}

void LDrawModelViewer::reparse(void)
{
	if (mainModel)
	{
		parseModel();
	}
	flags.needsReparse = false;
}

void LDrawModelViewer::reload(void)
{
	if (filename)
	{
		bool lastStep = step == getNumSteps() - 1;

		loadModel(false);
		if (lastStep)
		{
			setStep(getNumSteps());
		}
	}
	flags.needsReload = false;
}

void LDrawModelViewer::setObi(bool value)
{
	if (value != flags.obi)
	{
		flags.obi = value;
		flags.needsReparse = true;
	}
}

void LDrawModelViewer::setGl2ps(bool value)
{
	if (value != flags.gl2ps)
	{
		flags.gl2ps = value;
		if (mainTREModel)
		{
			mainTREModel->setGl2psFlag(value);
		}
	}
}

void LDrawModelViewer::setUsesSpecular(bool value)
{
	if (value != flags.usesSpecular)
	{
		flags.usesSpecular = value;
		flags.needsMaterialSetup = true;
		flags.needsLightingSetup = true;
	}
}

void LDrawModelViewer::setOneLight(bool value)
{
	if (value != flags.oneLight)
	{
		flags.oneLight = value;
		flags.needsLightingSetup = true;
	}
}

void LDrawModelViewer::setupMaterial(void)
{
//	GLfloat mAmbient[] = {0.5f, 0.5f, 0.5f, 1.0f};
	GLfloat mAmbient[] = {0.0f, 0.0f, 0.0f, 1.0f};
//	GLfloat mSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};
	GLfloat mSpecular[] = {0.5f, 0.5f, 0.5f, 1.0f};
//	GLfloat mSpecular[] = {0.0f, 0.0f, 0.0f, 1.0f};

	// Note: default emission is <0,0,0,1>, which is what we want.
	if (!flags.usesSpecular)
	{
		mSpecular[0] = mSpecular[1] = mSpecular[2] = 0.0f;
	}
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mAmbient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mSpecular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 64.0f);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);
	flags.needsMaterialSetup = false;
}

void LDrawModelViewer::setupLight(GLenum light, const TCVector &color)
{
	GLfloat lDiffuse[4];
	GLfloat lSpecular[4];
	int i;

	if (flags.subduedLighting)
	{
		for (i = 0; i < 3; i++)
		{
			lDiffuse[i] = color.get(i) * 0.5f;
		}
	}
	else
	{
		for (i = 0; i < 3; i++)
		{
			lDiffuse[i] = color.get(i);
		}
	}
	if (!flags.usesSpecular)
	{
		lSpecular[0] = lSpecular[1] = lSpecular[2] = 0.0f;
	}
	else
	{
		for (i = 0; i < 3; i++)
		{
			lSpecular[i] = color.get(i);
		}
	}
	lDiffuse[3] = 1.0f;
	lSpecular[3] = 1.0f;
	glLightfv(light, GL_SPECULAR, lSpecular);
	glLightfv(light, GL_DIFFUSE, lDiffuse);
	glEnable(light);
}

void LDrawModelViewer::setupLighting(void)
{
	glDisable(GL_NORMALIZE);
	if (flags.useLighting)
	{
		GLint maxLights;
		int i;
		bool lightDats = haveLightDats();
		GLfloat ambient[] = {0.2f, 0.2f, 0.2f, 1.0f};

		if (flags.subduedLighting)
		{
			ambient[0] = ambient[1] = ambient[2] = 0.7f;
		}
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
		glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
		glEnable(GL_LIGHTING);
		glGetIntegerv(GL_MAX_LIGHTS, &maxLights);
		for (i = 0; i < maxLights; i++)
		{
			glDisable(GL_LIGHT0 + i);
		}
		if (!lightDats || !flags.optionalStandardLight)
		{
			setupLight(GL_LIGHT0);
			glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1.0f);
			glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.0f);
			glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.0f);
		}
		if (lightDats)
		{
			const TCULongList &lightColors = mainTREModel->getLightColors();
			TCULongList::const_iterator itColor = lightColors.begin();
			float lightScale = 1.0f / 255.0f;
			float atten = (float)sqr(size);
			int start = 0;

			glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
			if (!flags.optionalStandardLight)
			{
				start = 1;
			}
			for (i = start; i < maxLights && itColor != lightColors.end(); i++)
			{
				TCByte rgb[4];
				TCULong color = htonl(*itColor);
				float minBrightness = 0.5f;
				float brightness;

				memcpy(rgb, &color, 4);
				brightness = std::max(std::max(rgb[0] * lightScale,
					rgb[1] * lightScale), rgb[2] * lightScale) * (1.0f - minBrightness) +
					minBrightness;
				setupLight(GL_LIGHT0 + i, TCVector(rgb[0] * lightScale,
					rgb[1] * lightScale, rgb[2] * lightScale));
				glLightf(GL_LIGHT0 + i, GL_CONSTANT_ATTENUATION, 0.5f);
				glLightf(GL_LIGHT0 + i, GL_LINEAR_ATTENUATION, 0.0f);//1.0f / size);
				glLightf(GL_LIGHT0 + i, GL_QUADRATIC_ATTENUATION, 2.0f / atten /
					brightness);
				itColor++;
			}
		}
		else
		{
			if (forceOneLight())
			{
				glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
			}
			else
			{
				setupLight(GL_LIGHT1);
				glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);
			}
		}
		flags.needsLightingSetup = false;
	}
	else
	{
		glDisable(GL_LIGHTING);
		flags.needsLightingSetup = false;
	}
}

void LDrawModelViewer::setFontData(TCByte *fontData, long length)
{
	if (length == 4096)
	{
		int i, j;
		TCByte *imageData = NULL;
		int rowSize;
		int imageSize;

		fontImage1x = new TCImage;
		fontImage1x->setFlipped(true);
		fontImage1x->setLineAlignment(4);
		fontImage1x->setDataFormat(TCRgba8);
		fontImage1x->setSize(FONT_IMAGE_WIDTH, FONT_IMAGE_HEIGHT);
		fontImage1x->allocateImageData();
		imageData = fontImage1x->getImageData();
		rowSize = fontImage1x->getRowSize();
		imageSize = rowSize * FONT_IMAGE_HEIGHT;
		for (i = 0; i < imageSize; i++)
		{
			if (i % 4 == 3)
			{
				imageData[i] = 0x00;	// Init alpha channel to clear.
			}
			else
			{
				imageData[i] = 0xFF;	// Init color channels to white.
			}
		}
		for (i = 0; i < 4096; i++)
		{
			int row = i / 256 * 16 + i % 16;	// logical row (kinda)
			int col = i / 16 % 16;				// logical column
			int yOffset = (FONT_IMAGE_HEIGHT - row - 1) * rowSize;
			TCByte fontByte = fontData[i];

			for (j = 0; j < 8; j++)
			{
				if (fontByte & (1 << (7 - j)))
				{
					int offset = yOffset + (col * FONT_CHAR_WIDTH + j) * 4 + 3;

					// That spot should be on, so set the bit.
					if (offset < 131072)
					{
						imageData[offset] = 0xFF;
					}
				}
			}
		}
	}
}

// Loads a font file in the format of VGA text-mode font data.
void LDrawModelViewer::loadVGAFont(const char *fontFilename)
{
	if (fontImage1x == NULL)
	{
		FILE *fontFile = ucfopen(fontFilename, "rb");

		if (fontFile)
		{
			TCByte fontData[4096];

			if (fread(fontData, 1, 4096, fontFile) == 4096)
			{
				setFontData(fontData, 4096);
			}
			fclose(fontFile);
		}
	}
}

void LDrawModelViewer::setFont2x(TCImage *image)
{
	if (fontImage2x == NULL)
	{
		fontImage2x = TCObject::retain(image);
	}
}

void LDrawModelViewer::setRawFont2xData(const TCByte *data, long length)
{
	if (fontImage2x == NULL)
	{
		int rowSize;
		const int fontWidth = 256;
		const int fontHeight = 512;
		
		fontImage2x = new TCImage;
		fontImage2x->setFlipped(true);
		fontImage2x->setLineAlignment(4);
		fontImage2x->setDataFormat(TCRgba8);
		fontImage2x->setSize(fontWidth, fontHeight);
		fontImage2x->allocateImageData();
		rowSize = fontImage2x->getRowSize();
		if (length == rowSize * fontHeight)
		{
			TCByte *imageData = fontImage2x->getImageData();
			int i;
			
			for (i = 0; i < fontHeight; ++i)
			{
				memcpy(imageData + rowSize * (fontHeight - 1 - i),
					   data + rowSize * i, rowSize);
			}
		}
		else
		{
			fontImage2x->release();
			fontImage2x = NULL;
		}
	}
}

void LDrawModelViewer::setupFont2x(const char *fontFilename)
{
	if (fontImage2x == NULL)
	{
		fontImage2x = new TCImage;
		fontImage2x->setFlipped(true);
		fontImage2x->setLineAlignment(4);
		if (!fontImage2x->loadFile(fontFilename))
		{
			fontImage2x->release();
		}
	}
	setupFont(NULL);
}

void LDrawModelViewer::setupFont(const char *fontFilename)
{
//	printf("LDrawModelViewer::setupFont\n");
	if (fontFilename)
	{
		loadVGAFont(fontFilename);
	}
	TCImage *fontImage = fontImage1x;
	TCFloat fontScale = scaleFactor;
	if (scaleFactor >= 1.5 && fontImage2x != NULL)
	{
		fontImage = fontImage2x;
		fontScale = scaleFactor / 2.0;
	}
	if (fontImage != NULL)
	{
		int i;
		int fontImageWidth = fontImage->getWidth();
		int fontImageHeight = fontImage->getHeight();
		int fontCharWidth = fontImageWidth / 16;
		int fontCharHeight = fontImageHeight / 16;

		glGenTextures(1, &fontTextureID);
		glBindTexture(GL_TEXTURE_2D, fontTextureID);
		glTexImage2D(GL_TEXTURE_2D, 0, 4, fontImageWidth, fontImageHeight,
			0, GL_RGBA, GL_UNSIGNED_BYTE, fontImage->getImageData());
		if (fontListBase)
		{
			glDeleteLists(fontListBase, FONT_NUM_CHARACTERS);
		}
		fontListBase = glGenLists(FONT_NUM_CHARACTERS);
		for (i = 0; i < FONT_NUM_CHARACTERS; i++)
		{
			TCFloat cx, cy;
			TCFloat wx, hy;
			TCFloat tx, ty;

			cx = (TCFloat)(i % 16) * fontCharWidth /
				(TCFloat)(fontImageWidth);
			cy = (TCFloat)(i / 16) * fontCharHeight /
				(TCFloat)(fontImageHeight);
			wx = (TCFloat)fontCharWidth / fontImageWidth;
			hy = (TCFloat)fontCharHeight / fontImageHeight;
			glNewList(fontListBase + i, GL_COMPILE);
				glBegin(GL_QUADS);
					tx = cx;
					ty = 1.0f - cy - hy;
					treGlTexCoord2f(tx, ty);			// Bottom Left
					glVertex2i(0, 0);
					tx = cx + wx;
					ty = 1.0f - cy - hy;
					treGlTexCoord2f(tx, ty);			// Bottom Right
					glVertex2i(fontCharWidth * fontScale, 0);
					tx = cx + wx;
					ty = 1 - cy;
					treGlTexCoord2f(tx, ty);			// Top Right
					glVertex2i(fontCharWidth * fontScale,
						fontCharHeight * fontScale);
					tx = cx;
					ty = 1 - cy;
					treGlTexCoord2f(tx , ty);			// Top Left
					glVertex2i(0, fontCharHeight * fontScale);
				glEnd();
				glTranslated((fontCharWidth + 1) * fontScale, 0, 0);
			glEndList();
		}
	}
}

void LDrawModelViewer::setupTextures(void)
{
	if (programPath)
	{
		char textureFilename[1024];

		sprintf(textureFilename, "%s/StudLogo.png", programPath);
		TREMainModel::loadStudTexture(textureFilename);
//		sprintf(textureFilename, "%s/Font.png", programPath);
		sprintf(textureFilename, "%s/SansSerif.fnt", programPath);
		setupFont(textureFilename);
	}
	else if (fontImage1x != NULL)
	{
		setupFont(NULL);
	}
	flags.needsTextureSetup = false;
}

void LDrawModelViewer::setup(void)
{
	glEnable(GL_DEPTH_TEST);
	setupLighting();
	setupMaterial();
	setupTextures();
	flags.needsSetup = false;
}

void LDrawModelViewer::drawBoundingBox(void)
{
	int lightingEnabled = glIsEnabled(GL_LIGHTING);

	if (lightingEnabled)
	{
		glDisable(GL_LIGHTING);
	}
	if ((backgroundR + backgroundG + backgroundB) / 3.0 < 0.5)
	{
		glColor3ub(255, 255, 255);
	}
	else
	{
		glColor3ub(0, 0, 0);
	}
	glBegin(GL_LINE_STRIP);
		treGlVertex3fv(boundingMin);
		treGlVertex3f(boundingMax[0], boundingMin[1], boundingMin[2]);
		treGlVertex3f(boundingMax[0], boundingMax[1], boundingMin[2]);
		treGlVertex3f(boundingMin[0], boundingMax[1], boundingMin[2]);
		treGlVertex3fv(boundingMin);
		treGlVertex3f(boundingMin[0], boundingMin[1], boundingMax[2]);
		treGlVertex3f(boundingMax[0], boundingMin[1], boundingMax[2]);
		treGlVertex3fv(boundingMax);
		treGlVertex3f(boundingMin[0], boundingMax[1], boundingMax[2]);
		treGlVertex3f(boundingMin[0], boundingMin[1], boundingMax[2]);
	glEnd();
	glBegin(GL_LINES);
		treGlVertex3f(boundingMin[0], boundingMax[1], boundingMin[2]);
		treGlVertex3f(boundingMin[0], boundingMax[1], boundingMax[2]);
		treGlVertex3f(boundingMax[0], boundingMax[1], boundingMin[2]);
		treGlVertex3f(boundingMax[0], boundingMax[1], boundingMax[2]);
		treGlVertex3f(boundingMax[0], boundingMin[1], boundingMin[2]);
		treGlVertex3f(boundingMax[0], boundingMin[1], boundingMax[2]);
	glEnd();
	if (lightingEnabled)
	{
		glEnable(GL_LIGHTING);
	}
}

void LDrawModelViewer::orthoView(void)
{
	int actualWidth = (int)scale(width);
	int actualHeight = (int)scale(height);
	const char *glVendor = "";
	const GLubyte *origVendorString = glGetString(GL_VENDOR);

	if (origVendorString)
	{
		glVendor = (const char *)origVendorString;
	}
	if (stereoMode == LDVStereoCrossEyed || stereoMode == LDVStereoParallel)
	{
		glViewport(0, 0, actualWidth, actualHeight);
	}
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, actualWidth, 0.0, actualHeight);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	if (strncmp(glVendor, "ATI Technologies Inc.", 3) != 0)
	{
		// This doesn't work right on ATI video cards, so skip.
		treGlTranslatef(0.375f, 0.375f, 0.0f);
	}
}

void LDrawModelViewer::setSeamWidth(TCFloat value)
{
	if (!fEq(value, seamWidth))
	{
		seamWidth = value;
		flags.needsReparse = true;
		if (mainModel)
		{
			mainModel->setSeamWidth(seamWidth);
		}
		flags.needsCalcSize = true;
	}
}

void LDrawModelViewer::drawString(TCFloat xPos, TCFloat yPos, char* string)
{
	if (!fontListBase)
	{
		setupTextures();
	}
	if (!fontListBase)
	{
		// Font file wasn't found; bail.
		return;
	}
	if ((backgroundR + backgroundG + backgroundB) / 3.0f > 0.5)
	{
		glColor3ub(0, 0, 0);
	}
	else
	{
		glColor3ub(255, 255, 255);
	}
	orthoView();
	treGlTranslatef(xPos * scaleFactor, yPos * scaleFactor, 0);
	glPushAttrib(GL_ENABLE_BIT | GL_POLYGON_BIT | GL_TEXTURE_BIT);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	enable(GL_BLEND);
	blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindTexture(GL_TEXTURE_2D, fontTextureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glListBase(fontListBase);
	glCallLists((GLsizei)strlen(string), GL_UNSIGNED_BYTE, string);
	glPopAttrib();
	perspectiveView();
}

void LDrawModelViewer::drawFPS(TCFloat fps)
{
	if (mainTREModel && fps > 0.0f)
	{
		char fpsString[1024];
		int lightingEnabled = glIsEnabled(GL_LIGHTING);
		int zBufferEnabled = glIsEnabled(GL_DEPTH_TEST);

		if (lightingEnabled)
		{
			glDisable(GL_LIGHTING);
		}
		if (zBufferEnabled)
		{
			glDisable(GL_DEPTH_TEST);
		}
		sprintf(fpsString, "%4.4f", fps);
		drawString(2.0f, 0.0f, fpsString);
		if (lightingEnabled)
		{
			glEnable(GL_LIGHTING);
		}
		if (zBufferEnabled)
		{
			glEnable(GL_DEPTH_TEST);
		}
	}
}

void LDrawModelViewer::drawLight(GLenum light, TCFloat x, TCFloat y, TCFloat z)
{
	GLfloat position[4];

	position[0] = (GLfloat)x;
	position[1] = (GLfloat)y;
	position[2] = (GLfloat)z;
	// 0.0 in the w component of the light "position" vector means that this is
	// a directional light.  As such, the "position" provided is actually a
	// direction (or maybe the inverse of the direction).
	position[3] = 0.0f;
	glLightfv(light, GL_POSITION, position);
}

void LDrawModelViewer::drawLights(void)
{
	drawLight(GL_LIGHT0, lightVector[0], lightVector[1], lightVector[2]);
	if (!forceOneLight())
	{
		drawLight(GL_LIGHT1, -lightVector[0], -lightVector[1], -lightVector[2]);
	}
}

void LDrawModelViewer::setLightVector(const TCVector &value)
{
	TCFloat length = value.length();
	bool oldForce = forceOneLight();

	if (length)
	{
		lightVector = value / length;
	}
	if (lightVector.approxEquals(TCVector(0.0f, 0.0f, 1.0f), 0.000001f))
	{
		flags.defaultLightVector = true;
	}
	else
	{
		flags.defaultLightVector = false;
	}
	if (forceOneLight() != oldForce)
	{
		flags.needsLightingSetup = true;
	}
}

void LDrawModelViewer::setBackgroundRGB(int r, int g, int b)
{
	setBackgroundRGBA(r, g, b, 255);
/*
	backgroundR = (GLclampf)r / 255.0f;
	backgroundG = (GLclampf)g / 255.0f;
	backgroundB = (GLclampf)b / 255.0f;
	backgroundA = 1.0f;
*/
}

void LDrawModelViewer::setDefaultRGB(TCByte r, TCByte g, TCByte b,
									 bool transparent)
{
	if (defaultR != r || defaultG != g || defaultB != b ||
		flags.defaultTrans != transparent)
	{
		defaultR = r;
		defaultG = g;
		defaultB = b;
		flags.defaultTrans = transparent;
		if (mainTREModel && defaultColorNumber == -1)
		{
			flags.needsReparse = true;
		}
	}
}

void LDrawModelViewer::getDefaultRGB(
	TCByte &r,
	TCByte &g,
	TCByte &b,
	bool &transparent) const
{
	r = defaultR;
	g = defaultG;
	b = defaultB;
	transparent = flags.defaultTrans;
}

void LDrawModelViewer::setDefaultColorNumber(int value)
{
	if (value != defaultColorNumber)
	{
		defaultColorNumber = value;
		flags.needsReparse = true;
	}
}

void LDrawModelViewer::setBackgroundRGBA(int r, int g, int b, int a)
{
	backgroundR = (GLclampf)r / 255.0f;
	backgroundG = (GLclampf)g / 255.0f;
	backgroundB = (GLclampf)b / 255.0f;
	backgroundA = (GLclampf)a / 255.0f;
}

void LDrawModelViewer::setRedBackFaces(bool value)
{
	if (value != flags.redBackFaces)
	{
		flags.redBackFaces = value;
		if (flags.bfc)
		{
			flags.needsReload = true;
			flags.needsLightingSetup = true;
		}
	}
}

void LDrawModelViewer::setGreenFrontFaces(bool value)
{
	if (value != flags.greenFrontFaces)
	{
		flags.greenFrontFaces = value;
		if (flags.bfc)
		{
			flags.needsReload = true;
			flags.needsLightingSetup = true;
		}
	}
}

void LDrawModelViewer::setBlueNeutralFaces(bool value)
{
	if (value != flags.blueNeutralFaces)
	{
		flags.blueNeutralFaces = value;
		if (flags.bfc)
		{
			flags.needsReload = true;
			flags.needsLightingSetup = true;
		}
	}
}

void LDrawModelViewer::setBfc(bool value)
{
	if (value != flags.bfc)
	{
		flags.bfc = value;
		flags.needsReparse = true;
		if (flags.redBackFaces || flags.greenFrontFaces ||
			flags.blueNeutralFaces)
		{
			flags.needsReload = true;
			//flags.needsMaterialSetup = true;
			flags.needsLightingSetup = true;
		}
	}
}

void LDrawModelViewer::setDrawWireframe(bool value)
{
	if (value != flags.drawWireframe)
	{
		flags.drawWireframe = value;
		flags.needsReparse = true;
	}
}

void LDrawModelViewer::setUseWireframeFog(bool value)
{
	flags.useWireframeFog = value;
}

void LDrawModelViewer::setDrawLightDats(bool value)
{
	if (value != flags.drawLightDats)
	{
		flags.drawLightDats = value;
		flags.needsLightingSetup = true;
	}
}

void LDrawModelViewer::setOptionalStandardLight(bool value)
{
	if (value != flags.optionalStandardLight)
	{
		flags.optionalStandardLight = value;
		flags.needsLightingSetup = true;
	}
}

void LDrawModelViewer::setNoLightGeom(bool value)
{
	if (value != flags.noLightGeom)
	{
		flags.noLightGeom = value;
		flags.needsReparse = true;
	}
}

void LDrawModelViewer::setRemoveHiddenLines(bool value)
{
	if (value != flags.removeHiddenLines)
	{
		flags.removeHiddenLines = value;
		flags.needsReparse = true;
	}
}

void LDrawModelViewer::setEdgesOnly(bool value)
{
	if (value != flags.edgesOnly)
	{
		flags.edgesOnly = value;
	}
	if (mainTREModel)
	{
		bool realValue = flags.edgesOnly && flags.showsHighlightLines;

		if (realValue != mainTREModel->getEdgesOnlyFlag())
		{
			flags.needsReparse = true;
		}
	}
}

void LDrawModelViewer::setHiResPrimitives(bool value)
{
	if (value != flags.hiResPrimitives)
	{
		flags.hiResPrimitives = value;
		flags.needsReparse = true;
	}
}

void LDrawModelViewer::setUsePolygonOffset(bool value)
{
	if (value != flags.usePolygonOffset)
	{
		flags.usePolygonOffset = value;
		if (mainTREModel)
		{
			mainTREModel->setPolygonOffsetFlag(flags.usePolygonOffset);
			if (!flags.usePolygonOffset)
			{
				disable(GL_POLYGON_OFFSET_FILL);
			}
		}
	}
}

void LDrawModelViewer::setMultiThreaded(bool value)
{
	if (value != flags.multiThreaded)
	{
		flags.multiThreaded = value;
		flags.needsReparse = true;
	}
	
}

void LDrawModelViewer::setBlackHighlights(bool value)
{
	if (value != flags.blackHighlights)
	{
		flags.blackHighlights = value;
		if (flags.showsHighlightLines)
		{
			flags.needsReload = true;
		}
	}
}

void LDrawModelViewer::setShowAllConditionalLines(bool value)
{
	if (value != flags.showAllConditionalLines)
	{
		flags.showAllConditionalLines = value;
		flags.needsReparse = true;
	}
}

void LDrawModelViewer::setShowConditionalControlPoints(bool value)
{
	if (value != flags.showConditionalControlPoints)
	{
		flags.showConditionalControlPoints = value;
		flags.needsReparse = true;
	}
}

void LDrawModelViewer::setCurveQuality(int value)
{
	if (value != curveQuality)
	{
		curveQuality = value;
		if (flags.allowPrimitiveSubstitution)
		{
			flags.needsReparse = true;
		}
	}
}

void LDrawModelViewer::setRandomColors(bool value)
{
	if (value != flags.randomColors)
	{
		flags.randomColors = value;
		flags.needsReparse = true;
	}
}

void LDrawModelViewer::setTextureStuds(bool value)
{
	if (value != flags.textureStuds)
	{
		flags.textureStuds = value;
//		TGLStudLogo::setTextureStuds(value);
		flags.needsReparse = true;
	}
}

void LDrawModelViewer::setTextureFilterType(int value)
{
	if (value != textureFilterType)
	{
		textureFilterType = value;
		if (mainTREModel)
		{
			mainTREModel->setStudTextureFilter(value);
		}
//		TGLStudLogo::setTextureFilterType(value);
//		flags.needsRecompile = true;
//		flags.needsTextureSetup = true;
	}
}

void LDrawModelViewer::setScaleFactor(TCFloat value)
{
	if (value != scaleFactor)
	{
		if (fontListBase)
		{
			glDeleteLists(fontListBase, 128);
			fontListBase = 0;
		}
		scaleFactor = value;
		flags.needsResize = true;
		if (mainTREModel)
		{
			mainTREModel->setEdgeLineWidth(getScaledHighlightLineWidth());
		}
		requestRedraw();
	}
}

void LDrawModelViewer::setWidth(TCFloat value)
{
	if (value != width)
	{
		width = value;
		flags.needsResize = true;
	}
}

void LDrawModelViewer::setHeight(TCFloat value)
{
	if (value != height)
	{
		height = value;
		flags.needsResize = true;
	}
}

void LDrawModelViewer::setXTile(int value)
{
	if (value != xTile)
	{
		xTile = value;
		flags.needsResize = true;
	}
}

void LDrawModelViewer::setYTile(int value)
{
	if (value != yTile)
	{
		yTile = value;
		flags.needsResize = true;
	}
}

void LDrawModelViewer::setNumXTiles(int value)
{
	if (value != numXTiles)
	{
		numXTiles = value;
		flags.needsResize = true;
	}
}

void LDrawModelViewer::setNumYTiles(int value)
{
	if (value != numYTiles)
	{
		numYTiles = value;
		flags.needsResize = true;
	}
}

void LDrawModelViewer::setStereoMode(LDVStereoMode mode)
{
	if (mode != stereoMode)
	{
		stereoMode = mode;
		flags.needsResize = true;
	}
}

void LDrawModelViewer::setCutawayMode(LDVCutawayMode mode)
{
	cutawayMode = mode;
}

TCFloat32 LDrawModelViewer::getScaledCutawayLineWidth(void) const
{
	return std::max(1.0f, scale(cutawayLineWidth));
}

void LDrawModelViewer::setCutawayLineWidth(TCFloat32 value)
{
	cutawayLineWidth = value;
}

void LDrawModelViewer::setCutawayAlpha(TCFloat32 value)
{
	cutawayAlpha = value;
}

void LDrawModelViewer::setUseLighting(bool value)
{
	if (value != flags.useLighting)
	{
		flags.useLighting = value;
		if (mainTREModel)
		{
			mainTREModel->setLightingFlag(value);
		}
		flags.needsRecompile = true;
		flags.needsLightingSetup = true;
	}
}

void LDrawModelViewer::setUseStipple(bool value)
{
	if (value != flags.useStipple)
	{
		flags.useStipple = value;
		if (mainTREModel)
		{
			mainTREModel->setStippleFlag(value);
		}
		if (flags.sortTransparent)
		{
			flags.needsReparse = true;
		}
		else
		{
//			flags.needsRecompile = true;
		}
	}
}

void LDrawModelViewer::setSortTransparent(bool value)
{
	if (value != flags.sortTransparent)
	{
		flags.sortTransparent = value;
		flags.needsReparse = true;
	}
}

TCFloat32 LDrawModelViewer::getScaledHighlightLineWidth(void) const
{
	return std::max(1.0f, scale(highlightLineWidth));
}

void LDrawModelViewer::setHighlightLineWidth(TCFloat32 value)
{
	if (value != highlightLineWidth)
	{
		highlightLineWidth = value;
		if (mainTREModel)
		{
			mainTREModel->setEdgeLineWidth(getScaledHighlightLineWidth());
		}
	}
}

TCFloat32 LDrawModelViewer::getScaledWireframeLineWidth(void) const
{
	return std::max(1.0f, scale(wireframeLineWidth));
}

void LDrawModelViewer::setWireframeLineWidth(TCFloat32 value)
{
	wireframeLineWidth = value;
}

void LDrawModelViewer::setAnisoLevel(TCFloat32 value)
{
	if (value != anisoLevel)
	{
		anisoLevel = value;
		if (mainTREModel)
		{
			mainTREModel->setStudAnisoLevel(anisoLevel);
		}
	}
}

void LDrawModelViewer::setTexmaps(bool value)
{
	if (value != flags.texmaps)
	{
		flags.texmaps = value;
		flags.needsReload = true;
	}
}

void LDrawModelViewer::setTexturesAfterTransparent(bool value)
{
	if (value != flags.texturesAfterTransparent)
	{
		flags.texturesAfterTransparent = value;
		if (mainTREModel)
		{
			mainTREModel->setTexturesAfterTransparentFlag(value);
		}
		requestRedraw();
	}
}

void LDrawModelViewer::setTextureOffsetFactor(TCFloat value)
{
	if (value != textureOffsetFactor)
	{
		textureOffsetFactor = value;
		if (mainTREModel)
		{
			mainTREModel->setTextureOffsetFactor(value);
		}
		requestRedraw();
	}
}

void LDrawModelViewer::setQualityStuds(bool value)
{
	if (value != flags.qualityStuds)
	{
		flags.qualityStuds = value;
		flags.needsReload = true;
	}
}

void LDrawModelViewer::setAllowPrimitiveSubstitution(bool value)
{
	if (value != flags.allowPrimitiveSubstitution)
	{
		flags.allowPrimitiveSubstitution = value;
		flags.needsReparse = true;
	}
}

void LDrawModelViewer::updateCameraPosition(void)
{
	float multiplier = 100.0f;

#ifdef USE_STD_CHRONO
	std::chrono::duration<double> diff = std::chrono::steady_clock::now() -
		frameTime;
	float factor = diff.count();
#else
	float factor;
#ifdef WIN32
	if (hrpcFrequency.QuadPart != 0)
	{
		LARGE_INTEGER newCount;

		QueryPerformanceCounter(&newCount);
		factor = (float)((newCount.QuadPart - hrpcFrameCount.QuadPart) /
			(double)hrpcFrequency.QuadPart);
	}
	else
	{
		factor = (GetTickCount() - frameTicks) / 1000.0f;
	}
#endif // WIN32
#ifdef _QT
	factor = qtime.elapsed() /1000.0f;
#endif
#ifdef COCOA
	if (FRAME_TIME != nil)
	{
		factor = (float)-[FRAME_TIME timeIntervalSinceNow];
	}
#endif // COCOA
#if !defined(WIN32) && !defined(_QT) && !defined(COCOA)
	factor = 1.0f / multiplier;
#endif // None of the above
#endif // !USE_STD_CHRONO
	if (viewMode == VMWalk)
	{
		TCVector upVector(0.0f, -1.0f, 0.0f);
		TCVector tempMotion(0.0f, 0.0f, 0.0f);
		TCFloat matrix[16];
		TCFloat inverseMatrix[16];
		TCVector::invertMatrix(camera.getFacing().getMatrix(), inverseMatrix);
		TCVector::multMatrix(inverseMatrix, rotationMatrix, matrix);
		TCVector tempVector = upVector.transformNormal(matrix);
		TCFloat motionAmount = 20.0f * (TCFloat)sqrt(fov / 45.0f);

//		TCFloat identity[16];
//		memcpy(identity, TCVector::sm_identityMatrix, sizeof(TCVector::sm_identityMatrix));
//		TCVector::calcRotationMatrix(cameraXRotate, cameraYRotate, identity);

		TCFloat identity[16] =
		{
			1.0, 0.0, 0.0, 0.0,
			0.0, 1.0, 0.0, 0.0,
			0.0, 0.0, 1.0, 0.0,
			0.0, 0.0, 0.0, 1.0
		};
		TCVector::calcRotationMatrix(cameraXRotate, cameraYRotate, identity);
		TCVector whateverVector = upVector.transformNormal(identity);

		if (cameraMotion[2] > 0.0f)
		{
			tempMotion[1] += -tempVector[2];
			tempMotion[2] += tempVector[1];
		}
		else if (cameraMotion[2] < 0.0f)
		{
			tempMotion[1] += tempVector[2];
			tempMotion[2] += -tempVector[1];
		}
		if (cameraMotion[1] > 0.0f)
		{
			tempMotion[1] += (1 - tempVector[2]);
			tempMotion[2] += (1 - tempVector[1]);
		}
		else if (cameraMotion[1] < 0.0f)
		{
			tempMotion[1] += -(1 - tempVector[2]);
			tempMotion[2] += -(1 - tempVector[1]);
		}
		tempMotion[0] = cameraMotion[0];
		camera.move(tempMotion * motionAmount * factor * multiplier);
	}
	else
	{
		camera.move(cameraMotion * size / 100.0f * factor * multiplier);
	}
	camera.rotate(TCVector(cameraXRotate, cameraYRotate, cameraZRotate) *
		factor * multiplier * 1.5f);
}

void LDrawModelViewer::zoom(TCFloat amount, bool apply)
{
	if (flags.paused)
	{
		return;
	}
	if (clipZoom)
	{
		nextClipAmount = clipAmount - amount / 1000.0f;

		if (nextClipAmount > aspectRatio * 2)
		{
			nextClipAmount = aspectRatio * 2;
		}
/*
		if (nextClipAmount > 1.0f)
		{
			nextClipAmount = 1.0f;
		}
*/
		else if (nextClipAmount < 0.0f)
		{
			nextClipAmount = 0.0f;
		}
	}
	else
	{
		TCFloat distance = (camera.getPosition()).length();
//		TCFloat distance = (camera.getPosition() - center).length();

		nextDistance = distance + (amount * distance / 300.0f);
		if (flags.constrainZoom && !skipCameraPositioning())
		{
			if (nextDistance < size / zoomMax)
			{
				nextDistance = size / zoomMax;
			}
		}
		// We may as well always constrain the maximum zoom, since there not
		// really any reason to move too far away.
		if (nextDistance > defaultDistance * 10.0f && !skipCameraPositioning())
		{
			nextDistance = defaultDistance * 10.0f;
		}
	}
	if (apply)
	{
		applyZoom();
	}
	if (!fEq(amount, 0))
	{
		requestRedraw();
	}
}

void LDrawModelViewer::requestRedraw(void)
{
	TCAlertManager::sendAlert(redrawAlertClass(), this);
}

void LDrawModelViewer::setShowBoundingBox(bool value)
{
	if (value != flags.showBoundingBox)
	{
		flags.showBoundingBox = value;
		requestRedraw();
	}
}

void LDrawModelViewer::setBoundingBoxesOnly(bool value)
{
	if (value != flags.boundingBoxesOnly)
	{
		flags.boundingBoxesOnly = value;
		flags.needsReparse = true;
		flags.needsCalcSize = true;
	}
}

void LDrawModelViewer::applyZoom(void)
{
	if (flags.paused)
	{
		return;
	}
	if (clipZoom)
	{
		if (!fEq(clipAmount, nextClipAmount))
		{
			clipAmount = nextClipAmount;
			perspectiveView(false);
		}
	}
	else
	{
		TCFloat distance = (camera.getPosition()).length();

		if (!fEq(distance, nextDistance))
		{
			camera.move(TCVector(0.0f, 0.0f, nextDistance - distance));
//			distance = newDistance;
//			camera.move(TCVector(0.0f, 0.0f, amount * size / 300.0f));
			perspectiveView(false);
		}
	}
}

void LDrawModelViewer::clearBackground(void)
{
	GLfloat backgroundColor[3];

	if (cullBackFaces)
	{
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	}
	else
	{
		glDisable(GL_CULL_FACE);
	}
	glClearDepth(1.0);
	backgroundColor[0] = (GLfloat)backgroundR;
	backgroundColor[1] = (GLfloat)backgroundG;
	backgroundColor[2] = (GLfloat)backgroundB;
	glFogfv(GL_FOG_COLOR, backgroundColor);
	if (flags.slowClear && !getGl2ps())
	{
		GLint oldDepthFunc;
		bool oldBlendEnabled = false;
		bool oldPolygonOffsetEnabled = false;
		bool oldLightingEnabled = false;

		orthoView();
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glGetIntegerv(GL_DEPTH_FUNC, &oldDepthFunc);
		glDepthFunc(GL_ALWAYS);
		glColor4f(backgroundR, backgroundG, backgroundB, backgroundA);
		if (glIsEnabled(GL_BLEND))
		{
			glDisable(GL_BLEND);
			oldBlendEnabled = true;
		}
		if (glIsEnabled(GL_POLYGON_OFFSET_FILL))
		{
			disable(GL_POLYGON_OFFSET_FILL);
			oldPolygonOffsetEnabled = true;
		}
		if (glIsEnabled(GL_LIGHTING))
		{
			glDisable(GL_LIGHTING);
			oldLightingEnabled = true;
		}
		glDepthMask(1);
		glBegin(GL_QUADS);
		glVertex3i(-1, -1, -1);
		glVertex3i((GLint)scale(width) + 1, -1, -1);
		glVertex3i((GLint)scale(width) + 1, (GLint)scale(height) + 1, -1);
		glVertex3i(-1, (GLint)scale(height) + 1, -1);
		glEnd();
		glDepthFunc(oldDepthFunc);
		if (oldBlendEnabled)
		{
			glEnable(GL_BLEND);
		}
		if (oldPolygonOffsetEnabled)
		{
			enable(GL_POLYGON_OFFSET_FILL);
		}
		if (oldLightingEnabled)
		{
			glEnable(GL_LIGHTING);
		}
		perspectiveView();
	}
	else
	{
		glClearColor(backgroundR, backgroundG, backgroundB, backgroundA);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
			GL_STENCIL_BUFFER_BIT);
	}
	if (flags.drawWireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		lineWidth(getScaledWireframeLineWidth());
		if (flags.useWireframeFog)
		{
			glEnable(GL_FOG);
		}
		else
		{
			glDisable(GL_FOG);
		}
	}
	else
	{
		lineWidth(getScaledHighlightLineWidth());
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDisable(GL_FOG);
	}
}

void LDrawModelViewer::projectCamera(const TCVector &distance)
{
	TCFloat inverseMatrix[16];

	camera.getFacing().getInverseMatrix(inverseMatrix);
	TCVector camCenter = -distance.mult(inverseMatrix) - camera.getPosition();
	treGlMultMatrixf(inverseMatrix);
	treGlTranslatef(camCenter[0], camCenter[1], camCenter[2]);
}

void LDrawModelViewer::drawSetup(TCFloat eyeXOffset)
{
	glLoadIdentity();
	if (flags.qualityLighting)
	{
		glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);
	}
	else
	{
		glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 0);
	}
	if (flags.usesFlatShading)
	{
		glShadeModel(GL_FLAT);
	}
	else
	{
		glShadeModel(GL_SMOOTH);
	}
	drawLights();
	glLoadIdentity();
	if (mainTREModel)
	{
		applyZoom();
//		camera.move(cameraMotion * size / 100.0f);
		perspectiveView(false);
//		camera.rotate(TCVector(cameraXRotate, cameraYRotate, cameraZRotate));
		projectCamera(TCVector(-eyeXOffset - xPan, -yPan, 0.0f));
//		treGlTranslatef(eyeXOffset + xPan, yPan, -distance);
	}
	else
	{
		projectCamera(TCVector(-eyeXOffset - xPan, -yPan, 10.0f));
//		treGlTranslatef(eyeXOffset + xPan, yPan, -10.0f);
	}
}

void LDrawModelViewer::setExamineMode(ExamineMode value)
{
	examineMode = value;
}

void LDrawModelViewer::setViewMode(ViewMode value)
{
	viewMode = value;
}

void LDrawModelViewer::innerDrawModel(void)
{
	mainTREModel->draw();
	if (highlightModel != NULL)
	{
		highlightModel->draw();
	}
	drawAxes(true);
	if (flags.showBoundingBox)
	{
		drawBoundingBox();
	}
}

void LDrawModelViewer::drawToClipPlaneUsingStencil(TCFloat eyeXOffset)
{
	glPushAttrib(GL_POLYGON_BIT | GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT |
		GL_LINE_BIT | GL_STENCIL_BUFFER_BIT);
	perspectiveViewToClipPlane();
	glDisable(GL_DEPTH_TEST);
	glClear(GL_STENCIL_BUFFER_BIT);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS, 1, ~0u);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	lineWidth(getScaledCutawayLineWidth());
	glDisable(GL_FOG);
	glLoadIdentity();
	projectCamera(TCVector(-eyeXOffset - xPan, -yPan, 0.0f));
	if (viewMode == VMFlyThrough || viewMode == VMWalk || examineMode == EMFree)
	{
		if (rotationMatrix)
		{
			glPushMatrix();
			glLoadIdentity();
			treGlRotatef(rotationSpeed, xRotate, yRotate, zRotate);
			treGlMultMatrixf(rotationMatrix);
			treGlGetFloatv(GL_MODELVIEW_MATRIX, rotationMatrix);
			glPopMatrix();
			treGlMultMatrixf(rotationMatrix);
		}
	}
	else if (viewMode == VMExamine && examineMode == EMLatLong)
	{
		//examineLongitude += rotationSpeed;
		//examineLatitude += rotationSpeed;
		//examineLongitude = fmodf(examineLongitude, 360.0f);
		//examineLatitude = fmodf(examineLatitude, 360.0f);
		treGlRotatef(examineLongitude, 0.0f, 1.0f, 0.0f);
		treGlRotatef(examineLatitude, 1.0f, 0.0f, 0.0f);
	}
	if (flags.autoCenter)
	{
		treGlTranslatef(-center[0], -center[1], -center[2]);
	}
	showLight();
	innerDrawModel();
	glDisable(GL_LIGHTING);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 1, 0, 1, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	if ((backgroundR + backgroundG + backgroundB) / 3.0f > 0.5)
	{
		glColor4f(0.0f, 0.0f, 0.0f, cutawayAlpha);
	}
	else
	{
		glColor4f(1.0f, 1.0f, 1.0f, cutawayAlpha);
	}
	enable(GL_BLEND);
	blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glStencilFunc(GL_EQUAL, 1, ~0u);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glRectf(0.0, 0.0, 1.0, 1.0);
	perspectiveView();
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
	disable(GL_BLEND);
	glEnable(GL_LIGHTING);
	glPopAttrib();
}

/*
void LDrawModelViewer::drawToClipPlaneUsingAccum(GLfloat eyeXOffset)
{
	TCFloat weight = 0.25f;
	TCFloat oldZoomSpeed = zoomSpeed;

	glReadBuffer(GL_BACK);
	glAccum(GL_LOAD, 1.0f - weight);
	glClearColor(backgroundR, backgroundG, backgroundB, backgroundA);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	zoomSpeed = 0.0f;
	clearBackground();
	drawSetup();
	zoomSpeed = oldZoomSpeed;
	if (rotationMatrix)
	{
		glPushMatrix();
		glLoadIdentity();
		treGlMultMatrixf(rotationMatrix);
		treGlGetFloatv(GL_MODELVIEW_MATRIX, rotationMatrix);
		glPopMatrix();
		treGlMultMatrixf(rotationMatrix);
	}
	treGlTranslatef(-center[0], -center[1], -center[2]);
	glColor3ub(192, 192, 192);
	mainTREModel->draw();

	glClearDepth(1.0);
	glClear(GL_DEPTH_BUFFER_BIT);
	perspectiveViewToClipPlane();
	glClearDepth(1.0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	lineWidth(cutawayLineWidth);
	glDisable(GL_FOG);
	glLoadIdentity();
	treGlTranslatef(eyeXOffset + xPan, yPan, -distance);
	if (rotationMatrix)
	{
		glPushMatrix();
		glLoadIdentity();
		treGlMultMatrixf(rotationMatrix);
		treGlGetFloatv(GL_MODELVIEW_MATRIX, rotationMatrix);
		glPopMatrix();
		treGlMultMatrixf(rotationMatrix);
	}
	treGlTranslatef(-center[0], -center[1], -center[2]);
	mainTREModel->draw();
	perspectiveView();
	glAccum(GL_ACCUM, weight);
	glAccum(GL_RETURN, 1.0f);
}
*/

void LDrawModelViewer::drawToClipPlaneUsingDestinationAlpha(TCFloat eyeXOffset)
{
	TCFloat32 weight = cutawayAlpha;

	glPushAttrib(GL_POLYGON_BIT | GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT |
		GL_LINE_BIT);
	perspectiveViewToClipPlane();
	glClearDepth(1.0);
	glClearColor(weight, weight, weight, weight);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
	enable(GL_BLEND);
	blendFunc(GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	lineWidth(getScaledCutawayLineWidth());
	glDisable(GL_FOG);
	glLoadIdentity();
	projectCamera(TCVector(-eyeXOffset - xPan, -yPan, 0.0f));
//	treGlTranslatef(eyeXOffset + xPan, yPan, -distance);
	//treGlTranslatef(0.0f, 0.0f, -distance);
	if (rotationMatrix)
	{
		glPushMatrix();
		glLoadIdentity();
		treGlRotatef(rotationSpeed, xRotate, yRotate, zRotate);
		treGlMultMatrixf(rotationMatrix);
		treGlGetFloatv(GL_MODELVIEW_MATRIX, rotationMatrix);
		glPopMatrix();
		treGlMultMatrixf(rotationMatrix);
	}
	if (flags.autoCenter)
	{
		treGlTranslatef(-center[0], -center[1], -center[2]);
	}
	showLight();
	mainTREModel->setCutawayDrawFlag(true);
	innerDrawModel();
	mainTREModel->setCutawayDrawFlag(false);
	perspectiveView();
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	disable(GL_BLEND);
	glPopAttrib();
}

void LDrawModelViewer::drawToClipPlaneUsingNoEffect(TCFloat eyeXOffset)
{
	glPushAttrib(GL_POLYGON_BIT | GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT |
		GL_LINE_BIT);
	perspectiveViewToClipPlane();
	glClearDepth(1.0);
	glClear(GL_DEPTH_BUFFER_BIT);
//	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	lineWidth(getScaledCutawayLineWidth());
	glDisable(GL_FOG);
	glLoadIdentity();
	projectCamera(TCVector(-eyeXOffset - xPan, -yPan, 0.0f));
//	treGlTranslatef(eyeXOffset + xPan, yPan, -distance);
	if (rotationMatrix)
	{
		glPushMatrix();
		glLoadIdentity();
		treGlRotatef(rotationSpeed, xRotate, yRotate, zRotate);
		treGlMultMatrixf(rotationMatrix);
		treGlGetFloatv(GL_MODELVIEW_MATRIX, rotationMatrix);
		glPopMatrix();
		treGlMultMatrixf(rotationMatrix);
	}
	if (flags.autoCenter)
	{
		treGlTranslatef(-center[0], -center[1], -center[2]);
	}
	showLight();
	innerDrawModel();
	perspectiveView();
	glPopAttrib();
//	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

void LDrawModelViewer::drawToClipPlane(TCFloat eyeXOffset)
{
	switch (cutawayMode)
	{
	case LDVCutawayNormal:
		// Don't do anything
		break;
	case LDVCutawayWireframe:
		if (fEq(cutawayAlpha, 1.0f))
		{
			drawToClipPlaneUsingNoEffect(eyeXOffset);
		}
		else
		{
			drawToClipPlaneUsingDestinationAlpha(eyeXOffset);
		}
		break;
	case LDVCutawayStencil:
		drawToClipPlaneUsingStencil(eyeXOffset);
		break;
	}
//	drawToClipPlaneUsingAccum(eyeXOffset);
}

void LDrawModelViewer::clear(void)
{
	glClearStencil(0);
	glClearDepth(1.0);
	glClearColor(backgroundR, backgroundG, backgroundB, backgroundA);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

bool LDrawModelViewer::getLDrawCommandLineMatrix(char *matrixString,
												 int bufferLength)
{
	if (rotationMatrix && bufferLength)
	{
		char buf[1024];
		TCFloat matrix[16];
		int i;
		TCVector point;
		
		if (flags.autoCenter)
		{
			point = -center;
		}
		for (i = 0; i < 16; i++)
		{
			if (fEq(rotationMatrix[i], 0.0f))
			{
				matrix[i] = 0.0f;
			}
			else if (i == 0 || i  == 4 || i == 8)
			{
				matrix[i] = rotationMatrix[i];
			}
			else
			{
				matrix[i] = -rotationMatrix[i];
			}
		}
		point = point.transformPoint(matrix);
		sprintf(buf, "-a%.2g,%.2g,%.2g,%.2g,%.2g,%.2g,%.2g,%.2g,%.2g",
			matrix[0], matrix[4], matrix[8],
			matrix[1], matrix[5], matrix[9],
			matrix[2], matrix[6], matrix[10]);
		strncpy(matrixString, buf, bufferLength);
		matrixString[bufferLength - 1] = 0;
		return true;
	}
	else
	{
		return false;
	}
}

bool LDrawModelViewer::getLDGLiteCommandLine(char *commandString,
											 int bufferLength)
{
	if (rotationMatrix && bufferLength)
	{
		char buf[1024];
		char matrixString[512];
		TCVector cameraPoint = camera.getPosition();
//		TCVector cameraPoint = TCVector(0.0f, 0.0f, distance);
		TCVector lookAt;

		if (flags.autoCenter)
		{
			lookAt = center;
		}
		int i;
		TCFloat transformationMatrix[16];

		if (!getLDrawCommandLineMatrix(matrixString, 512))
		{
			return false;
		}
		TCVector::invertMatrix(rotationMatrix, transformationMatrix);
		lookAt = lookAt.transformPoint(rotationMatrix);
		cameraPoint += lookAt;
//		cameraPoint = TGLShape::transformPoint(cameraPoint, rotationMatrix);
		for (i = 0; i < 3; i++)
		{
			if (fEq(cameraPoint[i], 0.0f))
			{
				cameraPoint[i] = 0.0f;
			}
		}
		sprintf(buf, "ldglite -J -v%d,%d "
			"-cc%.4f,%.4f,%.4f -co%.4f,%.4f,%.4f "
			"-cu0,1,0 %s \"%s\"", (int)scale(width), (int)scale(height),
			cameraPoint[0], cameraPoint[1], cameraPoint[2],
			lookAt[0], lookAt[1], lookAt[2],
			matrixString, filename);
		strncpy(commandString, buf, bufferLength);
		commandString[bufferLength - 1] = 0;
		return true;
	}
	return false;
}

bool LDrawModelViewer::getLDrawCommandLine(char *shortFilename,
										   char *commandString,
										   int bufferLength)
{
	if (rotationMatrix && bufferLength)
	{
		char buf[1024];
		TCFloat matrix[16];
		int i;
		TCVector point;
		TCFloat lScaleFactor = 500.0f;
		TCFloat distance = (camera.getPosition()).length();
//		TCFloat distance = (camera.getPosition() - center).length();

		if (flags.autoCenter)
		{
			point = -center;
		}
		for (i = 0; i < 16; i++)
		{
			if (fEq(rotationMatrix[i], 0.0f))
			{
				matrix[i] = 0.0f;
			}
			else if (i == 0 || i  == 4 || i == 8)
			{
				matrix[i] = rotationMatrix[i];
			}
			else
			{
				matrix[i] = -rotationMatrix[i];
			}
		}
		point = point.transformPoint(matrix);
		sprintf(buf, "ldraw -s%.4g -o%d,%d "
			"-a%.4g,%.4g,%.4g,%.4g,%.4g,%.4g,%.4g,%.4g,%.4g "
			"%s",
			lScaleFactor / distance,
			(int)(point[0] * lScaleFactor / distance),
			(int)(point[1] * lScaleFactor / distance),
			matrix[0], matrix[4], matrix[8],
			matrix[1], matrix[5], matrix[9],
			matrix[2], matrix[6], matrix[10],
			shortFilename);
		strncpy(commandString, buf, bufferLength);
		commandString[bufferLength - 1] = 0;
		return true;
	}
	else
	{
		return false;
	}
}

// Note: static method
void LDrawModelViewer::fixLongitude(TCFloat &lon)
{
	lon = fmodf(lon, 360.0f);
	if (lon > 180.0f)
	{
		lon -= 360.0f;
	}
	if (lon <= -180.0f)
	{
		lon += 360.0f;
	}
}

void LDrawModelViewer::applyModelRotation(void)
{
	if (rotationMatrix)
	{
		if (!flags.paused)
		{
			if (viewMode == VMFlyThrough || viewMode == VMWalk || examineMode == EMFree)
			{
				TCFloat matrix[16];
				TCVector rotation = TCVector(xRotate, yRotate, zRotate);

				camera.getFacing().getInverseMatrix(matrix);
				glPushMatrix();
				glLoadIdentity();
				rotation = rotation.mult(matrix);
				treGlRotatef(rotationSpeed, rotation[0], rotation[1], rotation[2]);
				treGlMultMatrixf(rotationMatrix);
				treGlGetFloatv(GL_MODELVIEW_MATRIX, rotationMatrix);
				glPopMatrix();
			}
			else if (viewMode == VMExamine && examineMode == EMLatLong)
			{
				if (rotationSpeed != 0)
				{
					examineLongitude += rotationSpeed * -yRotate * 0.1f;
					fixLongitude(examineLongitude);
					examineLatitude += rotationSpeed * xRotate * 0.2f;
					if (examineLatitude > 90.0f)
					{
						examineLatitude = 90.0f;
					}
					if (examineLatitude < -90.0f)
					{
						examineLatitude = -90.0f;
					}
				}
				glPushMatrix();
				glLoadIdentity();
				treGlRotatef(examineLatitude + 180.0f, 1.0f, 0.0f, 0.0f);
				treGlRotatef(examineLongitude, 0.0f, 1.0f, 0.0f);
				treGlGetFloatv(GL_MODELVIEW_MATRIX, rotationMatrix);
				glPopMatrix();
			}
		}
	}
}

void LDrawModelViewer::update(void)
{
	static GLubyte stipplePattern[128];
	static bool stipplePatternSet = false;
	TCFloat eyeXOffset = 0.0f;

	flags.updating = true;
	if (flags.needsReload)
	{
		reload();
	}
	if (flags.needsReparse)
	{
		parseModel();
	}
	if (!stipplePatternSet)
	{
		int i;

		for (i = 0; i < 32; i++)
		{
			if (i % 2)
			{
				memset(stipplePattern + i * 4, 0xAA, 4);
			}
			else
			{
				memset(stipplePattern + i * 4, 0x55, 4);
			}
		}
		stipplePatternSet = true;
	}
	if (!rotationMatrix)
	{
		rotationMatrix = new TCFloat[16];
		setupDefaultViewAngle();
		flags.needsRotationMatrixSetup = true;
	}
	if (flags.needsSetup)
	{
		setup();
	}
	if (flags.needsTextureSetup)
	{
		setupTextures();
	}
	if (flags.needsLightingSetup)
	{
		setupLighting();
	}
	if (flags.needsMaterialSetup)
	{
		setupMaterial();
	}
	else if (flags.needsRecompile)
	{
		recompile();
	}
	if (flags.needsViewReset)
	{
		perspectiveView();
		resetView();
	}
	if (flags.needsResize)
	{
		perspectiveView();
	}
	glPolygonStipple(stipplePattern);
	if (flags.needsRotationMatrixSetup)
	{
		setupRotationMatrix();
	}
	if ((flags.keepRightSideUp && viewMode == VMFlyThrough) || viewMode == VMWalk)
	{
		rightSideUp(false);
	}
	clearBackground();
	if (!mainTREModel)
	{
		flags.updating = false;
		return;
	}
	mainTREModel->setSaveAlphaFlag(flags.saveAlpha);
	if (stereoMode == LDVStereoCrossEyed || stereoMode == LDVStereoParallel)
	{
		TCFloat distance = (camera.getPosition()).length();

		eyeXOffset = stereoEyeSpacing * 2.0f / (TCFloat)pow((double)distance,
			0.25);
		if (stereoMode == LDVStereoCrossEyed)
		{
			eyeXOffset = -eyeXOffset;
		}
	}
	if (rotationMatrix)
	{
		applyModelRotation();
	}
	updateCameraPosition();
	zoom(zoomSpeed, false);
	if (flags.drawWireframe && flags.removeHiddenLines)
	{
		removeHiddenLines();
		drawModel(0.0f);
	}
	else
	{
		drawModel(eyeXOffset);
	}
	drawAxes(false);
	if (stereoMode == LDVStereoCrossEyed || stereoMode == LDVStereoParallel)
	{
		eyeXOffset = -eyeXOffset;
		GLint eyeWidth = (GLint)(scale(width) / 2);
		glViewport(eyeWidth, 0, eyeWidth, (GLsizei)scale(height));
		if (flags.slowClear)
		{
			clearBackground();
			glViewport(eyeWidth, 0, eyeWidth, (GLsizei)scale(height));
		}
		if (flags.drawWireframe && flags.removeHiddenLines)
		{
			removeHiddenLines();
			drawModel(0.0f);
		}
		else
		{
			drawModel(eyeXOffset);
		}
		drawAxes(false);
		glViewport(0, 0, eyeWidth, (GLsizei)scale(height));
	}
	flags.updating = false;
	if ((!fEq(rotationSpeed, 0.0f) ||
		!fEq(zoomSpeed, 0.0f) ||
		!fEq(cameraXRotate, 0.0f) ||
		!fEq(cameraYRotate, 0.0f) ||
		!fEq(cameraZRotate, 0.0f) ||
		!fEq(cameraMotion.length(), 0.0f))
		&& !getPaused())
	{
		requestRedraw();
		flags.animating = true;
	}
	else
	{
		flags.animating = false;
	}
	TCAlertManager::sendAlert(frameDoneAlertClass(), this);
	updateFrameTime(true);
}

void LDrawModelViewer::updateFrameTime(bool force /*=false*/)
{
	if (!flags.animating || force)
	{
#ifdef USE_STD_CHRONO
		frameTime = std::chrono::steady_clock::now();
#else
#ifdef WIN32
		if (hrpcFrequency.QuadPart != 0)
		{
			QueryPerformanceCounter(&hrpcFrameCount);
		}
		else
		{
			frameTicks = GetTickCount();
		}
#endif // WIN32
#ifdef _QT
		qtime.restart();
#endif
#ifdef COCOA
		[FRAME_TIME release];
		FRAME_TIME = [[NSDate alloc] init];
#endif // COCOA
#endif // !USE_STD_CHRONO
	}
}

void LDrawModelViewer::removeHiddenLines(TCFloat eyeXOffset)
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	mainTREModel->setLightingFlag(false);
	mainTREModel->setRemovingHiddenLines(true);
	if (flags.usePolygonOffset)
	{
		glPolygonOffset(POLYGON_OFFSET_FACTOR, POLYGON_OFFSET_UNITS);
		enable(GL_POLYGON_OFFSET_FILL);
	}
	drawModel(eyeXOffset);
	// Not sure why the following is necessary.
	lineWidth(getScaledWireframeLineWidth());
	if (flags.usePolygonOffset)
	{
		glPolygonOffset(0.0f, 0.0f);
		enable(GL_POLYGON_OFFSET_FILL);
	}
	mainTREModel->setRemovingHiddenLines(false);
	mainTREModel->setLightingFlag(flags.useLighting);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

void LDrawModelViewer::setupRotationMatrix(void)
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	treGlRotatef(180.0f, 0.0f, 0.0f, 1.0f);
	treGlRotatef(180.0f, 0.0f, 1.0f, 0.0f);
	treGlMultMatrixf(rotationMatrix);
	treGlGetFloatv(GL_MODELVIEW_MATRIX, rotationMatrix);
	glPopMatrix();
	flags.needsRotationMatrixSetup = false;
/*
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	treGlGetFloatv(GL_MODELVIEW_MATRIX, rotationMatrix);
	treGlRotatef(180.0f, 0.0f, 0.0f, 1.0f);
	treGlRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
	treGlRotatef(50.0f, 0.0f, -1.5f, 1.0f);
	treGlGetFloatv(GL_MODELVIEW_MATRIX, rotationMatrix);
	glPopMatrix();
*/
}

void LDrawModelViewer::initLightDirModel(
	TREMainModel *&lightDirModel,
	TCULong color)
{
	TREModel *subModel = new TREModel;
	lightDirModel = new TREMainModel;
	lightDirModel->setSendProgressFlag(false);
	TCFloat identityMatrix[16];
	float length = size / 1.5f;
	float radius = size / 100.0f;
	int segments = 32;
	float coneLength = radius * 4.0f;
	float coneRadius = radius * 2.0f;
	float offset = length / 3.0f;

	lightDirModel->setLightingFlag(true);
	TCVector::initIdentityMatrix(identityMatrix);
	subModel->setMainModel(lightDirModel);
	subModel->addCylinder(TCVector(0.0f, coneLength + offset, 0.0f), radius,
		length - coneLength - offset, segments);
	subModel->addDisc(TCVector(0.0, coneLength + offset, 0.0), coneRadius,
		segments);
	subModel->addDisc(TCVector(0.0, length, 0.0), radius, segments);
	subModel->addCone(TCVector(0.0, coneLength + offset, 0.0), coneRadius,
		-coneLength, segments);
	lightDirModel->addSubModel(color, color, identityMatrix, subModel, false);
	subModel->release();
	lightDirModel->postProcess();
}

void LDrawModelViewer::initLightDirModels(void)
{
	if (size > 0.0 && !flags.noUI)
	{
		if (whiteLightDirModel == NULL)
		{
			initLightDirModel(whiteLightDirModel,
				LDLPalette::colorForRGBA(255, 255, 255, 255));
		}
		if (blueLightDirModel == NULL)
		{
			initLightDirModel(blueLightDirModel,
				LDLPalette::colorForRGBA(128, 128, 255, 255));
		}
	}
}

TREMainModel *LDrawModelViewer::getContrastingLightDirModel()
{
	if ((backgroundR + backgroundG + backgroundB) / 3.0f > 0.9f)
	{
		return blueLightDirModel;
	}
	else
	{
		return whiteLightDirModel;
	}
}

void LDrawModelViewer::showLight(void)
{
	if (flags.showLight)
	{
		TCVector oldLightVector = lightVector;
		TCFloat rotInverse[16];
		LDLFacing facing;
		bool oldSpecular = flags.usesSpecular;
		bool oldSubdued = flags.subduedLighting;

		TCVector::invertMatrix(rotationMatrix, rotInverse);
		flags.usesSpecular = false;
		flags.subduedLighting = false;
		glPushMatrix();
		glPushAttrib(GL_POLYGON_BIT | GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT |
			GL_LINE_BIT | GL_LIGHTING_BIT);
		facing.setFacing(TCVector(1.0f, 0.0f, 0.0f), (TCFloat)M_PI / 2.0f);
		facing.pointAt(oldLightVector);
		treGlTranslatef(center[0], center[1], center[2]);
		treGlMultMatrixf(rotInverse);
		treGlMultMatrixf(camera.getFacing().getMatrix());
		lightVector = TCVector(0.0f, 0.0f, 1.0f);
		setupLight(GL_LIGHT0);
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		glDisable(GL_LIGHT1);
		glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
		glPushMatrix();
		glLoadIdentity();
		drawLights();
		glPopMatrix();
		lightVector = oldLightVector;
		treGlMultMatrixf(facing.getMatrix());
		getContrastingLightDirModel()->draw();
		glPopAttrib();
		glPopMatrix();
		flags.usesSpecular = oldSpecular;
		flags.subduedLighting = oldSubdued;
	}
}

void LDrawModelViewer::drawLightDats(void)
{
	if (haveLightDats())
	{
		const TCVectorList &lightLocs = mainTREModel->getLightLocations();
		GLint maxLights;

		glGetIntegerv(GL_MAX_LIGHTS, &maxLights);
		if (lightLocs.size() > 0)
		{
			int i;
			TCVectorList::const_iterator itLoc = lightLocs.begin();
			GLfloat position[4];
			int start = 0;

			if (!flags.optionalStandardLight)
			{
				start = 1;
			}
			// Point lights are distinguished from directional lights by the w
			// component of their position vector.  Setting it to 0 yields a
			// directional light.  Setting it to non-zero yields a point light.
			position[3] = 1.0f;
			for (i = start; i < maxLights && itLoc != lightLocs.end(); i++)
			{
				const TCVector &lightLoc = *itLoc;

				position[0] = (GLfloat)lightLoc.get(0);
				position[1] = (GLfloat)lightLoc.get(1);
				position[2] = (GLfloat)lightLoc.get(2);
				glLightfv(GL_LIGHT0 + i, GL_POSITION, position);
				itLoc++;
			}
		}
	}
}

void LDrawModelViewer::drawAxes(bool atOrigin)
{
	if (flags.showAxes && flags.axesAtOrigin == atOrigin)
	{
		if (flags.axesAtOrigin)
		{
			TCFloat margin = size / 8.0f;
			TCVector minPoint(boundingMin);
			TCVector maxPoint(boundingMax);
			//TCVector margin = (boundingMax - boundingMin) / 8.0f;
			int i;

			for (i = 0; i < 3; i++)
			{
				if (boundingMin[i] > 0.0f)
				{
					minPoint[i] = -margin;
				}
				else
				{
					minPoint[i] = boundingMin[i] - margin;
				}
				if (boundingMax[i] < 0.0f)
				{
					maxPoint[i] = margin;
				}
				else
				{
					maxPoint[i] = boundingMax[i] + margin;
				}
			}
			glPushAttrib(GL_LIGHTING_BIT | GL_LINE_BIT);
			lineWidth(2.0f);
			glDisable(GL_LIGHTING);
			glBegin(GL_LINES);
				glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
				glVertex3f(0.0f, 0.0f, 0.0f);
				glVertex3f(maxPoint[0], 0.0f, 0.0f);
				glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
				glVertex3f(0.0f, 0.0f, 0.0f);
				glVertex3f(0.0f, maxPoint[1], 0.0f);
				glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
				glVertex3f(0.0f, 0.0f, 0.0f);
				glVertex3f(0.0f, 0.0f, maxPoint[2]);

				glColor4f(0.5f, 0.0f, 0.0f, 1.0f);
				glVertex3f(0.0f, 0.0f, 0.0f);
				glVertex3f(minPoint[0], 0.0f, 0.0f);
				glColor4f(0.0f, 0.5f, 0.0f, 1.0f);
				glVertex3f(0.0f, 0.0f, 0.0f);
				glVertex3f(0.0f, minPoint[1], 0.0f);
				glColor4f(0.0f, 0.0f, 0.5f, 1.0f);
				glVertex3f(0.0f, 0.0f, 0.0f);
				glVertex3f(0.0f, 0.0f, minPoint[2]);
				glEnd();
			glPopAttrib();
		}
		else
		{
			int actualWidth = (int)scale(width);

			if (stereoMode == LDVStereoCrossEyed || stereoMode == LDVStereoParallel)
			{
				actualWidth = (int)(scale(width) / 2);
			}
			glPushAttrib(GL_LIGHTING_BIT | GL_VIEWPORT_BIT);
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glOrtho(0.0f, actualWidth, 0.0f, (int)scale(height), -25.0f, 25.0f);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glTranslatef(30.0f, 30.0f, 0.0f);
			if (rotationMatrix)
			{
				treGlMultMatrixf(rotationMatrix);
			}
			glDisable(GL_LIGHTING);
			glBegin(GL_LINES);
				glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
				glVertex3f(0.0f, 0.0f, 0.0f);
				glVertex3f(25.0f, 0.0f, 0.0f);
				glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
				glVertex3f(0.0f, 0.0f, 0.0f);
				glVertex3f(0.0f, 25.0f, 0.0f);
				glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
				glVertex3f(0.0f, 0.0f, 0.0f);
				glVertex3f(0.0f, 0.0f, 25.0f);
			glEnd();
			glPopAttrib();
		}
	}
}

void LDrawModelViewer::drawModel(TCFloat eyeXOffset)
{
	drawSetup(eyeXOffset);
	if (rotationMatrix)
	{
		treGlMultMatrixf(rotationMatrix);
	}
	if (flags.autoCenter)
	{
		treGlTranslatef(-center[0], -center[1], -center[2]);
	}
	drawLightDats();
//	drawBoundingBox();
	glColor3ub(192, 192, 192);
	if (mainTREModel)
	{
		showLight();
		innerDrawModel();
		if (clipAmount > 0.01)
		{
			drawToClipPlane(eyeXOffset);
		}
	}
}

void LDrawModelViewer::pause(void)
{
	flags.paused = true;
}

void LDrawModelViewer::unpause(void)
{
	flags.paused = false;
}

void LDrawModelViewer::setRotationSpeed(TCFloat value)
{
	rotationSpeed = value;
	if (value)
	{
		flags.paused = false;
	}
}

void LDrawModelViewer::setZoomSpeed(TCFloat value)
{
	zoomSpeed = value;
	if (value)
	{
		flags.paused = false;
	}
}

void LDrawModelViewer::setExtraSearchDirs(TCStringArray *value)
{
	if (extraSearchDirs != value)
	{
		TCObject::release(extraSearchDirs);
		extraSearchDirs = value;
		TCObject::retain(extraSearchDirs);
	}
	// Since it is a string array, the contents might have changed, even if
	// the array pointer itself didn't.
	flags.needsReload = true;
}

void LDrawModelViewer::panXY(int xValue, int yValue)
{
	TCFloat adjustment;
	TCFloat distance = (camera.getPosition()).length();
//	TCFloat distance = (camera.getPosition() - center).length();

	if (width > height)
	{
		adjustment = scale(width);
	}
	else
	{
		adjustment = scale(height);
	}
//	xPan += xValue / (TCFloat)pow(distance, 0.001);
//	yPan -= yValue / (TCFloat)pow(distance, 0.001);
	xPan += xValue / adjustment / (TCFloat)pow(2.0 / distance, 1.1) *
		(TCFloat)(sin(deg2rad(fov)) / sin(deg2rad(45.0)));
	yPan -= yValue / adjustment / (TCFloat)pow(2.0 / distance, 1.1) *
		(TCFloat)(sin(deg2rad(fov)) / sin(deg2rad(45.0)));
}

void LDrawModelViewer::setXYPan(TCFloat xValue, TCFloat yValue)
{
	xPan = xValue;
	yPan = yValue;
}

void LDrawModelViewer::openGlWillEnd(void)
{
	if (mainTREModel)
	{
		mainTREModel->openGlWillEnd();
	}
}

bool LDrawModelViewer::getCompiled(void) const
{
	if (mainTREModel)
	{
		return mainTREModel->getCompiled();
	}
	else
	{
		return false;
	}
}

//bool LDrawModelViewer::connectionFailure(TCWebClient *webClient)
//{
//	// I'm not sure if we should add more errors here or not.  Hopefully this is
//	// enough.  In reality, the hostname lookup will fail if the user doesn't
//	// have an internet connection.  If they have a mis-configured proxy, they
//	// should either get TCNCE_CONNECT or TCNCE_CONNECTION_REFUSED.
//	switch (webClient->getErrorNumber())
//	{
//	case TCNCE_HOSTNAME_LOOKUP:
//	case TCNCE_NO_PORT:
//	case TCNCE_CONNECT:
//	case TCNCE_CONNECTION_REFUSED:
//		return true;
//	default:
//		return false;
//	}
//}

//void LDrawModelViewer::findFileAlertCallback(LDLFindFileAlert *alert)
//{
//	char *lfilename = copyString(alert->getFilename());
//	size_t len = strlen(lfilename);
//	char *url;
//	char *partOutputFilename = copyString(LDLModel::lDrawDir(), len + 32);
//	char *primitiveOutputFilename = copyString(LDLModel::lDrawDir(), len + 32);
//	bool primitive = false;
//	bool part = false;
//	//const char *partUrlBase = "http://media.peeron.com/tmp/";
//	const char *partUrlBase = "http://www.ldraw.org/library/unofficial/parts/";
//	const char *primitiveUrlBase = "http://www.ldraw.org/library/unofficial/p/";
//	bool found = false;
//	char *key = new char[strlen(lfilename) + 128];

//	replaceStringCharacter(partOutputFilename, '\\', '/');
//	replaceStringCharacter(primitiveOutputFilename, '\\', '/');
//	strcat(partOutputFilename, "/Unofficial/parts/");
//	strcat(primitiveOutputFilename, "/Unofficial/p/");
//	convertStringToLower(lfilename);
//	replaceStringCharacter(lfilename, '\\', '/');
//	if (stringHasPrefix(lfilename, "48/"))
//	{
//		primitive = true;
//		url = copyString(primitiveUrlBase, len + 2);
//	}
//	else
//	{
//		if (stringHasPrefix(lfilename, "s/"))
//		{
//			// The only thing this is used for is to prevent it from checking
//			// for the file as a primitive if it's not found as a part.
//			part = true;
//		}
//		url = copyString(partUrlBase, len + 2);
//	}
//	strcat(partOutputFilename, lfilename);
//	strcat(primitiveOutputFilename, lfilename);
//	if (fileExists(partOutputFilename))
//	{
//		primitive = false;
//		found = true;
//		alert->setPartFlag(true);
//	}
//	else if (!part && fileExists(primitiveOutputFilename))
//	{
//		primitive = true;
//		found = true;
//		delete[] url;
//		url = copyString(primitiveUrlBase, len + 2);
//	}
//	if (canCheckForUnofficialPart(lfilename, found))
//	{
//		TCWebClient *webClient;
//		// FIX: dynamically allocate and use local string AND handle abort
//		UCCHAR message[1024];
//		bool abort;
//		UCSTR ucFilename = mbstoucstring(lfilename);

//		sprintf(key, "UnofficialPartChecks/%s/LastModified", lfilename);
//		if (found)
//		{
//			sucprintf(message, COUNT_OF(message), ls(_UC("CheckingForUpdates")),
//				ucFilename);
//		}
//		else
//		{
//			sucprintf(message, COUNT_OF(message), ls(_UC("TryingToDownload")),
//				ucFilename);
//		}
//		delete[] ucFilename;
//		TCProgressAlert::send("LDrawModelViewer", message, -1.0f, &abort, this);
//		strcat(url, lfilename);
//		webClient = new TCWebClient(url);
//		if (found)
//		{
//			char *lastModified = TCUserDefaults::stringForKey(key, NULL, false);

//			if (lastModified)
//			{
//				webClient->setLastModifiedString(lastModified);
//				delete[] lastModified;
//			}
//		}
//		if (primitive)
//		{
//			*strrchr(primitiveOutputFilename, '/') = 0;
//			webClient->setOutputDirectory(primitiveOutputFilename);
//			primitiveOutputFilename[strlen(primitiveOutputFilename)] = '/';
//		}
//		else
//		{
//			*strrchr(partOutputFilename, '/') = 0;
//			webClient->setOutputDirectory(partOutputFilename);
//			partOutputFilename[strlen(partOutputFilename)] = '/';
//		}
//		if (webClient->fetchURL() ||
//			webClient->getErrorNumber() == WCE_NOT_MODIFIED)
//		{
//			found = true;
//			if (!primitive)
//			{
//				alert->setPartFlag(true);
//			}
//		}
//		else if (connectionFailure(webClient))
//		{
//			// If we had a connection failure, we probably don't have an
//			// internet connection, or our proxy is mis-configured.  Don't try
//			// to connect again for now, and let the user know that auto part
//			// updates have been disabled.

//			preferences->setCheckPartTracker(false, true);
//			flags.checkPartTracker = false;
//			TCAlertManager::sendAlert(alertClass(), this,
//				ls(_UC("PartCheckDisabled")));
//		}
//		else
//		{
//			if (!primitive && !part)
//			{
//				// We don't know if it's a primitive or a part.  The part
//				// download failed, so try as a primitive.
//				delete[] url;
//				url = copyString(primitiveUrlBase, len + 2);
//				strcat(url, lfilename);
//				webClient->release();
//				webClient = new TCWebClient(url);
//				*strrchr(primitiveOutputFilename, '/') = 0;
//				webClient->setOutputDirectory(primitiveOutputFilename);
//				primitiveOutputFilename[strlen(primitiveOutputFilename)] = '/';
//				if (webClient->fetchURL() ||
//					webClient->getErrorNumber() == WCE_NOT_MODIFIED)
//				{
//					primitive = true;
//					found = true;
//				}
//			}
//		}
//		if (webClient->getLastModifiedString())
//		{
//			TCUserDefaults::setStringForKey(
//				webClient->getLastModifiedString(), key, false);
//		}
//		webClient->release();
//		sprintf(key, "UnofficialPartChecks/%s/LastUpdateCheckTime",
//			lfilename);
//		TCUserDefaults::setLongForKey((long)time(NULL), key, false);
//		if (!found)
//		{
//			unofficialPartNotFound(lfilename);
//		}
//	}
//	if (found)
//	{
//		alert->setFileFound(true);
//		if (primitive)
//		{
//			alert->setFilename(primitiveOutputFilename);
//		}
//		else
//		{
//			alert->setFilename(partOutputFilename);
//		}
//		setUnofficialPartPrimitive(lfilename, primitive);
//	}
//	delete[] key;
//	delete[] lfilename;
//	delete[] url;
//	delete[] partOutputFilename;
//	delete[] primitiveOutputFilename;
//}

LDPartsList *LDrawModelViewer::getPartsList(void)
{
	if (mainModel)
	{
		LDPartsList *partsList = new LDPartsList;

		partsList->scanModel(getCurModel(), defaultColorNumber);
		return partsList;
	}
	else
	{
		return NULL;
	}
}

// NOTE: static function
bool LDrawModelViewer::fileExists(const char* filename)
{
	FILE* file = ucfopen(filename, "r");

	if (file)
	{
		fclose(file);
		return true;
	}
	else
	{
		return false;
	}
}

// NOTE: static function
void LDrawModelViewer::setUnofficialPartPrimitive(const char *filename,
												  bool primitive)
{
	char *key = new char[strlen(filename) + 128];

	sprintf(key, "UnofficialPartChecks/%s/Primitive", filename);
	TCUserDefaults::setLongForKey(primitive ? 1 : 0, key, false);
	delete[] key;
}

LDViewPoint *LDrawModelViewer::saveViewPoint(void) const
{
	LDViewPoint *viewPoint = new LDViewPoint;

	viewPoint->setCamera(camera);
	viewPoint->setRotation(TCVector(xRotate, yRotate, zRotate));
	viewPoint->setCameraRotation(TCVector(cameraXRotate, cameraYRotate,
		cameraZRotate));
	viewPoint->setPan(TCVector(xPan, yPan, 0.0f));
	viewPoint->setRotationMatrix(rotationMatrix);
	viewPoint->setRotationSpeed(rotationSpeed);
	viewPoint->setAutoCenter(flags.autoCenter != false);
	viewPoint->setBackgroundColor(backgroundR, backgroundG, backgroundB,
		backgroundA);
	viewPoint->setStereoMode(stereoMode);
	viewPoint->setClipAmount(clipAmount);
	viewPoint->setDefaultDistance(defaultDistance);
	return viewPoint;
}

void LDrawModelViewer::rightSideUp(bool shouldRequestRedraw /*= true*/)
{
	if (!flags.needsSetup)
	{
		TCVector upVector(0.0, -1.0, 0.0);
		TCFloat matrix[16];
		TCFloat inverseMatrix[16];
		TCVector::invertMatrix(camera.getFacing().getMatrix(), inverseMatrix);
		TCVector::multMatrix(inverseMatrix, rotationMatrix, matrix);
		TCVector tempVector = upVector.transformNormal(matrix);
		float lzRotate = 0.0;

		if (tempVector[0] == 0.0 && tempVector[1] == 0.0)
		{
			lzRotate = 0;
		}
		else
		{
			lzRotate = atan2(tempVector[0], tempVector[1]);
		}
		camera.rotate(TCVector(0.0f, 0.0f, lzRotate));
		if (shouldRequestRedraw)
		{
			requestRedraw();
		}
	}
}

void LDrawModelViewer::restoreViewPoint(const LDViewPoint *viewPoint)
{
	TCVector tempVector;

	camera = viewPoint->getCamera();
	tempVector = viewPoint->getRotation();
	xRotate = tempVector[0];
	yRotate = tempVector[1];
	zRotate = tempVector[2];
	tempVector = viewPoint->getCameraRotation();
	cameraXRotate = tempVector[0];
	cameraYRotate = tempVector[1];
	cameraZRotate = tempVector[2];
	tempVector = viewPoint->getPan();
	xPan = tempVector[0];
	yPan = tempVector[1];
	memcpy(rotationMatrix, viewPoint->getRotationMatrix(),
		16 * sizeof(rotationMatrix[0]));
	rotationSpeed = viewPoint->getRotationSpeed();
	flags.autoCenter = viewPoint->getAutoCenter();
	viewPoint->getBackgroundColor(backgroundR, backgroundG, backgroundB,
		backgroundA);
	stereoMode = viewPoint->getStereoMode();
	clipAmount = viewPoint->getClipAmount();
	defaultDistance = viewPoint->getDefaultDistance();
}

bool LDrawModelViewer::canCheckForUnofficialPart(const char *lfilename,
												 bool exists)
{
	bool retValue = false;

	if (flags.checkPartTracker)
	{
		char *key = new char[strlen(lfilename) + 128];
		time_t lastCheck;
		time_t now = time(NULL);
		int days;

		if (exists)
		{
			sprintf(key, "UnofficialPartChecks/%s/LastUpdateCheckTime",
				lfilename);
			days = updatedPartWait;
		}
		else
		{
			sprintf(key, "UnofficialPartChecks/%s/LastCheckTime", lfilename);
			days = missingPartWait;
		}
		lastCheck = (time_t)TCUserDefaults::longForKey(key, 0, false);
		if (days < 1)
		{
			days = 1;
		}
		if (now - lastCheck > 24 * 3600 * days)
		{
			retValue = true;
		}
		delete[] key;
	}
	return retValue;
}

void LDrawModelViewer::unofficialPartNotFound(const char *lfilename)
{
	if (flags.checkPartTracker)
	{
		char *key = new char[strlen(lfilename) + 128];
		time_t now = time(NULL);

		sprintf(key, "UnofficialPartChecks/%s/LastCheckTime", lfilename);
		TCUserDefaults::setLongForKey((long)now, key, false);
		delete[] key;
	}
}

// NOTE: static function
void LDrawModelViewer::cleanupFloats(TCFloat *array, int count)
{
	int i;

	for (i = 0; i < count; i++)
	{
		if (fabs(array[i]) < 1e-6)
		{
			array[i] = 0.0f;
		}
	}
}

// Note: static method
bool LDrawModelViewer::checkAspect(
	TCFloat width,
	TCFloat height,
	int aspectW,
	int aspectH)
{
	return fabs(width * aspectH / aspectW - height) < 1e-5;
}

// Note: static method
ucstring LDrawModelViewer::getAspectString(
	TCFloat width,
	TCFloat height,
	CUCSTR separator,
	bool standardOnly /*= false*/)
{
	ucstring aspect;
	ucstring denom;

	if (checkAspect(width, height, 235, 100))
	{
		aspect = _UC("2.35");
		denom = _UC("1");
	}
	else if (checkAspect(width, height, 16, 9))
	{
		aspect = _UC("16");
		denom = _UC("9");
	}
	else if (checkAspect(width, height, 5, 3))
	{
		aspect = _UC("5");
		denom = _UC("3");
	}
	else if (checkAspect(width, height, 16, 10))
	{
		aspect = _UC("16");
		denom = _UC("10");
	}
	else if (checkAspect(width, height, 3, 2))
	{
		aspect = _UC("3");
		denom = _UC("2");
	}
	else if (checkAspect(width, height, 4, 3))
	{
		aspect = _UC("4");
		denom = _UC("3");
	}
	else if (checkAspect(width, height, 5, 4))
	{
		aspect = _UC("5");
		denom = _UC("4");
	}
	else if (standardOnly)
	{
		return _UC("");
	}
	else
	{
		aspect = ftoucstr(width / height);
	}
	if (denom.length() > 0)
	{
		aspect += separator;
		aspect += denom;
	}
	return aspect;
}

void LDrawModelViewer::getPovCameraInfo(UCCHAR *&userMessage, char *&povCamera)
{
	TCFloat tmpMatrix[16];
	TCFloat matrix[16];
	TCFloat lrotationMatrix[16];
	TCFloat centerMatrix[16];
	TCFloat positionMatrix[16];
	TCFloat cameraMatrix[16];
	TCFloat otherMatrix[16] = {1,0,0,0,0,-1,0,0,0,0,-1,0,0,0,0,1};
	UCCHAR locationString[1024];
	UCCHAR lookAtString[1204];
	UCCHAR upString[1024];
	UCCHAR message[4096];
	TCVector directionVector = TCVector(0.0f, 0.0f, 1.0f);
	TCVector locationVector;
	TCVector lookAtVector;
	TCVector upVector = TCVector(0.0f, -1.0f, 0.0f);
	double direction[3];
	double up[3];
	double location[3];
	LDLFacing facing;
	UCCHAR cameraString[4096];
	double lookAt[3];
	double tempV[3];
	ucstring povAspect;

	if (!mainTREModel)
	{
		userMessage = NULL;
		povCamera = NULL;
		return;
	}
	TCVector cameraPosition = camera.getPosition();

	memcpy(lrotationMatrix, getRotationMatrix(), sizeof(lrotationMatrix));
	TCVector::initIdentityMatrix(positionMatrix);
	positionMatrix[12] = cameraPosition[0] - getXPan();
	positionMatrix[13] = -cameraPosition[1] + getYPan();
	positionMatrix[14] = -cameraPosition[2];
	TCVector::initIdentityMatrix(centerMatrix);
	if (getAutoCenter())
	{
		centerMatrix[12] = center[0];
		centerMatrix[13] = center[1];
		centerMatrix[14] = center[2];
	}
	TCVector::multMatrix(otherMatrix, lrotationMatrix, tmpMatrix);
	TCVector::invertMatrix(tmpMatrix, cameraMatrix);
	TCVector::multMatrix(centerMatrix, cameraMatrix, tmpMatrix);
	TCVector::multMatrix(tmpMatrix, positionMatrix, matrix);

	facing = camera.getFacing();
	facing[0] = -facing[0];
	facing.getInverseMatrix(cameraMatrix);
	TCVector::multMatrix(matrix, cameraMatrix, tmpMatrix);
	memcpy(matrix, tmpMatrix, sizeof(matrix));
	cleanupFloats(matrix);
	locationVector = TCVector(matrix[12], matrix[13], matrix[14]);
	location[0] = (double)matrix[12];
	location[1] = (double)matrix[13];
	location[2] = (double)matrix[14];
	cleanupFloats(matrix);
	// Note that the location accuracy isn't nearly as important as the
	// directional accuracy, so we don't have to re-do this string prior
	// to putting it on the clipboard in the POV code copy.
	sucprintf(locationString, COUNT_OF(locationString), _UC("%s,%s,%s"),
		ftoucstr(location[0]).c_str(), ftoucstr(location[1]).c_str(),
		ftoucstr(location[2]).c_str());

	matrix[12] = matrix[13] = matrix[14] = 0.0f;
	directionVector = directionVector.transformPoint(matrix);
	upVector = upVector.transformPoint(matrix);
	// Grab the values prior to normalization.  That will make the
	// normalization more accurate in double precision.
	directionVector.upConvert(direction);
	lookAtVector = locationVector + directionVector *
		locationVector.length();
	upVector.upConvert(up);
	directionVector = directionVector.normalize();
	upVector = upVector.normalize();
	cleanupFloats(directionVector, 3);
	cleanupFloats(upVector, 3);
	// The following 3 strings will get re-done later at higher accuracy
	// for POV-Ray.
	sucprintf(lookAtString, COUNT_OF(lookAtString), _UC("%s"),
		lookAtVector.ucstring().c_str());
	sucprintf(upString, COUNT_OF(upString), _UC("%s"),
		upVector.ucstring().c_str());
	sucprintf(message, COUNT_OF(message), ls(_UC("PovCameraMessage")),
		locationString, lookAtString, upString);
	TCVector::doubleNormalize(up);
	TCVector::doubleNormalize(direction);
	TCVector::doubleMultiply(direction, tempV,
		TCVector::doubleLength(location));
	TCVector::doubleAdd(location, tempV, lookAt);
	// Re-do the strings with higher accuracy, so they'll be
	// accepted by POV-Ray.
	sucprintf(upString, COUNT_OF(upString), _UC("%s,%s,%s"),
		ftoucstr(up[0], 20).c_str(), ftoucstr(up[1], 20).c_str(),
		ftoucstr(up[2], 20).c_str());
	sucprintf(lookAtString, COUNT_OF(lookAtString), _UC("%s,%s,%s"),
		ftoucstr(lookAt[0], 20).c_str(), ftoucstr(lookAt[1], 20).c_str(),
		ftoucstr(lookAt[2], 20).c_str());
	if (flags.povCameraAspect)
	{
		povAspect = getAspectString(width, height, _UC("/"));
	}
	else
	{
		povAspect = _UC("4/3");
	}
	sucprintf(cameraString, COUNT_OF(cameraString),
		_UC("camera\n")
		_UC("{\n")
		_UC("\t#declare ASPECT = %s;\n")
		_UC("\tlocation < %s >\n")
		_UC("\tsky < %s >\n")
		_UC("\tright ASPECT * < -1,0,0 >\n")
		_UC("\tlook_at < %s >\n")
		_UC("\tangle %g\n")
		_UC("}\n"),
		povAspect.c_str(), locationString, upString, lookAtString, getHFov());
	userMessage = copyString(message);
	povCamera = ucstringtombs(cameraString);
}

bool LDrawModelViewer::mouseDown(LDVMouseMode mode, int x, int y)
{
	if ((mouseMode != LDVMouseNone && mouseMode != mode) ||
		mode == LDVMouseNone)
	{
		return false;
	}
	if (mode != LDVMouseLight)
	{
		return false;
	}
	if (mode == LDVMouseLight && !flags.useLighting)
	{
		// Allowing this will likely just lead to confusion.
		return false;
	}
	lastMouseX = x;
	lastMouseY = y;
	mouseMode = mode;
	switch (mouseMode)
	{
	case LDVMouseLight:
		if (haveStandardLight())
		{
			flags.showLight = true;
			requestRedraw();
		}
		break;
	default:
		break;
	}
	return true;
}

void LDrawModelViewer::setShowLightDir(bool value)
{
	if (value != flags.showLight)
	{
		flags.showLight = value;
		if (!value && preferences != NULL)
		{
			preferences->setLightVector(lightVector, true);
		}
	}
}

bool LDrawModelViewer::mouseUp(int x, int y)
{
	int deltaX = x - lastMouseX;
	int deltaY = y - lastMouseY;

	if (mouseMode != LDVMouseLight)
	{
		debugPrintf("LDVMouseLight is the only mode currently supported.\n");
		return false;
	}
	lastMouseX = x;
	lastMouseY = y;
	switch (mouseMode)
	{
	case LDVMouseLight:
		if (haveStandardLight())
		{
			mouseMoveLight(deltaX, deltaY);
			preferences->setLightVector(lightVector, true);
			flags.showLight = false;
			requestRedraw();
		}
		break;
	default:
		break;
	}
	mouseMode = LDVMouseNone;
	return true;
}

bool LDrawModelViewer::mouseMove(int x, int y)
{
	int deltaX = x - lastMouseX;
	int deltaY = y - lastMouseY;

	if (mouseMode != LDVMouseLight)
	{
		//debugPrintf("LDVMouseLight is the only mode currently supported.\n");
		return false;
	}
	lastMouseX = x;
	lastMouseY = y;
	switch (mouseMode)
	{
	case LDVMouseLight:
		mouseMoveLight(deltaX, deltaY);
		requestRedraw();
		break;
	default:
		break;
	}
	return true;
}

void LDrawModelViewer::mouseMoveLight(int deltaX, int deltaY)
{
	TCFloat matrix[16];
	double lightScale = std::min(scale(width), scale(height)) / 10.0;
	double angle = deltaX / lightScale;
	TCVector newLightVector;

	TCVector::initIdentityMatrix(matrix);
	matrix[0] = (float)cos(angle);
	matrix[2] = (float)-sin(angle);
	matrix[8] = (float)sin(angle);
	matrix[10] = (float)cos(angle);
	newLightVector = lightVector.transformPoint(matrix);
	angle = deltaY / lightScale;
	TCVector::initIdentityMatrix(matrix);
	matrix[5] = (float)cos(angle);
	matrix[6] = (float)sin(angle);
	matrix[9] = (float)-sin(angle);
	matrix[10] = (float)cos(angle);
	setLightVector(newLightVector.transformPoint(matrix));
}

UCSTR LDrawModelViewer::getOpenGLDriverInfo(int &numExtensions)
{
	UCSTR vendorString = mbstoucstring((const char*)glGetString(GL_VENDOR));
	UCSTR rendererString = mbstoucstring((const char*)glGetString(GL_RENDERER));
	UCSTR versionString = mbstoucstring((const char*)glGetString(GL_VERSION));
	const char *extensionsString = (const char*)glGetString(GL_EXTENSIONS);
	UCSTR extensionsList;
	size_t len;
	UCSTR message;

	numExtensions = 0;
	if (!vendorString)
	{
		vendorString = copyString(ls(_UC("*Unknown*")));
	}
	if (!rendererString)
	{
		rendererString = copyString(ls(_UC("*Unknown*")));
	}
	if (!versionString)
	{
		versionString = copyString(ls(_UC("*Unknown*")));
	}
	if (extensionsString)
	{
		char *temp = stringByReplacingSubstring(extensionsString, " ",
			"\r\n");

		stripCRLF(temp);
		numExtensions = countStringLines(temp);
		extensionsList = mbstoucstring(temp);
		delete[] temp;
	}
	else
	{
		extensionsList = copyString(ls(_UC("*None*")));
	}
	len = ucstrlen(vendorString) + ucstrlen(rendererString) +
		ucstrlen(versionString) + ucstrlen(extensionsList) + 128;
	message = new UCCHAR[len];
	sucprintf(message, len, ls(_UC("OpenGlInfo")), vendorString, rendererString,
		versionString, extensionsList);
	delete[] vendorString;
	delete[] rendererString;
	delete[] versionString;
	delete[] extensionsList;
	return message;
}

bool LDrawModelViewer::getViewInfo(ucstring &message, ucstring &commandLine)
{
	if (mainModel)
	{
		TCFloat matrix[16];
		TCFloat otherMatrix[16] = {1,0,0,0,0,-1,0,0,0,0,-1,0,0,0,0,1};
		UCCHAR matrixString[1024];
		UCCHAR zoomString[128];
		UCCHAR messageBuf[4096];
		UCCHAR commandLineBuf[1024];
		TCFloat cameraDistance;

		TCVector::multMatrix(otherMatrix, rotationMatrix, matrix);
		cleanupFloats(matrix);
		sucprintf(matrixString, COUNT_OF(matrixString),
			_UC("%.6g,%.6g,%.6g,%.6g,%.6g,%.6g,%.6g,%.6g,%.6g"), matrix[0],
			matrix[4], matrix[8], matrix[1], matrix[5], matrix[9],
			matrix[2], matrix[6], matrix[10]);
		cameraDistance = camera.getPosition().length();
		if (distanceMultiplier == 0.0f || cameraDistance == 0.0f)
		{
			// If we don't have a model, we don't know the default zoom, so
			// just say 1.
			ucstrcpy(zoomString, _UC("1"));
		}
		else
		{
			sucprintf(zoomString, COUNT_OF(zoomString), _UC("%.6g"),
				defaultDistance / distanceMultiplier / cameraDistance);
		}
		sucprintf(messageBuf, COUNT_OF(messageBuf), ls(_UC("ViewInfoMessage")),
			matrixString, zoomString);
		sucprintf(commandLineBuf, COUNT_OF(commandLineBuf),
			_UC("-DefaultMatrix=%s -DefaultZoom=%s"), matrixString, zoomString);
		message = messageBuf;
		commandLine = commandLineBuf;
		return true;
	}
	else
	{
		return false;
	}
}

TCFloat LDrawModelViewer::getWideLineMargin(void)
{
	TCFloat margin = 0.0f;

	if (flags.showsHighlightLines)
	{
		if (getScaledHighlightLineWidth() >= 2.0f)
		{
			margin = getScaledHighlightLineWidth() / 2.0f;
		}
		else
		{
			margin = 1.0f;
		}
	}
	if (flags.drawWireframe && getScaledWireframeLineWidth() > 1.0)
	{
		if (getScaledWireframeLineWidth() / 2.0f > margin)
		{
			margin = getScaledWireframeLineWidth() / 2.0f;
		}
	}
	return margin;
}

//static int _numPoints = 0;

// This is conversion of Lars Hassing's auto camera code from L3P.  It computes
// the correct distance and pan amount for the camera so that the viewing
// pyramid will be positioned in the closest possible position, such that the
// model just touches the edges of the view on either the top and bottom, the
// left and right, or all four.
// After processing all the model data for the current camera angle, it ends up
// with 6 equations with 6 unknowns.  It uses a matrix solving routine to solve
// these.  The 6 values seem to be the X, Y, and Z coordinates of two points.
// Once it has the values, it decides which point is the correct point, and then
// uses that as the camera location.
void LDrawModelViewer::zoomToFit(void)
{
	if (mainTREModel)
	{
		LDLAutoCamera *autoCamera = new LDLAutoCamera;
		char *cameraGlobe = TCUserDefaults::stringForKey(CAMERA_GLOBE_KEY, NULL,
			false);

		autoCamera->setModel(getCurModel());
		autoCamera->setModelCenter(center);
		autoCamera->setRotationMatrix(rotationMatrix);
		autoCamera->setCamera(camera);
		autoCamera->setCameraGlobe(cameraGlobe);
		autoCamera->setDistanceMultiplier(distanceMultiplier);
		autoCamera->setWidth(scale(width) * numXTiles / getStereoWidthModifier());
		autoCamera->setHeight(scale(height) * numYTiles);
		autoCamera->setMargin(getWideLineMargin() * 2.0f);
		autoCamera->setFov(fov);
		autoCamera->setStep(step);
		autoCamera->setScanConditionalControlPoints(
			getShowConditionalControlPoints());

		autoCamera->zoomToFit();
		camera.setPosition(autoCamera->getCamera().getPosition());
		xPan = 0.0f;
		yPan = 0.0f;
		TCObject::release(autoCamera);
	}
}

void LDrawModelViewer::setStep(int value)
{
	step = value - 1;
	if (mainTREModel)
	{
		mainTREModel->setStep(step);
	}
}

int LDrawModelViewer::getNumSteps(void) const
{
	if (mainTREModel)
	{
		return mainTREModel->getNumSteps();
	}
	else
	{
		return 0;
	}
}

void LDrawModelViewer::setLatLon(
	float lat,
	float lon,
	float distance /*= -1.0f*/)
{
	fixLongitude(lon);
	if (viewMode == VMExamine && examineMode == EMLatLong)
	{
		examineLatitude = lat;
		examineLongitude = lon;
	}
	else if (rotationMatrix)
	{
		TCVector::calcRotationMatrix(lat, lon, rotationMatrix);
		flags.needsRotationMatrixSetup = true;
	}
	if (distance >= 0.0f)
	{
		camera.setPosition(TCVector(0.0, 0.0, distance));
		flags.constrainZoom = false;
	}
	else
	{
		flags.constrainZoom = viewMode == VMExamine;
	}
	requestRedraw();
}

void LDrawModelViewer::enable(GLenum cap)
{
	if (getGl2ps())
	{
		GLint mode = GL2PS_BLEND;

		if (cap == GL_POLYGON_OFFSET_FILL)
		{
			mode = GL2PS_POLYGON_OFFSET_FILL;
		}
		gl2psEnable(mode);
	}
	glEnable(cap);
}

void LDrawModelViewer::disable(GLenum cap)
{
	if (getGl2ps())
	{
		GLint mode = GL2PS_BLEND;

		if (cap == GL_POLYGON_OFFSET_FILL)
		{
			mode = GL2PS_POLYGON_OFFSET_FILL;
		}
		gl2psDisable(mode);
	}
	glDisable(cap);
}

void LDrawModelViewer::blendFunc(GLenum sfactor, GLenum dfactor)
{
	if (getGl2ps())
	{
		gl2psBlendFunc(sfactor, dfactor);
	}
	glBlendFunc(sfactor, dfactor);
}

void LDrawModelViewer::lineWidth(GLfloat lwidth)
{
	if (getGl2ps())
	{
		gl2psLineWidth(scale(lwidth));
	}
	glLineWidth(scale(lwidth));
}

void LDrawModelViewer::setMpdChildIndex(int index)
{
	if (index != mpdChildIndex)
	{
		mpdChildIndex = index;
		flags.needsReparse = true;
		flags.needsCalcSize = true;
		flags.needsViewReset = true;
		flags.needsResetStep = true;
		requestRedraw();
	}
}

LDLModel *LDrawModelViewer::getMpdChild(void)
{
	if (mainModel == NULL)
	{
		return NULL;
	}
	LDLModelVector &mpdModels = mainModel->getMpdModels();

	if (mpdModels.size() > 0 && mpdName.size() > 0)
	{
		for (size_t i = 0; i < mpdModels.size(); i++)
		{
			if (strcasecmp(mpdModels[i]->getName(), mpdName.c_str()) == 0)
			{
				mpdChildIndex = (int)i;
				mpdName = "";
				break;
			}
		}
	}
	if ((int)mpdModels.size() > mpdChildIndex && mpdChildIndex >= 0)
	{
		return mpdModels[mpdChildIndex];
	}
	else
	{
		return NULL;
	}
}

const LDLModel *LDrawModelViewer::getMpdChild(void) const
{
	return const_cast<LDrawModelViewer *>(this)->getMpdChild();
}

LDLModel *LDrawModelViewer::getCurModel(void)
{
	LDLModel *curModel = getMpdChild();

	if (curModel == NULL)
	{
		curModel = mainModel;
	}
	return curModel;
}

const LDLModel *LDrawModelViewer::getCurModel(void) const
{
	return const_cast<LDrawModelViewer *>(this)->getCurModel();
}

LDExporter *LDrawModelViewer::initExporter(void)
{
	if (exporter == NULL)
	{
		switch (exportType)
		{
		case ETPov:
			exporter = new LDPovExporter;
			break;
#ifdef EXPORT_LDR
		case ETLdr:
			exporter = new LDLdrExporter;
			break;
#endif // EXPORT_LDR
//		case ETStl:
//			exporter = new LDStlExporter;
//			break;
#ifdef EXPORT_3DS
		case ET3ds:
			exporter = new LD3dsExporter;
			break;
#endif // EXPORT_3DS
		default:
			exporter = NULL;
			break;
		}
	}
	return exporter;
}

void LDrawModelViewer::setExportType(ExportType type, bool forceNew /*= false*/)
{
	if (forceNew || (type != exportType && type != (ExportType)0))
	{
		if (type != (ExportType)0)
		{
			exportType = type;
		}
		TCObject::release(exporter);
		exporter = NULL;
	}
	initExporter();
}

LDExporter *LDrawModelViewer::getExporter(
	ExportType type /*= (ExportType)0*/,
	bool forceNew /*= false*/)
{
	setExportType(type, forceNew);
	return initExporter();
}

int LDrawModelViewer::exportCurModel(
	const char *lfilename,
	const char *version /*= NULL*/,
	const char *copyright /*= NULL*/,
	ExportType type /*= (ExportType)0*/)
{
	LDLModel *model = getCurModel();
	int retValue = 1;

	try
	{
		if (model != NULL)
		{
			setExportType(type);
			TCObject::release(exporter);
			exporter = NULL;
			if (initExporter() != NULL)
			{
				exporter->setBoundingBox(boundingMin, boundingMax);
				exporter->setCenter(center);
				exporter->setWidth(width);
				exporter->setHeight(height);
				exporter->setRadius(size / 2.0f);
				exporter->setBackgroundColor(backgroundR, backgroundG,
					backgroundB);
				exporter->setCamera(camera);
				exporter->setRotationMatrix(rotationMatrix);
				exporter->setFov(fov);
				exporter->setXPan(xPan);
				exporter->setYPan(yPan);
				exporter->setAppUrl("https://trevorsandy.github.io/lpub3d/");
				exporter->setAppName("LPub3D Native POV Generator by LDView");
				if (version != NULL)
				{
					exporter->setAppVersion(version);
				}
				if (copyright)
				{
					exporter->setAppCopyright(copyright);
				}
				if (lfilename != NULL)
				{
					exporter->setFilename(lfilename);
				}
				if (exporter->usesLDLModel())
				{
					if (exporter->usesTREModel())
					{
						retValue = exporter->doExport(model, mainTREModel);
					}
					else
					{
						retValue = exporter->doExport(model);
					}
				}
				else
				{
					retValue = exporter->doExport(mainTREModel);
				}
				exporter->release();
				exporter = NULL;
			}
		}
	}
	catch (char const* exception)
	{
		debugPrintf("Exception during export: %s\n", exception);
		// Until we actually handle this, leave the exception alone.
		throw exception;
	}
	return retValue;
}

std::string LDrawModelViewer::getCurFilename(void) const
{
	const LDLModel *curModel = getCurModel();
	const char *lfilename = getFilename();
	
	if (curModel == mainModel)
	{
		return lfilename;
	}
	else
	{
		const char *modelName = curModel->getName();
		std::string retValue = directoryFromPath(lfilename);
		char *temp;

		retValue += '/';
		retValue += modelName;
		temp = cleanedUpPath(retValue.c_str());
		retValue = temp;
		delete[] temp;
		return retValue;
	}
}

// Note: static method
void LDrawModelViewer::addStandardSize(int width, int height)
{
	StandardSize standardSize;
	ucstring buf = ltoucstr(width);
	ucstring aspectString = getAspectString(width, height, _UC(":"), true);

	standardSize.width = width;
	standardSize.height = height;
	standardSizes.push_back(standardSize);
	buf += _UC(" x ");
	buf += ltoucstr(height);
	if (aspectString.length() > 0)
	{
		buf += _UC(" (");
		buf += aspectString.c_str();
		buf += _UC(")");
	}
	standardSizes.back().name = buf;
}

// Note: static method
void LDrawModelViewer::initStandardSizes(void)
{
	if (standardSizes.size() == 0)
	{
		addStandardSize(640, 480);
		// Note: 720 width has all heights for "standard" aspect ratios.
		addStandardSize(720, 306);	// 2.35:1
		addStandardSize(720, 405);	// 16:9
		addStandardSize(720, 432);	// 5:3
		addStandardSize(720, 450);	// 16:10
		addStandardSize(720, 480);	// 3:2
		addStandardSize(720, 540);	// 4:3
		addStandardSize(720, 576);	// 5:4
		addStandardSize(800, 450);
		addStandardSize(800, 480);
		addStandardSize(800, 600);
		addStandardSize(1024, 576);
		addStandardSize(1024, 768);
		addStandardSize(1152, 864);
		addStandardSize(1280, 544);
		addStandardSize(1280, 720);
		addStandardSize(1280, 768);
		addStandardSize(1280, 960);
		addStandardSize(1280, 1024);
		addStandardSize(1600, 1200);
		addStandardSize(1680, 1050);
		addStandardSize(1920, 817);
		addStandardSize(1920, 1080);
		addStandardSize(1920, 1200);
	}
}

// Note: static method
void LDrawModelViewer::getStandardSizes(
	int maxWidth,
	int maxHeight,
	StandardSizeVector &sizes)
{
	initStandardSizes();
	sizes.clear();
	for (StandardSizeList::const_iterator it = standardSizes.begin();
		it != standardSizes.end(); ++it)
	{
		const StandardSize &standardSize = *it;

		if (standardSize.width <= maxWidth && standardSize.height <= maxHeight)
		{
			sizes.push_back(standardSize);
		}
	}
}

TCFloat LDrawModelViewer::getDistance(void) const
{
	return camera.getPosition().length();
}

void LDrawModelViewer::setCameraMotion(const TCVector &value)
{
	cameraMotion = value;
	updateFrameTime();
}

void LDrawModelViewer::setCameraXRotate(TCFloat value)
{
	cameraXRotate = value;
	updateFrameTime();
}

void LDrawModelViewer::setCameraYRotate(TCFloat value)
{
	cameraYRotate = value;
	updateFrameTime();
}

void LDrawModelViewer::setCameraZRotate(TCFloat value)
{
	cameraZRotate = value;
	updateFrameTime();
}

void LDrawModelViewer::setHighlightPaths(std::string value)
{
	highlightPaths.clear();
	while (value.size() > 0)
	{
		size_t index = value.find('\n');
		std::string line;

		if (index < value.size())
		{
			line = value.substr(0, index);
			value = value.substr(index + 1);
		}
		else
		{
			line = value;
			value = "";
		}
		if (line.size() > 0)
		{
			highlightPaths.push_back(line);
		}
	}
	highlightPathsChanged();
}

void LDrawModelViewer::setHighlightPaths(const StringList &value)
{
	highlightPaths = value;
	highlightPathsChanged();
}

void LDrawModelViewer::attachFileLine(
	LDLFileLine *dstFileLine,
	LDLFileLineArray *dstFileLines,
	LDLModel *dstModel)
{
	if (dstFileLine)
	{
		dstFileLine->setLineNumber(dstFileLines->getCount());
		dstFileLine->setParentModel(dstModel);
		dstFileLines->addObject(dstFileLine);
		dstModel->setActiveLineCount(dstModel->getActiveLineCount() + 1);
		dstFileLine->release();
	}
}

void LDrawModelViewer::attachLineLine(
	LDLFileLineArray *dstFileLines,
	LDLModel *dstModel,
	const TCVector &pt0,
	const TCVector &pt1)
{
	char line[1024];
	LDLLineLine *dstFileLine;

	sprintf(line, "2 16 %s %s", pt0.string().c_str(), pt1.string().c_str());
	dstFileLine = new LDLLineLine(dstModel, line, dstFileLines->getCount());
	dstFileLine->parse();
	attachFileLine(dstFileLine, dstFileLines, dstModel);
}

void LDrawModelViewer::resetColors(LDLModel *model)
{
	if (model != NULL)
	{
		LDLFileLineArray *fileLines = model->getFileLines();

		if (fileLines != NULL)
		{
			int count = model->getActiveLineCount();

			for (int i = 0; i < count; i++)
			{
				resetColors((*fileLines)[i]);
			}
		}
	}
}

void LDrawModelViewer::resetColors(LDLFileLine *fileLine)
{
	if (fileLine != NULL && fileLine->isActionLine())
	{
		LDLActionLine *actionLine = (LDLActionLine *)fileLine;

		actionLine->setColorNumber(16);
		if (actionLine->getLineType() == LDLLineTypeModel)
		{
			resetColors(((LDLModelLine *)actionLine)->getLowResModel());
			resetColors(((LDLModelLine *)actionLine)->getModel(true));
		}
	}
}

void LDrawModelViewer::parseHighlightPath(
	const std::string &path,
	const LDLModel *srcModel,
	LDLModel *dstModel,
	const std::string &prePath,
	int pathNum)
{
	int lineNum = atoi(&path[1]) - 1;
	const LDLFileLineArray *srcFileLines = srcModel->getFileLines();

	if (lineNum < srcFileLines->getCount())
	{
		const LDLFileLine *srcFileLine = (*srcFileLines)[lineNum];
		LDLFileLineArray *dstFileLines = dstModel->getFileLines(true);
		LDLFileLine *dstFileLine = NULL;
		bool isModel = false;
		size_t nextSlash = path.find('/', 1);

		switch (srcFileLine->getLineType())
		{
		case LDLLineTypeModel:
			isModel = true;
			if (nextSlash < path.size())
			{
				const LDLModelLine *srcModelLine =
					(const LDLModelLine *)srcFileLine;
				// The names need to be unique to prevent them from being
				// combined.  Since we're not showing the actual model, we
				// use a combination of our pathNum and the path to now as
				// the model name.
				std::string name = ltostr(pathNum) + prePath + '/';

				name += ltostr(lineNum + 1);
				dstFileLine = new LDLModelLine(dstModel, srcFileLine->getLine(),
					dstFileLines->getCount(), srcFileLine->getOriginalLine());
				LDLModelLine *dstModelLine = (LDLModelLine *)dstFileLine;
				dstModelLine->setMatrix(srcModelLine->getMatrix());
				if (srcModelLine->getLowResModel() != NULL)
				{
					dstModelLine->createLowResModel(dstModel->getMainModel(),
						name.c_str());
				}
				if (srcModelLine->getHighResModel() != NULL)
				{
					dstModelLine->createHighResModel(dstModel->getMainModel(),
						name.c_str());
				}
				const LDLModel *srcHighResModel =
					srcModelLine->getHighResModel();
				if (srcHighResModel->isPart() && !srcModel->isPart() &&
					seamWidth > 0.0)
				{
					// We have to apply seams manually here, because TRE, which
					// normally handles the seams, does so based on the actual
					// geometry, not the LDLModel's bounding box.  Since only a
					// subset of the geometry makes it to TRE, we have to do the
					// seams adjustment here instead.
					TCFloat scaleMatrix[16];
					TCFloat newMatrix[16];

					TCVector lboundingMin, lboundingMax;
					srcHighResModel->getBoundingBox(lboundingMin, lboundingMax);
					TCVector::calcScaleMatrix(seamWidth, scaleMatrix,
						lboundingMin, lboundingMax);
					TCVector::multMatrix(dstModelLine->getMatrix(), scaleMatrix,
						newMatrix);
					dstModelLine->setMatrix(newMatrix);
					dstModelLine->setColorNumber(16);
				}
			}
			else
			{
				dstFileLine = (LDLFileLine *)srcFileLine->copy();
				resetColors((LDLModelLine *)dstFileLine);
			}
			break;
		case LDLLineTypeLine:
		case LDLLineTypeTriangle:
		case LDLLineTypeQuad:
			dstFileLine = (LDLFileLine *)srcFileLine->copy();
			break;
		case LDLLineTypeConditionalLine:
			{
				LDLConditionalLineLine *condLine =
					(LDLConditionalLineLine *)srcFileLine;

				attachLineLine(dstFileLines, dstModel, condLine->getPoints()[0],
					condLine->getPoints()[1]);
				attachLineLine(dstFileLines, dstModel, condLine->getPoints()[0],
					condLine->getControlPoints()[0]);
				attachLineLine(dstFileLines, dstModel, condLine->getPoints()[0],
					condLine->getControlPoints()[1]);
			}
			break;
		default:
			// Don't do anything for other line types.
			break;
		}
		if (!isModel)
		{
			if (nextSlash < path.size())
			{
				throw "Invalid Tree Path";
			}
		}
		attachFileLine(dstFileLine, dstFileLines, dstModel);
		if (isModel)
		{
			if (nextSlash < path.size())
			{
				LDLModelLine *dstModelLine = (LDLModelLine *)dstFileLine;

				if (dstModelLine->getLowResModel())
				{
					parseHighlightPath(path.substr(nextSlash),
						((LDLModelLine *)srcFileLine)->getModel(),
						dstModelLine->getLowResModel(),
						prePath + path.substr(0, nextSlash),
						pathNum);
				}
				if (dstModelLine->getHighResModel())
				{
					parseHighlightPath(path.substr(nextSlash),
						((LDLModelLine *)srcFileLine)->getModel(),
						dstModelLine->getHighResModel(),
						prePath + path.substr(0, nextSlash),
						pathNum);
				}
			}
		}
	}
}

void LDrawModelViewer::setHighlightColor(
	int r,
	int g,
	int b,
	bool redraw /*= true*/)
{
	highlightR = r & 0xFF;
	highlightG = g & 0xFF;
	highlightB = b & 0xFF;
	if (redraw)
	{
		highlightPathsChanged();
	}
}

std::string LDrawModelViewer::adjustHighlightPath(
	std::string path,
	LDLModel *mpdChild)
{
	LDLModel *curModel = mainModel;

	while (curModel != mpdChild && path.size() > 0)
	{
		int lineNum = atoi(&path[1]) - 1;
		const LDLFileLineArray *fileLines = curModel->getFileLines();

		if (lineNum < fileLines->getCount())
		{
			const LDLFileLine *fileLine = (*fileLines)[lineNum];
			size_t index;

			if (fileLine->getLineType() != LDLLineTypeModel)
			{
				return "";
			}
			curModel = ((LDLModelLine *)fileLine)->getModel();
			index = path.find('/', 1);
			if (index < path.size())
			{
				path = path.substr(index);
			}
		}
		else
		{
			return "";
		}
	}
	return path;
}

void LDrawModelViewer::highlightPathsChanged(void)
{
	TCObject::release(highlightModel);
	highlightModel = NULL;
	if (highlightPaths.size() > 0)
	{
		LDModelParser *modelParser = NULL;
		LDLMainModel *ldlModel = new LDLMainModel;
		int i = 0;
		LDLModel *mpdChild = getMpdChild();

		ldlModel->setMainModel(ldlModel);
		ldlModel->setForceHighlightColor(true);
		ldlModel->setHighlightColorNumber(0x3000000 |
			(highlightR << 16) | (highlightG << 8) | highlightB);
		ldlModel->setLowResStuds(!flags.qualityStuds);
		ldlModel->setTexmaps(false);
		for (StringList::const_iterator it = highlightPaths.begin();
			it != highlightPaths.end(); ++it)
		{
			if (mpdChild != NULL)
			{
				std::string path = adjustHighlightPath(*it, mpdChild);
				if (path.size() > 0)
				{
					parseHighlightPath(path, mpdChild, ldlModel, "", i++);
				}
			}
			else
			{
				parseHighlightPath(*it, mainModel, ldlModel, "", i++);
			}
		}
		modelParser = new LDModelParser(this);
		modelParser->setTexmapsFlag(false);
		if (modelParser->parseMainModel(ldlModel))
		{
			highlightModel = modelParser->getMainTREModel();
			highlightModel->retain();
			highlightModel->setSaveAlphaFlag(false);
		}
		ldlModel->release();
		modelParser->release();
	}
	requestRedraw();
}

// Note: static method
void LDrawModelViewer::resetUnofficialDownloadTimes(void)
{
	TCUserDefaults::removeValueGroup("UnofficialPartChecks", false);
}
