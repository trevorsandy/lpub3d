#ifndef __TRESHAPEGROUP_H__
#define __TRESHAPEGROUP_H__

#include <TCFoundation/TCObject.h>
#include <TCFoundation/TCTypedValueArray.h>
#include <TCFoundation/TCTypedObjectArray.h>
#include <TCFoundation/TCVector.h>
#include <TCFoundation/TCStlIncludes.h>
#include <TRE/TREGL.h>

typedef void (TCObject::*TREScanPointCallback)(const TCVector &point);

typedef TCTypedObjectArray<TCULongArray> TCULongArrayArray;
typedef std::vector<GLboolean> GLbooleanVector;
typedef std::vector<int> IntVector;

struct TREVertex;
class TREVertexStore;
class TREMainModel;
class TREModel;
class TREVertexArray;
class TCVector;

typedef enum TREShapeType
{
	TRESLine			= 0x0001,
	TRESFirst			= TRESLine,
	TRESConditionalLine	= 0x0002,
	TRESTriangle		= 0x0004,
	TRESQuad			= 0x0008,
	TRESTriangleStrip	= 0x0010,
	TRESFirstStrip		= TRESTriangleStrip,
	TRESQuadStrip		= 0x0020,
	TRESTriangleFan		= 0x0040,
	TRESLast			= TRESTriangleFan
} TREShapeType;

typedef std::map<TREShapeType, IntVector> ShapeTypeIntVectorMap;

class TREShapeGroup : public TCObject
{
public:
	enum TRESTransferType
	{
		TTTransparent,
		TTTexmapped,
	};
	TREShapeGroup(void);
	TREShapeGroup(const TREShapeGroup &other);
	virtual TCObject *copy(void) const;
	int addLine(const TCVector *vertices);
	int addConditionalLine(const TCVector *vertices,
		const TCVector *controlPoints);
	int addTriangle(const TCVector *vertices);
	int addTriangle(const TCVector *vertices, const TCVector *normals);
	int addTriangle(const TCVector *vertices, const TCVector *normals,
		const TCVector *textureCoords);
	int addQuad(const TCVector *vertices);
	int addQuad(const TCVector *vertices, const TCVector *normals);
	int addTriangleStrip(const TCVector *vertices,
		const TCVector *normals, int count);
	int addQuadStrip(const TCVector *vertices, const TCVector *normals,
		int count);
	int addTriangleFan(const TCVector *vertices,
		const TCVector *normals, int count);
	int addTriangleFan(const TCVector *vertices,
		const TCVector *normals, const TCVector *textureCoords, int count);
	TCULongArray *getIndices(TREShapeType shapeType,
		bool create = false);
	virtual TCULongArray *getControlPointIndices(bool create = false);
	virtual TCULongArray *getStripCounts(TREShapeType shapeType,
		bool create = false);
    void draw(bool skipTexmapped = false);
	virtual void drawTextured(void);
	virtual void drawLines(void);
	virtual void drawConditionalLines(void);
	virtual void drawConditionalLines(const TCULongArray *activeIndices);
	virtual void setVertexStore(TREVertexStore *vertexStore);
	virtual TREVertexStore *getVertexStore(void) { return m_vertexStore; }
	virtual void scanPoints(TCObject *scanner,
		TREScanPointCallback scanPointCallback, const TCFloat *matrix);
	virtual void unshrinkNormals(const TCFloat *matrix,
		const TCFloat *unshrinkMatrix);
	virtual void unMirror(void);
	virtual void invert(void);
	virtual void transfer(TRESTransferType type, TCULong color,
		const TCFloat *matrix);
	virtual void flatten(TREShapeGroup *srcShapes, const TCFloat *matrix,
		TCULong color, bool colorSet, bool skipTexmapped = false);
	void setModel(TREModel *value);
	TREMainModel *getMainModel(void) { return m_mainModel; }
	virtual TCULongArray *getActiveConditionalIndices(TCULongArray *indices,
		const TCFloat *modelMatrix = NULL, int start = 0, int count = -1);
	virtual void nextStep(void);
	virtual void updateConditionalsStepCount(int step);
	virtual int getIndexCount(TREShapeType shapeType);
	void setBfc(bool value) { m_bfc = value; }
	bool getBfc(void) const { return m_bfc; }
	virtual void drawShapeType(TREShapeType shapeType, int offset = 0,
		int count = -1);
	virtual void cleanupTransfer(void);

	static GLenum modeForShapeType(TREShapeType shapeType);
	static int numPointsForShapeType(TREShapeType shapeType);
	static bool isTransparent(TCULong color, bool hostFormat);
	static void transformVertex(TREVertex &vertex, const TCFloat *matrix);
	static void transformNormal(TREVertex &normal, const TCFloat *matrix);

