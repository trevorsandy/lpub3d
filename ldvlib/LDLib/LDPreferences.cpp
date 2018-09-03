#include "LDPreferences.h"
#include "LDUserDefaultsKeys.h"
#include <TCFoundation/TCAlertManager.h>
#include <TCFoundation/TCMacros.h>
#include <TCFoundation/TCUserDefaults.h>
//#include <TCFoundation/TCWebClient.h>
#include <LDLoader/LDLPalette.h>
#include <LDLoader/LDLModel.h>

#ifdef WIN32
#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
#define new DEBUG_CLIENTBLOCK
#endif // _DEBUG
#endif // WIN32

LDPreferences::LDPreferences(LDrawModelViewer* modelViewer)
	:m_modelViewer(modelViewer ? ((LDrawModelViewer*)modelViewer->retain()) :
	NULL)
{
	int i;
	SaveOpStringMap::const_iterator it;

	m_globalSettings[ZOOM_MAX_KEY] = true;
	m_globalSettings[SHOW_ERRORS_KEY] = true;
	m_globalSettings[PROXY_TYPE_KEY] = true;
	m_globalSettings[PROXY_SERVER_KEY] = true;
	m_globalSettings[PROXY_PORT_KEY] = true;
	m_globalSettings[CHECK_PART_TRACKER_KEY] = true;
	m_globalSettings[CHECK_PART_WAIT_KEY] = true;
	m_globalSettings[CHECK_PART_UPDATE_WAIT_KEY] = true;
	m_globalSettings[CAMERA_GLOBE_KEY] = true;
	m_globalSettings[INV_SHOW_MODEL_KEY] = true;
	m_globalSettings[INV_EXTERNAL_CSS_KEY] = true;
	m_globalSettings[INV_PART_IMAGES_KEY] = true;
	m_globalSettings[INV_SHOW_FILE_KEY] = true;
	m_globalSettings[INV_SHOW_TOTAL_KEY] = true;
	m_globalSettings[INV_COLUMN_ORDER_KEY] = true;
	m_globalSettings[INV_LAST_SAVE_PATH_KEY] = true;
	m_globalSettings[MULTI_THREADED_KEY] = true;
	m_globalSettings[LDRAWDIR_KEY] = true;
	m_globalSettings[EXTRA_SEARCH_DIRS_KEY] = true;
	m_defaultColorNumber = -1;
	for (i = 0; i < 16; i++)
	{
		char key[128];

		sprintf(key, "%s/Color%02d", CUSTOM_COLORS_KEY, i);
		m_globalSettings[key] = true;
	}
	if (m_modelViewer != NULL)
	{
		m_modelViewer->setPreferences(this);
	}
	setupSaveDir(SOSnapshot, SNAPSHOTS_DIR_MODE_KEY, SNAPSHOTS_DIR_KEY,
		LAST_SNAPSHOT_DIR_KEY);
	setupSaveDir(SOPartsList, PARTS_LISTS_DIR_MODE_KEY, PARTS_LISTS_DIR_KEY,
		INV_LAST_SAVE_PATH_KEY);
	setupSaveDir(SOExport, EXPORTS_DIR_MODE_KEY, EXPORTS_DIR_KEY,
		LAST_EXPORT_DIR_KEY);
}

LDPreferences::~LDPreferences(void)
{
}

void LDPreferences::dealloc(void)
{
	TCObject::release(m_modelViewer);
	TCObject::dealloc();
}

void LDPreferences::setupSaveDir(
	SaveOp op,
	const std::string &dirModeKey,
	const std::string &dirKey,
	const std::string &lastDirKey)
{
	m_saveDirModeKeys[op] = dirModeKey;
	m_saveDirKeys[op] = dirKey;
	m_lastSaveDirKeys[op] = lastDirKey;
	m_globalSettings[dirModeKey] = true;
	m_globalSettings[dirKey] = true;
	m_globalSettings[lastDirKey] = true;
}

void LDPreferences::setModelViewer(LDrawModelViewer *value)
{
	if (value != m_modelViewer)
	{
		//if (m_modelViewer != NULL)
		//{
		//	m_modelViewer->setPreferences(NULL);
		//}
		TCObject::release(m_modelViewer);
		m_modelViewer = value;
		if (m_modelViewer != NULL)
		{
			m_modelViewer->retain();
			m_modelViewer->setPreferences(this);
		}
	}
}

int LDPreferences::getColor(int r, int g, int b)
{
	return ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | (b & 0xFF);
}

void LDPreferences::getRGB(int color, int &r, int &g, int &b)
{
	// Colors are stored in RGB format.
	r = (color >> 16) & 0xFF;
	g = (color >> 8) & 0xFF;
	b = color & 0xFF;
}

void LDPreferences::getColorSetting(const char *key, int &r, int &g, int &b,
	TCULong defaultColor)
{
	int value = (int)getLongSetting(key, defaultColor);

	getRGB(value, r, g, b);
}

void LDPreferences::applySettings(void)
{
	applyGeneralSettings();
	applyLDrawSettings();
	applyGeometrySettings();
	applyEffectsSettings();
	applyPrimitivesSettings();
	applyUpdatesSettings();
	if (m_modelViewer != NULL)
	{
		m_modelViewer->setZoomMax(m_zoomMax);
		m_modelViewer->setDistanceMultiplier(1.0f / m_defaultZoom);
		m_modelViewer->setLDConfig(m_ldConfig);
	}
	setupDefaultRotationMatrix();
	setupModelCenter();
	setupModelSize();
}

void LDPreferences::applyGeneralSettings(void)
{
	if (m_modelViewer != NULL)
	{
		int r, g, b;

		// FSAA taken care of automatically.
		getRGB(m_backgroundColor, r, g, b);
		m_modelViewer->setBackgroundRGBA(r, g, b, 0);
		getRGB(m_defaultColor, r, g, b);
		m_modelViewer->setProcessLDConfig(m_processLdConfig);
		m_modelViewer->setRandomColors(m_randomColors);
		m_modelViewer->setSkipValidation(m_skipValidation);
		// showFrameRate taken care of automatically.
		m_modelViewer->setShowAxes(m_showAxes);
		m_modelViewer->setMultiThreaded(m_multiThreaded);
		// showErrors taken care of automatically.
		// fullScreenRefresh taken care of automatically.
		m_modelViewer->setFov(m_fov);
		m_modelViewer->setDefaultRGB((TCByte)r, (TCByte)g, (TCByte)b,
			m_transDefaultColor);
		m_modelViewer->setDefaultColorNumber(m_defaultColorNumber);
		m_modelViewer->setLineSmoothing(m_lineSmoothing);
		m_modelViewer->setMemoryUsage(m_memoryUsage);
	}
}

void LDPreferences::applyLDrawSettings(void)
{
	if (m_ldrawDir.size() > 0)
	{
		if (strcmp(m_ldrawDir.c_str(), LDLModel::lDrawDir()) != 0)
		{
			LDLModel::setLDrawDir(m_ldrawDir.c_str());
			if (m_modelViewer != NULL)
			{
				m_modelViewer->setNeedsReload();
			}
		}
	}
	if (m_modelViewer != NULL)
	{
		TCStringArray *oldExtraDirs = m_modelViewer->getExtraSearchDirs();
		TCStringArray *extraDirs = new TCStringArray;
		bool different = false;
		unsigned int oldCount = 0;
		size_t newCount = m_extraDirs.size();

		if (oldExtraDirs)
		{
			oldCount = oldExtraDirs->getCount();
		}
		if (oldCount != newCount)
		{
			different = true;
		}
		for (int i = 0; i < (int)newCount; i++)
		{
			const char *extraDir = m_extraDirs[i].c_str();

			extraDirs->addString(extraDir);
			if (!different && strcmp(extraDir, oldExtraDirs->stringAtIndex(i)))
			{
				different = true;
			}
		}
		if (different)
		{
			m_modelViewer->setExtraSearchDirs(extraDirs);
		}
		extraDirs->release();
	}
}

void LDPreferences::applyGeometrySettings(void)
{
	if (m_modelViewer != NULL)
	{
		if (m_useSeams)
		{
			m_modelViewer->setSeamWidth(m_seamWidth / 100.0f);
		}
		else
		{
			m_modelViewer->setSeamWidth(0.0f);
		}
		m_modelViewer->setBoundingBoxesOnly(m_boundingBoxesOnly);
		m_modelViewer->setDrawWireframe(m_drawWireframe);
		m_modelViewer->setUseWireframeFog(m_useWireframeFog);
		m_modelViewer->setRemoveHiddenLines(m_removeHiddenLines);
		m_modelViewer->setWireframeLineWidth((GLfloat)m_wireframeThickness);
		m_modelViewer->setBfc(m_bfc);
		m_modelViewer->setRedBackFaces(m_redBackFaces);
		m_modelViewer->setGreenFrontFaces(m_greenFrontFaces);
		m_modelViewer->setBlueNeutralFaces(m_blueNeutralFaces);
		m_modelViewer->setShowsHighlightLines(m_showHighlightLines);
		m_modelViewer->setEdgesOnly(m_edgesOnly);
		m_modelViewer->setDrawConditionalHighlights(m_drawConditionalHighlights);
		m_modelViewer->setShowAllConditionalLines(m_showAllConditionalLines);
		m_modelViewer->setShowConditionalControlPoints(
			m_showConditionalControlPoints);
		m_modelViewer->setUsePolygonOffset(m_usePolygonOffset);
		m_modelViewer->setBlackHighlights(m_blackHighlights);
		m_modelViewer->setHighlightLineWidth((GLfloat)m_edgeThickness);
	}
}

