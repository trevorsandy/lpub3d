#include "TREMainModel.h"
#include "TREVertexStore.h"
#include "TRETransShapeGroup.h"
#include "TRETexmappedShapeGroup.h"
#include "TREGL.h"
#include "TREGLExtensions.h"
#include "TRESubModel.h"
#include <math.h>
#include <string.h>
#include <gl2ps/gl2ps.h>

#include <TCFoundation/TCDictionary.h>
#include <TCFoundation/TCProgressAlert.h>
#include <TCFoundation/TCLocalStrings.h>

#ifdef USE_CPP11
#include <thread>
#endif

#ifdef WIN32
#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
#define new DEBUG_CLIENTBLOCK
#endif // _DEBUG
#endif // WIN32

#ifdef USE_CPP11
typedef std::unique_lock<std::mutex> ScopedLock;
#else
#ifndef _NO_TRE_THREADS
#include <boost/bind.hpp>
//#define ANTI_DEADLOCK_HACK
#ifdef ANTI_DEADLOCK_HACK
#include <boost/thread/xtime.hpp>
#endif // ANTI_DEADLOCK_HACK
typedef boost::mutex::scoped_lock ScopedLock;

#ifdef __APPLE__
#include <CoreServices/CoreServices.h>
#endif // __APPLE__
#endif // !_NO_TRE_THREADS
#endif // !USE_CPP11

//const GLfloat POLYGON_OFFSET_FACTOR = 0.85f;
//const GLfloat POLYGON_OFFSET_UNITS = 0.0f;
const GLfloat POLYGON_OFFSET_FACTOR = 1.0f;
// NOTE: setting the following to a non-zero value results in "implementation-
// specific" behavior.  In other words, "undefined".  Yech.
const GLfloat POLYGON_OFFSET_UNITS = 0.0f;

TCImageArray *TREMainModel::sm_studTextures = NULL;
GLuint TREMainModel::sm_studTextureID = 0;
TREMainModel::TREMainModelCleanup TREMainModel::sm_mainModelCleanup;

TREMainModel::TREMainModelCleanup::~TREMainModelCleanup(void)
{
	TCObject::release(TREMainModel::sm_studTextures);
	TREMainModel::sm_studTextures = NULL;
	// We can't deletet the texture from GL, since there's not likely to be an
	// OpenGL context active at this point in the app shutdown process.
	//if (TREMainModel::sm_studTextureID)
	//{
	//	glDeleteTextures(1, &TREMainModel::sm_studTextureID);
	//}
}

//TREMainModel::TREMainModel(const TREMainModel &other)
//	:TREModel(other),
//	m_loadedModels((TCDictionary *)TCObject::copy(other.m_loadedModels)),
//	m_loadedBFCModels((TCDictionary *)TCObject::copy(other.m_loadedBFCModels)),
//	m_vertexStore((TREVertexStore *)TCObject::copy(other.m_vertexStore)),
//	m_studVertexStore((TREVertexStore *)TCObject::copy(
//		other.m_studVertexStore)),
//	m_coloredVertexStore((TREVertexStore *)TCObject::copy(
//		other.m_coloredVertexStore)),
//	m_coloredStudVertexStore((TREVertexStore *)TCObject::copy(
//		other.m_coloredStudVertexStore)),
//	m_transVertexStore((TREVertexStore *)TCObject::copy(
//		other.m_transVertexStore)),
//	m_color(other.m_color),
//	m_edgeColor(other.m_edgeColor),
//	m_maxRadiusSquared(other.m_maxRadiusSquared),
//	m_edgeLineWidth(other.m_edgeLineWidth),
//	m_studAnisoLevel(other.m_studAnisoLevel),
//	m_abort(false),
//	m_studTextureFilter(other.m_studTextureFilter),
//	m_mainFlags(other.m_mainFlags)
//{
//#ifdef _LEAK_DEBUG
//	strcpy(className, "TREMainModel");
//#endif // _LEAK_DEBUG
//	m_mainModel = this;
//}

TREMainModel::TREMainModel(void)
	: m_alertSender(NULL)
	, m_loadedModels(NULL)
	, m_loadedBFCModels(NULL)
	, m_vertexStore(new TREVertexStore)
	, m_studVertexStore(new TREVertexStore)
	, m_coloredVertexStore(new TREVertexStore)
	, m_coloredStudVertexStore(new TREVertexStore)
	, m_transVertexStore(new TREVertexStore)
	, m_texmapVertexStore(new TREVertexStore)
#ifndef __INTEL_COMPILER
	, m_color(htonl(0x999999FF))
	, m_edgeColor(htonl(0x666658FF))
#endif // !__INTEL_COMPILER
	, m_maxRadiusSquared(0.0f)
	, m_textureOffsetFactor(5.0f)
	, m_edgeLineWidth(1.0f)
	, m_studAnisoLevel(1.0f)
	, m_abort(false)
	, m_studTextureFilter(GL_LINEAR_MIPMAP_LINEAR)
	, m_step(-1)
	, m_curGeomModel(NULL)
	, m_texClampMode(GL_CLAMP)
	, m_seamWidth(0.5f)
#if defined(USE_CPP11) || !defined(_NO_TRE_THREADS)
#ifdef USE_CPP11
    , m_threads(NULL)
#else
	, m_threadGroup(NULL)
#endif
	, m_workerMutex(NULL)
	, m_workerCondition(NULL)
	, m_sortCondition(NULL)
	, m_conditionalsCondition(NULL)
	, m_exiting(false)
#endif // !_NO_TRE_THREADS
{
#ifdef __INTEL_COMPILER
	m_color = htonl(0x999999FF);
	m_edgeColor = htonl(0x666658FF);
#endif // __INTEL_COMPILER
#ifdef _LEAK_DEBUG
	strcpy(className, "TREMainModel");
#endif // _LEAK_DEBUG
	m_mainModel = this;
	m_mainFlags.compiled = false;
	m_mainFlags.compiling = false;
	m_mainFlags.removingHiddenLines = false;
	m_mainFlags.cutawayDraw = false;
	m_mainFlags.activeLineJoins = false;
	m_mainFlags.frameSorted = false;
	m_mainFlags.frameSortStarted = false;
	m_mainFlags.frameStarted = false;

	m_mainFlags.compileParts = false;
	m_mainFlags.compileAll = false;
	m_mainFlags.edgeLines = false;
	m_mainFlags.edgesOnly = false;
	m_mainFlags.twoSidedLighting = false;
	m_mainFlags.lighting = false;
	m_mainFlags.useStrips = true;
	m_mainFlags.disableStrips = false;
	m_mainFlags.useTriStrips = true;
	m_mainFlags.useTriFans = true;
	m_mainFlags.useQuadStrips = true;
	m_mainFlags.useFlatStrips = false;
	m_mainFlags.bfc = false;
	m_mainFlags.aaLines = false;
	m_mainFlags.sortTransparent = false;
	m_mainFlags.stipple = false;
	m_mainFlags.wireframe = false;
	m_mainFlags.conditionalLines = false;
	m_mainFlags.smoothCurves = true;
	m_mainFlags.showAllConditional = false;
	m_mainFlags.conditionalControlPoints = false;
	m_mainFlags.studLogo = true;
	m_mainFlags.redBackFaces = false;
	m_mainFlags.greenFrontFaces = false;
	m_mainFlags.blueNeutralFaces = false;
	m_mainFlags.lineJoins = false;	// Doesn't work right
	m_mainFlags.drawNormals = false;
	m_mainFlags.stencilConditionals = false;
	m_mainFlags.vertexArrayEdgeFlags = false;
	m_mainFlags.multiThreaded = true;
	m_mainFlags.gl2ps = false;
	m_mainFlags.sendProgress = true;
	m_mainFlags.modelTexmapTransfer = false;
	m_mainFlags.flattenParts = true;
	m_mainFlags.texturesAfterTransparent = false;

	m_conditionalsDone = 0;
	m_conditionalsStep = 0;
	memset(m_activeConditionals, 0, sizeof(m_activeConditionals));
	memset(m_activeColorConditionals, 0, sizeof(m_activeColorConditionals));
	memset(m_texmappedShapes, 0, sizeof(m_texmappedShapes));
}

TREMainModel::~TREMainModel(void)
{
}

void TREMainModel::dealloc(void)
{
#if defined(USE_CPP11) || !defined(_NO_TRE_THREADS)
#ifdef USE_CPP11
	if (m_threads != NULL)
#else
	if (m_threadGroup)
#endif
	{
		if (m_workerMutex != NULL)
		{
			ScopedLock lock(*m_workerMutex);

			m_exiting = true;
			m_workerCondition->notify_all();
			lock.unlock();
#ifdef USE_CPP11
			for (auto&& thread: *m_threads)
			{
				thread.join();
			}
			delete m_threads;
			m_threads = NULL;
#else
			m_threadGroup->join_all();
			delete m_threadGroup;
			m_threadGroup = NULL;
#endif
		}
		delete m_workerMutex;
		m_workerMutex = NULL;
		delete m_workerCondition;
		m_workerCondition = NULL;
		delete m_sortCondition;
		m_sortCondition = NULL;
		delete m_conditionalsCondition;
		m_conditionalsCondition = NULL;
	}
#endif // !_NO_TRE_THREADS
	uncompile();
	for (size_t i = 0; i < 2; i++)
	{
		TCObject::release(m_texmappedShapes[i]);
	}
	deleteGLTexmaps();
	TCObject::release(m_loadedModels);
	TCObject::release(m_loadedBFCModels);
	TCObject::release(m_vertexStore);
	TCObject::release(m_studVertexStore);
	TCObject::release(m_coloredVertexStore);
	TCObject::release(m_coloredStudVertexStore);
	TCObject::release(m_transVertexStore);
	TCObject::release(m_texmapVertexStore);
	TREModel::dealloc();
}

TCObject *TREMainModel::copy(void) const
{
	return new TREMainModel(*this);
}

TCDictionary *TREMainModel::getLoadedModels(bool bfc)
{
	if (bfc)
	{
		if (!m_loadedBFCModels)
		{
			m_loadedBFCModels = new TCDictionary(0);
		}
		return m_loadedBFCModels;
	}
	else
	{
		if (!m_loadedModels)
		{
			m_loadedModels = new TCDictionary(0);
		}
		return m_loadedModels;
	}
}

void TREMainModel::activateBFC(void)
{
	if (getRedBackFacesFlag() || getGreenFrontFacesFlag())
	{
		bool needColorMaterial = true;

		if (getRedBackFacesFlag() && getGreenFrontFacesFlag())
		{
			needColorMaterial = false;
			glDisable(GL_COLOR_MATERIAL);
		}
		else if (getBlueNeutralFacesFlag())
		{
			glEnable(GL_COLOR_MATERIAL);
		}
		if (getRedBackFacesFlag())
		{
			GLfloat mRed[] = {1.0f, 0.0f, 0.0f, 1.0f};

			if (needColorMaterial)
			{
				glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
			}
			glMaterialfv(GL_BACK, GL_AMBIENT, mRed);
			glMaterialfv(GL_BACK, GL_DIFFUSE, mRed);
		}
		if (getGreenFrontFacesFlag())
		{
			GLfloat mGreen[] = {0.0f, 1.0f, 0.0f, 1.0f};

			if (needColorMaterial)
			{
				glColorMaterial(GL_BACK, GL_AMBIENT_AND_DIFFUSE);
			}
			glMaterialfv(GL_FRONT, GL_AMBIENT, mGreen);
			glMaterialfv(GL_FRONT, GL_DIFFUSE, mGreen);
		}
	}
	else
	{
		if (getBlueNeutralFacesFlag())
		{
			glEnable(GL_COLOR_MATERIAL);
			glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
		}
		// Note that GL_BACK is the default face to cull, and GL_CCW is the
		// default polygon winding.
		glEnable(GL_CULL_FACE);
		if (getTwoSidedLightingFlag() && getLightingFlag())
		{
			glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);
		}
	}
}

