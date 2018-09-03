#ifndef __TREMAINMODEL_H__
#define __TREMAINMODEL_H__

#include <TRE/TREModel.h>
#include <TCFoundation/TCImage.h>
#include <TCFoundation/TCStlIncludes.h>

#ifdef USE_CPP11
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#else
#if defined(_MSC_VER) && _MSC_VER <= 1200	// VS 6
#define _NO_TRE_THREADS
#else  // VS 6
#ifdef _NO_BOOST
#define _NO_TRE_THREADS
#endif // _NO_BOOST
#endif // VS 6
#ifndef _NO_TRE_THREADS
#ifdef WIN32
#pragma warning(push)
#pragma warning(disable:4244 4512)
#endif // WIN32
#ifndef _NO_BOOST
//#include <boost/thread.hpp>
//#include <boost/thread/condition.hpp>
#endif // _NO_BOOST
#ifdef WIN32
#pragma warning(pop)
#endif // WIN32
#endif // !_NO_TRE_THREADS
#endif

class TCDictionary;
class TREVertexStore;
class TREColoredShapeGroup;
class TRETransShapeGroup;
class TRETexmappedShapeGroup;
class TCImage;

extern const GLfloat POLYGON_OFFSET_FACTOR;
extern const GLfloat POLYGON_OFFSET_UNITS;

typedef std::list<TCVector> TCVectorList;
typedef std::list<TCULong> TCULongList;
typedef std::list<TREMSection> SectionList;
typedef std::list<std::string> StringList;

