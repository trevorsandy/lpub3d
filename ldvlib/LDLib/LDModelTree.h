#ifndef __LDMODELTREE_H__
#define __LDMODELTREE_H__

#include <TCFoundation/TCObject.h>
#include <TCFoundation/TCTypedObjectArray.h>
#include <TCFoundation/TCStlIncludes.h>
#include <LDLoader/LDLFileLine.h>

class LDModelTree;
class LDLModel;
class LDLModelLine;

typedef TCTypedObjectArray<LDModelTree> LDModelTreeArray;

class LDModelTree : public TCObject
{
public:
	LDModelTree(LDLModel *model = NULL);
	void scanModel(LDLModel *model, int defaultColor) const;
	const LDLModel *getModel(void) const { return m_model; }
	const LDModelTreeArray *getChildren(bool filter = true) const;
	bool hasChildren(bool filter = true) const;
	int getNumChildren(bool filter = true) const;
#ifndef TC_NO_UNICODE
	const ucstring &getTextUC(void) const { return m_text; }
	const std::string &getText(void) const;
#else // TC_NO_UNICODE
	const std::string &getTextUC(void) const { return m_text; }
	const std::string &getText(void) const { return m_text; }
#endif // TC_NO_UNICODE
	const std::string &getTreePath(void) const { return m_treePath; }
	const ucstring &getStatusText(void) const;
	LDLLineType getLineType(void) const { return m_lineType; }
	void setShowLineType(LDLLineType lineType, bool value);
	bool getShowLineType(LDLLineType lineType) const
	{
		return (m_activeLineTypes & (1 << lineType)) != 0;
	}
	bool getViewPopulated(void) const { return m_viewPopulated; }
	void setViewPopulated(bool value) { m_viewPopulated = value; }
	bool getBackgroundRGB(TCFloat &r, TCFloat &g, TCFloat &b) const;
	bool getBackgroundRGB(TCByte &r, TCByte &g, TCByte &b) const;
	bool getBackgroundRGB(int &r, int &g, int &b) const;
	bool getTextRGB(TCFloat &r, TCFloat &g, TCFloat &b) const;
	bool getTextRGB(TCByte &r, TCByte &g, TCByte &b) const;
	bool getTextRGB(int &r, int &g, int &b) const;
	std::string adjustHighlightPath(std::string path);
protected:
	bool getRGB(TCFloat l, TCFloat h, TCFloat &r, TCFloat &g, TCFloat &b) const;
	LDModelTree(TCULong activeLineTypes, TCULong allLineTypes);
	virtual ~LDModelTree(void);
	virtual void dealloc(void);
	void scanLine(LDLFileLine *fileLine, int defaultColor);
	void setModel(LDLModel *model);
	bool childFilterCheck(const LDModelTree *child) const;
	void clearFilteredChildren(void);
	std::string lineTypeKey(LDLLineType lineType) const;

	LDLModel *m_model;
	const LDLFileLine *m_fileLine;
	mutable LDModelTreeArray *m_children;
	mutable LDModelTreeArray *m_filteredChildren;
	ucstring m_text;
#ifndef TC_NO_UNICODE
	mutable std::string m_aText;
#endif // TC_NO_UNICODE
	std::string m_treePath;
	mutable ucstring m_statusText;
	LDLLineType m_lineType;
	bool m_replaced;
	int m_defaultColor;
	TCULong m_activeLineTypes;
	TCULong m_allLineTypes;
	bool m_viewPopulated;
};

#endif // __LDMODELTREE_H__
