#ifndef __LDSNAPSHOTTAKER_H__
#define __LDSNAPSHOTTAKER_H__

#include <TCFoundation/TCAlertSender.h>
#include <TCFoundation/TCStlIncludes.h>
#include <TCFoundation/TCImage.h>
#include <LDLib/LDrawModelViewer.h>

class LDrawModelViewer;
class LDConsoleAlertHandler;

class LDSnapshotTaker : public TCAlertSender
{
public:
	enum ImageType
	{
		ITFirst = 1,
		ITPng = 1,
		ITBmp = 2,
		ITJpg = 3,
		ITSvg = 4,
		ITEps = 5,
		ITPdf = 6,
		ITLast = 6
	};
	LDSnapshotTaker(void);
	LDSnapshotTaker(LDrawModelViewer *modelViewer);
	LDrawModelViewer *getModelViewer(void) { return m_modelViewer; }
	void setImageType(ImageType value) { m_imageType = value; }
	ImageType getImageType(void) const { return m_imageType; }
	void setTrySaveAlpha(bool value) { m_trySaveAlpha = value; }
	bool getTrySaveAlpha(void) const { return m_trySaveAlpha; }
	void setSaveZMap(bool value) { m_saveZMap = value; }
	bool getSaveZMap(void) const { return m_saveZMap; }
	void setAutoCrop(bool value) { m_autoCrop = value; }
	bool getAutoCrop(void) const { return m_autoCrop; }
	void setUseFBO(bool value);
	bool getUseFBO(void) const { return m_useFBO; }
	void set16BPC(bool value) { m_16BPC = value; }
	bool get16BPC(void) const { return m_16BPC; }
	int getFBOSize(void) const;
	void setProductVersion(const std::string &value)
	{
		m_productVersion = value;
	}
	const std::string &getProductVersion(void) const
	{
		return m_productVersion;
	}
	void setScaleFactor(TCFloat value) { m_scaleFactor = value; }
	TCFloat getScaleFactor(void) const { return m_scaleFactor; }
	void setRenderSize(int width, int height);
	bool hasRenderSize(void) const { return m_width != -1 && m_height != -1; }
	void calcTiling(int desiredWidth, int desiredHeight, int &bitmapWidth,
		int &bitmapHeight, int &numXTiles, int &numYTiles);

	bool saveImage(const char *filename, int imageWidth, int imageHeight,
		bool zoomToFit);
	bool saveImage(bool *tried = nullptr);
	bool exportFiles(bool *tried = nullptr);
	TCByte *grabImage(int &imageWidth, int &imageHeight, bool zoomToFit,
		TCByte *buffer, bool *saveAlpha, GLfloat *zBuffer = NULL);
	void cancel(void) { m_canceled = true;  }
	static void setShowConsoleAlerts(bool value) { sm_consoleAlerts = value; }
	static LDConsoleAlertHandler* getConsoleAlertHandler(void);

	static bool doCommandLine(bool doSnapshots = true, bool doExports = true,
		bool *tried = nullptr);
	static std::string removeStepSuffix(const std::string &filename,
		const std::string &stepSuffix);
	static std::string addStepSuffix(const std::string &filename,
		const std::string &stepSuffix, int step, int numSteps);
	static std::string extensionForType(ImageType type,
		bool includeDot = false);
	static ImageType typeForFilename(const char *filename, bool gl2psAllowed);
	static ImageType lastImageType(void);
	static LDrawModelViewer::ExportType exportTypeForFilename(
		const char* filename);

	static const char *alertClass(void) { return "LDSnapshotTaker"; }
protected:
	virtual ~LDSnapshotTaker(void);
	virtual void dealloc(void);
	bool writeJpg(const char *filename, int width, int height, TCByte *buffer);
	bool writeBmp(const char *filename, int width, int height, TCByte *buffer);
	bool writePng(const char *filename, int width, int height, TCByte *buffer,
		bool saveAlpha);
	bool writeZMap(const char *filename, int width, int height,
		TCFloat *zBuffer);
	bool writeZMap(FILE *zMapFile, int width, int height,
		TCFloat *zBuffer);
	bool writeImage(const char *filename, int width, int height, TCByte *buffer,
		const char *formatName, bool saveAlpha);
	bool canSaveAlpha(void);
	void renderOffscreenImage(void);
	bool imageProgressCallback(CUCSTR message, float progress);
	bool shouldZoomToFit(bool zoomToFit);
	void initModelViewer(void);
	void grabSetup(void);
	bool exportFile(const std::string& exportFilename, const char *modelPath,
		bool zoomToFit);
	bool saveStepImage(const char *filename, int imageWidth, int imageHeight,
		bool zoomToFit);
	bool saveGl2psStepImage(const char *filename, int imageWidth,
		int imageHeight, bool zoomToFit);
	int scale(int value) const { return (int)(value * m_scaleFactor); }
	int unscale(int value) const { return (int)(value / m_scaleFactor); }
	TCStringArray *getUnhandledCommandLineArgs(const char *listKey,
		bool &foundList);
	void updateModelFilename(const char *modelFilename);
	bool doCommandLine(bool doSnapshots, bool doExports,
		bool *tried, LDConsoleAlertHandler *consoleAlertHandler);
	void commandLineChanged(void);

	static void getViewportSize(int &width, int &height);
	static bool staticImageProgressCallback(CUCSTR message, float progress,
		void* userData);

	LDrawModelViewer *m_modelViewer;
	ImageType m_imageType;
	LDrawModelViewer::ExportType m_exportType;
	std::string m_productVersion;
	bool m_trySaveAlpha;
	bool m_saveZMap;
	bool m_autoCrop;
	bool m_fromCommandLine;
	bool m_commandLineSaveSteps;
	bool m_commandLineStep;
	int m_step;
	bool m_grabSetupDone;
	bool m_gl2psAllowed;
	bool m_useFBO;
	bool m_16BPC;
	bool m_canceled;
	int m_width;
	int m_height;
	int m_croppedX;
	int m_croppedY;
	int m_croppedWidth;
	int m_croppedHeight;
	TCFloat m_scaleFactor;
	std::string m_modelFilename;
	std::string m_currentImageFilename;
	std::set<std::string> m_commandLinesLists;
	static bool sm_consoleAlerts;
};

#endif // __LDSNAPSHOTTAKER_H__