class TREMainModel : public TREModel
{
public:
	TREMainModel(void);
	//TREMainModel(const TREMainModel &other);
	virtual TCObject *copy(void) const;
	virtual TCDictionary* getLoadedModels(bool bfc);
	void draw(void);
	virtual TREVertexStore *getVertexStore(void) { return m_vertexStore; }
	virtual TREVertexStore *getStudVertexStore(void)
	{
		return m_studVertexStore;
	}
	virtual TREVertexStore *getColoredStudVertexStore(void)
	{
		return m_coloredStudVertexStore;
	}
	virtual TREModel *modelNamed(const char *name, bool bfc);
	virtual void registerModel(TREModel *model, bool bfc);
	void setCompilePartsFlag(bool value) { m_mainFlags.compileParts = value; }
	bool getCompilePartsFlag(void) const
	{
		return m_mainFlags.compileParts != false;
	}
	void setCompileAllFlag(bool value) { m_mainFlags.compileAll = value; }
	bool getCompileAllFlag(void) const
	{
		return m_mainFlags.compileAll != false;
	}
	void setFlattenConditionalsFlag(bool value)
	{
		m_mainFlags.flattenConditionals = value;
	}
	bool getFlattenConditionalsFlag(void) const
	{
		return m_mainFlags.flattenConditionals != false;
	}
	void setEdgeLinesFlag(bool value) { m_mainFlags.edgeLines = value; }
	bool getEdgeLinesFlag(void) { return m_mainFlags.edgeLines != false; }
	void setEdgesOnlyFlag(bool value) { m_mainFlags.edgesOnly = value; }
	bool getEdgesOnlyFlag(void)
	{
		return m_mainFlags.edgesOnly != false && getEdgeLinesFlag();
	}
	void setTwoSidedLightingFlag(bool value);
	bool getTwoSidedLightingFlag(void)
	{
		return m_mainFlags.twoSidedLighting != false;
	}
	void setLightingFlag(bool value);
	bool getLightingFlag(void) { return m_mainFlags.lighting != false; }
	void setUseStripsFlag(bool value) { m_mainFlags.useStrips = value; }
	bool getUseStripsFlag(void) { return m_mainFlags.useStrips != false; }
	void setDisableStrips(bool value) { m_mainFlags.disableStrips = value; }
	bool getUseTriFansFlag(void)
	{
		return m_mainFlags.useStrips && !m_mainFlags.disableStrips &&
			m_mainFlags.useTriFans;
	}
	bool getUseTriStripsFlag(void)
	{
		return m_mainFlags.useStrips && !m_mainFlags.disableStrips &&
			m_mainFlags.useTriStrips;
	}
	bool getUseQuadStripsFlag(void)
	{
		return m_mainFlags.useStrips && !m_mainFlags.disableStrips &&
			m_mainFlags.useQuadStrips;
	}
	void setUseFlatStripsFlag(bool value) { m_mainFlags.useFlatStrips = value; }
	bool getUseFlatStripsFlag(void)
	{
		return m_mainFlags.useFlatStrips != false;
	}
	void setBFCFlag(bool value) { m_mainFlags.bfc = value; }
	bool getBFCFlag(void) { return m_mainFlags.bfc != false; }
	void setRedBackFacesFlag(bool value) { m_mainFlags.redBackFaces = value; }
	bool getRedBackFacesFlag(void) { return m_mainFlags.redBackFaces != false; }
	void setGreenFrontFacesFlag(bool value)
	{
		m_mainFlags.greenFrontFaces = value;
	}
	bool getGreenFrontFacesFlag(void)
	{
		return m_mainFlags.greenFrontFaces != false;
	}
	void setBlueNeutralFacesFlag(bool value)
	{
		m_mainFlags.blueNeutralFaces = value;
	}
	bool getBlueNeutralFacesFlag(void)
	{
		return m_mainFlags.blueNeutralFaces != false &&
			m_mainFlags.bfc != false;
	}
	void setDrawNormalsFlag(bool value) { m_mainFlags.drawNormals = value; }
	bool getDrawNormalsFlag(void) { return m_mainFlags.drawNormals != false; }
	void setStencilConditionalsFlag(bool value)
	{
		m_mainFlags.stencilConditionals = value;
	}
	bool getStencilConditionalsFlag(void);
	void setVertexArrayEdgeFlagsFlag(bool value)
	{
		m_mainFlags.vertexArrayEdgeFlags = value;
	}
	bool getVertexArrayEdgeFlagsFlag(void)
	{
		return m_mainFlags.vertexArrayEdgeFlags != false;
	}
	void setMultiThreadedFlag(bool value) { m_mainFlags.multiThreaded = value; }
	bool getMultiThreadedFlag(void) const
	{
		return m_mainFlags.multiThreaded != false;
	}
	void setSaveAlphaFlag(bool value) { m_mainFlags.saveAlpha = value; }
	bool getSaveAlphaFlag(void) { return m_mainFlags.saveAlpha != false; }
	void setGl2psFlag(bool value) { m_mainFlags.gl2ps = value; }
	bool getGl2psFlag(void) const { return m_mainFlags.gl2ps != false; }
	void setSendProgressFlag(bool value) { m_mainFlags.sendProgress = value; }
	bool getSendProgressFlag(void) const
	{
		return m_mainFlags.sendProgress != false;
	}
	void setLineJoinsFlag(bool value) { m_mainFlags.lineJoins = value; }
	bool getLineJoinsFlag(void) { return m_mainFlags.lineJoins != false; }
	bool getActiveLineJoinsFlag(void)
	{
		return m_mainFlags.activeLineJoins != false;
	}
	void setAALinesFlag(bool value) { m_mainFlags.aaLines = value; }
	bool getAALinesFlag(void) { return m_mainFlags.aaLines != false; }
	void setSortTransparentFlag(bool value)
	{
		m_mainFlags.sortTransparent = value;
	}
	bool getSortTransparentFlag(void)
	{
		return m_mainFlags.sortTransparent != false;
	}
	void setStippleFlag(bool value) { m_mainFlags.stipple = value; }
	bool getStippleFlag(void) { return m_mainFlags.stipple != false; }
	void setWireframeFlag(bool value) { m_mainFlags.wireframe = value; }
	bool getWireframeFlag(void) { return m_mainFlags.wireframe != false; }
	void setConditionalLinesFlag(bool value)
	{
		m_mainFlags.conditionalLines = value;
	}
	bool getConditionalLinesFlag(void)
	{
		return m_mainFlags.conditionalLines != false && getEdgeLinesFlag();
	}
	void setSmoothCurvesFlag(bool value) { m_mainFlags.smoothCurves = value; }
	bool getSmoothCurvesFlag(void) { return m_mainFlags.smoothCurves != false; }
	void setShowAllConditionalFlag(bool value);
	bool getShowAllConditionalFlag(void)
	{
		return m_mainFlags.showAllConditional != false &&
			getConditionalLinesFlag();
	}
	void setConditionalControlPointsFlag(bool value);
	bool getConditionalControlPointsFlag(void)
	{
		return m_mainFlags.conditionalControlPoints != false &&
			getConditionalLinesFlag();
	}
	void setPolygonOffsetFlag(bool value)
	{
		m_mainFlags.polygonOffset = value;
	}
	bool getPolygonOffsetFlag(void)
	{
		return m_mainFlags.polygonOffset != false;
	}
	void setStudLogoFlag(bool value) { m_mainFlags.studLogo = value; }
	bool getStudLogoFlag(void) { return m_mainFlags.studLogo != false; }
	void setRemovingHiddenLines(bool value)
	{
		m_mainFlags.removingHiddenLines = value;
	}
	bool getRemovingHiddenLines(void)
	{
		return m_mainFlags.removingHiddenLines != false;
	}
	void setCutawayDrawFlag(bool value) { m_mainFlags.cutawayDraw = value; }
	bool getCutawayDrawFlag(void) { return m_mainFlags.cutawayDraw != false; }
	void setEdgeLineWidth(GLfloat value) { m_edgeLineWidth = value; }
	TCFloat getEdgeLineWidth(void) { return m_edgeLineWidth; }
	void setStudAnisoLevel(GLfloat value) { m_studAnisoLevel = value; }
	TCFloat getStudAnisoLevel(void) { return m_studAnisoLevel; }
	void setStudTextureFilter(int value) { m_studTextureFilter = value; }
	int getStudTextureFilter(void) { return m_studTextureFilter; }
	virtual bool getCompiled(void) const
	{
		return m_mainFlags.compiled != false;
	}
	virtual bool getCompiling(void) { return m_mainFlags.compiling != false; }
	void setTextureOffsetFactor(TCFloat value)
	{
		m_textureOffsetFactor = value;
	}
	TCFloat getTextureOffsetFactor(void) const { return m_textureOffsetFactor; }
	virtual TCFloat getMaxRadiusSquared(const TCVector &center);
	virtual TCFloat getMaxRadius(const TCVector &center);
	TREVertexStore *getColoredVertexStore(void)
	{
		return m_coloredVertexStore;
	}
	TREVertexStore *getTransVertexStore(void)
	{
		return m_transVertexStore;
	}
	void setColor(TCULong color, TCULong edgeColor);
	TCULong getColor(void);
	TCULong getEdgeColor(void);
	bool postProcess(void);
	void compile(void);
	void recompile(void);
	virtual void addTransferTriangle(TREShapeGroup::TRESTransferType type,
		TCULong color, const TCVector vertices[], const TCVector normals[],
		bool bfc, const TCVector *textureCoords, const TCFloat *matrix);
	virtual bool shouldLoadConditionalLines(void);
	bool isStudSection(TREMSection section)
	{
		return section == TREMStud || section == TREMStudBFC;
	}
	virtual void openGlWillEnd(void);
	virtual void finish(void);
	virtual void addLight(const TCVector &location, TCULong color);
	virtual const TCVectorList &getLightLocations(void) const
	{
		return m_lightLocations;
	}
	virtual const TCULongList &getLightColors(void) const
	{
		return m_lightColors;
	}
	const TCFloat *getCurrentModelViewMatrix(void) const
	{
		return m_currentModelViewMatrix;
	}
	const TCFloat *getCurrentProjectionMatrix(void) const
	{
		return m_currentProjectionMatrix;
	}
	bool hasWorkerThreads(void);
	void waitForSort(void);
	void waitForConditionals(int step);
	const TCULongArray *getActiveConditionals(int step) const
	{
		return m_activeConditionals[step];
	}
	const TCULongArray *getActiveColorConditionals(int step) const
	{
		return m_activeColorConditionals[step];
	}
	bool doingBackgroundConditionals(void);
	virtual TCObject *getAlertSender(void) { return m_alertSender; }
	virtual void setAlertSender(TCObject *value) { m_alertSender = value; }
	int getStep(void) const { return m_step; }
	void setStep(int value);
	int getNumSteps(void) const { return m_numSteps; }
	void transferPrep(void);
	void updateModelTransferStep(int subModelIndex,
		bool isConditionals = false);
	bool onLastStep(void);
	virtual void nextStep(void);