void LDPreferences::applyEffectsSettings(void)
{
	if (m_modelViewer != NULL)
	{
		m_modelViewer->setUseLighting(m_useLighting);
		m_modelViewer->setQualityLighting(m_qualityLighting);
		m_modelViewer->setSubduedLighting(m_subduedLighting);
		m_modelViewer->setUsesSpecular(m_useSpecular);
		m_modelViewer->setOneLight(m_oneLight);
		m_modelViewer->setLightVector(m_lightVector);
		m_modelViewer->setDrawLightDats(m_drawLightDats);
		m_modelViewer->setOptionalStandardLight(m_optionalStandardLight);
		m_modelViewer->setNoLightGeom(m_noLightGeom);
		m_modelViewer->setStereoMode(m_stereoMode);
		m_modelViewer->setStereoEyeSpacing((GLfloat)m_stereoEyeSpacing);
		m_modelViewer->setCutawayMode(m_cutawayMode);
		m_modelViewer->setCutawayAlpha((TCFloat32)m_cutawayAlpha / 100.0f);
		m_modelViewer->setCutawayLineWidth((TCFloat32)m_cutawayThickness);
		m_modelViewer->setSortTransparent(m_sortTransparent);
		m_modelViewer->setUseStipple(m_useStipple);
		m_modelViewer->setUsesFlatShading(m_useFlatShading);
		m_modelViewer->setObi(m_obi);
		m_modelViewer->setPerformSmoothing(m_performSmoothing);
	}
}

void LDPreferences::applyPrimitivesSettings(void)
{
	if (m_modelViewer != NULL)
	{
		m_modelViewer->setAllowPrimitiveSubstitution(m_allowPrimitiveSubstitution);
		m_modelViewer->setTextureStuds(m_textureStuds);
		m_modelViewer->setTextureFilterType(m_textureFilterType);
		m_modelViewer->setAnisoLevel((TCFloat32)m_anisoLevel);
		m_modelViewer->setAnisoLevel(m_anisoLevel);
		m_modelViewer->setCurveQuality(m_curveQuality);
		m_modelViewer->setQualityStuds(m_qualityStuds);
		m_modelViewer->setHiResPrimitives(m_hiResPrimitives);
		m_modelViewer->setTexmaps(m_texmaps);
		m_modelViewer->setTexturesAfterTransparent(m_texturesAfterTransparent);
		m_modelViewer->setTextureOffsetFactor(m_textureOffsetFactor);
	}
}

void LDPreferences::applyUpdatesSettings(void)
{
	if (m_modelViewer != NULL)
	{
		m_modelViewer->setCheckPartTracker(m_checkPartTracker);
		m_modelViewer->setMissingPartWait(m_missingPartWait);
		m_modelViewer->setUpdatedPartWait(m_updatedPartWait);
		if (m_proxyType == 2)
		{
//			TCWebClient::setProxyServer(m_proxyServer.c_str());
//			TCWebClient::setProxyPort(m_proxyPort);
		}
#ifdef WIN32
		else if (m_proxyType == 1)
		{
			std::string appName = TCUserDefaults::getAppName();

			// This is sort of cheating, but since I know exactly how TCUserDefaults
			// works, I know that this will get the job done.  Note that if the
			// actual type of the data in the registry doesn't match the requested
			// type, TCUserDefaults treats the registry value as if it doesn't
			// exist.
			TCUserDefaults::setAppName(
				"Microsoft/Windows/CurrentVersion/Internet Settings");
			if (TCUserDefaults::longForKey("ProxyEnable", 0, false))
			{
				char *proxyServer = TCUserDefaults::stringForKey("ProxyServer",
					NULL, false);

				if (proxyServer)
				{
					char *colon = (char *)strchr(proxyServer, ':');

					if (colon)
					{
						int proxyPort;

						*colon = 0;
						if (sscanf(&colon[1], "%d", &proxyPort) == 1)
						{
//							TCWebClient::setProxyServer(proxyServer);
//							TCWebClient::setProxyPort(proxyPort);
						}
					}
					delete[] proxyServer;
				}
			}
			TCUserDefaults::setAppName(appName.c_str());
		}
#endif
		else
		{
//			TCWebClient::setProxyServer(NULL);
		}
	}
}

void LDPreferences::loadSettings(void)
{
	loadGeneralSettings();
	loadLDrawSettings();
	loadGeometrySettings();
	loadEffectsSettings();
	loadPrimitivesSettings();
	loadUpdatesSettings();
	loadInventorySettings();
	m_changedSettings.clear();

	m_skipValidation = false;
	m_zoomMax = getLongSetting(ZOOM_MAX_KEY, 199) / 100.0f;
	m_defaultZoom = getFloatSetting(DEFAULT_ZOOM_KEY, 1.0f);
	m_ldConfig = getStringSetting(LDCONFIG_KEY);
}

void LDPreferences::loadDefaultGeneralSettings(bool initializing /*= true*/)
{
	int i;

	if (initializing)
	{
		m_initializing = true;
	}
	setFsaaMode(0);
	setLineSmoothing(false);
	setBackgroundColor(0, 0, 0);
	setDefaultColor(0x99, 0x99, 0x99);
	setTransDefaultColor(false);
	setProcessLdConfig(true);
	setRandomColors(false);
	setShowFps(false);
	setShowAxes(false);
	setShowErrors(true);
	setDisableSmp(false);
	setFullScreenRefresh(0);
	setFov(45.0f);
	setMemoryUsage(2);
	for (i = 0; i < 16; i++)
	{
		int r, g, b, a;

		LDLPalette::getDefaultRGBA(i, r, g, b, a);
		setCustomColor(i, r, g, b);
	}
	setSaveDirDefault(SOSnapshot, DDMModelDir);
	setSaveDirDefault(SOPartsList, DDMLastDir);
	setSaveDirDefault(SOExport, DDMModelDir);
	m_initializing = false;
}

void LDPreferences::loadDefaultLDrawSettings(bool initializing /*= true*/)
{
	if (initializing)
	{
		m_initializing = true;
	}
	setLDrawDir(LDLModel::lDrawDir(true));
	setExtraDirs(StringVector());
	m_initializing = false;
}

void LDPreferences::loadDefaultGeometrySettings(bool initializing /*= true*/)
{
	if (initializing)
	{
		m_initializing = true;
	}
	setUseSeams(true);
	setSeamWidth(50);
	setBoundingBoxesOnly(false);
	setDrawWireframe(false);
	setUseWireframeFog(false);
	setRemoveHiddenLines(false);
	setWireframeThickness(1);
	setBfc(true);
	setRedBackFaces(false);
	setGreenFrontFaces(false);
	setBlueNeutralFaces(false);
	setShowHighlightLines(false);
	setEdgesOnly(false);
	setDrawConditionalHighlights(true);
	setShowAllConditionalLines(false);
	setShowConditionalControlPoints(false);
	setUsePolygonOffset(true);
	setBlackHighlights(false);
	setEdgeThickness(1);
	m_initializing = false;
}

void LDPreferences::loadDefaultEffectsSettings(bool initializing /*= true*/)
{
	if (initializing)
	{
		m_initializing = true;
	}
	setUseLighting(true);
	setQualityLighting(false);
	setSubduedLighting(false);
	setUseSpecular(true);
	setOneLight(false);
	setLightVector(TCVector(0.0, 0.0, 1.0));
	setDrawLightDats(true);
	setOptionalStandardLight(true);
	setNoLightGeom(false);
	setStereoMode(LDVStereoNone);
	setStereoEyeSpacing(50);
	setCutawayMode(LDVCutawayNormal);
	setCutawayAlpha(100);
	setCutawayThickness(1);
	setSortTransparent(true);
	setUseStipple(false);
	setUseFlatShading(false);
	setObi(false);
	setPerformSmoothing(true);
	m_initializing = false;
}

void LDPreferences::loadDefaultPrimitivesSettings(bool initializing /*= true*/)
{
	if (initializing)
	{
		m_initializing = true;
	}
	setAllowPrimitiveSubstitution(true);
	setTextureStuds(true);
	setTextureFilterType(GL_LINEAR_MIPMAP_LINEAR);
	setAnisoLevel(1.0f);
	setCurveQuality(2);
	setQualityStuds(false);
	setHiResPrimitives(false);
	setTexmaps(true);
	setTexturesAfterTransparent(false);
	setTextureOffsetFactor(5.0);
	m_initializing = false;
}

void LDPreferences::loadDefaultUpdatesSettings(bool initializing /*= true*/)
{
	if (initializing)
	{
		m_initializing = true;
	}
	setProxyType(0);
	setProxyServer("");
	setProxyPort(80);
	setCheckPartTracker(true);
	setMissingPartWait(7);
	setUpdatedPartWait(7);
	m_initializing = false;
}

void LDPreferences::loadDefaultInventorySettings(bool initializing /*= true*/)
{
	if (initializing)
	{
		m_initializing = true;
	}
	setInvShowModel(false);
	setInvOverwriteSnapshot(false);
	setInvExternalCss(false);
	setInvPartImages(true);
	setInvShowFile(true);
	setInvShowTotal(true);
	LongVector columnOrder;
	columnOrder.push_back(1);	// Part
	columnOrder.push_back(3);	// Color
	columnOrder.push_back(4);	// Quantity
	setInvColumnOrder(columnOrder);
	setSaveDirDefault(SOPartsList, DDMLastDir);
	m_initializing = false;
}

