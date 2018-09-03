#ifndef __LDRAWMODELVIEWER_H__
#define __LDRAWMODELVIEWER_H__

#include <TCFoundation/TCAlertSender.h>
#include <TCFoundation/TCStringArray.h>
#include <TCFoundation/mystring.h>
#include <TCFoundation/TCStlIncludes.h>
#include <TCFoundation/TCTypedObjectArray.h>
#include <LDLoader/LDLCamera.h>
#include <TRE/TREGL.h>
#include <LDExporter/LDExporter.h>
#ifdef USE_CPP11
#define USE_STD_CHRONO
#include <chrono>
#endif // USE_CPP11
#ifndef USE_STD_CHRONO
#ifdef _QT
#include <QtCore/qdatetime.h>
#endif
#endif // !USE_STD_CHRONO

typedef enum
{
	LDVStereoNone,
	LDVStereoHardware,
	LDVStereoCrossEyed,
	LDVStereoParallel
} LDVStereoMode;

typedef enum
{
	LDVCutawayNormal,
	LDVCutawayWireframe,
	LDVCutawayStencil
} LDVCutawayMode;

typedef enum
{
	LDVAngleDefault,
	LDVAngleFront,
	LDVAngleBack,
	LDVAngleLeft,
	LDVAngleRight,
	LDVAngleTop,
	LDVAngleBottom,
	LDVAngleIso,
} LDVAngle;

typedef enum
{
	LDVMouseNone,
	LDVMouseNormal,
	LDVMouseZoom,
	LDVMousePan,
	LDVMouseLight
} LDVMouseMode;

class TCImage;
class LDLError;
class TCProgressAlert;
class TREMainModel;
class LDLMainModel;
class LDLModel;
class LDLFileLine;
class LDLFindFileAlert;
class TCWebClient;
class LDPreferences;
class LDPartsList;
class LDViewPoint;
class LDInputHandler;
class LDLFileLine;
typedef TCTypedObjectArray<LDLFileLine> LDLFileLineArray;

typedef std::list<std::string> StringList;

class LDrawModelViewer: public TCAlertSender
{
	public:
		enum ViewMode
		{
			VMExamine,
			VMFlyThrough,
			VMWalk,
		};
		enum ExamineMode
		{
			EMFree,
			EMLatLong,
		};
		enum ExportType
		{
			ETFirst = 1,
			ETPov = ETFirst,
#ifdef EXPORT_LDR
			ETLdr,
#endif // EXPORT_LDR
			ETStl,
#ifdef EXPORT_3DS
			ET3ds,
			ETLast = ET3ds
#else // EXPORT_3DS
			ETLast = ETStl
#endif // EXPORT_3DS
		};
		struct StandardSize
		{
			int width;
			int height;
			ucstring name;
		};
		typedef std::list<StandardSize> StandardSizeList;
		typedef std::vector<StandardSize> StandardSizeVector;

