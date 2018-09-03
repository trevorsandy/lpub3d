#ifndef __LDEXPORTERSETTING_H__
#define __LDEXPORTERSETTING_H__

#include <TCFoundation/TCStlIncludes.h>
#include <TCFoundation/mystring.h>

typedef std::vector<ucstring> UCStringVector;
typedef std::map<size_t, long> SizeTLongMap;
typedef std::map<long, size_t> LongSizeTMap;

class LDExporterSetting
{
public:
	enum Type
	{
		TUnknown,
		TGroup,
		TBool,
		TLong,
		TFloat,
		TString,
		TEnum,
	};
	LDExporterSetting(void) : m_type(TUnknown) {}
	LDExporterSetting(CUCSTR name, int groupSize):
		m_name(name),
		m_type(TGroup),
		m_hasMin(false),
		m_hasMax(false),
		m_groupSize(groupSize),
		m_isPath(false) {}
	LDExporterSetting(CUCSTR name, const char *key):
		m_name(name),
		m_key(key),
		m_type(TUnknown),
		m_long(-1),
		m_hasMin(false),
		m_hasMax(false),
		m_groupSize(0),
		m_isPath(false) {}
	LDExporterSetting(CUCSTR name, bool value, const char *key):
		m_name(name),
		m_key(key),
		m_type(TBool),
		m_bool(value),
		m_hasMin(false),
		m_hasMax(false),
		m_groupSize(0),
		m_isPath(false) {}
	LDExporterSetting(CUCSTR name, long value, const char *key):
		m_name(name),
		m_key(key),
		m_type(TLong),
		m_long(value),
		m_hasMin(false),
		m_hasMax(false),
		m_groupSize(0),
		m_isPath(false) {}
	LDExporterSetting(CUCSTR name, TCFloat value, const char *key):
		m_name(name),
		m_key(key),
		m_type(TFloat),
		m_float(value),
		m_hasMin(false),
		m_hasMax(false),
		m_groupSize(0),
		m_isPath(false) {}
	LDExporterSetting(CUCSTR name, CUCSTR value, const char *key,
		bool isPath = false):
		m_name(name),
		m_key(key),
		m_type(TString),
		m_string(value),
		m_hasMin(false),
		m_hasMax(false),
		m_groupSize(0),
		m_isPath(isPath) {}
#ifndef TC_NO_UNICODE
	LDExporterSetting(CUCSTR name, const char *value, const char *key,
		bool isPath = false);
#endif // TC_NO_UNICODE

	Type getType(void) { return m_type; }

	const ucstring &getName(void) const { return m_name; }
	void setName(CUCSTR value);
	int getGroupSize(void) const { return m_groupSize; }
	void setGroupSize(int value) { m_groupSize = value; }

	bool isPath(void) const { return m_isPath; }
	void setIsPath(bool value);

	void setValue(bool value, bool commit = false);
	void setValue(long value, bool commit = false);
	void setValue(TCFloat value, bool commit = false);
	void setValue(CUCSTR value, bool commit = false);

	bool hasMin(void) { return m_hasMin; }
	bool hasMax(void) { return m_hasMax; }
	bool getBoolValue(void);
	long getLongValue(void);
	long getMinLongValue(void);
	long getMaxLongValue(void);
	void setMinValue(long value);
	void setMaxValue(long value);
	TCFloat getFloatValue(void);
	TCFloat getMinFloatValue(void);
	TCFloat getMaxFloatValue(void);
	void setMinValue(TCFloat value);
	void setMaxValue(TCFloat value);
	const ucstring &getStringValue(void);
	
	void reset(bool commit = false);

	void setTooltip(const char *localStringKey);
	const ucstring &getTooltip(void) const { return m_tooltip; }

	const UCStringVector &getOptions(void) const { return m_options; }
	void addOption(long lValue, const ucstring &name);
	void selectOption(size_t index, bool commit = false);
	size_t getSelectedOption(void) const;
protected:
	ucstring m_name;
	std::string m_key;
	Type m_type;
	union
	{
		bool m_bool;
		long m_long;
		TCFloat m_float;
	};
	// Classes with constructors can't go into unions; also, you can ask for the
	// string value of any type, and m_string is used for that.
	ucstring m_string;
	UCStringVector m_options;
	SizeTLongMap m_optionValues;
	LongSizeTMap m_optionIndices;
	bool m_hasMin;
	bool m_hasMax;
	long m_minLong;
	long m_maxLong;
	TCFloat m_minFloat;
	TCFloat m_maxFloat;
	int m_groupSize;
	bool m_isPath;
	ucstring m_tooltip;
};

typedef std::list<LDExporterSetting> LDExporterSettingList;

#endif // __LDEXPORTERSETTING_H__
