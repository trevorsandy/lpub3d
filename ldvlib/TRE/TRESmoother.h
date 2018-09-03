#ifndef __TRESMOOTHER_H__
#define __TRESMOOTHER_H__

#include <TRE/TREVertexArray.h>
#include <TCFoundation/TCVector.h>
#include <TRE/TREShapeGroup.h>
#include <vector>

class TCVector;

class TRESmoother
{
public:
	TRESmoother(void);
	TRESmoother(const TREVertex &vertex);
	TRESmoother(const TRESmoother &other);
	~TRESmoother(void);
	void setStartVertex(const TREVertex &vertex) { m_startVertex = vertex; }
	const TREVertex &getStartVertex(void) { return m_startVertex; }
	void addVertex(const TREVertex &vertex);
	int getVertexCount(void) { return m_vertices->getCount(); }
	void markShared(int index0, int index1);
	const TREVertex &getVertex(int index) const
	{
		return m_vertices->vertexAtIndex(index);
	}
	TCVector &getNormal(int index);
	TRESmoother &operator=(const TRESmoother &other);
	void finish(void);
	static bool shouldFlipNormal(const TCVector &normal1,
		const TCVector &normal2);
protected:
	TREVertex m_startVertex;
	TREVertexArray *m_vertices;
	std::vector<TCVector> m_normals;
	TCULongArrayArray *m_sharedList;
};

#endif // __TRESMOOTHER_H__