		LDrawModelViewer(TCFloat, TCFloat);
		LDInputHandler *getInputHandler(void);
		virtual void update(void);
		virtual void perspectiveView(void);
		virtual void perspectiveView(bool resetViewport);
		void setLDConfig(const std::string& value) { m_ldConfig = value; }
		const std::string& getLDConfig(void) const { return m_ldConfig; }
		void setQualityLighting(bool value) { flags.qualityLighting = value; }
		bool getQualityLighting(void) const
		{
			return flags.qualityLighting != false;
		}
		virtual void setSubduedLighting(bool value);
		bool getSubduedLighting(void) const
		{
			return flags.subduedLighting != false;
		}
		virtual void setShowsHighlightLines(bool value);
		bool getShowsHighlightLines(void) const
		{
			return flags.showsHighlightLines != false;
		}
		virtual void setDrawConditionalHighlights(bool value);
		bool getDrawConditionalHighlights(void) const
		{
			return flags.drawConditionalHighlights != false;
		}
		virtual void setPerformSmoothing(bool value);
		bool getPerformSmoothing(void) const
		{
			return flags.performSmoothing != false;
		}
		void setConstrainZoom(bool value) { flags.constrainZoom = value; }
		bool getConstrainZoom(void) const
		{
			return flags.constrainZoom != false;
		}
		virtual void setLineSmoothing(bool value);
		bool getLineSmoothing(void) const
		{
			return flags.lineSmoothing != false;
		}
		virtual void setMemoryUsage(int value);
		int getMemoryUsage(void) const { return memoryUsage; }
		virtual void setQualityStuds(bool value);
		bool getQualityStuds(void) const
		{
			return flags.qualityStuds != false;
		}
		virtual void setAllowPrimitiveSubstitution(bool value);
		bool getAllowPrimitiveSubstitution(void) const
		{
			return flags.allowPrimitiveSubstitution != false;
		}
		void setUsesFlatShading(bool value) { flags.usesFlatShading = value; }
		bool getUsesFlatShading(void) const
		{
			return flags.usesFlatShading != false;
		}
		void setObi(bool value);
		bool getObi(void) const { return flags.obi != false; }
		void setGl2ps(bool value);
		bool getGl2ps(void) const { return flags.gl2ps; }
		virtual void setUsesSpecular(bool value);
		bool getUsesSpecular(void) const { return flags.usesSpecular != false; }
		virtual void setOneLight(bool value);
		bool getOneLight(void) const { return flags.oneLight != false; }
		virtual bool forceOneLight(void) const;
		virtual void setWidth(TCFloat value);
		virtual void setHeight(TCFloat value);
		int getWidth(void) const { return (int)width; }
		int getHeight(void) const { return (int)height; }
		TCFloat getFloatWidth(void) const { return width; }
		TCFloat getFloatHeight(void) const { return height; }
		virtual void setScaleFactor(TCFloat value);
		TCFloat getScaleFactor(void) const { return scaleFactor; }
		int scale(int value) const { return (int)(scaleFactor * value); }
		TCFloat scale(TCFloat value) const { return (TCFloat)(scaleFactor * value); }
		void setViewMode(ViewMode value);
		ViewMode getViewMode(void) const { return viewMode; }
		void setExamineMode(ExamineMode value);
		ExamineMode getExamineMode(void) const { return examineMode; }
		void setXRotate(TCFloat value) { xRotate = value; }
		void setYRotate(TCFloat value) { yRotate = value; }
		void setZRotate(TCFloat value) { zRotate = value; }
		TCFloat getXRotate(void) { return xRotate; }
		TCFloat getYRotate(void) { return yRotate; }
		TCFloat getZRotate(void) { return zRotate; }
		void panXY(int xValue, int yValue);
		TCFloat getXPan(void) { return xPan; }
		TCFloat getYPan(void) { return yPan; }
		void setXYPan(TCFloat xValue, TCFloat yValue);
		void setRotationSpeed(TCFloat value);
		TCFloat getRotationSpeed(void) const { return rotationSpeed; }
		void setCameraXRotate(TCFloat value);
		void setCameraYRotate(TCFloat value);
		void setCameraZRotate(TCFloat value);
		TCFloat getCameraXRotate(void) { return cameraXRotate; }
		TCFloat getCameraYRotate(void) { return cameraYRotate; }
		TCFloat getCameraZRotate(void) { return cameraZRotate; }
		void setCameraMotion(const TCVector &value);
		TCVector getCameraMotion(void) { return cameraMotion; }
		virtual void setZoomSpeed(TCFloat value);
		TCFloat getZoomSpeed(void) { return zoomSpeed; }
		virtual void zoom(TCFloat amount, bool apply = true);
		virtual void updateCameraPosition(void);
		virtual void applyZoom(void);
		void setClipZoom(bool value) { clipZoom = value; }
		bool getClipZoom(void) const { return clipZoom != false; }
		virtual void setFilename(const char*);
		virtual void setProgramPath(const char *value);
		void setFileIsPart(bool);
		bool getFileIsPart(void) const { return flags.fileIsPart != false; }
		char* getFilename(void) { return filename; }
		const char *getFilename(void) const { return filename; }
		std::string getCurFilename(void) const;
		const StringList &getHighlightPaths(void) const
		{
			return highlightPaths;
		}
		void setHighlightPaths(std::string value);
		void setHighlightPaths(const StringList &value);
		void setHighlightColor(int r, int g, int b, bool redraw = true);
		virtual int loadModel(bool = true);
		virtual void drawFPS(TCFloat);
		virtual void drawBoundingBox(void);
		virtual void setup(void);
		virtual void setBackgroundRGB(int r, int g, int b);
		virtual void setBackgroundRGBA(int r, int g, int b, int a);
		int getBackgroundR(void) { return (int)(backgroundR * 255); }
		int getBackgroundG(void) { return (int)(backgroundG * 255); }
		int getBackgroundB(void) { return (int)(backgroundB * 255); }
		int getBackgroundA(void) { return (int)(backgroundA * 255); }
		virtual void setDefaultRGB(TCByte r, TCByte g, TCByte b,
			bool transparent);
		virtual void getDefaultRGB(TCByte &r, TCByte &g, TCByte &b,
			bool &transparent) const;
		virtual void setDefaultColorNumber(int value);
		int getDefaultColorNumber(void) const { return defaultColorNumber; }
		virtual void setSeamWidth(TCFloat);
		TCFloat getSeamWidth(void) const { return seamWidth; }
		virtual void setDrawWireframe(bool);
		bool getDrawWireframe(void) const
		{
			return flags.drawWireframe != false;
		}
		virtual void setBfc(bool value);
		bool getBfc(void) const { return flags.bfc != false; }
		virtual void setRedBackFaces(bool value);
		bool getRedBackFaces(void) const { return flags.redBackFaces != false; }
		virtual void setGreenFrontFaces(bool value);
		bool getGreenFrontFaces(void) const
		{
			return flags.greenFrontFaces != false;
		}
		virtual void setBlueNeutralFaces(bool value);
		bool getBlueNeutralFaces(void) const
		{
			return flags.blueNeutralFaces != false;
		}
		void setCheckPartTracker(bool value) { flags.checkPartTracker = value; }
		bool getCheckPartTracker(void) const
		{
			return flags.checkPartTracker != false;
		}
		void setDrawLightDats(bool value);
		bool getDrawLightDats(void) const
		{
			return flags.drawLightDats != false;
		}
		void setOptionalStandardLight(bool value);
		bool getOptionalStandardLight(void) const
		{
			return flags.optionalStandardLight != false;
		}
		bool getNoLightGeom(void) const { return flags.noLightGeom != false; }
		void setNoLightGeom(bool value);
		bool getUpdating(void) const { return flags.updating != false; }
		void setMissingPartWait(int value) { missingPartWait = value; }
		int getMissingPartWait(void) { return missingPartWait; }
		void setUpdatedPartWait(int value) { updatedPartWait = value; }
		int getUpdatedPartWait(void) { return updatedPartWait; }
		virtual void setUseWireframeFog(bool);
		bool getUseWireframeFog(void) const
		{
			return flags.useWireframeFog != false;
		}
		virtual void setRemoveHiddenLines(bool value);
		bool getRemoveHiddenLines(void) const
		{
			return flags.removeHiddenLines != false;
		}
		virtual void setEdgesOnly(bool value);
		bool getEdgesOnly(void) const { return flags.edgesOnly != false; }
		virtual void setHiResPrimitives(bool value);
		bool getHiResPrimitives(void) const
		{
			return flags.hiResPrimitives != false;
		}
		virtual void setUsePolygonOffset(bool);
		bool getUsePolygonOffset(void) const
		{
			return flags.usePolygonOffset != false;
		}
		virtual void setUseLighting(bool);
		bool getUseLighting(void) const { return flags.useLighting != false; }
		void setCommandLineStep(int value) { commandLineStep = value; }
		bool getShowLightDir(void) const { return flags.showLight != false; }
		void setShowLightDir(bool value);
		virtual void setUseStipple(bool);
		bool getUseStipple(void) const { return flags.useStipple != false; }
		virtual void setSortTransparent(bool);
		bool getSortTransparent(void) const
		{
			return flags.sortTransparent != false;
		}
		virtual void setHighlightLineWidth(TCFloat32 value);
		TCFloat32 getHighlightLineWidth(void) const
		{
			return highlightLineWidth;
		}
		TCFloat32 getScaledHighlightLineWidth(void) const;
		virtual void setWireframeLineWidth(TCFloat32 value);
		TCFloat32 getWireframeLineWidth(void) const
		{
			return wireframeLineWidth;
		}
		TCFloat32 getScaledWireframeLineWidth(void) const;
		virtual void setAnisoLevel(TCFloat32 value);
		TCFloat32 getAnisoLevel(void) const { return anisoLevel; }
		virtual void setProcessLDConfig(bool value);
		bool getProcessLDConfig(void) const
		{
			return flags.processLDConfig != false;
		}
		virtual void setSkipValidation(bool value);
		bool getSkipValidation(void) const
		{
			return flags.skipValidation != false;
		}
		void setAutoCenter(bool value) { flags.autoCenter = value; }
		bool getAutoCenter(void) const { return flags.autoCenter != false; }
		virtual void setForceZoomToFit(bool value);
		bool getForceZoomToFit(void) const
		{
			return flags.forceZoomToFit != false;
		}
		virtual void setSaveAlpha(bool value);
		bool getSaveAlpha(void) const { return flags.saveAlpha != false; }
		virtual void setMultiThreaded(bool value);
		bool getMultiThreaded(void) const
		{
			return flags.multiThreaded != false;
		}
		virtual void setShowAxes(bool value) { flags.showAxes = value; }
		bool getShowAxes(void) const { return flags.showAxes != false; }
		virtual void setAxesAtOrigin(bool value) { flags.axesAtOrigin = value; }
		bool getAxesAtOrigin(void) const { return flags.axesAtOrigin != false; }
		virtual void setShowBoundingBox(bool value);
		bool getShowBoundingBox(void) const
		{
			return flags.showBoundingBox != false;
		}
		virtual void setBoundingBoxesOnly(bool value);
		bool getBoundingBoxesOnly(void) const
		{
			return flags.boundingBoxesOnly != false;
		}
		const TCVector &getBoundingMin(void) const { return boundingMin; }
		const TCVector &getBoundingMax(void) const { return boundingMax; }
		virtual bool recompile(void);
		virtual void uncompile(void);
		virtual void reload(void);
		virtual void reparse(void);
//		virtual void setProgressCallback(LDMProgressCallback callback,
//			void* userData);
//		virtual void setErrorCallback(LDMErrorCallback callback,
//			void* userData);
		virtual void clear(void);
		virtual void resetView(LDVAngle viewAngle = LDVAngleDefault);
		virtual void setLatLon(float lat, float lon, float distance = -1.0);
		virtual void pause(void);
		virtual void unpause(void);
		bool getPaused(void) const { return flags.paused != false; }
		virtual void setXTile(int value);
		int getXTile(void) { return xTile; }
		virtual void setYTile(int value);
		int getYTile(void) { return yTile; }
		virtual void setNumXTiles(int value);
		int getNumXTiles(void) { return numXTiles; }
		virtual void setNumYTiles(int value);
		int getNumYTiles(void) { return numYTiles; }
		virtual void setStereoMode(LDVStereoMode mode);
		LDVStereoMode getStereoMode(void) { return stereoMode; }
		void setStereoEyeSpacing(TCFloat spacing)
		{
			stereoEyeSpacing = spacing;
		}
		TCFloat getStereoEyeSpacing(void) { return stereoEyeSpacing; }
		virtual void setCutawayMode(LDVCutawayMode mode);
		LDVCutawayMode getCutawayMode(void) { return cutawayMode; }
		virtual void setCutawayAlpha(TCFloat32 value);
		TCFloat32 getCutawayAlpha(void) { return cutawayAlpha; }
		virtual void setCutawayLineWidth(TCFloat32 value);
		TCFloat32 getCutawayLineWidth(void) { return cutawayLineWidth; }
		TCFloat32 getScaledCutawayLineWidth(void) const;
		void setSlowClear(bool value) { flags.slowClear = value; }
		bool getSlowClear(void) const { return flags.slowClear != false; }
		virtual void setBlackHighlights(bool value);
		bool getBlackHighlights(void) const
		{
			return flags.blackHighlights != false;
		}
		void setZoomMax(TCFloat value) { zoomMax = value; }
		TCFloat getZoomMax(void) { return zoomMax; }
		virtual void setShowAllConditionalLines(bool value);
		bool getShowAllConditionalLines(void) const
		{
			return flags.showAllConditionalLines != false;
		}
		virtual void setShowConditionalControlPoints(bool value);
		bool getShowConditionalControlPoints(void) const
		{
			return flags.showConditionalControlPoints != false;
		}
		bool getNeedsReload(void) const { return flags.needsReload != false; }
		void setNeedsReload(void) { flags.needsReload = true; }
		bool getNeedsReparse(void) const { return flags.needsReparse != false; }
		void setNeedsReparse(void) { flags.needsReparse = true; }
		bool getNeedsRecompile(void) const
		{
			return flags.needsRecompile != false;
		}
		void setNeedsRecompile(void) { flags.needsRecompile = true; }
		bool getNeedsCalcSize(void) const
		{
			return flags.needsCalcSize != false;
		}
		void setCurveQuality(int value);
		int getCurveQuality(void) const { return curveQuality; }
		void setTextureStuds(bool value);
		bool getTextureStuds(void) const { return flags.textureStuds != false; }
		void setTextureFilterType(int value);
		int getTextureFilterType(void) const { return textureFilterType; }
		TREMainModel *getMainTREModel(void) { return mainTREModel; }
		void setMpdChildIndex(int index);
		int getMpdChildIndex(void) const { return mpdChildIndex; }
		LDLModel *getMpdChild(void);
		const LDLModel *getMpdChild(void) const;
		LDLModel *getCurModel(void);
		const LDLModel *getCurModel(void) const;
		LDLMainModel *getMainModel(void) { return mainModel; }
		const LDLMainModel *getMainModel(void) const { return mainModel; }
		bool getCompiled(void) const;
		void setPixelAspectRatio(TCFloat value) { pixelAspectRatio = value; }
		TCFloat getPixelAspectRatio(void) { return pixelAspectRatio; }
		bool getLDrawCommandLineMatrix(char *matrixString, int bufferLength);
		bool getLDrawCommandLine(char *shortFilename, char *commandString,
			int bufferLength);
		bool getLDGLiteCommandLine(char *commandString, int bufferLength);
		void setDistanceMultiplier(TCFloat value)
		{
			distanceMultiplier = value;
		}
		TCFloat getDistanceMultiplier(void) { return distanceMultiplier; }
		virtual void clearBackground(void);
		virtual void setFontData(TCByte *fontData, long length);
		virtual void setModelCenter(const TCFloat *value);
		virtual void setModelSize(const TCFloat value);
		bool getNoUI(void) const { return flags.noUI ? true : false; }
		void setNoUI(bool value) { flags.noUI = value; }
		bool getKeepRightSideUp(void) const
		{
			return flags.keepRightSideUp ? true : false;
		}
		void setKeepRightSideUp(bool value) { flags.keepRightSideUp = value; }
		bool getTexmaps(void) const { return flags.texmaps ? true : false; }
		void setTexmaps(bool value);
		bool getTexturesAfterTransparent(void) const
		{
			return flags.texturesAfterTransparent ? true : false;
		}
		void setTexturesAfterTransparent(bool value);
		TCFloat getTextureOffsetFactor(void) const
		{
			return textureOffsetFactor;
		}
		void setTextureOffsetFactor(TCFloat value);
		bool getRandomColors(void) const
		{
			return flags.randomColors ? true : false;
		}
		virtual void setRandomColors(bool value);
		bool getPovCameraAspect(void) const
		{
			return flags.povCameraAspect ? true : false;
		}
		virtual void setPovCameraAspect(bool value, bool saveSetting);
		virtual void setDefaultRotationMatrix(const TCFloat *value);
		virtual void setDefaultLatLong(TCFloat latitude, TCFloat longitude);
		const TCFloat *getDefaultRotationMatrix(void)
		{
			return defaultRotationMatrix;
		}
		const TCFloat *getRotationMatrix(void) { return rotationMatrix; }
		TCFloat getExamineLatitude(void) { return examineLatitude; }
		TCFloat getExamineLongitude(void) { return examineLongitude; }
		virtual void setFov(TCFloat value);
		TCFloat getFov(void) { return fov; }
		TCFloat getHFov(void);
		TCFloat getDistance(void) const;
		TCFloat getDefaultDistance(void) const { return defaultDistance; }
		void setDefaultDistance(TCFloat value);
		void setExtraSearchDirs(TCStringArray *value);
		TCStringArray *getExtraSearchDirs(void) { return extraSearchDirs; }
		bool skipCameraPositioning(void);
		virtual LDLCamera &getCamera(void) { return camera; }
		virtual void zoomToFit(void);
		virtual void openGlWillEnd(void);
		virtual void setLightVector(const TCVector &value);
		TCVector getLightVector(void) { return lightVector; }
		virtual void getPovCameraInfo(UCCHAR *&userMessage, char *&povCamera);
		virtual void setPreferences(LDPreferences *value)
		{
			// Don't retain; it retains us.
			preferences = value;
		}
		virtual LDPartsList *getPartsList(void);
		LDViewPoint *saveViewPoint(void) const;
		void restoreViewPoint(const LDViewPoint *viewPoint);
		void rightSideUp(bool shouldRequestRedraw = true);
		virtual void setupFont(const char *fontFilename);
		virtual void setupFont2x(const char *fontFilename);
		virtual void setRawFont2xData(const TCByte *data, long length);
		virtual void setFont2x(TCImage *image);
		virtual int exportCurModel(const char *filename,
			const char *version = NULL, const char *copyright = NULL,
			ExportType type = (ExportType)0);
		virtual void setExportType(ExportType type, bool forceNew = false);
		virtual LDExporter *getExporter(ExportType type = (ExportType)0,
			bool forceNew = false);
		virtual ExportType getExportType(void) const { return exportType; }