void TREMainModel::deactivateBFC(bool transparent /*= false*/)
{
	if (getBlueNeutralFacesFlag() && !transparent)
	{
		GLfloat mBlue[] = {0.0f, 0.0f, 1.0f, 1.0f};

		glDisable(GL_COLOR_MATERIAL);
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mBlue);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mBlue);
	}
	else
	{
		glEnable(GL_COLOR_MATERIAL);
		if (getRedBackFacesFlag() && getGreenFrontFacesFlag())
		{
			// Don't do anything here.
		}
		else if ((getRedBackFacesFlag() || getGreenFrontFacesFlag()))
		{
			glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
		}
		else
		{
			if (getTwoSidedLightingFlag() && getLightingFlag())
			{
				glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
			}
			glDisable(GL_CULL_FACE);
		}
	}
}

/*
void TREMainModel::compileTransparent(void)
{
	if (!m_coloredListIDs[TREMTransparent] &&
		m_coloredShapes[TREMTransparent])
	{
		int listID = glGenLists(1);

		glNewList(listID, GL_COMPILE);
		((TRETransShapeGroup *)m_coloredShapes[TREMTransparent])->
			draw(false);
		glEndList();
		m_coloredListIDs[TREMTransparent] = listID;
	}
}
*/

bool TREMainModel::getStencilConditionalsFlag(void)
{
	// Save alpha requires the stencil buffer for another purpose; don't allow
	// stencil conditionals when it's set.
	return m_mainFlags.stencilConditionals != false &&
		!getSaveAlphaFlag();
}

bool TREMainModel::shouldCompileSection(TREMSection section)
{
	if (getStencilConditionalsFlag())
	{
		return section != TREMTransparent;
	}
	else
	{
		return section != TREMConditionalLines && section != TREMTransparent;
	}
}

void TREMainModel::compile(void)
{
	if (!m_mainFlags.compiled)
	{
		int i;
		float numSections = (float)(TREMLast - TREMFirst + 1);

//		TCProgressAlert::send("TREMainModel",
//			TCLocalStrings::get("TREMainModelCompiling"), 0.0f, &m_abort);
		if (!m_abort)
		{
			m_mainFlags.compiling = true;
			for (i = TREMFirst; i <= TREMLast && !m_abort; i++)
			{
				TREMSection section = (TREMSection)i;

				if (shouldCompileSection(section))
				{
					float progress = (float)i / (numSections * 2.0f);

					if (m_mainFlags.sendProgress)
					{
						TCProgressAlert::send("TREMainModel",
							TCLocalStrings::get(_UC("TREMainModelCompiling")),
							progress, &m_abort, this);
					}
					if (!m_abort && isSectionPresent(section, false))
					{
						if (isStudSection(section))
						{
							m_studVertexStore->activate(true);
						}
						else
						{
							m_vertexStore->activate(true);
						}
						if (section == TREMEdgeLines)
						{
							if (getLineJoinsFlag() && m_edgeLineWidth > 1.0f)
							{
//								pointSize(m_edgeLineWidth);
								m_mainFlags.activeLineJoins = true;
//								glEnable(GL_POINT_SMOOTH);
//								glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
							}
						}
						TREModel::compile(section, false);
						m_mainFlags.activeLineJoins = false;
					}
				}
			}
/*
			TREModel::compile(TREMStandard, false);
			TCProgressAlert::send("TREMainModel",
				TCLocalStrings::get(L"TREMainModelCompiling"), 0.2f, &m_abort);
*/
		}
/*
		if (!m_abort)
		{
			if (getBFCFlag())
			{
				TREModel::compile(TREMBFC, false);
			}
			TCProgressAlert::send("TREMainModel",
				TCLocalStrings::get(L"TREMainModelCompiling"), 0.3f, &m_abort);
		}
		if (!m_abort)
		{
			TREModel::compile(TREMLines, false);
			TCProgressAlert::send("TREMainModel",
				TCLocalStrings::get(L"TREMainModelCompiling"), 0.35f, &m_abort);
		}
		if (!m_abort)
		{
			TREModel::compile(TREMEdgeLines, false);
			TCProgressAlert::send("TREMainModel",
				TCLocalStrings::get(L"TREMainModelCompiling"), 0.45f, &m_abort);
		}
*/
		if (!m_abort)
		{
			for (i = TREMFirst; i <= TREMLast && !m_abort; i++)
			{
				TREMSection section = (TREMSection)i;

				if (shouldCompileSection(section))
				{
					float progress = (float)i / (numSections * 2.0f) + 0.5f;

					if (m_mainFlags.sendProgress)
					{
						TCProgressAlert::send("TREMainModel",
							TCLocalStrings::get(_UC("TREMainModelCompiling")),
							progress, &m_abort, this);
					}
					if (!m_abort && isSectionPresent(section, true))
					{
						if (isStudSection(section))
						{
							m_coloredStudVertexStore->activate(true);
						}
						else
						{
							m_coloredVertexStore->activate(true);
						}
						TREModel::compile(section, true);
					}
				}
			}
/*
			TREModel::compile(TREMStandard, true);
			TCProgressAlert::send("TREMainModel",
				TCLocalStrings::get(L"TREMainModelCompiling"), 0.55f, &m_abort);
*/
		}
/*
		if (!m_abort)
		{
			if (getBFCFlag())
			{
				TREModel::compile(TREMBFC, true);
			}
			TCProgressAlert::send("TREMainModel",
				TCLocalStrings::get(L"TREMainModelCompiling"), 0.65f, &m_abort);
		}
		if (!m_abort)
		{
			TREModel::compile(TREMBFC, true);
			TCProgressAlert::send("TREMainModel",
				TCLocalStrings::get(L"TREMainModelCompiling"), 0.7f, &m_abort);
		}
		if (!m_abort)
		{
			TREModel::compile(TREMEdgeLines, true);
			TCProgressAlert::send("TREMainModel",
				TCLocalStrings::get(L"TREMainModelCompiling"), 0.8f, &m_abort);
		}
		if (!m_abort)
		{
			if (m_transVertexStore)
			{
				m_transVertexStore->activate(true);
			}
			if (!getSortTransparentFlag())
			{
				TREModel::compile(TREMTransparent, true);
			}
			TCProgressAlert::send("TREMainModel",
				TCLocalStrings::get(L"TREMainModelCompiling"), 0.9f, &m_abort);
		}
*/
		if (!m_abort)
		{
			m_mainFlags.compiled = true;
			m_mainFlags.compiling = false;
			if (m_mainFlags.sendProgress)
			{
				TCProgressAlert::send("TREMainModel",
					TCLocalStrings::get(_UC("TREMainModelCompiling")), 1.0f,
					&m_abort, this);
			}
//			TCProgressAlert::send("LDrawModelViewer", "Done.", 2.0f);
		}
		TREVertexStore::deactivateActiveVertexStore();
	}
}

void TREMainModel::recompile(void)
{
	if (m_mainFlags.compiled)
	{
		uncompile();
		m_mainFlags.compiled = false;
	}
	compile();
}

/*
void checkNormals(TREVertexStore *vertexStore)
{
	if (vertexStore)
	{
		TREVertexArray *normals = vertexStore->getNormals();

		if (normals)
		{
			int i;
			int count = normals->getCount();

			for (i = 0; i < count; i++)
			{
				TCVector normal = (*normals)[i].v;

				if (!fEq(normal.lengthSquared(), 1.0))
				{
					MessageBox(NULL, "Bad Normal", "NativePOVGen", MB_OK);
				}
			}
		}
	}
}
*/

void TREMainModel::passOnePrep(void)
{
	glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT | GL_STENCIL_BUFFER_BIT);
	// First pass: draw just the color of all the transparent polygons
	// that are just in front of the background.  Don't draw anywhere
	// that has already been drawn to.  This makes sure that subsequent
	// color blend operations don't include the background color.  Also
	// set the stencil to 1 for every pixel we update the alpha on.
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_EQUAL, 0, 0xFFFFFFFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
	glStencilMask(0xFFFFFFFF);
	disable(GL_BLEND);
}

void TREMainModel::passTwoPrep(void)
{
	// Second pass: update just the alpha channel on all transparent
	// polygons that have the background visible through them (which
	// is all the ones we set the stencil to 1 in on pass 1 above).
	// Note that depth testing is disabled, and the blend function
	// is set to allow the alpha channel to build up as transparent
	// polygons overlap.
	glStencilFunc(GL_EQUAL, 1, 0xFFFFFFFF);
	glDisable(GL_DEPTH_TEST);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
}

void TREMainModel::passThreePrep(void)
{
	// Third pass (actual drawing is outside this if statement):
	// Now draw the color component of all the transparent polygons
	// except for the ones drawn in step 1.  (Depth test is set to
	// GL_LESS, so that those don't ge redrawn.)  Other than that
	// depth test setting, everything is configure the same as for
	// standard one-pass transparency.
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
	glDisable(GL_STENCIL_TEST);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
}

int TREMainModel::getNumBackgroundTasks(void)
{
	int numTasks = 0;

	if (backgroundSortNeeded())
	{
		numTasks++;
	}
	if (backgroundConditionalsNeeded())
	{
		numTasks += 32;
	}
	return numTasks;
}

int TREMainModel::getNumWorkerThreads(void)
{
#if defined(USE_CPP11) || !defined(_NO_TRE_THREADS)
	if (getMultiThreadedFlag())
	{
#ifdef USE_CPP11
		int numProcessors = std::thread::hardware_concurrency();
#else // USE_CPP11
		int numProcessors = 1;

#if defined(WIN32)
		DWORD_PTR processAffinityMask;
		DWORD_PTR systemAffinityMask;
		
		if (GetProcessAffinityMask(GetCurrentProcess(), &processAffinityMask,
			&systemAffinityMask))
		{
			numProcessors = 0;

			while (processAffinityMask)
			{
				if (processAffinityMask & 0x01)
				{
					numProcessors++;
				}
				processAffinityMask >>= 1;
			}
		}
#elif defined(__APPLE__)
		numProcessors = (int)MPProcessorsScheduled();
#elif defined(_QT)
		int result = -1;
#ifdef _SC_NPROCESSORS_CONF
		result = (int)sysconf(_SC_NPROCESSORS_CONF);
		if (result > 0)
		{
			numProcessors = result;
		}
#endif // _SC_NPROCESSORS_CONF
#ifdef _SC_NPROCESSORS_ONLN
		result = (int)sysconf(_SC_NPROCESSORS_ONLN);
		if (result > 0)
		{
			numProcessors = result;
		}
#endif // _SC_NPROCESSORS_ONLN
#endif // _QT
#endif // !USE_CPP11
		if (numProcessors > 1)
		{
			return std::min(numProcessors - 1, getNumBackgroundTasks());
		}
	}
#endif // USE_CPP11 || !_NO_TRE_THREADS
	return 0;
}

bool TREMainModel::backgroundSortNeeded(void)
{
	TRETransShapeGroup* transShapeGroup =
		(TRETransShapeGroup*)m_coloredShapes[TREMTransparent];

	return (transShapeGroup && getSortTransparentFlag() &&
		!getCutawayDrawFlag());
}

