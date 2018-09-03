#include "TREColoredShapeGroup.h"
#include "TREVertexStore.h"
#include "TREGL.h"
#include "TREMainModel.h"

#ifdef WIN32
#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
#define new DEBUG_CLIENTBLOCK
#endif // _DEBUG
#endif // WIN32

TREColoredShapeGroup::TREColoredShapeGroup(void)
	: m_transferStripCounts(NULL)
{
}

TREColoredShapeGroup::TREColoredShapeGroup(const TREColoredShapeGroup &other)
	: TREShapeGroup(other)
	, m_transferStripCounts(TCObject::copy(other.m_transferStripCounts))
{
}

TREColoredShapeGroup::~TREColoredShapeGroup(void)
{
}

void TREColoredShapeGroup::dealloc(void)
{
	TCObject::release(m_transferStripCounts);
	TREShapeGroup::dealloc();
}

TCObject *TREColoredShapeGroup::copy(void) const
{
	return new TREColoredShapeGroup(*this);
}

int TREColoredShapeGroup::addShape(TREShapeType shapeType, TCULong color,
								   const TCVector *vertices, int count)
{
	return addShape(shapeType, color, vertices, NULL, count);
}

int TREColoredShapeGroup::addShape(TREShapeType shapeType, TCULong color,
								   const TCVector *vertices,
								   const TCVector *normals, int count)
{
	return addShape(shapeType, color, vertices, normals, NULL, count);
}

int TREColoredShapeGroup::addShape(TREShapeType shapeType, TCULong color,
								   const TCVector *vertices,
								   const TCVector *normals,
								   const TCVector *textureCoords, int count)
{
	int index;

	m_vertexStore->setupColored();
	if (textureCoords)
	{
		m_vertexStore->setupTextured();
	}
	if (normals)
	{
		if (textureCoords)
		{
			index = m_vertexStore->addVertices(htonl(color), vertices, normals,
				textureCoords, count, m_mainModel->getCurStepIndex());
		}
		else
		{
			index = m_vertexStore->addVertices(htonl(color), vertices, normals,
				count, m_mainModel->getCurStepIndex());
		}
	}
	else
	{
		index = m_vertexStore->addVertices(htonl(color), vertices, count,
			m_mainModel->getCurStepIndex());
	}
	addShapeIndices(shapeType, index, count);
	return index;
}

int TREColoredShapeGroup::addLine(TCULong color, const TCVector *vertices)
{
	return addShape(TRESLine, color, vertices, 2);
}

// I really hate to have some much code copied from TREShapeGroup, but there's
// really no easy way to get the color in.
int TREColoredShapeGroup::addConditionalLine(
	TCULong color,
	const TCVector *vertices,
	const TCVector *controlPoints)
{
	int index;

	color = htonl(color);
	m_vertexStore->setupColored();
	if (!m_controlPointIndices)
	{
		m_controlPointIndices = new TCULongArray;
	}
	if (m_mainModel->getStencilConditionalsFlag() &&
		m_mainModel->getVertexArrayEdgeFlagsFlag())
	{
		m_vertexStore->setConditionalsFlag(true);
	}
	// The edge flag for control point vertices will always be GL_FALSE.
	index = m_vertexStore->addVertices(color, controlPoints, 2,
		m_mainModel->getCurStepIndex(), GL_FALSE);
	addIndices(m_controlPointIndices, index, 2);
	if (m_mainModel->getStencilConditionalsFlag() &&
		m_mainModel->getVertexArrayEdgeFlagsFlag())
	{
		index = m_vertexStore->addVertices(color, vertices, 1,
			m_mainModel->getCurStepIndex());
		addShapeIndices(TRESConditionalLine, index, 1);
		// We need a second copy, and this one needs to have the edge flag set
		// to GL_FALSE.  Note that these will always be accessed as index + 1.
		m_vertexStore->addVertices(color, vertices, 1,
			m_mainModel->getCurStepIndex(), GL_FALSE);
		index = m_vertexStore->addVertices(color, &vertices[1], 1,
			m_mainModel->getCurStepIndex());
		addShapeIndices(TRESConditionalLine, index, 1);
		index = m_vertexStore->addVertices(color, &vertices[1], 1,
			m_mainModel->getCurStepIndex(), GL_FALSE);
	}
	else
	{
		index = m_vertexStore->addVertices(color, vertices, 2,
			m_mainModel->getCurStepIndex());
		addShapeIndices(TRESConditionalLine, index, 2);
	}
	return index;
}

