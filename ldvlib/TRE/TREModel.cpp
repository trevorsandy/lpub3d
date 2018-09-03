#include "TREModel.h"
#include "TRESubModel.h"
#include "TREMainModel.h"
#include "TREShapeGroup.h"
#include "TREColoredShapeGroup.h"
#include "TREVertexArray.h"
#include "TREVertexStore.h"
#include "TREGL.h"

#include <TCFoundation/mystring.h>
#include <TCFoundation/TCMacros.h>

#ifdef WIN32
#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
#define new DEBUG_CLIENTBLOCK
#endif // _DEBUG
#endif // WIN32

// Max smooth angle == 50 (value is cos(25))
//#define SMOOTH_THRESHOLD 0.906307787f
// Max smooth angle == 80 (value is cos(40))
#define SMOOTH_THRESHOLD 0.766044443f

// NOTE: When a texture-mapped piece of geometry is added, it gets moved into
// the main model. In this case, needDupe is set to true to indicate that the
// piece of geometry needs to be added to the model twice; once for the texture,
// and once for the underlying polygon.
#define TEXMAP_ADD_INDEX(field, shapeGroup, shapeType)					\
bool needDupe = false;													\
{																		\
	TexmapInfo *texmapInfo = getActiveTexmapInfo();						\
																		\
	if (texmapInfo != NULL)												\
	{																	\
		needDupe = true;												\
		int shapeSize = shapeType == TRESTriangle ? 3 : 4;				\
		TCULongArray *indices = shapeGroup->getIndices(shapeType);		\
		int index = (int)(*indices)[indices->getCount() - shapeSize];	\
																		\
		if (shapeGroup->getBfc())										\
		{																\
			texmapInfo->bfc.field.insert(index);						\
		}																\
		else															\
		{																\
			texmapInfo->standard.field.insert(index);					\
		}																\
	}																	\
}

#define TEXMAP_ADD_STRIP(field, shapeGroup, shapeType)					\
bool needDupe = false;													\
{																		\
	TexmapInfo *texmapInfo = getActiveTexmapInfo();						\
																		\
	if (texmapInfo != NULL)												\
	{																	\
		needDupe = true;												\
		TCULongArray *stripCounts =										\
			shapeGroup->getStripCounts(shapeType);						\
		int stripCount =												\
			(int)(*stripCounts)[stripCounts->getCount() - 1];			\
		TCULongArray *indices = shapeGroup->getIndices(shapeType);		\
		int index = (int)(*indices)[indices->getCount() - stripCount];	\
		TexmapInfo::GeomInfo &geomInfo =								\
			shapeGroup->getBfc() ? texmapInfo->bfc :					\
			texmapInfo->standard;										\
																		\
		geomInfo.field.insert(index);									\
	}																	\
}


//00000001111111111222222222233333333334444444444555555555566666666667777777777888888888899999999990000000000111111111122222222223333333333444444444455555555556
//34567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890

TREModel::TREModel(void)
	:m_name(NULL),
	m_mainModel(NULL),
	m_subModels(NULL),
	m_unMirroredModel(NULL),
	m_invertedModel(NULL),
	m_sectionsPresent(0),
	m_coloredSectionsPresent(0),
	m_curStepIndex(0)
{
#ifdef _LEAK_DEBUG
	strcpy(className, "TREModel");
#endif // _LEAK_DEBUG
	//int i;

	memset(m_shapes, 0, sizeof(m_shapes));
	memset(m_coloredShapes, 0, sizeof(m_coloredShapes));
	memset(m_listIDs, 0, sizeof(m_listIDs));
	memset(m_coloredListIDs, 0, sizeof(m_coloredListIDs));
	memset(m_texListIDs, 0, sizeof(m_texListIDs));
	memset(m_texColoredListIDs, 0, sizeof(m_texColoredListIDs));
	//for (i = 0; i <= TREMLast; i++)
	//{
	//	m_shapes[i] = NULL;
	//	m_coloredShapes[i] = NULL;
	//	m_listIDs[i] = 0;
	//	m_coloredListIDs[i] = 0;
	//}
	m_flags.part = false;
	m_flags.boundingBox = false;
	m_flags.unshrunkNormals = false;
	m_flags.unMirrored = false;
	m_flags.inverted = false;
	m_flags.flattened = false;
}

TREModel::TREModel(const TREModel &other)
	:m_name(copyString(other.m_name)),
	m_mainModel(other.m_mainModel),
	m_subModels((TRESubModelArray *)TCObject::copy(other.m_subModels)),
	m_unMirroredModel((TREModel *)TCObject::copy(other.m_unMirroredModel)),
	m_invertedModel((TREModel *)TCObject::copy(other.m_invertedModel)),
	m_sectionsPresent(other.m_sectionsPresent),
	m_coloredSectionsPresent(other.m_coloredSectionsPresent),
	m_boundingMin(other.m_boundingMin),
	m_boundingMax(other.m_boundingMax),
	m_curStepIndex(other.m_curStepIndex),
	m_stepCounts(other.m_stepCounts),
	m_flags(other.m_flags)
{
#ifdef _LEAK_DEBUG
	strcpy(className, "TREModel");
#endif // _LEAK_DEBUG
	int i;

	for (i = 0; i <= TREMLast; i++)
	{
		m_shapes[i] = (TREShapeGroup *)TCObject::copy(other.m_shapes[i]);
		m_coloredShapes[i] =
			(TREColoredShapeGroup *)TCObject::copy(other.m_coloredShapes[i]);
		//m_listIDs[i] = 0;
		//m_coloredListIDs[i] = 0;
	}
	memset(m_listIDs, 0, sizeof(m_listIDs));
	memset(m_coloredListIDs, 0, sizeof(m_coloredListIDs));
	memset(m_texListIDs, 0, sizeof(m_texListIDs));
	memset(m_texColoredListIDs, 0, sizeof(m_texColoredListIDs));
}

TREModel::TREModel(const TREModel &other, bool shallow)
	:m_name(copyString(other.m_name)),
	m_mainModel(other.m_mainModel),
	m_subModels(shallow ? NULL :
		(TRESubModelArray *)TCObject::copy(other.m_subModels)),
	m_unMirroredModel(shallow ? NULL :
		(TREModel *)TCObject::copy(other.m_unMirroredModel)),
	m_invertedModel(shallow ? NULL :
		(TREModel *)TCObject::copy(other.m_invertedModel)),
	m_sectionsPresent(other.m_sectionsPresent),
	m_coloredSectionsPresent(other.m_coloredSectionsPresent),
	m_boundingMin(other.m_boundingMin),
	m_boundingMax(other.m_boundingMax),
	m_curStepIndex(other.m_curStepIndex),
	m_stepCounts(other.m_stepCounts),
	m_flags(other.m_flags)
{
#ifdef _LEAK_DEBUG
	strcpy(className, "TREModel");
#endif // _LEAK_DEBUG
	int i;

	for (i = 0; i <= TREMLast; i++)
	{
		m_shapes[i] = (TREShapeGroup *)TCObject::copy(other.m_shapes[i]);
		m_coloredShapes[i] =
			(TREColoredShapeGroup *)TCObject::copy(other.m_coloredShapes[i]);
		//m_listIDs[i] = 0;
		//m_coloredListIDs[i] = 0;
	}
	memset(m_listIDs, 0, sizeof(m_listIDs));
	memset(m_coloredListIDs, 0, sizeof(m_coloredListIDs));
	memset(m_texListIDs, 0, sizeof(m_texListIDs));
	memset(m_texColoredListIDs, 0, sizeof(m_texColoredListIDs));
}

TREModel::~TREModel(void)
{
}

void TREModel::dealloc(void)
{
	int i;

	delete[] m_name;
	// Don't release m_mainModel
	TCObject::release(m_subModels);
	if (!m_flags.unMirrored)
	{
		//if (m_unMirroredModel != NULL)
		//{
		//	// The following points back to us, and since we're being deallocated
		//	// right now, we don't want it to deallocate us.
		//	m_unMirroredModel->m_unMirroredModel = NULL;
		//}
		TCObject::release(m_unMirroredModel);
	}
	m_unMirroredModel = NULL;
	if (!m_flags.inverted)
	{
		//if (m_invertedModel != NULL)
		//{
		//	// The following points back to us, and since we're being deallocated
		//	// right now, we don't want it to deallocate us.
		//	m_invertedModel->m_invertedModel = NULL;
		//}
		TCObject::release(m_invertedModel);
	}
	m_invertedModel = NULL;
	uncompile(false);
	for (i = 0; i <= TREMLast; i++)
	{
		TCObject::release(m_shapes[i]);
		TCObject::release(m_coloredShapes[i]);
	}
	TCObject::dealloc();
}

TCObject *TREModel::copy(void) const
{
	return new TREModel(*this);
}

TREModel *TREModel::shallowCopy(void)
{
	return new TREModel(*this, true);
}

TREModel *TREModel::getUnMirroredModel(void)
{
	if (m_unMirroredModel == NULL)
	{
		m_unMirroredModel = shallowCopy();
		m_unMirroredModel->unMirror(this);
	}
	return m_unMirroredModel;
}

TREModel *TREModel::getInvertedModel(void)
{
	if (m_invertedModel == NULL)
	{
		m_invertedModel = shallowCopy();
		m_invertedModel->invert(this);
	}
	return m_invertedModel;
}

void TREModel::unMirror(TREModel *originalModel)
{
	int i;

	m_unMirroredModel = originalModel;
	m_flags.unMirrored = true;
	if (m_unMirroredModel->m_invertedModel != NULL)
	{
		m_invertedModel = m_unMirroredModel->m_invertedModel->m_unMirroredModel;
		if (m_invertedModel != NULL)
		{
			m_invertedModel->m_invertedModel = this;
			if (m_flags.inverted)
			{
				retain();
			}
			else
			{
				m_invertedModel->retain();
			}
		}
	}
	if (originalModel->m_subModels)
	{
		int count;

		count = originalModel->m_subModels->getCount();
		m_subModels = new TRESubModelArray(count);
		for (i = 0; i < count; i++)
		{
			TRESubModel *subModel =
				(*originalModel->m_subModels)[i]->getUnMirroredSubModel();

			m_subModels->addObject(subModel);
		}
	}
	for (i = 0; i <= TREMLast; i++)
	{
		if (!isLineSection(i))
		{
			TREShapeGroup *shapeGroup = m_shapes[i];
			TREColoredShapeGroup *coloredShapeGroup = m_coloredShapes[i];

			if (shapeGroup)
			{
				shapeGroup->unMirror();
			}
			if (coloredShapeGroup)
			{
				coloredShapeGroup->unMirror();
			}
		}
	}
}

void TREModel::invert(TREModel *originalModel)
{
	int i;

	m_invertedModel = originalModel;
	m_flags.inverted = true;
	if (m_invertedModel->m_unMirroredModel != NULL)
	{
		m_unMirroredModel = m_invertedModel->m_unMirroredModel->m_invertedModel;
		if (m_unMirroredModel != NULL)
		{
			m_unMirroredModel->m_unMirroredModel = this;
			if (m_flags.unMirrored)
			{
				retain();
			}
			else
			{
				m_unMirroredModel->retain();
			}
		}
	}
	if (originalModel->m_subModels)
	{
		int count;

		count = originalModel->m_subModels->getCount();
		m_subModels = new TRESubModelArray(count);
		for (i = 0; i < count; i++)
		{
			TRESubModel *subModel =
				(*originalModel->m_subModels)[i]->getInvertedSubModel();

			m_subModels->addObject(subModel);
		}
	}
	for (i = 0; i <= TREMLast; i++)
	{
		if (!isLineSection(i))
		{
			TREShapeGroup *shapeGroup = m_shapes[i];
			TREColoredShapeGroup *coloredShapeGroup = m_coloredShapes[i];

			if (shapeGroup)
			{
				shapeGroup->invert();
			}
			if (coloredShapeGroup)
			{
				coloredShapeGroup->invert();
			}
		}
	}
}

void TREModel::setName(const char *name)
{
	delete[] m_name;
	m_name = copyString(name);
}

GLuint *TREModel::getListIDs(bool colored, bool skipTexmapped)
{
	skipTexmapped = false;
	if (skipTexmapped)
	{
		if (colored)
		{
			return m_texColoredListIDs;
		}
		else
		{
			return m_texListIDs;
		}
	}
	else
	{
		if (colored)
		{
			return m_coloredListIDs;
		}
		else
		{
			return m_listIDs;
		}
	}
}

void TREModel::compile(
	TREMSection section,
	bool colored,
	bool nonUniform /*= false*/,
	bool skipTexmapped /*= false*/)
{
	skipTexmapped = false;
	GLuint *listIDs = getListIDs(colored, skipTexmapped);

	if (!listIDs[section] && isSectionPresent(section, colored))
	{
		if (m_subModels != NULL)
		{
			int i;
			int count = m_subModels->getCount();

			for (i = 0; i < count; i++)
			{
				TRESubModel *subModel = (*m_subModels)[i];
				bool subSkipTexmapped = skipTexmapped;

				if (subModel->getTransferredFlag())
				{
					subSkipTexmapped = true;
				}
				subModel->getEffectiveModel()->compile(section, colored,
					nonUniform | subModel->getNonUniformFlag(),
					subSkipTexmapped);
			}
		}
		if (m_mainModel->getCompileAllFlag() ||
			(m_flags.part && m_mainModel->getCompilePartsFlag()))
		{
			GLuint listID = glGenLists(1);

			glNewList(listID, GL_COMPILE);
			draw(section, colored, false, nonUniform, skipTexmapped);
			glEndList();
			listIDs[section] = listID;
//			debugPrintf("U Standard <<%s>> %d %d\n", m_name, m_flags.unMirrored,
//				m_flags.inverted);
		}
	}
}

void TREModel::draw(TREMSection section)
{
	draw(section, false);
}

void TREModel::checkGLError(char *msg)
{
	GLenum errorCode;
	
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		const char* errorString;// = interpretGLError(errorCode);

		switch (errorCode)
		{
			case GL_INVALID_ENUM:
				errorString = "GL_INVALID_ENUM";
				break;
			case GL_INVALID_VALUE:
				errorString = "GL_INVALID_VALUE";
				break;
			case GL_INVALID_OPERATION:
				errorString = "GL_INVALID_OPERATION";
				break;
			case GL_STACK_OVERFLOW:
				errorString = "GL_STACK_OVERFLOW";
				break;
			case GL_STACK_UNDERFLOW:
				errorString = "GL_STACK_UNDERFLOW";
				break;
			case GL_OUT_OF_MEMORY:
				errorString = "GL_OUT_OF_MEMORY";
				break;
			default:
				errorString = "Unknown Error";
				break;
		}
#ifdef _DEBUG
		debugPrintf("%s: %s\n", msg, errorString);
#else // _DEBUG
		debugPrintf(3, "%s: %s\n", msg, errorString);
#endif // _DEBUG
//		reportError("OpenGL error:\n%s: %s\n", LDMEOpenGL, msg, errorString);
	}
}

