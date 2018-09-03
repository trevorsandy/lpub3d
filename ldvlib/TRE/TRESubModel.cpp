#include "TRESubModel.h"
#include "TREGL.h"
#include "TREMainModel.h"
#include <TCFoundation/mystring.h>
#include <string.h>

#ifdef WIN32
#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
#define new DEBUG_CLIENTBLOCK
#endif // _DEBUG
#endif // WIN32

TRESubModel::TRESubModel(void)
	:m_model(NULL),
	m_unMirroredSubModel(NULL),
	m_invertedSubModel(NULL),
	m_color(0),
	m_edgeColor(0)
{
#ifdef _LEAK_DEBUG
	strcpy(className, "TRESubModel");
#endif // _LEAK_DEBUG
	memset(&m_flags, 0, sizeof(m_flags));
}

TRESubModel::TRESubModel(const TRESubModel &other)
	:m_model((TREModel *)TCObject::copy(other.m_model)),
	m_unMirroredSubModel((TRESubModel *)TCObject::copy(
		other.m_unMirroredSubModel)),
	m_invertedSubModel((TRESubModel *)TCObject::copy(
		other.m_invertedSubModel)),
	m_color(other.m_color),
	m_edgeColor(other.m_edgeColor),
	m_flags(other.m_flags)
{
#ifdef _LEAK_DEBUG
	strcpy(className, "TRESubModel");
#endif // _LEAK_DEBUG
	memcpy(m_matrix, other.m_matrix, sizeof(m_matrix));
	memcpy(m_originalMatrix, other.m_originalMatrix, sizeof(m_originalMatrix));
}

TRESubModel::TRESubModel(const TRESubModel &other, bool shallow)
	:m_model(shallow ? NULL : (TREModel *)TCObject::copy(other.m_model)),
	m_unMirroredSubModel(shallow ? NULL :
		(TRESubModel *)TCObject::copy(other.m_unMirroredSubModel)),
	m_invertedSubModel(shallow ? NULL :
		(TRESubModel *)TCObject::copy(other.m_invertedSubModel)),
	m_color(other.m_color),
	m_edgeColor(other.m_edgeColor),
	m_flags(other.m_flags)
{
#ifdef _LEAK_DEBUG
	strcpy(className, "TRESubModel");
#endif // _LEAK_DEBUG
	memcpy(m_matrix, other.m_matrix, sizeof(m_matrix));
	memcpy(m_originalMatrix, other.m_originalMatrix, sizeof(m_originalMatrix));
}

TRESubModel::~TRESubModel(void)
{
}

void TRESubModel::dealloc(void)
{
	TCObject::release(m_model);
	if (!m_flags.unMirrored)
	{
		TCObject::release(m_unMirroredSubModel);
	}
	m_unMirroredSubModel = NULL;
	if (!m_flags.inverted)
	{
		TCObject::release(m_invertedSubModel);
	}
	m_invertedSubModel = NULL;
	//if (m_unMirroredSubModel)
	//{
	//	// The following points back to us, and since we're being deallocated
	//	// right now, we don't want it to deallocate us.
	//	m_unMirroredSubModel->m_unMirroredSubModel = NULL;
	//}
	//if (m_invertedSubModel)
	//{
	//	// The following points back to us, and since we're being deallocated
	//	// right now, we don't want it to deallocate us.
	//	m_invertedSubModel->m_invertedSubModel = NULL;
	//}
	//if (!m_flags.unMirrored)
	//{
	//	TCObject::release(m_unMirroredSubModel);
	//}
	//m_unMirroredSubModel = NULL;
	//if (!m_flags.inverted)
	//{
	//	TCObject::release(m_invertedSubModel);
	//}
	//m_invertedSubModel = NULL;
	TCObject::dealloc();
}

TCObject *TRESubModel::copy(void) const
{
	return new TRESubModel(*this);
}

TRESubModel *TRESubModel::shallowCopy(void)
{
	return new TRESubModel(*this, true);
}

TRESubModel *TRESubModel::getUnMirroredSubModel(void)
{
	if (!m_unMirroredSubModel)
	{
		m_unMirroredSubModel = shallowCopy();
		m_unMirroredSubModel->unMirror(this);
	}
	return m_unMirroredSubModel;
}

TRESubModel *TRESubModel::getInvertedSubModel(void)
{
	if (!m_invertedSubModel)
	{
		m_invertedSubModel = shallowCopy();
		m_invertedSubModel->invert(this);
	}
	return m_invertedSubModel;
}

void TRESubModel::unMirror(TRESubModel *originalSubModel)
{
	m_unMirroredSubModel = originalSubModel;
	m_flags.unMirrored = !originalSubModel->m_flags.unMirrored;
	if (m_unMirroredSubModel->m_invertedSubModel)
	{
		m_invertedSubModel =
			m_unMirroredSubModel->m_invertedSubModel->m_unMirroredSubModel;
		if (m_invertedSubModel)
		{
			m_invertedSubModel->m_invertedSubModel = this;
			if (m_flags.inverted)
			{
				retain();
			}
			else
			{
				m_invertedSubModel->retain();
			}
		}
	}
	m_model = originalSubModel->m_model->getUnMirroredModel();
	m_model->retain();
}

