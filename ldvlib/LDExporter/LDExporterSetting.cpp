#include "LDExporterSetting.h"
#include <TCFoundation/mystring.h>
#include <TCFoundation/TCUserDefaults.h>
#include <TCFoundation/TCLocalStrings.h>
#include <stdlib.h>

#ifndef TC_NO_UNICODE
LDExporterSetting::LDExporterSetting(
	CUCSTR name,
	const char *value,
	const char *key,
	bool isPath /*= false*/):
m_name(name),
m_key(key),
m_type(TString),
m_hasMin(false),
m_hasMax(false),
m_groupSize(0),
m_isPath(isPath)
{
	stringtowstring(m_string, value);
}
#endif // TC_NO_UNICODE

void LDExporterSetting::setName(CUCSTR value)
{
	if (m_type == TUnknown)
	{
		m_name = value;
	}
	else
	{
		throw "Name cannot be changed after type is set.";
	}
}

void LDExporterSetting::setValue(bool value, bool commit /*= false*/)
{
	if (m_type == TBool || m_type == TUnknown)
	{
		m_type = TBool;
		m_bool = value;
		if (commit)
		{
			TCUserDefaults::setBoolForKey(value, m_key.c_str());
		}
	}
	else
	{
		throw "Invalid type";
	}
}

void LDExporterSetting::setValue(long value, bool commit /*= false*/)
{
	if (m_type == TLong || m_type == TUnknown)
	{
		m_type = TLong;
		m_long = value;
		if (commit)
		{
			TCUserDefaults::setLongForKey(value, m_key.c_str());
		}
	}
	else
	{
		throw "Invalid type";
	}
}

void LDExporterSetting::setValue(TCFloat value, bool commit /*= false*/)
{
	if (m_type == TFloat || m_type == TUnknown)
	{
		m_type = TFloat;
		m_float = value;
		if (commit)
		{
			TCUserDefaults::setFloatForKey(value, m_key.c_str());
		}
	}
	else
	{
		throw "Invalid type";
	}
}

void LDExporterSetting::reset(bool commit /*= false*/)
{
	switch (m_type)
	{
	case TLong:
		setValue(TCUserDefaults::defaultLongForKey(m_key.c_str()), commit);
		break;
	case TEnum:
		try
		{
			selectOption(
				(size_t)TCUserDefaults::defaultLongForKey(m_key.c_str()),
				commit);
		}
		catch (...)
		{
		}
		break;
	case TBool:
		setValue(TCUserDefaults::defaultBoolForKey(m_key.c_str()), commit);
		break;
	case TFloat:
		setValue(TCUserDefaults::defaultFloatForKey(m_key.c_str()), commit);
		break;
	case TString:
		setValue(TCUserDefaults::defaultStringForKeyUC(m_key.c_str()), commit);
		break;
	default:
		break;
	}
}

void LDExporterSetting::setValue(CUCSTR value, bool commit /*= false*/)
{
	switch (m_type)
	{
	case TLong:
		{
			char *string = ucstringtombs(value);

			setValue((long)atoi(string), commit);
			delete[] string;
		}
		break;
	case TFloat:
		{
			char *string = ucstringtombs(value);

			setValue((TCFloat)atof(string), commit);
			delete[] string;
		}
		break;
	case TUnknown:
		m_type = TString;
		// NO break
	case TString:
		m_string = value;
		if (commit)
		{
			if (m_isPath)
			{
#ifdef TC_NO_UNICODE
				std::string &path = m_string;
#else // TC_NO_UNICODE
				std::string path;

				wstringtostring(path, m_string);
#endif // TC_NO_UNICODE
				TCUserDefaults::setPathForKey(path.c_str(), m_key.c_str());
			}
			else
			{
				TCUserDefaults::setStringForKey(value, m_key.c_str());
			}
		}
		break;
	default:
		throw "Invalid type";
	}
}

bool LDExporterSetting::getBoolValue(void)
{
	if (m_type == TBool)
	{
		return m_bool;
	}
	else
	{
		throw "Invalid type";
	}
}

long LDExporterSetting::getLongValue(void)
{
	if (m_type == TLong)
	{
		return m_long;
	}
	else
	{
		throw "Invalid type";
	}
}