void TREModel::draw(
	TREMSection section,
	bool colored,
	bool subModelsOnly /*= false*/,
	bool nonUniform /*= false*/,
	bool skipTexmapped /*= false*/)
{
	skipTexmapped = false;
	GLuint listID = getListIDs(colored, skipTexmapped)[section];

	if (listID && !subModelsOnly &&
		(this != m_mainModel || m_mainModel->onLastStep()))
	{
		// Note that subModelsOnly gets set when the current color is
		// transparent.  In that case, we don't want to draw our geometry,
		// because transparent geometry gets drawn elsewhere.  However, we do
		// want to draw any sub-models, because some of them not be
		// transparent.
		glCallList(listID);
	}
	else if (isSectionPresent(section, colored))
	{
		if (!subModelsOnly)
		{
			TREShapeGroup *shapeGroup;

			if (colored)
			{
				shapeGroup = m_coloredShapes[section];
			}
			else
			{
				shapeGroup = m_shapes[section];
			}
			if (isLineSection(section))
			{
				if (shapeGroup)
				{
					shapeGroup->drawLines();
				}
			}
			else if (section == TREMConditionalLines)
			{
				if (shapeGroup)
				{
					shapeGroup->drawConditionalLines();
				}
			}
			else
			{
				if (m_flags.part && isFlattened())
				{
					setGlNormalize(nonUniform);
				}
				else
				{
					setGlNormalize(true);
				}
				if (shapeGroup)
				{
					shapeGroup->draw(skipTexmapped);
				}
			}
		}
		if (m_subModels)
		{
			int i;
			int count = m_subModels->getCount();
			int step = m_mainModel->getStep();

			if (!m_mainModel->onLastStep() &&
				m_stepCounts.size() > (size_t)step)
			{
				count = std::min(m_stepCounts[step], count);
			}
			for (i = 0; i < count; i++)
			{
				TRESubModel *subModel = (*m_subModels)[i];
				bool subSkipTexmapped = skipTexmapped;

				if (subModel->getTransferredFlag())
				{
					subSkipTexmapped = true;
				}
				if (subModel)
				{
					subModel->draw(section, colored, subModelsOnly,
						nonUniform, subSkipTexmapped);
				}
			}
		}
	}
}

void TREModel::drawColored(TREMSection section)
{
	draw(section, true);
}

void TREModel::setup(TREMSection section)
{
	if (!m_shapes[section])
	{
		TREShapeGroup *shapeGroup = new TREShapeGroup;

		shapeGroup->setModel(this);
		if (section == TREMStud || section == TREMStudBFC)
		{
			shapeGroup->setVertexStore(m_mainModel->getStudVertexStore());
		}
		else
		{
			shapeGroup->setVertexStore(m_mainModel->getVertexStore());
		}
		shapeGroup->setBfc(section == TREMBFC || section == TREMStudBFC);
		// No need to release previous, since we determined it is NULL.
		m_shapes[section] = shapeGroup;
		// Don't release shapeGroup, becase m_shapes isn't a TCObjectArray.
	}
}

void TREModel::setupColored(TREMSection section)
{
	if (!m_coloredShapes[section])
	{
		TREColoredShapeGroup *shapeGroup = new TREColoredShapeGroup;

		shapeGroup->setModel(this);
		if (section == TREMStud || section == TREMStudBFC)
		{
			shapeGroup->setVertexStore(
				m_mainModel->getColoredStudVertexStore());
		}
		else
		{
			shapeGroup->setVertexStore(m_mainModel->getColoredVertexStore());
		}
		shapeGroup->setBfc(section == TREMBFC || section == TREMStudBFC);
		// No need to release previous, since we determined it is NULL.
		m_coloredShapes[section] = shapeGroup;
		// Don't release shapeGroup, becase m_shapes isn't a TCObjectArray.
	}
}

void TREModel::setupStandard(void)
{
	setup(TREMStandard);
}

void TREModel::setupStud(void)
{
	setup(TREMStud);
}

void TREModel::setupStudBFC(void)
{
	setup(TREMStudBFC);
}

void TREModel::setupLines(void)
{
	setup(TREMLines);
}

void TREModel::setupBFC(void)
{
	setup(TREMBFC);
}

void TREModel::setupEdges(void)
{
	setup(TREMEdgeLines);
}

void TREModel::setupConditional(void)
{
	setup(TREMConditionalLines);
}

void TREModel::setupColored(void)
{
	setupColored(TREMStandard);
}

void TREModel::setupColoredStud(void)
{
	setupColored(TREMStud);
}

void TREModel::setupColoredStudBFC(void)
{
	setupColored(TREMStudBFC);
}

void TREModel::setupColoredLines(void)
{
	setupColored(TREMLines);
}

void TREModel::setupColoredBFC(void)
{
	setupColored(TREMBFC);
}

void TREModel::setupColoredEdges(void)
{
	setupColored(TREMEdgeLines);
}

void TREModel::setupColoredConditional(void)
{
	setupColored(TREMConditionalLines);
}

void TREModel::addLine(TCULong color, const TCVector *vertices)
{
	setupColoredLines();
	m_coloredShapes[TREMLines]->addLine(color, vertices);
}

void TREModel::addLine(const TCVector *vertices)
{
	setupLines();
	m_shapes[TREMLines]->addLine(vertices);
}

void TREModel::addConditionalLine(
	const TCVector *vertices,
	const TCVector *controlPoints,
	TCULong color)
{
	// Note a color of 0 would have an alpha of 0, which would make it
	// invisible.  LDModelParser needs to be smart enough to ignore geometry
	// that's invisible.
	if (color == 0)
	{
		setup(TREMConditionalLines);
		m_shapes[TREMConditionalLines]->addConditionalLine(vertices,
			controlPoints);
	}
	else
	{
		setupColored(TREMConditionalLines);
		m_coloredShapes[TREMConditionalLines]->addConditionalLine(color,
			vertices, controlPoints);
	}
}

void TREModel::addEdgeLine(const TCVector *vertices, TCULong color)
{
	if (color == 0)
	{
		setupEdges();
		m_shapes[TREMEdgeLines]->addLine(vertices);
	}
	else
	{
		setupColoredEdges();
		m_coloredShapes[TREMEdgeLines]->addLine(color, vertices);
	}
}

void TREModel::addTriangle(TCULong color, const TCVector *vertices)
{
	setupColored();
	m_coloredShapes[TREMStandard]->addTriangle(color, vertices);
	TEXMAP_ADD_INDEX(colored.triangles, m_coloredShapes[TREMStandard],
		TRESTriangle);
	if (needDupe)
	{
		m_coloredShapes[TREMStandard]->addTriangle(color, vertices);
	}
	//TEXMAP_INCREMENT(false, colored.triangleCount);
}

void TREModel::addTriangle(
	TCULong color,
	const TCVector *vertices,
	const TCVector *normals)
{
	setupColored();
	m_coloredShapes[TREMStandard]->addTriangle(color, vertices, normals);
	TEXMAP_ADD_INDEX(colored.triangles, m_coloredShapes[TREMStandard],
		TRESTriangle);
	if (needDupe)
	{
		m_coloredShapes[TREMStandard]->addTriangle(color, vertices, normals);
	}
	//TEXMAP_INCREMENT(false, colored.triangleCount);
}

void TREModel::addTriangle(const TCVector *vertices)
{
	setupStandard();
	m_shapes[TREMStandard]->addTriangle(vertices);
	TEXMAP_ADD_INDEX(standard.triangles, m_shapes[TREMStandard], TRESTriangle);
	if (needDupe)
	{
		m_shapes[TREMStandard]->addTriangle(vertices);
	}
	//TEXMAP_INCREMENT(false, standard.triangleCount);
}

void TREModel::addTriangle(const TCVector *vertices, const TCVector *normals)
{
	setupStandard();
	m_shapes[TREMStandard]->addTriangle(vertices, normals);
	TEXMAP_ADD_INDEX(standard.triangles, m_shapes[TREMStandard], TRESTriangle);
	if (needDupe)
	{
		m_shapes[TREMStandard]->addTriangle(vertices, normals);
	}
	//TEXMAP_INCREMENT(false, standard.triangleCount);
}

void TREModel::addBFCTriangle(TCULong color, const TCVector *vertices)
{
	setupColoredBFC();
	m_coloredShapes[TREMBFC]->addTriangle(color, vertices);
	TEXMAP_ADD_INDEX(colored.triangles, m_coloredShapes[TREMBFC], TRESTriangle);
	if (needDupe)
	{
		m_coloredShapes[TREMBFC]->addTriangle(color, vertices);
	}
	//TEXMAP_INCREMENT(true, colored.triangleCount);
}

void TREModel::addBFCTriangle(
	TCULong color,
	const TCVector *vertices,
	const TCVector *normals)
{
	setupColoredBFC();
	m_coloredShapes[TREMBFC]->addTriangle(color, vertices, normals);
	TEXMAP_ADD_INDEX(colored.triangles, m_coloredShapes[TREMBFC], TRESTriangle);
	if (needDupe)
	{
		m_coloredShapes[TREMBFC]->addTriangle(color, vertices, normals);
	}
	//TEXMAP_INCREMENT(true, colored.triangleCount);
}

void TREModel::addBFCTriangle(const TCVector *vertices)
{
	setupBFC();
	m_shapes[TREMBFC]->addTriangle(vertices);
	TEXMAP_ADD_INDEX(standard.triangles, m_shapes[TREMBFC], TRESTriangle);
	if (needDupe)
	{
		m_shapes[TREMBFC]->addTriangle(vertices);
	}
	//TEXMAP_INCREMENT(true, standard.triangleCount);
}

void TREModel::addBFCTriangle(const TCVector *vertices, const TCVector *normals)
{
	setupBFC();
	m_shapes[TREMBFC]->addTriangle(vertices, normals);
	TEXMAP_ADD_INDEX(standard.triangles, m_shapes[TREMBFC], TRESTriangle);
	if (needDupe)
	{
		m_shapes[TREMBFC]->addTriangle(vertices, normals);
	}
	//TEXMAP_INCREMENT(true, standard.triangleCount);
}

TREModel::TexmapInfo *TREModel::getActiveTexmapInfo(void)
{
	const std::string *activeTextureFilename =
		m_mainModel->getActiveTextureFilename();
	if (activeTextureFilename != NULL)
	{
		if (m_texmapInfos.size() > 0)
		{
			TexmapInfo &texmapInfo = m_texmapInfos.back();

			if (texmapInfo.filename == *activeTextureFilename)
			{
				return &texmapInfo;
			}
		}
	}
	return NULL;
}

void TREModel::addQuad(TCULong color, const TCVector *vertices)
{
	setupColored();
	m_coloredShapes[TREMStandard]->addQuad(color, vertices);
	TEXMAP_ADD_INDEX(colored.quads, m_coloredShapes[TREMStandard], TRESQuad);
	if (needDupe)
	{
		m_coloredShapes[TREMStandard]->addQuad(color, vertices);
	}
	//TEXMAP_INCREMENT(false, colored.quadCount);
}

void TREModel::addQuad(const TCVector *vertices)
{
	setupStandard();
	m_shapes[TREMStandard]->addQuad(vertices);
	TEXMAP_ADD_INDEX(standard.quads, m_shapes[TREMStandard], TRESQuad);
	if (needDupe)
	{
		m_shapes[TREMStandard]->addQuad(vertices);
	}
	//TEXMAP_INCREMENT(false, standard.quadCount);
}

void TREModel::addBFCQuad(const TCVector *vertices)
{
	setupBFC();
	m_shapes[TREMBFC]->addQuad(vertices);
	TEXMAP_ADD_INDEX(standard.quads, m_shapes[TREMBFC], TRESQuad);
	if (needDupe)
	{
		m_shapes[TREMBFC]->addQuad(vertices);
	}
	//TEXMAP_INCREMENT(true, standard.quadCount);
}

void TREModel::addBFCQuad(TCULong color, const TCVector *vertices)
{
	setupColoredBFC();
	m_coloredShapes[TREMBFC]->addQuad(color, vertices);
	TEXMAP_ADD_INDEX(colored.quads, m_coloredShapes[TREMBFC], TRESQuad);
	if (needDupe)
	{
		m_coloredShapes[TREMBFC]->addQuad(color, vertices);
	}
	//TEXMAP_INCREMENT(true, colored.quadCount);
}

void TREModel::triangleStripToTriangle(
	int index,
	const TCVector *stripVertices,
	const TCVector *stripNormals,
	TCVector *triangleVertices,
	TCVector *triangleNormals)
{
	int ofs1 = 1;
	int ofs2 = 2;

	if (index % 2)
	{
		ofs1 = 2;
		ofs2 = 1;
	}
	triangleVertices[0] = stripVertices[index];
	triangleVertices[1] = stripVertices[index + ofs1];
	triangleVertices[2] = stripVertices[index + ofs2];
	triangleNormals[0] = stripNormals[index];
	triangleNormals[1] = stripNormals[index + ofs1];
	triangleNormals[2] = stripNormals[index + ofs2];
}

void TREModel::quadStripToQuad(
	int index,
	const TCVector *stripVertices,
	const TCVector *stripNormals,
	TCVector *quadVertices,
	TCVector *quadNormals)
{
	quadVertices[0] = stripVertices[index];
	quadVertices[1] = stripVertices[index + 1];
	quadVertices[2] = stripVertices[index + 3];
	quadVertices[3] = stripVertices[index + 2];
	quadNormals[0] = stripNormals[index];
	quadNormals[1] = stripNormals[index + 1];
	quadNormals[2] = stripNormals[index + 3];
	quadNormals[3] = stripNormals[index + 2];
}

void TREModel::addQuadStrip(
	const TCVector *vertices,
	const TCVector *normals,
	int count,
	bool flat)
{
	setupStandard();
	addQuadStrip(m_shapes[TREMStandard], vertices, normals, count, flat);
}

void TREModel::addTriangleStrip(
	const TCVector *vertices,
	const TCVector *normals,
	int count,
	bool flat)
{
	setupStandard();
	addTriangleStrip(m_shapes[TREMStandard], vertices, normals, count, flat);
}

void TREModel::addQuadStrip(TREShapeGroup *shapeGroup, const TCVector *vertices,
							const TCVector *normals, int count, bool flat)
{
	if (m_mainModel->getUseQuadStripsFlag() && (!flat ||
		m_mainModel->getUseFlatStripsFlag()))
	{
		shapeGroup->addQuadStrip(vertices, normals, count);
		TEXMAP_ADD_STRIP(standard.quadStrips, shapeGroup, TRESQuadStrip);
		if (needDupe)
		{
			shapeGroup->addQuadStrip(vertices, normals, count);
		}
	}
	else
	{
		int i;
		TCVector quadVertices[4];
		TCVector quadNormals[4];

		for (i = 0; i < count - 3; i += 2)
		{
			quadStripToQuad(i, vertices, normals, quadVertices, quadNormals);
			shapeGroup->addQuad(quadVertices, quadNormals);
		}
	}
}

void TREModel::addBFCQuadStrip(
	const TCVector *vertices,
	const TCVector *normals,
	int count,
	bool flat)
{
	setupBFC();
	addQuadStrip(m_shapes[TREMBFC], vertices, normals, count, flat);
}

void TREModel::addQuadStrip(TCULong color, const TCVector *vertices,
							const TCVector *normals, int count, bool flat)
{
	setupColored();
	addQuadStrip(m_coloredShapes[TREMStandard], color, vertices, normals,
		count, flat);
}

void TREModel::addTriangleStrip(
	TREShapeGroup *shapeGroup,
	const TCVector *vertices,
	const TCVector *normals,
	int count,
	bool flat)
{
	if (m_mainModel->getUseTriStripsFlag() && (!flat ||
		m_mainModel->getUseFlatStripsFlag()))
	{
		shapeGroup->addTriangleStrip(vertices, normals, count);
		TEXMAP_ADD_STRIP(standard.triStrips, shapeGroup, TRESTriangleStrip);
		if (needDupe)
		{
			shapeGroup->addTriangleStrip(vertices, normals, count);
		}
	}
	else
	{
		int i;
		TCVector triangleVertices[3];
		TCVector triangleNormals[3];

		for (i = 0; i < count - 2; i++)
		{
			triangleStripToTriangle(i, vertices, normals, triangleVertices,
				triangleNormals);
			shapeGroup->addTriangle(triangleVertices, triangleNormals);
			TEXMAP_ADD_INDEX(standard.triangles, shapeGroup, TRESTriangle);
			if (needDupe)
			{
				shapeGroup->addTriangle(triangleVertices, triangleNormals);
			}
			//TEXMAP_INCREMENT(shapeGroup == m_shapes[TREMBFC],
			//	standard.triangleCount);
		}
	}
}

