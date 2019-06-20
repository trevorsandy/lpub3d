#include "lc_global.h"
#include "lc_colors.h"
#include "lc_file.h"
#include <float.h>

lcArray<lcColor> gColorList;
lcColorGroup gColorGroups[LC_NUM_COLORGROUPS];
int gNumUserColors;
int gEdgeColor;
int gDefaultColor;

lcVector4 gInterfaceColors[LC_NUM_INTERFACECOLORS] = // todo: make the colors configurable and include the grid and other hardcoded colors here as well.
{
	lcVector4(0.898f, 0.298f, 0.400f, 1.000f), // LC_COLOR_SELECTED
	lcVector4(0.400f, 0.298f, 0.898f, 1.000f), // LC_COLOR_FOCUSED
	lcVector4(0.800f, 0.800f, 0.800f, 1.000f), // LC_COLOR_DISABLED
	lcVector4(0.500f, 0.800f, 0.500f, 1.000f), // LC_COLOR_CAMERA
	lcVector4(0.500f, 0.800f, 0.500f, 1.000f), // LC_COLOR_LIGHT
	lcVector4(0.500f, 0.800f, 0.500f, 0.500f), // LC_COLOR_CONTROL_POINT
	lcVector4(0.400f, 0.298f, 0.898f, 0.500f), // LC_COLOR_CONTROL_POINT_FOCUSED
	lcVector4(0.098f, 0.898f, 0.500f, 1.000f)  // LC_COLOR_HIGHLIGHT
};

static void GetToken(char*& Ptr, char* Token)
{
	while (*Ptr && *Ptr <= 32)
		Ptr++;

	while (*Ptr > 32)
		*Token++ = *Ptr++;

	*Token = 0;
}