	virtual void addLine(const TCVector *vertices);
	virtual void addLine(TCULong color, const TCVector *vertices);
	virtual void addEdgeLine(const TCVector *vertices, TCULong color = 0);
	virtual void addTriangle(const TCVector *vertices);
	virtual void addTriangle(const TCVector *vertices,
		const TCVector *normals);
	virtual void addTriangle(TCULong color, const TCVector *vertices);
	virtual void addTriangle(TCULong color, const TCVector *vertices,
		const TCVector *normals);
	virtual void addBFCTriangle(const TCVector *vertices);
	virtual void addBFCTriangle(const TCVector *vertices,
		const TCVector *normals);
	virtual void addBFCTriangle(TCULong color, const TCVector *vertices);
	virtual void addBFCTriangle(TCULong color, const TCVector *vertices,
		const TCVector *normals);
	virtual void addQuad(const TCVector *vertices);
	virtual void addQuad(TCULong color, const TCVector *vertices);
	virtual void addBFCQuad(const TCVector *vertices);
	virtual void addBFCQuad(TCULong color, const TCVector *vertices);
	virtual void addConditionalLine(const TCVector *vertices,
		const TCVector *controlPoints, TCULong color = 0);
	virtual void addTriangleStrip(const TCVector *vertices,
		const TCVector *normals, int count, bool flat = false);
	virtual void addTriangleStrip(TREShapeGroup *shapeGroup,
		const TCVector *vertices, const TCVector *normals, int count,
		bool flat = false);
	virtual void addBFCTriangleStrip(const TCVector *vertices,
		const TCVector *normals, int count, bool flat = false);
	virtual void addTriangleFan(const TCVector *vertices,
		const TCVector *normals, const TCVector *textureCoords, int count,
		bool flat = false);
	virtual void addTriangleFan(TCULong color, const TCVector *vertices,
		const TCVector *normals, int count, bool flat = false);
	virtual void addTriangleFan(TREShapeGroup *shapeGroup,
		const TCVector *vertices, const TCVector *normals,
		const TCVector *textureCoords, int count, bool flat = false);
	virtual void addTriangleFan(TREColoredShapeGroup *shapeGroup, TCULong color,
		const TCVector *vertices, const TCVector *normals, int count,
		bool flat = false);
	virtual void addBFCTriangleFan(const TCVector *vertices,
		const TCVector *normals, const TCVector *textureCoords, int count,
		bool flat = false);
	virtual void addBFCTriangleFan(TCULong color, const TCVector *vertices,
		const TCVector *normals, int count, bool flat = false);
	virtual void addQuadStrip(const TCVector *vertices, const TCVector *normals,
		int count, bool flat = false);
	virtual void addQuadStrip(TREShapeGroup *shapeGroup,
		const TCVector *vertices, const TCVector *normals, int count,
		bool flat);
	virtual void addQuadStrip(TCULong color, const TCVector *vertices,
		const TCVector *normals, int count, bool flat = false);
	virtual void addQuadStrip(TREColoredShapeGroup *shapeGroup, TCULong color,
		const TCVector *vertices, const TCVector *normals, int count,
		bool flat = false);
	virtual void addBFCQuadStrip(const TCVector *vertices,
		const TCVector *normals, int count, bool flat = false);
	virtual void addBFCQuadStrip(TCULong color, const TCVector *vertices,
		const TCVector *normals, int count, bool flat = false);
	void loadTexture(const std::string &filename, TCImage *image);
	void startTexture(const std::string &filename, TCImage *image);
	bool endTexture(void);
	const std::string *getActiveTextureFilename(void) const;
	GLuint getTexmapTextureID(const std::string &filename) const;
	const TCImage *getTexmapImage(const std::string &filename) const;
	virtual void startTexture(int type, const std::string &filename,
		TCImage *image, const TCVector *points, const TCFloat *extra);
	void setTransferTexmapInfo(const TexmapInfo &texmapInfo, bool bfc,
		const TCFloat *matrix);
	void setModelTexmapTransferFlag(bool value)
	{
		m_mainFlags.modelTexmapTransfer = value;
	}
	bool getModelTexmapTransferFlag(void) const
	{
		return m_mainFlags.modelTexmapTransfer != false;
	}
	void setFlattenPartsFlag(bool value) { m_mainFlags.flattenParts = value; }
	bool getFlattenPartsFlag(void) const
	{
		return m_mainFlags.flattenParts != false;
	}
	void setTexturesAfterTransparentFlag(bool value) { m_mainFlags.texturesAfterTransparent = value; }
	bool getTexturesAfterTransparentFlag(void) const
	{
		return m_mainFlags.texturesAfterTransparent != false;
	}
	void setSeamWidth(TCFloat value) { m_seamWidth = value; }
	TCFloat getSeamWidth(void) const { return m_seamWidth; }
	GLint getTexClampMode(void) const { return m_texClampMode; }

