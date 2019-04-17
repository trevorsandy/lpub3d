#ifndef __LDVHTMLINVENTORY_H__
#define __LDVHTMLINVENTORY_H__

#include <TCFoundation/TCObject.h>
#include <TCFoundation/TCStlIncludes.h>
#include <LDLoader/LDLPalette.h>
#include <stdio.h>

class LDPartsList;
class LDPartCount;
class LDLPalette;
class LDPreferences;
class LDrawModelViewer;
class LDViewPoint;

typedef enum
{
    LDVPLCFirst = 1,
    LDVPLCPart = LDVPLCFirst,
    LDVPLCDescription,
    LDVPLCColor,
    LDVPLCElement,
    LDVPLCQuantity,
    LDVPLCLast = LDVPLCQuantity
} LDVPartListColumn;

typedef std::vector<LDVPartListColumn> LDVPartListColumnVector;
typedef std::map<LDVPartListColumn, bool> LDVPartListColumnBoolMap;

class LDVHtmlInventory : public TCObject
{
public:
	LDVHtmlInventory(void);

	void setShowModelFlag(bool value);
	bool getShowModelFlag(void) { return m_showModel; }
	void setExternalCssFlag(bool value);
	bool getOverwriteSnapshotFlag(void) const { return m_overwriteSnapshot; }
	void setOverwriteSnapshotFlag(bool value);
	bool getExternalCssFlag(void) { return m_externalCss; }
	void setPartImagesFlag(bool value);
	bool getPartImagesFlag(void) { return m_partImages; }
	void setShowFileFlag(bool value);
	bool getShowFileFlag(void) { return m_showFile; }
	void setShowTotalFlag(bool value);
	bool getShowTotalFlag(void) { return m_showTotal; }
    void setPartListKey(const char *value) { m_partListKey = value; }
    void setUserDefinedSnapshot(const char *value) { m_userDefinedSnapshot = value; }
    const char *getUserDefinedSnapshot(void) { return m_userDefinedSnapshot.c_str(); }

	const LDVPartListColumnVector &getColumnOrder(void) const
	{
		return m_columnOrder;
	}
	void setColumnOrder(const LDVPartListColumnVector &value); 
	const char *getLastSavePath(void) { return m_lastSavePath.c_str(); }
	const char *getSnapshotPath(void) const;
	bool isColumnEnabled(LDVPartListColumn column);
	bool isSnapshotNeeded(void) const;
	std::string defaultFilename(const char *modelFilename);

	bool generateHtml(const char *filename, LDPartsList *partsList,
		const char *modelName);

	void prepForSnapshot(LDrawModelViewer *modelViewer);
	void restoreAfterSnapshot(LDrawModelViewer *modelViewer);

	static const char *getColumnName(LDVPartListColumn column);
	static CUCSTR getColumnNameUC(LDVPartListColumn column);
	
	void setGeneratePdfFlag(bool value);
    void setLookupSiteFlag(int value);
    void setElementSourceFlag(int value);
    void setSnapshotWidthFlag(int value);
    void setSnapshotHeightFlag(int value);

    bool getGeneratePdfFlag(void);
     int getLookupSiteFlag(void);
     int getElementSourceFlag(void);
     int getSnapshotWidthFlag(void);
     int getSnapshotHeightFlag(void);
	 
	void loadOtherSettings(void);

    enum LookUp { Bricklink, Peeron };
    enum ElementSrc { BL, LEGO };

    bool getLookupSite(void) { return m_lookupSite; }

protected:
	virtual ~LDVHtmlInventory(void);
	virtual void dealloc(void);
	void writeHeader(FILE *file);
	void writeFooter(FILE *file);
    void writeTableHeader(FILE *file, int totalParts,
                          int invalidElements, int uniqueElements,
                          int uniqueParts, int uniqueColors);
	void writeTableFooter(FILE *file);
	void writePartRow(FILE *file, const LDPartCount &partCount,
		LDLPalette *palette, const LDLColorInfo &colorInfo, int colorNumber);
	bool writeExternalCss(void);
	FILE *safeOpenCssFile(const std::string &cssFilename, bool &match);
	void writePartHeaderCell(FILE *file);
	void writeHeaderCell(FILE *file, LDVPartListColumn column, int colSpan);
	void writeHeaderCell(FILE *file, LDVPartListColumn column);
	void writePartCell(FILE *file, const LDPartCount &partCount,
		LDLPalette *palette, const LDLColorInfo &colorInfo, int colorNumber);
	void writeDescriptionCell(FILE *file, const LDPartCount &partCount);
	void writeColorCell(FILE *file, LDLPalette *palette,
		const LDLColorInfo &colorInfo, int colorNumber);
    void writeElementCell(FILE *file, const LDPartCount &partCount,
		int colorNumber);
    void writeQuantityCell(FILE *file, const LDPartCount &partCount,
        int colorNumber);
	void writeCell(FILE *file, LDVPartListColumn column,
		const LDPartCount &partCount, LDLPalette *palette,
		const LDLColorInfo &colorInfo, int colorNumber);
	void populateColumnMap(void);
	std::string getSnapshotFilename(void) const;

    void setOtherSetting(bool &setting, bool value, const char *key, bool commit);
    void setOtherSetting(int &setting, int value, const char *key, bool commit);
    bool getBoolSetting(const char *key, bool defaultValue = false);
     int getIntSetting(const char *key, int defaultValue);
	 
	std::string m_modelName;
	LDPreferences *m_prefs;
    LDViewPoint *m_viewPoint;

    bool m_generatePdf;
    int  m_lookupSite;
    int  m_elementSource;
    int  m_lookupDefault;
    int  m_elementDefault;
    int  m_snapshotWidth;
    int  m_snapshotHeight;
	
	bool m_showModel;
	bool m_overwriteSnapshot;
	bool m_externalCss;
	bool m_partImages;
	bool m_showFile;
	bool m_showTotal;
	std::string m_lastSavePath;
	std::string m_lastFilename;
    std::string m_imageFilename;
    std::string m_partListKey;
    std::string m_userDefinedSnapshot;
	int m_columns;
	LDVPartListColumnVector m_columnOrder;
	LDVPartListColumnBoolMap m_columnMap;
	mutable std::string m_snapshotPath;

	static const char *sm_style;
    static const char *sm_script;
	static const char *sm_cssHeader;
	static const char *sm_cssFilename;
};

#endif // __LDVHTMLINVENTORY_H__
