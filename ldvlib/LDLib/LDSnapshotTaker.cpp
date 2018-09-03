#include "LDSnapshotTaker.h"
#include "LDUserDefaultsKeys.h"
#include <TCFoundation/mystring.h>
#include <TCFoundation/TCImage.h>
#include <TCFoundation/TCAlertManager.h>
#include <TCFoundation/TCLocalStrings.h>
#include <TCFoundation/TCProgressAlert.h>
#include <TCFoundation/TCUserDefaults.h>
#include <TCFoundation/TCStringArray.h>
#include <LDLib/LDUserDefaultsKeys.h>
#include <LDLib/LDPreferences.h>
#include <LDLib/LDViewPoint.h>
#include <LDLib/LDConsoleAlertHandler.h>
#include <LDLoader/LDLModel.h>
#include <TRE/TREGLExtensions.h>
#include <gl2ps/gl2ps.h>

#ifdef WIN32
#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
#define new DEBUG_CLIENTBLOCK
#endif // _DEBUG
#endif // WIN32

using namespace TREGLExtensionsNS;

#define FBO_SIZE 1024

#ifndef GL_EXT_packed_depth_stencil
#define GL_DEPTH_STENCIL_EXT              0x84F9
#define GL_UNSIGNED_INT_24_8_EXT          0x84FA
#define GL_DEPTH24_STENCIL8_EXT           0x88F0
#define GL_TEXTURE_STENCIL_SIZE_EXT       0x88F1
#endif

#ifdef USE_CPP11

#define ET_LDR LDrawModelViewer::ExportType::ETLdr
#define ET_STL LDrawModelViewer::ExportType::ETStl
#define ET_3DS LDrawModelViewer::ExportType::ET3ds
#define ET_POV LDrawModelViewer::ExportType::ETPov

#else // USE_CPP11

#define ET_LDR LDrawModelViewer::ETLdr
#define ET_STL LDrawModelViewer::ETStl
#define ET_3DS LDrawModelViewer::ET3ds
#define ET_POV LDrawModelViewer::ETPov

#endif // !USE_CPP11

class FBOHelper
{
public:
	FBOHelper(bool useFBO, bool b16BPC, LDSnapshotTaker* snapshotTaker = NULL) :
		m_useFBO(useFBO),
		m_16BPC(b16BPC),
		m_stencilBuffer(0)
	{
		if (m_useFBO)
		{
			GLint depthBits, stencilBits;
			GLenum colorFormat = GL_RGBA8;

			glGetIntegerv(GL_DEPTH_BITS, &depthBits);
			glGetIntegerv(GL_STENCIL_BITS, &stencilBits);
#ifdef __APPLE__
			// Apple supports packed depth stencil, so force 8 bits of stencil,
			// since LDView requires that for working transparent background
			// support.
			stencilBits = 8;
#endif
			glGenFramebuffersEXT(1, &m_fbo);
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);
			if (depthBits == 24 && stencilBits == 8 &&
				TREGLExtensions::checkForExtension(
				"GL_EXT_packed_depth_stencil"))
			{
				// nVidia cards and Mac users come here.  This part sucks.
				// Packed Depth/Stencil buffer
				glGenRenderbuffersEXT(1, &m_depthBuffer);
				glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_depthBuffer);
				glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT,
					GL_DEPTH_STENCIL_EXT, FBO_SIZE, FBO_SIZE);
				glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
					GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT,
					m_depthBuffer);
				glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
					GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT,
					m_depthBuffer);
				if (m_16BPC)
				{
					// Note: GL_RGBA16 doesn't work right on nVidia cards, but
					// GL_RGBA12 does.  So use 12, so that at least we get 12
					// bits worth, which is still better than 8.
					colorFormat = GL_RGBA12;
				}
			}
			else
			{
				// Depth buffer
				glGenRenderbuffersEXT(1, &m_depthBuffer);
				glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_depthBuffer);
				glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT,
					GL_DEPTH_COMPONENT24, FBO_SIZE, FBO_SIZE);
				glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
					GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT,
					m_depthBuffer);

				if (stencilBits != 0)
				{
					// Stencil buffer
					glGenRenderbuffersEXT(1, &m_stencilBuffer);
					glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_stencilBuffer);
					glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT,
						GL_STENCIL_INDEX, FBO_SIZE, FBO_SIZE);
					glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
						GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT,
						m_stencilBuffer);
				}
				if (m_16BPC)
				{
					// Note: this doesn't work on nVidia cards; hence it's
					// not being done above.
					colorFormat = GL_RGBA16;
				}
			}

			// Color buffer
			glGenRenderbuffersEXT(1, &m_colorBuffer);
			glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_colorBuffer);
			glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, colorFormat,
				FBO_SIZE, FBO_SIZE);
			glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
				GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER_EXT, m_colorBuffer);

			glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
			GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
			if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
			{
				debugPrintf("FBO Failed!\n");
			}
		}
		if (snapshotTaker != NULL && !snapshotTaker->hasRenderSize())
		{
			snapshotTaker->setRenderSize(FBO_SIZE, FBO_SIZE);
		}
		sm_active = true;
	}
	~FBOHelper()
	{
		if (m_useFBO)
		{
			glDeleteFramebuffersEXT(1, &m_fbo);
			glDeleteRenderbuffersEXT(1, &m_depthBuffer);
			if (m_stencilBuffer != 0)
			{
				glDeleteRenderbuffersEXT(1, &m_stencilBuffer);
			}
			glDeleteRenderbuffersEXT(1, &m_colorBuffer);
			glReadBuffer(GL_BACK);
		}
		sm_active = false;
	}
	static bool isActive(void) { return sm_active; }
	bool m_useFBO;
	bool m_16BPC;
	GLuint m_fbo;
	GLuint m_depthBuffer;
	GLuint m_stencilBuffer;
	GLuint m_colorBuffer;
	static bool sm_active;
};

bool FBOHelper::sm_active = false;

bool LDSnapshotTaker::sm_consoleAlerts = true;

LDSnapshotTaker::LDSnapshotTaker(void):
m_modelViewer(NULL),
m_imageType(ITPng),
m_trySaveAlpha(TCUserDefaults::boolForKey(SAVE_ALPHA_KEY, false, false)),
m_saveZMap(TCUserDefaults::boolForKey(SAVE_Z_MAP_KEY, false, false)),
m_autoCrop(TCUserDefaults::boolForKey(AUTO_CROP_KEY, false, false)),
m_fromCommandLine(true),
m_commandLineSaveSteps(false),
m_commandLineStep(false),
m_step(-1),
m_grabSetupDone(false),
m_gl2psAllowed(TCUserDefaults::boolForKey(GL2PS_ALLOWED_KEY, false, false)),
m_useFBO(false),
m_16BPC(false),
m_canceled(false),
m_width(-1),
m_height(-1),
m_scaleFactor(1.0)
{
}

LDSnapshotTaker::LDSnapshotTaker(LDrawModelViewer *m_modelViewer):
m_modelViewer(m_modelViewer),
m_imageType(ITPng),
m_trySaveAlpha(false),
m_saveZMap(false),
m_autoCrop(false),
m_fromCommandLine(false),
m_commandLineSaveSteps(false),
m_commandLineStep(false),
m_step(-1),
m_grabSetupDone(false),
m_gl2psAllowed(TCUserDefaults::boolForKey(GL2PS_ALLOWED_KEY, false, false)),
m_useFBO(false),
m_16BPC(false),
m_canceled(false),
m_width(-1),
m_height(-1),
m_scaleFactor(1.0f)
{
}