void TREModel::addQuadStrip(
	TREColoredShapeGroup *shapeGroup,
	TCULong color,
	const TCVector *vertices,
	const TCVector *normals,
	int count,
	bool flat)
{
	if (m_mainModel->getUseQuadStripsFlag() && (!flat ||
		m_mainModel->getUseFlatStripsFlag()))
	{
		shapeGroup->addQuadStrip(color, vertices, normals, count);
		TEXMAP_ADD_STRIP(colored.quadStrips, shapeGroup, TRESQuadStrip);
		if (needDupe)
		{
			shapeGroup->addQuadStrip(color, vertices, normals, count);
		}
	}
	else
	{
		int i;
		TCVector quadVertices[4];
		TCVector quadNormals[4];

		for (i = 0; i < count - 3; i += 2)
		{
			quadStripToQuad(i, vertices, normals, quadVertices, quadNormals);
			shapeGroup->addQuad(color, quadVertices, quadNormals);
			TEXMAP_ADD_INDEX(standard.quads, shapeGroup, TRESQuad);
			if (needDupe)
			{
				shapeGroup->addQuad(color, quadVertices, quadNormals);
			}
			//TEXMAP_INCREMENT(shapeGroup == m_shapes[TREMBFC],
			//	standard.quadCount);
		}
	}
}

void TREModel::addBFCQuadStrip(TCULong color, const TCVector *vertices,
							   const TCVector *normals, int count, bool flat)
{
	setupColoredBFC();
	addQuadStrip(m_coloredShapes[TREMBFC], color, vertices, normals, count,
		flat);
}

void TREModel::addBFCTriangleStrip(
	const TCVector *vertices,
	const TCVector *normals,
	int count,
	bool flat)
{
	setupBFC();
	addTriangleStrip(m_shapes[TREMBFC], vertices, normals, count, flat);
}

void TREModel::triangleFanToTriangle(
	int index,
	const TCVector *fanVertices,
	const TCVector *fanNormals,
	const TCVector *fanTextureCoords,
	TCVector *triangleVertices,
	TCVector *triangleNormals,
	TCVector *triangleTextureCoords)
{
	triangleVertices[0] = fanVertices[0];
	triangleVertices[1] = fanVertices[index];
	triangleVertices[2] = fanVertices[index + 1];
	triangleNormals[0] = fanNormals[0];
	triangleNormals[1] = fanNormals[index];
	triangleNormals[2] = fanNormals[index + 1];
	if (fanTextureCoords)
	{
		triangleTextureCoords[0] = fanTextureCoords[0];
		triangleTextureCoords[1] = fanTextureCoords[index];
		triangleTextureCoords[2] = fanTextureCoords[index + 1];
	}
}

void TREModel::addTriangleFan(
	const TCVector *vertices,
	const TCVector *normals,
	const TCVector *textureCoords,
	int count,
	bool flat)
{
	TREShapeGroup *shapeGroup;

	if (textureCoords)
	{
		setupStud();
		shapeGroup = m_shapes[TREMStud];
	}
	else
	{
		setupStandard();
		shapeGroup = m_shapes[TREMStandard];
	}
	addTriangleFan(shapeGroup, vertices, normals, textureCoords, count, flat);
}

void TREModel::addTriangleFan(
	TREShapeGroup *shapeGroup,
	const TCVector *vertices,
	const TCVector *normals,
	const TCVector *textureCoords,
	int count,
	bool flat)
{
	if (m_mainModel->getUseTriFansFlag() && (!flat ||
		m_mainModel->getUseFlatStripsFlag()))
	{
		if (textureCoords)
		{
			shapeGroup->addTriangleFan(vertices, normals, textureCoords, count);
		}
		else
		{
			shapeGroup->addTriangleFan(vertices, normals, count);
		}
	}
	else
	{
		int i;
		TCVector triangleVertices[3];
		TCVector triangleNormals[3];
		TCVector triangleTextureCoords[3];

		for (i = 1; i < count - 1; i++)
		{
			triangleFanToTriangle(i, vertices, normals, textureCoords,
				triangleVertices, triangleNormals, triangleTextureCoords);
			if (textureCoords)
			{
				shapeGroup->addTriangle(triangleVertices, triangleNormals,
					triangleTextureCoords);
			}
			else
			{
				shapeGroup->addTriangle(triangleVertices, triangleNormals);
			}
		}
	}
}

void TREModel::addBFCTriangleFan(
	const TCVector *vertices,
	const TCVector *normals,
	const TCVector *textureCoords,
	int count,
	bool flat)
{
	TREShapeGroup *shapeGroup;

	if (textureCoords)
	{
		setupStudBFC();
		shapeGroup = m_shapes[TREMStudBFC];
	}
	else
	{
		setupBFC();
		shapeGroup = m_shapes[TREMBFC];
	}
	addTriangleFan(shapeGroup, vertices, normals, textureCoords, count, flat);
}

void TREModel::addTriangleFan(TCULong color, const TCVector *vertices,
							  const TCVector *normals, int count, bool flat)
{
	setupColored();
	addTriangleFan(m_coloredShapes[TREMStandard], color, vertices, normals,
		count, flat);
}

void TREModel::addTriangleFan(
	TREColoredShapeGroup *shapeGroup,
	TCULong color,
	const TCVector *vertices,
	const TCVector *normals,
	int count,
	bool flat)
{
	if (m_mainModel->getUseStripsFlag() && (!flat ||
		m_mainModel->getUseFlatStripsFlag()))
	{
		shapeGroup->addTriangleFan(color, vertices, normals, count);
	}
	else
	{
		int i;
		TCVector triangleVertices[3];
		TCVector triangleNormals[3];

		for (i = 1; i < count - 1; i++)
		{
			triangleFanToTriangle(i, vertices, normals, NULL, triangleVertices,
				triangleNormals, NULL);
			shapeGroup->addTriangle(color, triangleVertices, triangleNormals);
		}
	}
}

void TREModel::addBFCTriangleFan(TCULong color, const TCVector *vertices,
								 const TCVector *normals, int count, bool flat)
{
	setupColoredBFC();
	addTriangleFan(m_coloredShapes[TREMBFC], color, vertices, normals, count,
		flat);
}

TRESubModel *TREModel::addSubModel(
	const TCFloat *matrix,
	TREModel *model,
	bool invert)
{
	TRESubModel *subModel = new TRESubModel;
	TexmapInfo *texmapInfo = getActiveTexmapInfo();

	if (!m_subModels)
	{
		m_subModels = new TRESubModelArray;
	}
	subModel->setMatrix(matrix);
	subModel->setModel(model);
	subModel->setBFCInvertFlag(invert);
	m_subModels->addObject(subModel);
	subModel->release();
	if (texmapInfo != NULL)
	{
		texmapInfo->subModelCount++;
	}
	return subModel;
}

TRESubModel *TREModel::addSubModel(
	TCULong color,
	TCULong edgeColor,
	const TCFloat *matrix,
	TREModel *model,
	bool invert)
{
	TRESubModel *subModel = addSubModel(matrix, model, invert);

	subModel->setColor(color, edgeColor);
	return subModel;
}

void TREModel::smooth(void)
{
	TREConditionalMap conditionalMap;
	TRENormalInfoArray *normalInfos = new TRENormalInfoArray;

	fillConditionalMap(conditionalMap);
	calcShapeNormals(conditionalMap, normalInfos, TRESTriangle);
	calcShapeNormals(conditionalMap, normalInfos, TRESQuad);
	finishShapeNormals(conditionalMap);
	applyShapeNormals(normalInfos);
	normalInfos->release();
}

void TREModel::finishShapeNormals(TREConditionalMap &conditionalMap)
{
	TREConditionalMap::iterator it = conditionalMap.begin();

	while (it != conditionalMap.end())
	{
		TRESmoother &smoother0 = it->second;

		smoother0.finish();
		it++;
	}
}

void TREModel::applyShapeNormals(TRENormalInfoArray *normalInfos)
{
	int i;
	int infoCount = normalInfos->getCount();
	TCVector normal;
	TCVector vertexNormal;

	for (i = 0; i < infoCount; i++)
	{
		TRENormalInfo *normalInfo = (*normalInfos)[i];
		TREVertexArray *normals = normalInfo->m_normals;
		TRESmoother *smoother = normalInfo->m_smoother;
		TREVertex &vertex = normals->vertexAtIndex(normalInfo->m_normalIndex);
		
		vertexNormal = TCVector(vertex.v);
		normal = smoother->getNormal(normalInfo->m_smootherIndex);
		if (TRESmoother::shouldFlipNormal(normal, TCVector(vertex.v)))
		{
			normal *= -1.0f;
		}
		// The following number is the cos of n degrees, where n is half of the
		// maximum smoothing angle.  (See SMOOTH_THRESHOLD definition for
		// current value.  I don't want to calculate it on the fly.
		// We only want to apply this normal if the difference between it an the
		// original normal is less than SMOOTH_THRESHOLD degrees. If the normal
		// only applies to two faces, then the faces have to be less than
		// SMOOTH_THRESHOLD * 2 degrees apart for this to happen.  Note that
		// low-res studs have 45-degree angles between the faces, so
		// SMOOTH_THRESHOLD ideally needs to be cos of at least 25 in order to
		// have a little leeway.
		if (normal.dot(vertexNormal) > SMOOTH_THRESHOLD)
		{
			memcpy(vertex.v, (const TCFloat *)normal, 3 * sizeof(TCFloat));
		}
	}
}

void TREModel::calcShapeNormals(TREConditionalMap &conditionalMap,
								TRENormalInfoArray *normalInfos,
								TREShapeType shapeType)
{
	calcShapeNormals(conditionalMap, normalInfos, TREMStandard, shapeType);
	calcShapeNormals(conditionalMap, normalInfos, TREMBFC, shapeType);
}

void TREModel::calcShapeNormals(TREConditionalMap &conditionalMap,
								TRENormalInfoArray *normalInfos,
								TREMSection section, TREShapeType shapeType)
{
	calcShapeNormals(conditionalMap, normalInfos, m_shapes[section], shapeType);
	calcShapeNormals(conditionalMap, normalInfos, m_coloredShapes[section],
		shapeType);
}

/*
This function causes all the smoothed normals to be calculated.  It doesn't
actually apply them, but calculates their values, and records them so that they
can be later applied to the actual geometry.
*/
void TREModel::calcShapeNormals(TREConditionalMap &conditionalMap,
								TRENormalInfoArray *normalInfos,
								TREShapeGroup *shapeGroup,
								TREShapeType shapeType)
{
	if (shapeGroup)
	{
		TCULongArray *indices = shapeGroup->getIndices(shapeType);

		if (indices)
		{
			int i, j;
			int count = indices->getCount();
			int shapeSize = 3;
			TREVertexArray *vertices =
				shapeGroup->getVertexStore()->getVertices();
			TREVertexArray *normals =
				shapeGroup->getVertexStore()->getNormals();

			if (shapeType == TRESQuad)
			{
				shapeSize = 4;
			}
			for (i = 0; i < count; i += shapeSize)
			{
				for (j = 0; j < shapeSize; j++)
				{
					// Process the smooth edge between points i + j and
					// i + j + 1 (with wrap-around based on shapeSize).  Pass
					// i + j + 2 (with wrap-around) in to allow multiple
					// continuous shapes that all share one point to all get
					// smoothed together.
					processSmoothEdge(conditionalMap, normalInfos, vertices,
						normals, (*indices)[i + j],
						(*indices)[i + ((j + 1) % shapeSize)],
						(*indices)[i + ((j + 2) % shapeSize)]);
				}
			}
		}
	}
}

/*
This function looks for a conditional line between the points at index0 and
index1, and also between index1 and index2.  If it finds one between index0 and
index1, it considers that edge to be smooth, and adds the surface normal
associated with the point at index0 to the surface normal stored in the
TRESmoother associated with each of the two points at the ends of the
conditional line.  If it finds the second conditional line, it adds that line to
the TGLSmoother associated with index1.  Smoothers that are associated with
each other will later be smoothed together.
*/
void TREModel::processSmoothEdge(TREConditionalMap &conditionalMap,
								TRENormalInfoArray *normalInfos,
								 const TREVertexArray *vertices,
								 TREVertexArray *normals, int index0,
								 int index1, int index2)
{
	TRESmoother *smoother0 = NULL;
	int line0Index = getConditionalLine(conditionalMap,
		vertices->vertexAtIndex(index0), vertices->vertexAtIndex(index1),
		smoother0);

	if (line0Index >= 0)
	{
		// line0Index is the index in smoother0 of the condtional line that goes
		// between the points at index0 and index1.  Note that smoother has
		// been initialized to point to the smoother associated with the point
		// at index0.
		TRESmoother *smoother1 = NULL;
		int line1Index = getConditionalLine(conditionalMap,
			vertices->vertexAtIndex(index1), vertices->vertexAtIndex(index0),
			smoother1);

		if (smoother1)
		{
			// If we found the first conditional line (line0Index >= 0), then we
			// should always get smoother1, since smoother1 just corresponds to
			// the point at the other end of the conditional line.  Since we
			// found a conditional line, and all of them are inserted keyed off
			// of both ends, smoother1 should always be non-NULL if line0Index
			// >= 0.
			const TCVector &normal0 =
				TCVector(normals->vertexAtIndex(index0).v);
			TRENormalInfo *normalInfo;
			// Check to see if there is conditional line between the points at
			// index1 and index2.  Note that by passing smoother1 into
			// getConditionalLine with a value already set, it uses that
			// smoother, and skips the lookup.  If line2Index comes out >= 0,
			// the point will be marked as shared between the two conditionals,
			// so that it will be smoothed betwen them.
			int line2Index = getConditionalLine(conditionalMap,
				vertices->vertexAtIndex(index1),
				vertices->vertexAtIndex(index2), smoother1);

			// Check to see if the normal for index0 is more than 90 degrees
			// away from the running total normal stored in smoother0.  Note
			// that each conditional line gets its own normal unless two are
			// merged together below.
			if (TRESmoother::shouldFlipNormal(smoother0->getNormal(line0Index),
				normal0))
			{
				// Subtract shape's normal from the running total, since it is
				// more than 90 degrees off.
				smoother0->getNormal(line0Index) -= normal0;
			}
			else
			{
				// Add shape's normal to the running total.
				smoother0->getNormal(line0Index) += normal0;
			}
			normalInfo = new TRENormalInfo;
			normalInfo->m_normals = normals;
			normalInfo->m_normalIndex = index0;
			normalInfo->m_smoother = smoother0;
			normalInfo->m_smootherIndex = line0Index;
			normalInfos->addObject(normalInfo);
			normalInfo->release();
			if (line1Index >= 0)
			{
				// This should always be the case.
				const TCVector &normal1 =
					TCVector(normals->vertexAtIndex(index1).v);

				// Check to see if the normal for index1 is more than 90 degrees
				// away from the running total normal stored in smoother1.  Note
				// that each conditional line gets its own normal unless two are
				// merged together below.
				if (TRESmoother::shouldFlipNormal(smoother1->getNormal(
					line1Index), normal1))
				{
					// Subtract shape's normal from the running total, since it
					// is more than 90 degrees off.
					smoother1->getNormal(line1Index) -= normal1;
				}
				else
				{
					// Add shape's normal to the running total.
					smoother1->getNormal(line1Index) += normal1;
				}
				normalInfo = new TRENormalInfo;
				normalInfo->m_normals = normals;
				normalInfo->m_normalIndex = index1;
				normalInfo->m_smoother = smoother1;
				normalInfo->m_smootherIndex = line1Index;
				normalInfos->addObject(normalInfo);
				normalInfo->release();
			}
			if (line2Index >= 0)
			{
				smoother1->markShared(line1Index, line2Index);
			}
		}
	}
}

