#include "lc_global.h"
#include "lc_objectproperty.h"
#include "lc_math.h"

#define LC_OBJECT_PROPERTY(T) \
	template void lcObjectProperty<T>::Update(lcStep Step); \
	template bool lcObjectProperty<T>::ChangeKey(const T& Value, lcStep Step, bool AddKey); \
	template void lcObjectProperty<T>::InsertTime(lcStep Start, lcStep Time); \
	template void lcObjectProperty<T>::RemoveTime(lcStep Start, lcStep Time); \
	template bool lcObjectProperty<T>::HasKeyFrame(lcStep Time) const; \
	template bool lcObjectProperty<T>::SetKeyFrame(lcStep Time, bool KeyFrame); \
	template void lcObjectProperty<T>::Save(QTextStream& Stream, const char* ObjectName, const char* VariableName, bool SaveEmpty, bool LPubMeta) const; \
	template bool lcObjectProperty<T>::Load(QTextStream& Stream, const QString& Token, const char* VariableName, bool LPubMeta);

LC_OBJECT_PROPERTY(float)
LC_OBJECT_PROPERTY(int)
LC_OBJECT_PROPERTY(lcVector2i)
LC_OBJECT_PROPERTY(lcVector2)
LC_OBJECT_PROPERTY(lcVector3)
LC_OBJECT_PROPERTY(lcVector4)
LC_OBJECT_PROPERTY(lcMatrix33)

template<typename T>
static void lcObjectPropertySaveValue(QTextStream& Stream, const T& Value)
{
	constexpr int Count = sizeof(T) / sizeof(float);

	for (int ValueIndex = 0; ValueIndex < Count; ValueIndex++)
		Stream << ((const float*)&Value)[ValueIndex] << ' ';
}

template<>
void lcObjectPropertySaveValue(QTextStream& Stream, const int& Value)
{
	constexpr int Count = sizeof(int) / sizeof(int);

	for (int ValueIndex = 0; ValueIndex < Count; ValueIndex++)
		Stream << ((const int*)&Value)[ValueIndex] << ' ';
}

template<>
void lcObjectPropertySaveValue(QTextStream& Stream, const lcVector2i& Value)
{
	constexpr int Count = sizeof(lcVector2i) / sizeof(int);

	for (int ValueIndex = 0; ValueIndex < Count; ValueIndex++)
		Stream << ((const int*)&Value)[ValueIndex] << ' ';
}

template<typename T>
static void lcObjectPropertyLoadValue(QTextStream& Stream, T& Value)
{
	constexpr int Count = sizeof(T) / sizeof(float);

	for (int ValueIdx = 0; ValueIdx < Count; ValueIdx++)
		Stream >> ((float*)&Value)[ValueIdx];
}

template<>
void lcObjectPropertyLoadValue(QTextStream& Stream, int& Value)
{
	constexpr int Count = sizeof(int) / sizeof(int);

	for (int ValueIdx = 0; ValueIdx < Count; ValueIdx++)
		Stream >> ((int*)&Value)[ValueIdx];
}

template<>
void lcObjectPropertyLoadValue(QTextStream& Stream, lcVector2i& Value)
{
	constexpr int Count = sizeof(lcVector2i) / sizeof(int);

	for (int ValueIdx = 0; ValueIdx < Count; ValueIdx++)
		Stream >> ((int*)&Value)[ValueIdx];
}

template<typename T>
void lcObjectProperty<T>::Update(lcStep Step)
{
	if (mKeys.empty())
		return;

	const lcObjectPropertyKey<T>* PreviousKey = &mKeys[0];

	for (const lcObjectPropertyKey<T>& Key : mKeys)
	{
		if (Key.Step > Step)
			break;

		PreviousKey = &Key;
	}

	mValue = PreviousKey->Value;
}