int TREColoredShapeGroup::addTriangle(TCULong color, const TCVector *vertices)
{
	return addShape(TRESTriangle, color, vertices, 3);
}

int TREColoredShapeGroup::addTriangle(TCULong color, const TCVector *vertices,
									  const TCVector *normals)
{
	return addShape(TRESTriangle, color, vertices, normals, 3);
}

int TREColoredShapeGroup::addTriangle(TCULong color, const TCVector *vertices,
									  const TCVector *normals,
									  const TCVector *textureCoords)
{
	return addShape(TRESTriangle, color, vertices, normals, textureCoords, 3);
}

int TREColoredShapeGroup::addQuad(TCULong color, const TCVector *vertices)
{
	return addShape(TRESQuad, color, vertices, 4);
}

int TREColoredShapeGroup::addQuad(TCULong color, const TCVector *vertices,
								  const TCVector *normals)
{
	return addShape(TRESQuad, color, vertices, normals, 4);
}

int TREColoredShapeGroup::addQuadStrip(TCULong color, const TCVector *vertices,
									   const TCVector *normals, int count)
{
	return addStrip(color, TRESQuadStrip, vertices, normals, count);
}

int TREColoredShapeGroup::addTriangleFan(TCULong color,
										 const TCVector *vertices,
										 const TCVector *normals, int count)
{
	return addStrip(color, TRESTriangleFan, vertices, normals, count);
}

int TREColoredShapeGroup::addStrip(TCULong color, TREShapeType shapeType,
								   const TCVector *vertices,
								   const TCVector *normals, int count)
{
	int index;

	m_vertexStore->setupColored();
	index = m_vertexStore->addVertices(htonl(color), vertices, normals, count,
		m_mainModel->getCurStepIndex());
	addShapeStripCount(shapeType, count);
	addShapeIndices(shapeType, index, count);
	return index;
}

void TREColoredShapeGroup::transferColored(
	TRESTransferType type,
	const TCFloat *matrix)
{
	if (m_indices)
	{
		int bit;

		for (bit = TRESFirst; (TREShapeType)bit <= TRESLast; bit = bit << 1)
		{
			TREShapeType shapeType = (TREShapeType)bit;
			TCULongArray *transferIndices =
				getTransferIndices(type, (TREShapeType)bit);

			if (transferIndices != NULL && transferIndices->getCount())
			{
				// If we already have transfer indices, then we've
				// already processed this model and recorded which indices
				// to transfer.  If that is the case, we don't want to
				// re-record the indices.
				transferIndices = NULL;
			}
			if (type != TTTexmapped || isTexmappedShapeType(shapeType))
			{
				transferColored(type, shapeType, getIndices(shapeType),
					transferIndices, matrix);
			}
		}
	}
}

bool TREColoredShapeGroup::shouldTransferIndex(
	TRESTransferType type,
	TREShapeType shapeType,
	TCULong color,
	int index,
	const TCFloat *matrix)
{
	return TREShapeGroup::shouldTransferIndex(type, shapeType, color, index,
		true, matrix);
}