/*
This functions finds a conditional line in conditionalMap that goes from vertex0
to vertex1, and returns its index if it exists.  Returns -1 otherwise.  Also,
sets smoother equal to the TRESmoother that goes with point0 in conditionalMap
in order to speed up processing in the function that calls us.
*/
int TREModel::getConditionalLine(TREConditionalMap &conditionalMap,
								 const TREVertex point0, const TREVertex point1,
								 TRESmoother *&smoother)
{
	int i;
	int count;

	if (!smoother)
	{
		TREVertexKey pointKey(point0);
		TREConditionalMap::iterator it = conditionalMap.find(pointKey);

		if (it == conditionalMap.end())
		{
			return -1;
		}
		else
		{
			smoother = &(*it).second;
		}
	}
	count = smoother->getVertexCount();
	for (i = 0; i < count; i++)
	{
		// Note that by definition point0 will match the start point of the
		// smoother, so there's no need to check.
		if (smoother->getVertex(i).approxEquals(point1, 0.01f))
		{
			return i;
		}
	}
	return -1;
}

/*
This function fills conditionalMap with all the conditional lines, using the
points at each end of each conditional line as keys in the tree.  This means
that each conditional line will go into the tree twice.
*/
void TREModel::fillConditionalMap(TREConditionalMap &conditionalMap)
{
	fillConditionalMap(conditionalMap, m_shapes[TREMConditionalLines]);
	fillConditionalMap(conditionalMap, m_coloredShapes[TREMConditionalLines]);
}

void TREModel::fillConditionalMap(TREConditionalMap &conditionalMap,
								  TREShapeGroup *shapeGroup)
{
	if (shapeGroup)
	{
		TCULongArray *indices = shapeGroup->getIndices(TRESConditionalLine);

		if (indices)
		{
			int i;
			int count = indices->getCount();
			TREVertexArray *vertices =
				shapeGroup->getVertexStore()->getVertices();

			// Note there are 2 indices per conditional line; hence the += 2.
			for (i = 0; i < count; i += 2)
			{
				int index0 = (*indices)[i];
				int index1 = (*indices)[i + 1];
				const TREVertex &vertex0 = vertices->vertexAtIndex(index0);
				const TREVertex &vertex1 = vertices->vertexAtIndex(index1);
				TREVertexKey vertex0Key(vertex0);
				TREVertexKey vertex1Key(vertex1);

				if (vertex0Key < vertex1Key || vertex1Key < vertex0Key)
				{
					// Add the conditional line to the map using its first point
					// as the key in the map.
					addConditionalPoint(conditionalMap, vertices, index0,
						index1, vertex0Key);
					// Add the conditional line to the map using its second point
					// as the key in the map.
					addConditionalPoint(conditionalMap, vertices, index1,
						index0, vertex1Key);
				}
				else
				{
					TCVector length = TCVector(vertex0.v) - TCVector(vertex1.v);

					debugPrintf(2, "Conditional too short to map: %f.\n",
						length.length());
				}
			}
		}
	}
}

/*
This function adds the line between index0 and index1 to conditionalMap, using
the point at index0 as its key in the map.
*/
void TREModel::addConditionalPoint(TREConditionalMap &conditionalMap,
								   const TREVertexArray *vertices, int index0,
								   int index1, const TREVertexKey &vertexKey)
{
	TREConditionalMap::iterator it = conditionalMap.find(vertexKey);

	if (it == conditionalMap.end())
	{
		// Note that this would probably be more clear if we used the []
		// operator of map.  However, that would require extra lookups, and
		// we're trying to keep the lookups to a minimum.
		TREConditionalMap::value_type newValue(vertexKey,
			TRESmoother(vertices->vertexAtIndex(index0)));

		// The insert function returns a pair, where the first of the pair is
		// an iterator pointing to the newly inserted item, and the second of
		// the pair is a boolean saying whether or not an insertion occurred.
		// Since we got here, we know the item doesn't already exist, so we
		// really don't care about the second of the pair.
		it = conditionalMap.insert(newValue).first;
	}
	// If the item didn't originally exist, it gets set during insertion above.
	// If the item did exist, it's set in the find() call.  Note that not all
	// the vertices that match are necessarily equal.  However, this won't
	// effect the results.
	(*it).second.addVertex(vertices->vertexAtIndex(index1));
}

void TREModel::scaleConditionalControlPoint(
	int index,
	int cpIndex,
	TREVertexArray *vertices)
{
	const TREVertex &vertex = (*vertices)[index];
	TREVertex &cpVertex = (*vertices)[cpIndex];
	TCVector p1(vertex.v[0], vertex.v[1], vertex.v[2]);
	TCVector p2(cpVertex.v[0], cpVertex.v[1], cpVertex.v[2]);
	TCVector delta = p2 - p1;

	p2 = p1 + delta / delta.length();
	cpVertex.v[0] = p2[0];
	cpVertex.v[1] = p2[1];
	cpVertex.v[2] = p2[2];
}

void TREModel::scaleConditionalControlPoints(TREShapeGroup *shapeGroup)
{
	// If a conditional line's control points are outside the view frustum
	// while the conditional line itself is inside, it sometimes gets drawn
	// when it's not supposed to be.  This doesn't really fix that problem, but
	// by pulling the control points closer to the line, it makes it much less
	// likely to occur.
	if (shapeGroup != NULL)
	{
		TCULongArray *indices = shapeGroup->getIndices(TRESConditionalLine);
		TCULongArray *cpIndices = shapeGroup->getControlPointIndices();
		TREVertexArray *vertices = shapeGroup->getVertexStore()->getVertices();

		if (indices != NULL && cpIndices != NULL)
		{
			for (int i = 0; i < indices->getCount(); i += 2)
			{
				scaleConditionalControlPoint((*indices)[i], (*cpIndices)[i],
					vertices);
				scaleConditionalControlPoint((*indices)[i], (*cpIndices)[i + 1],
					vertices);
			}
		}
	}
}

void TREModel::flatten(void)
{
	if (m_subModels && m_subModels->getCount() &&
		!m_mainModel->getShowAllConditionalFlag())
	{
		flatten(this, TCVector::getIdentityMatrix(), 0, false, 0, false, false);
		if (m_subModels)
		{
			m_subModels->removeAll();
		}
		scaleConditionalControlPoints(m_shapes[TREMConditionalLines]);
		scaleConditionalControlPoints(m_coloredShapes[TREMConditionalLines]);
		m_flags.flattened = true;
	}
}

void TREModel::flatten(
	TREModel *model,
	const TCFloat *matrix,
	TCULong color,
	bool colorSet,
	TCULong edgeColor,
	bool edgeColorSet,
	bool includeShapes,
	bool skipTexmapped /*= false*/)
{
	skipTexmapped = false;
	TRESubModelArray *subModels = model->m_subModels;

	if (includeShapes)
	{
		int i;

		for (i = 0; i <= TREMLast; i++)
		{
			TREShapeGroup *otherShapeGroup = model->m_shapes[i];
			TREColoredShapeGroup *otherColoredShapeGroup =
				model->m_coloredShapes[i];

			if (otherShapeGroup)
			{
				bool actualColorSet = colorSet;
				TCULong actualColor = color;

				if (i == TREMEdgeLines || i == TREMConditionalLines)
				{
					actualColorSet = edgeColorSet;
					actualColor = edgeColor;
				}
				if (actualColorSet)
				{
					TREColoredShapeGroup *coloredShapeGroup;
					
					setupColored((TREMSection)i);
					coloredShapeGroup = m_coloredShapes[i];
					coloredShapeGroup->getVertexStore()->setupColored();
					if (i == TREMStud || i == TREMStudBFC)
					{
						coloredShapeGroup->getVertexStore()->setupTextured();
					}
					coloredShapeGroup->flatten(otherShapeGroup, matrix,
						actualColor, true, skipTexmapped);
				}
				else
				{
					TREShapeGroup *shapeGroup;

					setup((TREMSection)i);
					shapeGroup = m_shapes[i];
					if (i == TREMStud || i == TREMStudBFC)
					{
						shapeGroup->getVertexStore()->setupTextured();
					}
					shapeGroup->flatten(otherShapeGroup, matrix, 0, false,
						skipTexmapped);
				}
			}
			if (otherColoredShapeGroup)
			{
				setupColored((TREMSection)i);
				m_coloredShapes[i]->flatten(otherColoredShapeGroup, matrix, 0,
					false, skipTexmapped);
			}
		}
	}
	if (subModels)
	{
		int i;
		int count = subModels->getCount();
		TCFloat newMatrix[16];

		for (i = 0; i < count; i++)
		{
			TRESubModel *subModel = (*subModels)[i];
			bool subSkipTexmapped = skipTexmapped;

			// Note: when curve smoothing is enabled, texmapped geometry is
			// removed while the updated normals are being transferred.  It
			// cannot be removed here during the flatten, because then it won't
			// be smoothed.
			if (subModel->getTransferredFlag() &&
				!m_mainModel->getSmoothCurvesFlag())
			{
				subSkipTexmapped = true;
			}
			TCVector::multMatrix(matrix, subModel->getMatrix(), newMatrix);
			if (subModel->isColorSet())
			{
				flatten(subModel->getEffectiveModel(), newMatrix,
					htonl(subModel->getColor()), true,
					htonl(subModel->getEdgeColor()), true, true,
					subSkipTexmapped);
			}
			else
			{
				flatten(subModel->getEffectiveModel(), newMatrix, color,
					colorSet, edgeColor, edgeColorSet, true,
					subSkipTexmapped);
			}
		}
	}
}

void TREModel::setGlNormalize(bool value)
{
	if (value)
	{
		// Note: GL_RESCALE_NORMAL doesn't seem to work.
		glEnable(GL_NORMALIZE);
	}
	else
	{
		glDisable(GL_NORMALIZE);
	}
}

void TREModel::addSlopedCylinder(const TCVector& center, TCFloat radius,
								 TCFloat height, int numSegments,
								 int usedSegments, bool bfc)
{
	int vertexCount;
	TCVector *points;
	TCVector *normals;
	int i;
	TCVector top = center;
	TCVector normal = TCVector(0.0f, 1.0f, 0.0f);

	if (usedSegments == -1)
	{
		usedSegments = numSegments;
	}
	vertexCount = usedSegments * 2 + 2;
	points = new TCVector[vertexCount];
	normals = new TCVector[vertexCount];
	for (i = 0; i <= usedSegments; i++)
	{
		TCFloat angle;

		angle = 2.0f * (TCFloat)M_PI / numSegments * i;
		setCirclePoint(angle, radius, center, points[i * 2]);
		top[1] =
			center.get(1) + height - ((height / radius) * points[i * 2][0]);
		setCirclePoint(angle, radius, top, points[i * 2 + 1]);
		if (height == 0.0f)
		{
			normals[i * 2] = normal;
			normals[i * 2 + 1] = normal;
		}
		else
		{
			normals[i * 2] = (points[i * 2] - center).normalize();
			normals[i * 2 + 1] =
				(points[i * 2 + 1] - top).normalize();
		}
	}
	if (bfc)
	{
		addBFCQuadStrip(points, normals, vertexCount);
	}
	else
	{
		addQuadStrip(points, normals, vertexCount);
	}
	if (shouldLoadConditionalLines() && !fEq(height, 0.0f))
	{
		addOpenConeConditionals(points, numSegments, usedSegments);
	}
	delete[] points;
	delete[] normals;
}

void TREModel::addSlopedCylinder2(const TCVector& center, TCFloat radius,
								  TCFloat height, int numSegments,
								  int usedSegments, bool bfc)
{
	int vertexCount;
	TCVector *points;
	TCVector *normals;
	int i;
	TCVector top = center;
	TCVector normal = TCVector(0.0f, 1.0f, 0.0f);

	if (usedSegments == -1)
	{
		usedSegments = numSegments;
	}
	vertexCount = usedSegments * 2 + 2;
	points = new TCVector[vertexCount];
	normals = new TCVector[vertexCount];
	for (i = 0; i <= usedSegments; i++)
	{
		TCFloat angle;

		angle = 2.0f * (TCFloat)M_PI / numSegments * i + (TCFloat)M_PI / 2.0f;
		setCirclePoint(angle, radius, center, points[i * 2]);
		top[1] = myabs(points[i * 2][0]);
		setCirclePoint(angle, radius, top, points[i * 2 + 1]);
		if (height == 0.0f)
		{
			normals[i * 2] = normal;
			normals[i * 2 + 1] = normal;
		}
		else
		{
			normals[i * 2] = (points[i * 2] - center).normalize();
			normals[i * 2 + 1] =
				(points[i * 2 + 1] - top).normalize();
		}
	}
	if (bfc)
	{
		addBFCQuadStrip(points, normals, vertexCount);
	}
	else
	{
		addQuadStrip(points, normals, vertexCount);
	}
	if (shouldLoadConditionalLines() && !fEq(height, 0.0f))
	{
		addSlopedCylinder2Conditionals(points, numSegments, usedSegments);
	}
	delete[] points;
	delete[] normals;
}

void TREModel::addCylinder(
	const TCVector& center,
	TCFloat radius,
	TCFloat height,
	int numSegments,
	int usedSegments,
	bool bfc,
	TCULong color,
	TCULong edgeColor)
{
	addOpenCone(center, radius, radius, height, numSegments, usedSegments, bfc,
		color, edgeColor);
}

void TREModel::addStudDisc(const TCVector &center, TCFloat radius,
						   int numSegments, int usedSegments, bool bfc)
{
	addDisc(center, radius, numSegments, usedSegments, bfc,
		m_mainModel->getStudLogoFlag());
}

void TREModel::genStudTextureCoords(TCVector *textureCoords, int vertexCount)
{
	int i;
	TCVector p1;
	TCVector offset = TCVector(0.5f, 0.5f, 0.0f);
	int numSegments = vertexCount - 2;

	textureCoords[0] = TCVector(0.5f, 0.5f, 0.0f);
	for (i = 1; i < vertexCount; i++)
	{
		TCFloat x, z;
		TCFloat angle;

		angle = 2.0f * (TCFloat)M_PI / numSegments * (i - 1);
		x = (TCFloat)cos(angle) * 0.5f;
		z = (TCFloat)sin(angle) * 0.5f;
		p1[0] = z;
		p1[1] = x;
		p1 += offset;
		p1[1] = 1.0f - p1[1];
		textureCoords[i] = p1;
	}
}

void TREModel::addChrd(const TCVector &center, TCFloat radius, int numSegments,
					   int usedSegments, bool bfc)
{
	int vertexCount;
	TCVector *points;
	TCVector *normals;
	int i;
	TCVector normal = TCVector(0.0f, -1.0f, 0.0f);

	if (usedSegments == -1)
	{
		usedSegments = numSegments;
	}
	vertexCount = usedSegments + 1;
	points = new TCVector[vertexCount];
	normals = new TCVector[vertexCount];
	for (i = 0; i <= usedSegments; i++)
	{
		TCFloat angle;

		angle = 2.0f * (TCFloat)M_PI / numSegments * i;
		setCirclePoint(angle, radius, center, points[i]);
		normals[i] = normal;
	}
	if (bfc)
	{
		addBFCTriangleFan(points, normals, NULL, vertexCount, true);
	}
	else
	{
		addTriangleFan(points, normals, NULL, vertexCount, true);
	}
	delete[] points;
	delete[] normals;
}