		virtual bool mouseDown(LDVMouseMode mode, int x, int y);
		virtual bool mouseUp(int x, int y);
		virtual bool mouseMove(int x, int y);
		virtual void showLight(void);
		virtual void orthoView(void);

		virtual void requestRedraw(void);
		virtual void mouseMoveLight(int deltaX, int deltaY);

		bool getViewInfo(ucstring &message, ucstring &commandLine);
		TREMainModel *getContrastingLightDirModel();
		virtual TCObject *getAlertSender(void) { return this; }

		void setStep(int value);
		int getStep(void) const { return step + 1; }
		int getNumSteps(void) const;

		void enable(GLenum cap);
		void disable(GLenum cap);
		void blendFunc(GLenum sfactor, GLenum dfactor);
		void lineWidth(GLfloat width);

		static UCSTR getOpenGLDriverInfo(int &numExtensions);
		static void cleanupFloats(TCFloat *array, int count = 16);
		static bool fileExists(const char *filename);
		static const char *alertClass(void) { return "LDrawModelViewerAlert"; }
		static const char *redrawAlertClass(void) { return "LDRedrawNeeded"; }
		static const char *frameDoneAlertClass(void) { return "LDFrameDone"; }
		static const char *loadAlertClass(void)
		{
			return "LDrawModelViewerLoad";
		}
		static void getStandardSizes(int maxWidth, int maxHeight,
			StandardSizeVector &sizes);
		static void resetUnofficialDownloadTimes(void);
//		static bool doCommandLineExport(void);
	protected:
		~LDrawModelViewer(void);
		void dealloc(void);
//		bool commandLineExport(void);
//		ExportType exportTypeForFilename(const char* filename);
		virtual void drawSetup(TCFloat eyeXOffset = 0.0f);
		virtual void drawModel(TCFloat eyeXOffset = 0.0f);
		virtual void innerDrawModel(void);
		virtual void drawAxes(bool atOrigin);
		virtual void removeHiddenLines(TCFloat eyeXOffset = 0.0f);
		virtual void setFieldOfView(double, TCFloat, TCFloat);
		virtual void setupRotationMatrix(void);
		virtual void setupMaterial(void);
		virtual void setupLight(GLenum light,
			const TCVector &color = TCVector(1.0, 1.0, 1.0));
		void drawLight(GLenum, TCFloat, TCFloat, TCFloat);
		virtual void setupLighting(void);
		virtual void setupTextures(void);
		void drawLights(void);
		void drawLightDats(void);
		virtual void drawToClipPlane(TCFloat eyeXOffset);
		virtual void drawToClipPlaneUsingStencil(TCFloat eyeXOffset);
//		virtual void drawToClipPlaneUsingAccum(TCFloat eyeXOffset);
		virtual void drawToClipPlaneUsingDestinationAlpha(TCFloat eyeXOffset);
		virtual void drawToClipPlaneUsingNoEffect(TCFloat eyeXOffset);
		virtual void perspectiveViewToClipPlane(void);
		virtual void applyTile(void);
		virtual void drawString(TCFloat xPos, TCFloat yPos, char* string);
		virtual void loadVGAFont(const char *fontFilename);
		virtual void setupDefaultViewAngle(void);
		virtual void setupFrontViewAngle(void);
		virtual void setupBackViewAngle(void);
		virtual void setupLeftViewAngle(void);
		virtual void setupRightViewAngle(void);
		virtual void setupTopViewAngle(void);
		virtual void setupBottomViewAngle(void);
//		void ldlErrorCallback(LDLError *error);
//		void progressAlertCallback(TCProgressAlert *error);
		virtual void setupIsoViewAngle(void);
		virtual TCFloat calcDefaultDistance(void);
		virtual void updateCurrentFov(void);
		virtual TCFloat getStereoWidthModifier(void);
		virtual TCFloat getWideLineMargin(void);
		virtual TCFloat getClipRadius(void);
		virtual TCFloat getZDistance(void);
		virtual bool haveLightDats(void) const;
		virtual bool haveStandardLight(void);
//		void findFileAlertCallback(LDLFindFileAlert *alert);
		virtual bool canCheckForUnofficialPart(const char *filename,
			bool exists);
		virtual void unofficialPartNotFound(const char *filename);
//		virtual bool connectionFailure(TCWebClient *webClient);
		virtual void applyModelRotation(void);
		virtual bool loadLDLModel(void);
		virtual bool calcSize(void);
		virtual bool parseModel(void);
		virtual void releaseTREModels(void);
		virtual LDExporter *initExporter(void);

