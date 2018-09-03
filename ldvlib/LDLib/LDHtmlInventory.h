#ifndef __LDHTMLINVENTORY_H__
#define __LDHTMLINVENTORY_H__

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
	LDPLCFirst = 1,
	LDPLCPart = LDPLCFirst,
	LDPLCDescription,
	LDPLCColor,
	LDPLCQuantity,
	LDPLCLast = LDPLCQuantity
} LDPartListColumn;

typedef std::vector<LDPartListColumn> LDPartListColumnVector;
typedef std::map<LDPartListColumn, bool> LDPartListColumnBoolMap;

class LDHtmlInventory : public TCObject
{
public:
	LDHtmlInventory(void);

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
	const LDPartListColumnVector &getColumnOrder(void) const
	{
		return m_columnOrder;
	}
	void setColumnOrder(const LDPartListColumnVector &value); 
	const char *getLastSavePath(void) { return m_lastSavePath.c_str(); }
	const char *getSnapshotPath(void) const;
	bool isColumnEnabled(LDPartListColumn column);
	bool isSnapshotNeeded(void) const;
	std::string defaultFilename(const char *modelFilename);

	bool generateHtml(const char *filename, LDPartsList *partsList,
		const char *modelName);

	void prepForSnapshot(LDrawModelViewer *modelViewer);
	void restoreAfterSnapshot(LDrawModelViewer *modelViewer);

	static const char *getColumnName(LDPartListColumn column);
	static CUCSTR getColumnNameUC(LDPartListColumn column);
protected:
	virtual ~LDHtmlInventory(void);
	virtual void dealloc(void);
	void writeHeader(FILE *file);
	void writeFooter(FILE *file);
	void writeTableHeader(FILE *file, int totalParts);
	void writeTableFooter(FILE *file);
	void writePartRow(FILE *file, const LDPartCount &partCount,
		LDLPalette *palette, const LDLColorInfo &colorInfo, int colorNumber);
	bool writeExternalCss(void);
	FILE *safeOpenCssFile(const std::string &cssFilename, bool &match);
	void writePartHeaderCell(FILE *file);
	void writeHeaderCell(FILE *file, LDPartListColumn column, int colSpan);
	void writeHeaderCell(FILE *file, LDPartListColumn column);
	void writePartCell(FILE *file, const LDPartCount &partCount,
		LDLPalette *palette, const LDLColorInfo &colorInfo, int colorNumber);
	void writeDescriptionCell(FILE *file, const LDPartCount &partCount);
	void writeColorCell(FILE *file, LDLPalette *palette,
		const LDLColorInfo &colorInfo, int colorNumber);
	void writeQuantityCell(FILE *file, const LDPartCount &partCount,
		int colorNumber);
	void writeCell(FILE *file, LDPartListColumn column,
		const LDPartCount &partCount, LDLPalette *palette,
		const LDLColorInfo &colorInfo, int colorNumber);
	void populateColumnMap(void);
	std::string getSnapshotFilename(void) const;

	std::string m_modelName;
	LDPreferences *m_prefs;

	bool m_showModel;
	bool m_overwriteSnapshot;
	bool m_externalCss;
	bool m_partImages;
	bool m_showFile;
	bool m_showTotal;
	std::string m_lastSavePath;
	std::string m_lastFilename;
	int m_columns;
	LDPartListColumnVector m_columnOrder;
	LDPartListColumnBoolMap m_columnMap;
	LDViewPoint *m_viewPoint;
	mutable std::string m_snapshotPath;

	static const char *sm_style;
	static const char *sm_cssHeader;
	static const char *sm_cssFilename;
};

#endif // __LDHTMLINVENTORY_H__