void LDPreferences::loadGeneralSettings(void)
{
	int i;

	loadDefaultGeneralSettings();
	m_fsaaMode = getIntSetting(FSAA_MODE_KEY, m_fsaaMode);
	m_lineSmoothing = getBoolSetting(LINE_SMOOTHING_KEY, m_lineSmoothing);
	m_backgroundColor = (TCULong)getLongSetting(BACKGROUND_COLOR_KEY,
		m_backgroundColor);
	m_defaultColor = (TCULong)getLongSetting(DEFAULT_COLOR_KEY, m_defaultColor);
	m_transDefaultColor = getBoolSetting(TRANS_DEFAULT_COLOR_KEY,
		m_transDefaultColor);
	m_defaultColorNumber = getIntSetting(DEFAULT_COLOR_NUMBER_KEY,
		m_defaultColorNumber);
	m_processLdConfig = getBoolSetting(PROCESS_LDCONFIG_KEY, m_processLdConfig);
	m_randomColors = getBoolSetting(RANDOM_COLORS_KEY, m_randomColors);
	m_skipValidation = getBoolSetting(SKIP_VALIDATION_KEY, m_skipValidation);
	m_showFps = getBoolSetting(SHOW_FPS_KEY, m_showFps);
	m_showAxes = getBoolSetting(SHOW_AXES_KEY, m_showAxes);
	m_showErrors = getBoolSetting(SHOW_ERRORS_KEY, m_showErrors);
	m_multiThreaded = getBoolSetting(MULTI_THREADED_KEY, m_multiThreaded);
	m_fullScreenRefresh = getIntSetting(FULLSCREEN_REFRESH_KEY,
		m_fullScreenRefresh);
	m_fov = getFloatSetting(FOV_KEY, (TCFloat32)m_fov);
	m_memoryUsage = getIntSetting(MEMORY_USAGE_KEY, m_memoryUsage);
	if (m_memoryUsage < 0 || m_memoryUsage > 2)
	{
		setMemoryUsage(2, true);
	}
	for (i = 0; i < 16; i++)
	{
		char key[128];

		sprintf(key, "%s/Color%02d", CUSTOM_COLORS_KEY, i);
		// Windows XP doesn't like the upper bits to be set, so mask those out.
		m_customColors[i] = getLongSetting(key, m_customColors[i]) & 0xFFFFFF;
	}
	loadSaveDir(SOSnapshot);
	loadSaveDir(SOPartsList);
	loadSaveDir(SOExport);
	//m_snapshotsDirMode = (DefaultDirMode)getIntSetting(SNAPSHOTS_DIR_MODE_KEY,
	//	m_snapshotsDirMode);
	//m_snapshotsDir = getStringSetting(SNAPSHOTS_DIR_KEY, m_snapshotsDir.c_str(),
	//	true);
	//m_partsListsDirMode = (DefaultDirMode)getIntSetting(
	//	PARTS_LISTS_DIR_MODE_KEY, m_partsListsDirMode);
	//m_partsListsDir = getStringSetting(PARTS_LISTS_DIR_KEY,
	//	m_partsListsDir.c_str(), true);
}

void LDPreferences::loadLDrawSettings(void)
{
	std::string dirKey = std::string(EXTRA_SEARCH_DIRS_KEY) + "/Dir";

	loadDefaultLDrawSettings();
	m_ldrawDir = getStringSetting(LDRAWDIR_KEY, m_ldrawDir.c_str(), true);
	m_extraDirs = getStringVectorSetting(dirKey.c_str(), m_extraDirs,
		true, 3, 1);
}

void LDPreferences::loadGeometrySettings(void)
{
	int useSeams;

	loadDefaultGeometrySettings();
	useSeams = getIntSetting(SEAMS_KEY, -1);
	m_seamWidth = getIntSetting(SEAM_WIDTH_KEY, m_seamWidth);
	if (useSeams == -1)
	{
		if (m_seamWidth)
		{
			m_useSeams = true;
		}
		else
		{
			m_useSeams = false;
		}
	}
	else
	{
		m_useSeams = useSeams != 0;
	}
	m_boundingBoxesOnly = getBoolSetting(BOUNDING_BOXES_ONLY_KEY,
		m_boundingBoxesOnly);
	m_drawWireframe = getBoolSetting(WIREFRAME_KEY, m_drawWireframe);
	m_useWireframeFog = getBoolSetting(WIREFRAME_FOG_KEY, m_useWireframeFog);
	m_removeHiddenLines = getBoolSetting(REMOVE_HIDDEN_LINES_KEY,
		m_removeHiddenLines);
	m_wireframeThickness = getIntSetting(WIREFRAME_THICKNESS_KEY,
		m_wireframeThickness);
	m_bfc = getBoolSetting(BFC_KEY, m_bfc);
	m_redBackFaces = getBoolSetting(RED_BACK_FACES_KEY, m_redBackFaces);
	m_greenFrontFaces = getBoolSetting(GREEN_FRONT_FACES_KEY,
		m_greenFrontFaces);
	m_blueNeutralFaces = getBoolSetting(BLUE_NEUTRAL_FACES_KEY,
		m_blueNeutralFaces);
	m_showHighlightLines = getBoolSetting(SHOW_HIGHLIGHT_LINES_KEY,
		m_showHighlightLines);
	m_edgesOnly = getBoolSetting(EDGES_ONLY_KEY, m_edgesOnly);
	m_drawConditionalHighlights = getBoolSetting(CONDITIONAL_HIGHLIGHTS_KEY,
		m_drawConditionalHighlights);
	m_showAllConditionalLines = getBoolSetting(SHOW_ALL_TYPE5_KEY,
		m_showAllConditionalLines);
	m_showConditionalControlPoints =
		getBoolSetting(SHOW_TYPE5_CONTROL_POINTS_KEY,
		m_showConditionalControlPoints);
	m_usePolygonOffset = getBoolSetting(POLYGON_OFFSET_KEY, m_usePolygonOffset);
	m_blackHighlights = getBoolSetting(BLACK_HIGHLIGHTS_KEY, m_blackHighlights);
	m_edgeThickness = getIntSetting(EDGE_THICKNESS_KEY, m_edgeThickness);
}

void LDPreferences::loadEffectsSettings(void)
{
	loadDefaultEffectsSettings();
	m_useLighting = getBoolSetting(LIGHTING_KEY, m_useLighting);
	m_qualityLighting = getBoolSetting(QUALITY_LIGHTING_KEY, m_qualityLighting);
	m_subduedLighting = getBoolSetting(SUBDUED_LIGHTING_KEY, m_subduedLighting);
	m_useSpecular = getBoolSetting(SPECULAR_KEY, m_useSpecular);
	m_oneLight = getBoolSetting(ONE_LIGHT_KEY, m_oneLight);
	m_lightVector = getTCVectorSetting(LIGHT_VECTOR_KEY, m_lightVector);
	m_drawLightDats = getBoolSetting(DRAW_LIGHT_DATS_KEY, m_drawLightDats);
	m_optionalStandardLight = getBoolSetting(OPTIONAL_STANDARD_LIGHT_KEY,
		m_optionalStandardLight);
	m_noLightGeom = getBoolSetting(NO_LIGHT_GEOM_KEY, m_noLightGeom);
	m_stereoMode = (LDVStereoMode)getLongSetting(STEREO_MODE_KEY, m_stereoMode);
	m_stereoEyeSpacing = getIntSetting(STEREO_SPACING_KEY, m_stereoEyeSpacing);
	m_cutawayMode = (LDVCutawayMode)getLongSetting(CUTAWAY_MODE_KEY,
		m_cutawayMode);
	m_cutawayAlpha = getIntSetting(CUTAWAY_ALPHA_KEY, m_cutawayAlpha);
	m_cutawayThickness = getIntSetting(CUTAWAY_THICKNESS_KEY,
		m_cutawayThickness);
	m_sortTransparent = getBoolSetting(SORT_KEY, m_sortTransparent);
	m_useStipple = getBoolSetting(STIPPLE_KEY, m_useStipple);
	m_useFlatShading = getBoolSetting(FLAT_SHADING_KEY, m_useFlatShading);
	m_obi = getBoolSetting(OBI_KEY, m_obi);
	m_performSmoothing = getBoolSetting(PERFORM_SMOOTHING_KEY,
		m_performSmoothing);
}

void LDPreferences::loadPrimitivesSettings(void)
{
	loadDefaultPrimitivesSettings();
	m_allowPrimitiveSubstitution =
		getBoolSetting(PRIMITIVE_SUBSTITUTION_KEY,
		m_allowPrimitiveSubstitution);
	m_textureStuds = getBoolSetting(TEXTURE_STUDS_KEY, m_textureStuds);
	m_textureFilterType = getIntSetting(TEXTURE_FILTER_TYPE_KEY,
		m_textureFilterType);
	m_anisoLevel = getFloatSetting(ANISO_LEVEL_KEY, m_anisoLevel);
	m_curveQuality = getIntSetting(CURVE_QUALITY_KEY, m_curveQuality);
	m_qualityStuds = getBoolSetting(QUALITY_STUDS_KEY, m_qualityStuds);
	m_hiResPrimitives = getBoolSetting(HI_RES_PRIMITIVES_KEY,
		m_hiResPrimitives);
	m_texmaps = getBoolSetting(TEXMAPS_KEY, m_texmaps);
	m_texturesAfterTransparent = getBoolSetting(TEXTURES_AFTER_TRANSPARENT_KEY,
		m_texturesAfterTransparent);
	m_textureOffsetFactor = getFloatSetting(TEXTURE_OFFSET_FACTOR_KEY,
		m_textureOffsetFactor);
}

