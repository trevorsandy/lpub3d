#ifndef __TREMODEL_H__
#define __TREMODEL_H__

#include <TCFoundation/TCAlertSender.h>
#include <TCFoundation/TCTypedObjectArray.h>
#include <TRE/TREShapeGroup.h>
#include <TRE/TREColoredShapeGroup.h>
#include <TRE/TREVertexKey.h>
#include <TRE/TRESmoother.h>
#include <TCFoundation/TCVector.h>
#include <TCFoundation/TCStlIncludes.h>

struct TREVertex;
class TRESubModel;
class TREMainModel;
class TREColoredShapeGroup;
class TREVertexArray;
class TCImage;

class TRENormalInfo : public TCAlertSender
{
public:
	TRENormalInfo(void)
		:m_normals(NULL),
		m_normalIndex(-1),
		m_smoother(NULL),
		m_smootherIndex(-1) {}

	TREVertexArray *m_normals;
	int m_normalIndex;
	TRESmoother *m_smoother;
	int m_smootherIndex;
};

typedef TCTypedObjectArray<TRESubModel> TRESubModelArray;
typedef TCTypedObjectArray<TREShapeGroup> TREShapeGroupArray;
typedef TCTypedObjectArray<TREColoredShapeGroup> TREColoredShapeGroupArray;
typedef TCTypedObjectArray<TRENormalInfo> TRENormalInfoArray;
typedef std::map<TREVertexKey, TRESmoother> TREConditionalMap;
typedef std::vector<int> IntVector;
typedef std::set<int> IntSet;

typedef enum
{
	TREMStandard,
	TREMFirst = TREMStandard,
	TREMLines,
	TREMEdgeLines,
	TREMConditionalLines,
	TREMStud,
	TREMBFC,
	TREMStudBFC,
	TREMTransparent,
	TREMLast = TREMTransparent
} TREMSection;

class TREModel : public TCAlertSender
{
public:
	enum TexmapType
	{
		TTPlanar,
		TTCylindrical,
		TTSpherical,
	};
	struct TexmapInfo
	{
		struct GeomSubInfo
		{
			IntSet triangles;
			IntSet quads;
			IntSet triStrips;
			IntSet quadStrips;
		};
		struct GeomInfo
		{
			GeomSubInfo standard;
			GeomSubInfo colored;
		};
		TexmapInfo(void);
		TexmapInfo(TexmapType type, const std::string &filename,
			const TCVector *otherPoints, const TCFloat *extra);
		bool texmapEquals(const TexmapInfo &other);
		void copyPoints(const TCVector *otherPoints);
		void transform(const TCFloat* matrix);
		void calcCylFields(void);
		void calcSphereFields(void);
		TCVector cylDirectionFrom(const TCVector& point);
		TCVector directionFrom(const TCVector& point, const TCVector& norm);
		TCFloat distanceToPlane(const TCVector& point,
			const TCVector& planePoint, const TCVector& planeNormal);
		void calcTextureCoords(const TCVector* ppoints,
			TCVector* textureCoords);
		TCFloat calcSAngle(const TCVector& point, bool isFirst,
			TCVector& baseDir, TCFloat& baseAngle);
		void calcCylTextureCoords(const TCVector* ppoints,
			TCVector* textureCoords);
		void calcSphereTextureCoords(const TCVector* ppoints,
			TCVector* textureCoords);
		TexmapType type;
		std::string filename;
		TCVector points[3];
		TCVector normal;
		TCVector normal2;
		TCVector dir;
		TCVector a;
		TCFloat cylHeight;
		TCFloat sAngle;
		TCFloat tAngle;
		bool sAngleIs360;
		GeomInfo standard;
		GeomInfo bfc;
		GeomInfo transparent;
		int subModelOffset;
		int subModelCount;
	};
	typedef std::list<TexmapInfo> TexmapInfoList;