LDSnapshotTaker::~LDSnapshotTaker(void)
{
}

void LDSnapshotTaker::commandLineChanged(void)
{
	m_trySaveAlpha = TCUserDefaults::boolForKey(SAVE_ALPHA_KEY, false, false);
	m_saveZMap = TCUserDefaults::boolForKey(SAVE_Z_MAP_KEY, false, false);
	m_autoCrop = TCUserDefaults::boolForKey(AUTO_CROP_KEY, false, false);
	m_gl2psAllowed =
		TCUserDefaults::boolForKey(GL2PS_ALLOWED_KEY, false, false);
}

int LDSnapshotTaker::getFBOSize(void) const
{
	return FBO_SIZE;
}

void LDSnapshotTaker::dealloc(void)
{
	if (m_fromCommandLine)
	{
		TCObject::release(m_modelViewer);
	}
	TCObject::dealloc();
}

void LDSnapshotTaker::setUseFBO(bool value)
{
	m_useFBO = value && TREGLExtensions::haveFramebufferObjectExtension();
}

LDrawModelViewer::ExportType LDSnapshotTaker::exportTypeForFilename(
	const char* filename)
{
	if (stringHasCaseInsensitiveSuffix(filename, ".pov"))
	{
		return ET_POV;
	}
#ifdef LDR_EXPORTER
	else if (stringHasCaseInsensitiveSuffix(filename, ".ldr"))
	{
		return ET_LDR;
	}
#endif
	else if (stringHasCaseInsensitiveSuffix(filename, ".stl"))
	{
		return ET_STL;
	}
#ifdef EXPORT_3DS
	else if (stringHasCaseInsensitiveSuffix(filename, ".3ds"))
	{
		return ET_3DS;
	}
#endif
	else
	{
		// POV is the default;
		return ET_POV;
	}
}

bool LDSnapshotTaker::exportFiles(bool *tried /*= nullptr*/)
{
	bool retValue = false;
	bool exportFiles = false;
	TCStringArray *unhandledArgs = getUnhandledCommandLineArgs(
		EXPORT_FILES_LIST_KEY, exportFiles);

	if (unhandledArgs)
	{
		int i;
		int count = unhandledArgs->getCount();
		char *exportsDir = NULL;
		const char *exportExt = NULL;
		bool commandLineType = false;
		bool zoomToFit = TCUserDefaults::boolForKey(SAVE_ZOOM_TO_FIT_KEY, true,
			false);
		std::string exportSuffix =
			TCUserDefaults::commandLineStringForKey(EXPORT_SUFFIX_KEY);

		if (!exportFiles)
		{
			exportFiles = TCUserDefaults::boolForKey(EXPORT_FILES_KEY,
				false, false);
		}
		m_width = (int)TCUserDefaults::longForKey(SAVE_WIDTH_KEY, 640, false);
		m_height = (int)TCUserDefaults::longForKey(SAVE_HEIGHT_KEY, 480, false);
		if (!exportSuffix.empty())
		{
			m_exportType = exportTypeForFilename(exportSuffix.c_str());
			commandLineType = true;
		}
		else
		{
			if (!TCUserDefaults::commandLineStringForKey(
				SAVE_EXPORT_TYPE_KEY).empty())
			{
				commandLineType = true;
			}
			m_exportType =
				(LDrawModelViewer::ExportType)TCUserDefaults::longForKey(
				SAVE_EXPORT_TYPE_KEY, LDrawModelViewer::ETPov, false);
		}
		if (exportFiles)
		{
			switch (m_exportType)
			{
#ifdef LDR_EXPORTER
			case ET_LDR:
				exportExt = ".ldr";
				break;
#endif
			case ET_STL:
				exportExt = ".stl";
				break;
#ifdef EXPORT_3DS
			case ET_3DS:
				exportExt = ".3ds";
				break;
#endif
			case ET_POV:
			default:
				exportExt = ".pov";
				break;
			}
			exportsDir = TCUserDefaults::stringForKey(EXPORTS_DIR_KEY, NULL,
				false);
			if (exportsDir)
			{
				stripTrailingPathSeparators(exportsDir);
			}
		}
		for (i = 0; i < count; i++)
		{
			char *arg = unhandledArgs->stringAtIndex(i);
			char newArg[1024];

			if (stringHasCaseInsensitivePrefix(arg, "-ca"))
			{
				float value;

				if (sscanf(arg + 3, "%f", &value) == 1)
				{
					sprintf(newArg, "-%s=%f", HFOV_KEY, value);
					TCUserDefaults::addCommandLineArg(newArg);
				}
			}
			else if (stringHasCaseInsensitivePrefix(arg, "-cg"))
			{
				sprintf(newArg, "-%s=%s", CAMERA_GLOBE_KEY, arg + 3);
				TCUserDefaults::addCommandLineArg(newArg);
				zoomToFit = true;
			}
		}
		for (i = 0; i < count && (exportFiles || !retValue); ++i)
		{
			char *arg = unhandledArgs->stringAtIndex(i);
			
			if (arg[0] != '-' && arg[0] != 0)
			{
				std::string exportFilename;
				
				if (exportFiles)
				{
					char *baseFilename = filenameFromPath(arg);
					std::string mpdName;
					size_t mpdSpot;

					if (exportsDir)
					{
						exportFilename = exportsDir;
						exportFilename += "/";
						exportFilename += baseFilename;
					}
					else
					{
						exportFilename = arg;
					}
#ifdef WIN32
					mpdSpot = exportFilename.find(':', 2);
#else // WIN32
					mpdSpot = exportFilename.find(':');
#endif // WIN32
					if (mpdSpot < exportFilename.size())
					{
						char *baseMpdSpot = strrchr(baseFilename, ':');
						std::string mpdExt;

						mpdName = '-';
						mpdName += exportFilename.substr(mpdSpot + 1);
						exportFilename = exportFilename.substr(0, mpdSpot);
						if (baseMpdSpot != NULL &&
							strlen(baseMpdSpot) == mpdName.size())
						{
							baseMpdSpot[0] = 0;
						}
						mpdSpot = mpdName.rfind('.');
						if (mpdSpot < mpdName.length())
						{
							mpdExt = mpdName.substr(mpdSpot);
							convertStringToLower(&mpdExt[0]);
							if (mpdExt == ".dat" || mpdExt == ".ldr" ||
								mpdExt == ".mpd")
							{
								mpdName = mpdName.substr(0, mpdSpot);
							}
						}
					}
					// Note: we need there to be a dot in the base filename,
					// not the path before that.
					if (strchr(baseFilename, '.'))
					{
						exportFilename = exportFilename.substr(0,
							exportFilename.rfind('.'));
					}
					delete[] baseFilename;
					exportFilename += mpdName;
					exportFilename += exportExt;
				}
				else
				{
					char *tempFilename = TCUserDefaults::stringForKey(
						EXPORT_FILE_KEY, NULL, false);

					if (tempFilename != NULL)
					{
						exportFilename = tempFilename;
						delete[] tempFilename;
					}
					if (exportFilename.size() > 0 && !commandLineType)
					{
						m_exportType = exportTypeForFilename(
							exportFilename.c_str());
					}
				}
				if (exportFilename.size() > 0)
				{
					updateModelFilename(arg);
					retValue = exportFile(exportFilename, arg, zoomToFit) ||
						retValue;
					if (tried != NULL)
					{
						*tried = true;
					}
				}
			}
		}
		delete[] exportsDir;
		unhandledArgs->release();
	}
	return retValue;
}