void LDPreferences::loadUpdatesSettings(void)
{
	loadDefaultUpdatesSettings();
	m_proxyType = getIntSetting(PROXY_TYPE_KEY, m_proxyType);
	m_proxyServer = getStringSetting(PROXY_SERVER_KEY, NULL);
	m_proxyPort = getIntSetting(PROXY_PORT_KEY, m_proxyPort);
	m_checkPartTracker = getBoolSetting(CHECK_PART_TRACKER_KEY,
		m_checkPartTracker);
	m_missingPartWait = getIntSetting(CHECK_PART_WAIT_KEY, m_missingPartWait);
	m_updatedPartWait = getIntSetting(CHECK_PART_UPDATE_WAIT_KEY,
		m_updatedPartWait);
}

void LDPreferences::loadInventorySettings(void)
{
	loadDefaultInventorySettings();
	m_invShowModel = getBoolSetting(INV_SHOW_MODEL_KEY, m_invShowModel);
	m_invOverwriteSnapshot = getBoolSetting(INV_OVERWRITE_SNAPSHOT_KEY,
		m_invOverwriteSnapshot);
	m_invExternalCss = getBoolSetting(INV_EXTERNAL_CSS_KEY, m_invExternalCss);
	m_invPartImages = getBoolSetting(INV_PART_IMAGES_KEY, m_invPartImages);
	m_invShowFile = getBoolSetting(INV_SHOW_FILE_KEY, m_invShowFile);
	m_invShowTotal = getBoolSetting(INV_SHOW_TOTAL_KEY, m_invShowTotal);
	m_invColumnOrder = getLongVectorSetting(INV_COLUMN_ORDER_KEY,
		m_invColumnOrder);
	loadSaveDir(SOPartsList);
	//m_invLastSavePath = getStringSetting(INV_LAST_SAVE_PATH_KEY,
	//	m_invLastSavePath.c_str(), true);
}

void LDPreferences::commitSettings(void)
{
	commitGeneralSettings(false);
	commitLDrawSettings(false);
	commitGeometrySettings(false);
	commitEffectsSettings(false);
	commitPrimitivesSettings(false);
	commitUpdatesSettings(false);
	commitInventorySettings(false);
	TCUserDefaults::flush();
	//commitPrefSetsSettings();
}

void LDPreferences::commitGeneralSettings(bool flush /*= true*/)
{
	int i;
	int r, g, b;

	setFsaaMode(m_fsaaMode, true);
	setLineSmoothing(m_lineSmoothing, true);
	getRGB(m_backgroundColor, r, g, b);
	setBackgroundColor(r, g, b, true);
	getRGB(m_defaultColor, r, g, b);
	setDefaultColor(r, g, b, true);
	setTransDefaultColor(m_transDefaultColor, true);
	for (i = 0; i < 16; i++)
	{
		getRGB(m_customColors[i], r, g, b);
		setCustomColor(i, r, g, b, true);
	}
	setProcessLdConfig(m_processLdConfig, true);
	setRandomColors(m_randomColors, true);
	setShowFps(m_showFps, true);
	setShowAxes(m_showAxes, true);
	setShowErrors(m_showErrors, true);
	setDisableSmp(!m_multiThreaded, true);
	setFullScreenRefresh(m_fullScreenRefresh, true);
	setFov(m_fov, true);
	setMemoryUsage(m_memoryUsage, true);
	commitSaveDir(SOSnapshot);
	commitSaveDir(SOPartsList);
	commitSaveDir(SOExport);
	//setSnapshotsDirMode(m_snapshotsDirMode, true);
	//setSnapshotsDir(m_snapshotsDir.c_str(), true);
	//setPartsListsDirMode(m_partsListsDirMode, true);
	//setPartsListsDir(m_partsListsDir.c_str(), true);
	if (flush)
	{
		TCUserDefaults::flush();
	}
}

void LDPreferences::commitLDrawSettings(bool flush /*= true*/)
{
	setLDrawDir(m_ldrawDir.c_str(), true);
	setExtraDirs(m_extraDirs, true);
	if (flush)
	{
		TCUserDefaults::flush();
	}
}

void LDPreferences::commitGeometrySettings(bool flush /*= true*/)
{
	setSetting(m_useSeams, m_useSeams, SEAMS_KEY, true);
	if (getUseSeams())
	{
		setSeamWidth(m_seamWidth, true);
	}
	setBoundingBoxesOnly(m_boundingBoxesOnly, true);
	setDrawWireframe(m_drawWireframe, true);
	setUseWireframeFog(m_useWireframeFog, true);
	setRemoveHiddenLines(m_removeHiddenLines, true);
	setWireframeThickness(m_wireframeThickness, true);
	setBfc(m_bfc, true);
	setRedBackFaces(m_redBackFaces, true);
	setGreenFrontFaces(m_greenFrontFaces, true);
	setBlueNeutralFaces(m_blueNeutralFaces, true);
	setShowHighlightLines(m_showHighlightLines, true);
	if (m_showHighlightLines)
	{
		setEdgesOnly(m_edgesOnly, true);
		setDrawConditionalHighlights(m_drawConditionalHighlights, true);
		if (m_drawConditionalHighlights)
		{
			setShowAllConditionalLines(m_showAllConditionalLines, true);
			setShowConditionalControlPoints(m_showConditionalControlPoints,
				true);
		}
		setUsePolygonOffset(m_usePolygonOffset, true);
		setBlackHighlights(m_blackHighlights, true);
	}
	setEdgeThickness(m_edgeThickness, true);
	if (flush)
	{
		TCUserDefaults::flush();
	}
}

void LDPreferences::commitEffectsSettings(bool flush /*= true*/)
{
	setUseLighting(m_useLighting, true);
	if (m_useLighting)
	{
		setQualityLighting(m_qualityLighting, true);
		setSubduedLighting(m_subduedLighting, true);
		setUseSpecular(m_useSpecular, true);
		setOneLight(m_oneLight, true);
		setLightVector(m_lightVector, true);
		setDrawLightDats(m_drawLightDats, true);
		setOptionalStandardLight(m_optionalStandardLight, true);
	}
	// NOTE: lighting doesn't have to be enabled for this one.
	setNoLightGeom(m_noLightGeom, true);
	setStereoMode(m_stereoMode, true);
	setStereoEyeSpacing(m_stereoEyeSpacing, true);
	setCutawayMode(m_cutawayMode, true);
	setCutawayAlpha(m_cutawayAlpha, true);
	setCutawayThickness(m_cutawayThickness, true);
	setUseStipple(m_useStipple, true);
	setSortTransparent(m_sortTransparent, true);
	setUseFlatShading(m_useFlatShading, true);
	setObi(m_obi, true);
	setPerformSmoothing(m_performSmoothing, true);
	if (flush)
	{
		TCUserDefaults::flush();
	}
}

void LDPreferences::commitPrimitivesSettings(bool flush /*= true*/)
{
	setAllowPrimitiveSubstitution(m_allowPrimitiveSubstitution, true);
	if (m_allowPrimitiveSubstitution)
	{
		setTextureStuds(m_textureStuds, true);
		setTextureFilterType(m_textureFilterType, true);
		setAnisoLevel(m_anisoLevel, true);
		setCurveQuality(m_curveQuality, true);
	}
	setQualityStuds(m_qualityStuds, true);
	setHiResPrimitives(m_hiResPrimitives, true);
	setTexmaps(m_texmaps, true);
	setTexturesAfterTransparent(m_texturesAfterTransparent, true);
	setTextureOffsetFactor(m_textureOffsetFactor, true);
	if (flush)
	{
		TCUserDefaults::flush();
	}
}

void LDPreferences::commitUpdatesSettings(bool flush /*= true*/)
{
	setProxyType(m_proxyType, true);
	if (m_proxyType == 2)
	{
		setProxyServer(m_proxyServer.c_str(), true);
		setProxyPort(m_proxyPort, true);
	}
	setCheckPartTracker(m_checkPartTracker, true);
	setMissingPartWait(m_missingPartWait, true);
	setUpdatedPartWait(m_updatedPartWait, true);
	if (flush)
	{
		TCUserDefaults::flush();
	}
}

void LDPreferences::commitInventorySettings(bool flush /*= true*/)
{
	setInvShowModel(m_invShowModel, true);
	setInvOverwriteSnapshot(m_invOverwriteSnapshot, true);
	setInvExternalCss(m_invExternalCss, true);
	setInvPartImages(m_invPartImages, true);
	setInvShowFile(m_invShowFile, true);
	setInvShowTotal(m_invShowTotal, true);
	setInvColumnOrder(m_invColumnOrder, true);
	//setInvLastSavePath(m_invLastSavePath.c_str(), true);
	commitSaveDir(SOPartsList);
	if (flush)
	{
		TCUserDefaults::flush();
	}
}