bool TREMainModel::backgroundConditionalsNeeded(void)
{
	return getFlattenConditionalsFlag() && getConditionalLinesFlag();
}

bool TREMainModel::doingBackgroundConditionals(void)
{
	return backgroundConditionalsNeeded() && getMultiThreadedFlag() &&
		getNumBackgroundTasks() > 0 && getNumWorkerThreads() > 0;
}

TCULongArray *TREMainModel::backgroundConditionals(
	TREShapeGroup *shapes,
	int step)
{
	if (shapes)
	{
		TCULongArray *indices = shapes->getIndices(TRESConditionalLine);

		if (indices)
		{
			int subCount = shapes->getIndexCount(TRESConditionalLine) / 2;
			int stepSize = subCount / 32 * 2;
			int stepCount = stepSize;

			if (step == 31)
			{
				stepCount += (subCount % 32) * 2;
			}
			return shapes->getActiveConditionalIndices(indices,
				TCVector::getIdentityMatrix(), stepSize * step, stepCount);
		}
	}
	return NULL;
}

void TREMainModel::backgroundConditionals(int step)
{
	m_activeConditionals[step] =
		backgroundConditionals(m_shapes[TREMConditionalLines], step);
	m_activeColorConditionals[step] = 
		backgroundConditionals(m_coloredShapes[TREMConditionalLines], step);
}

#if defined(USE_CPP11) || !defined(_NO_TRE_THREADS)

template <class _ScopedLock>
void TREMainModel::nextConditionalsStep(_ScopedLock &lock)
{
	// lock is always locked here.
	int step = m_conditionalsStep;

	m_conditionalsStep++;
	lock.unlock();
	backgroundConditionals(step);
	lock.lock();
	m_conditionalsDone |= 1 << step;
}

template <class _ScopedLock>
bool TREMainModel::workerThreadDoWork(_ScopedLock &lock)
{
	// lock is always locked here.
	if (!m_mainFlags.frameStarted)
	{
		return false;
	}
	if (backgroundConditionalsNeeded() && (m_conditionalsStep < 32))
	{
		nextConditionalsStep(lock);
		if (m_conditionalsDone == 0xFFFFFFFF)
		{
			m_conditionalsCondition->notify_all();
		}
		return true;
	}
	if (!m_mainFlags.frameSortStarted && backgroundSortNeeded())
	{
		TRETransShapeGroup* transShapeGroup =
			(TRETransShapeGroup*)m_coloredShapes[TREMTransparent];

		m_mainFlags.frameSortStarted = true;
		lock.unlock();
		transShapeGroup->backgroundSort();
		lock.lock();
		m_mainFlags.frameSorted = true;
		m_sortCondition->notify_all();
		return true;
	}
	return false;
}

void TREMainModel::workerThreadProc(void)
{
	ScopedLock lock(*m_workerMutex);

	if (!m_mainFlags.frameStarted)
	{
		m_workerCondition->wait(lock);
	}
	while (1)
	{
		if (m_exiting)
		{
			break;
		}
		if (!workerThreadDoWork(lock))
		{
			if (!m_exiting)
			{
#ifdef ANTI_DEADLOCK_HACK
#ifdef USE_CPP11
                m_workerCondition->wait_for(lock, std::chrono::milliseconds(100));
#else
				boost::xtime xt;

				boost::xtime_get(&xt, boost::TIME_UTC);
				// 100,000,000 nsec == 100 msec
				xt.nsec += 100 * 1000000;
				// HACK: If any deadlocks are encountered during testing,
				// ANTI_DEADLOCK_HACK can be defined, and hopefully they'll go
				// away.
				m_workerCondition->timed_wait(lock, xt);
#endif
#else // ANTI_DEADLOCK_HACK
				m_workerCondition->wait(lock);
#endif // ANTI_DEADLOCK_HACK
			}
		}
	}
}
#endif // USE_CPP11 || !_NO_TRE_THREADS

void TREMainModel::launchWorkerThreads()
{
#if defined(USE_CPP11) || !defined(_NO_TRE_THREADS)
#ifdef USE_CPP11
	if (m_threads == NULL)
#else
	if (m_threadGroup == NULL)
#endif
	{
		int workerThreadCount = getNumWorkerThreads();
#ifdef USE_CPP11
        m_threads = new std::vector<std::thread>;
#endif

		if (workerThreadCount > 0)
		{
#ifdef USE_CPP11
			m_mainFlags.frameStarted = false;
			m_workerMutex = new std::mutex;
			m_workerCondition = new std::condition_variable;
			m_sortCondition = new std::condition_variable;
			m_conditionalsCondition = new std::condition_variable;
#else
			m_threadGroup = new boost::thread_group;
			m_workerMutex = new boost::mutex;
			m_workerCondition = new boost::condition;
			m_sortCondition = new boost::condition;
			m_conditionalsCondition = new boost::condition;
#endif
			for (int i = 0; i < workerThreadCount; i++)
			{
#ifdef USE_CPP11
                m_threads->emplace_back(&TREMainModel::workerThreadProc, this);
#else
				m_threadGroup->create_thread(
					boost::bind(&TREMainModel::workerThreadProc, this));
#endif
			}
		}
	}
#endif // USE_CPP11 || !_NO_TRE_THREADS
}

void TREMainModel::triggerWorkerThreads(void)
{
#if defined(USE_CPP11) || !defined(_NO_TRE_THREADS)
	if (m_workerMutex)
	{
		ScopedLock lock(*m_workerMutex);
		m_mainFlags.frameSorted = false;
		m_mainFlags.frameSortStarted = false;
		m_mainFlags.frameStarted = true;
		m_conditionalsDone = 0;
		m_conditionalsStep = 0;
		m_workerCondition->notify_all();
		memset(m_activeConditionals, 0, sizeof(m_activeConditionals));
		memset(m_activeColorConditionals, 0, sizeof(m_activeColorConditionals));
	}
#endif // USE_CPP11 || !_NO_TRE_THREADS
}

bool TREMainModel::hasWorkerThreads(void)
{
#if defined(USE_CPP11) || !defined(_NO_TRE_THREADS)
	if (m_workerMutex)
	{
		ScopedLock lock(*m_workerMutex);
#ifdef USE_CPP11
        return m_threads != NULL;
#else
		return m_threadGroup != NULL;
#endif
	}
#endif // USE_CPP11 || !_NO_TRE_THREADS
	return false;
}

void TREMainModel::waitForSort(void)
{
#if defined(USE_CPP11) || !defined(_NO_TRE_THREADS)
	if (m_workerMutex)
	{
		ScopedLock lock(*m_workerMutex);
		if (!m_mainFlags.frameSorted)
		{
			m_sortCondition->wait(lock);
		}
	}
#endif // USE_CPP11 || !_NO_TRE_THREADS
}

#if !defined(USE_CPP11) && defined(_NO_TRE_THREADS)
void TREMainModel::waitForConditionals(int /*step*/)
#else // !USE_CPP11 && _NO_TRE_THREADS
void TREMainModel::waitForConditionals(int step)
#endif // USE_CPP11 || !_NO_TRE_THREADS
{
#if defined(USE_CPP11) || !defined(_NO_TRE_THREADS)
	if (m_workerMutex)
	{
		ScopedLock lock(*m_workerMutex);

		while ((m_conditionalsDone & (1 << step)) == 0)
		{
			if (m_conditionalsStep < 32)
			{
				nextConditionalsStep(lock);
			}
			else
			{
				m_conditionalsCondition->wait(lock);
			}
		}
	}
#endif // USE_CPP11 || !_NO_TRE_THREADS
}

void TREMainModel::draw(void)
{
	GLfloat normalSpecular[4];
	bool multiPass = false;

	treGlGetFloatv(GL_MODELVIEW_MATRIX, m_currentModelViewMatrix);
	treGlGetFloatv(GL_PROJECTION_MATRIX, m_currentProjectionMatrix);
	if (getSaveAlphaFlag() && (!getStippleFlag() || getAALinesFlag()))
	{
		GLint stencilBits;

		multiPass = true;
		glGetIntegerv(GL_STENCIL_BITS, &stencilBits);
		glPushAttrib(GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT |
			GL_STENCIL_BUFFER_BIT);
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_ALWAYS, 2, 0xFFFFFFFF);
		glStencilMask(0xFFFFFFFF);
		glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
	}
	glGetLightfv(GL_LIGHT0, GL_SPECULAR, normalSpecular);
	if (m_mainFlags.compileParts || m_mainFlags.compileAll)
	{
		compile();
	}
	launchWorkerThreads();
	triggerWorkerThreads();
	if (getEdgeLinesFlag() && !getWireframeFlag() && getPolygonOffsetFlag())
	{
		glPolygonOffset(POLYGON_OFFSET_FACTOR, POLYGON_OFFSET_UNITS);
		enable(GL_POLYGON_OFFSET_FILL);
	}
	else
	{
		disable(GL_POLYGON_OFFSET_FILL);
	}
	if (getWireframeFlag())
	{
		enableLineSmooth();
	}
	if (getEdgesOnlyFlag())
	{
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	}
	if (!getEdgesOnlyFlag() || !getWireframeFlag())
	{
		drawSolid();
	}
	if (getEdgesOnlyFlag())
	{
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	}
	// Next, disable lighting and draw lines.  First draw default colored lines,
	// which probably don't exist, since color number 16 doesn't often get used
	// for lines.
	if (getSaveAlphaFlag() && !getEdgesOnlyFlag())
	{
		passOnePrep();
		drawLines(1);
		passTwoPrep();
		drawLines(2);
		passThreePrep();
		drawLines(3);
		glPopAttrib();
	}
	else
	{
		drawLines();
	}
	if (getAALinesFlag() && getWireframeFlag())
	{
		// We use glPushAttrib() when we enable line smoothing.
		glPopAttrib();
	}
	drawTexmapped(false);
	if (!getTexturesAfterTransparentFlag())
	{
		// There is a trade-off. If textures are drawn before transparent
		// geometry, then everything looks great as long as the textures don't
		// contain alpha values that are semi-transparent. However, if a
		// texture attached to a transparent part includes semi-transparent
		// portions, those portions don't get blended with the underlying part,
		// which looks wrong.
		// If textures are drawn after transparent geometry, semi-transparent
		// portions look fine, but the textures show up in front of any
		// transparent geometry that is in front of them.
		drawTexmapped(true);
	}
	if (!getEdgesOnlyFlag() && !getRemovingHiddenLines())
	{
		if (getSaveAlphaFlag() && !getStippleFlag())
		{
			passOnePrep();
			drawTransparent(1);
			passTwoPrep();
			drawTransparent(2);
			passThreePrep();
			drawTransparent(3);
			glPopAttrib();
		}
		else
		{
			drawTransparent();
		}
	}
	if (getTexturesAfterTransparentFlag())
	{
		drawTexmapped(true);
	}
	if (multiPass)
	{
		glPopAttrib();
	}
	if (backgroundConditionalsNeeded())
	{
		for (int i = 0; i < 32; i++)
		{
			TCObject::release(m_activeConditionals[i]);
			TCObject::release(m_activeColorConditionals[i]);
		}
	}
	m_mainFlags.frameStarted = false;
//	checkNormals(m_vertexStore);
//	checkNormals(m_coloredVertexStore);
}

void TREMainModel::enable(GLenum cap)
{
	if (getGl2psFlag())
	{
		GLint mode = GL2PS_BLEND;

		if (cap == GL_POLYGON_OFFSET_FILL)
		{
			mode = GL2PS_POLYGON_OFFSET_FILL;
		}
		gl2psEnable(mode);
	}
	glEnable(cap);
}