template<typename T>
bool lcObjectProperty<T>::ChangeKey(const T& Value, lcStep Step, bool AddKey)
{
	if (!AddKey && mKeys.empty())
	{
		if (mValue == Value)
			return false;

		mValue = Value;

		return true;
	}

	for (typename std::vector<lcObjectPropertyKey<T>>::iterator KeyIt = mKeys.begin(); KeyIt != mKeys.end(); KeyIt++)
	{
		if (KeyIt->Step < Step)
			continue;

		if (KeyIt->Step == Step)
		{
			if (KeyIt->Value == Value)
				return false;

			KeyIt->Value = Value;
		}
		else if (AddKey)
			mKeys.insert(KeyIt, lcObjectPropertyKey<T>{ Step, Value });
		else if (KeyIt == mKeys.begin())
		{
			if (KeyIt->Value == Value)
				return false;

			KeyIt->Value = Value;
		}
		else
		{
			KeyIt = KeyIt - 1;

			if (KeyIt->Value == Value)
				return false;

			KeyIt->Value = Value;
		}

		return true;
	}

	if (AddKey || mKeys.empty())
		mKeys.emplace_back(lcObjectPropertyKey<T>{ Step, Value });
	else
		mKeys.back().Value = Value;

	return true;
}

template<typename T>
void lcObjectProperty<T>::InsertTime(lcStep Start, lcStep Time)
{
	bool EndKey = false;

	for (typename std::vector<lcObjectPropertyKey<T>>::iterator KeyIt = mKeys.begin(); KeyIt != mKeys.end();)
	{
		if ((KeyIt->Step < Start) || (KeyIt->Step == 1))
		{
			KeyIt++;
			continue;
		}

		if (EndKey)
		{
			KeyIt = mKeys.erase(KeyIt);
			continue;
		}

		if (KeyIt->Step >= LC_STEP_MAX - Time)
		{
			KeyIt->Step = LC_STEP_MAX;
			EndKey = true;
		}
		else
			KeyIt->Step += Time;

		KeyIt++;
	}
}

template<typename T>
void lcObjectProperty<T>::RemoveTime(lcStep Start, lcStep Time)
{
	for (typename std::vector<lcObjectPropertyKey<T>>::iterator KeyIt = mKeys.begin(); KeyIt != mKeys.end();)
	{
		if ((KeyIt->Step < Start) || (KeyIt->Step == 1))
		{
			KeyIt++;
			continue;
		}

		if (KeyIt->Step < Start + Time)
		{
			KeyIt = mKeys.erase(KeyIt);
			continue;
		}

		KeyIt->Step -= Time;
		KeyIt++;
	}
}

template<typename T>
bool lcObjectProperty<T>::HasKeyFrame(lcStep Time) const
{
	for (typename std::vector<lcObjectPropertyKey<T>>::const_iterator KeyIt = mKeys.begin(); KeyIt != mKeys.end(); KeyIt++)
	{
		if (KeyIt->Step == Time)
			return true;
		else if (KeyIt->Step > Time)
			return false;
	}

	return false;
}

template<typename T>
bool lcObjectProperty<T>::SetKeyFrame(lcStep Time, bool KeyFrame)
{
	if (KeyFrame)
	{
		typename std::vector<lcObjectPropertyKey<T>>::const_iterator KeyIt;

		for (KeyIt = mKeys.begin(); KeyIt != mKeys.end(); KeyIt++)
		{
			if (KeyIt->Step == Time)
				return false;
			else if (KeyIt->Step > Time)
				break;
		}

		mKeys.insert(KeyIt, lcObjectPropertyKey<T>{ Time, mValue });

		return true;
	}
	else
	{
		for (typename std::vector<lcObjectPropertyKey<T>>::const_iterator KeyIt = mKeys.begin(); KeyIt != mKeys.end(); KeyIt++)
		{
			if (KeyIt->Step == Time)
			{
				if (mKeys.size() == 1)
					mValue = KeyIt->Value;

				mKeys.erase(KeyIt);

				return true;
			}
			else if (KeyIt->Step > Time)
				return false;
		}
	}

	return false;
}