		void updateFrameTime(bool force = false);
		void highlightPathsChanged(void);
		void parseHighlightPath(const std::string &path,
			const LDLModel *srcModel, LDLModel *dstModel,
			const std::string &prePath, int pathNum);
		void resetColors(LDLFileLine *actionLine);
		void resetColors(LDLModel *model);
		void attachFileLine(LDLFileLine *dstFileLine,
			LDLFileLineArray *dstFileLines, LDLModel *dstModel);
		void attachLineLine(LDLFileLineArray *dstFileLines, LDLModel *dstModel,
			const TCVector &pt0, const TCVector &pt1);
		std::string adjustHighlightPath(std::string path, LDLModel *mpdChild);

		static void fixLongitude(TCFloat &lon);
		static void setUnofficialPartPrimitive(const char *filename,
			bool primitive);
		static void initStandardSizes(void);
		static void addStandardSize(int width, int height);
		static bool checkAspect(TCFloat width, TCFloat height, int aspectW,
			int aspectH);
		static ucstring getAspectString(TCFloat aWidth, TCFloat aHeight,
			CUCSTR separator, bool standardOnly = false);

		//int L3Solve6(TCFloat x[L3ORDERN], const TCFloat A[L3ORDERM][L3ORDERN],
		//	const TCFloat b[L3ORDERM]);
		//void scanCameraPoint(const TCVector &point);
		//virtual void preCalcCamera(void);