void LDPreferences::setupDefaultRotationMatrix(void)
{
	std::string value = getStringSetting(CAMERA_GLOBE_KEY);
	TCFloat latitude = getFloatSetting(DEFAULT_LATITUDE_KEY, 30.0f);
	TCFloat longitude = getFloatSetting(DEFAULT_LONGITUDE_KEY, 45.0f);

	if (!value.length())
	{
		value = getStringSetting(DEFAULT_LAT_LONG_KEY);
	}
	if (value.length())
	{
		if (sscanf(value.c_str(), "%f,%f", &latitude, &longitude) == 2)
		{
			TCFloat radius = -1;
			TCFloat resultMatrix[16];

			TCVector::calcRotationMatrix(latitude, longitude, resultMatrix);
			if (sscanf(value.c_str(), "%*f,%*f,%f", &radius) != 1)
			{
				radius = -1;
			}
			if (m_modelViewer != NULL)
			{
				m_modelViewer->setDefaultRotationMatrix(resultMatrix);
				if (radius > 0)
				{
					m_modelViewer->setDefaultDistance(radius);
				}
			}
		}
	}
	else
	{
		value = getStringSetting(DEFAULT_MATRIX_KEY);
		if (value.length())
		{
			TCFloat matrix[16];

			memset(matrix, 0, sizeof(matrix));
			matrix[15] = 1.0f;
			// ToDo: how to deal with 64-bit float scanf?
			if (sscanf(value.c_str(), "%f,%f,%f,%f,%f,%f,%f,%f,%f",
				&matrix[0], &matrix[4], &matrix[8],
				&matrix[1], &matrix[5], &matrix[9],
				&matrix[2], &matrix[6], &matrix[10]) == 9)
			{
				if (m_modelViewer != NULL)
				{
					m_modelViewer->setDefaultRotationMatrix(matrix);
				}
			}
		}
	}
	if (m_modelViewer != NULL)
	{
		m_modelViewer->setDefaultLatLong(latitude, longitude);
	}
}

void LDPreferences::setupModelCenter(void)
{
	std::string value = getStringSetting(MODEL_CENTER_KEY);

	if (value.length())
	{
		TCFloat center[3];
		// ToDo: how to deal with 64-bit float scanf?
		if (sscanf(value.c_str(), "%f,%f,%f", &center[0], &center[1],&center[2])
			== 3)
		{
			if (m_modelViewer != NULL)
			{
				m_modelViewer->setModelCenter(center);
			}
		}
	}
}

void LDPreferences::setupModelSize(void)
{
	std::string value = getStringSetting(MODEL_SIZE_KEY);

	if (value.length())
	{
		TCFloat size;
		// ToDo: how to deal with 64-bit float scanf?
		if (sscanf(value.c_str(), "%f", &size) == 1)
		{
			if (m_modelViewer != NULL)
			{
				m_modelViewer->setModelSize(size);
			}
		}
	}
}

void LDPreferences::setColorSetting(TCULong &setting, int r, int g, int b,
									 const char *key, bool commit)
{
	TCULong value = getColor(r, g, b);
	if (setting != value || (m_changedSettings[key] && commit))
	{
		setting = value;
		if (commit)
		{
			TCUserDefaults::setLongForKey(setting, key, !m_globalSettings[key]);
			m_changedSettings.erase(key);
		}
		else
		{
			m_changedSettings[key] = true;
		}
	}
}

void LDPreferences::setSetting(
	StringVector &setting,
	const StringVector &value,
	const char *key,
	bool commit,
	bool isPath, /*= false*/
	int keyDigits /*= 2*/,
	int startIndex /*= 0*/)
{
	if (setting != value || (m_changedSettings[key] && commit))
	{
		setting = value;
		if (commit)
		{
			TCUserDefaults::setStringVectorForKey(value, key,
				!m_globalSettings[key], isPath, keyDigits, startIndex);
			m_changedSettings.erase(key);
		}
		else if (!m_initializing)
		{
			m_changedSettings[key] = true;
		}
	}
}

void LDPreferences::setSetting(LongVector &setting, const LongVector &value,
							   const char *key, bool commit)
{
	if (setting != value || (m_changedSettings[key] && commit))
	{
		setting = value;
		if (commit)
		{
			TCUserDefaults::setLongVectorForKey(value, key,
				!m_globalSettings[key]);
			m_changedSettings.erase(key);
		}
		else if (!m_initializing)
		{
			m_changedSettings[key] = true;
		}
	}
}

bool LDPreferences::setSetting(TCVector &setting, const TCVector &value,
							   const char *key, bool commit)
{
	if (setting != value || (m_changedSettings[key] && commit))
	{
		setting = value;
		if (commit)
		{
			char stringValue[128];

			sprintf(stringValue, "%f,%f,%f", value.get(0), value.get(1),
				value.get(2));
			TCUserDefaults::setStringForKey(stringValue, key,
				!m_globalSettings[key]);
			m_changedSettings.erase(key);
		}
		else if (!m_initializing)
		{
			m_changedSettings[key] = true;
		}
		return true;
	}
	return false;
}

void LDPreferences::setSetting(bool &setting, bool value, const char *key,
								bool commit)
{
	if (setting != value || (m_changedSettings[key] && commit))
	{
		setting = value;
		if (commit)
		{
			TCUserDefaults::setLongForKey(value ? 1 : 0, key,
				!m_globalSettings[key]);
			m_changedSettings.erase(key);
		}
		else if (!m_initializing)
		{
			m_changedSettings[key] = true;
		}
	}
}

void LDPreferences::setSetting(int &setting, int value, const char *key,
								bool commit)
{
	if (setting != value || (m_changedSettings[key] && commit))
	{
		setting = value;
		if (commit)
		{
			TCUserDefaults::setLongForKey(value, key, !m_globalSettings[key]);
			m_changedSettings.erase(key);
		}
		else if (!m_initializing)
		{
			m_changedSettings[key] = true;
		}
	}
}

void LDPreferences::setSetting(TCULong &setting, TCULong value, const char *key,
								bool commit)
{
	if (setting != value || (m_changedSettings[key] && commit))
	{
		setting = value;
		if (commit)
		{
			TCUserDefaults::setLongForKey((long)value, key,
				!m_globalSettings[key]);
			m_changedSettings.erase(key);
		}
		else if (!m_initializing)
		{
			m_changedSettings[key] = true;
		}
	}
}

void LDPreferences::setSetting(TCFloat &setting, TCFloat value, const char *key,
								bool commit)
{
	if (setting != value || (m_changedSettings[key] && commit))
	{
		setting = value;
		if (commit)
		{
			TCUserDefaults::setFloatForKey(value, key, !m_globalSettings[key]);
			m_changedSettings.erase(key);
		}
		else if (!m_initializing)
		{
			m_changedSettings[key] = true;
		}
	}
}

void LDPreferences::setSetting(
	std::string &setting,
	const std::string &value,
	const char *key,
	bool commit,
	bool isPath /*= false*/)
{
	if (setting != value || (m_changedSettings[key] && commit))
	{
		setting = value;
		if (commit)
		{
			if (isPath)
			{
				TCUserDefaults::setPathForKey(value.c_str(), key,
					!m_globalSettings[key]);
			}
			else
			{
				TCUserDefaults::setStringForKey(value.c_str(), key,
					!m_globalSettings[key]);
			}
			m_changedSettings.erase(key);
		}
		else if (!m_initializing)
		{
			m_changedSettings[key] = true;
		}
	}
}

bool LDPreferences::getBoolSetting(const char *key, bool defaultValue)
{
	return TCUserDefaults::longForKey(key, (long)defaultValue,
		!m_globalSettings[key]) != 0;
}

LongVector LDPreferences::getLongVectorSetting(
	const char *key,
	const LongVector &defaultValue)
{
	return TCUserDefaults::longVectorForKey(key, defaultValue,
		!m_globalSettings[key]);
}

StringVector LDPreferences::getStringVectorSetting(
	const char *key,
	const StringVector &defaultValue,
	bool isPath, /*= false*/
	int keyDigits /*= 2*/,
	int startIndex /*= 0*/)
{
	return TCUserDefaults::stringVectorForKey(key, defaultValue,
		!m_globalSettings[key], isPath, keyDigits, startIndex);
}

TCVector LDPreferences::getTCVectorSetting(const char *key,
										   const TCVector &defaultValue)
{
	std::string vectorString;

	vectorString = getStringSetting(key);
	if (vectorString.length())
	{
		TCFloat lx, ly, lz;

		// ToDo: how to deal with 64-bit float scanf?
		if (sscanf(vectorString.c_str(), "%f,%f,%f", &lx, &ly, &lz) == 3)
		{
			return TCVector(lx, ly, lz);
		}
	}
	return defaultValue;
}

long LDPreferences::getLongSetting(const char *key, long defaultValue)
{
	return TCUserDefaults::longForKey(key, defaultValue, !m_globalSettings[key]);
}

int LDPreferences::getIntSetting(const char *key, int defaultValue)
{
	return (int)TCUserDefaults::longForKey(key, defaultValue,
		!m_globalSettings[key]);
}

float LDPreferences::getFloatSetting(const char *key, float defaultValue)
{
	return TCUserDefaults::floatForKey(key, defaultValue, !m_globalSettings[key]);
}

std::string LDPreferences::getStringSetting(
	const char *key,
	const char *defaultValue /*= NULL*/,
	bool isPath /*= false*/)
{
	char *tmpString;
	std::string result;

	if (isPath)
	{
		tmpString = TCUserDefaults::pathForKey(key, defaultValue,
			!m_globalSettings[key]);
	}
	else
	{
		tmpString = TCUserDefaults::stringForKey(key, defaultValue,
			!m_globalSettings[key]);
	}
	if (tmpString)
	{
		result = tmpString;
		delete[] tmpString;
	}
	return result;
}

// General settings
void LDPreferences::setFsaaMode(int value, bool commit)
{
	setSetting(m_fsaaMode, value, FSAA_MODE_KEY, commit);
}

void LDPreferences::setLineSmoothing(bool value, bool commit)
{
	setSetting(m_lineSmoothing, value, LINE_SMOOTHING_KEY, commit);
}

void LDPreferences::setBackgroundColor(int r, int g, int b, bool commit)
{
	setColorSetting(m_backgroundColor, r, g, b, BACKGROUND_COLOR_KEY, commit);
}

