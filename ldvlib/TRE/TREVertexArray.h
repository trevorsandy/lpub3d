#ifndef __TREVERTEXARRAY_H__
#define __TREVERTEXARRAY_H__

#include <TCFoundation/TCObject.h>
#include <TCFoundation/TCMacros.h>

struct TREVertex
{
	TCFloat v[3];
	bool approxEquals(const TREVertex &right, TCFloat epsilon) const
	{
		return fEq2(v[0], right.v[0], epsilon) &&
			fEq2(v[1], right.v[1], epsilon) &&
			fEq2(v[2], right.v[2], epsilon);
	}
};


class TREVertexArray : public TCObject
{
public:
	explicit TREVertexArray(unsigned int allocated = 0);
	TREVertexArray(const TREVertexArray &other);
	virtual TCObject *copy(void) const;

	virtual bool addVertex(const TREVertex &vertex);
	virtual bool insertVertex(const TREVertex &vertex, unsigned int index = 0);
	virtual bool replaceVertex(const TREVertex &vertex, unsigned int index);
	virtual bool removeVertex(int index);
	virtual void removeAll(void);
	virtual TREVertex &vertexAtIndex(unsigned int index);
	virtual const TREVertex &vertexAtIndex(unsigned int index) const;
	virtual const TREVertex &operator[](unsigned int index) const;
	virtual TREVertex &operator[](unsigned int index);
	int getCount(void) const { return m_count; }
	virtual bool addEmptyValues(int count);
	virtual void shrinkToFit(void) { setCapacity(m_count); }
	virtual bool setCapacity(unsigned newCapacity, bool updateCount = false, bool clear = false);
//	virtual void sortUsingFunction(TCArraySortFunction function);
	TREVertex *getVertices(void) const { return m_vertices; }
protected:
	virtual ~TREVertexArray(void);
	virtual void dealloc(void);

	TREVertex* m_vertices;
	unsigned int m_count;
	unsigned int m_allocated;
};

#endif // __TREVERTEXARRAY_H__