void TREMainModel::disable(GLenum cap)
{
	if (getGl2psFlag())
	{
		GLint mode = GL2PS_BLEND;

		if (cap == GL_POLYGON_OFFSET_FILL)
		{
			mode = GL2PS_POLYGON_OFFSET_FILL;
		}
		gl2psDisable(mode);
	}
	glDisable(cap);
}

void TREMainModel::blendFunc(GLenum sfactor, GLenum dfactor)
{
	if (getGl2psFlag())
	{
		gl2psBlendFunc(sfactor, dfactor);
	}
	glBlendFunc(sfactor, dfactor);
}

void TREMainModel::lineWidth(GLfloat width)
{
	if (getGl2psFlag())
	{
		gl2psLineWidth(width);
	}
	glLineWidth(width);
}

void TREMainModel::pointSize(GLfloat size)
{
	if (getGl2psFlag())
	{
		gl2psPointSize(size);
	}
	glPointSize(size);
}

void TREMainModel::enableLineSmooth(int pass /*= -1*/)
{
	if (getAALinesFlag())
	{
		glPushAttrib(GL_ENABLE_BIT);
		glEnable(GL_LINE_SMOOTH);
		if (pass != 1)
		{
			enable(GL_BLEND);
			if (pass == 2)
			{
				blendFunc(GL_SRC_ALPHA_SATURATE, GL_ONE_MINUS_SRC_ALPHA);
			}
			else
			{
				blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}
		}
	}
}

void TREMainModel::drawSolid(void)
{
	bool subModelsOnly = false;

	deactivateBFC();
	if (TREShapeGroup::isTransparent(m_color, true))
	{
		subModelsOnly = true;
	}
	// I admit, this is a mess.  But I'm not sure how to make it less of a mess.
	// The various things do need to be drawn separately, and they have to get
	// drawn in a specific order.
	//
	// First, draw all opaque triangles and quads that are color number 16 (the
	// default color inherited from above.  Note that the actual drawing color
	// will generally be changed before each part, since you don't usually use
	// color number 16 when you use a part in your model.

	glColor4ubv((GLubyte*)&m_color);
	m_vertexStore->activate(m_mainFlags.compileAll || m_mainFlags.compileParts);
	TREModel::draw(TREMStandard, false, subModelsOnly);
	if (getStudLogoFlag())
	{
		glEnable(GL_TEXTURE_2D);
		bindStudTexture();
		configureStudTexture();
		m_studVertexStore->activate(m_mainFlags.compileAll ||
			m_mainFlags.compileParts);
		TREModel::draw(TREMStud, false, subModelsOnly);
		glDisable(GL_TEXTURE_2D);
	}
	if (getBFCFlag())
	{
		activateBFC();
		m_vertexStore->activate(m_mainFlags.compileAll ||
			m_mainFlags.compileParts);
		TREModel::draw(TREMBFC, false, subModelsOnly);
		if (getStudLogoFlag())
		{
			glEnable(GL_TEXTURE_2D);
			m_studVertexStore->activate(m_mainFlags.compileAll ||
				m_mainFlags.compileParts);
			TREModel::draw(TREMStudBFC, false, subModelsOnly);
			glDisable(GL_TEXTURE_2D);
		}
		deactivateBFC();
	}
	// Next draw all opaque triangles and quads that were specified with a color
	// number other than 16.  Note that the colored vertex store includes color
	// information for every vertex.
	m_coloredVertexStore->activate(m_mainFlags.compileAll ||
		m_mainFlags.compileParts);
	drawColored(TREMStandard);
	if (getStudLogoFlag())
	{
		glEnable(GL_TEXTURE_2D);
		m_coloredStudVertexStore->activate(m_mainFlags.compileAll ||
			m_mainFlags.compileParts);
		drawColored(TREMStud);
		glDisable(GL_TEXTURE_2D);
	}
	if (getBFCFlag())
	{
		activateBFC();
		m_coloredVertexStore->activate(m_mainFlags.compileAll ||
			m_mainFlags.compileParts);
		drawColored(TREMBFC);
		if (getStudLogoFlag())
		{
			glEnable(GL_TEXTURE_2D);
			m_coloredStudVertexStore->activate(m_mainFlags.compileAll ||
				m_mainFlags.compileParts);
			drawColored(TREMStudBFC);
			glDisable(GL_TEXTURE_2D);
		}
		deactivateBFC();
	}
	TREVertexStore::deactivateActiveVertexStore();
}

void TREMainModel::drawTexmappedInternal(
	bool texture,
	bool colorMaterialOff,
	bool transparent)
{
	for (TexmapInfoList::const_iterator it = m_mainTexmapInfos.begin();
		it != m_mainTexmapInfos.end(); it ++)
	{
		size_t i = 0;
		const IntSet *shapeSet = NULL;

		if (texture)
		{
			activateTexmap(*it);
		}
		if (transparent)
		{
			shapeSet = &it->transparent.colored.triangles;
			deactivateBFC(true);
			i = 2;
		}
		else if (it->bfc.colored.triangles.size() > 0)
		{
			shapeSet = &it->bfc.colored.triangles;
			// TODO Texmaps: BFC
			activateBFC();
			i = 1;
		}
		else if (it->standard.colored.triangles.size() > 0)
		{
			shapeSet = &it->standard.colored.triangles;
			// TODO Texmaps: BFC
			deactivateBFC(false);
		}
		if (colorMaterialOff)
		{
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			glDisable(GL_COLOR_MATERIAL);
		}
		if (m_texmappedShapes[i] == NULL)
		{
			continue;
		}
		if (shapeSet != NULL && shapeSet->size() > 0)
		{
			IntSet::const_iterator itShape;
			
			for (itShape = shapeSet->begin(); itShape != shapeSet->end();
				++itShape, ++itShape, ++itShape)
			{
				m_texmappedShapes[i]->drawShapeType(TRESTriangle, *itShape,
					3);
			}
		}
	}
}