void LDPreferences::setDefaultColor(int r, int g, int b, bool commit)
{
	setColorSetting(m_defaultColor, r, g, b, DEFAULT_COLOR_KEY, commit);
}

void LDPreferences::setTransDefaultColor(bool value, bool commit, bool apply)
{
	setSetting(m_transDefaultColor, value, TRANS_DEFAULT_COLOR_KEY, commit);
	if (apply && m_modelViewer != NULL)
	{
		int r, g, b;

		getRGB(m_defaultColor, r, g, b);
		m_modelViewer->setDefaultRGB((TCByte)r, (TCByte)g, (TCByte)b,
			m_transDefaultColor);
	}
}

void LDPreferences::setProcessLdConfig(bool value, bool commit)
{
	setSetting(m_processLdConfig, value, PROCESS_LDCONFIG_KEY, commit);
}

void LDPreferences::setRandomColors(bool value, bool commit, bool apply)
{
	setSetting(m_randomColors, value, RANDOM_COLORS_KEY, commit);
	if (apply && m_modelViewer != NULL)
	{
		m_modelViewer->setRandomColors(value);
	}
}

void LDPreferences::setShowFps(bool value, bool commit)
{
	setSetting(m_showFps, value, SHOW_FPS_KEY, commit);
}

void LDPreferences::setShowAxes(bool value, bool commit, bool apply)
{
	setSetting(m_showAxes, value, SHOW_AXES_KEY, commit);
	if (apply && m_modelViewer != NULL)
	{
		m_modelViewer->setShowAxes(value);
	}
}

void LDPreferences::setShowErrors(bool value, bool commit)
{
	setSetting(m_showErrors, value, SHOW_ERRORS_KEY, commit);
}

void LDPreferences::setDisableSmp(bool value, bool commit)
{
	setSetting(m_multiThreaded, !value, MULTI_THREADED_KEY, commit);
}

void LDPreferences::setFullScreenRefresh(int value, bool commit)
{
	setSetting(m_fullScreenRefresh, value, FULLSCREEN_REFRESH_KEY, commit);
}

void LDPreferences::setFov(TCFloat value, bool commit)
{
	setSetting(m_fov, value, FOV_KEY, commit);
}

void LDPreferences::setMemoryUsage(int value, bool commit)
{
	setSetting(m_memoryUsage, value, MEMORY_USAGE_KEY, commit);
}

void LDPreferences::setCustomColor(int index, int r, int g, int b, bool commit)
{
	char key[128];

	sprintf(key, "%s/Color%02d", CUSTOM_COLORS_KEY, index);
	setColorSetting(m_customColors[index], r, g, b, key, commit);
}

void LDPreferences::setSaveDirMode(
	SaveOp op,
	DefaultDirMode value,
	bool commit /*= false*/)
{
	DefaultDirMode &mode = m_saveDirModes[op];
	int intMode = mode;

	setSetting(intMode, value, m_saveDirModeKeys[op].c_str(), commit);
	mode = (DefaultDirMode)intMode;
}

void LDPreferences::setSaveDir(
	SaveOp op,
	const char *value,
	bool commit /*= false*/)
{
	setSetting(m_saveDirs[op], value, m_saveDirKeys[op].c_str(), commit, true);
}

void LDPreferences::setLastSaveDir(
	SaveOp op,
	const char *value,
	bool commit /*= false*/)
{
	setSetting(m_lastSaveDirs[op], value, m_lastSaveDirKeys[op].c_str(), commit,
		true);
}

//void LDPreferences::setSnapshotsDirMode(
//	DefaultDirMode value,
//	bool commit /*= false*/)
//{
//	int intValue = m_snapshotsDirMode;
//	
//	setSetting(intValue, value, SNAPSHOTS_DIR_MODE_KEY, commit);
//	m_snapshotsDirMode = (DefaultDirMode)intValue;
//}
//
//void LDPreferences::setSnapshotsDir(const char *value, bool commit)
//{
//	setSetting(m_snapshotsDir, value, SNAPSHOTS_DIR_KEY, commit);
//}
//
//void LDPreferences::setPartsListsDirMode(
//	DefaultDirMode value,
//	bool commit /*= false*/)
//{
//	int intValue = m_partsListsDirMode;
//	
//	setSetting(intValue, value, PARTS_LISTS_DIR_MODE_KEY, commit);
//	m_partsListsDirMode = (DefaultDirMode)intValue;
//}
//
//void LDPreferences::setPartsListsDir(const char *value, bool commit)
//{
//	setSetting(m_partsListsDir, value, PARTS_LISTS_DIR_KEY, commit);
//}

// LDraw settings
void LDPreferences::setLDrawDir(const char *value, bool commit)
{
	setSetting(m_ldrawDir, value, LDRAWDIR_KEY, commit);
}

void LDPreferences::setExtraDirs(
	const StringVector &value,
	bool commit)
{
	std::string dirKey = std::string(EXTRA_SEARCH_DIRS_KEY) + "/Dir";
	setSetting(m_extraDirs, value, dirKey.c_str(), commit, true, 3, 1);
}

// Geometry settings
void LDPreferences::setUseSeams(bool value, bool commit, bool apply)
{
	setSetting(m_useSeams, value, SEAMS_KEY, commit);
	if (apply && m_modelViewer != NULL)
	{
		if (m_useSeams)
		{
			m_modelViewer->setSeamWidth(m_seamWidth / 100.0f);
		}
		else
		{
			m_modelViewer->setSeamWidth(0.0f);
		}
	}
}

void LDPreferences::setSeamWidth(int value, bool commit)
{
	setSetting(m_seamWidth, value, SEAM_WIDTH_KEY, commit);
}

void LDPreferences::setBoundingBoxesOnly(bool value, bool commit, bool apply)
{
	setSetting(m_boundingBoxesOnly, value, BOUNDING_BOXES_ONLY_KEY, commit);
	if (apply && m_modelViewer != NULL)
	{
		m_modelViewer->setBoundingBoxesOnly(m_boundingBoxesOnly);
	}
}

void LDPreferences::setDrawWireframe(bool value, bool commit, bool apply)
{
	setSetting(m_drawWireframe, value, WIREFRAME_KEY, commit);
	if (apply && m_modelViewer != NULL)
	{
		m_modelViewer->setDrawWireframe(m_drawWireframe);
	}
}

void LDPreferences::setUseWireframeFog(bool value, bool commit, bool apply)
{
	setSetting(m_useWireframeFog, value, WIREFRAME_FOG_KEY, commit);
	if (apply && m_modelViewer != NULL)
	{
		m_modelViewer->setUseWireframeFog(m_useWireframeFog);
	}
}

void LDPreferences::setRemoveHiddenLines(bool value, bool commit, bool apply)
{
	setSetting(m_removeHiddenLines, value, REMOVE_HIDDEN_LINES_KEY, commit);
	if (apply && m_modelViewer != NULL)
	{
		m_modelViewer->setRemoveHiddenLines(m_removeHiddenLines);
	}
}

void LDPreferences::setWireframeThickness(int value, bool commit)
{
	setSetting(m_wireframeThickness, value, WIREFRAME_THICKNESS_KEY, commit);
}

void LDPreferences::setBfc(bool value, bool commit, bool apply)
{
	setSetting(m_bfc, value, BFC_KEY, commit);
	if (apply && m_modelViewer != NULL)
	{
		m_modelViewer->setBfc(m_bfc);
	}
}

void LDPreferences::setRedBackFaces(bool value, bool commit, bool apply)
{
	setSetting(m_redBackFaces, value, RED_BACK_FACES_KEY, commit);
	if (apply && m_modelViewer != NULL)
	{
		m_modelViewer->setRedBackFaces(m_redBackFaces);
	}
}

void LDPreferences::setGreenFrontFaces(bool value, bool commit, bool apply)
{
	setSetting(m_greenFrontFaces, value, GREEN_FRONT_FACES_KEY, commit);
	if (apply && m_modelViewer != NULL)
	{
		m_modelViewer->setGreenFrontFaces(m_greenFrontFaces);
	}
}

void LDPreferences::setBlueNeutralFaces(bool value, bool commit, bool apply)
{
	setSetting(m_blueNeutralFaces, value, BLUE_NEUTRAL_FACES_KEY, commit);
	if (apply && m_modelViewer != NULL)
	{
		m_modelViewer->setBlueNeutralFaces(m_blueNeutralFaces);
	}
}

void LDPreferences::setShowHighlightLines(bool value, bool commit, bool apply)
{
	setSetting(m_showHighlightLines, value, SHOW_HIGHLIGHT_LINES_KEY, commit);
	if (apply && m_modelViewer != NULL)
	{
		m_modelViewer->setShowsHighlightLines(m_showHighlightLines);
	}
}

void LDPreferences::setDrawConditionalHighlights(bool value, bool commit,
												 bool apply)
{
	setSetting(m_drawConditionalHighlights, value, CONDITIONAL_HIGHLIGHTS_KEY,
		commit);
	if (apply && m_modelViewer != NULL)
	{
		m_modelViewer->setDrawConditionalHighlights(
			m_drawConditionalHighlights);
	}
}

void LDPreferences::setShowAllConditionalLines(
	bool value,
	bool commit,
	bool apply)
{
	setSetting(m_showAllConditionalLines, value, SHOW_ALL_TYPE5_KEY, commit);
	if (apply && m_modelViewer != NULL)
	{
		m_modelViewer->setShowAllConditionalLines(m_showAllConditionalLines);
	}
}