	TREModel(void);
	TREModel(const TREModel &other);
	TREModel(const TREModel &other, bool shallow);
	virtual TCObject *copy(void) const;
	virtual TREModel *shallowCopy(void);
	virtual void setMainModel(TREMainModel *mainModel)
	{
		m_mainModel = mainModel;
	}
	virtual TREMainModel *getMainModel(void) const { return m_mainModel; }
	virtual void setName(const char *name);
	virtual const char *getName(void) const { return m_name; }
	virtual TRESubModel *addSubModel(const TCFloat *matrix, TREModel *model,
		bool invert);
	virtual TRESubModel *addSubModel(TCULong color, TCULong edgeColor,
		const TCFloat *matrix, TREModel *model, bool invert);
	virtual void addLine(const TCVector *vertices);
	virtual void addLine(TCULong color, const TCVector *vertices);
	virtual void addEdgeLine(const TCVector *vertices, TCULong color = 0);
	virtual void addConditionalLine(const TCVector *vertices,
		const TCVector *controlPoints, TCULong color = 0);
	virtual void addConditionalLine(const TCVector &p1, const TCVector &p2,
		const TCVector &c1, const TCVector &c2);
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
	void compile(TREMSection section, bool colored,
		bool nonUniform = false, bool skipTexmapped = false);
	void draw(TREMSection section);
	void draw(TREMSection section, bool colored,
		bool subModelsOnly = false, bool nonUniform = false,
		bool skipTexmapped = false);
	virtual void drawColored(TREMSection section);
	virtual void setPartFlag(bool value) { m_flags.part = value; }
	virtual bool isPart(void) { return m_flags.part != false; }
	virtual void setNoShrinkFlag(bool value) { m_flags.noShrink = value; }
	virtual bool getNoShrinkFlag(void) { return m_flags.noShrink != false; }
	virtual bool isFlattened(void) { return m_flags.flattened != false; }
	virtual void flatten(void);
	virtual void smooth(void);
	virtual void addCylinder(const TCVector &center, TCFloat radius,
		TCFloat height, int numSegments, int usedSegments = -1,
		bool bfc = false, TCULong color = 0, TCULong edgeColor = 0);
	virtual void addSlopedCylinder(const TCVector &center, TCFloat radius,
		TCFloat height, int numSegments, int usedSegments = -1, bool bfc = false);
	virtual void addSlopedCylinder2(const TCVector &center, TCFloat radius,
		TCFloat height, int numSegments, int usedSegments = -1, bool bfc = false);
	virtual void addStudDisc(const TCVector &center, TCFloat radius,
		int numSegments, int usedSegments = -1, bool bfc = false);
	virtual void addChrd(const TCVector &center, TCFloat radius, int numSegments,
		int usedSegments = -1, bool bfc = false);
	virtual void addDisc(const TCVector &center, TCFloat radius, int numSegments,
		int usedSegments = -1, bool bfc = false, bool stud = false);
	virtual void addNotDisc(const TCVector &center, TCFloat radius,
		int numSegments, int usedSegments = -1, bool bfc = false);
	virtual void addTangent(const TCVector &center, TCFloat radius,
		int numSegments, int usedSegments = -1, bool bfc = false);
	virtual void addCone(const TCVector &center, TCFloat radius, TCFloat height,
		int numSegments, int usedSegments = -1, bool bfc = false,
		TCULong color = 0, TCULong edgeColor = 0);
	virtual void addEighthSphere(const TCVector& center, TCFloat radius,
		int numSegments, bool bfc);
	virtual void addTorusIO(bool inner, const TCVector& center, TCFloat yRadius,
		TCFloat xzRadius, int numSegments, int usedSegments, int minorSegments,
		bool bfc);
	virtual void addOpenCone(const TCVector &center, TCFloat radius1,
		TCFloat radius2, TCFloat height, int numSegments, int usedSegments = -1,
		bool bfc = false, TCULong color = 0, TCULong edgeColor = 0);
	virtual void addCircularEdge(const TCVector &center, TCFloat radius,
		int numSegments, int usedSegments = -1, TCULong color = 0);
	virtual void addRing(const TCVector &center, TCFloat radius1, TCFloat radius2,
		int numSegments, int usedSegments = -1, bool bfc = false);
	virtual void addOpenConeConditionals(TCVector *points, int numSegments,
		int usedSegments, TCULong color = 0);
	virtual void addSlopedCylinder2Conditionals(TCVector *points,
		int numSegments, int usedSegments);
	virtual void addTorusIOConditionals(bool innder, TCVector *points,
		int numSegments, int usedSegments, int minorSegments,
		const TCVector& center, TCFloat radius, TCFloat height);
	virtual void addEighthSphereConditionals(TCVector *points, int numSegments);
	TCVector calcIntersection(int i, int j, int num, TCVector* zeroXPoints,
		TCVector* zeroYPoints, TCVector* zeroZPoints);
	virtual void getBoundingBox(TCVector& min, TCVector& max);
	virtual void scanPoints(TCObject *scanner,
		TREScanPointCallback scanPointCallback, const TCFloat *matrix);
	virtual void unshrinkNormals(const TCFloat *scaleMatrix);
	void unshrinkNormals(const TCFloat *matrix, const TCFloat *unshrinkMatrix);
	TREModel *getUnMirroredModel(void);
	TREModel *getInvertedModel(void);
	virtual void uncompile(bool includeSubModels = true);
	virtual void nextStep(void);
	virtual int getCurStepIndex(void) const { return m_curStepIndex; }
	bool isLineSection(int section)
	{
		return isLineSection((TREMSection)section);
	}
	bool isLineSection(TREMSection section)
	{
		return section == TREMLines || section == TREMEdgeLines;
	}
	virtual void transferColored(TREShapeGroup::TRESTransferType type,
		TREMSection section, const TCFloat *matrix);
	virtual void transferColoredSubModels(TREShapeGroup::TRESTransferType type,
		TREMSection section, const TCFloat *matrix);
	virtual void transfer(TREShapeGroup::TRESTransferType type, TCULong color,
		TREMSection section, const TCFloat *matrix);
	virtual void transferSubModels(TREShapeGroup::TRESTransferType type,
		TCULong color, TREMSection section, const TCFloat *matrix);
	virtual void cleanupTransfer(TREShapeGroup::TRESTransferType type,
		TREMSection section);
	virtual TCObject *getAlertSender(void);
	virtual void saveSTL(FILE *file, float scale);
	virtual void startTexture(int type, const std::string &filename,
		TCImage *image, const TCVector *points, const TCFloat *extra);
	virtual bool endTexture(void);
	virtual void finishPart(void);
	virtual void finishParts(void);
	virtual void shrinkParts(void);