void LDSnapshotTaker::updateModelFilename(const char *modelFilename)
{
	if (m_modelViewer && m_modelViewer->getFilename())
	{
		if (strcmp(modelFilename, m_modelViewer->getFilename()) != 0)
		{
			m_modelViewer->setFilename(modelFilename);
			m_modelViewer->loadModel();
		}
		else
		{
			// Note: it seems like this stuff should go into the
			// commandLineChanged member function. However, that won't work,
			// because commandLineChanged gets called before the CameraGlobe
			// setting has been updated, and LDPreferences is what parses that
			// setting.
			LDPreferences *prefs = new LDPreferences(m_modelViewer);
			prefs->loadSettings();
			prefs->applySettings();
			prefs->release();
			TCFloat fov = m_modelViewer->getFov();
			// Force view setup.
			m_modelViewer->setFov(fov + 1.0);
			m_modelViewer->setFov(fov );
		}
	}
	else if (m_modelViewer)
	{
		m_modelViewer->setFilename(modelFilename);
	}
	else
	{
		m_modelFilename = modelFilename;
	}
}

bool LDSnapshotTaker::exportFile(
	const std::string& exportFilename,
	const char *modelPath,
	bool zoomToFit)
{
	TCAlertManager::sendAlert(alertClass(), this, _UC("PreFbo"));
#ifdef __APPLE__
	FBOHelper fboHelper(m_useFBO, m_16BPC, this);
#else // __APPLE__
	FBOHelper fboHelper(m_useFBO, m_16BPC);
#endif // !__APPLE__
	grabSetup();
	if (m_canceled)
	{
		return false;
	}
	// Unfortunately, some of the camera setup is deferred until the first time
	// the model is drawn, so draw it offscreen before doing the export.
	renderOffscreenImage();
	if (zoomToFit)
	{
		m_modelViewer->zoomToFit();
	}
	try
	{
		if (m_modelViewer->exportCurModel(
			exportFilename.c_str(), NULL, NULL, m_exportType)
			== 0)
		{
			return true;
		}
	}
	catch (...)
	{
		// ignore
	}
	return false;
}

TCStringArray *LDSnapshotTaker::getUnhandledCommandLineArgs(
	const char *listKey,
	bool &foundList)
{
	TCStringArray *unhandledArgs =
		TCUserDefaults::getUnhandledCommandLineArgs();
	std::string listFilename =
		TCUserDefaults::commandLineStringForKey(listKey);

	if (!listFilename.empty())
	{
		std::ifstream stream;
		if (LDLModel::openStream(listFilename.c_str(), stream))
		{
			skipUtf8BomIfPresent(stream);
			std::string line;
			while (std::getline(stream, line))
			{
				stripCRLF(&line[0]);
				if (!line.empty() && line[0] != '-')
				{
					if (unhandledArgs == NULL)
					{
						unhandledArgs = new TCStringArray;
					}
					unhandledArgs->addString(line.c_str());
					foundList = true;
				}
			}
		}
	}
	return unhandledArgs;
}

bool LDSnapshotTaker::saveImage(bool *tried /*= nullptr*/)
{
	bool retValue = false;
	bool saveSnapshots = false;
	TCStringArray *unhandledArgs = getUnhandledCommandLineArgs(
		SAVE_SNAPSHOTS_LIST_KEY, saveSnapshots);

	if (unhandledArgs)
	{
		int i;
		int count = unhandledArgs->getCount();
		char *saveDir = NULL;
		const char *imageExt = NULL;
		int width = (int)TCUserDefaults::longForKey(SAVE_WIDTH_KEY, 640, false);
		int height = (int)TCUserDefaults::longForKey(SAVE_HEIGHT_KEY, 480, false);
		bool zoomToFit = TCUserDefaults::boolForKey(SAVE_ZOOM_TO_FIT_KEY, true,
			false);
		bool commandLineType = false;
		std::string snapshotSuffix =
			TCUserDefaults::commandLineStringForKey(SNAPSHOT_SUFFIX_KEY);
		std::string commandLineScaleFactor =
			TCUserDefaults::commandLineStringForKey(SAVE_SCALE_FACTOR_KEY);

		if (!commandLineScaleFactor.empty())
		{
			double scaleFactor;
			if (sscanf(commandLineScaleFactor.c_str(), "%lf", &scaleFactor) == 1)
			{
				m_scaleFactor = (TCFloat)scaleFactor;
			}
		}
		if (!saveSnapshots)
		{
			saveSnapshots = TCUserDefaults::boolForKey(SAVE_SNAPSHOTS_KEY,
				false, false);
		}
		if (TCUserDefaults::commandLineStringForKey(SAVE_IMAGE_TYPE_KEY).size()
			> 0)
		{
			m_imageType =
				(ImageType)TCUserDefaults::longForKey(SAVE_IMAGE_TYPE_KEY);
			commandLineType = true;
		}
		if (snapshotSuffix.size()
			> 0)
		{
			imageExt = snapshotSuffix.c_str();
			m_imageType = typeForFilename(imageExt, m_gl2psAllowed);
			commandLineType = true;
		}
		if (TCUserDefaults::commandLineStringForKey(SAVE_STEPS_KEY).size() > 0)
		{
			m_commandLineSaveSteps = true;
		}
		if (TCUserDefaults::commandLineStringForKey(STEP_KEY).size() > 0)
		{
			m_commandLineStep = true;
		}
		if (saveSnapshots)
		{
			if (imageExt == NULL)
			{
				switch (TCUserDefaults::longForKey(SAVE_IMAGE_TYPE_KEY, 1,
					false))
				{
				case ITBmp:
					imageExt = ".bmp";
					break;
				case ITJpg:
					imageExt = ".jpg";
					break;
				case ITSvg:
					if (m_gl2psAllowed)
					{
						imageExt = ".svg";
						// NOTE: break is INTENTIONALLY inside the if statement.
						break;
					}
				case ITEps:
					if (m_gl2psAllowed)
					{
						imageExt = ".eps";
						// NOTE: break is INTENTIONALLY inside the if statement.
						break;
					}
				case ITPdf:
					if (m_gl2psAllowed)
					{
						imageExt = ".pdf";
						// NOTE: break is INTENTIONALLY inside the if statement.
						break;
					}
				default:
					imageExt = ".png";
					break;
				}
			}
			saveDir = TCUserDefaults::stringForKey(SAVE_DIR_KEY, NULL, false);
			if (saveDir)
			{
				stripTrailingPathSeparators(saveDir);
			}
		}
		for (i = 0; i < count; i++)
		{
			char *arg = unhandledArgs->stringAtIndex(i);
			char newArg[1024];

			if (stringHasCaseInsensitivePrefix(arg, "-ca"))
			{
				float value;

				if (sscanf(arg + 3, "%f", &value) == 1)
				{
					sprintf(newArg, "-%s=%f", HFOV_KEY, value);
					TCUserDefaults::addCommandLineArg(newArg);
				}
			}
			else if (stringHasCaseInsensitivePrefix(arg, "-cg"))
			{
				sprintf(newArg, "-%s=%s", CAMERA_GLOBE_KEY, arg + 3);
				TCUserDefaults::addCommandLineArg(newArg);
				zoomToFit = true;
			}
		}
		for (i = 0; i < count && (saveSnapshots || !retValue); i++)
		{
			char *arg = unhandledArgs->stringAtIndex(i);
			
			if (arg[0] != '-' && arg[0] != 0)
			{
				std::string imageFilename;

				if (saveSnapshots)
				{
					char *baseFilename = filenameFromPath(arg);
					std::string mpdName;
					size_t mpdSpot;

					if (saveDir)
					{
						imageFilename = saveDir;
						imageFilename += "/";
						imageFilename += baseFilename;
					}
					else
					{
						imageFilename = arg;
					}
#ifdef WIN32
					mpdSpot = imageFilename.find(':', 2);
#else // WIN32
					mpdSpot = imageFilename.find(':');
#endif // WIN32
					if (mpdSpot < imageFilename.size())
					{
						char *baseMpdSpot = strrchr(baseFilename, ':');
						std::string mpdExt;

						mpdName = '-';
						mpdName += imageFilename.substr(mpdSpot + 1);
						imageFilename = imageFilename.substr(0, mpdSpot);
						if (baseMpdSpot != NULL &&
							strlen(baseMpdSpot) == mpdName.size())
						{
							baseMpdSpot[0] = 0;
						}
						mpdSpot = mpdName.rfind('.');
						if (mpdSpot < mpdName.length())
						{
							mpdExt = mpdName.substr(mpdSpot);
							convertStringToLower(&mpdExt[0]);
							if (mpdExt == ".dat" || mpdExt == ".ldr" ||
								mpdExt == ".mpd")
							{
								mpdName = mpdName.substr(0, mpdSpot);
							}
						}
					}
					// Note: we need there to be a dot in the base filename,
					// not the path before that.
					if (strchr(baseFilename, '.'))
					{
						imageFilename = imageFilename.substr(0,
							imageFilename.rfind('.'));
					}
					delete[] baseFilename;
					imageFilename += mpdName;
					imageFilename += imageExt;
				}
				else
				{
					char *tempFilename = TCUserDefaults::stringForKey(
						SAVE_SNAPSHOT_KEY, NULL, false);

					if (tempFilename != NULL)
					{
						imageFilename = tempFilename;
						delete[] tempFilename;
					}
					if (imageFilename.size() > 0 && !commandLineType)
					{
						m_imageType = typeForFilename(imageFilename.c_str(),
							m_gl2psAllowed);
					}
				}
				if (imageFilename.size() > 0)
				{
					updateModelFilename(arg);
					retValue = saveImage(imageFilename.c_str(), width, height,
						zoomToFit) || retValue;
					if (tried != NULL)
					{
						*tried = true;
					}
				}
			}
		}
		delete[] saveDir;
		unhandledArgs->release();
	}
	return retValue;
}