void TREModel::addDisc(const TCVector &center, TCFloat radius, int numSegments,
					   int usedSegments, bool bfc, bool stud)
{
	int vertexCount;
	TCVector *points;
	TCVector *normals;
	TCVector *textureCoords = NULL;
	int i;
	TCVector normal = TCVector(0.0f, -1.0f, 0.0f);

	if (usedSegments == -1)
	{
		usedSegments = numSegments;
	}
	vertexCount = usedSegments + 2;
	points = new TCVector[vertexCount];
	normals = new TCVector[vertexCount];
	points[0] = center;
	normals[0] = normal;
	for (i = 0; i <= usedSegments; i++)
	{
		TCFloat angle;

		angle = 2.0f * (TCFloat)M_PI / numSegments * i;
		setCirclePoint(angle, radius, center, points[i + 1]);
		normals[i + 1] = normal;
	}
	if (stud && TREMainModel::getStudTextures())
	{
		textureCoords = new TCVector[vertexCount];
		genStudTextureCoords(textureCoords, vertexCount);
	}
	if (bfc)
	{
		addBFCTriangleFan(points, normals, textureCoords, vertexCount, true);
	}
	else
	{
		addTriangleFan(points, normals, textureCoords, vertexCount, true);
	}
	delete[] points;
	delete[] normals;
	delete[] textureCoords;
}

void TREModel::addNotDisc(
	const TCVector &center,
	TCFloat radius,
	int numSegments,
	int usedSegments,
	bool bfc)
{
	int quarter = numSegments / 4;
	int numQuarters;
	int i, j;
	TCVector normal = TCVector(0.0f, -1.0f, 0.0f);
	TCVector p1;

	if (usedSegments == -1)
	{
		usedSegments = numSegments;
	}
	numQuarters = (usedSegments + quarter - 1) / quarter;
	for (i = 0; i < numQuarters; i++)
	{
		TCVector *points;
		TCVector *normals;
		int vertexCount;
		int quarterSegments = quarter;
		TCFloat zMult = 1.0f;
		TCFloat xMult = 1.0f;

		if (i >= 2)
		{
			zMult = -1.0f;
		}
		if (i == 1 || i == 2)
		{
			xMult = -1.0f;
		}
		if (i == numQuarters - 1)
		{
			quarterSegments = usedSegments % quarter;
			if (!quarterSegments)
			{
				quarterSegments = quarter;
			}
		}
		vertexCount = quarterSegments + 2;
		points = new TCVector[vertexCount];
		normals = new TCVector[vertexCount];
		points[0] = center + TCVector(xMult * radius, 0.0f, zMult * radius);
		normals[0] = normal;
		for (j = 0; j <= quarterSegments; j++)
		{
			TCFloat x, z;
			TCFloat angle;

			angle = 2.0f * (TCFloat)M_PI / numSegments * (j + i * quarter);
			x = radius * (TCFloat)cos(angle);
			z = radius * (TCFloat)sin(angle);
			p1[0] = center.get(0) + x;
			p1[2] = center.get(2) + z;
			p1[1] = center.get(1);
			points[quarterSegments - j + 1] = p1;
			normals[quarterSegments - j + 1] = normal;
		}
		if (bfc)
		{
			addBFCTriangleFan(points, normals, NULL, vertexCount, true);
		}
		else
		{
			addTriangleFan(points, normals, NULL, vertexCount, true);
		}
		delete[] points;
		delete[] normals;
	}
}

void TREModel::addTangent(
	const TCVector &center,
	TCFloat radius,
	int numSegments,
	int usedSegments,
	bool bfc)
{
	int quarter = numSegments / 4;
	int trianglesPerChunk = quarter / 4;
	int numQuarters;
	int i, j;

	if (usedSegments == -1)
	{
		usedSegments = numSegments;
	}
	numQuarters = (usedSegments + quarter - 1) / quarter;
	TCVector points[3];
	TCVector normals[3] =
	{
		TCVector(0.0f, -1.0f, 0.0f),
		TCVector(0.0f, -1.0f, 0.0f),
		TCVector(0.0f, -1.0f, 0.0f)
	};
	for (i = 0; i < numQuarters; i++)
	{
		int quarterSegments = quarter;
		TCFloat zMult = 1.0f;
		TCFloat xMult = 1.0f;

		if (i >= 2)
		{
			zMult = -1.0f;
		}
		if (i == 1 || i == 2)
		{
			xMult = -1.0f;
		}
		TCVector corners[4] =
		{
			TCVector(1.0 * xMult, 0.0, 0.1989 * zMult),
			TCVector(0.8478 * xMult, 0.0, 0.5665 * zMult),
			TCVector(0.5665 * xMult, 0.0, 0.8478 * zMult),
			TCVector(0.1989 * xMult, 0.0, 1.0 * zMult)
		};
		if (i == numQuarters - 1)
		{
			quarterSegments = usedSegments % quarter;
			if (!quarterSegments)
			{
				quarterSegments = quarter;
			}
		}
		for (j = 0; j < quarterSegments; j++)
		{
			points[0] = center + corners[j / trianglesPerChunk];
			TCFloat x, z;
			TCFloat angle;
			int angleIndex1 = j + 1;
			int angleIndex2 = j;

			if (i == 1 || i == 3)
			{
				angleIndex1 = quarterSegments - j;
				angleIndex2 = quarterSegments - j - 1;
			}
			angle = 2.0f * (TCFloat)M_PI / numSegments * (angleIndex1 + i * quarter);
			x = radius * (TCFloat)cos(angle);
			z = radius * (TCFloat)sin(angle);
			points[1][0] = center.get(0) + x;
			points[1][2] = center.get(2) + z;
			points[1][1] = center.get(1);
			angle = 2.0f * (TCFloat)M_PI / numSegments * (angleIndex2 + i * quarter);
			x = radius * (TCFloat)cos(angle);
			z = radius * (TCFloat)sin(angle);
			points[2][0] = center.get(0) + x;
			points[2][2] = center.get(2) + z;
			points[2][1] = center.get(1);
			if (bfc)
			{
				addBFCTriangle(points, normals);
			}
			else
			{
				addTriangle(points, normals);
			}
		}
	}
}

void TREModel::setCirclePoint(
	TCFloat angle,
	TCFloat radius,
	const TCVector& center,
	TCVector& point)
{
	TCFloat x1, z1;

	x1 = radius * (TCFloat)cos(angle);
	z1 = radius * (TCFloat)sin(angle);
	point[0] = center.get(0) + x1;
	point[1] = center.get(1);
	point[2] = center.get(2) + z1;
}

void TREModel::addCone(
	const TCVector &center,
	TCFloat radius,
	TCFloat height,
	int numSegments,
	int usedSegments,
	bool bfc,
	TCULong color,
	TCULong edgeColor)
{
	int i;
	TCVector top = center;
	TCVector p1, p2, p3;
	TCVector linePoints[2];
	TCVector controlPoints[2];
	TCVector *points = new TCVector[3];
	TCVector *normals = new TCVector[3];
	TCVector tri0Cross;
	TCVector tri1Cross;
	TCVector tri2Cross;
	int axis1 = 2;

	if (usedSegments == -1)
	{
		usedSegments = numSegments;
	}
	top[1] += height;
	points[2] = top;
	for (i = 0; i < usedSegments; i++)
	{
		TCFloat angle0, angle1, angle2, angle3;

		angle0 = 2.0f * (TCFloat)M_PI / numSegments * (i - 1);
		angle1 = 2.0f * (TCFloat)M_PI / numSegments * i;
		angle2 = 2.0f * (TCFloat)M_PI / numSegments * (i + 1);
		angle3 = 2.0f * (TCFloat)M_PI / numSegments * (i + 2);
		setCirclePoint(angle1, radius, center, p1);
		setCirclePoint(angle2, radius, center, p2);
		points[0] = p2;
		points[1] = p1;
		if (i == 0)
		{
			setCirclePoint(angle0, radius, center, p3);
			tri0Cross = (p3 - top) * (p3 - p1);
			tri1Cross = (p1 - top) * (p1 - p2);
			if (shouldLoadConditionalLines())
			{
				linePoints[0] = p1;
				linePoints[1] = top;
				controlPoints[0] = p1;
				controlPoints[0][axis1] -= 1.0f;
				controlPoints[1] = p2;
				addConditionalLine(linePoints, controlPoints, edgeColor);
			}
		}
		else
		{
			tri0Cross = tri1Cross;
			tri1Cross = tri2Cross;
		}
		normals[1] = tri1Cross + tri0Cross;
		normals[1].normalize();
		setCirclePoint(angle3, radius, center, p3);
		if (shouldLoadConditionalLines())
		{
			linePoints[0] = p2;
			linePoints[1] = top;
			controlPoints[0] = p1;
			if (i == usedSegments - 1)
			{
				controlPoints[1] = p2;
				calcTangentControlPoint(controlPoints[1], i + 1, numSegments);
			}
			else
			{
				controlPoints[1] = p3;
			}
			addConditionalLine(linePoints, controlPoints, edgeColor);
		}
		tri2Cross = (p2 - top) * (p2 - p3);
		normals[0] = tri2Cross + tri1Cross;
		normals[0].normalize();
		normals[2] = tri1Cross;
		normals[2].normalize();
		if (bfc)
		{
			if (color == 0)
			{
				addBFCTriangle(points, normals);
			}
			else
			{
				addBFCTriangle(color, points, normals);
			}
		}
		else
		{
			if (color == 0)
			{
				addTriangle(points, normals);
			}
			else
			{
				addTriangle(color, points, normals);
			}
		}
	}
	delete[] points;
	delete[] normals;
}

TCVector TREModel::calcIntersection(int i, int j, int num,
									TCVector* zeroXPoints,
									TCVector* zeroYPoints,
									TCVector* zeroZPoints)
{
	TCVector temp1, temp2, temp3, temp4, temp5, temp6;
	TCVector p1, p2, p3;

	if (i + j == num)
	{
		return zeroXPoints[j];
	}
	else if (i == 0)
	{
		return zeroZPoints[num - j];
	}
	else if (j == 0)
	{
		return zeroYPoints[i];
	}
	temp1 = zeroYPoints[i];
	temp2 = zeroXPoints[num - i];
	temp3 = zeroZPoints[num - j];
	temp4 = zeroXPoints[j];
	temp5 = zeroYPoints[i + j];
	temp6 = zeroZPoints[num - i - j];
	return (temp1 + temp2 + temp3 + temp4 + temp5 + temp6 -
		zeroXPoints[0] - zeroYPoints[0] - zeroZPoints[0]) / 9.0f;
}

void TREModel::addTorusIO(bool inner, const TCVector& center, TCFloat yRadius,
						  TCFloat xzRadius, int numSegments, int usedSegments,
						  int minorSegments, bool bfc)
{
	int i, j;
	TCVector p1, p2;
	TCVector top = center;
	int ySegments = minorSegments / 4;
	TCVector *points;
	TCVector *stripPoints;
	TCVector *stripNormals;
	int spot;
	int stripSize = (ySegments + 1) * 2;

	points = new TCVector[(ySegments + 1) * (usedSegments + 1)];
	stripPoints = new TCVector[stripSize];
	stripNormals = new TCVector[stripSize];
	for (i = 0; i <= usedSegments; i++)
	{
		TCFloat xzAngle;	// Angle in the xz plane

		xzAngle = 2.0f * (TCFloat)M_PI / numSegments * i;
		for (j = 0; j <= ySegments; j++)
		{
			TCFloat yAngle; // Angle above the xz plane
			TCFloat currentRadius;

			if (inner)
			{
				yAngle = (TCFloat)M_PI - 2.0f * (TCFloat)M_PI /
					minorSegments * j;
			}
			else
			{
				yAngle = 2.0f * (TCFloat)M_PI / minorSegments * j;
			}
			top[1] = xzRadius * (TCFloat)sin(yAngle) + center.get(1);
			currentRadius = xzRadius * (TCFloat)cos(yAngle) + yRadius;
			setCirclePoint(xzAngle, currentRadius, top, p1);
			points[i * (ySegments + 1) + j] = p1;
		}
	}
	top = center;
	for (i = 0; i < usedSegments; i++)
	{
		TCFloat xzAngle;	// Angle in the xz plane
		int ofs1 = 1;
		int ofs2 = 0;

		if (inner)
		{
			ofs1 = 0;
			ofs2 = 1;
		}

		xzAngle = 2.0f * (TCFloat)M_PI / numSegments * (i + ofs2);
		setCirclePoint(xzAngle, yRadius, top, p1);
		xzAngle = 2.0f * (TCFloat)M_PI / numSegments * (i + ofs1);
		setCirclePoint(xzAngle, yRadius, top, p2);
		spot = 0;
		for (j = 0; j <= ySegments; j++)
		{
			stripPoints[spot] = points[(i + ofs1) * (ySegments + 1) + j];
			stripNormals[spot] = (stripPoints[spot] - p2).normalize();
			spot++;
			stripPoints[spot] = points[(i + ofs2) * (ySegments + 1) + j];
			stripNormals[spot] = (stripPoints[spot] - p1).normalize();
			spot++;
		}
		if (m_mainModel->getSmoothCurvesFlag())
		{
			m_mainModel->setDisableStrips(true);
		}
		if (bfc)
		{
			addBFCQuadStrip(stripPoints, stripNormals, stripSize);
		}
		else
		{
			addQuadStrip(stripPoints, stripNormals, stripSize);
		}
		m_mainModel->setDisableStrips(false);
	}
	if (shouldLoadConditionalLines())
	{
		addTorusIOConditionals(inner, points, numSegments, usedSegments,
			minorSegments, center, yRadius, xzRadius);
	}
	delete[] stripPoints;
	delete[] stripNormals;
	delete[] points;
}

void TREModel::addTorusIOConditionals(bool inner, TCVector *points,
									  int numSegments, int usedSegments,
									  int minorSegments, const TCVector& center,
									  TCFloat radius, TCFloat height)
{
	int i, j;
	TCVector p1, p2, p3, p4;
	TCVector top = center;
	top[1] = height;
	int ySegments = minorSegments / 4;

	if ((inner && height > 0.0f) || (!inner && height < 0.0f))
	{
		radius += 0.1f;
	}
	else
	{
		radius -= 0.1f;
	}
	for (i = 0; i <= usedSegments; i++)
	{
		for (j = 0; j < ySegments; j++)
		{
			p1 = points[i * (ySegments + 1) + j];
			p2 = points[i * (ySegments + 1) + j + 1];
			if (i == 0)
			{
				p3 = p1;
				p3[2] -= 0.1f;
			}
			else
			{
				p3 = points[(i - 1) * (ySegments + 1) + j];
			}
			if (i == usedSegments)
			{
				p4 = p1;
				calcTangentControlPoint(p4, i, numSegments);
			}
			else
			{
				p4 = points[(i + 1) * (ySegments + 1) + j];
			}
			addConditionalLine(p1, p2, p3, p4);
		}
	}
	for (i = 0; i < usedSegments; i++)
	{
		for (j = 0; j <= ySegments; j++)
		{
			p1 = points[i * (ySegments + 1) + j];
			p2 = points[(i + 1) * (ySegments + 1) + j];
			if (j == 0)
			{
				p3 = p1;
				if (height > 0.0f)
				{
					p3[1] -= 0.1f;
				}
				else
				{
					p3[1] += 0.1f;
				}
			}
			else
			{
				p3 = points[i * (ySegments + 1) + (j - 1)];
			}
			if (j == ySegments)
			{
				TCFloat angle = 2.0f * (TCFloat)M_PI / numSegments * i;

				setCirclePoint(angle, radius, top, p4);
				p4[1] = height;
			}
			else
			{
				p4 = points[i * (ySegments + 1) + (j + 1)];
			}
			addConditionalLine(p1, p2, p3, p4);
		}
	}
}