	static void loadStudTexture(const char *filename);
	static void setStudTextureData(TCByte *data, long length);
	static void setRawStudTextureData(TCByte *data, long length);
	static TCImageArray *getStudTextures(void) { return sm_studTextures; }
	static unsigned getStudTextureID(void) { return sm_studTextureID; }
protected:
	void populateTrianglesMap(TRETexmappedShapeGroup *shapeGroup,
		TRETrianglesMap &triangles);
	void transferSmoothNormals(const TRETrianglesMap triangles[]);
	void transferSmoothNormals(const TRETrianglesMap triangles[],
		TREModel *model, const TCFloat *matrix);
	void transferSmoothNormals(const TRETrianglesMap &triangles,
		TREShapeGroup *shapeGroup, const TCFloat *matrix);
	void transferSmoothNormals(const TRETrianglesMap &triangles,
		TREShapeGroup *shapeGroup, TREShapeType shapeType,
		int shapeSize, const TCFloat *matrix);
	bool transferSmoothNormals(
		const TRETrianglesMap &triangles,
		TREVertexStore *vertexStore,
		TCULongArray *indices,
		TREVertexStore *dstVertexStore,
		TCULongArray *dstIndices,
		int i0,
		int i1,
		int i2,
		const TCFloat *matrix);

	struct TexmapImageInfo
	{
		TexmapImageInfo(void) : image(NULL) {}
		TexmapImageInfo(const std::string &filename, TCImage *image)
			: filename(filename)
			, image(TCObject::retain(image))
			, textureID(0)
		{}
		TexmapImageInfo(const TexmapImageInfo &other)
			: filename(other.filename)
			, image(TCObject::retain(other.image))
			, textureID(other.textureID)
		{}
		TexmapImageInfo &operator=(const TexmapImageInfo &other)
		{
			filename = other.filename;
			image = TCObject::retain(other.image);
			textureID = other.textureID;
			return *this;
		}
		~TexmapImageInfo()
		{
			TCObject::release(image);
		}
		std::string filename;
		TCImage *image;
		GLuint textureID;
	};
	typedef std::map<std::string, TexmapImageInfo> TexmapImageInfoMap;
	virtual ~TREMainModel(void);
	virtual void dealloc(void);
	void scanMaxRadiusSquaredPoint(const TCVector &point);
	virtual void activateBFC(void);
	virtual void deactivateBFC(bool transparent = false);
	void transferTransparent(void);
	virtual void transferTransparent(const SectionList &sectionList);
	void transferTexmapped(void);
	virtual void transferTexmapped(const SectionList &sectionList);
	virtual void drawTransparent(int pass = -1);
	virtual void drawLines(int pass = -1);
	virtual void drawSolid(void);
	virtual void enableLineSmooth(int pass = -1);
	virtual void bindStudTexture(void);
	void bindTexmaps(void);
	void configTexmaps(void);
	void configTextureFilters(void);
	void deleteGLTexmaps(void);
	virtual void configureStudTexture(bool allowMipMap = true);
	virtual bool shouldCompileSection(TREMSection section);
	virtual void passOnePrep(void);
	virtual void passTwoPrep(void);
	virtual void passThreePrep(void);
#if defined(USE_CPP11) || !defined(_NO_TRE_THREADS)
	template <class _ScopedLock> bool workerThreadDoWork(_ScopedLock &lock);
	template <class _ScopedLock> void nextConditionalsStep(_ScopedLock &lock);
	void workerThreadProc(void);
#endif // USE_CPP11 || !_NO_TRE_THREADS
	void launchWorkerThreads(void);
	int getNumWorkerThreads(void);
	int getNumBackgroundTasks(void);
	void triggerWorkerThreads(void);
	bool backgroundSortNeeded(void);
	bool backgroundConditionalsNeeded(void);
	void flattenConditionals(void);
	void backgroundConditionals(int step);
	TCULongArray *backgroundConditionals(TREShapeGroup *shapes, int step);
	TREModel *getCurGeomModel(void);
	void drawTexmapped(bool transparent);
	void drawTexmappedInternal(bool texture, bool colorMaterialOff,
		bool transparent);