void TREMainModel::drawTexmapped(bool transparent)
{
	if (m_mainTexmapInfos.size() > 0)
	{
		if (transparent)
		{
			// It is important that the textures on transparent geometry are
			// drawn at the same Z value as the transparent geometry.
			if (getEdgeLinesFlag() && !getWireframeFlag() && getPolygonOffsetFlag())
			{
				// Push textures on transparent geometry back the same amount
				// as the geometry itself.
				glPolygonOffset(POLYGON_OFFSET_FACTOR, POLYGON_OFFSET_UNITS);
				enable(GL_POLYGON_OFFSET_FILL);
			}
			else
			{
				// Don't offset on transparent geometry, because the original
				// transparent geometry wasn't offset.
				disable(GL_POLYGON_OFFSET_FILL);
			}
		}
		else
		{
			// Pull textures on opaque geometry forward, so that when they
			// draw they will be closer than the geometry they're drawing
			// over. Even pulling them this far, textures can disappear when
			// FOV is very low (like 0.1), but pulling it too far introduces
			// really bad artifacts.
			glPolygonOffset(-POLYGON_OFFSET_FACTOR * m_textureOffsetFactor,
				POLYGON_OFFSET_UNITS);
			enable(GL_POLYGON_OFFSET_FILL);
		}
		m_texmapVertexStore->activate(false);
		glPushAttrib(GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthFunc(GL_LEQUAL);
		configTexmaps();
		drawTexmappedInternal(true, true, transparent);
		disableTexmaps();
		glPopAttrib();
		m_texmapVertexStore->deactivate();
	}
}

void TREMainModel::drawLines(int pass /*= -1*/)
{
	if (getLightingFlag())
	{
		glDisable(GL_LIGHTING);
	}
	if (getAALinesFlag() && !getWireframeFlag())
	{
		// Note that if we're in wireframe mode, smoothing is already enabled.
		enableLineSmooth(pass);
		// Smooth lines produce odd effects on the edge of transparent surfaces
		// when depth writing is enabled, so disable.
		glDepthMask(GL_FALSE);
	}
	glColor4ubv((GLubyte*)&m_color);
	m_vertexStore->activate(m_mainFlags.compileAll || m_mainFlags.compileParts);
	TREModel::draw(TREMLines);
	// Next, switch to the default edge color, and draw the edge lines.  By
	// definition, edge lines in the original files use the default edge color.
	// However, if parts are flattened, they can contain sub-models of a
	// different color, which can lead to non-default colored edge lines.
	glColor4ubv((GLubyte*)&m_edgeColor);
	lineWidth(m_edgeLineWidth);
	if (getLineJoinsFlag() && m_edgeLineWidth > 1.0f)
	{
		pointSize(m_edgeLineWidth);
		m_mainFlags.activeLineJoins = true;
		glEnable(GL_POINT_SMOOTH);
		glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	}
	TREModel::draw(TREMEdgeLines);
	if (!getStencilConditionalsFlag())
	{
		m_vertexStore->deactivate();
		m_vertexStore->activate(false);
	}
	if (getConditionalLinesFlag())
	{
		TREModel::draw(TREMConditionalLines);
	}
	if (getLineJoinsFlag() && m_edgeLineWidth > 1.0f)
	{
		pointSize(1.0f);
		m_mainFlags.activeLineJoins = false;
	}
	// Next, draw the specific colored lines.  As with the specific colored
	// triangles and quads, every point in the vertex store specifies a color.
	m_coloredVertexStore->activate(m_mainFlags.compileAll ||
		m_mainFlags.compileParts);
	drawColored(TREMLines);
	// Next draw the specific colored edge lines.  Note that if it weren't for
	// the fact that edge lines can be turned off, these could simply be added
	// to the colored lines list.
	drawColored(TREMEdgeLines);
	if (!getStencilConditionalsFlag())
	{
		m_coloredVertexStore->deactivate();
		m_coloredVertexStore->activate(false);
	}
	if (getConditionalLinesFlag())
	{
		drawColored(TREMConditionalLines);
	}
	if (getAALinesFlag() && !getWireframeFlag())
	{
		// Note that if we're in wireframe mode, smoothing was enabled
		// elsewhere, and will therefore be disabled eleswhere.
		glDepthMask(GL_TRUE);
		// We use glPushAttrib() when we enable line smoothing.
		glPopAttrib();
	}
	if (getLightingFlag())
	{
		glEnable(GL_LIGHTING);
	}
	m_vertexStore->deactivate();
	m_coloredVertexStore->deactivate();
}

TREModel *TREMainModel::modelNamed(const char *name, bool bfc)
{
	return (TREMainModel *)getLoadedModels(bfc)->objectForKey(name);
}

void TREMainModel::registerModel(TREModel *model, bool bfc)
{
	getLoadedModels(bfc)->setObjectForKey(model, model->getName());
}

void TREMainModel::setColor(TCULong color, TCULong edgeColor)
{
	m_color = htonl(color);
	m_edgeColor = htonl(edgeColor);
}

TCULong TREMainModel::getColor(void)
{
	return htonl(m_color);
}

TCULong TREMainModel::getEdgeColor(void)
{
	return htonl(m_edgeColor);
}

void TREMainModel::setLightingFlag(bool value)
{
	m_mainFlags.lighting = value;
	m_vertexStore->setLightingFlag(value);
	m_studVertexStore->setLightingFlag(value);
	m_coloredVertexStore->setLightingFlag(value);
	m_coloredStudVertexStore->setLightingFlag(value);
	m_transVertexStore->setLightingFlag(value);
	m_texmapVertexStore->setLightingFlag(value);
}

void TREMainModel::setTwoSidedLightingFlag(bool value)
{
	m_mainFlags.twoSidedLighting = value;
	m_vertexStore->setTwoSidedLightingFlag(value);
	m_studVertexStore->setTwoSidedLightingFlag(value);
	m_coloredVertexStore->setTwoSidedLightingFlag(value);
	m_coloredStudVertexStore->setTwoSidedLightingFlag(value);
	m_transVertexStore->setTwoSidedLightingFlag(value);
	m_texmapVertexStore->setTwoSidedLightingFlag(value);
}

void TREMainModel::setShowAllConditionalFlag(bool value)
{
	m_mainFlags.showAllConditional = value;
	m_vertexStore->setShowAllConditionalFlag(value);
	m_studVertexStore->setShowAllConditionalFlag(value);
	m_coloredVertexStore->setShowAllConditionalFlag(value);
	m_coloredStudVertexStore->setShowAllConditionalFlag(value);
	m_transVertexStore->setShowAllConditionalFlag(value);
	m_texmapVertexStore->setShowAllConditionalFlag(value);
}

void TREMainModel::setConditionalControlPointsFlag(bool value)
{
	m_mainFlags.conditionalControlPoints = value;
	m_vertexStore->setConditionalControlPointsFlag(value);
	m_studVertexStore->setConditionalControlPointsFlag(value);
	m_coloredVertexStore->setConditionalControlPointsFlag(value);
	m_coloredStudVertexStore->setConditionalControlPointsFlag(value);
	m_transVertexStore->setConditionalControlPointsFlag(value);
	m_texmapVertexStore->setConditionalControlPointsFlag(value);
}

TCFloat TREMainModel::getMaxRadiusSquared(const TCVector &center)
{
	if (!m_maxRadiusSquared)
	{
		m_center = center;
		scanPoints(this,
			(TREScanPointCallback)&TREMainModel::scanMaxRadiusSquaredPoint,
			TCVector::getIdentityMatrix());
	}
	return m_maxRadiusSquared;
}

void TREMainModel::scanMaxRadiusSquaredPoint(const TCVector &point)
{
	TCFloat rSquared = (point - m_center).lengthSquared();

	if (rSquared > m_maxRadiusSquared)
	{
		m_maxRadiusSquared = rSquared;
	}
}

// By asking for the maximum radius squared, and then returning the square root
// of that, we only have to do one square root for the whole radius calculation.
// Otherwise, we would have to do one for every point.
TCFloat TREMainModel::getMaxRadius(const TCVector &center)
{
	return (TCFloat)sqrt(getMaxRadiusSquared(center));
}

bool TREMainModel::postProcess(void)
{
	int i;
	float numSections = (float)(TREMTransparent - TREMStandard);

	if (m_mainFlags.sendProgress)
	{
		TCProgressAlert::send("TREMainModel",
			TCLocalStrings::get(_UC("TREMainModelProcessing")), 0.0f, &m_abort,
			this);
	}
	if (m_abort)
	{
		return false;
	}
	transferTransparent();
	// Note: I DON'T want to check if the transparent section is present.
	// That's why I'm using < below, instead of <=.
	for (i = TREMFirst; i < TREMTransparent && !m_abort; i++)
	{
		float progress = (float)i / numSections * 0.8f + 0.2f;

		if (m_mainFlags.sendProgress)
		{
			TCProgressAlert::send("TREMainModel",
				TCLocalStrings::get(_UC("TREMainModelProcessing")), progress,
				&m_abort, this);
		}
		if (!m_abort)
		{
			checkSectionPresent((TREMSection)i);
			checkSectionPresent((TREMSection)i, true);
		}
	}
	if (m_abort)
	{
		return false;
	}
	if (m_mainFlags.sendProgress)
	{
		TCProgressAlert::send("TREMainModel",
			TCLocalStrings::get(_UC("TREMainModelProcessing")), 1.0f, &m_abort,
			this);
	}
	if (m_abort)
	{
		return false;
	}

	bindTexmaps();
	if (getCompilePartsFlag() || getCompileAllFlag())
	{
		compile();
	}

	return !m_abort;
}

void TREMainModel::transferTexmapped(void)
{
	SectionList sectionList;

	sectionList.push_back(TREMStandard);
	//sectionList.push_back(TREMStud);
	if (getBFCFlag())
	{
		sectionList.push_back(TREMBFC);
		//sectionList.push_back(TREMStudBFC);
	}
	transferTexmapped(sectionList);
	for (size_t i = 0; i < 3; i++)
	{
		for (size_t j = 1; j < m_texmappedStepCounts[i].size(); j++)
		{
			m_texmappedStepCounts[i][j] += m_texmappedStepCounts[i][j - 1];
		}
		if (m_texmappedShapes[i])
		{
			m_texmappedShapes[i]->setStepCounts(m_texmappedStepCounts[i]);
		}
	}
}

// We have to remove all the transparent objects from the whole tree after we've
// completely built the tree.  Before it is completely built, we don't know
// which items are transparent.  The actual removal is complicated.
//
// First, any geometry that has a specific color specified (and thus is in a
// TREColoredShapeGroup at the point this method is called) is simply removed
// from its parent model completely, and then added to the transparent triangle
// list (after being transformed into its coordinate system).  Strips, fans,
// and quads are all split into triangles.
//
// Next, any "default color" geometry that will in the end by transparent will
// be transferred to the triangle list (with coordinate transformation and
// strip/quad splitting).  When the non-transparent portions are drawn, they
// will completely skip default colored geomtry when the default color is
// transparent.
void TREMainModel::transferTransparent(void)
{
	SectionList sectionList;

	sectionList.push_back(TREMStandard);
	sectionList.push_back(TREMStud);
	if (getBFCFlag())
	{
		sectionList.push_back(TREMBFC);
		sectionList.push_back(TREMStudBFC);
	}
	transferPrep();
	transferTransparent(sectionList);
	for (size_t i = 1; i < m_transStepCounts.size(); i++)
	{
		m_transStepCounts[i] += m_transStepCounts[i - 1];
	}
	TRETransShapeGroup *transShapes =
		(TRETransShapeGroup *)m_coloredShapes[TREMTransparent];

	if (transShapes)
	{
		transShapes->setStepCounts(m_transStepCounts);
	}
}

void TREMainModel::transferTexmapped(const SectionList &sectionList)
{
	const TCFloat *matrix = TCVector::getIdentityMatrix();
	SectionList::const_iterator it;

	for (it = sectionList.begin(); it != sectionList.end(); ++it)
	{
		TREShapeGroup *shapeGroup = m_shapes[*it];
		TREColoredShapeGroup *coloredShapeGroup = m_coloredShapes[*it];

		if (shapeGroup)
		{
			shapeGroup->transfer(TREShapeGroup::TTTexmapped, m_color, matrix);
		}
		if (coloredShapeGroup)
		{
			coloredShapeGroup->transferColored(TREShapeGroup::TTTexmapped,
				matrix);
		}
		transferPrep();
		transferSubModels(TREShapeGroup::TTTexmapped, m_color, *it, matrix);
		//transferPrep();
		//transferColoredSubModels(TREShapeGroup::TTTexmapped, *it, matrix);
	}
	//if (m_subModels != NULL)
	//{
	//	int i;
	//	int count = m_subModels->getCount();

	//	for (i = 0; i < count; i++)
	//	{
	//		updateModelTransferStep(i);
	//		for (SectionList::const_iterator it = sectionList.begin();
	//			it != sectionList.end(); ++it)
	//		{
	//			TRESubModel *subModel = (*m_subModels)[i];

	//			subModel->transfer(TREShapeGroup::TTTexmapped, m_color, *it,
	//				matrix);
	//			subModel->transferColored(TREShapeGroup::TTTexmapped, *it,
	//				matrix);
	//		}
	//	}
	//}
	for (it = sectionList.begin(); it != sectionList.end(); ++it)
	{
		TREModel::cleanupTransfer(TREShapeGroup::TTTexmapped, *it);
	}
}

void TREMainModel::transferTransparent(const SectionList &sectionList)
{
	const TCFloat *matrix = TCVector::getIdentityMatrix();
	SectionList::const_iterator it;

	for (it = sectionList.begin(); it != sectionList.end(); ++it)
	{
		TREShapeGroup *shapeGroup = m_shapes[*it];
		TREColoredShapeGroup *coloredShapeGroup = m_coloredShapes[*it];

		if (shapeGroup)
		{
			shapeGroup->transfer(TREShapeGroup::TTTransparent, m_color, matrix);
		}
		if (coloredShapeGroup)
		{
			coloredShapeGroup->transferColored(TREShapeGroup::TTTransparent,
				matrix);
		}
	}
	if (m_subModels)
	{
		int i;
		int count = m_subModels->getCount();

		for (i = 0; i < count; i++)
		{
			updateModelTransferStep(i);
			for (it = sectionList.begin(); it != sectionList.end(); ++it)
			{
				TRESubModel *subModel = (*m_subModels)[i];

				subModel->transfer(TREShapeGroup::TTTransparent, m_color, *it,
					matrix);
				subModel->transferColored(TREShapeGroup::TTTransparent, *it,
					matrix);
			}
		}
	}
	for (it = sectionList.begin(); it != sectionList.end(); ++it)
	{
		TREModel::cleanupTransfer(TREShapeGroup::TTTransparent, *it);
	}
}

void TREMainModel::addTransferTriangle(
	TREShapeGroup::TRESTransferType type,
	TCULong color,
	const TCVector vertices[],
	const TCVector normals[],
	bool bfc,
	const TCVector *textureCoords,
	const TCFloat *matrix)
{
	if (type == TREShapeGroup::TTTransparent)
	{
		if (!m_coloredShapes[TREMTransparent])
		{
			m_coloredShapes[TREMTransparent] = new TRETransShapeGroup;
			m_coloredShapes[TREMTransparent]->setModel(this);
			m_coloredShapes[TREMTransparent]->setVertexStore(m_transVertexStore);
		}
		if (getStudLogoFlag() && getStudTextures())
		{
			if (textureCoords)
			{
				m_coloredShapes[TREMTransparent]->addTriangle(color, vertices,
					normals, textureCoords);
			}
			else
			{
				static TCVector zeroTextureCoords[3] =
				{
					TCVector(0.0, 0.0, 0.0),
					TCVector(0.0, 0.0, 0.0),
					TCVector(0.0, 0.0, 0.0)
				};

				m_coloredShapes[TREMTransparent]->addTriangle(color, vertices,
					normals, zeroTextureCoords);
			}
		}
		else
		{
			m_coloredShapes[TREMTransparent]->addTriangle(color, vertices, normals);
		}
		if (m_transStepCounts.size() <= (size_t)m_transferStep)
		{
			m_transStepCounts.resize(m_transferStep + 1);
		}
		m_transStepCounts[m_transferStep] += 3;
	}
	else // type == TREShapeGroup::TTTexmapped
	{
		int shapeIndex;
		bool mirror = bfc && TCVector::determinant(matrix) < 0.0f;
		TexmapInfo& texmapInfo = m_mainTexmapInfos.back();
		TREModel::TexmapInfo::GeomInfo *geomInfo = bfc ? &texmapInfo.bfc :
			&texmapInfo.standard;
		TCVector finalTextureCoords[3];

		if (TREShapeGroup::isTransparent(color, false))
		{
			shapeIndex = 2;
			geomInfo = &m_mainTexmapInfos.back().transparent;
		}
		else
		{
			shapeIndex = bfc ? 1 : 0;
		}
		texmapInfo.calcTextureCoords(vertices, finalTextureCoords);
		if (m_texmappedShapes[shapeIndex] == NULL)
		{
			m_texmappedShapes[shapeIndex] = new TRETexmappedShapeGroup;
			m_texmappedShapes[shapeIndex]->setModel(this);
			m_texmappedShapes[shapeIndex]->setVertexStore(m_texmapVertexStore);
		}
		if (mirror)
		{
			TCVector mirroredVertices[3];
			TCVector *mirroredNormals = NULL;
			int i;

			for (i = 0; i < 3; i++)
			{
				mirroredVertices[2 - i] = vertices[i];
			}
			if (normals != NULL)
			{
				mirroredNormals = new TCVector[3];

				for (i = 0; i < 3; i++)
				{
					mirroredNormals[2 - i] = -normals[i];
				}
			}
			std::swap(finalTextureCoords[0], finalTextureCoords[2]);
			m_texmappedShapes[shapeIndex]->addTriangle(0xFFFFFFFF,
				mirroredVertices, mirroredNormals, finalTextureCoords);
			delete[] mirroredNormals;
		}
		else
		{
			m_texmappedShapes[shapeIndex]->addTriangle(0xFFFFFFFF, vertices,
				normals, finalTextureCoords);
		}
		int indexCount =
			m_texmappedShapes[shapeIndex]->getIndexCount(TRESTriangle);
		if (mirror)
		{
			for (int i = 2; i >= 0; i--)
			{
				geomInfo->colored.triangles.insert(indexCount - 3 + i);
			}
		}
		else
		{
			for (int i = 0; i < 3; i++)
			{
				geomInfo->colored.triangles.insert(indexCount - 3 + i);
			}
		}
		if (m_texmappedStepCounts[shapeIndex].size() <= (size_t)m_transferStep)
		{
			m_texmappedStepCounts[shapeIndex].resize(m_transferStep + 1);
		}
		m_texmappedStepCounts[shapeIndex][m_transferStep] += 3;
	}
}

bool TREMainModel::onLastStep(void)
{
	return m_step == -1 || m_step == m_numSteps - 1;
}

void TREMainModel::drawTransparent(int pass /*= -1*/)
{
	if (m_coloredShapes[TREMTransparent])
	{
		GLfloat specular[] = {0.75f, 0.75f, 0.75f, 1.0f};
		GLfloat oldSpecular[4];
		GLfloat oldShininess;

		deactivateBFC(true);
		if (getStudLogoFlag())
		{
			glEnable(GL_TEXTURE_2D);
			bindStudTexture();
			// Don't allow mip-mapping for transparent textures; it might cause
			// artifacts.
			configureStudTexture(false);
		}
		glGetMaterialfv(GL_FRONT, GL_SHININESS, &oldShininess);
		glGetMaterialfv(GL_FRONT, GL_SPECULAR, oldSpecular);
		m_transVertexStore->activate(!m_mainFlags.sortTransparent);
		if (getStippleFlag())
		{
			glEnable(GL_POLYGON_STIPPLE);
		}
		else if (!getCutawayDrawFlag())
		{
			if (pass != 1)
			{
				enable(GL_BLEND);
				if (pass == 2)
				{
					blendFunc(GL_SRC_ALPHA_SATURATE, GL_ONE_MINUS_SRC_ALPHA);
				}
				else
				{
					blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				}
				glDepthMask(GL_FALSE);
			}
		}
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 128.0f);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
		if (!getWireframeFlag() && getPolygonOffsetFlag())
		{
			if (getEdgeLinesFlag())
			{
				disable(GL_POLYGON_OFFSET_FILL);
//				glPolygonOffset(0.0f, 0.0f);
			}
			else
			{
				glPolygonOffset(-POLYGON_OFFSET_FACTOR, -POLYGON_OFFSET_UNITS);
				enable(GL_POLYGON_OFFSET_FILL);
			}
		}
		if (onLastStep() && m_coloredListIDs[TREMTransparent])
		{
			glCallList(m_coloredListIDs[TREMTransparent]);
		}
		else
		{
			((TRETransShapeGroup *)m_coloredShapes[TREMTransparent])->
				draw(getSortTransparentFlag() && !getCutawayDrawFlag() &&
				pass < 2);
		}
		if (getStudLogoFlag())
		{
			glDisable(GL_TEXTURE_2D);
		}
		disable(GL_POLYGON_OFFSET_FILL);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, oldSpecular);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, oldShininess);
		if (getStippleFlag())
		{
			glDisable(GL_POLYGON_STIPPLE);
		}
		else if (!getCutawayDrawFlag())
		{
			disable(GL_BLEND);
			glDepthMask(GL_TRUE);
		}
		m_transVertexStore->deactivate();
	}
}