void TREModel::addEighthSphere(const TCVector& center, TCFloat radius,
							   int numSegments, bool bfc)
{
	TCVector* zeroXPoints;
	TCVector* zeroYPoints;
	TCVector* zeroZPoints;
	int usedSegments = numSegments / 4;
	int i, j;
	TCVector p1, p2, p3;
	TCVector *spherePoints = NULL;
	int numMainPoints = (usedSegments + 1) * (usedSegments + 1) - 1;
	int mainSpot = 0;
	bool shouldLoadConditionals = shouldLoadConditionalLines();

	if (shouldLoadConditionals)
	{
		spherePoints = new TCVector[numMainPoints];
	}
	zeroXPoints = new TCVector[usedSegments + 1];
	zeroYPoints = new TCVector[usedSegments + 1];
	zeroZPoints = new TCVector[usedSegments + 1];
	for (i = 0; i <= usedSegments; i++)
	{
		TCFloat angle = 2.0f * (TCFloat)M_PI / numSegments * i;

		zeroYPoints[i][0] = 1.0f / ((TCFloat)tan(angle) + 1);
		zeroYPoints[i][1] = 0.0f;
		zeroYPoints[i][2] = 1.0f - zeroYPoints[i][0];
		zeroZPoints[i] = zeroYPoints[i].rearrange(2, 0, 1);
		zeroXPoints[i] = zeroYPoints[i].rearrange(1, 2, 0);
		zeroXPoints[i] += center;
		zeroYPoints[i] += center;
		zeroZPoints[i] += center;
	}
	for (j = 0; j < usedSegments; j++)
	{
		int stripCount = usedSegments - j;
		int stripSpot = 0;
		TCVector *points = new TCVector[stripCount * 2 + 1];
		TCVector *normals = new TCVector[stripCount * 2 + 1];

		for (i = 0; i < stripCount; i++)
		{
			if (i == 0)
			{
				p1 = calcIntersection(i, j, usedSegments, zeroXPoints,
					zeroYPoints, zeroZPoints);
				p1 *= radius / p1.length();
				normals[stripSpot] = (p1 - center).normalize();
				points[stripSpot++] = p1;
				if (shouldLoadConditionals)
				{
					spherePoints[mainSpot++] = p1;
				}
			}
			p2 = calcIntersection(i, j + 1, usedSegments, zeroXPoints,
				zeroYPoints, zeroZPoints);
			p2 *= radius / p2.length();
			p3 = calcIntersection(i + 1, j, usedSegments, zeroXPoints,
				zeroYPoints, zeroZPoints);
			p3 *= radius / p3.length();
			normals[stripSpot] = (p2 - center).normalize();
			points[stripSpot++] = p2;
			normals[stripSpot] = (p3 - center).normalize();
			points[stripSpot++] = p3;
			if (shouldLoadConditionals)
			{
				spherePoints[mainSpot++] = p2;
				spherePoints[mainSpot++] = p3;
			}
		}
		if (m_mainModel->getSmoothCurvesFlag())
		{
			m_mainModel->setDisableStrips(true);
		}
		if (bfc)
		{
			addBFCTriangleStrip(points, normals, stripSpot);
		}
		else
		{
			addTriangleStrip(points, normals, stripSpot);
		}
		m_mainModel->setDisableStrips(false);
		delete[] points;
		delete[] normals;
	}
	if (shouldLoadConditionals)
	{
		addEighthSphereConditionals(spherePoints, numSegments);
	}
	delete[] spherePoints;
	delete[] zeroXPoints;
	delete[] zeroYPoints;
	delete[] zeroZPoints;
}

void TREModel::addConditionalLine(const TCVector &p1, const TCVector &p2,
								  const TCVector &c1, const TCVector &c2)
{
	TCVector points[2];
	TCVector conditionalPoints[2];

	points[0] = p1;
	points[1] = p2;
	conditionalPoints[0] = c1;
	conditionalPoints[1] = c2;
	addConditionalLine(points, conditionalPoints);
}

void TREModel::addEighthSphereConditionals(TCVector *points, int numSegments)
{
	int usedSegments = numSegments / 4;
	int i, j;
	TCVector p1, p2, p3, p4;
	TCVector circlePoint;
	int mainSpot = 0;

	for (j = 0; j < usedSegments; j++)
	{
		int stripCount = usedSegments - j;
		
		for (i = 0; i < stripCount; i++)
		{
			if (i > 0)
			{
				p3 = points[mainSpot - 1];
			}
			else
			{
				if (j > 0)
				{
					p3 = points[mainSpot];
					p3[2] -= 0.1f;
				}
				else
				{
					p3 = points[mainSpot];
					p3[2] -= 0.1f;
				}
			}
			p4 = points[mainSpot + 2];
			p1 = points[mainSpot];
			p2 = points[mainSpot + 1];
			addConditionalLine(p1, p2, p3, p4);
			p3 = p1;
			p1 = p2;
			p2 = p4;
			if (i < stripCount - 1)
			{
				p4 = points[mainSpot + 3];
			}
			else
			{
				p4 = points[mainSpot + 1];
				p4[0] -= 0.1f;
			}
			addConditionalLine(p1, p2, p3, p4);
			p1 = points[mainSpot];
			p2 = points[mainSpot + 2];
			p3 = points[mainSpot + 1];
			if (j == 0)
			{
				p4 = points[mainSpot];
				p4[1] -= 0.1f;
			}
			else
			{
				p4 = points[sphereIndex(i * 2 + 2, j - 1, usedSegments)];
			}
			addConditionalLine(p1, p2, p3, p4);
			mainSpot += 2;
		}
		mainSpot++;
	}
}

int TREModel::sphereIndex(int i, int j, int usedSegments)
{
	int retVal = 0;
	int k;

	for (k = 0; k < j; k++)
	{
		int rowSize = usedSegments - k;

		retVal += rowSize * 2 + 1;
	}
	return retVal + i;
}

void TREModel::addOpenCone(
	const TCVector& center,
	TCFloat radius1,
	TCFloat radius2,
	TCFloat height,
	int numSegments,
	int usedSegments,
	bool bfc,
	TCULong color,
	TCULong edgeColor)
{
	if (usedSegments == -1)
	{
		usedSegments = numSegments;
	}
	if (radius1 == 0.0f)
	{
		addCone(center, radius2, height, numSegments, usedSegments, bfc);
	}
	else if (radius2 == 0.0f)
	{
		addCone(center, radius1, height, numSegments, usedSegments, bfc);
	}
	else
	{
		int vertexCount = usedSegments * 2 + 2;
		TCVector *points = new TCVector[vertexCount];
		TCVector *normals = new TCVector[vertexCount];
		int i;
		TCVector top = center;
		TCVector normal = TCVector(0.0f, -1.0f, 0.0f);
		TCVector topNormalPoint;
		TCVector normalPoint;
		TCFloat normalAdjust = 1.0f;

		if (height < 0.0f)
		{
			normalAdjust = -1.0f;
		}
		top[1] += height;
		if (height)
		{
			topNormalPoint = top - normal * radius2 * (radius2 - radius1) /
				height;
			normalPoint = center - normal * radius1 * (radius2 - radius1) /
				height;
		}
		for (i = 0; i <= usedSegments; i++)
		{
			TCFloat angle;

			angle = 2.0f * (TCFloat)M_PI / numSegments * i;
			setCirclePoint(angle, radius1, center, points[i * 2]);
			setCirclePoint(angle, radius2, top, points[i * 2 + 1]);
			if (height == 0.0f)
			{
				normals[i * 2] = normal;
				normals[i * 2 + 1] = normal;
			}
			else
			{
				normals[i * 2] = (points[i * 2] - normalPoint).normalize() *
					normalAdjust;
				normals[i * 2 + 1] =
					(points[i * 2 + 1] - topNormalPoint).normalize() *
					normalAdjust;
			}
		}
		if (m_mainModel->getSmoothCurvesFlag())
		{
			m_mainModel->setDisableStrips(true);
		}
		if (bfc)
		{
			if (color == 0)
			{
				addBFCQuadStrip(points, normals, vertexCount, height == 0.0f);
			}
			else
			{
				addBFCQuadStrip(color, points, normals, vertexCount,
					height == 0.0f);
			}
		}
		else
		{
			if (color == 0)
			{
				addQuadStrip(points, normals, vertexCount, height == 0.0f);
			}
			else
			{
				addQuadStrip(color, points, normals, vertexCount,
					height == 0.0f);
			}
		}
		m_mainModel->setDisableStrips(false);
		if (shouldLoadConditionalLines() && !fEq(height, 0.0f))
		{
			addOpenConeConditionals(points, numSegments, usedSegments,
				edgeColor);
		}
		delete[] points;
		delete[] normals;
	}
}

void TREModel::calcTangentControlPoint(TCVector &controlPoint, int index,
									   int numSegments)
{
	// The next control point needs to form a tangent with the circle from the
	// last point on the circle.  On input, controlPoint starts as the last
	// point on the circle.
	TCFloat angle;

	// First, calculate the angle for the last point on the circle.
	angle = 2.0f * (TCFloat)M_PI / numSegments * index;
	// Next, add 90 degrees to that to get the tangent angle
	angle += (TCFloat)deg2rad(90);
	controlPoint[0] += (TCFloat)cos(angle) * 0.1f;
	controlPoint[2] += (TCFloat)sin(angle) * 0.1f;
}

void TREModel::addOpenConeConditionals(
	TCVector *points,
	int numSegments,
	int usedSegments,
	TCULong color)
{
	int i;
	TCVector controlPoints[2];
	TCVector *p1;
	TCVector *p2;

	for (i = 0; i <= usedSegments; i++)
	{
		p1 = &points[i * 2];
		p2 = &points[i * 2 + 1];
		if (*p1 == *p2)
		{
			continue;
		}
		if (i == 0)
		{
			if (numSegments == usedSegments)
			{
				controlPoints[0] = points[numSegments * 2 - 2];
			}
			else
			{
				controlPoints[0] = *p1;
				controlPoints[0][2] -= 1.0f;
			}
		}
		else
		{
			controlPoints[0] = points[(i - 1) * 2];
		}
		if (i == usedSegments)
		{
			if (numSegments == usedSegments)
			{
				// No need to repeat the last one if it's a closed surface.
				return;
			}
			else
			{
				controlPoints[1] = *p1;
				calcTangentControlPoint(controlPoints[1], i, numSegments);
			}
		}
		else
		{
			controlPoints[1] = points[(i + 1) * 2];
		}
		addConditionalLine(p1, controlPoints, color);
	}
}

void TREModel::addSlopedCylinder2Conditionals(TCVector *points,
											  int numSegments, int usedSegments)
{
	int i;
	TCVector linePoints[2];
	TCVector controlPoints[2];
	TCVector *p1;
	TCVector *p2;

	for (i = 1; i <= usedSegments; i++)
	{
		linePoints[0] = points[i * 2];
		linePoints[1] = points[i * 2 + 1];
		p1 = &linePoints[0];
		p2 = &linePoints[1];
		if (p1 == p2)
		{
			continue;
		}
		controlPoints[0] = points[(i - 1) * 2];
		if (i == usedSegments)
		{
			if (numSegments == usedSegments)
			{
				controlPoints[1] = points[2];
			}
			else
			{
				controlPoints[1] = *p1;
				// This primitive starts at a different angle, so we want the
				// tangent that is 90 degrees further around vs. what would
				// normally be the tangent for the given index, so add
				// numSegments / 4 to do the calculation for 90 degrees further.
				calcTangentControlPoint(controlPoints[1], i + numSegments / 4,
					numSegments);
			}
		}
		else
		{
			controlPoints[1] = points[(i + 1) * 2];
		}
		addConditionalLine(linePoints, controlPoints);
	}
}

void TREModel::addCircularEdge(
	const TCVector& center,
	TCFloat radius,
	int numSegments,
	int usedSegments,
	TCULong color)
{
	int i;
	TCVector p1;
	TCVector *allPoints;
	TCVector points[2];

	if (usedSegments == -1)
	{
		usedSegments = numSegments;
	}
	allPoints = new TCVector[usedSegments + 1];
	for (i = 0; i <= usedSegments; i++)
	{
		TCFloat x, z;
		TCFloat angle;

		angle = 2.0f * (TCFloat)M_PI / numSegments * i;
		x = radius * (TCFloat)cos(angle);
		z = radius * (TCFloat)sin(angle);
		p1[0] = center.get(0) + x;
		p1[2] = center.get(2) + z;
		p1[1] = center.get(1);
		allPoints[i] = p1;
	}
	for (i = 0; i < usedSegments; i++)
	{
		points[0] = allPoints[i];
		points[1] = allPoints[i + 1];
		addEdgeLine(points, color);
	}
	delete[] allPoints;
}

void TREModel::addRing(const TCVector& center, TCFloat radius1, TCFloat radius2,
					   int numSegments, int usedSegments, bool bfc)
{
	addOpenCone(center, radius1, radius2, 0.0f, numSegments, usedSegments,
		bfc);
}

void TREModel::calculateBoundingBox(void)
{
	if (!m_flags.boundingBox)
	{
		m_boundingMin[0] = 1e10f;
		m_boundingMin[1] = 1e10f;
		m_boundingMin[2] = 1e10f;
		m_boundingMax[0] = -1e10f;
		m_boundingMax[1] = -1e10f;
		m_boundingMax[2] = -1e10f;
		scanPoints(this,
			(TREScanPointCallback)&TREModel::scanBoundingBoxPoint,
			TCVector::getIdentityMatrix());
		m_flags.boundingBox = true;
	}
}

void TREModel::scanPoints(TCObject *scanner,
						  TREScanPointCallback scanPointCallback,
						  const TCFloat *matrix)
{
	int i;

	for (i = 0; i <= TREMLast; i++)
	{
		TREShapeGroup *shapeGroup = m_shapes[i];

		if (shapeGroup)
		{
			shapeGroup->scanPoints(scanner, scanPointCallback, matrix);
		}
		shapeGroup = m_coloredShapes[i];
		if (shapeGroup)
		{
			shapeGroup->scanPoints(scanner, scanPointCallback, matrix);
		}
	}
	if (m_subModels)
	{
		int count = m_subModels->getCount();

		for (i = 0; i < count; i++)
		{
			(*m_subModels)[i]->scanPoints(scanner, scanPointCallback, matrix);
		}
	}
}

void TREModel::unshrinkNormals(
	const TCFloat *matrix,
	const TCFloat *unshrinkMatrix)
{
	int i;

	for (i = 0; i <= TREMLast; i++)
	{
		if (!isLineSection(i))
		{
			TREShapeGroup *shapeGroup = m_shapes[i];

			if (shapeGroup)
			{
				shapeGroup->unshrinkNormals(matrix, unshrinkMatrix);
			}
			shapeGroup = m_coloredShapes[i];
			if (shapeGroup)
			{
				shapeGroup->unshrinkNormals(matrix, unshrinkMatrix);
			}
		}
	}
	if (m_subModels)
	{
		int count = m_subModels->getCount();

		for (i = 0; i < count; i++)
		{
			(*m_subModels)[i]->unshrinkNormals(matrix, unshrinkMatrix);
		}
	}
}

void TREModel::getBoundingBox(TCVector& min, TCVector& max)
{
	if (!m_flags.boundingBox)
	{
		calculateBoundingBox();
	}
	min = m_boundingMin;
	max = m_boundingMax;
}

void TREModel::scanBoundingBoxPoint(const TCVector &point)
{
	if (point.get(0) < m_boundingMin[0])
	{
		m_boundingMin[0] = point.get(0);
	}
	if (point.get(1) < m_boundingMin[1])
	{
		m_boundingMin[1] = point.get(1);
	}
	if (point.get(2) < m_boundingMin[2])
	{
		m_boundingMin[2] = point.get(2);
	}
	if (point.get(0) > m_boundingMax[0])
	{
		m_boundingMax[0] = point.get(0);
	}
	if (point.get(1) > m_boundingMax[1])
	{
		m_boundingMax[1] = point.get(1);
	}
	if (point.get(2) > m_boundingMax[2])
	{
		m_boundingMax[2] = point.get(2);
	}
}