	static void transformPoint(const TCVector &point, const TCFloat *matrix,
		TCFloat *tx, TCFloat *ty);
	static int turnVector(TCFloat vx1, TCFloat vy1, TCFloat vx2, TCFloat vy2);
	static bool isTexmappedShapeType(TREShapeType shapeType);
protected:
	virtual ~TREShapeGroup(void);
	virtual void dealloc(void);
	virtual TCULong getShapeTypeIndex(TREShapeType shapeType);
	int addShape(TREShapeType shapeType, const TCVector *vertices,
		int count);
	int addShape(TREShapeType shapeType, const TCVector *vertices,
		const TCVector *normals, int count);
	int addShape(TREShapeType shapeType, const TCVector *vertices,
		const TCVector *normals, const TCVector *textureCoords, int count);
	void addShapeIndices(TREShapeType shapeType, int firstIndex,
		int count);
	void addIndices(TCULongArray *indices, int firstIndex, int count);
	void addShapeStripCount(TREShapeType shapeType, int count);
	void addShapeType(TREShapeType shapeType, int index);
	virtual void drawNormals(TCULongArray *indexArray, int count,
		int offset = 0);
	virtual void drawStripShapeType(TREShapeType shapeType);
	int addStrip(TREShapeType shapeType, const TCVector *vertices,
		const TCVector *normals, int count);
	int addStrip(TREShapeType shapeType, const TCVector *vertices,
		const TCVector *normals, const TCVector *textureCoords, int count);
	virtual void initMultiDrawIndices(void);
	virtual void deleteMultiDrawIndices(void);
	virtual void invertShapes(TCULongArray *oldIndices,
		TCULongArray *newIndices);
	virtual int flipNormal(int index);
	virtual void transferTriangle(TRESTransferType type, TCULong color,
		TCULong index0, TCULong index1, TCULong index2, const TCFloat *matrix);
	virtual void transferQuadStrip(TRESTransferType type, int shapeTypeIndex,
		TCULong color, int offset, int stripCount, const TCFloat *matrix);
	virtual void transferTriangleStrip(TRESTransferType type, int shapeTypeIndex,
		TCULong color, int offset, int stripCount, const TCFloat *matrix);
	virtual void transferTriangleFan(TRESTransferType type, int shapeTypeIndex,
		TCULong color, int offset, int stripCount, const TCFloat *matrix);
	virtual void transfer(TRESTransferType type, TCULong color,
		TREShapeType shapeType, TCULongArray *indices,
		TCULongArray *transferIndices, const TCFloat *matrix);
	virtual bool shouldDrawConditional(TCULong index1, TCULong index2,
		TCULong cpIndex1, TCULong cpIndex2, const TCFloat *matrix);
	virtual bool isColored(void) { return false; }
	virtual void recordTransfer(TCULongArray *transferIndices, int index,
		int shapeSize);

	virtual void scanPoints(TCULong index, TCObject *scanner,
		TREScanPointCallback scanPointCallback, const TCFloat *matrix);
	virtual void scanPoints(const TREVertex &vertex, TCObject *scanner,
		TREScanPointCallback scanPointCallback);
	virtual void scanPoints(TCULongArray *indices, TCObject *scanner,
		TREScanPointCallback scanPointCallback, const TCFloat *matrix);
	virtual void scanStripPoints(TCULongArray *indices,
		TCULongArray *stripCounts, TCObject *scanner,
		TREScanPointCallback scanPointCallback, const TCFloat *matrix);
	virtual void flattenShapes(TREVertexArray *dstVertices,
		TREVertexArray *dstNormals,
		TREVertexArray *dstTextureCoords,
		TCULongArray *dstColors,
		TCULongArray *dstIndices,
		TCULongArray *dstCPIndices,
		GLbooleanVector &dstEdgeFlags,
		TREVertexArray *srcVertices,
		TREVertexArray *srcNormals,
		TREVertexArray *srcTextureCoords,
		TCULongArray *srcColors,
		TCULongArray *srcIndices,
		TCULongArray *srcCPIndices,
		GLbooleanVector &srcEdgeFlags,
		const TCFloat *matrix,
		TCULong color,
		bool colorSet);
	virtual void flattenStrips(TREVertexArray *dstVertices,
		TREVertexArray *dstNormals,
		TREVertexArray *dstTextureCoords,
		TCULongArray *dstColors,
		TCULongArray *dstIndices,
		TCULongArray *dstStripCounts,
		TREVertexArray *srcVertices,
		TREVertexArray *srcNormals,
		TREVertexArray *srcTextureCoords,
		TCULongArray *srcColors,
		TCULongArray *srcIndices,
		TCULongArray *srcStripCounts,
		const TCFloat *matrix,
		TCULong color,
		bool colorSet);
	virtual void mirrorTextureCoords(TCULongArray *indices);

	virtual void unshrinkNormal(TCULong index, const TCFloat *matrix,
		const TCFloat *unshrinkMatrix);
	virtual void unshrinkNormals(TCULongArray *indices, const TCFloat *matrix,
		const TCFloat *unshrinkMatrix);
	virtual void unshrinkStripNormals(TCULongArray *indices,
		TCULongArray *stripCounts, const TCFloat *matrix,
		const TCFloat *unshrinkMatrix);
	virtual void nextStep(TREShapeType shapeType);
	virtual bool shouldTransferIndex(TRESTransferType type,
		TREShapeType shapeType, TCULong color, int index,
		const TCFloat *matrix);
	bool shouldTransferIndex(TRESTransferType type, TREShapeType shapeType,
		TCULong color, int index, bool colored, const TCFloat *matrix);
	virtual TCULongArray *getTransferIndices(TRESTransferType type,
		TREShapeType shapeType);
	virtual bool shouldGetTransferIndices(TRESTransferType type);

	TREVertexStore *m_vertexStore;
	TCULongArrayArray *m_indices;
	TCULongArray *m_controlPointIndices;
	TCULongArrayArray *m_stripCounts;
	TCULong ***m_multiDrawIndices;
	TCULong m_shapesPresent;
	TREMainModel *m_mainModel;
	TREModel *m_model;
	ShapeTypeIntVectorMap m_stepCounts;
	bool m_bfc;
	TCULongArrayArray *m_transferIndices;
};

#endif // __TRESHAPEGROUP_H__