	void enable(GLenum cap);
	void disable(GLenum cap);
	void blendFunc(GLenum sfactor, GLenum dfactor);
	void lineWidth(GLfloat width);
	void pointSize(GLfloat size);

	static void loadStudMipTextures(TCImage *mainImage);

	TCObject *m_alertSender;
	TCDictionary *m_loadedModels;
	TCDictionary *m_loadedBFCModels;
	TREVertexStore *m_vertexStore;
	TREVertexStore *m_studVertexStore;
	TREVertexStore *m_coloredVertexStore;
	TREVertexStore *m_coloredStudVertexStore;
	TREVertexStore *m_transVertexStore;
	TREVertexStore *m_texmapVertexStore;
	TCULong m_color;
	TCULong m_edgeColor;
	TCFloat m_maxRadiusSquared;
	TCFloat m_textureOffsetFactor;
	TCVector m_center;
	GLfloat m_edgeLineWidth;
	GLfloat m_studAnisoLevel;
	bool m_abort;	// Easier not to be a bit field.  A pointer to it is passed
					// into other functions, and that doesn't work with a bit
					// field.
	GLint m_studTextureFilter;
	TCVectorList m_lightLocations;
	TCULongList m_lightColors;
	TCFloat m_currentModelViewMatrix[16];
	TCFloat m_currentProjectionMatrix[16];
	TCULong m_conditionalsDone;
	int m_conditionalsStep;
	TCULongArray *m_activeConditionals[32];
	TCULongArray *m_activeColorConditionals[32];
	int m_step;
	int m_numSteps;
	int m_transferStep;
	IntVector m_transStepCounts;
	IntVector m_texmappedStepCounts[3];
	TREModel *m_curGeomModel;
	TexmapImageInfoMap m_texmapImages;
	StringList m_activeTextures;
	TRETexmappedShapeGroup *m_texmappedShapes[3];
	TexmapInfoList m_mainTexmapInfos;
	GLint m_texClampMode;
	TCFloat m_seamWidth;
#if defined(USE_CPP11) || !defined(_NO_TRE_THREADS)
#ifdef USE_CPP11
    std::vector<std::thread> *m_threads;
	std::mutex *m_workerMutex;
	std::condition_variable *m_workerCondition;
	std::condition_variable *m_sortCondition;
	std::condition_variable *m_conditionalsCondition;
#else
//	boost::thread_group *m_threadGroup;
//	boost::mutex *m_workerMutex;
//	boost::condition *m_workerCondition;
//	boost::condition *m_sortCondition;
//	boost::condition *m_conditionalsCondition;
#endif
	bool m_exiting;
#endif // USE_CPP11 || !_NO_TRE_THREADS
	struct
	{
		// The following are temporal
		bool compiling:1;
		bool compiled:1;
		bool removingHiddenLines:1;	// This one is changed externally
		bool cutawayDraw:1;			// This one is changed externally
		bool activeLineJoins:1;
		bool frameSorted:1;
		bool frameSortStarted:1;
		bool frameStarted:1;
		// The following aren't temporal
		bool compileParts:1;
		bool compileAll:1;
		bool flattenConditionals:1;
		bool edgeLines:1;
		bool edgesOnly:1;
		bool twoSidedLighting:1;
		bool lighting:1;
		bool useStrips:1;
		bool disableStrips:1;
		bool useTriStrips:1;
		bool useTriFans:1;
		bool useQuadStrips:1;
		bool useFlatStrips:1;
		bool bfc:1;
		bool aaLines:1;
		bool sortTransparent:1;
		bool stipple:1;
		bool wireframe:1;
		bool conditionalLines:1;
		bool smoothCurves:1;
		bool showAllConditional:1;
		bool conditionalControlPoints:1;
		bool polygonOffset:1;
		bool studLogo:1;
		bool redBackFaces:1;
		bool greenFrontFaces:1;
		bool blueNeutralFaces:1;
		bool lineJoins:1;
		bool drawNormals:1;
		bool stencilConditionals:1;
		bool vertexArrayEdgeFlags:1;
		bool multiThreaded:1;
		bool saveAlpha:1;
		bool gl2ps:1;
		bool sendProgress:1;
		bool modelTexmapTransfer:1;
		bool flattenParts:1;
		bool texturesAfterTransparent:1;
	} m_mainFlags;

	static TCImageArray *sm_studTextures;
	static GLuint sm_studTextureID;
	static class TREMainModelCleanup
	{
	public:
		~TREMainModelCleanup(void);
	} sm_mainModelCleanup;
	friend class TREMainModelCleanup;
};

#endif // __TREMAINMODEL_H__