// When you shrink a part, all the normals end up getting lengthened by an
// amount that is based on their direction and the magnitude of the shrinkage
// matrix.  If that isn't adjusted for, then all normals have to be normalized
// by OpenGL, which slows things down.  This functions shortens all the normals
// in a part by the appropriate amount based on the shrinkage matrix.  Then when
// the part is drawn with the shrinkage matrix, they automatically get adjusted
// back to being unit length, and we don't have to force OpenGL to normalize
// them.
//
// Note: At first glance, this would appear to mess up parts that are mirror
// images of each other.  Since one part will reference the other with a mirror
// matrix, it makes it possible to shrink the normals twice.  However, since
// all parts get flattenned, and the flatenning process re-normalizes the
// normals to be unit lenght, everything is fine.  If it ever becomes desirable
// to allow parts not to be flattened, things will get more complicated.
void TREModel::unshrinkNormals(const TCFloat *scaleMatrix)
{
	// If the same part is referenced twice in a model, we'll get here twice.
	// We only want to adjust the normals once, or we'll be in trouble, so
	// record the fact that the normals have been adjusted.
	if (!m_flags.unshrunkNormals)
	{
		unshrinkNormals(TCVector::getIdentityMatrix(), scaleMatrix);
		m_flags.unshrunkNormals = true;
	}
}

void TREModel::setSectionPresent(TREMSection section, bool colored)
{
	TCULong bit = 1 << section;

	if (colored)
	{
		m_coloredSectionsPresent |= bit;
	}
	else
	{
		m_sectionsPresent |= bit;
	}
}

bool TREModel::isSectionPresent(TREMSection section, bool colored)
{
	TCULong bit = 1 << section;

	if (colored)
	{
		return (m_coloredSectionsPresent & bit) != 0;
	}
	else
	{
		return (m_sectionsPresent & bit) != 0;
	}
}

bool TREModel::checkShapeGroupPresent(TREShapeGroup *shapeGroup,
									  TREMSection section, bool colored)
{
	if (shapeGroup)
	{
		setSectionPresent(section, colored);
	}
	if (m_subModels)
	{
		int i;
		int count = m_subModels->getCount();

		for (i = 0; i < count; i++)
		{
			if ((*m_subModels)[i]->getEffectiveModel()->
				checkSectionPresent(section, colored))
			{
				setSectionPresent(section, colored);
			}
		}
	}
	return isSectionPresent(section, colored);
}

bool TREModel::checkSectionPresent(TREMSection section, bool colored)
{
	if (colored)
	{
		return checkShapeGroupPresent(m_coloredShapes[section], section,
			true);
	}
	else
	{
		return checkShapeGroupPresent(m_shapes[section], section, false);
	}
}

bool TREModel::checkSectionPresent(TREMSection section)
{
	return checkSectionPresent(section, false);
}

bool TREModel::checkColoredSectionPresent(TREMSection section)
{
	return checkSectionPresent(section, true);
}

// Note: static method
void TREModel::uncompileListID(GLuint &listID)
{
	if (listID)
	{
		glDeleteLists(listID, 1);
		listID = 0;
	}
}

void TREModel::uncompile(bool includeSubModels /*= true*/)
{
	int i;

	for (i = 0; i <= TREMLast; i++)
	{
		uncompileListID(m_listIDs[i]);
		uncompileListID(m_coloredListIDs[i]);
		uncompileListID(m_texListIDs[i]);
		uncompileListID(m_texColoredListIDs[i]);
	}
	if (m_subModels && includeSubModels)
	{
		int count = m_subModels->getCount();

		for (i = 0; i < count; i++)
		{
			(*m_subModels)[i]->getEffectiveModel()->uncompile();
		}
	}
}

void TREModel::cleanupTransfer(
	TREShapeGroup::TRESTransferType type,
	TREMSection section)
{
	if (type == TREShapeGroup::TTTexmapped)
	{
		return;
	}
	if (m_shapes[section] != NULL)
	{
		m_shapes[section]->cleanupTransfer();
	}
	if (m_coloredShapes[section] != NULL)
	{
		m_coloredShapes[section]->cleanupTransfer();
	}
	if (m_subModels)
	{
		int i;
		int count = m_subModels->getCount();

		for (i = 0; i < count; i++)
		{
			(*m_subModels)[i]->getModel()->cleanupTransfer(type, section);
		}
		//for (i = count - 1; i >= 0; i--)
		//{
		//	if ((*m_subModels)[i]->getTransferredFlag())
		//	{
		//		m_subModels->removeObjectAtIndex(i);
		//	}
		//}
	}
}

void TREModel::transferColored(
	TREShapeGroup::TRESTransferType type,
	TREMSection section,
	const TCFloat *matrix)
{
	TREColoredShapeGroup *shapeGroup = m_coloredShapes[section];

	if (shapeGroup)
	{
		shapeGroup->transferColored(type, matrix);
	}
	transferColoredSubModels(type, section, matrix);
}

void TREModel::transferColoredSubModels(
	TREShapeGroup::TRESTransferType type,
	TREMSection section,
	const TCFloat *matrix)
{
	if (m_subModels != NULL)
	{
		int i;
		int count = m_subModels->getCount();
		TexmapInfoList::const_iterator it;
		const TexmapInfo *texmapInfo = NULL;
		bool texmapActive = false;

		if (type == TREShapeGroup::TTTexmapped)
		{
			it = m_texmapInfos.begin();
			if (it != m_texmapInfos.end())
			{
				texmapInfo = &*it;
			}
		}
		for (i = 0; i < count; i++)
		{
			texmapActive = false;
			if (type == TREShapeGroup::TTTexmapped &&
				!m_mainModel->getModelTexmapTransferFlag())
			{
				if (texmapInfo != NULL && i >= texmapInfo->subModelOffset &&
					i < texmapInfo->subModelOffset + texmapInfo->subModelCount)
				{
					texmapActive = true;
					m_mainModel->setTransferTexmapInfo(*texmapInfo,
						section == TREMBFC, matrix);
					m_mainModel->setModelTexmapTransferFlag(true);
				}
			}
			(*m_subModels)[i]->transferColored(type, section, matrix);
			if (texmapActive)
			{
				//(*m_subModels)[i]->setTransferredFlag(true);
				m_mainModel->setModelTexmapTransferFlag(false);
				if (i == texmapInfo->subModelOffset + texmapInfo->subModelCount
					- 1)
				{
					it++;
					if (it != m_texmapInfos.end())
					{
						texmapInfo = &*it;
					}
					else
					{
						texmapInfo = NULL;
					}
				}
			}
		}
	}
}

void TREModel::transfer(
	TREShapeGroup::TRESTransferType type,
	TCULong color,
	TREMSection section,
	const TCFloat *matrix)
{
	TREShapeGroup *shapeGroup = m_shapes[section];

	if (shapeGroup)
	{
		shapeGroup->transfer(type, color, matrix);
	}
	transferSubModels(type, color, section, matrix);
}

void TREModel::transferSubModels(
	TREShapeGroup::TRESTransferType type,
	TCULong color,
	TREMSection section,
	const TCFloat *matrix)
{
	if (m_subModels != NULL)
	{
		int i;
		int count = m_subModels->getCount();
		TexmapInfoList::const_iterator it;
		const TexmapInfo *texmapInfo = NULL;
		bool texmapActive = false;

		if (type == TREShapeGroup::TTTexmapped)
		{
			it = m_texmapInfos.begin();
			while (it != m_texmapInfos.end() && it->subModelCount == 0)
			{
				it++;
			}
			if (it != m_texmapInfos.end())
			{
				texmapInfo = &*it;
			}
		}
		for (i = 0; i < count; i++)
		{
			if (this == m_mainModel)
			{
				m_mainModel->updateModelTransferStep(i);
			}
			texmapActive = false;
			if (type == TREShapeGroup::TTTexmapped &&
				!m_mainModel->getModelTexmapTransferFlag())
			{
				if (texmapInfo != NULL && i >= texmapInfo->subModelOffset &&
					i < texmapInfo->subModelOffset + texmapInfo->subModelCount)
				{
					texmapActive = true;
					m_mainModel->setTransferTexmapInfo(*texmapInfo,
						section == TREMBFC, matrix);
					m_mainModel->setModelTexmapTransferFlag(true);
				}
			}
			(*m_subModels)[i]->transfer(type, color, section, matrix);
			if (type == TREShapeGroup::TTTexmapped && this == m_mainModel)
			{
				(*m_subModels)[i]->transferColored(type, section, matrix);
			}
			if (texmapActive)
			{
				//(*m_subModels)[i]->setTransferredFlag(true);
				m_mainModel->setModelTexmapTransferFlag(false);
				if (i == texmapInfo->subModelOffset + texmapInfo->subModelCount
					- 1)
				{
					it++;
					while (it != m_texmapInfos.end() && it->subModelCount == 0)
					{
						it++;
					}
					if (it != m_texmapInfos.end())
					{
						texmapInfo = &*it;
					}
					else
					{
						texmapInfo = NULL;
					}
				}
			}
		}
	}
}

bool TREModel::shouldLoadConditionalLines(void)
{
	return m_mainModel->shouldLoadConditionalLines();
}

void TREModel::findLights(void)
{
	TCFloat matrix[16];

	TCVector::initIdentityMatrix(matrix);
	findLights(matrix);
}

void TREModel::findLights(float *matrix)
{
	if (m_subModels)
	{
		int i;
		int count = m_subModels->getCount();
		TCFloat newMatrix[16];
		TCVector origin;

		for (i = 0; i < count; i++)
		{
			TRESubModel *subModel = (*m_subModels)[i];

			TCVector::multMatrix(matrix, subModel->getMatrix(), newMatrix);
			if (subModel->getLightFlag())
			{
				m_mainModel->addLight(origin.transformPoint(newMatrix),
					subModel->getColor());
			}
			else
			{
				subModel->getModel()->findLights(newMatrix);
			}
		}
	}
}

void TREModel::flattenNonUniform(void)
{
	if (m_subModels)
	{
		int i;
		int count = m_subModels->getCount();
		TCFloat determinant;

		for (i = count - 1; i >= 0; i--)
		{
			TRESubModel *subModel = (*m_subModels)[i];
			TREModel *newModel = subModel->getEffectiveModel();

			determinant = TCVector::determinant(subModel->getOriginalMatrix());
			if (!fEq(determinant, 1.0) && !fEq(determinant, -1.0))
			{
				if (subModel->isColorSet())
				{
					flatten(newModel, subModel->getMatrix(),
						htonl(subModel->getColor()), true,
						htonl(subModel->getEdgeColor()), true, true);
				}
				else
				{
					flatten(newModel, subModel->getMatrix(), 0, false,
						0, false, true);
				}
				m_subModels->removeObjectAtIndex(i);
				debugPrintf("Flattened non-uniform sub-model: %g.\n",
					determinant);
				if (this == m_mainModel)
				{
					for (int j = (int)m_stepCounts.size() - 1; j >= 0; j--)
					{
						if (m_stepCounts[j] > i)
						{
							m_stepCounts[j]--;
						}
						else
						{
							break;
						}
					}
				}
			}
			else
			{
				if (subModel->isColorSet())
				{
					newModel->flattenNonUniform();
				}
				else
				{
					newModel->flattenNonUniform();
				}
			}
		}
	}
}

void TREModel::removeConditionals(void)
{
	if (this != m_mainModel)
	{
		TREShapeGroup *shapeGroup = m_shapes[TREMConditionalLines];
		TREColoredShapeGroup *coloredShapeGroup =
			m_coloredShapes[TREMConditionalLines];

		if (shapeGroup)
		{
			shapeGroup->release();
			m_shapes[TREMConditionalLines] = NULL;
		}
		if (coloredShapeGroup)
		{
			coloredShapeGroup->release();
			m_coloredShapes[TREMConditionalLines] = NULL;
		}
	}
	if (m_subModels)
	{
		int count = m_subModels->getCount();

		for (int i = 0; i < count; i++)
		{
			TRESubModel *subModel = (*m_subModels)[i];

			subModel->getEffectiveModel()->removeConditionals();
		}
	}
}

void TREModel::flattenConditionals(
	const TCFloat *matrix,
	TCULong edgeColor,
	bool edgeColorSet)
{
	if (this != m_mainModel)
	{
		TREShapeGroup *shapeGroup = m_shapes[TREMConditionalLines];
		TREColoredShapeGroup *coloredShapeGroup =
			m_coloredShapes[TREMConditionalLines];

		if (shapeGroup)
		{
			if (edgeColorSet)
			{
				TREShapeGroup *mainColoredShapeGroup =
					m_mainModel->m_coloredShapes[TREMConditionalLines];

				mainColoredShapeGroup->flatten(shapeGroup, matrix, edgeColor,
					true);
			}
			else
			{
				TREShapeGroup *mainShapeGroup =
					m_mainModel->m_shapes[TREMConditionalLines];

				mainShapeGroup->flatten(shapeGroup, matrix, 0, false);
			}
		}
		if (coloredShapeGroup)
		{
			TREShapeGroup *mainColoredShapeGroup =
				m_mainModel->m_coloredShapes[TREMConditionalLines];

			mainColoredShapeGroup->flatten(coloredShapeGroup, matrix, 0, false);
		}
	}
	if (m_subModels)
	{
		int count = m_subModels->getCount();
		TCFloat newMatrix[16];

		for (int i = 0; i < count; i++)
		{
			TRESubModel *subModel = (*m_subModels)[i];

			if (this == m_mainModel)
			{
				m_mainModel->updateModelTransferStep(i, true);
			}
			TCVector::multMatrix(matrix, subModel->getMatrix(), newMatrix);
			if (subModel->isColorSet())
			{
				subModel->getEffectiveModel()->flattenConditionals(newMatrix,
					htonl(subModel->getEdgeColor()), true);
			}
			else
			{
				subModel->getEffectiveModel()->flattenConditionals(newMatrix,
					edgeColor, edgeColorSet);
			}
		}
	}
}

TCObject *TREModel::getAlertSender(void)
{
	return m_mainModel->getAlertSender();
}

void TREModel::printStlTriangle(
	FILE *file,
	TREVertexArray *vertices,
	TCULongArray *indices,
	int ix,
	int i0,
	int i1,
	int i2,
	const TCFloat *matrix,
	float scale)
{
	int ip[3];
	ip[0]=i0; ip[1]=i1; ip[2]=i2;

	fprintf(file, "  facet normal %f %f %f\n", 0.0, 0.0, 0.0);
	fprintf(file, "    outer loop\n");
	for (int i = 0; i < 3; i++)
	{
		int index = (*indices)[ix + ip[i]];
		const TREVertex &treVertex = (*vertices)[index];
		TCVector vector(treVertex.v[0], treVertex.v[1], treVertex.v[2]);

		vector = vector.transformPoint(matrix);
		fprintf(file, "      vertex %f %f %f\n",  vector[0] * scale, 
			vector[1] * scale, vector[2] * scale);
	}
	fprintf(file, "    endloop\n");
	fprintf(file, "  endfacet\n");
}

void TREModel::saveSTL(FILE *file, float scale)
{
	fprintf(file, "solid MYSOLID created by NativeSTLGen, original data in %s\n",
		m_name);
	saveSTL(file, TCVector::getIdentityMatrix(), scale);
	fprintf(file, "endsolid MYSOLID\n");
}