int lcGetBrickLinkColor(int ColorIndex)
{
	struct lcBrickLinkEntry
	{
		int Code;
		const char* Name;
	};

/*** LPub3D Mod - Updated Bricklink Colour List ***/
	lcBrickLinkEntry BrickLinkColors[] =
	{
		{  0, "(Not Applicable)" },
		{  41, "Aqua" },
		{  11, "Black" },
		{  7, "Blue" },
		{  97, "Blue Violet" },
		{  36, "Bright Green" },
		{  105, "Bright Light Blue" },
		{  110, "Bright Light Orange" },
		{  103, "Bright Light Yellow" },
		{  104, "Bright Pink" },
		{  8, "Brown" },
		{  220, "Coral" },
		{  153, "Dark Azure" },
		{  63, "Dark Blue" },
		{  109, "Dark Blue Violet" },
		{  85, "Dark Bluish Grey" },
		{  120, "Dark Brown" },
		{  91, "Dark Flesh" },
		{  10, "Dark Grey" },
		{  80, "Dark Green" },
		{  68, "Dark Orange" },
		{  47, "Dark Pink" },
		{  89, "Dark Purple" },
		{  59, "Dark Red" },
		{  69, "Dark Tan" },
		{  39, "Dark Turquoise" },
		{  161, "Dark Yellow" },
		{  29, "Earth Orange" },
		{  106, "Fabuland Brown" },
		{  160, "Fabuland Orange" },
		{  28, "Flesh" },
		{  6, "Green" },
		{  154, "Lavender" },
		{  152, "Light Aqua" },
		{  62, "Light Blue" },
		{  86, "Light Bluish Grey" },
		{  90, "Light Flesh" },
		{  9, "Light Grey" },
		{  38, "Light Green" },
		{  35, "Light Lime" },
		{  32, "Light Orange" },
		{  56, "Light Pink" },
		{  93, "Light Purple" },
		{  26, "Light Salmon" },
		{  40, "Light Turquoise" },
		{  44, "Light Violet" },
		{  33, "Light Yellow" },
		{  34, "Lime" },
		{  72, "Maersk Blue" },
		{  71, "Magenta" },
		{  156, "Medium Azure" },
		{  42, "Medium Blue" },
		{  150, "Medium Dark Flesh" },
		{  94, "Medium Dark Pink" },
		{  37, "Medium Green" },
		{  157, "Medium Lavender" },
		{  76, "Medium Lime" },
		{  31, "Medium Orange" },
		{  73, "Medium Violet" },
		{  166, "Neon Green" },
		{  165, "Neon Orange" },
		{  155, "Olive Green" },
		{  4, "Orange" },
		{  23, "Pink" },
		{  24, "Purple" },
		{  5, "Red" },
		{  88, "Reddish Brown" },
		{  27, "Rust" },
		{  25, "Salmon" },
		{  55, "Sand Blue" },
		{  48, "Sand Green" },
		{  54, "Sand Purple" },
		{  58, "Sand Red" },
		{  87, "Sky Blue" },
		{  2, "Tan" },
		{  99, "Very Light Bluish Grey" },
		{  49, "Very Light Grey" },
		{  96, "Very Light Orange" },
		{  43, "Violet" },
		{  1, "White" },
		{  3, "Yellow" },
		{  158, "Yellowish Green" },
		{  13, "Trans Black" },
		{  108, "Trans Bright Green" },
		{  12, "Trans Clear" },
		{  14, "Trans Dark Blue" },
		{  50, "Trans Dark Pink" },
		{  20, "Trans Green" },
		{  15, "Trans Light Blue" },
		{  221, "Trans Light Green" },
		{  164, "Trans Light Orange" },
		{  114, "Trans Light Purple" },
		{  74, "Trans Medium Blue" },
		{  16, "Trans Neon Green" },
		{  18, "Trans Neon Orange" },
		{  121, "Trans Neon Yellow" },
		{  98, "Trans Orange" },
		{  107, "Trans Pink" },
		{  51, "Trans Purple" },
		{  17, "Trans Red" },
		{  113, "Trans Very Lt Blue" },
		{  19, "Trans Yellow" },
		{  57, "Chrome Antique Brass" },
		{  122, "Chrome Black" },
		{  52, "Chrome Blue" },
		{  21, "Chrome Gold" },
		{  64, "Chrome Green" },
		{  82, "Chrome Pink" },
		{  22, "Chrome Silver" },
		{  84, "Copper" },
		{  81, "Flat Dark Gold" },
		{  95, "Flat Silver" },
		{  78, "Metal Blue" },
		{  77, "Pearl Dark Grey" },
		{  115, "Pearl Gold" },
		{  61, "Pearl Light Gold" },
		{  66, "Pearl Light Grey" },
		{  119, "Pearl Very Light Grey" },
		{  83, "Pearl White" },
		{  65, "Metallic Gold" },
		{  70, "Metallic Green" },
		{  67, "Metallic Silver" },
		{  46, "Glow In Dark Opaque" },
		{  118, "Glow In Dark Trans" },
		{  159, "Glow In Dark White" },
		{  60, "Milky White" },
		{  101, "Glitter Trans Clear" },
		{  100, "Glitter Trans Dark Pink" },
		{  162, "Glitter Trans Light Blue" },
		{  163, "Glitter Trans Neon Green" },
		{  102, "Glitter Trans Purple" },
		{  116, "Speckle Black Copper" },
		{  151, "Speckle Black Gold" },
		{  111, "Speckle Black Silver" },
		{  117, "Speckle DBGrey Silver" },
		{  142, "Mx Aqua Green" },
		{  128, "Mx Black" },
		{  132, "Mx Brown" },
		{  133, "Mx Buff" },
		{  126, "Mx Charcoal Grey" },
		{  149, "Mx Clear" },
		{  214, "Mx Foil Dark Blue" },
		{  210, "Mx Foil Dark Grey" },
		{  212, "Mx Foil Dark Green" },
		{  215, "Mx Foil Light Blue" },
		{  211, "Mx Foil Light Grey" },
		{  213, "Mx Foil Light Green" },
		{  219, "Mx Foil Orange" },
		{  217, "Mx Foil Red" },
		{  216, "Mx Foil Violet" },
		{  218, "Mx Foil Yellow" },
		{  139, "Mx Lemon" },
		{  124, "Mx Light Bluish Grey" },
		{  125, "Mx Light Grey" },
		{  136, "Mx Light Orange" },
		{  137, "Mx Light Yellow" },
		{  144, "Mx Medium Blue" },
		{  138, "Mx Ochre Yellow" },
		{  140, "Mx Olive Green" },
		{  135, "Mx Orange" },
		{  145, "Mx Pastel Blue" },
		{  141, "Mx Pastel Green" },
		{  148, "Mx Pink" },
		{  130, "Mx Pink Red" },
		{  129, "Mx Red" },
		{  146, "Mx Teal Blue" },
		{  134, "Mx Terracotta" },
		{  143, "Mx Tile Blue" },
		{  131, "Mx Tile Brown" },
		{  127, "Mx Tile Grey" },
		{  147, "Mx Violet" },
		{  123, "Mx White" },
		{  172, "BA Black" },
		{  207, "BA Black Rubber" },
		{  183, "BA Blue" },
		{  199, "BA Blue Chrome" },
		{  197, "BA Brass" },
		{  198, "BA Bronze" },
		{  177, "BA Brown" },
		{  209, "BA Chrome" },
		{  168, "BA Cobalt" },
		{  190, "BA Dark Blue" },
		{  178, "BA Dark Brown" },
		{  175, "BA Dark Grey" },
		{  204, "BA Dark Grey Rubber" },
		{  179, "BA Dark Tan" },
		{  208, "BA Glow In Dark" },
		{  203, "BA Grey Rubber" },
		{  182, "BA Green" },
		{  170, "BA Gunmetal" },
		{  205, "BA Gunmetal Rubber" },
		{  174, "BA Light Grey" },
		{  189, "BA OD Green" },
		{  200, "BA OD Metallic" },
		{  181, "BA Olive" },
		{  192, "BA Pink" },
		{  191, "BA Purple" },
		{  176, "BA Red" },
		{  201, "BA Red Chrome" },
		{  184, "BA Sand Blue" },
		{  195, "BA Silver" },
		{  206, "BA Silver Rubber" },
		{  180, "BA Tan" },
		{  196, "BA Titanium" },
		{  186, "BA Trans Black" },
		{  188, "BA Trans Blue" },
		{  185, "BA Trans Clear" },
		{  171, "BA Trans Green" },
		{  187, "BA Trans Orange" },
		{  169, "BA Trans Red" },
		{  194, "BA Trans Red Sparkle" },
		{  193, "BA Trans Smoke" },
		{  167, "BA UN Blue" },
		{  173, "BA White" },
		{  202, "BA White Rubber" },
	};
/*** LPub3D Mod end ***/

	const char* Name = gColorList[ColorIndex].Name;

	for (unsigned int Color = 0; Color < sizeof(BrickLinkColors) / sizeof(BrickLinkColors[0]); Color++)
		if (!strcmp(Name, BrickLinkColors[Color].Name))
			return BrickLinkColors[Color].Code;

	return 0;
}

