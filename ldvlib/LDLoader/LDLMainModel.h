#ifndef __LDLMAINMODEL_H__
#define __LDLMAINMODEL_H__

#include <LDLoader/LDLModel.h>
#include <TCFoundation/TCStlIncludes.h>

class LDLPalette;

typedef std::map<std::string, bool> StringBoolMap;
typedef std::vector<LDLModel *> LDLModelVector;

class LDLMainModel : public LDLModel
{
public:
	LDLMainModel(void);
	bool load(const char *filename);
	virtual TCDictionary* getLoadedModels(void);
	void print(void);
	virtual int getEdgeColorNumber(int colorNumber);
	virtual void getRGBA(int colorNumber, int& r, int& g, int& b, int& a);
	virtual bool hasSpecular(int colorNumber);
	virtual bool hasShininess(int colorNumber);
	virtual void getSpecular(int colorNumber, float *specular);
	virtual void getShininess(int colorNumber, float &shininess);
	virtual bool colorNumberIsTransparent(int colorNumber);
	virtual LDLPalette *getPalette(void) { return m_mainPalette; }
	virtual const LDLPalette *getPalette(void) const { return m_mainPalette; }
	virtual void setExtraSearchDirs(TCStringArray *value);
	TCStringArray *getExtraSearchDirs(void) { return m_extraSearchDirs; }
	virtual bool isMainModel(void) const { return true; }

	virtual void setMainModel(LDLMainModel *value) { m_mainModel = value; }

	void setLDConfig(const std::string& value) { m_ldConfig = value; }
	const std::string& getLDConfig(void) const { return m_ldConfig; }
	// Flags
	void setLowResStuds(bool value) { m_mainFlags.lowResStuds = value; }
	bool getLowResStuds(void) const { return m_mainFlags.lowResStuds; }
	void setBlackEdgeLines(bool value) { m_mainFlags.blackEdgeLines = value; }
	bool getBlackEdgeLines(void) const { return m_mainFlags.blackEdgeLines; }
	void setProcessLDConfig(bool value) { m_mainFlags.processLDConfig = value; }
	bool getProcessLDConfig(void) const
	{
		return m_mainFlags.processLDConfig != false;
	}
	void setSkipValidation(bool value) { m_mainFlags.skipValidation = value; }
	bool getSkipValidation(void) const
	{
		return m_mainFlags.skipValidation != false;
	}
	void setBoundingBoxesOnly(bool value)
	{
		m_mainFlags.boundingBoxesOnly = value;
	}
	bool getBoundingBoxesOnly(void) const
	{
		return m_mainFlags.boundingBoxesOnly != false;
	}
	void setRandomColors(bool value) { m_mainFlags.randomColors = value; }
	bool getRandomColors(void) const {
		return m_mainFlags.randomColors != false;
	}
	void setForceHighlightColor(bool value)
	{
		m_mainFlags.forceHighlightColor = value;
	}
	bool getForceHighlightColor(void) const
	{
		return m_mainFlags.forceHighlightColor != false;
	}
	void setCheckPartTracker(bool value)
	{
		m_mainFlags.checkPartTracker = value;
	}
	bool getCheckPartTracker(void) const {
		return m_mainFlags.checkPartTracker != false;
	}
	void setHighlightColorNumber(int value)
	{
		m_highlightColorNumber = value;
	}
	int getHighlightColorNumber(void) const { return m_highlightColorNumber; }
	void setSeamWidth(float value);
	float getSeamWidth(void) const { return m_seamWidth; }
	virtual void cancelLoad(void) { m_mainFlags.loadCanceled = true; }
	virtual bool getLoadCanceled(void)
	{
		return m_mainFlags.loadCanceled != false;
	}
	virtual void setBBoxIgnoreUsed(bool value)
	{
		m_mainFlags.bboxIgnoreUsed = value;
	}
	virtual bool getBBoxIgnoreUsed(void) const
	{
		return m_mainFlags.bboxIgnoreUsed != false;
	}
	void setGreenFrontFaces(bool value)
	{
		m_mainFlags.greenFrontFaces = value;
	}
	bool getGreenFrontFaces(void) const
	{
		return m_mainFlags.greenFrontFaces != false;
	}
	void setRedBackFaces(bool value)
	{
		m_mainFlags.redBackFaces = value;
	}
	bool getRedBackFaces(void) const
	{
		return m_mainFlags.redBackFaces != false;
	}
	void setBlueNeutralFaces(bool value)
	{
		m_mainFlags.blueNeutralFaces = value;
	}
	bool getBlueNeutralFaces(void) const
	{
		return m_mainFlags.blueNeutralFaces != false;
	}
	void setTexmaps(bool value) { m_mainFlags.texmaps = value; }
	bool getTexmaps(void) const { return m_mainFlags.texmaps != false; }
	void setScanConditionalControlPoints(bool value)
	{
		m_mainFlags.scanConditionalControlPoints = value;
	}
	bool getScanConditionalControlPoints(void) const
	{
		return m_mainFlags.scanConditionalControlPoints != false;
	}
	void setHaveMpdTexmaps(void) { m_mainFlags.haveMpdTexmaps = true; }
	bool getHaveMpdTexmaps(void) const { return m_mainFlags.haveMpdTexmaps != false; }
	bool &ancestorCheck(const std::string &name) { return m_ancestorMap[name]; }
	void ancestorClear(const std::string &name) { m_ancestorMap[name] = false; }
	virtual TCObject *getAlertSender(void) { return m_alertSender; }
	virtual void setAlertSender(TCObject *value) { m_alertSender = value; }
	virtual void addMpdModel(LDLModel *model) { m_mpdModels.push_back(model); }
	virtual LDLModelVector &getMpdModels(void) { return m_mpdModels; }
	virtual void release(void);
	virtual bool transparencyIsDisabled(void) const;
private:
	//LDLMainModel(const LDLMainModel &other);
	virtual TCObject *copy(void) const;
protected:
	virtual void dealloc(void);
	virtual void processLDConfig(void);
	void ldrawDirNotFound(void);

	TCObject *m_alertSender;
	TCDictionary *m_loadedModels;
	LDLPalette *m_mainPalette;
	TCStringArray *m_extraSearchDirs;
	// This needs to not retain its children; hence, the std::vector, instead of
	// TCTypedObjectArray.
	LDLModelVector m_mpdModels;
	float m_seamWidth;
	int m_highlightColorNumber;
	std::string m_ldConfig;
	struct
	{
		// Public flags
		bool lowResStuds:1;
		bool blackEdgeLines:1;
		bool processLDConfig:1;
		bool skipValidation:1;
		bool boundingBoxesOnly:1;
		bool randomColors:1;
		bool forceHighlightColor:1;
		bool checkPartTracker:1;
		bool bboxIgnoreUsed:1;
		bool greenFrontFaces:1;
		bool redBackFaces:1;
		bool blueNeutralFaces:1;
		bool texmaps:1;
		bool scanConditionalControlPoints:1;
		bool haveMpdTexmaps:1;
		// Semi-public flags
		bool loadCanceled:1;
	} m_mainFlags;
	StringBoolMap m_ancestorMap;
};

#endif // __LDLMAINMODEL_H__