void LDPreferences::setShowConditionalControlPoints(
	bool value,
	bool commit,
	bool apply)
{
	setSetting(m_showConditionalControlPoints, value,
		SHOW_TYPE5_CONTROL_POINTS_KEY, commit);
	if (apply && m_modelViewer != NULL)
	{
		m_modelViewer->setShowConditionalControlPoints(
			m_showConditionalControlPoints);
	}
}

void LDPreferences::setEdgesOnly(bool value, bool commit, bool apply)
{
	setSetting(m_edgesOnly, value, EDGES_ONLY_KEY, commit);
	if (apply && m_modelViewer != NULL)
	{
		m_modelViewer->setEdgesOnly(m_edgesOnly);
	}
}

void LDPreferences::setUsePolygonOffset(bool value, bool commit, bool apply)
{
	setSetting(m_usePolygonOffset, value, POLYGON_OFFSET_KEY, commit);
	if (apply && m_modelViewer != NULL)
	{
		m_modelViewer->setUsePolygonOffset(m_usePolygonOffset);
	}
}

void LDPreferences::setBlackHighlights(bool value, bool commit, bool apply)
{
	setSetting(m_blackHighlights, value, BLACK_HIGHLIGHTS_KEY, commit);
	if (apply && m_modelViewer != NULL)
	{
		m_modelViewer->setBlackHighlights(m_blackHighlights);
	}
}

void LDPreferences::setEdgeThickness(int value, bool commit)
{
	setSetting(m_edgeThickness, value, EDGE_THICKNESS_KEY, commit);
}


// Effects settings
void LDPreferences::setUseLighting(bool value, bool commit, bool apply)
{
	setSetting(m_useLighting, value, LIGHTING_KEY, commit);
	if (apply && m_modelViewer != NULL)
	{
		m_modelViewer->setUseLighting(m_useLighting);
	}
}

void LDPreferences::setQualityLighting(bool value, bool commit, bool apply)
{
	setSetting(m_qualityLighting, value, QUALITY_LIGHTING_KEY, commit);
	if (apply && m_modelViewer != NULL)
	{
		m_modelViewer->setQualityLighting(m_qualityLighting);
	}
}

void LDPreferences::setSubduedLighting(bool value, bool commit, bool apply)
{
	setSetting(m_subduedLighting, value, SUBDUED_LIGHTING_KEY, commit);
	if (apply && m_modelViewer != NULL)
	{
		m_modelViewer->setSubduedLighting(m_subduedLighting);
	}
}

void LDPreferences::setUseSpecular(bool value, bool commit, bool apply)
{
	setSetting(m_useSpecular, value, SPECULAR_KEY, commit);
	if (apply && m_modelViewer != NULL)
	{
		m_modelViewer->setUsesSpecular(m_useSpecular);
	}
}

void LDPreferences::setOneLight(bool value, bool commit, bool apply)
{
	setSetting(m_oneLight, value, ONE_LIGHT_KEY, commit);
	if (apply && m_modelViewer != NULL)
	{
		m_modelViewer->setOneLight(m_oneLight);
	}
}

void LDPreferences::setDrawLightDats(bool value, bool commit, bool apply)
{
	setSetting(m_drawLightDats, value, DRAW_LIGHT_DATS_KEY, commit);
	if (apply && m_modelViewer != NULL)
	{
		m_modelViewer->setDrawLightDats(m_drawLightDats);
	}
}

void LDPreferences::setOptionalStandardLight(bool value, bool commit,
	bool apply)
{
	setSetting(m_optionalStandardLight, value, OPTIONAL_STANDARD_LIGHT_KEY,
		commit);
	if (apply && m_modelViewer != NULL)
	{
		m_modelViewer->setOptionalStandardLight(m_optionalStandardLight);
	}
}

void LDPreferences::setNoLightGeom(bool value, bool commit, bool apply)
{
	setSetting(m_noLightGeom, value, NO_LIGHT_GEOM_KEY, commit);
	if (apply && m_modelViewer != NULL)
	{
		m_modelViewer->setNoLightGeom(m_noLightGeom);
	}
}

void LDPreferences::setLightDirection(LightDirection value, bool commit,
									  bool apply)
{
	TCVector lightVector;

	switch (value)
	{
	case UpperLeft:
		lightVector = TCVector(-1.0f, 1.0f, 1.0f);
		break;
	case UpperMiddle:
		lightVector = TCVector(0.0f, 1.0f, 1.0f);
		break;
	case UpperRight:
		lightVector = TCVector(1.0f, 1.0f, 1.0f);
		break;
	case MiddleLeft:
		lightVector = TCVector(-1.0f, 0.0f, 1.0f);
		break;
	case MiddleRight:
		lightVector = TCVector(1.0f, 0.0f, 1.0f);
		break;
	case LowerLeft:
		lightVector = TCVector(-1.0f, -1.0f, 1.0f);
		break;
	case LowerMiddle:
		lightVector = TCVector(0.0f, -1.0f, 1.0f);
		break;
	case LowerRight:
		lightVector = TCVector(1.0f, -1.0f, 1.0f);
		break;
	default:
		lightVector = TCVector(0.0f, 0.0f, 1.0f);
		break;
	}
	setLightVector(lightVector, commit, apply);
}

void LDPreferences::setLightVector(const TCVector &value, bool commit,
								   bool apply)
{
	if (setSetting(m_lightVector, value, LIGHT_VECTOR_KEY, commit))
	{
		TCAlertManager::sendAlert(lightVectorChangedAlertClass(), this);
	}
	if (apply && m_modelViewer != NULL)
	{
		m_modelViewer->setLightVector(m_lightVector);
	}
}

void LDPreferences::setStereoMode(LDVStereoMode value, bool commit)
{
	int temp = (int)m_stereoMode;

	setSetting(temp, value, STEREO_MODE_KEY, commit);
	m_stereoMode = (LDVStereoMode)temp;
}

void LDPreferences::setStereoEyeSpacing(int value, bool commit)
{
	setSetting(m_stereoEyeSpacing, value, STEREO_SPACING_KEY, commit);
}

void LDPreferences::setCutawayMode(
	LDVCutawayMode value,
	bool commit,
	bool apply)
{
	int temp = (int)m_cutawayMode;

	setSetting(temp, value, CUTAWAY_MODE_KEY, commit);
	m_cutawayMode = (LDVCutawayMode)temp;
	if (apply && m_modelViewer != NULL)
	{
		m_modelViewer->setCutawayMode(m_cutawayMode);
	}
}

void LDPreferences::setCutawayAlpha(int value, bool commit)
{
	setSetting(m_cutawayAlpha, value, CUTAWAY_ALPHA_KEY, commit);
}

void LDPreferences::setCutawayThickness(int value, bool commit)
{
	setSetting(m_cutawayThickness, value, CUTAWAY_THICKNESS_KEY, commit);
}

void LDPreferences::setSortTransparent(bool value, bool commit)
{
	setSetting(m_sortTransparent, value, SORT_KEY, commit);
}

void LDPreferences::setPerformSmoothing(bool value, bool commit, bool apply)
{
	setSetting(m_performSmoothing, value, PERFORM_SMOOTHING_KEY, commit);
	if (apply && m_modelViewer != NULL)
	{
		m_modelViewer->setPerformSmoothing(m_performSmoothing);
	}
}

void LDPreferences::setUseStipple(bool value, bool commit)
{
	setSetting(m_useStipple, value, STIPPLE_KEY, commit);
}

void LDPreferences::setUseFlatShading(bool value, bool commit, bool apply)
{
	setSetting(m_useFlatShading, value, FLAT_SHADING_KEY, commit);
	if (apply && m_modelViewer != NULL)
	{
		m_modelViewer->setUsesFlatShading(m_useFlatShading);
	}
}

void LDPreferences::setObi(bool value, bool commit)
{
	setSetting(m_obi, value, OBI_KEY, commit);
}


// Primitives settings
void LDPreferences::setAllowPrimitiveSubstitution(bool value, bool commit,
												  bool apply)
{
	setSetting(m_allowPrimitiveSubstitution, value, PRIMITIVE_SUBSTITUTION_KEY,
		commit);
	if (apply && m_modelViewer != NULL)
	{
		m_modelViewer->setAllowPrimitiveSubstitution(
			m_allowPrimitiveSubstitution);
	}
}

void LDPreferences::setTextureStuds(bool value, bool commit, bool apply)
{
	setSetting(m_textureStuds, value, TEXTURE_STUDS_KEY, commit);
	if (apply && m_modelViewer != NULL)
	{
		m_modelViewer->setTextureStuds(m_textureStuds);
	}
}

void LDPreferences::setTextureFilterType(int value, bool commit)
{
	setSetting(m_textureFilterType, value, TEXTURE_FILTER_TYPE_KEY, commit);
}

void LDPreferences::setAnisoLevel(TCFloat value, bool commit)
{
	setSetting(m_anisoLevel, value, ANISO_LEVEL_KEY, commit);
}

void LDPreferences::setCurveQuality(int value, bool commit)
{
	setSetting(m_curveQuality, value, CURVE_QUALITY_KEY, commit);
}

void LDPreferences::setQualityStuds(bool value, bool commit, bool apply)
{
	setSetting(m_qualityStuds, value, QUALITY_STUDS_KEY, commit);
	if (apply && m_modelViewer != NULL)
	{
		m_modelViewer->setQualityStuds(m_qualityStuds);
	}
}

void LDPreferences::setHiResPrimitives(bool value, bool commit)
{
	setSetting(m_hiResPrimitives, value, HI_RES_PRIMITIVES_KEY, commit);
}