bool lcLoadColorFile(lcFile& File)
{
	char Line[1024], Token[1024];
	lcArray<lcColor>& Colors = gColorList;
	lcColor Color, MainColor, EdgeColor;

	Colors.RemoveAll();

	for (int GroupIdx = 0; GroupIdx < LC_NUM_COLORGROUPS; GroupIdx++)
		gColorGroups[GroupIdx].Colors.RemoveAll();

	gColorGroups[0].Name = QApplication::tr("Solid Colors", "Colors");
	gColorGroups[1].Name = QApplication::tr("Translucent Colors", "Colors");
	gColorGroups[2].Name = QApplication::tr("Special Colors", "Colors");
/*** LPub3D Mod - load color entry - add LPub3D color group ***/
	gColorGroups[3].Name = QApplication::tr("LPub3D Colors", "Colors");
/*** LPub3D Mod end ***/

	MainColor.Code = 16;
	MainColor.Translucent = false;
	MainColor.Value[0] = 1.0f;
	MainColor.Value[1] = 1.0f;
	MainColor.Value[2] = 0.5f;
	MainColor.Value[3] = 1.0f;
	MainColor.Edge[0] = 0.2f;
	MainColor.Edge[1] = 0.2f;
	MainColor.Edge[2] = 0.2f;
	MainColor.Edge[3] = 1.0f;
	strcpy(MainColor.Name, "Main Color");
	strcpy(MainColor.SafeName, "Main_Color");

	EdgeColor.Code = 24;
	EdgeColor.Translucent = false;
	EdgeColor.Value[0] = 0.5f;
	EdgeColor.Value[1] = 0.5f;
	EdgeColor.Value[2] = 0.5f;
	EdgeColor.Value[3] = 1.0f;
	EdgeColor.Edge[0] = 0.2f;
	EdgeColor.Edge[1] = 0.2f;
	EdgeColor.Edge[2] = 0.2f;
	EdgeColor.Edge[3] = 1.0f;
	strcpy(EdgeColor.Name, "Edge Color");
	strcpy(EdgeColor.SafeName, "Edge_Color");

	while (File.ReadLine(Line, sizeof(Line)))
	{
		char* Ptr = Line;

		GetToken(Ptr, Token);
		if (strcmp(Token, "0"))
			continue;

		GetToken(Ptr, Token);
		strupr(Token);
		if (strcmp(Token, "!COLOUR"))
			continue;

		bool GroupTranslucent = false;
		bool GroupSpecial = false;

		Color.Code = ~0U;
		Color.Translucent = false;
		Color.Value[0] = FLT_MAX;
		Color.Value[1] = FLT_MAX;
		Color.Value[2] = FLT_MAX;
		Color.Value[3] = 1.0f;
		Color.Edge[0] = FLT_MAX;
		Color.Edge[1] = FLT_MAX;
		Color.Edge[2] = FLT_MAX;
		Color.Edge[3] = 1.0f;
/*** LPub3D Mod - use 3DViewer colors ***/
		Color.CValue = ~0U;
		Color.EValue = ~0U;
		Color.Alpha = 255;
/*** LPub3D Mod end ***/

		GetToken(Ptr, Token);
		strncpy(Color.Name, Token, sizeof(Color.Name));
		Color.Name[LC_MAX_COLOR_NAME - 1] = 0;
		strncpy(Color.SafeName, Color.Name, sizeof(Color.SafeName));

		for (char* Underscore = strchr((char*)Color.Name, '_'); Underscore; Underscore = strchr(Underscore, '_'))
			*Underscore = ' ';

		for (GetToken(Ptr, Token); Token[0]; GetToken(Ptr, Token))
		{
			strupr(Token);

			if (!strcmp(Token, "CODE"))
			{
				GetToken(Ptr, Token);
				Color.Code = atoi(Token);
			}
			else if (!strcmp(Token, "VALUE"))
			{
				GetToken(Ptr, Token);
				if (Token[0] == '#')
					Token[0] = ' ';

				int Value;
				if (sscanf(Token, "%x", &Value) != 1)
					Value = 0;
/*** LPub3D Mod - use 3DViewer colors ***/
				Color.CValue = Value;
/*** LPub3D Mod end ***/
				Color.Value[2] = (float)(Value & 0xff) / 255.0f;
				Value >>= 8;
				Color.Value[1] = (float)(Value & 0xff) / 255.0f;
				Value >>= 8;
				Color.Value[0] = (float)(Value & 0xff) / 255.0f;
			}
			else if (!strcmp(Token, "EDGE"))
			{
				GetToken(Ptr, Token);
				if (Token[0] == '#')
					Token[0] = ' ';

				int Value;
				if (sscanf(Token, "%x", &Value) != 1)
					Value = 0;
/*** LPub3D Mod - use 3DViewer colors ***/
				Color.EValue = Value;
/*** LPub3D Mod end ***/

				Color.Edge[2] = (float)(Value & 0xff) / 255.0f;
				Value >>= 8;
				Color.Edge[1] = (float)(Value & 0xff) / 255.0f;
				Value >>= 8;
				Color.Edge[0] = (float)(Value & 0xff) / 255.0f;
			}
			else if (!strcmp(Token, "ALPHA"))
			{
				GetToken(Ptr, Token);
				int Value = atoi(Token);
				Color.Value[3] = (float)(Value & 0xff) / 255.0f;
				if (Value != 255)
					Color.Translucent = true;
/*** LPub3D Mod - use 3DViewer colors ***/
				Color.Alpha = Value;
/*** LPub3D Mod end ***/
				if (Value == 128)
					GroupTranslucent = true;
				else if (Value != 0)
					GroupSpecial = true;
			}
			else if (!strcmp(Token, "CHROME") || !strcmp(Token, "PEARLESCENT") || !strcmp(Token, "RUBBER") ||
			         !strcmp(Token, "MATTE_METALIC") || !strcmp(Token, "METAL") || !strcmp(Token, "LUMINANCE"))
			{
				GroupSpecial = true;
			}
			else if (!strcmp(Token, "MATERIAL"))
			{
				GroupSpecial = true;
				break; // Material is always last so ignore it and the rest of the line.
			}
		}

		if (Color.Code == ~0U || Color.Value[0] == FLT_MAX)
			continue;

		if (Color.Edge[0] == FLT_MAX)
		{
			Color.Edge[0] = 33.0f / 255.0f;
			Color.Edge[1] = 33.0f / 255.0f;
			Color.Edge[2] = 33.0f / 255.0f;
		}

		bool Duplicate = false;

		for (int i = 0; i < Colors.GetSize(); i++)
		{
			if (Colors[i].Code == Color.Code)
			{
				Colors[i] = Color;
				Duplicate = true;
				break;
			}
		}

		if (Duplicate)
			continue;

		if (Color.Code == 16)
		{
			MainColor = Color;
			continue;
		}

		if (Color.Code == 24)
		{
			EdgeColor = Color;
			continue;
		}

		Colors.Add(Color);

		if (GroupSpecial)
			gColorGroups[LC_COLORGROUP_SPECIAL].Colors.Add(Colors.GetSize() - 1);
		else if (GroupTranslucent)
			gColorGroups[LC_COLORGROUP_TRANSLUCENT].Colors.Add(Colors.GetSize() - 1);
		else
			gColorGroups[LC_COLORGROUP_SOLID].Colors.Add(Colors.GetSize() - 1);
	}

	gDefaultColor = Colors.GetSize();
	Colors.Add(MainColor);
	gColorGroups[LC_COLORGROUP_SOLID].Colors.Add(gDefaultColor);

	gNumUserColors = Colors.GetSize();

	gEdgeColor = Colors.GetSize();
	Colors.Add(EdgeColor);

	return Colors.GetSize() > 2;
}