template<typename T>
/*** LPub3D Mod - LPUB meta command ***/
void lcObjectProperty<T>::Save(QTextStream& Stream, const char* ObjectName, const char* VariableName, bool SaveEmpty, bool LPubMeta) const
{
	QLatin1String Meta(LPubMeta ? "0 !LPUB " : "0 !LEOCAD ");
	const bool SwitchAxis = LPubMeta && (QString(VariableName) == "POSITION" ||
										 QString(VariableName) == "TARGET_POSITION" ||
										 QString(VariableName) == "UP_VECTOR");
/*** LPub3D Mod end ***/

	if (mKeys.empty())
	{
		if (SaveEmpty)
		{
/*** LPub3D Mod - LPUB meta command ***/
			Stream << Meta << ObjectName << ' ' << VariableName << ' ';
/*** LPub3D Mod end ***/
/*** LPub3D Mod - Camera Globe, Switch Y and Z axis with -Y(LC -Z) in the up direction ***/
			T Value = mValue;
			if (SwitchAxis)
			{
				constexpr int Count = sizeof(T) / sizeof(float);
				if (Count == 3)
				{
					const lcVector3 Vector = lcVector3LeoCADToLDraw(lcVector3(((const float*)&mValue)[0], ((const float*)&mValue)[1], ((const float*)&mValue)[2]));
					for (int ValueIdx = 0; ValueIdx < Count; ValueIdx++)
						((float*)&Value)[ValueIdx] = Vector[ValueIdx];
				}
			}
			lcObjectPropertySaveValue(Stream, Value);
/*** LPub3D Mod end ***/
			Stream << QLatin1String("\r\n");
		}
	}
	else
	{
		for (const lcObjectPropertyKey<T>& Key : mKeys)
		{
/*** LPub3D Mod - LPUB meta command ***/
			Stream << Meta << ObjectName << ' ' << VariableName << "_KEY " << Key.Step << ' ';
/*** LPub3D Mod end ***/
/*** LPub3D Mod - Camera Globe, Switch Y and Z axis with -Y(LC -Z) in the up direction ***/
			T Value = Key.Value;
			if (SwitchAxis)
			{
				constexpr int Count = sizeof(T) / sizeof(float);
				if (Count == 3)
				{
					const lcVector3 Vector = lcVector3LeoCADToLDraw(lcVector3(((const float*)&mValue)[0], ((const float*)&mValue)[1], ((const float*)&mValue)[2]));
					for (int ValueIdx = 0; ValueIdx < Count; ValueIdx++)
						((float*)&Value)[ValueIdx] = Vector[ValueIdx];
				}
			}
			lcObjectPropertySaveValue(Stream, Value);
/*** LPub3D Mod end ***/
			Stream << QLatin1String("\r\n");
		}
	}
}

template<typename T>
/*** LPub3D Mod - LPUB meta command ***/
bool lcObjectProperty<T>::Load(QTextStream& Stream, const QString& Token, const char* VariableName, bool LPubMeta)
{
	const bool SwitchAxis = LPubMeta && (QString(VariableName) == "POSITION" ||
										 QString(VariableName) == "TARGET_POSITION" ||
										 QString(VariableName) == "UP_VECTOR");
/*** LPub3D Mod end ***/
/*** LPub3D Mod - Camera Globe, Switch Y and Z axis with -Y(LC -Z) in the up direction ***/
	if (Token == VariableName)
	{
		if (SwitchAxis && (sizeof(T) / sizeof(float) == 3))
		{
			QString Line;
			QTextStream SwitchStream(&Line, QIODevice::ReadWrite);
			lcVector3 Vector;
			Stream >> Vector[0] >> Vector[1] >> Vector[2];
			Vector = lcVector3LDrawToLeoCAD(Vector);
			lcObjectPropertySaveValue(SwitchStream, Vector);
			lcObjectPropertyLoadValue(SwitchStream, mValue);
		}
		else
		{
			lcObjectPropertyLoadValue(Stream, mValue);
		}
/*** LPub3D Mod end ***/

		return true;
	}

	if (Token.endsWith(QLatin1String("_KEY")) && Token.left(Token.size() - 4) == VariableName)
	{
		QString StepString;
		Stream >> StepString;

		const int Step = StepString.toInt();
		T Value;

		constexpr int Count = sizeof(T) / sizeof(float);
/*** LPub3D Mod - Camera Globe, Switch Y and Z axis with -Y(LC -Z) in the up direction ***/
		if (SwitchAxis && Count == 3)
		{
			lcVector3 Vector;
			Stream >> Vector[0] >> Vector[1] >> Vector[2];
			Vector = lcVector3LDrawToLeoCAD(Vector);

			for (int ValueIdx = 0; ValueIdx < Count; ValueIdx++)
				((float*)&Value)[ValueIdx] = Vector[ValueIdx];
		}
		else
		{
			for (int ValueIdx = 0; ValueIdx < Count; ValueIdx++)
				Stream >> ((float*)&Value)[ValueIdx];
		}
/*** LPub3D Mod end ***/

		ChangeKey(Value, Step, true);

		return true;
	}

	return false;
}