	TREShapeGroup **getShapes(void) { return m_shapes; }
	TREColoredShapeGroup **getColoredShapes(void) { return m_coloredShapes; }
	TREShapeGroup *getShape(int index) { return m_shapes[index]; }
	TREColoredShapeGroup *getColoredShape(int index)
	{
		return m_coloredShapes[index];
	}
	TRESubModelArray *getSubModels(void) { return m_subModels; }
	int getSubModelCount(void) const;
	void activateTexmap(const TexmapInfo &texmapInfo);
	void disableTexmaps(void);
	TexmapInfo *getActiveTexmapInfo(void);
	const TexmapInfoList &getTexmapInfos(void) const { return m_texmapInfos; }
protected:
	struct TRETriangle
	{
		int index;
		TREVertex *points[3];
	};
	typedef std::list<TRETriangle> TRETriangleList;
	typedef std::map<TREVertexKey, TRETriangleList> TRETrianglesMap;

	virtual ~TREModel(void);
	virtual void dealloc(void);
	virtual void setup(TREMSection section);
	virtual void setupLines(void);
	virtual void setupStandard(void);
	virtual void setupStud(void);
	virtual void setupStudBFC(void);
	virtual void setupBFC(void);
	virtual void setupColored(TREMSection section);
	virtual void setupColored(void);
	virtual void setupColoredStud(void);
	virtual void setupColoredStudBFC(void);
	virtual void setupColoredLines(void);
	virtual void setupColoredBFC(void);
	virtual void setupEdges(void);
	virtual void setupColoredEdges(void);
	virtual void setupConditional(void);
	virtual void setupColoredConditional(void);
	virtual void flatten(TREModel *model, const TCFloat *matrix, TCULong color,
		bool colorSet, TCULong edgeColor, bool edgeColorSet,
		bool includeShapes, bool skipTexmapped = false);
	virtual void checkGLError(char *msg);
	void setCirclePoint(TCFloat angle, TCFloat radius, const TCVector& center,
		TCVector& point);
	void scanBoundingBoxPoint(const TCVector &point);
	virtual void calculateBoundingBox(void);
	virtual void quadStripToQuad(int index, const TCVector *stripVertices,
		const TCVector *stripNormals, TCVector *quadVertices,
		TCVector *quadNormals);
	virtual void triangleStripToTriangle(int index,
		const TCVector *stripVertices, const TCVector *stripNormals,
		TCVector *triangleVertices, TCVector *triangleNormals);
	virtual void triangleFanToTriangle(int index, const TCVector *fanVertices,
		const TCVector *fanNormals, const TCVector *fanTextureCoords,
		TCVector *triangleVertices, TCVector *triangleNormals,
		TCVector *triangleTextureCoords);
	virtual void unMirror(TREModel *originalModel);
	virtual void invert(TREModel *originalModel);
	virtual bool checkShapeGroupPresent(TREShapeGroup *shapeGroup,
		TREMSection section, bool colored);
	virtual bool checkSectionPresent(TREMSection section);
	virtual bool checkColoredSectionPresent(TREMSection section);
	virtual bool checkSectionPresent(TREMSection section, bool colored);
	virtual int sphereIndex(int i, int j, int usedSegments);
	virtual void setSectionPresent(TREMSection section, bool colored);
	virtual bool isSectionPresent(TREMSection section, bool colored);
	virtual bool shouldLoadConditionalLines(void);
	virtual void genStudTextureCoords(TCVector *textureCoords, int vertexCount);
	virtual void fillConditionalMap(TREConditionalMap &conditionalMap);
	virtual void fillConditionalMap(TREConditionalMap &conditionalMap,
		TREShapeGroup *shapeGroup);
	virtual void addConditionalPoint(TREConditionalMap &conditionalMap,
		const TREVertexArray *vertices, int index0, int index1,
		const TREVertexKey &vertexKey);
	virtual void calcShapeNormals(TREConditionalMap &conditionalMap,
		TRENormalInfoArray *normalInfos, TREShapeType shapeType);
	virtual void calcShapeNormals(TREConditionalMap &conditionalMap,
		TRENormalInfoArray *normalInfos, TREMSection section,
		TREShapeType shapeType);
	virtual void calcShapeNormals(TREConditionalMap &conditionalMap,
		TRENormalInfoArray *normalInfos, TREShapeGroup *shapeGroup,
		TREShapeType shapeType);
	virtual void processSmoothEdge(TREConditionalMap &conditionalMap,
		TRENormalInfoArray *normalInfos, const TREVertexArray *vertices,
		TREVertexArray *normals, int index0, int index1, int index2);
	virtual int getConditionalLine(TREConditionalMap &conditionalMap,
		const TREVertex point0, const TREVertex point1, TRESmoother *&smoother);
	void applyShapeNormals(TRENormalInfoArray *normalInfos);
	void finishShapeNormals(TREConditionalMap &conditionalMap);
	void flattenNonUniform(void);
	void flattenConditionals(const TCFloat *matrix, TCULong edgeColor,
		bool edgeColorSet);
	void removeConditionals(void);
	void findLights(void);
	void findLights(float *matrix);
	void calcTangentControlPoint(TCVector &controlPoint, int index,
		int numSegments);
	void saveSTLShapes(TREShapeGroup *shapes[], FILE *file, const TCFloat *matrix,
		float scale);
	void saveSTL(FILE *file, const TCFloat *matrix, float scale);
	void scaleConditionalControlPoints(TREShapeGroup *shapeGroup);
	void scaleConditionalControlPoint(int index, int cpIndex,
		TREVertexArray *vertices);
	int getShapeCount(TREMSection section, TREShapeType shapeType,
		bool colored);
	GLuint *getListIDs(bool colored, bool skipTexmapped);