void LDPreferences::setTexmaps(bool value, bool commit, bool apply)
{
	setSetting(m_texmaps, value, TEXMAPS_KEY, commit);
	if (apply && m_modelViewer != NULL)
	{
		m_modelViewer->setTexmaps(m_texmaps);
	}
}

void LDPreferences::setTexturesAfterTransparent(bool value, bool commit, bool apply)
{
	setSetting(m_texturesAfterTransparent, value,
		TEXTURES_AFTER_TRANSPARENT_KEY, commit);
	if (apply && m_modelViewer != NULL)
	{
		m_modelViewer->setTexturesAfterTransparent(m_texturesAfterTransparent);
	}
}

void LDPreferences::setTextureOffsetFactor(TCFloat value, bool commit, bool apply)
{
	setSetting(m_textureOffsetFactor, value, TEXTURE_OFFSET_FACTOR_KEY, commit);
	if (apply && m_modelViewer != NULL)
	{
		m_modelViewer->setTextureOffsetFactor(value);
	}
}


// Update settings
void LDPreferences::setProxyType(int value, bool commit)
{
	setSetting(m_proxyType, value, PROXY_TYPE_KEY, commit);
}

void LDPreferences::setProxyServer(const char *value, bool commit)
{
	setSetting(m_proxyServer, value, PROXY_SERVER_KEY, commit);
}

void LDPreferences::setProxyPort(int value, bool commit)
{
	setSetting(m_proxyPort, value, PROXY_PORT_KEY, commit);
}

void LDPreferences::setCheckPartTracker(bool value, bool commit)
{
	setSetting(m_checkPartTracker, value, CHECK_PART_TRACKER_KEY, commit);
}

void LDPreferences::setMissingPartWait(int value, bool commit)
{
	setSetting(m_missingPartWait, value, CHECK_PART_WAIT_KEY, commit);
}

void LDPreferences::setUpdatedPartWait(int value, bool commit)
{
	setSetting(m_updatedPartWait, value, CHECK_PART_UPDATE_WAIT_KEY, commit);
}

void LDPreferences::setInvShowModel(bool value, bool commit)
{
	setSetting(m_invShowModel, value, INV_SHOW_MODEL_KEY, commit);
}

void LDPreferences::setInvOverwriteSnapshot(bool value, bool commit)
{
	setSetting(m_invOverwriteSnapshot, value, INV_OVERWRITE_SNAPSHOT_KEY,
		commit);
}

void LDPreferences::setInvExternalCss(bool value, bool commit)
{
	setSetting(m_invExternalCss, value, INV_EXTERNAL_CSS_KEY, commit);
}

void LDPreferences::setInvPartImages(bool value, bool commit)
{
	setSetting(m_invPartImages, value, INV_PART_IMAGES_KEY, commit);
}

void LDPreferences::setInvShowFile(bool value, bool commit)
{
	setSetting(m_invShowFile, value, INV_SHOW_FILE_KEY, commit);
}

void LDPreferences::setInvShowTotal(bool value, bool commit)
{
	setSetting(m_invShowTotal, value, INV_SHOW_TOTAL_KEY, commit);
}

void LDPreferences::setInvColumnOrder(const LongVector &value, bool commit)
{
	setSetting(m_invColumnOrder, value, INV_COLUMN_ORDER_KEY, commit);
}

//void LDPreferences::setInvLastSavePath(const char *value, bool commit)
//{
//	setSetting(m_invLastSavePath, value, INV_LAST_SAVE_PATH_KEY, commit, true);
//}

void LDPreferences::setDefaultZoom(TCFloat value, bool commit)
{
	setSetting(m_defaultZoom, value, DEFAULT_ZOOM_KEY, commit);
}

void LDPreferences::saveDefaultView(void)
{
	if (m_modelViewer != NULL)
	{
		TCFloat matrix[16];
		TCFloat rotationMatrix[16];
		TCFloat otherMatrix[16] = {1,0,0,0,0,-1,0,0,0,0,-1,0,0,0,0,1};
		char matrixString[1024];
		TCFloat latitude = m_modelViewer->getExamineLatitude();
		TCFloat longitude = m_modelViewer->getExamineLongitude();

		memcpy(rotationMatrix, m_modelViewer->getRotationMatrix(),
			sizeof(rotationMatrix));
		TCVector::multMatrix(otherMatrix, rotationMatrix, matrix);
		matrix[12] = 0.0f;
		matrix[13] = 0.0f;
		matrix[14] = 0.0f;
		sprintf(matrixString, "%.6g,%.6g,%.6g,%.6g,%.6g,%.6g,%.6g,%.6g,%.6g",
			matrix[0], matrix[4], matrix[8],
			matrix[1], matrix[5], matrix[9],
			matrix[2], matrix[6], matrix[10]);
		TCUserDefaults::setStringForKey(matrixString, DEFAULT_MATRIX_KEY);
		TCUserDefaults::setFloatForKey(latitude, DEFAULT_LATITUDE_KEY);
		TCUserDefaults::setFloatForKey(longitude, DEFAULT_LONGITUDE_KEY);
		m_modelViewer->setDefaultRotationMatrix(matrix);
		m_modelViewer->setDefaultLatLong(latitude, longitude);
	}
}

void LDPreferences::resetDefaultView(void)
{
	TCUserDefaults::removeValue(DEFAULT_MATRIX_KEY);
	TCUserDefaults::removeValue(DEFAULT_LATITUDE_KEY);
	TCUserDefaults::removeValue(DEFAULT_LONGITUDE_KEY);
	if (m_modelViewer != NULL)
	{
		m_modelViewer->setDefaultRotationMatrix(NULL);
		m_modelViewer->setDefaultLatLong(30.0f, 45.0f);
	}
}

void LDPreferences::getBackgroundColor(int &r, int &g, int &b)
{
	getRGB(m_backgroundColor, r, g, b);
}

void LDPreferences::getDefaultColor(int &r, int &g, int &b)
{
	getRGB(m_defaultColor, r, g, b);
}

void LDPreferences::getCustomColor(int index, int &r, int &g, int &b)
{
	getRGB(m_customColors[index], r, g, b);
}

LDPreferences::LightDirection LDPreferences::getLightDirection(void)
{
	if (m_lightVector == TCVector(-1.0f, 1.0f, 1.0f))
	{
		return UpperLeft;
	}
	else if (m_lightVector == TCVector(0.0f, 1.0f, 1.0f))
	{
		return UpperMiddle;
	}
	else if (m_lightVector == TCVector(1.0f, 1.0f, 1.0f))
	{
		return UpperRight;
	}
	else if (m_lightVector == TCVector(-1.0f, 0.0f, 1.0f))
	{
		return MiddleLeft;
	}
	else if (m_lightVector == TCVector(0.0f, 0.0f, 1.0f))
	{
		return Middle;
	}
	else if (m_lightVector == TCVector(1.0f, 0.0f, 1.0f))
	{
		return MiddleRight;
	}
	else if (m_lightVector == TCVector(-1.0f, -1.0f, 1.0f))
	{
		return LowerLeft;
	}
	else if (m_lightVector == TCVector(0.0f, -1.0f, 1.0f))
	{
		return LowerMiddle;
	}
	else if (m_lightVector == TCVector(1.0f, -1.0f, 1.0f))
	{
		return LowerRight;
	}
	return CustomDirection;
}

LDPreferences::DefaultDirMode LDPreferences::getSaveDirMode(SaveOp op) const
{
	DirModeMap::const_iterator it = m_saveDirModes.find(op);

	if (it != m_saveDirModes.end())
	{
		return it->second;
	}
	else
	{
		return DDMUnknown;
	}
}

std::string LDPreferences::getSaveDir(SaveOp op) const
{
	SaveOpStringMap::const_iterator it = m_saveDirs.find(op);

	if (it != m_saveDirs.end())
	{
		return it->second;
	}
	else
	{
		return "";
	}
}

std::string LDPreferences::getLastSaveDir(SaveOp op) const
{
	SaveOpStringMap::const_iterator it = m_lastSaveDirs.find(op);

	if (it != m_lastSaveDirs.end())
	{
		return it->second;
	}
	else
	{
		return "";
	}
}

std::string LDPreferences::getDefaultSaveDir(
	SaveOp op,
	const std::string &modelFilename)
{
	switch (getSaveDirMode(op))
	{
	case DDMLastDir:
		return getLastSaveDir(op);
	case DDMSpecificDir:
		return getSaveDir(op);
	default:
		{
			char *temp = directoryFromPath(modelFilename.c_str());
			std::string modelDir(temp);

			delete[] temp;
			return modelDir;
		}
	}
}

void LDPreferences::loadSaveDir(SaveOp op)
{
	DefaultDirMode &mode = m_saveDirModes[op];
	std::string &dir = m_saveDirs[op];
	std::string &lastDir = m_lastSaveDirs[op];

	mode = (DefaultDirMode)getIntSetting(m_saveDirModeKeys[op].c_str(), mode);
	dir = getStringSetting(m_saveDirKeys[op].c_str(), dir.c_str(), true);
	lastDir = getStringSetting(m_lastSaveDirKeys[op].c_str(), lastDir.c_str(),
		true);
}

void LDPreferences::commitSaveDir(SaveOp op)
{
	setSaveDirMode(op, m_saveDirModes[op], true);
	setSaveDir(op, m_saveDirs[op].c_str(), true);
	setLastSaveDir(op, m_lastSaveDirs[op].c_str(), true);
}

void LDPreferences::setSaveDirDefault(SaveOp op, DefaultDirMode mode)
{
	setSaveDirMode(op, mode);
	setSaveDir(op, "");
	setLastSaveDir(op, "");
}
