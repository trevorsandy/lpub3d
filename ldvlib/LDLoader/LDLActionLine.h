#ifndef __LDLACTIONLINE_H__
#define __LDLACTIONLINE_H__

#include <LDLoader/LDLFileLine.h>
#include <LDLoader/LDLModel.h>

class LDLActionLine : public LDLFileLine
{
public:
	virtual bool isActionLine(void) const { return true; }
	virtual void setBFCSettings(BFCState bfcCertify, bool bfcClip,
		bool bfcWindingCCW, bool bfcInvert);
	virtual void setTexmapFallback(void);
	virtual void setBBoxIgnore(bool bValue)
	{
		m_actionFlags.bboxIgnore = bValue;
	}
	bool getBBoxIgnore(void) const { return m_actionFlags.bboxIgnore != false; }
	bool getBFCClip(void) const { return m_actionFlags.bfcClip != false; }
	bool getBFCWindingCCW(void) const
	{
		return m_actionFlags.bfcWindingCCW != false;
	}
	bool getBFCInvert(void) const { return m_actionFlags.bfcInvert != false; }
	BFCState getBFCState(void) const { return m_actionFlags.bfcCertify; }
	virtual int getColorNumber(void) const;
	virtual void setColorNumber(int value) { m_colorNumber = value; }
	bool getBFCOn(void) const
	{
		return (m_actionFlags.bfcCertify == BFCOnState ||
			m_actionFlags.bfcCertify == BFCForcedOnState) &&
			m_actionFlags.bfcClip;
	}
	bool isTexmapFallback(void) const
	{
		return m_actionFlags.texmapFallback ? true : false;
	}
	virtual void scanPoints(TCObject *scanner,
		LDLScanPointCallback scanPointCallback, const TCFloat *matrix,
		bool watchBBoxIgnore)
		const = 0;
protected:
	LDLActionLine(LDLModel *parentModel, const char *line, int lineNumber,
		const char *originalLine = NULL);
	LDLActionLine(const LDLActionLine &other);
	int getRandomColorNumber(void) const;

	static bool colorsAreSimilar(int r1, int g1, int b1, int r2, int g2,
		int b2);

	struct
	{
		// Public flags
		BFCState bfcCertify:3;
		bool bfcClip:1;
		bool bfcWindingCCW:1;
		bool bfcInvert:1;
		bool bboxIgnore:1;
		bool texmapFallback:1;
	} m_actionFlags;
	int m_colorNumber;
};

#endif // __LDLACTIONLINE_H__