bool LDSnapshotTaker::shouldZoomToFit(bool zoomToFit)
{
	char *cameraGlobe = TCUserDefaults::stringForKey(CAMERA_GLOBE_KEY, NULL,
		false);
	bool retValue = false;

	if (zoomToFit)
	{
		retValue = true;
	}
	else if (cameraGlobe)
	{
		float globeRadius;

		if (sscanf(cameraGlobe, "%*f,%*f,%f", &globeRadius) == 1)
		{
			retValue = true;
		}
	}
	delete[] cameraGlobe;
	return retValue;
}

bool LDSnapshotTaker::saveImage(
	const char *filename,
	int imageWidth,
	int imageHeight,
	bool zoomToFit)
{
	bool steps = false;
	imageWidth = scale(imageWidth);
	imageHeight = scale(imageHeight);
	TCAlertManager::sendAlert(alertClass(), this, _UC("PreFbo"));
#ifdef __APPLE__
	FBOHelper fboHelper(m_useFBO, m_16BPC, this);
#else // __APPLE__
	FBOHelper fboHelper(m_useFBO, m_16BPC);
#endif // !__APPLE__

	if (!m_fromCommandLine || m_commandLineSaveSteps)
	{
		steps = TCUserDefaults::boolForKey(SAVE_STEPS_KEY, false, false);
	}
	if (steps || m_commandLineStep)
	{
		char *stepSuffix = TCUserDefaults::stringForKey(SAVE_STEPS_SUFFIX_KEY,
			"-Step", false);
		bool retValue = true;
		int numSteps;
		int origStep;
		LDViewPoint *viewPoint = NULL;

		if (!m_modelViewer)
		{
			grabSetup();
		}
		if (m_modelViewer == NULL)
		{
			return false;
		}
		origStep = m_modelViewer->getStep();
		if (m_modelViewer->getMainModel() == NULL)
		{
			// This isn't very efficient, but it gets the job done.  A
			// number of things need to happen before we can do the initial
			// zoomToFit.  We need to load the model, create the rotation
			// matrix, and set up the camera.  Maybe other things need to be
			// done too.  This update makes sure that things are OK for the
			// zoomToFit to execute properly.
			renderOffscreenImage();
		}
		if (TCUserDefaults::boolForKey(SAVE_STEPS_SAME_SCALE_KEY, true, false)
			&& zoomToFit)
		{
			if (!m_fromCommandLine)
			{
				viewPoint = m_modelViewer->saveViewPoint();
			}
			numSteps = m_modelViewer->getNumSteps();
			m_modelViewer->setStep(numSteps);
			m_modelViewer->zoomToFit();
			zoomToFit = false;
		}
		if (steps)
		{
			numSteps = m_modelViewer->getNumSteps();
		}
		else
		{
			numSteps = 1;
		}
		for (int step = 1; step <= numSteps && retValue; step++)
		{
			std::string stepFilename;

			if (steps)
			{
				stepFilename = removeStepSuffix(filename, stepSuffix);
				stepFilename = addStepSuffix(stepFilename, stepSuffix, step,
					numSteps);
				m_step = step;
			}
			else
			{
				stepFilename = filename;
				m_step = (int)TCUserDefaults::longForKey(STEP_KEY);
			}
			retValue = saveStepImage(stepFilename.c_str(), imageWidth,
				imageHeight, zoomToFit);
		}
		delete[] stepSuffix;
		m_modelViewer->setStep(origStep);
		if (viewPoint)
		{
			m_modelViewer->restoreViewPoint(viewPoint);
			viewPoint->release();
		}
		return retValue;
	}
	else
	{
		m_step = -1;
		return saveStepImage(filename, imageWidth, imageHeight, zoomToFit);
	}
}

bool LDSnapshotTaker::saveGl2psStepImage(
	const char *filename,
	int /*imageWidth*/,
	int /*imageHeight*/,
	bool zoomToFit)
{
	int bufSize;
	int state = GL2PS_OVERFLOW;
	FILE *file = ucfopen(filename, "wb");
	bool retValue = false;

	if (file != NULL)
	{
		bool origForceZoomToFit;
		LDViewPoint *viewPoint = NULL;

		grabSetup();
		origForceZoomToFit = m_modelViewer->getForceZoomToFit();
		if (zoomToFit)
		{
			viewPoint = m_modelViewer->saveViewPoint();
			m_modelViewer->setForceZoomToFit(true);
		}
#ifdef COCOA
		// For some reason, we get nothing in the feedback buffer on the Mac if
		// we don't reparse the model.  No idea why that is.
		m_modelViewer->reparse();
#endif // COCOA
		m_modelViewer->setGl2ps(true);
		m_modelViewer->setup();
		m_modelViewer->setHighlightPaths("");
		TCAlertManager::sendAlert(alertClass(), this, _UC("PreRender"));
		TCAlertManager::sendAlert(alertClass(), this, _UC("MakeCurrent"));
		for (bufSize = 1024 * 1024; state == GL2PS_OVERFLOW; bufSize *= 2)
		{
			GLint format;
			GLint options = GL2PS_USE_CURRENT_VIEWPORT
				| GL2PS_OCCLUSION_CULL
				| GL2PS_BEST_ROOT
				| GL2PS_NO_PS3_SHADING;

			if (m_autoCrop)
			{
				options |= GL2PS_TIGHT_BOUNDING_BOX;
			}
			switch (m_imageType)
			{
			case ITEps:
				format = GL2PS_EPS;
				break;
			case ITPdf:
				format = GL2PS_PDF;
				options |= GL2PS_COMPRESS;
				break;
			default:
				format = GL2PS_SVG;
				break;
			}
			state = gl2psBeginPage(filename, "LDView", NULL, format,
				GL2PS_BSP_SORT,	options, GL_RGBA, 0, NULL, 0, 0, 0, bufSize,
				file, filename);
			if (state == GL2PS_ERROR)
			{
				debugPrintf("ERROR in gl2ps routine!");
			}
			else
			{
				renderOffscreenImage();
				glFinish();
				state = gl2psEndPage();
				if (state == GL2PS_ERROR)
				{
					debugPrintf("ERROR in gl2ps routine!");
				}
				else
				{
					retValue = true;
				}
			}
		}
		m_modelViewer->setGl2ps(false);
		if (zoomToFit)
		{
			m_modelViewer->setForceZoomToFit(origForceZoomToFit);
			m_modelViewer->restoreViewPoint(viewPoint);
		}
		fclose(file);
		if (viewPoint != NULL)
		{
			m_modelViewer->restoreViewPoint(viewPoint);
			viewPoint->release();
		}
	}
	return retValue;
}