bool TREMainModel::shouldLoadConditionalLines(void)
{
	return (m_mainFlags.edgeLines && m_mainFlags.conditionalLines) ||
		m_mainFlags.smoothCurves;
}

void TREMainModel::configTextureFilters(void)
{
	if (m_studTextureFilter == GL_NEAREST_MIPMAP_NEAREST ||
		m_studTextureFilter == GL_NEAREST_MIPMAP_LINEAR ||
		m_studTextureFilter == GL_NEAREST)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	if (m_studTextureFilter == GL_LINEAR_MIPMAP_LINEAR)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			m_studTextureFilter);
	}
	else
	{
		if (m_studTextureFilter == GL_NEAREST)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
				GL_NEAREST);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
				GL_LINEAR);
		}
		//glTexImage2D(GL_TEXTURE_2D, 0, format, info.image->getWidth(),
		//	info.image->getHeight(), 0, format, GL_UNSIGNED_BYTE,
		//	info.image->getImageData());
	}
	if (m_studTextureFilter == GL_LINEAR_MIPMAP_LINEAR &&
		TREGLExtensions::haveAnisoExtension())
	{
		GLfloat aniso = 1.0f;

		if (m_studTextureFilter == GL_LINEAR_MIPMAP_LINEAR)
		{
			aniso = m_studAnisoLevel;
		}
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT,
			aniso);
	}
}

void TREMainModel::configTexmaps(void)
{
	if (m_texmapImages.size() > 0)
	{
		for (TexmapImageInfoMap::iterator it = m_texmapImages.begin();
			it != m_texmapImages.end(); ++it)
		{
			TexmapImageInfo &info = it->second;
			GLint textureMode = GL_MODULATE;

			if (!getLightingFlag())
			{
				textureMode = GL_REPLACE;
			}
			glBindTexture(GL_TEXTURE_2D, info.textureID);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, textureMode);
			configTextureFilters();
		}
	}
}

void TREMainModel::bindTexmaps(void)
{
	if (m_texmapImages.size() > 0)
	{
		std::vector<GLuint> textureIDs;
		size_t i = 0;

		if (TREGLExtensions::haveClampToBorderExtension())
		{
			m_texClampMode = GL_CLAMP_TO_BORDER;
		}
		textureIDs.resize(m_texmapImages.size());
		glGenTextures((GLsizei)m_texmapImages.size(), &textureIDs[0]);
		for (TexmapImageInfoMap::iterator it = m_texmapImages.begin();
			it != m_texmapImages.end(); ++it)
		{
			TexmapImageInfo &info = it->second;
			if (info.image == NULL)
			{
				continue;
			}

			info.textureID = textureIDs[i++];
			if (info.image->getDataFormat() == TCRgb8)
			{
				// Source image is RGB; we need to convert to RGBA.  We can't just
				// send the RGB data to OpenGL because that causes it to use black
				// for all pixels outside the border of the texture instead of
				// transparent.
				TCImage *rgbaImage = new TCImage;
				int width = info.image->getWidth();
				int height = info.image->getHeight();
				TCByte *src = info.image->getImageData();
				TCByte *dst = NULL;
				int srcPadding = info.image->getRowSize() - width * 3;

				// Note: no need to set the line alignment to 4, because each pixel
				// is 4 bytes, so we're already guaranteed to have that alignment.
				rgbaImage->setDataFormat(TCRgba8);
				rgbaImage->setSize(width, height);
				rgbaImage->allocateImageData();
				dst = rgbaImage->getImageData();
				for (int row = 0; row < height; row++)
				{
					for (int col = 0; col < width; col++)
					{
						for (int component = 0; component < 3; component++)
						{
							*dst++ = *src++;
						}
						*dst++ = 255;	// Fully opaque.
					}
					src += srcPadding;
				}
				info.image->release();
				info.image = rgbaImage;
			}
			glBindTexture(GL_TEXTURE_2D, info.textureID);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_texClampMode);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_texClampMode);
			//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			configTextureFilters();
			gluBuild2DMipmaps(GL_TEXTURE_2D, 4, info.image->getWidth(),
				info.image->getHeight(), GL_RGBA, GL_UNSIGNED_BYTE,
				info.image->getImageData());
		}
	}
}

void TREMainModel::bindStudTexture(void)
{
	if (!sm_studTextureID && sm_studTextures)
	{
		int i;
		int count = sm_studTextures->getCount();

		glGenTextures(1, (GLuint*)&sm_studTextureID);
		glBindTexture(GL_TEXTURE_2D, sm_studTextureID);
		for (i = 0; i < count; i++)
		{
			TCImage *texture = (*sm_studTextures)[i];

			if (texture)
			{
				int textureSize = texture->getWidth();

				glTexImage2D(GL_TEXTURE_2D, i, 4, textureSize, textureSize, 0,
					GL_RGBA, GL_UNSIGNED_BYTE, texture->getImageData());
			}
		}
	}
	if (sm_studTextureID)
	{
		glBindTexture(GL_TEXTURE_2D, sm_studTextureID);
	}
}

void TREMainModel::configureStudTexture(bool allowMipMap)
{
	if (sm_studTextureID)
	{
		glBindTexture(GL_TEXTURE_2D, sm_studTextureID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_texClampMode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_texClampMode);
		// Note: if the card doesn't support the following extension, it will
		// just silently fail, and nothing bad will have happened.  If it is
		// supported, it makes textures a bit sharper, with a higher possibility
		// of moire.
		glTexEnvf(GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, -0.5f);
		while (glGetError() != GL_NO_ERROR)
		{
			// Don't do anything; just loop until error flag(s) is/are clear.
		}
		if (m_studTextureFilter == GL_NEAREST_MIPMAP_NEAREST ||
			m_studTextureFilter == GL_NEAREST_MIPMAP_LINEAR ||
			m_studTextureFilter == GL_NEAREST)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		if (allowMipMap)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
				m_studTextureFilter);
		}
		else
		{
			if (m_studTextureFilter == GL_NEAREST)
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
					GL_NEAREST);
			}
			else
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
					GL_LINEAR);
			}
		}
		if (TREGLExtensions::haveAnisoExtension())
		{
			GLfloat aniso = 1.0f;

			if (m_studTextureFilter == GL_LINEAR_MIPMAP_LINEAR)
			{
				aniso = m_studAnisoLevel;
			}
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT,
				aniso);
		}
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	}
}

// NOTE: static function
void TREMainModel::loadStudTexture(const char *filename)
{
	if (!sm_studTextures)
	{
		TCImage *mainImage = new TCImage;

		mainImage->setFlipped(true);
		mainImage->setLineAlignment(4);
		if (mainImage->loadFile(filename))
		{
			loadStudMipTextures(mainImage);
		}
		mainImage->release();
	}
}

