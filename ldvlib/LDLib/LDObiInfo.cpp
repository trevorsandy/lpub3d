#include "LDObiInfo.h"
#include <LDLoader/LDLCommentLine.h>
#include <LDLoader/LDLModel.h>

#ifdef WIN32
#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
#define new DEBUG_CLIENTBLOCK
#endif // _DEBUG
#endif // WIN32

TCULong LDObiInfo::getColor(int defaultColor) const
{
	if (m_colors.size() > 0)
	{
		return m_colors.top();
	}
	return defaultColor;
}

TCULong LDObiInfo::getEdgeColor(int defaultColor) const
{
	if (m_edgeColors.size() > 0)
	{
		return m_edgeColors.top();
	}
	return defaultColor;
}

bool LDObiInfo::checkConditional(
	LDLCommentLine *commentLine,
	const StringSet &tokens)
{
	if (commentLine->hasOBIConditional())
	{
		bool set = commentLine->getOBIConditional();
		std::string token = commentLine->getOBIToken();
		StringSet::const_iterator it;

		convertStringToLower(&token[0]);
		it = tokens.find(token);
		if (it == tokens.end())
		{
			return !set;
		}
		else
		{
			return set;
		}
	}
	return true;
}

void LDObiInfo::start(LDLCommentLine *commentLine, const StringSet &tokens)
{
	LDLModel *model = commentLine->getParentModel();

	if (model && checkConditional(commentLine, tokens))
	{
		int colorNumber = commentLine->getOBIColorNumber();
		int edgeColorNumber = 0;

		start(model->getPackedRGBA(colorNumber),
			model->getPackedRGBA(edgeColorNumber),
			commentLine->getOBICommand() == LDLCommentLine::OBICStart);
	}
}

void LDObiInfo::start(TCULong color, TCULong edgeColor, bool sticky)
{
	m_colors.push(color);
	m_edgeColors.push(edgeColor);
	m_stickies.push(sticky);
}

void LDObiInfo::end(void)
{
	if (m_colors.size() > 0)
	{
		m_colors.pop();
		m_edgeColors.pop();
		m_stickies.pop();
	}
}

void LDObiInfo::actionHappened(void)
{
	if (m_stickies.size() > 0 && !m_stickies.top())
	{
		end();
	}
}