bool LDSnapshotTaker::saveStepImage(
	const char *filename,
	int imageWidth,
	int imageHeight,
	bool zoomToFit)
{
	bool retValue = false;

	if (m_imageType >= ITSvg && m_imageType <= ITPdf)
	{
		retValue = saveGl2psStepImage(filename, imageWidth, imageHeight,
			zoomToFit);
	}
	else
	{
		bool saveAlpha = false;
		GLfloat *zBuffer = NULL;
		if (m_saveZMap)
		{
			zBuffer = new GLfloat[imageWidth * imageHeight];
		}
		TCByte *buffer = grabImage(imageWidth, imageHeight,
			shouldZoomToFit(zoomToFit), NULL, &saveAlpha, zBuffer);

		if (buffer)
		{
			switch (m_imageType)
			{
			case ITPng:
				retValue = writePng(filename, imageWidth, imageHeight, buffer,
					saveAlpha);
				break;
			case ITBmp:
				retValue = writeBmp(filename, imageWidth, imageHeight, buffer);
				break;
			case ITJpg:
				retValue = writeJpg(filename, imageWidth, imageHeight, buffer);
				break;
			default:
				// Get rid of warning
				break;
			}
			delete[] buffer;
		}
		if (zBuffer != NULL)
		{
			std::string zMapFilename = filename;
			removeExtenstion(zMapFilename);
			zMapFilename += ".ldvz";
			writeZMap(zMapFilename.c_str(), imageWidth, imageHeight, zBuffer);
			delete[] zBuffer;
		}
	}
	return retValue;
}

bool LDSnapshotTaker::staticImageProgressCallback(
	CUCSTR message,
	float progress,
	void* userData)
{
	return ((LDSnapshotTaker*)userData)->imageProgressCallback(message,
		progress);
}

bool LDSnapshotTaker::imageProgressCallback(CUCSTR message, float progress)
{
	bool aborted;
	ucstring newMessage;

	if (message != NULL)
	{
		char *filename = filenameFromPath(m_currentImageFilename.c_str());
		UCSTR ucFilename = mbstoucstring(filename);

		delete[] filename;
		if (stringHasCaseInsensitivePrefix(message, _UC("Saving")))
		{
			newMessage = TCLocalStrings::get(_UC("SavingPrefix"));
		}
		else
		{
			newMessage = TCLocalStrings::get(_UC("LoadingPrefix"));
		}
		newMessage += ucFilename;
		delete[] ucFilename;
	}

	TCProgressAlert::send("LDSnapshotTaker", newMessage.c_str(), progress,
		&aborted, this);
	return !aborted;
}

//#define TEST_ZMAP
#ifdef TEST_ZMAP

static bool testZMap2(FILE *zMapFile, TCImage *image)
{
	char magic[5] = { 0 };
	char endian[5] = { 0 };
	if (fread(magic, 4, 1, zMapFile) != 1)
	{
		debugPrintf("Error Reading Z Map Magic Number.\n");
		return false;
	}
	if (strcmp(magic, "ldvz") != 0)
	{
		debugPrintf("Invalid Z Map Magic Number.\n");
		return false;
	}
	if (fread(endian, 4, 1, zMapFile) != 1)
	{
		debugPrintf("Error Reading Z Map Endian.\n");
		return false;
	}
	const char *expectedEndian = "BIGE";
	if (isLittleEndian())
	{
		expectedEndian = "LITE";
	}
	if (strcmp(endian, expectedEndian) != 0)
	{
		debugPrintf("Invalid Z Map Endian.\n");
		return false;
	}
	int32_t size[2];
	if (fread(&size, sizeof(size[0]), 2, zMapFile) != 2)
	{
		debugPrintf("Error Reading Z Map Dimensions.\n");
		return false;
	}
	size_t zDataCount = size[0] * size[1];
	if (zDataCount >= (2 << 28))
	{
		debugPrintf("Z Map probably too big.\n");
		return false;
	}
	std::vector<GLfloat> zData;
	zData.resize(zDataCount);
	if (fread(&zData[0], sizeof(GLfloat), zData.size(), zMapFile) != zData.size())
	{
		debugPrintf("Error Reading Z Map Data.\n");
		return false;
	}
	image->setSize(size[0], size[1]);
	image->setDataFormat(TCRgb8);
	image->setLineAlignment(4);
	image->allocateImageData();
	image->setFormatName("PNG");
	int rowSize = image->roundUp(size[0] * 3, 4);
	for (int32_t y = 0; y < size[1]; ++y)
	{
		TCByte *pixelSpot = &image->getImageData()[y * rowSize];
		GLfloat *zSpot = &zData[y * size[0]];
		for (int32_t x = 0; x < size[0]; ++x)
		{
			TCByte pixelValue = (TCByte)(*zSpot++ * 255.0);
			*pixelSpot++ = pixelValue;
			*pixelSpot++ = pixelValue;
			*pixelSpot++ = pixelValue;
		}
	}
	return true;
}

// Generate a 24-bit RGB PNG file, where the grayscale value of each pixel
// represents how far away that pixel is, with white being the farthest away,
// and black being the closest.
static void testZMap(const char *filename)
{
	FILE *zMapFile = ucfopen(filename, "rb");
	TCImage *image = new TCImage;
	
	if (testZMap2(zMapFile, image))
	{
		std::string pngFilename = filename;
		pngFilename += ".png";
		image->saveFile(pngFilename.c_str());
	}
	else
	{
		debugPrintf("ZMap Test Failed.\n");
	}
	image->release();
	fclose(zMapFile);
}

#endif // TEST_ZMAP

