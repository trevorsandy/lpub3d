#include "LDLActionLine.h"
#include "LDLPalette.h"
#include "LDLMainModel.h"

#ifdef WIN32
#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
#define new DEBUG_CLIENTBLOCK
#endif // _DEBUG
#endif // WIN32

LDLActionLine::LDLActionLine(LDLModel *parentModel, const char *line,
							 int lineNumber, const char *originalLine)
	:LDLFileLine(parentModel, line, lineNumber, originalLine)
{
	memset(&m_actionFlags, 0, sizeof(m_actionFlags));
	m_actionFlags.bfcCertify = BFCUnknownState;
	m_actionFlags.bfcWindingCCW = true;
}

LDLActionLine::LDLActionLine(const LDLActionLine &other)
	:LDLFileLine(other),
	m_actionFlags(other.m_actionFlags),
	m_colorNumber(other.m_colorNumber)
{
}

void LDLActionLine::setBFCSettings(BFCState bfcCertify, bool bfcClip,
								  bool bfcWindingCCW, bool bfcInvert)
{
	m_actionFlags.bfcCertify = bfcCertify;
	m_actionFlags.bfcClip = bfcClip;
	m_actionFlags.bfcWindingCCW = bfcWindingCCW;
	m_actionFlags.bfcInvert = bfcInvert;
}

void LDLActionLine::setTexmapFallback(void)
{
	m_actionFlags.texmapFallback = true;
}

bool LDLActionLine::colorsAreSimilar(
	int r1,
	int g1,
	int b1,
	int r2,
	int g2,
	int b2)
{
#define COLORS_SIMILAR_THRESHOLD 128
	return abs(r1 - r2) < COLORS_SIMILAR_THRESHOLD &&
		abs(g1 - g2) < COLORS_SIMILAR_THRESHOLD &&
		abs(b1 - b2) < COLORS_SIMILAR_THRESHOLD;
}

int LDLActionLine::getRandomColorNumber(void) const
{
	const LDLMainModel *mainModel = getMainModel();
	int r = 0;
	int g = 0;
	int b = 0;
	bool failed = true;

	while (failed)
	{
		r = rand() % 256;
		g = rand() % 256;
		b = rand() % 256;
		failed = false;
		if (mainModel->getGreenFrontFaces() &&
			colorsAreSimilar(r, g, b, 0, 255, 0))
		{
			failed = true;
		}
		else if (mainModel->getRedBackFaces() &&
			colorsAreSimilar(r, g, b, 255, 0, 0))
		{
			failed = true;
		}
		else if (mainModel->getBlueNeutralFaces() &&
			colorsAreSimilar(r, g, b, 0, 0, 255))
		{
			failed = true;
		}
	}
	return LDLPalette::colorNumberForRGBA(r, g, b, 255);
}

int LDLActionLine::getColorNumber(void) const
{
	const LDLMainModel *mainModel = getMainModel();

	if (m_colorNumber != 24 && mainModel->getRandomColors() &&
		((const LDLModel *)mainModel == m_parentModel || m_colorNumber != 16))
	{
		return getRandomColorNumber();
	}
	else if (mainModel->getForceHighlightColor())
	{
		return mainModel->getHighlightColorNumber();
	}
	else
	{
		return m_colorNumber;
	}
}