void TREModel::printStlStrips(
	FILE *file,
	TREShapeGroup *shapeGroup,
	TREShapeType shapeType,
	const TCFloat *matrix,
	float scale)
{
	TCULongArray *indices = shapeGroup->getIndices(shapeType, false);
	TCULongArray *stripCounts = shapeGroup->getStripCounts(shapeType, false);
	TREVertexArray *vertices = shapeGroup->getVertexStore()->getVertices();
	int stripMargin = 2;
	int stripInc = 1;

	if (shapeType == TRESQuadStrip)
	{
		stripMargin = 3;
		stripInc = 2;
	}
	if (indices != NULL && stripCounts != NULL)
	{
		int numStrips = stripCounts->getCount();

		if (numStrips > 0)
		{
			int ofs = 0;

			for (int j = 0; j < numStrips; j++)
			{
				int stripCount = (*stripCounts)[j];

				for (int k = 0; k < stripCount - stripMargin; k += stripInc)
				{
					switch (shapeType)
					{
					case TRESTriangleStrip:
						if (k % 2 == 0)
						{
							printStlTriangle(file, vertices, indices,
								ofs + k, 0, 1, 2, matrix, scale);
						}
						else
						{
							printStlTriangle(file, vertices, indices,
								ofs + k, 0, 2, 1, matrix, scale);
						}
						break;
					case TRESTriangleFan:
						printStlTriangle(file, vertices, indices,
							ofs, 0, k + 1, k + 2, matrix, scale);
						break;
					case TRESQuadStrip:
						printStlTriangle(file, vertices, indices,
							ofs + k, 0, 1, 2, matrix, scale);
						printStlTriangle(file, vertices, indices,
							ofs + k, 1, 2, 3, matrix, scale);
						break;
					default:
						// Get rid of gcc warnings.
						break;
					}
				}
				ofs += stripCount;
			}
		}
	}
}

void TREModel::saveSTLShapes(
	TREShapeGroup *shapes[],
	FILE *file,
	const TCFloat *matrix,
	float scale)
{
	for (int i = 0; i <= TREMLast; i++)
	{
		TREShapeGroup *shape = shapes[i];

		if (shape != NULL)
		{
			TCULongArray *indices = 
				shape->getIndices(TRESTriangle, false);
			TREVertexStore *vertexStore = shape->getVertexStore();

			if (indices != NULL)
			{
				TREVertexArray *vertices = vertexStore->getVertices();
				int count = indices->getCount();

				for ( int p = 0;  p < count; p+=3 )
				{
					printStlTriangle(file, vertices, indices, p, 0, 1, 2,
						matrix, scale);
				}
			}
			indices = shape->getIndices(TRESQuad, false);
			if (indices != NULL)
			{
				TREVertexArray *vertices = vertexStore->getVertices();
				int count = indices->getCount();

				for ( int p = 0;  p < count; p+=4 )
				{
					printStlTriangle(file, vertices, indices, p, 0, 1, 2,
						matrix, scale);
					printStlTriangle(file, vertices, indices, p, 0, 2, 3,
						matrix, scale);
				}
			}
			printStlStrips(file, shape, TRESTriangleStrip, matrix, scale);
			printStlStrips(file, shape, TRESTriangleFan, matrix, scale);
			printStlStrips(file, shape, TRESQuadStrip, matrix, scale);
		}
	}
}

void TREModel::saveSTL(FILE *file, const TCFloat *matrix, float scale)
{
	saveSTLShapes(m_shapes, file, matrix, scale);
	saveSTLShapes((TREShapeGroup **)m_coloredShapes, file, matrix, scale);
	if (m_subModels != NULL)
	{
		for (int i = 0; i < m_subModels->getCount(); i++)
		{
			TRESubModel *subModel = (*m_subModels)[i];
			TCFloat newMatrix[16];

			TCVector::multMatrix(matrix, subModel->getMatrix(), newMatrix);
			subModel->getEffectiveModel()->saveSTL(file, newMatrix, scale);
		}
	}
}

void TREModel::nextStep(void)
{
	// Don't do anything.
}

int TREModel::getShapeCount(
	TREMSection section,
	TREShapeType shapeType,
	bool colored)
{
	TREShapeGroup *shapes;
	
	if (colored)
	{
		shapes = m_coloredShapes[section];
	}
	else
	{
		shapes = m_shapes[section];
	}
	if (shapes != NULL)
	{
		TCULongArray *indices = shapes->getIndices(shapeType);

		if (indices != NULL)
		{
			int size = 1;

			switch (shapeType)
			{
			case TRESTriangle:
				size = 3;
				break;
			case TRESQuad:
				size = 4;
				break;
			default:
				// Get rid of warning.
				break;
			}
			return indices->getCount() / size;
		}
	}
	return 0;
}

void TREModel::startTexture(
	int type,
	const std::string &filename,
	TCImage *image,
	const TCVector *points,
	const TCFloat *extra)
{
	TexmapInfo info((TexmapType)type, filename, points, extra);

	if (this != m_mainModel)
	{
		m_mainModel->startTexture(filename, image);
	}
	//info.standard.standard.triangleOffset = getShapeCount(TREMStandard,
	//	TRESTriangle, false);
	//info.standard.colored.triangleOffset = getShapeCount(TREMStandard,
	//	TRESTriangle, true);
	//info.bfc.standard.triangleOffset = getShapeCount(TREMBFC, TRESTriangle,
	//	false);
	//info.bfc.colored.triangleOffset = getShapeCount(TREMBFC, TRESTriangle,
	//	true);
	//info.standard.standard.quadOffset = getShapeCount(TREMStandard,
	//	TRESQuad, false);
	//info.standard.colored.quadOffset = getShapeCount(TREMStandard,
	//	TRESQuad, true);
	//info.bfc.standard.quadOffset = getShapeCount(TREMBFC, TRESQuad, false);
	//info.bfc.colored.quadOffset = getShapeCount(TREMBFC, TRESQuad, true);
	info.subModelOffset = getSubModelCount();
	m_texmapInfos.push_back(info);
}

bool TREModel::endTexture(void)
{
	return m_mainModel->endTexture();
}

void TREModel::disableTexmaps(void)
{
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glDisable(GL_TEXTURE_2D);
}

void TREModel::activateTexmap(const TexmapInfo &texmapInfo)
{
	GLuint textureID = m_mainModel->getTexmapTextureID(texmapInfo.filename);

	if (textureID == 0)
	{
		return;
	}
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureID);
	if (texmapInfo.type == TTPlanar)
	{
		const TCVector *points = texmapInfo.points;

		TCVector point = points[0];
		TCVector normal = points[1] - point;
		double length = normal.length();
		double scale = 1.0;
		normal /= (TCFloat)length;	// Normalize normal
		double planeCoefficients[4];
		planeCoefficients[0] = (normal[0] * scale) / length;
		planeCoefficients[1] = (normal[1] * scale) / length;
		planeCoefficients[2] = (normal[2] * scale) / length;
		planeCoefficients[3] = -(normal.dot(point) * scale) / length;

		glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
		glTexGendv(GL_S, GL_OBJECT_PLANE, planeCoefficients);
		glEnable(GL_TEXTURE_GEN_S);

		normal = points[2] - point;
		length = normal.length();
		normal /= (TCFloat)length;	// Normalize normal
		planeCoefficients[0] = (normal[0] * scale) / length;
		planeCoefficients[1] = (normal[1] * scale) / length;
		planeCoefficients[2] = (normal[2] * scale) / length;
		planeCoefficients[3] = -(normal.dot(point) * scale) / length;

		glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
		glTexGendv(GL_T, GL_OBJECT_PLANE, planeCoefficients);
		glEnable(GL_TEXTURE_GEN_T);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
			m_mainModel->getTexClampMode());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
			m_mainModel->getTexClampMode());
	}
	else
	{
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
		if (texmapInfo.type == TTCylindrical || texmapInfo.type == TTSpherical)
		{
			GLint sClamp = m_mainModel->getTexClampMode();
			if (texmapInfo.sAngleIs360)
			{
				sClamp = GL_REPEAT;
			}
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, sClamp);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
				m_mainModel->getTexClampMode());
		}
	}
}

int TREModel::getSubModelCount(void) const
{
	if (m_subModels != NULL)
	{
		return m_subModels->getCount();
	}
	else
	{
		return 0;
	}
}

void TREModel::finishPart(void)
{
	if (m_mainModel->getFlattenPartsFlag())
	{
		flatten();
	}
	if (m_mainModel->getSmoothCurvesFlag())
	{
		smooth();
	}
}

void TREModel::shrinkParts(void)
{
	if (!isPart())
	{
		if (m_subModels != NULL)
		{
			for (int i = 0; i < m_subModels->getCount(); i++)
			{
				TRESubModel *subModel = (*m_subModels)[i];
				TREModel *model = subModel->getModel();

				if (model->isPart())
				{
					// Note: if we even get here, then
					// m_mainModel->getSeamWidth() is non-zero.
					if (!model->getNoShrinkFlag())
					{
						subModel->shrink(m_mainModel->getSeamWidth());
					}
				}
				else
				{
					model->shrinkParts();
				}
			}
		}
	}}

void TREModel::finishParts(void)
{
	if (isPart())
	{
		finishPart();
	}
	else
	{
		if (m_subModels != NULL)
		{
			for (int i = 0; i < m_subModels->getCount(); i++)
			{
				TRESubModel *subModel = (*m_subModels)[i];
				TREModel *model = subModel->getModel();

				if (model->isPart())
				{
					model->finishPart();
				}
				else
				{
					model->finishParts();
				}
			}
		}
	}
}

TREModel::TexmapInfo::TexmapInfo(void)
	: type(TTPlanar)
	, cylHeight(0.0)
	, sAngle(0.0)
	, tAngle(0.0)
	, sAngleIs360(false)
	, subModelOffset(0)
	, subModelCount(0)
{
}

TREModel::TexmapInfo::TexmapInfo(
	TexmapType type,
	const std::string &filename,
	const TCVector *otherPoints,
	const TCFloat *extra)
	: type(type)
	, filename(filename)
	, cylHeight(0.0)
	, sAngle(0.0)
	, tAngle(0.0)
	, sAngleIs360(false)
	, subModelOffset(0)
	, subModelCount(0)
{
	copyPoints(otherPoints);
	if (type == TTCylindrical)
	{
		calcCylFields();
		sAngle = deg2rad(extra[0]);
		sAngleIs360 = extra[0] >= 360 || extra[0] <= -360;
	}
	else if (type == TTSpherical)
	{
		calcSphereFields();
		sAngle = deg2rad(extra[0]);
		sAngleIs360 = extra[0] >= 360 || extra[0] <= -360;
		tAngle = deg2rad(extra[1]);
	}
}

bool TREModel::TexmapInfo::texmapEquals(const TexmapInfo &other)
{
	return type == other.type &&
	points[0] == other.points[0] &&
	points[1] == other.points[1] &&
	points[2] == other.points[2] &&
	sAngle == other.sAngle &&
	tAngle == other.tAngle &&
	cylHeight == other.cylHeight &&
	filename == other.filename;
}

void TREModel::TexmapInfo::copyPoints(const TCVector *otherPoints)
{
	points[0] = otherPoints[0];
	points[1] = otherPoints[1];
	points[2] = otherPoints[2];
}

void TREModel::TexmapInfo::transform(const TCFloat* matrix)
{
	if (matrix != NULL)
	{
		for (size_t i = 0; i < 3; i++)
		{
			points[i] = points[i].transformPoint(matrix);
		}
		if (type == TTCylindrical)
		{
			calcCylFields();
		}
		else if (type == TTSpherical)
		{
			calcSphereFields();
		}
	}
}

void TREModel::TexmapInfo::calcCylFields(void)
{
	a = points[0];
	TCVector b = points[1];
	normal = a - b;
	cylHeight = normal.length();
	normal /= cylHeight;
	dir = cylDirectionFrom(points[2]);
}

void TREModel::TexmapInfo::calcSphereFields(void)
{
	a = points[0];
	normal = -((points[0] - points[1]) * (points[2] - points[1])).normalize();
	normal2 = (normal * (points[1] - a)).normalize();
	dir = (points[1] - points[0]).normalize();
}

TCVector TREModel::TexmapInfo::cylDirectionFrom(const TCVector& point)
{
	return directionFrom(point, normal);
}

TCVector TREModel::TexmapInfo::directionFrom(
	const TCVector& point,
	const TCVector& norm)
{
	TCVector ap = point - a;
	TCVector proj = a + ap.dot(norm) / norm.dot(norm) * norm;
	TCVector dir = point - proj;
	return dir.normalize();
}

TCFloat TREModel::TexmapInfo::distanceToPlane(
	const TCVector& point,
	const TCVector& planePoint,
	const TCVector& planeNormal)
{
	return planeNormal.dot(planePoint - point);
}

void TREModel::TexmapInfo::calcTextureCoords(
	const TCVector* ppoints,
	TCVector* textureCoords)
{
	if (type == TTCylindrical)
	{
		calcCylTextureCoords(ppoints, textureCoords);
	}
	else if (type == TTSpherical)
	{
		calcSphereTextureCoords(ppoints, textureCoords);
	}
}

TCFloat TREModel::TexmapInfo::calcSAngle(
	const TCVector& point,
	bool isFirst,
	TCVector& baseDir,
	TCFloat& baseAngle)
{
	TCFloat curAngle;
	if (isFirst)
	{
		baseDir = cylDirectionFrom(point);
		curAngle = atan2((dir * baseDir).dot(normal), baseDir.dot(dir));
	}
	else
	{
		TCVector curDir = cylDirectionFrom(point);
		curAngle = atan2((baseDir * curDir).dot(normal), curDir.dot(baseDir)) +
			baseAngle;
	}
	if (isFirst)
	{
		baseAngle = curAngle;
	}
	return curAngle;
}

void TREModel::TexmapInfo::calcCylTextureCoords(
	const TCVector* ppoints,
	TCVector* textureCoords)
{
	TCVector baseDir;
	TCFloat baseAngle = 0.0;
	for (size_t i = 0; i < 3; ++i)
	{
		const TCVector& point(ppoints[i]);
		TCVector& tc(textureCoords[i]);
		tc[0] = 0.5 + calcSAngle(point, i == 0, baseDir, baseAngle) / sAngle;
		tc[1] = distanceToPlane(point, a, normal) / cylHeight;
	}
}


void TREModel::TexmapInfo::calcSphereTextureCoords(
	const TCVector* ppoints,
	TCVector* textureCoords)
{
	TCVector baseDir;
	TCFloat baseAngle = 0.0;
	std::vector<size_t> poleIndices;
	TCFloat uSum = 0.0;
	for (size_t i = 0; i < 3; ++i)
	{
		const TCVector& point(ppoints[i]);
		TCVector& tc(textureCoords[i]);
		TCVector pointDir = (point - a).normalize();
		bool northPole = pointDir.approxEquals(normal, 1e-05);
		bool southPole = false;
		if (!northPole)
		{
			southPole = pointDir.approxEquals(-normal, 1e-05);
		}
		TCFloat pointSAngle = 0.0;
		TCFloat pointTAngle;
		if (northPole || southPole)
		{
			poleIndices.push_back(i);
			if (northPole)
			{
				pointTAngle = -M_PI_2;
			}
			else
			{
				pointTAngle = M_PI_2;
			}
		}
		else
		{
			pointSAngle = calcSAngle(point, i == 0, baseDir, baseAngle);
			uSum += pointSAngle;
			TCFloat theta = -pointSAngle;
			// Rotate pointDir around normal until it's at 0 longitude.
			TCVector refPointDir = (pointDir * cos(theta) +
				(normal * pointDir) * sin(theta) + normal *
				normal.dot(pointDir) * (1.0 - cos(theta))).normalize();
			pointTAngle = atan2((dir * refPointDir).dot(normal2),
				refPointDir.dot(dir));
		}
		tc[0] = 0.5 + pointSAngle / sAngle;
		tc[1] = 0.5 + pointTAngle / tAngle;
	}
	if (!poleIndices.empty() && poleIndices.size() < 3)
	{
		TCFloat uAverage = 0.5 + uSum / (3 - poleIndices.size()) / sAngle;
		for (size_t i = 0; i < poleIndices.size(); ++i)
		{
			TCVector& tc(textureCoords[poleIndices[i]]);
			tc[0] = uAverage;
		}
	}
}