// NOTE: static function
void TREMainModel::loadStudMipTextures(TCImage *mainImage)
{
	int i;
	int yPosition = 0;
	int thisSize = mainImage->getWidth();
	int imageHeight = mainImage->getHeight();

	TCObject::release(sm_studTextures);
	sm_studTextures = new TCImageArray(8);
	for (i = 0; thisSize > 0 && yPosition + thisSize <= imageHeight; i++)
	{
		TCImage *texture = mainImage->createSubImage(0, yPosition,
			thisSize, thisSize);
		sm_studTextures->addObject(texture);
		texture->release();
		yPosition += thisSize;
		thisSize /= 2;
	}
}

// NOTE: static function
void TREMainModel::setStudTextureData(TCByte *data, long length)
{
	if (!sm_studTextures)
	{
		TCImage *mainImage = new TCImage;

		mainImage->setFlipped(true);
		mainImage->setLineAlignment(4);
		if (mainImage->loadData(data, length))
		{
			loadStudMipTextures(mainImage);
		}
		mainImage->release();
	}
}

// NOTE: static function
void TREMainModel::setRawStudTextureData(TCByte *data, long length)
{
	if (!sm_studTextures)
	{
		TCImage *mainImage = new TCImage;
		int rowSize;

		mainImage->setFlipped(true);
		mainImage->setLineAlignment(4);
		mainImage->setDataFormat(TCRgba8);
		mainImage->setSize(128, 255);
		mainImage->allocateImageData();
		rowSize = mainImage->getRowSize();
		if (length == rowSize * 255)
		{
			TCByte *imageData = mainImage->getImageData();
			int i;

			for (i = 0; i < 255; i++)
			{
				memcpy(imageData + rowSize * (254 - i), data + rowSize * i,
					rowSize);
			}
			loadStudMipTextures(mainImage);
		}
		mainImage->release();
	}
}

void TREMainModel::openGlWillEnd(void)
{
	uncompile();
	m_vertexStore->openGlWillEnd();
	m_studVertexStore->openGlWillEnd();
	m_coloredVertexStore->openGlWillEnd();
	m_coloredStudVertexStore->openGlWillEnd();
	m_transVertexStore->openGlWillEnd();
	if (sm_studTextureID)
	{
		glDeleteTextures(1, &TREMainModel::sm_studTextureID);
		sm_studTextureID = 0;
	}
	deleteGLTexmaps();
}

void TREMainModel::deleteGLTexmaps(void)
{
	std::vector<GLuint> textureIDs;

	textureIDs.reserve(m_texmapImages.size());
	for (TexmapImageInfoMap::iterator it = m_texmapImages.begin();
		it != m_texmapImages.end(); ++it)
	{
		TexmapImageInfo &info = it->second;

		if (info.textureID != 0)
		{
			textureIDs.push_back(info.textureID);
			info.textureID = 0;
		}
	}
	if (textureIDs.size() > 0)
	{
		glDeleteTextures((GLsizei)textureIDs.size(), &textureIDs[0]);
	}
}

void TREMainModel::finish(void)
{
	// transferTexmapped() has to happen before finishParts does any part
	// flattening.  And it has to happen after shrinkParts().
	if (m_seamWidth != 0.0 && !isPart())
	{
		shrinkParts();
	}
	transferTexmapped();
	//flattenNonUniform();
	finishParts();
	findLights();
	flattenConditionals();
	if (m_stepCounts.size() > 0)
	{
		if (m_subModels->getCount() > m_stepCounts.back())
		{
			m_numSteps = (int)m_stepCounts.size() + 1;
		}
		else
		{
			m_numSteps = (int)m_stepCounts.size();
		}
	}
	else
	{
		m_numSteps = 1;
	}
	if (getSmoothCurvesFlag())
	{
		// We are smoothing curves; check to see if we have any texmapped
		// triangles.
		TRETrianglesMap triangles[2];
		int i;

		for (i = 0; i < 2; i++)
		{
			if (m_texmappedShapes[i] != NULL &&
				m_texmappedShapes[i]->getIndexCount(TRESTriangle) > 0)
			{
				populateTrianglesMap(m_texmappedShapes[i], triangles[i]);
			}
		}
		transferSmoothNormals(triangles);
	}
}

bool TREMainModel::transferSmoothNormals(
	const TRETrianglesMap &triangles,
	TREVertexStore *vertexStore,
	TCULongArray *indices,
	TREVertexStore *dstVertexStore,
	TCULongArray *dstIndices,
	int i0,
	int i1,
	int i2,
	const TCFloat *matrix)
{
	int triIndices[3] = { i0, i1, i2 };
	TRETrianglesMap::const_iterator itMap = triangles.end();
	int i;
	TREVertex triangleVerts[3];
	TREVertexArray *vertices = vertexStore->getVertices();
	TREVertexArray *normals = vertexStore->getNormals();
	TREVertexArray *dstNormals = dstVertexStore->getNormals();

	for (i = 0; i < 3; i++)
	{
		triangleVerts[i] = (*vertices)[(*indices)[triIndices[i]]];
		TCVector vec(
			TCVector(triangleVerts[i].v).transformPoint(matrix));
		memcpy(triangleVerts[i].v, &vec[0],
			sizeof(triangleVerts[i].v));
		if (i == 0)
		{
			itMap = triangles.find(triangleVerts[0]);
			if (itMap == triangles.end())
			{
				break;
			}
		}
	}
	if (itMap != triangles.end())
	{
		const TRETriangleList &triList = itMap->second;
		TRETriangleList::const_iterator itList;

		for (itList = triList.begin(); itList != triList.end();
			++itList)
		{
			const TRETriangle &triangle = *itList;
			bool match1 = true;
			bool match2 = false;

			for (i = 0; i < 3; i++)
			{
				if (!triangle.points[i]->approxEquals(triangleVerts[i],
					1e-5f))
				{
					match1 = false;
					break;
				}
			}
			if (!match1)
			{
				match2 = true;
				for (i = 0; i < 3; i++)
				{
					if (!triangle.points[2 - i]->approxEquals(triangleVerts[i],
						1e-5f))
					{
						match2 = false;
						break;
					}
				}
			}
			if (match1 || match2)
			{
				// This is our texmapped triangle.
				for (i = 0; i < 3; i++)
				{
					int dstNormalIndex;
					TREVertex normal = (*normals)[(*indices)[triIndices[i]]];
					TCVector normalVec(
						TCVector(normal.v).transformNormal(matrix));
					if (match1)
					{
						dstNormalIndex = (*dstIndices)[triangle.index + i];
					}
					else
					{
						dstNormalIndex = (*dstIndices)[triangle.index + 2 - i];
					}
					TREVertex &dstNormal = (*dstNormals)[dstNormalIndex];
					TCVector dstNormalVec(dstNormal.v);
					if (TRESmoother::shouldFlipNormal(normalVec, dstNormalVec))
					{
						normalVec *= -1.0f;
					}
					memcpy(dstNormal.v, &normalVec[0], sizeof(dstNormal.v));
				}
				return true;
			}
		}
	}
	return false;
}

void TREMainModel::transferSmoothNormals(
	const TRETrianglesMap &triangles,
	TREShapeGroup *shapeGroup,
	TREShapeType shapeType,
	int shapeSize,
	const TCFloat *matrix)
{
	int dstIndex = shapeGroup->getBfc() ? 1 : 0;
	TREShapeGroup *dstShapeGroup = m_texmappedShapes[dstIndex];

	if (dstShapeGroup != NULL)
	{
		TREVertexStore *dstVertexStore = dstShapeGroup->getVertexStore();
		TCULongArray *indices = shapeGroup->getIndices(shapeType);
		TCULongArray *dstIndices = dstShapeGroup->getIndices(TRESTriangle);
		int count = shapeGroup->getIndexCount(shapeType);
		TREVertexStore *vertexStore = shapeGroup->getVertexStore();

		if (count > 0 && indices != NULL && vertexStore != NULL &&
			dstVertexStore != NULL)
		{
			for (int i = count - shapeSize; i >= 0; i -= shapeSize)
			{
				bool found = transferSmoothNormals(triangles, vertexStore,
					indices, dstVertexStore, dstIndices, i, i + 1, i + 2, matrix);
				if (shapeSize == 4)
				{
					transferSmoothNormals(triangles, vertexStore, indices,
						dstVertexStore, dstIndices, i, i + 2, i + 3, matrix);
				}
				if (!found && shapeSize == 4)
				{
					transferSmoothNormals(triangles, vertexStore,
						indices, dstVertexStore, dstIndices, i + 3, i + 2, i + 1, matrix);
					if (shapeSize == 4)
					{
						transferSmoothNormals(triangles, vertexStore, indices,
							dstVertexStore, dstIndices, i + 3, i + 1, i + 0, matrix);
					}
				}
			}
		}
	}
}

void TREMainModel::transferSmoothNormals(
	const TRETrianglesMap &triangles,
	TREShapeGroup *shapeGroup,
	const TCFloat *matrix)
{
	if (shapeGroup != NULL)
	{
		transferSmoothNormals(triangles, shapeGroup, TRESTriangle, 3, matrix);
		transferSmoothNormals(triangles, shapeGroup, TRESQuad, 4, matrix);
	}
}

void TREMainModel::transferSmoothNormals(
	const TRETrianglesMap triangles[],
	TREModel *model,
	const TCFloat *matrix)
{
	if (model != NULL)
	{
		int subModelCount = model->getSubModelCount();
		TRESubModelArray *subModels = model->getSubModels();

		transferSmoothNormals(triangles[0], model->getShape(TREMStandard), matrix);
		transferSmoothNormals(triangles[1], model->getShape(TREMBFC), matrix);
		transferSmoothNormals(triangles[0], model->getColoredShape(TREMStandard), matrix);
		transferSmoothNormals(triangles[1], model->getColoredShape(TREMBFC), matrix);
		for (int i = 0; i < subModelCount; i++)
		{
			TRESubModel *subModel = (*subModels)[i];
			TCFloat newMatrix[16];

			TCVector::multMatrix(matrix, subModel->getMatrix(), newMatrix);
			transferSmoothNormals(triangles, subModel->getModel(), newMatrix);
		}
	}
}

void TREMainModel::transferSmoothNormals(const TRETrianglesMap triangles[])
{
	TCFloat matrix[16];

	TCVector::initIdentityMatrix(matrix);
	transferSmoothNormals(triangles, this, matrix);
}

void TREMainModel::populateTrianglesMap(
	TRETexmappedShapeGroup *shapeGroup,
	TRETrianglesMap &triangles)
{
	TCULongArray *indices = shapeGroup->getIndices(TRESTriangle);
	TREVertexStore *vertexStore = shapeGroup->getVertexStore();

	if (indices != NULL && vertexStore != NULL)
	{
		TREVertexArray *vertices = vertexStore->getVertices();

		if (vertices != NULL)
		{
			int count = shapeGroup->getIndexCount(TRESTriangle);
			TRETriangle triangle;
			int i, j;

			memset(&triangle, 0, sizeof(triangle));
			for (i = 0; i < count; i += 3)
			{
				triangle.index = i;
				for (j = 0; j < 3; j++)
				{
					triangle.points[j] = &(*vertices)[(*indices)[i + j]];
				}
				for (j = 0; j < 3; j++)
				{
					triangles[*triangle.points[j]].push_back(triangle);
				}
			}
		}
	}
}

void TREMainModel::addLight(const TCVector &location, TCULong color)
{
	m_lightLocations.push_back(location);
	m_lightColors.push_back(color);
}

