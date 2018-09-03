#ifndef __LDMODELPARSER_H__
#define __LDMODELPARSER_H__

#include <TCFoundation/TCObject.h>
#include <TCFoundation/TCStlIncludes.h>
#include <LDLoader/LDLPrimitiveCheck.h>

#include <string.h>

class LDLMainModel;
class TREMainModel;
class TREModel;
class TRESubModel;
class LDLCommentLine;
class LDLActionLine;
class LDLShapeLine;
class LDLModelLine;
class LDLConditionalLineLine;
class LDrawModelViewer;
class TCVector;
class LDObiInfo;

typedef std::set<std::string> StringSet;

class LDModelParser : public LDLPrimitiveCheck
{
public:
	LDModelParser(LDrawModelViewer *modelViewer);
	virtual bool parseMainModel(LDLModel *mainLDLModel);
	virtual void release(void) { LDLPrimitiveCheck::release(); }
	TREMainModel *getMainTREModel(void) { return m_mainTREModel; }

protected:
	virtual ~LDModelParser(void);
	virtual void dealloc(void);
	void setSeamsFlag(bool value) { m_flags.seams = value; }
	void setDefaultColorSetFlag(bool value) { m_flags.defaultColorSet = value; }
	int getActiveColorNumber(LDLModelLine *modelLine, int activeColorNumber);
	bool getDefaultColorSetFlag(void)
	{
		return m_flags.defaultColorSet != false;
	}
	void setDefaultColorNumberSetFlag(bool value)
	{
		m_flags.defaultColorNumberSet = value;
	}
	bool getDefaultColorNumberSetFlag(void)
	{
		return m_flags.defaultColorNumberSet != false;
	}
	virtual void parseCommentLine(LDLCommentLine *commentLine,
		TREModel *treModel);
	virtual bool parseModel(LDLModel *ldlModel, TREModel *treModel, bool bfc,
		int activeColorNumber);
	virtual bool parseModel(LDLModelLine *modelLine, TREModel *treModel,
		bool bfc, int activeColorNumber);
	virtual void parseLine(LDLShapeLine *shapeLine, TREModel *treModel,
		int activeColorNumber);
	virtual void parseTriangle(LDLShapeLine *shapeLine, TREModel *treModel,
		bool bfc, bool invert, int activeColorNumber);
	virtual void parseQuad(LDLShapeLine *shapeLine, TREModel *treModel,
		bool bfc, bool invert, int activeColorNumber);
	virtual void parseConditionalLine(LDLConditionalLineLine *conditionalLine,
		TREModel *treModel, int activeColorNumber);
	virtual bool addSubModel(LDLModelLine *modelLine, TREModel *treParentModel,
		TREModel *treModel, bool invert, int activeColorNumber);
	virtual bool performPrimitiveSubstitution2(LDLModel *ldlModel,
		TREModel *treModel, int activeColorNumber, bool bfc);
	virtual bool substituteStud(int numSegments);
	virtual bool substituteStud(void);
	virtual bool substituteStu2(void);
	virtual bool substituteStu22(bool isA, bool bfc);
	virtual bool substituteStu23(bool isA, bool bfc);
	virtual bool substituteStu24(bool isA, bool bfc);
	virtual bool substituteTorusIO(bool inner, TCFloat fraction, int size,
		bool bfc, bool isMixed, bool is48 = false);
	virtual bool substituteTorusQ(TCFloat fraction, int size, bool bfc,
		bool isMixed, bool is48 = false);
	virtual bool substituteEighthSphere(bool bfc,
		bool is48 = false);
	virtual bool substituteCylinder(TCFloat fraction,
		bool bfc, bool is48 = false);
	virtual bool substituteSlopedCylinder(TCFloat fraction,
		bool bfc, bool is48 = false);
	virtual bool substituteSlopedCylinder2(TCFloat fraction,
		bool bfc, bool is48 = false);
	virtual bool substituteChrd(TCFloat fraction, bool bfc,
		bool is48 = false);
	virtual bool substituteDisc(TCFloat fraction, bool bfc,
		bool is48 = false);
	virtual bool substituteNotDisc(TCFloat fraction,
		bool bfc, bool is48 = false);
	virtual bool substituteTangent(TCFloat fraction,
		bool bfc, bool is48 = false);
	virtual bool substituteCircularEdge(TCFloat fraction,
		bool is48 = false);
	virtual bool substituteCone(TCFloat fraction, int size,
		bool bfc, bool is48 = false);
	virtual bool substituteRing(TCFloat fraction, int size,
		bool bfc, bool is48 = false, bool isOld = false);
	//virtual void finishPart(TREModel *treModel, TRESubModel *subModel = NULL);
	virtual bool shouldFlipWinding(bool invert, bool windingCCW);
	void setSeamWidth(TCFloat seamWidth);
	TCFloat getSeamWidth(void);
	void setDefaultRGB(TCByte r, TCByte g, TCByte b, bool transparent);
	void setDefaultColorNumber(int colorNumber);
	bool getFileIsPartFlag(void) const;
	bool getEdgeLinesFlag(void) const;
	bool getEdgesOnlyFlag(void) const;
	bool getLightingFlag(void) const;
	bool getTwoSidedLightingFlag(void) const;
	bool getBFCFlag(void) const;
	bool getAALinesFlag(void) const;
	bool getSortTransparentFlag(void) const;
	bool getStippleFlag(void) const;
	bool getWireframeFlag(void) const;
	bool getConditionalLinesFlag(void) const;
	bool getSmoothCurvesFlag(void) const;
	bool getShowAllConditionalFlag(void) const;
	bool getConditionalControlPointsFlag(void) const;
	bool getPolygonOffsetFlag(void) const;
	bool getStudLogoFlag(void) const;
	bool getRedBackFacesFlag(void) const;
	bool getGreenFrontFacesFlag(void) const;
	bool getBlueNeutralFacesFlag(void) const;
	bool getMultiThreadedFlag(void) const;
	void setFlattenPartsFlag(bool value) { m_flags.flattenParts = value; }
	bool getFlattenPartsFlag(void) { return m_flags.flattenParts != false; }
	bool getSeamsFlag(void) { return m_flags.seams != false; }
	bool getBoundingBoxesOnlyFlag(void) const
	{
		return m_flags.boundingBoxesOnly != false;
	}
	virtual bool shouldLoadConditionalLines(void);
	void addBoundingQuad(TREModel *model, const TCVector *minMax, int face);
	int actualColorNumber(LDLActionLine *actionLine, int activeColorNumber);
	std::string modelNameKey(LDLModel *model, int activeColorNumber);
	bool actionLineIsActive(LDLActionLine *actionLine);

	static bool unsetToken(StringSet &tokens, const char *token);
	const LDrawModelViewer *m_modelViewer;
	LDLModel *m_topLDLModel;
	TREMainModel *m_mainTREModel;
	TREModel *m_currentTREModel;
	int m_currentColorNumber;
	TCFloat m_seamWidth;
	TCByte m_defaultR;
	TCByte m_defaultG;
	TCByte m_defaultB;
	int m_defaultColorNumber;
	StringSet m_obiTokens;
	LDObiInfo *m_obiInfo;
	TCULong m_obiUniqueId;
	bool m_abort;	// Easier to not be a bit field.
	struct
	{
		bool flattenParts:1;
		bool seams:1;
		bool defaultColorSet:1;
		bool defaultColorNumberSet:1;
		bool defaultTrans:1;
		bool boundingBoxesOnly:1;
		bool obi:1;
		bool newTexmap:1;
		bool texmapNext:1;
		bool texmapStarted:1;
	} m_flags;
};

#endif // __LDMODELPARSER_H__