void TREColoredShapeGroup::transferColored(
	TRESTransferType type,
	TREShapeType shapeType,
	TCULongArray *indices,
	TCULongArray *transferIndices,
	const TCFloat *matrix)
{
	TCULongArray *colors = m_vertexStore->getColors();
	TREVertexArray *oldVertices = m_vertexStore->getVertices();
	TREVertexArray *oldNormals = m_vertexStore->getNormals();

	if (indices && colors && oldVertices && oldNormals)
	{
		int i;
		int count = indices->getCount();

		if (shapeType == TRESTriangle || shapeType == TRESQuad)
		{
			int shapeSize = numPointsForShapeType(shapeType);

			if (shapeSize > 2)
			{
				// Start at the end and work backward.  Otherwise our index will
				// have to change every time items are removed.
				for (i = count - shapeSize; i >= 0; i -= shapeSize)
				{
					TCULong index = (*indices)[i];
					TCULong color = htonl((*colors)[index]);

					//if (isTransparent(color, false))
					if (shouldTransferIndex(type, shapeType, color, index,
						matrix))
					{
//						TCVector vertices[3];
//						TCVector normals[3];

						transferTriangle(type, color, index, (*indices)[i + 1],
							(*indices)[i + 2], matrix);
						if (shapeSize == 4)
						{
							transferTriangle(type, color, index,
								(*indices)[i + 2], (*indices)[i + 3], matrix);
						}
						recordTransfer(transferIndices, i, shapeSize);
					}
				}
			}
		}
		else if (shapeType >= TRESFirstStrip && shapeType <= TRESLast)
		{
			int shapeTypeIndex = getShapeTypeIndex(shapeType);
			TCULongArray *stripCounts = (*m_stripCounts)[shapeTypeIndex];
			TCULongArray *transferStripCounts =
				getTransferStripCounts(shapeType);
			int numStrips = stripCounts->getCount();
			int offset = count;

			if (transferStripCounts && transferStripCounts->getCount())
			{
				transferStripCounts = NULL;
			}
			for (i = numStrips - 1; i >= 0; i--)
			{
				int stripCount = (*stripCounts)[i];
				int index;
				int color;

				offset -= stripCount;
				index = (*indices)[offset];
				color = htonl((*colors)[index]);
				if (shouldTransferIndex(type, shapeType, color, index, matrix))
				{
					switch (shapeType)
					{
					case TRESTriangleStrip:
						transferTriangleStrip(type, shapeTypeIndex, color,
							offset, stripCount, matrix);
						break;
					case TRESQuadStrip:
						transferQuadStrip(type, shapeTypeIndex, color, offset,
							stripCount, matrix);
						break;
					case TRESTriangleFan:
						transferTriangleFan(type, shapeTypeIndex, color, offset,
							stripCount, matrix);
						break;
					default:
						break;
					}
					recordTransfer(transferIndices, offset, stripCount);
					if (transferStripCounts)
					{
						transferStripCounts->addValue(i);
					}
				}
			}
		}
	}
}

bool TREColoredShapeGroup::shouldGetTransferIndices(TRESTransferType /*type*/)
{
	return true;
}

TCULongArray *TREColoredShapeGroup::getTransferStripCounts(
	TREShapeType shapeType)
{
	TCULong index = getShapeTypeIndex(shapeType);

	if (!(m_shapesPresent & shapeType))
	{
		return NULL;
	}
	if (!m_transferStripCounts)
	{
		int i;
		int count = m_indices->getCount();

		m_transferStripCounts = new TCULongArrayArray;
		for (i = 0; i < count; i++)
		{
			TCULongArray *stripCounts = new TCULongArray;

			m_transferStripCounts->addObject(stripCounts);
			stripCounts->release();
		}
	}
	return (*m_transferStripCounts)[index];
}

void TREColoredShapeGroup::cleanupTransfer(void)
{
	int i, j;

	TREShapeGroup::cleanupTransfer();
	if (m_transferStripCounts)
	{
		int arrayCount = m_transferStripCounts->getCount();

		for (i = 0; i < arrayCount; i++)
		{
			TCULongArray *transferStripCounts =
				(*m_transferStripCounts)[i];
			TCULongArray *stripCounts = (*m_stripCounts)[i];
			int stripCountCount = transferStripCounts->getCount();

			for (j = 0; j < stripCountCount; j++)
			{
				stripCounts->removeValueAtIndex((*transferStripCounts)[j]);
			}
		}
		m_transferStripCounts->release();
		m_transferStripCounts = NULL;
	}
}
