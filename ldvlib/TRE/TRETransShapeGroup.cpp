#include "TRETransShapeGroup.h"
#include "TREVertexArray.h"
#include "TREVertexStore.h"
#include "TREMainModel.h"
#include <TCFoundation/TCMacros.h>
#include <stdlib.h>

#ifdef WIN32
#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
#define new DEBUG_CLIENTBLOCK
#endif // _DEBUG
#endif // WIN32

TRETransShapeGroup::TRETransShapeGroup(void)
	:m_sortedTriangles(NULL),
	m_origIndices(NULL)
	//m_useSortThread(false),
	//m_sortThread(NULL)
{
}

TRETransShapeGroup::TRETransShapeGroup(const TRETransShapeGroup &other)
	:TREColoredShapeGroup(other),
	m_sortedTriangles((TRESortedTriangleArray *)TCObject::copy(
		other.m_sortedTriangles)),
		m_origIndices((TCULongArray *)TCObject::copy(other.m_origIndices))
{
}

TRETransShapeGroup::~TRETransShapeGroup(void)
{
}

void TRETransShapeGroup::dealloc(void)
{
	TCObject::release(m_origIndices);
	TCObject::release(m_sortedTriangles);
	TREColoredShapeGroup::dealloc();
}

void TRETransShapeGroup::draw(bool sort)
{
	if (sort)
	{
		if (m_mainModel->hasWorkerThreads())
		{
			m_mainModel->waitForSort();
		}
		else
		{
			treGlGetFloatv(GL_MODELVIEW_MATRIX, m_sortMatrix);
			sortShapes();
		}
	}
	drawShapeType(TRESTriangle);
}

void TRETransShapeGroup::backgroundSort(void)
{
	memcpy(m_sortMatrix, m_mainModel->getCurrentModelViewMatrix(),
		sizeof(m_sortMatrix));
	sortShapes();
}

static int triangleCompareFunc(const void *left, const void *right)
{
	TRESortedTriangle *leftTriangle = *(TRESortedTriangle **)left;
	TRESortedTriangle *rightTriangle = *(TRESortedTriangle **)right;

	if (leftTriangle->depth > rightTriangle->depth)
	{
		return -1;
	}
	else if (leftTriangle->depth < rightTriangle->depth)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void TRETransShapeGroup::initSortedTriangles(void)
{
	if (!m_sortedTriangles)
	{
		TCULongArray *indices = getIndices(TRESTriangle);

		if (indices)
		{
			int i, j;
			int count = indices->getCount();
			TREVertexArray *vertices = m_vertexStore->getVertices();
			const TCFloat oneThird = 1.0f / 3.0f;

			if (!m_mainModel->onLastStep())
			{
				int step = m_mainModel->getStep();
				IntVector &stepCounts = m_stepCounts[TRESTriangle];

				if (stepCounts.size() > (size_t)step)
				{
					count = stepCounts[step];
				}
			}
			m_sortedTriangles = new TRESortedTriangleArray(count / 3);
			for (i = 0; i < count; i += 3)
			{
				TRESortedTriangle *sortedTriangle = new TRESortedTriangle;
				TCFloat midX = 0.0f;
				TCFloat midY = 0.0f;
				TCFloat midZ = 0.0f;

				sortedTriangle->indices[0] = (*indices)[i];
				sortedTriangle->indices[1] = (*indices)[i + 1];
				sortedTriangle->indices[2] = (*indices)[i + 2];
				for (j = 0; j < 3; j++)
				{
					const TREVertex &vertex =
						(*vertices)[sortedTriangle->indices[j]];

					midX += vertex.v[0];
					midY += vertex.v[1];
					midZ += vertex.v[2];
				}
				sortedTriangle->center = TCVector(midX * oneThird,
					midY * oneThird, midZ * oneThird);
				m_sortedTriangles->addObject(sortedTriangle);
				sortedTriangle->release();
			}
		}
		else
		{
			return;
		}
	}
}

void TRETransShapeGroup::sortShapes(void)
{
	int i;
	int count;
	TCULong *values;
	TCULongArray *indices = getIndices(TRESTriangle);
	int offset = 0;

	initSortedTriangles();
	count = m_sortedTriangles->getCount();
	for (i = 0; i < count; i++)
	{
		TRESortedTriangle *sortedTriangle = (*m_sortedTriangles)[i];
		sortedTriangle->depth =
			sortedTriangle->center.transformPoint(m_sortMatrix).
			lengthSquared();
	}
	qsort(m_sortedTriangles->getItems(), count, sizeof(void *),
		triangleCompareFunc);
	values = indices->getValues();
	for (i = 0; i < count; i++, offset += 3)
	{
		TRESortedTriangle *sortedTriangle = (*m_sortedTriangles)[i];

		values[offset] = sortedTriangle->indices[0];
		values[offset + 1] = sortedTriangle->indices[1];
		values[offset + 2] = sortedTriangle->indices[2];
	}
}

void TRETransShapeGroup::setStepCounts(const IntVector &value)
{
	TCULong index = getShapeTypeIndex(TRESTriangle);

	m_stepCounts[TRESTriangle] = value;
	TCObject::release(m_origIndices);
	m_origIndices = (TCULongArray*)TCObject::copy((*m_indices)[index]);
}

void TRETransShapeGroup::stepChanged(void)
{
	TCULong index = getShapeTypeIndex(TRESTriangle);
	TCULongArray *indicesCopy = (TCULongArray*)TCObject::copy(m_origIndices);

	m_indices->replaceObject(indicesCopy, index);
	TCObject::release(indicesCopy);
	TCObject::release(m_sortedTriangles);
	m_sortedTriangles = NULL;
}