TCFloat LDExporterSetting::getFloatValue(void)
{
	if (m_type == TFloat)
	{
		return m_float;
	}
	else
	{
		throw "Invalid type";
	}
}

const ucstring &LDExporterSetting::getStringValue(void)
{
	switch (m_type)
	{
	case TLong:
		m_string = ltoucstr(m_long);
		break;
	case TFloat:
		m_string = ftoucstr(m_float);
		break;
	case TEnum:
		m_string = m_options[getSelectedOption()];
		break;
	case TString:
		break;
	default:
		throw "Invalid type";
	}
	return m_string;
}

long LDExporterSetting::getMinLongValue(void)
{
	if (m_type == TLong)
	{
		if (m_hasMin)
		{
			return m_minLong;
		}
		else
		{
			throw "Attempt to read min when no min is set.";
		}
	}
	else
	{
		throw "Invalid type";
	}
}

long LDExporterSetting::getMaxLongValue(void)
{
	if (m_type == TLong)
	{
		if (m_hasMax)
		{
			return m_maxLong;
		}
		else
		{
			throw "Attempt to read max when no max is set.";
		}
	}
	else
	{
		throw "Invalid type";
	}
}

void LDExporterSetting::setMinValue(long value)
{
	if (m_type == TLong)
	{
		m_minLong = value;
		m_hasMin = true;
	}
	else
	{
		throw "Invalid type";
	}
}

void LDExporterSetting::setMaxValue(long value)
{
	if (m_type == TLong)
	{
		m_maxLong = value;
		m_hasMax = true;
	}
	else
	{
		throw "Invalid type";
	}
}

TCFloat LDExporterSetting::getMinFloatValue(void)
{
	if (m_type == TFloat)
	{
		if (m_hasMin)
		{
			return m_minFloat;
		}
		else
		{
			throw "Attempt to read min when no min is set.";
		}
	}
	else
	{
		throw "Invalid type";
	}
}

TCFloat LDExporterSetting::getMaxFloatValue(void)
{
	if (m_type == TFloat)
	{
		if (m_hasMax)
		{
			return m_maxFloat;
		}
		else
		{
			throw "Attempt to read max when no max is set.";
		}
	}
	else
	{
		throw "Invalid type";
	}
}

void LDExporterSetting::setMinValue(TCFloat value)
{
	if (m_type == TFloat)
	{
		m_minFloat = value;
		m_hasMin = true;
	}
	else
	{
		throw "Invalid type";
	}
}

void LDExporterSetting::setMaxValue(TCFloat value)
{
	if (m_type == TFloat)
	{
		m_maxFloat = value;
		m_hasMax = true;
	}
	else
	{
		throw "Invalid type";
	}
}

void LDExporterSetting::setIsPath(bool value)
{
	if (m_type == TString)
	{
		m_isPath = value;
	}
	else
	{
		throw "Invalid type";
	}
}

void LDExporterSetting::addOption(long lValue, const ucstring &name)
{
	if (m_type == TUnknown || m_type == TEnum)
	{
		m_type = TEnum;
		m_optionValues[m_options.size()] = lValue;
		m_optionIndices[lValue] = m_options.size();
		m_options.push_back(name);
	}
	else
	{
		throw "Invalid type";
	}
}

void LDExporterSetting::selectOption(size_t index, bool commit /*= false*/)
{
	if (m_type == TEnum)
	{
		SizeTLongMap::const_iterator it = m_optionValues.find(index);

		if (it == m_optionValues.end())
		{
			throw "Invalid index";
		}
		else
		{
			m_long = it->second;
			if (commit)
			{
				TCUserDefaults::setLongForKey(m_long, m_key.c_str());
			}
		}
	}
	else
	{
		throw "Invalid type";
	}
}

size_t LDExporterSetting::getSelectedOption(void) const
{
	LongSizeTMap::const_iterator it = m_optionIndices.find(m_long);

	if (it == m_optionIndices.end())
	{
		throw "No option selected";
	}
	else
	{
		return it->second;
	}
}

void LDExporterSetting::setTooltip(const char *localStringName)
{
	ucstring key;
	
	mbstoucstring(key, localStringName);
	m_tooltip = TCLocalStrings::get(key.c_str());
}