		void initLightDirModel(TREMainModel *&lightDirModel, TCULong color);
		void initLightDirModels(void);
		void projectCamera(const TCVector &distance);

		TREMainModel *mainTREModel;
		LDLMainModel *mainModel;
		TREMainModel *whiteLightDirModel;
		TREMainModel *blueLightDirModel;
		TREMainModel *highlightModel;
		char* filename;
		std::string mpdName;
		char* programPath;
		TCFloat width;
		TCFloat height;
		TCFloat scaleFactor;
		TCFloat pixelAspectRatio;
		TCFloat size;
		TCFloat clipSize;
		TCVector center;
		TCVector boundingMin;
		TCVector boundingMax;
		int lastFrameTimeLength;
		int cullBackFaces;
		ViewMode viewMode;
		ExamineMode examineMode;
		TCFloat xRotate;
		TCFloat yRotate;
		TCFloat zRotate;
		TCFloat rotationSpeed;
		TCFloat cameraXRotate;
		TCFloat cameraYRotate;
		TCFloat cameraZRotate;
		TCVector cameraMotion;
		TCFloat zoomSpeed;
		TCFloat xPan;
		TCFloat yPan;
		TCFloat* rotationMatrix;
		TCFloat* defaultRotationMatrix;
		TCFloat defaultLatitude;
		TCFloat defaultLongitude;
		TCFloat examineLatitude;
		TCFloat examineLongitude;
		TCFloat clipAmount;
		TCFloat nextClipAmount;
		TCFloat nextDistance;
		TCFloat32 highlightLineWidth;
		TCFloat32 wireframeLineWidth;
		TCFloat32 anisoLevel;
		bool clipZoom;
		GLuint fontListBase;
		GLclampf backgroundR;
		GLclampf backgroundG;
		GLclampf backgroundB;
		GLclampf backgroundA;
		TCByte defaultR;
		TCByte defaultG;
		TCByte defaultB;
		int defaultColorNumber;
//		LDMProgressCallback progressCallback;
//		void* progressUserData;
//		LDMErrorCallback errorCallback;
//		void* errorUserData;
		int xTile;
		int yTile;
		int numXTiles;
		int numYTiles;
		LDVStereoMode stereoMode;
		TCFloat stereoEyeSpacing;
		LDVCutawayMode cutawayMode;
		TCFloat32 cutawayAlpha;
		TCFloat32 cutawayLineWidth;
		TCFloat zoomMax;
		int curveQuality;
		int textureFilterType;
		TCFloat distanceMultiplier;
		TCImage *fontImage1x;
		TCImage *fontImage2x;
		GLuint fontTextureID;
		LDLCamera camera;
		TCFloat aspectRatio;
		TCFloat currentFov;
		TCFloat fov;
		TCFloat defaultDistance;
		TCStringArray *extraSearchDirs;
		TCFloat seamWidth;
		TCFloat zoomToFitWidth;
		TCFloat zoomToFitHeight;
		TCFloat textureOffsetFactor;
		int memoryUsage;
		TCVector lightVector;
		LDPreferences *preferences;
		int missingPartWait;
		int updatedPartWait;
		LDVMouseMode mouseMode;
		int lastMouseX;
		int lastMouseY;
		LDInputHandler *inputHandler;
		int step;
		int commandLineStep;
		int mpdChildIndex;
		LDExporter *exporter;
		ExportType exportType;
		StringList highlightPaths;
		int highlightR;
		int highlightG;
		int highlightB;
#ifdef USE_STD_CHRONO
	std::chrono::time_point<std::chrono::steady_clock> frameTime;
#else
#ifdef WIN32
		DWORD frameTicks;
		LARGE_INTEGER hrpcFrequency;
		LARGE_INTEGER hrpcFrameCount;
#endif // WIN32
#ifdef COCOA
		void *frameTime;
#endif // COCOA
#ifdef _QT
		QTime qtime;
#endif
#endif // !USE_STD_CHRONO
		std::string m_ldConfig;
		struct
		{
			bool qualityLighting:1;
			bool showsHighlightLines:1;
			bool qualityStuds:1;
			bool usesFlatShading:1;
			bool usesSpecular:1;
			bool drawWireframe:1;
			bool useWireframeFog:1;
			bool removeHiddenLines:1;
			bool usePolygonOffset:1;
			bool useLighting:1;
			bool subduedLighting:1;
			bool allowPrimitiveSubstitution:1;
			bool useStipple:1;
			bool fileIsPart:1;
			bool sortTransparent:1;
			bool needsSetup:1;
			bool needsTextureSetup:1;
			bool needsMaterialSetup:1;
			bool needsLightingSetup:1;
			bool needsReload:1;
			bool needsReparse:1;
			bool needsRecompile:1;
			bool needsResize:1;
			bool needsCalcSize:1;
			bool needsRotationMatrixSetup:1;
			bool needsViewReset:1;
			bool needsResetStep:1;
			bool needsResetMpd:1;
			bool paused:1;
			bool slowClear:1;
			bool blackHighlights:1;
			bool textureStuds:1;
			bool oneLight:1;
			bool drawConditionalHighlights:1;
			bool showAllConditionalLines:1;
			bool showConditionalControlPoints:1;
			bool performSmoothing:1;
			bool lineSmoothing:1;
			bool constrainZoom:1;
			bool edgesOnly:1;
			bool hiResPrimitives:1;
			bool processLDConfig:1;
			bool skipValidation:1;
			bool autoCenter:1;
			bool forceZoomToFit:1;
			bool defaultTrans:1;
			bool bfc:1;
			bool redBackFaces:1;
			bool greenFrontFaces:1;
			bool blueNeutralFaces:1;
			bool defaultLightVector:1;
			bool overrideModelCenter:1;
			bool overrideModelSize:1;
			bool overrideDefaultDistance:1;
			bool checkPartTracker:1;
			bool showLight:1;
			bool drawLightDats:1;
			bool optionalStandardLight:1;
			bool noLightGeom:1;
			bool updating:1;
			bool saveAlpha:1;
			bool multiThreaded:1;
			bool showAxes:1;
			bool axesAtOrigin:1;
			bool showBoundingBox:1;
			bool boundingBoxesOnly:1;
			bool obi:1;
			bool gl2ps:1;
			bool povCameraAspect:1;
			bool animating:1;
			bool randomColors:1;
			bool noUI:1;
			bool keepRightSideUp:1;
			bool texmaps:1;
			bool texturesAfterTransparent:1;
		} flags;
		/*struct CameraData
		{
			CameraData(void)
				:direction(0.0f, 0.0f, -1.0f),
				horizontal(1.0f, 0.0f, 0.0f),
				vertical(0.0f, -1.0f, 0.0f),
				horMin(1e6),
				horMax(-1e6),
				verMin(1e6),
				verMax(-1e6)
			{
				int i;
//				TCVector up = vertical;

//				horizontal = direction * up;
//				vertical = horizontal * direction;
				for (i = 0; i < 4; i++)
				{
					dMin[i] = 1e6;
				}
			}
			TCVector direction;
			TCVector horizontal;
			TCVector vertical;
			TCVector normal[4];
			TCFloat dMin[4];
			TCFloat horMin;
			TCFloat horMax;
			TCFloat verMin;
			TCFloat verMax;
			TCFloat fov;
		} *cameraData;*/
		static StandardSizeList standardSizes;
};

#endif // __LDRAWMODELVIEWER_H__