	static void uncompileListID(GLuint &listID);
	static void setGlNormalize(bool value);
	static void printStlTriangle(FILE *file, TREVertexArray *vertices,
		TCULongArray *indices, int ix, int i0, int i1, int i2,
		const TCFloat *matrix, float scale);
	static void printStlStrips(FILE *file, TREShapeGroup *shapeGroup,
		TREShapeType shapeType, const TCFloat *matrix, float scale);

	char *m_name;
	TREMainModel *m_mainModel;
	TRESubModelArray *m_subModels;
	TREShapeGroup *m_shapes[TREMLast + 1];
	TREColoredShapeGroup *m_coloredShapes[TREMLast + 1];
	GLuint m_listIDs[TREMLast + 1];
	GLuint m_coloredListIDs[TREMLast + 1];
	GLuint m_texListIDs[TREMLast + 1];
	GLuint m_texColoredListIDs[TREMLast + 1];
	TREModel *m_unMirroredModel;
	TREModel *m_invertedModel;
	TCULong m_sectionsPresent;
	TCULong m_coloredSectionsPresent;
	TCVector m_boundingMin;
	TCVector m_boundingMax;
	int m_curStepIndex;
	IntVector m_stepCounts;
	TexmapInfoList m_texmapInfos;
	struct
	{
		bool part:1;
		bool noShrink:1;
		bool boundingBox:1;
		bool unshrunkNormals:1;
		bool unMirrored:1;
		bool inverted:1;
		bool flattened:1;
	} m_flags;
};

#endif // __TREMODEL_H__