bool LDSnapshotTaker::writeZMap(
	FILE *zMapFile,
	int width,
	int height,
	TCFloat *zBuffer)
{
	const char *magic = "ldvz";
	if (fwrite(magic, 4, 1, zMapFile) != 1)
	{
		return false;
	}
	const char *endian = "BIGE";
	if (isLittleEndian())
	{
		endian = "LITE";
	}
	if (fwrite(endian, 4, 1, zMapFile) != 1)
	{
		return false;
	}
	// Crop zBuffer in place.
	if (m_croppedX != 0 || m_croppedY != 0 || m_croppedWidth != width ||
		m_croppedHeight != height)
	{
		int newBytesPerLine = m_croppedWidth * sizeof(TCFloat);
		for (int y = 0; y < m_croppedHeight; ++y)
		{
			// Use memmove, since the source and destination might overlap.
			memmove(&zBuffer[y * m_croppedWidth],
					&zBuffer[(y + m_croppedY) * width + m_croppedX],
					newBytesPerLine);
		}
		width = m_croppedWidth;
		height = m_croppedHeight;
	}
	int32_t fileWidth = (int32_t)width;
	if (fwrite(&fileWidth, sizeof(fileWidth), 1, zMapFile) != 1)
	{
		return false;
	}
	int32_t fileHeight = (int32_t)height;
	if (fwrite(&fileHeight, sizeof(fileHeight), 1, zMapFile) != 1)
	{
		return false;
	}
	for (size_t y = 0; y < height; ++y)
	{
		size_t yOffset = (height - y - 1) * width;
		if (fwrite(&zBuffer[yOffset], sizeof(GLfloat), width, zMapFile) !=
			width)
		{
			return false;
		}
	}
	return true;
}

bool LDSnapshotTaker::writeZMap(
	const char *filename,
	int width,
	int height,
	TCFloat *zBuffer)
{
	FILE *zMapFile = ucfopen(filename, "wb");
	if (zMapFile != NULL)
	{
		bool retValue = writeZMap(zMapFile, width, height, zBuffer);
		fclose(zMapFile);
#ifdef TEST_ZMAP
		if (retValue)
		{
			testZMap(filename);
		}
#endif // TEST_ZMAP
		return retValue;
	}
	return false;
}

bool LDSnapshotTaker::writeImage(
	const char *filename,
	int width,
	int height,
	TCByte *buffer,
	const char *formatName,
	bool saveAlpha)
{
	TCImage *image = new TCImage;
	bool retValue;
	char comment[1024];

	image->setDpi((int)72 * m_scaleFactor);
	m_currentImageFilename = filename;
	if (saveAlpha)
	{
		if (m_16BPC)
		{
			image->setDataFormat(TCRgba16);
		}
		else
		{
			image->setDataFormat(TCRgba8);
		}
	}
	else if (m_16BPC)
	{
		image->setDataFormat(TCRgb16);
	}
	image->setSize(width, height);
	image->setLineAlignment(4);
	image->setImageData(buffer);
	image->setFormatName(formatName);
	image->setFlipped(true);
	if (strcasecmp(formatName, "PNG") == 0)
	{
		strcpy(comment, "Software:!:!:LDView");
	}
	else
	{
		strcpy(comment, "Created by LDView");
	}
	if (m_productVersion.size() > 0)
	{
		strcat(comment, " ");
		strcat(comment, m_productVersion.c_str());
	}
	image->setComment(comment);
	if (m_autoCrop)
	{
		image->autoCrop((TCByte)m_modelViewer->getBackgroundR(),
			(TCByte)m_modelViewer->getBackgroundG(),
			(TCByte)m_modelViewer->getBackgroundB());
		m_croppedX = image->getCroppedX();
		m_croppedY = image->getCroppedY();
	}
	else
	{
		m_croppedX = m_croppedY = 0;
	}
	m_croppedWidth = image->getWidth();
	m_croppedHeight = image->getHeight();
	retValue = image->saveFile(filename, staticImageProgressCallback, this);

    if (TCUserDefaults::boolForKey("Info"))
    {
        printf("\nLDView Image Output\n");
        printf("==========================\n");
        printf("Write %s image %s\n\n", formatName, filename);
    }

	debugPrintf("Saved image: %s\n", filename);
	image->release();
	return retValue;
}

bool LDSnapshotTaker::writeJpg(
	const char *filename,
	int width,
	int height,
	TCByte *buffer)
{
	return writeImage(filename, width, height, buffer, "JPG", false);
}

bool LDSnapshotTaker::writeBmp(
	const char *filename,
	int width,
	int height,
	TCByte *buffer)
{
	return writeImage(filename, width, height, buffer, "BMP", false);
}

bool LDSnapshotTaker::writePng(
	const char *filename,
	int width,
	int height,
	TCByte *buffer,
	bool saveAlpha)
{
	return writeImage(filename, width, height, buffer, "PNG", saveAlpha);
}

void LDSnapshotTaker::setRenderSize(int width, int height)
{
	m_width = width;
	m_height = height;
}

void LDSnapshotTaker::calcTiling(
	int desiredWidth,
	int desiredHeight,
	int &bitmapWidth,
	int &bitmapHeight,
	int &numXTiles,
	int &numYTiles)
{
	if (desiredWidth > bitmapWidth)
	{
		numXTiles = (desiredWidth + bitmapWidth - 1) / bitmapWidth;
	}
	else
	{
		numXTiles = 1;
	}
	bitmapWidth = desiredWidth / numXTiles;
	if (desiredHeight > bitmapHeight)
	{
		numYTiles = (desiredHeight + bitmapHeight - 1) / bitmapHeight;
	}
	else
	{
		numYTiles = 1;
	}
	bitmapHeight = desiredHeight / numYTiles;
	if (m_scaleFactor != 1.0)
	{
		while (unscale(bitmapWidth) != bitmapWidth / m_scaleFactor)
		{
			--bitmapWidth;
		}
		while (unscale(bitmapHeight) != bitmapHeight / m_scaleFactor)
		{
			--bitmapHeight;
		}
	}
}

bool LDSnapshotTaker::canSaveAlpha(void)
{
	if (m_trySaveAlpha && m_imageType == ITPng)
	{
		GLint alphaBits;

		glGetIntegerv(GL_ALPHA_BITS, &alphaBits);
		return alphaBits > 0;
	}
	return false;
}

void LDSnapshotTaker::renderOffscreenImage(void)
{
	TCAlertManager::sendAlert(alertClass(), this, _UC("MakeCurrent"));
	if (m_modelViewer->getMainModel() == NULL)
	{
		m_modelViewer->loadModel();
	}
	m_modelViewer->update();
}

void LDSnapshotTaker::grabSetup(void)
{
	if (m_grabSetupDone)
	{
		return;
	}
	TCAlertManager::sendAlert(alertClass(), this, _UC("PreSave"));
	m_grabSetupDone = true;
	if (!m_canceled)
	{
		initModelViewer();
	}
}

void LDSnapshotTaker::getViewportSize(int &width, int &height)
{
	GLint viewport[4] = {0};
	glGetError(); // Flush any previous errors.
	glGetIntegerv(GL_VIEWPORT, viewport);
	GLenum glError = glGetError();
	if (glError == GL_NO_ERROR)
	{
		width = (int)viewport[2];
		height = (int)viewport[3];
	}
	else
	{
		width = height = 0;
	}
}

void LDSnapshotTaker::initModelViewer(void)
{
	if (!m_modelViewer)
	{
		LDPreferences *prefs;
		if (!hasRenderSize())
		{
			getViewportSize(m_width, m_height);
		}
		m_modelViewer = new LDrawModelViewer(m_width, m_height);
		m_modelViewer->setFilename(m_modelFilename.c_str());
		m_modelViewer->setNoUI(true);
		m_modelFilename = "";
		prefs = new LDPreferences(m_modelViewer);
		prefs->loadSettings();
		prefs->applySettings();
		prefs->release();
		m_modelViewer->setViewMode(LDrawModelViewer::VMExamine);
	}
}