void TRESubModel::invert(TRESubModel *originalSubModel)
{
	m_invertedSubModel = originalSubModel;
	m_flags.inverted = !originalSubModel->m_flags.inverted;
	if (m_invertedSubModel->m_unMirroredSubModel)
	{
		m_unMirroredSubModel =
			m_invertedSubModel->m_unMirroredSubModel->m_invertedSubModel;
		if (m_unMirroredSubModel)
		{
			m_unMirroredSubModel->m_unMirroredSubModel = this;
			if (m_flags.unMirrored)
			{
				retain();
			}
			else
			{
				m_unMirroredSubModel->retain();
			}
		}
	}
	m_model = originalSubModel->m_model->getInvertedModel();
	m_model->retain();
}

void TRESubModel::setModel(TREModel *model)
{
	model->retain();
	TCObject::release(m_model);
	m_model = model;
}

void TRESubModel::setMatrix(const TCFloat *matrix)
{
	memcpy(m_matrix, matrix, sizeof(m_matrix));
	memcpy(m_originalMatrix, matrix, sizeof(m_originalMatrix));
	if (TCVector::determinant(m_matrix) < 0.0f)
	{
		m_flags.mirrorMatrix = true;
	}
	else
	{
		m_flags.mirrorMatrix = false;
	}
}

TREModel *TRESubModel::getEffectiveModel(void) const
{
	if (m_flags.mirrorMatrix)
	{
		if (m_flags.bfcInvert)
		{
			return m_model->getUnMirroredModel()->getInvertedModel();
		}
		else
		{
			return m_model->getUnMirroredModel();
		}
	}
	else if (m_flags.bfcInvert)
	{
		return m_model->getInvertedModel();
	}
	else
	{
		return m_model;
	}
}

void TRESubModel::setColor(TCULong color, TCULong edgeColor)
{
	m_color = htonl(color);
	m_edgeColor = htonl(edgeColor);
	m_flags.colorSet = true;
}

TCULong TRESubModel::getColor(void)
{
	return htonl(m_color);
}

TCULong TRESubModel::getEdgeColor(void)
{
	return htonl(m_edgeColor);
}

void TRESubModel::applyColor(TCULong color, bool applySpecular)
{
	GLbitfield attributes = GL_CURRENT_BIT;

	if (applySpecular && (m_flags.specular || m_flags.shininess))
	{
		attributes |= GL_LIGHTING_BIT;
	}
	glPushAttrib(attributes);
	glColor4ubv((GLubyte*)&color);
	if (applySpecular)
	{
		if (m_flags.shininess)
		{
			glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, m_shininess);
		}
		if (m_flags.specular)
		{
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, m_specular);
		}
	}
}

void TRESubModel::draw(
	TREMSection section,
	bool colored,
	bool subModelsOnly /*= false*/,
	bool nonUniform /*= false*/,
	bool skipTexmapped /*= false*/)
{
	skipTexmapped = false;
#ifdef _GL_POPCOLOR_BROKEN
	GLfloat oldColor[4] = { 0.0f };
#endif // _GL_POPCOLOR_BROKEN
	if (!colored)
	{
		// I know this looks backwards.  However, the colored geometry has the
		// color set for each primitive.  Consequently, it doesn't need the
		// OpenGL color to be set.
		if (m_flags.colorSet)
		{
			// If our color is set, reset to not be in subModelsOnly mode.  It
			// might get turned back on below, of course.
			subModelsOnly = false;
			if (section == TREMEdgeLines || section == TREMConditionalLines)
			{
#ifdef _GL_POPCOLOR_BROKEN
				glGetFloatv(GL_CURRENT_COLOR, oldColor);
#endif // _GL_POPCOLOR_BROKEN
				applyColor(m_edgeColor, false);
			}
			else
			{
				if (section != TREMLines)
				{
					if (TREShapeGroup::isTransparent(m_color, true))
					{
						// We don't want to draw transparent geometry here.
						// However, we do want to draw our submodels, just in
						// case any of them have non-transparent geometry.
						subModelsOnly = true;
					}
				}
				if (!subModelsOnly)
				{
#ifdef _GL_POPCOLOR_BROKEN
					glGetFloatv(GL_CURRENT_COLOR, oldColor);
#endif // _GL_POPCOLOR_BROKEN
					applyColor(m_color, true);
				}
			}
		}
	}
	glPushMatrix();
	treGlMultMatrixf(m_matrix);
	getEffectiveModel()->draw(section, colored, subModelsOnly,
		getNonUniformFlag() | nonUniform, skipTexmapped);
	glPopMatrix();
	if (!colored)
	{
		if (m_flags.colorSet && !subModelsOnly)
		{
			glPopAttrib();
#ifdef _GL_POPCOLOR_BROKEN
			glColor4fv(oldColor);
#endif // _GL_POPCOLOR_BROKEN
		}
	}
}