/*** LPub3D Mod - load color entry ***/
bool lcLoadColorEntry(const char* ColorEntry)
{
//        qDebug() << qPrintable(QString("DEBUG Load color entry %1.")
//                               .arg(ColorEntry));
	char Line[1024], Token[1024];
	lcArray<lcColor>& Colors = gColorList;
	lcColor Color;

	int gNumColorBeforeAdd = Colors.GetSize();

	strncpy(Line, ColorEntry, sizeof(Line));

	char* Ptr = Line;

	GetToken(Ptr, Token);
	if (strcmp(Token, "0"))   // if no match, token evaluates to 1 (true)
		return false;

	GetToken(Ptr, Token);
	strupr(Token);
	if (strcmp(Token, "!COLOUR"))
		return false;

	Color.Code = ~0U;
	Color.Translucent = false;
	Color.Value[0] = FLT_MAX;
	Color.Value[1] = FLT_MAX;
	Color.Value[2] = FLT_MAX;
	Color.Value[3] = 1.0f;
	Color.Edge[0] = FLT_MAX;
	Color.Edge[1] = FLT_MAX;
	Color.Edge[2] = FLT_MAX;
	Color.Edge[3] = 1.0f;
	Color.CValue = ~0U;
	Color.EValue = ~0U;
	Color.Alpha = 255;

	GetToken(Ptr, Token);
	strncpy(Color.Name, Token, sizeof(Color.Name));
	Color.Name[LC_MAX_COLOR_NAME - 1] = 0;
	strncpy(Color.SafeName, Color.Name, sizeof(Color.SafeName));

	for (char* Underscore = strchr((char*)Color.Name, '_'); Underscore; Underscore = strchr(Underscore, '_'))
		*Underscore = ' ';

	for (GetToken(Ptr, Token); Token[0]; GetToken(Ptr, Token))
	{
		strupr(Token);

		if (!strcmp(Token, "CODE"))
		{
			GetToken(Ptr, Token);
			Color.Code = atoi(Token);
		}
		else if (!strcmp(Token, "VALUE"))
		{
			GetToken(Ptr, Token);
			if (Token[0] == '#')
				Token[0] = ' ';

			int Value;
			if (sscanf(Token, "%x", &Value) != 1)
				Value = 0;
			Color.EValue = Value;

			Color.Value[2] = (float)(Value & 0xff) / 255.0f;
			Value >>= 8;
			Color.Value[1] = (float)(Value & 0xff) / 255.0f;
			Value >>= 8;
			Color.Value[0] = (float)(Value & 0xff) / 255.0f;
		}
		else if (!strcmp(Token, "EDGE"))
		{
			GetToken(Ptr, Token);
			if (Token[0] == '#')
				Token[0] = ' ';

			int Value;
			if (sscanf(Token, "%x", &Value) != 1)
				Value = 0;
			Color.EValue = Value;

			Color.Edge[2] = (float)(Value & 0xff) / 255.0f;
			Value >>= 8;
			Color.Edge[1] = (float)(Value & 0xff) / 255.0f;
			Value >>= 8;
			Color.Edge[0] = (float)(Value & 0xff) / 255.0f;
		}
		else if (!strcmp(Token, "ALPHA"))
		{
			GetToken(Ptr, Token);
			int Value = atoi(Token);
			Color.Alpha = Value;
			Color.Value[3] = (float)(Value & 0xff) / 255.0f;
			if (Value != 255)
			{
				Color.Translucent = true;
				Color.Edge[3] = (float)(Value & 0xff) / 255.0f;
			}
		}
	}

	if (Color.Code == ~0U || Color.Value[0] == FLT_MAX)  // Code or Value attribute not set
		return false;

	if (Color.Edge[0] == FLT_MAX)
	{
		Color.Edge[0] = 33.0f / 255.0f;
		Color.Edge[1] = 33.0f / 255.0f;
		Color.Edge[2] = 33.0f / 255.0f;
	}

	bool Duplicate = false;

	for (int i = 0; i < Colors.GetSize(); i++)
	{
		if (Colors[i].Code == Color.Code)
		{
			Colors[i] = Color;
			Duplicate = true;
			break;
		}
	}

	if (Duplicate)
		return true;

	Colors.Add(Color);

	gColorGroups[LC_COLORGROUP_LPUB3D].Colors.Add(Colors.GetSize() - 1);

	gNumUserColors = Colors.GetSize();

//        qDebug() << qPrintable(QString("DEBUG Colours New Size %1, Old Size %2.")
//                               .arg(Colors.GetSize()).arg(gNumColorBeforeAdd));

	return Colors.GetSize() > gNumColorBeforeAdd;
}
/*** LPub3D Mod end ***/