TCByte *LDSnapshotTaker::grabImage(
	int &imageWidth,
	int &imageHeight,
	bool zoomToFit,
	TCByte *buffer,
	bool *saveAlpha,
	GLfloat *zBuffer /*= NULL*/)
{
	FBOHelper *localHelper = NULL;
	TCAlertManager::sendAlert(alertClass(), this, _UC("PreFbo"));
	if (!FBOHelper::isActive())
	{
#ifdef __APPLE__
		localHelper = new FBOHelper(m_useFBO, m_16BPC, this);
#else // __APPLE__
		localHelper = new FBOHelper(m_useFBO, m_16BPC);
#endif // !__APPLE__
	}
	grabSetup();
	if (m_canceled)
	{
		delete localHelper;
		return NULL;
	}

	GLenum bufferFormat = GL_RGB;
	GLenum componentType = GL_UNSIGNED_BYTE;
	bool origForceZoomToFit = m_modelViewer->getForceZoomToFit();
	StringList origHighlightPaths = m_modelViewer->getHighlightPaths();
	TCVector origCameraPosition = m_modelViewer->getCamera().getPosition();
	TCFloat origXPan = m_modelViewer->getXPan();
	TCFloat origYPan = m_modelViewer->getYPan();
	TCFloat origWidth = m_modelViewer->getFloatWidth();
	TCFloat origHeight = m_modelViewer->getFloatHeight();
	TCFloat origScaleFactor = m_modelViewer->getScaleFactor();
	bool origAutoCenter = m_modelViewer->getAutoCenter();
	int newWidth, newHeight;
	int numXTiles, numYTiles;
	int xTile;
	int yTile;
	TCByte *smallBuffer;
	GLfloat *smallZBuffer = NULL;
	int bytesPerPixel;
	int bytesPerLine;
	int bytesPerChannel = 1;
	int smallBytesPerLine;
	int reallySmallBytesPerLine;
	bool canceled = false;
	bool bufferAllocated = false;

	if (m_16BPC)
	{
		bytesPerChannel = 2;
		componentType = GL_UNSIGNED_SHORT;
	}
	if (m_step > 0)
	{
		m_modelViewer->setStep(m_step);
	}
	m_modelViewer->setScaleFactor(m_scaleFactor);
	if (m_useFBO)
	{
		newWidth = FBO_SIZE;
		newHeight = FBO_SIZE;
	}
	else
	{
		getViewportSize(newWidth, newHeight);
	}
	if (newWidth == 0 || newHeight == 0)
	{
		delete localHelper;
		return NULL;
	}
	m_modelViewer->setWidth(unscale(newWidth));
	m_modelViewer->setHeight(unscale(newHeight));
	m_modelViewer->perspectiveView();
	calcTiling(imageWidth, imageHeight, newWidth, newHeight, numXTiles,
		numYTiles);
	m_modelViewer->setWidth(unscale(newWidth));
	m_modelViewer->setHeight(unscale(newHeight));
	if (zoomToFit)
	{
		m_modelViewer->setForceZoomToFit(true);
		m_modelViewer->perspectiveView();
	}
	m_modelViewer->setup();
	m_modelViewer->setHighlightPaths("");
	if (canSaveAlpha())
	{
		bytesPerPixel = 4 * bytesPerChannel;
		bufferFormat = GL_RGBA;
		m_modelViewer->setSaveAlpha(true);
		if (saveAlpha)
		{
			*saveAlpha = true;
		}
	}
	else
	{
		bytesPerPixel = 3 * bytesPerChannel;
		if (saveAlpha)
		{
			*saveAlpha = false;
		}
	}
	imageWidth = newWidth * numXTiles;
	imageHeight = newHeight * numYTiles;
	smallBytesPerLine = TCImage::roundUp(newWidth * bytesPerPixel, 4);
	reallySmallBytesPerLine = newWidth * bytesPerPixel;
	bytesPerLine = TCImage::roundUp(imageWidth * bytesPerPixel, 4);
	if (!buffer)
	{
		buffer = new TCByte[bytesPerLine * imageHeight];
		bufferAllocated = true;
	}
	if (numXTiles == 1 && numYTiles == 1)
	{
		smallBuffer = buffer;
		smallZBuffer = zBuffer;
	}
	else
	{
		smallBuffer = new TCByte[smallBytesPerLine * newHeight];
		if (zBuffer != NULL)
		{
			smallZBuffer = new GLfloat[newWidth * newHeight];
		}
	}
	m_modelViewer->setNumXTiles(numXTiles);
	m_modelViewer->setNumYTiles(numYTiles);
	TCAlertManager::sendAlert(alertClass(), this, _UC("PreRender"));
	for (yTile = 0; yTile < numYTiles; yTile++)
	{
		m_modelViewer->setYTile(yTile);
		for (xTile = 0; xTile < numXTiles && !canceled; xTile++)
		{
			m_modelViewer->setXTile(xTile);
			renderOffscreenImage();
			TCProgressAlert::send("LDSnapshotTaker",
				TCLocalStrings::get(_UC("RenderingSnapshot")),
				(float)(yTile * numXTiles + xTile) / (numYTiles * numXTiles),
				&canceled);
			if (!canceled)
			{
				glFinish();
				TCAlertManager::sendAlert(alertClass(), this,
					_UC("RenderDone"));
				glReadPixels(0, 0, newWidth, newHeight, bufferFormat,
					componentType, smallBuffer);
				if (smallZBuffer != NULL)
				{
					glReadPixels(0, 0, newWidth, newHeight, GL_DEPTH_COMPONENT,
						GL_FLOAT, smallZBuffer);
				}
				if (smallBuffer != buffer)
				{
					int y;

					for (y = 0; y < newHeight; y++)
					{
						int smallOffset = y * smallBytesPerLine;
						int offset = (y + (numYTiles - yTile - 1) * newHeight) *
							bytesPerLine + xTile * newWidth * bytesPerPixel;

						memcpy(&buffer[offset], &smallBuffer[smallOffset],
							reallySmallBytesPerLine);
						if (smallZBuffer != NULL)
						{
							smallOffset = y * newWidth;
							offset = (y + (numYTiles - yTile - 1) * newHeight) *
								newWidth + xTile * newWidth;
							memcpy(&zBuffer[offset], &smallZBuffer[smallOffset],
								newWidth * sizeof(GLfloat));
						}
					}
					// We only need to zoom to fit on the first tile; the
					// rest will already be correct.
					m_modelViewer->setForceZoomToFit(false);
				}
			}
			else
			{
				canceled = true;
			}
		}
	}
	m_modelViewer->setXTile(0);
	m_modelViewer->setYTile(0);
	m_modelViewer->setNumXTiles(1);
	m_modelViewer->setNumYTiles(1);
	m_modelViewer->setScaleFactor(origScaleFactor);
	m_modelViewer->setWidth(origWidth);
	m_modelViewer->setHeight(origHeight);
	m_modelViewer->setSaveAlpha(false);
	m_modelViewer->setHighlightPaths(origHighlightPaths);
	if (smallBuffer != buffer)
	{
		delete[] smallBuffer;
	}
	if (smallZBuffer != zBuffer)
	{
		delete[] smallZBuffer;
	}
	if (canceled && bufferAllocated)
	{
		delete[] buffer;
		buffer = NULL;
	}
	if (zoomToFit)
	{
		m_modelViewer->setForceZoomToFit(origForceZoomToFit);
		m_modelViewer->getCamera().setPosition(origCameraPosition);
		m_modelViewer->setXYPan(origXPan, origYPan);
		m_modelViewer->setAutoCenter(origAutoCenter);
	}
	delete localHelper;
	return buffer;
}

