#include "TRETexmappedShapeGroup.h"
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

TRETexmappedShapeGroup::TRETexmappedShapeGroup(void)
	: m_origIndices(NULL)
{
}

TRETexmappedShapeGroup::TRETexmappedShapeGroup(const TRETexmappedShapeGroup &other)
	: TREColoredShapeGroup(other)
	, m_origIndices((TCULongArray *)TCObject::copy(other.m_origIndices))
{
}

TRETexmappedShapeGroup::~TRETexmappedShapeGroup(void)
{
}

void TRETexmappedShapeGroup::dealloc(void)
{
	TCObject::release(m_origIndices);
	TREColoredShapeGroup::dealloc();
}

void TRETexmappedShapeGroup::draw(void)
{
	drawShapeType(TRESTriangle);
}

void TRETexmappedShapeGroup::setStepCounts(const IntVector &value)
{
	TCULong index = getShapeTypeIndex(TRESTriangle);

	m_stepCounts[TRESTriangle] = value;
	TCObject::release(m_origIndices);
	m_origIndices = (TCULongArray*)TCObject::copy((*m_indices)[index]);
}

void TRETexmappedShapeGroup::stepChanged(void)
{
	TCULong index = getShapeTypeIndex(TRESTriangle);
	TCULongArray *indicesCopy = (TCULongArray*)TCObject::copy(m_origIndices);

	m_indices->replaceObject(indicesCopy, index);
	TCObject::release(indicesCopy);
}