/*
void TRESubModel::compileDefaultColor(void)
{
	getEffectiveModel()->compileDefaultColor();
}

void TRESubModel::compileColored(void)
{
	getEffectiveModel()->compileColored();
}

void TRESubModel::drawDefaultColor(void)
{
	if (m_flags.colorSet)
	{
		glPushAttrib(GL_CURRENT_BIT);
		glColor4ubv((GLubyte*)&m_color);
	}
	glPushMatrix();
	treGlMultMatrixf(m_matrix);
	getEffectiveModel()->drawDefaultColor();
	glPopMatrix();
	if (m_flags.colorSet)
	{
		glPopAttrib();
	}
}

void TRESubModel::drawBFC(void)
{
	if (m_flags.colorSet)
	{
		glPushAttrib(GL_CURRENT_BIT);
		glColor4ubv((GLubyte*)&m_color);
	}
	glPushMatrix();
	treGlMultMatrixf(m_matrix);
	getEffectiveModel()->drawBFC();
	glPopMatrix();
	if (m_flags.colorSet)
	{
		glPopAttrib();
	}
}

void TRESubModel::drawDefaultColorLines(void)
{
	if (m_flags.colorSet)
	{
		glPushAttrib(GL_CURRENT_BIT);
		glColor4ubv((GLubyte*)&m_color);
	}
	glPushMatrix();
	treGlMultMatrixf(m_matrix);
	m_model->drawDefaultColorLines();
	glPopMatrix();
	if (m_flags.colorSet)
	{
		glPopAttrib();
	}
}

void TRESubModel::drawEdgeLines(void)
{
	if (m_flags.colorSet)
	{
		glPushAttrib(GL_CURRENT_BIT);
		glColor4ubv((GLubyte*)&m_edgeColor);
	}
	glPushMatrix();
	treGlMultMatrixf(m_matrix);
	m_model->drawEdgeLines();
	glPopMatrix();
	if (m_flags.colorSet)
	{
		glPopAttrib();
	}
}

void TRESubModel::drawColored(void)
{
	glPushMatrix();
	treGlMultMatrixf(m_matrix);
	getEffectiveModel()->drawColored();
	glPopMatrix();
}

void TRESubModel::drawColoredBFC(void)
{
	glPushMatrix();
	treGlMultMatrixf(m_matrix);
	getEffectiveModel()->drawColoredBFC();
	glPopMatrix();
}

void TRESubModel::drawColoredLines(void)
{
	glPushMatrix();
	treGlMultMatrixf(m_matrix);
	m_model->drawColoredLines();
	glPopMatrix();
}

void TRESubModel::drawColoredEdgeLines(void)
{
	glPushMatrix();
	treGlMultMatrixf(m_matrix);
	m_model->drawColoredEdgeLines();
	glPopMatrix();
}
*/

void TRESubModel::scanPoints(TCObject *scanner,
							 TREScanPointCallback scanPointCallback,
							 const TCFloat *matrix)
{
	TCFloat newMatrix[16];

	TCVector::multMatrix(matrix, m_matrix, newMatrix);
	getEffectiveModel()->scanPoints(scanner, scanPointCallback, newMatrix);
}

void TRESubModel::unshrinkNormals(const TCFloat *matrix,
								  const TCFloat *unshrinkMatrix)
{
	TCFloat newMatrix[16];

	TCVector::multMatrix(matrix, m_matrix, newMatrix);
	m_model->unshrinkNormals(newMatrix, unshrinkMatrix);
}

// Thanks go to Lars again here for the simplified positioning-after-shrinkage
// algorithm.
void TRESubModel::shrink(TCFloat amount)
{
	if (!m_flags.shrunk)
	{
		TCVector boundingMin;
		TCVector boundingMax;
		TCFloat scaleMatrix[16];
		TCFloat tempMatrix[16];

		m_model->getBoundingBox(boundingMin, boundingMax);
		TCVector::calcScaleMatrix(amount, scaleMatrix, boundingMin, boundingMax);
		TCVector::multMatrix(m_matrix, scaleMatrix, tempMatrix);
		memcpy(m_matrix, tempMatrix, sizeof(m_matrix));
		m_model->unshrinkNormals(scaleMatrix);
		m_flags.shrunk = true;
	}
}

void TRESubModel::transferColored(
	TREShapeGroup::TRESTransferType type,
	TREMSection section,
	const TCFloat *matrix)
{
	TCFloat newMatrix[16];

	TCVector::multMatrix(matrix, m_matrix, newMatrix);
	m_model->transferColored(type, section, newMatrix);
}

void TRESubModel::transfer(
	TREShapeGroup::TRESTransferType type,
	TCULong color,
	TREMSection section,
	const TCFloat *matrix)
{
	TCFloat newMatrix[16];

	TCVector::multMatrix(matrix, m_matrix, newMatrix);
	if (m_flags.colorSet)
	{
		color = m_color;
	}
	m_model->transfer(type, color, section, newMatrix);
}

void TRESubModel::setSpecular(const GLfloat *specular)
{
	memcpy(m_specular, specular, sizeof(m_specular));
	m_flags.specular = true;
}

void TRESubModel::setShininess(GLfloat shininess)
{
	m_shininess = shininess;
	m_flags.shininess = true;
}
