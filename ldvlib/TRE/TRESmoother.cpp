#include "TRESmoother.h"
#include "TREVertexArray.h"
#include <set>

typedef std::set<int> IntSet;
typedef IntSet::const_reverse_iterator IntSetCRI;

#ifdef WIN32
#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
#define new DEBUG_CLIENTBLOCK
#endif // _DEBUG
#endif // WIN32

TRESmoother::TRESmoother(void)
	:m_vertices(new TREVertexArray),
	m_sharedList(NULL)
{
}

TRESmoother::TRESmoother(const TREVertex &vertex)
	:m_startVertex(vertex),
	m_vertices(new TREVertexArray),
	m_sharedList(NULL)
{
}

TRESmoother::TRESmoother(const TRESmoother &other)
	:m_startVertex(other.m_startVertex),
	m_vertices((TREVertexArray *)TCObject::copy(other.m_vertices)),
	m_normals(other.m_normals),
	m_sharedList((TCULongArrayArray *)TCObject::copy(other.m_sharedList))
{
}

TRESmoother::~TRESmoother(void)
{
	TCObject::release(m_vertices);
	TCObject::release(m_sharedList);
}

void TRESmoother::addVertex(const TREVertex &vertex)
{
	m_vertices->addVertex(vertex);
}

TCVector &TRESmoother::getNormal(int index)
{
	if (m_normals.empty())
	{
		m_normals.resize(m_vertices->getCount());
	}
	return m_normals[index];
}

TRESmoother &TRESmoother::operator=(const TRESmoother &other)
{
	m_startVertex = other.m_startVertex;
	m_vertices = (TREVertexArray *)other.m_vertices->copy();
	m_normals = other.m_normals;
	return *this;
}

void TRESmoother::markShared(int index0, int index1)
{
	int i;
	TCULongArray *list;
	int index = index0;
	int sharedIndex = index1;

	if (index1 < index)
	{
		index = index1;
		sharedIndex = index0;
	}
	if (!m_sharedList)
	{
		m_sharedList = new TCULongArrayArray;
	}
	for (i = m_sharedList->getCount(); i <= index; i++)
	{
		m_sharedList->addObject(NULL);
	}
	list = (*m_sharedList)[index];
	if (list)
	{
		if (list->indexOfValue(sharedIndex) == -1)
		{
			list->addValue(sharedIndex);
		}
	}
	else
	{
		list = new TCULongArray;
		list->addValue(sharedIndex);
		m_sharedList->replaceObject(list, index);
		list->release();
	}
}

void TRESmoother::finish(void)
{
	int i = 0;
	int count;

	if (m_sharedList)
	{
		int j, k, l;

		count = m_sharedList->getCount();
		for (i = 0; i < count; i++)
		{
			TCULongArray *list = (*m_sharedList)[i];

			if (list)
			{
				int sharedCount;

				for (j = 0; list != NULL && j < list->getCount(); j++)
				{
					int otherIndex = (*list)[j];
					TCVector &normal = m_normals[i];
					const TCVector &otherNormal = m_normals[otherIndex];

					if (shouldFlipNormal(normal, otherNormal))
					{
						normal -= otherNormal;
					}
					else
					{
						normal += otherNormal;
					}
					if (otherIndex < m_sharedList->getCount())
					{
						TCULongArray *otherList = (*m_sharedList)[otherIndex];

						if (otherList)
						{
							int otherListCount = otherList->getCount();

							for (k = 0; k < otherListCount; k++)
							{
								int newIndex = (*otherList)[k];

								if (list->indexOfValue(newIndex) == -1)
								{
									list->addValue(newIndex);
								}
							}
							m_sharedList->replaceObject(NULL, otherIndex);
							if (otherIndex == i)
							{
								// This shouldn't be necessary, and I don't
								// think we can get here anymore. But even
								// though I think I fixed the problem that got
								// us here, I'm not sure about that, so I'm
								// leaving it here.
								list = NULL;
							}
						}
					}
					for (k = i + 1; list != NULL && k < count; k++)
					{
						TCULongArray *otherList = (*m_sharedList)[k];

						if (otherList)
						{
							int otherListCount = otherList->getCount();
							// Record deletion indices in a set (sorted).
							IntSet deleteIndices;

							for (l = 0; l < otherListCount; l++)
							{
								if (list->indexOfValue((*otherList)[l]) != -1)
								{
									if (list->indexOfValue(k) == -1)
									{
										list->addValue(k);
									}
									deleteIndices.insert(l);
								}
							}
							// Iterate through the deletion index set in
							// reverse order, deleting the items from
							// otherList.
							for (IntSetCRI it = deleteIndices.rbegin();
								 it != deleteIndices.rend(); ++it)
							{
								otherList->removeValueAtIndex(*it);
							}
						}
					}
				}
				m_normals[i].normalize();
				if (list)
				{
					sharedCount = list->getCount();
					for (j = 0; j < sharedCount; j++)
					{
						m_normals[(*list)[j]] = m_normals[i];
					}
				}
			}
			else
			{
				if (m_normals[i].lengthSquared() > 0)
				{
					m_normals[i].normalize();
				}
			}
		}
	}
	if (!m_normals.empty())
	{
		count = m_vertices->getCount();
		for (; i < count; i++)
		{
			TCFloat lengthSquared = m_normals[i].lengthSquared();
			if (lengthSquared > 0)
			{
				m_normals[i] /= sqrt(lengthSquared);
			}
		}
	}
}

bool TRESmoother::shouldFlipNormal(const TCVector &normal1,
								   const TCVector &normal2)
{
	if (normal1.get(0) == 0.0f && normal1.get(1) == 0.0f &&
		normal1.get(2) == 0.0f)
	{
		return false;
	}
	else
	{
		TCVector normal3 = normal1 / normal1.length();
		TCVector normal4 = normal2 / normal2.length();
		TCFloat dotProduct = normal3.dot(normal4);

		if (!fEq(dotProduct, 0.0f) && dotProduct < 0.0f)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}