void TREMainModel::flattenConditionals(void)
{
	if (doingBackgroundConditionals())
	{
		setup(TREMConditionalLines);
		m_shapes[TREMConditionalLines]->getVertexStore()->setup();
		setupColored(TREMConditionalLines);
		m_coloredShapes[TREMConditionalLines]->getVertexStore()->setupColored();
		transferPrep();
		TREModel::flattenConditionals(TCVector::getIdentityMatrix(), 0, false);
		removeConditionals();
	}
}

void TREMainModel::transferPrep(void)
{
	m_transferStep = 0;
}

void TREMainModel::updateModelTransferStep(
	int subModelIndex,
	bool isConditionals /*= false*/)
{
	if (m_stepCounts.size() > (size_t)m_transferStep)
	{
		if (m_stepCounts[m_transferStep] <= subModelIndex)
		{
			if (isConditionals)
			{
				TREShapeGroup *coloredShapeGroup =
					m_coloredShapes[TREMConditionalLines];
				if (coloredShapeGroup != NULL)
				{
					coloredShapeGroup->updateConditionalsStepCount(
						m_transferStep);
				}
			}
			m_transferStep++;
		}
	}
}

void TREMainModel::setStep(int value)
{
	m_step = value;
	TRETransShapeGroup *transShapes =
		(TRETransShapeGroup *)m_coloredShapes[TREMTransparent];

	if (transShapes)
	{
		transShapes->stepChanged();
	}
}

void TREMainModel::startTexture(
	int type,
	const std::string &filename,
	TCImage *image,
	const TCVector *points,
	const TCFloat *extra)
{
	getCurGeomModel()->startTexture(type, filename, image, points, extra);
	TREModel::startTexture(type, filename, image, points, extra);
}

TREModel *TREMainModel::getCurGeomModel(void)
{
	if (m_curGeomModel == NULL)
	{
		m_curGeomModel = new TREModel;

		m_curGeomModel->setMainModel(this);
		m_curGeomModel->setName("TREMainModel GeomModel");
		addSubModel(TCVector::getIdentityMatrix(), m_curGeomModel, false);
		m_curGeomModel->release();
	}
	return m_curGeomModel;
}

void TREMainModel::nextStep(void)
{
	if (m_subModels == NULL || (m_stepCounts.size() > 0 &&
		m_subModels->getCount() == m_stepCounts.back()) ||
		(m_stepCounts.size() == 0 && m_subModels->getCount() == 0))
	{
		// Empty step; ignore.
		return;
	}
	m_curStepIndex++;
	for (int i = 0; i <= TREMLast; i++)
	{
		if (m_shapes[i])
		{
			m_shapes[i]->nextStep();
		}
		if (m_coloredShapes[i])
		{
			m_coloredShapes[i]->nextStep();
		}
	}
	if (m_subModels)
	{
		m_stepCounts.push_back(m_subModels->getCount());
	}
	else
	{
		m_stepCounts.push_back(0);
	}
	m_curGeomModel = NULL;
}

void TREMainModel::addLine(TCULong color, const TCVector *vertices)
{
	getCurGeomModel()->addLine(color, vertices);
}

void TREMainModel::addLine(const TCVector *vertices)
{
	getCurGeomModel()->addLine(vertices);
}

void TREMainModel::addEdgeLine(const TCVector *vertices, TCULong color)
{
	getCurGeomModel()->addEdgeLine(vertices, color);
}

void TREMainModel::addTriangle(TCULong color, const TCVector *vertices)
{
	getCurGeomModel()->addTriangle(color, vertices);
}

void TREMainModel::addTriangle(
	TCULong color,
	const TCVector *vertices,
	const TCVector *normals)
{
	getCurGeomModel()->addTriangle(color, vertices, normals);
}

void TREMainModel::addTriangle(const TCVector *vertices)
{
	getCurGeomModel()->addTriangle(vertices);
}

void TREMainModel::addTriangle(
	const TCVector *vertices,
	const TCVector *normals)
{
	getCurGeomModel()->addTriangle(vertices, normals);
}

void TREMainModel::addBFCTriangle(TCULong color, const TCVector *vertices)
{
	getCurGeomModel()->addBFCTriangle(color, vertices);
}

void TREMainModel::addBFCTriangle(
	TCULong color,
	const TCVector *vertices,
	const TCVector *normals)
{
	getCurGeomModel()->addBFCTriangle(color, vertices, normals);
}

void TREMainModel::addBFCTriangle(const TCVector *vertices)
{
	getCurGeomModel()->addBFCTriangle(vertices);
}

void TREMainModel::addBFCTriangle(
	const TCVector *vertices,
	const TCVector *normals)
{
	getCurGeomModel()->addBFCTriangle(vertices, normals);
}

void TREMainModel::addQuad(TCULong color, const TCVector *vertices)
{
	getCurGeomModel()->addQuad(color, vertices);
}

void TREMainModel::addQuad(const TCVector *vertices)
{
	getCurGeomModel()->addQuad(vertices);
}

void TREMainModel::addBFCQuad(TCULong color, const TCVector *vertices)
{
	getCurGeomModel()->addBFCQuad(color, vertices);
}

void TREMainModel::addBFCQuad(const TCVector *vertices)
{
	getCurGeomModel()->addBFCQuad(vertices);
}

void TREMainModel::addConditionalLine(
	const TCVector *vertices,
	const TCVector *controlPoints,
	TCULong color)
{
	getCurGeomModel()->addConditionalLine(vertices, controlPoints, color);
}

void TREMainModel::addTriangleStrip(
	const TCVector *vertices,
	const TCVector *normals,
	int count,
	bool flat)
{
	getCurGeomModel()->addTriangleStrip(vertices, normals, count, flat);
}

void TREMainModel::addTriangleStrip(
	TREShapeGroup *shapeGroup,
	const TCVector *vertices,
	const TCVector *normals,
	int count,
	bool flat)
{
	getCurGeomModel()->addTriangleStrip(shapeGroup, vertices, normals, count,
		flat);
}

void TREMainModel::addBFCTriangleStrip(
	const TCVector *vertices,
	const TCVector *normals,
	int count,
	bool flat)
{
	getCurGeomModel()->addBFCTriangleStrip(vertices, normals, count, flat);
}

void TREMainModel::addTriangleFan(
	const TCVector *vertices,
	const TCVector *normals,
	const TCVector *textureCoords,
	int count,
	bool flat)
{
	getCurGeomModel()->addTriangleFan(vertices, normals, textureCoords, count,
		flat);
}

void TREMainModel::addTriangleFan(
	TREShapeGroup *shapeGroup,
	const TCVector *vertices,
	const TCVector *normals,
	const TCVector *textureCoords,
	int count,
	bool flat)
{
	getCurGeomModel()->addTriangleFan(shapeGroup, vertices, normals,
		textureCoords, count, flat);
}

void TREMainModel::addTriangleFan(
	TCULong color,
	const TCVector *vertices,
	const TCVector *normals,
	int count,
	bool flat)
{
	getCurGeomModel()->addTriangleFan(color, vertices, normals, count, flat);
}

void TREMainModel::addTriangleFan(
	TREColoredShapeGroup *shapeGroup,
	TCULong color,
	const TCVector *vertices,
	const TCVector *normals,
	int count,
	bool flat)
{
	getCurGeomModel()->addTriangleFan(shapeGroup, color, vertices, normals,
		count, flat);
}

void TREMainModel::addBFCTriangleFan(
	const TCVector *vertices,
	const TCVector *normals,
	const TCVector *textureCoords,
	int count,
	bool flat)
{
	getCurGeomModel()->addBFCTriangleFan(vertices, normals, textureCoords,
		count, flat);
}

void TREMainModel::addBFCTriangleFan(
	TCULong color,
	const TCVector *vertices,
	const TCVector *normals,
	int count,
	bool flat)
{
	getCurGeomModel()->addBFCTriangleFan(color, vertices, normals, count, flat);
}

void TREMainModel::addQuadStrip(
	const TCVector *vertices,
	const TCVector *normals,
	int count,
	bool flat)
{
	getCurGeomModel()->addQuadStrip(vertices, normals, count, flat);
}

void TREMainModel::addQuadStrip(
	TREShapeGroup *shapeGroup,
	const TCVector *vertices,
	const TCVector *normals,
	int count,
	bool flat)
{
	getCurGeomModel()->addQuadStrip(shapeGroup, vertices, normals, count, flat);
}

void TREMainModel::addQuadStrip(
	TCULong color,
	const TCVector *vertices,
	const TCVector *normals,
	int count,
	bool flat)
{
	getCurGeomModel()->addQuadStrip(color, vertices, normals, count, flat);
}

void TREMainModel::addQuadStrip(
	TREColoredShapeGroup *shapeGroup,
	TCULong color,
	const TCVector *vertices,
	const TCVector *normals,
	int count,
	bool flat)
{
	getCurGeomModel()->addQuadStrip(shapeGroup, color, vertices, normals, count,
		flat);
}

void TREMainModel::addBFCQuadStrip(
	const TCVector *vertices,
	const TCVector *normals,
	int count,
	bool flat)
{
	getCurGeomModel()->addBFCQuadStrip(vertices, normals, count, flat);
}

void TREMainModel::addBFCQuadStrip(
	TCULong color,
	const TCVector *vertices,
	const TCVector *normals,
	int count,
	bool flat)
{
	getCurGeomModel()->addBFCQuadStrip(color, vertices, normals, count, flat);
}

void TREMainModel::startTexture(const std::string &filename, TCImage *image)
{
	loadTexture(filename, image);
	m_activeTextures.push_front(filename);
}

bool TREMainModel::endTexture(void)
{
	if (m_activeTextures.empty())
	{
		return false;
	}
	m_activeTextures.pop_front();
	return true;
}

void TREMainModel::loadTexture(const std::string &filename, TCImage *image)
{
	if (m_texmapImages.find(filename) == m_texmapImages.end())
	{
		m_texmapImages[filename] = TexmapImageInfo(filename, image);
	}
}

const std::string *TREMainModel::getActiveTextureFilename(void) const
{
	if (m_activeTextures.size() > 0)
	{
		return &m_activeTextures.front();
	}
	else
	{
		return NULL;
	}
}

GLuint TREMainModel::getTexmapTextureID(const std::string &filename) const
{
	TexmapImageInfoMap::const_iterator it = m_texmapImages.find(filename);

	if (it != m_texmapImages.end())
	{
		return it->second.textureID;
	}
	else
	{
		return 0;
	}
}

const TCImage *TREMainModel::getTexmapImage(const std::string &filename) const
{
	TexmapImageInfoMap::const_iterator it = m_texmapImages.find(filename);

	if (it != m_texmapImages.end())
	{
		return it->second.image;
	}
	else
	{
		return NULL;
	}
}

void TREMainModel::setTransferTexmapInfo(
	const TexmapInfo &texmapInfo,
	bool /*bfc*/,
	const TCFloat *matrix)
{
	TexmapInfo transferTexmapInfo = texmapInfo;
	transferTexmapInfo.transform(matrix);
	if (m_mainTexmapInfos.size() == 0 ||
		!m_mainTexmapInfos.back().texmapEquals(transferTexmapInfo))
	{
		m_mainTexmapInfos.push_back(transferTexmapInfo);
		//TREModel::TexmapInfo::GeomInfo *geomInfo =
		//	bfc ? &m_mainTexmapInfos.back().bfc :
		//	&m_mainTexmapInfos.back().standard;
		//int bfcIndex = bfc ? 1 : 0;

		//if (m_texmappedShapes[bfcIndex] != NULL)
		//{
		//	geomInfo->colored.triangleOffset =
		//		m_texmappedShapes[bfcIndex]->getIndexCount(TRESTriangle) / 3;
		//}
	}
}