void lcLoadDefaultColors()
{
	QResource Resource(":/resources/ldconfig.ldr");

	if (!Resource.isValid())
		return;

	QByteArray Data;

	if (Resource.isCompressed())
		Data = qUncompress(Resource.data(), Resource.size());
	else
		Data = QByteArray::fromRawData((const char*)Resource.data(), Resource.size());

	lcMemFile MemSettings;

	MemSettings.WriteBuffer(Data.constData(), Data.size());
	MemSettings.Seek(0, SEEK_SET);
	lcLoadColorFile(MemSettings);
}

int lcGetColorIndex(quint32 ColorCode)
{
	for (int ColorIdx = 0; ColorIdx < gColorList.GetSize(); ColorIdx++)
		if (gColorList[ColorIdx].Code == ColorCode)
			return ColorIdx;

	lcColor Color;

	Color.Code = ColorCode;
	Color.Translucent = false;
	Color.Edge[0] = 0.2f;
	Color.Edge[1] = 0.2f;
	Color.Edge[2] = 0.2f;
	Color.Edge[3] = 1.0f;

	if (ColorCode & LC_COLOR_DIRECT)
	{
		Color.Value[0] = (float)((ColorCode & 0xff0000) >> 16) / 255.0f;
		Color.Value[1] = (float)((ColorCode & 0x00ff00) >>  8) / 255.0f;
		Color.Value[2] = (float)((ColorCode & 0x0000ff) >>  0) / 255.0f;
		Color.Value[3] = 1.0f;
		sprintf(Color.Name, "Color %06X", ColorCode & 0xffffff);
		sprintf(Color.SafeName, "Color_%06X", ColorCode & 0xffffff);
	}
	else
	{
		Color.Value[0] = 0.5f;
		Color.Value[1] = 0.5f;
		Color.Value[2] = 0.5f;
		Color.Value[3] = 1.0f;
		sprintf(Color.Name, "Color %03d", ColorCode);
		sprintf(Color.SafeName, "Color_%03d", ColorCode);
	}

	gColorList.Add(Color);
	return gColorList.GetSize() - 1;
}