LDConsoleAlertHandler* LDSnapshotTaker::getConsoleAlertHandler(void)
{
	if (sm_consoleAlerts)
	{
		int verbosity = 1;
		TCStringArray *unhandledArgs =
			TCUserDefaults::getUnhandledCommandLineArgs();
		
		if (unhandledArgs != NULL)
		{
			int count = unhandledArgs->getCount();
			for (int i = 0; i < count; ++i)
			{
				char *arg = unhandledArgs->stringAtIndex(i);
				if (strcasecmp(arg, "-q") == 0)
				{
					--verbosity;
				}
				else if (strcasecmp(arg, "-qq") == 0)
				{
					verbosity -= 2;
				}
				else if (strcasecmp(arg, "-v") == 0)
				{
					++verbosity;
				}
			}
			unhandledArgs->release();
		}
		return new LDConsoleAlertHandler(verbosity);
	}
	return NULL;
}

bool LDSnapshotTaker::doCommandLine(
	bool doSnapshots,
	bool doExports,
	bool *tried /*= nullptr*/)
{
	LDSnapshotTaker *snapshotTaker = new LDSnapshotTaker;
	LDConsoleAlertHandler *consoleAlertHandler = getConsoleAlertHandler();
	if (tried != NULL)
	{
		*tried = false;
	}
	bool retValue = snapshotTaker->doCommandLine(doSnapshots, doExports, tried,
		consoleAlertHandler);
	snapshotTaker->release();
	TCObject::release(consoleAlertHandler);
	return retValue;
}

bool LDSnapshotTaker::doCommandLine(
	bool doSnapshots,
	bool doExports,
	bool *tried,
	LDConsoleAlertHandler *consoleAlertHandler)
{
	bool retValue = false;
	std::string listFilename =
		TCUserDefaults::commandLineStringForKey(COMMAND_LINES_LIST_KEY);
	if (!listFilename.empty())
	{
		if (!m_commandLinesLists.insert(listFilename).second)
		{
			consolePrintf(ls("CommandLinesListRecursion"),
				listFilename.c_str());
		}
		else
		{
			std::ifstream stream;
			if (LDLModel::openStream(listFilename.c_str(), stream))
			{
				skipUtf8BomIfPresent(stream);
				std::string line;
				std::string commonArgs;
				bool firstLine = true;
				std::string commonPrefix = "Common: ";
				const TCStringArray *origCommandLine =
					TCUserDefaults::getProcessedCommandLine();
				if (origCommandLine != NULL)
				{
					int count = origCommandLine->getCount();
					std::string commandLinesListArg = "-";
					commandLinesListArg += COMMAND_LINES_LIST_KEY;
					commandLinesListArg += "=";
					for (int i = 0; i < count; ++i)
					{
						const char *arg = (*origCommandLine)[i];
						if (!stringHasPrefix(arg, commandLinesListArg.c_str()))
						{
							commonArgs += arg;
							commonArgs += " ";
						}
					}
				}
				while (std::getline(stream, line))
				{
					if (firstLine && stringHasPrefix(line.c_str(),
						commonPrefix.c_str()))
					{
						commonArgs += line.substr(commonPrefix.size()) + " ";
					}
					else if (!line.empty() && line[0] != ';')
					{
						firstLine = false;
						stripCRLF(&line[0]);
						if (commonArgs.empty())
						{
							TCUserDefaults::setCommandLine(line.c_str());
						}
						else
						{
							std::string commandLine = commonArgs + line;
							TCUserDefaults::setCommandLine(commandLine.c_str());
							commandLineChanged();
						}
						if (doCommandLine(doSnapshots, doExports, tried,
							consoleAlertHandler))
						{
							retValue = true;
						}
					}
				}
				return retValue;
			}
		}
	}
	if (doSnapshots)
	{
		retValue = saveImage(tried);
	}
	if (doExports)
	{
		retValue = exportFiles(tried) || retValue;
	}
	return retValue;
}

// Note: static method
std::string LDSnapshotTaker::removeStepSuffix(
	const std::string &filename,
	const std::string &stepSuffix)
{
	if (stepSuffix.size() == 0)
	{
		return filename;
	}
	char *dirPart = directoryFromPath(filename.c_str());
	char *filePart = filenameFromPath(filename.c_str());
	std::string fileString = filePart;
	size_t suffixLoc;
	std::string tempSuffix = stepSuffix;
	std::string newString;

	newString = dirPart;
	delete[] dirPart;
#if defined(WIN32) || defined(__APPLE__)
	// case-insensitive file systems
	convertStringToLower(&fileString[0]);
	convertStringToLower(&tempSuffix[0]);
#endif // WIN32 || __APPLE__
	suffixLoc = fileString.rfind(tempSuffix);
	if (suffixLoc < fileString.size())
	{
		size_t i;

		for (i = suffixLoc + tempSuffix.size(); isdigit(fileString[i]); i++)
		{
			// Don't do anything
		}
#if defined(WIN32) || defined(__APPLE__)
		// case-insensitive file systems
		// Restore filename to original case
		fileString = filePart;
		delete[] filePart;
#endif // WIN32 || __APPLE__
		fileString.erase(suffixLoc, i - suffixLoc);
	}
	else
	{
		delete[] filePart;
		return filename;
	}
	if (newString.size() > 0)
	{
		newString += "/";
	}
	newString += fileString;
	filePart = cleanedUpPath(newString.c_str());
	newString = filePart;
	delete[] filePart;
	return newString;
}

// Note: static method
std::string LDSnapshotTaker::addStepSuffix(
	const std::string &filename,
	const std::string &stepSuffix,
	int step,
	int numSteps)
{
	size_t dotSpot = filename.rfind('.');
	std::string newString;
	char format[32];
	char buf[32];
	int digits = 1;

	while ((numSteps = numSteps / 10) != 0)
	{
		digits++;
	}
	sprintf(format, "%%0%dd", digits);
	sprintf(buf, format, step);
	newString = filename.substr(0, dotSpot);
	newString += stepSuffix;
	newString += buf;
	if (dotSpot < filename.size())
	{
		newString += filename.substr(dotSpot);
	}
	return newString;
}

// Note: static method
LDSnapshotTaker::ImageType LDSnapshotTaker::typeForFilename(
	const char *filename,
	bool gl2psAllowed)
{
	if (stringHasCaseInsensitiveSuffix(filename, ".png"))
	{
		return ITPng;
	}
	else if (stringHasCaseInsensitiveSuffix(filename, ".bmp"))
	{
		return ITBmp;
	}
	else if (stringHasCaseInsensitiveSuffix(filename, ".jpg"))
	{
		return ITJpg;
	}
	else if (stringHasCaseInsensitivePrefix(filename, ".svg") && gl2psAllowed)
	{
		return ITSvg;
	}
	else if (stringHasCaseInsensitivePrefix(filename, ".eps") && gl2psAllowed)
	{
		return ITEps;
	}
	else if (stringHasCaseInsensitivePrefix(filename, ".pdf") && gl2psAllowed)
	{
		return ITPdf;
	}
	else
	{
		// PNG is the default;
		return ITPng;
	}
}

// Note: static method
std::string LDSnapshotTaker::extensionForType(
	ImageType type,
	bool includeDot /*= false*/)
{
	if (includeDot)
	{
		std::string retValue(".");
		
		retValue += extensionForType(type, false);
		return retValue;
	}
	switch (type)
	{
	case ITPng:
		return "png";
	case ITBmp:
		return "bmp";
	case ITJpg:
		return "jpg";
	case ITSvg:
		return "svg";
	case ITEps:
		return "eps";
	case ITPdf:
		return "pdf";
	default:
		return "png";
	}
}

// Note: static method
LDSnapshotTaker::ImageType LDSnapshotTaker::lastImageType(void)
{
	if (TCUserDefaults::boolForKey(GL2PS_ALLOWED_KEY, false, false))
	{
		return ITLast;
	}
	else
	{
		return ITJpg;
	}
}

