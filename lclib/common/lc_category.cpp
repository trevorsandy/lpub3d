#include "lc_global.h"
#include "lc_category.h"
#include "lc_file.h"
#include "lc_profile.h"

/*** LPub3D Mod - support TENTE and VEX Categories ***/
#include "lpub_preferences.h"
#include "name.h"
/*** LPub3D Mod end ***/

std::vector<lcLibraryCategory> gCategories;

void lcResetDefaultCategories()
{
	lcResetCategories(gCategories);

	lcRemoveProfileKey(LC_PROFILE_CATEGORIES);
}

void lcLoadDefaultCategories(bool BuiltInLibrary)
{
	QByteArray Buffer = lcGetProfileBuffer(LC_PROFILE_CATEGORIES);

	if (Buffer.isEmpty() || !lcLoadCategories(Buffer, gCategories))
		lcResetCategories(gCategories, BuiltInLibrary);
}

void lcSaveDefaultCategories()
{
	QByteArray ByteArray;
	QTextStream Stream(&ByteArray, QIODevice::WriteOnly);

	lcSaveCategories(Stream, gCategories);

	lcSetProfileBuffer(LC_PROFILE_CATEGORIES, ByteArray);
}

void lcResetCategories(std::vector<lcLibraryCategory>& Categories, bool BuiltInLibrary)
{
	const char DefaultCategories[] =
	{
		"Animal=^%Animal | ^%Bone\n"
		"Antenna=^%Antenna\n"
		"Arch=^%Arch\n"
		"Bar=^%Bar\n"
		"Baseplate=^%Baseplate | ^%Platform\n"
		"Boat=^%Boat | ^%Sail\n"
		"Brick=^%Brick\n"
		"Container=^%Container | ^%Box | ^Chest | ^%Storage | ^Mailbox\n"
		"Door and Window=^%Door | ^%Window | ^%Glass | ^%Freestyle | ^%Gate | ^%Garage | ^%Roller\n"
		"Electric=^%Battery | ^%Electric\n"
		"Hinge and Bracket=^%Hinge | ^%Bracket | ^%Turntable\n"
		"Hose=^%Hose | ^%Rubber | ^%String\n"
		"Minifig=^%Minifig\n"
		"Miscellaneous=^%Arm | ^%Barrel | ^%Brush | ^%Bucket | ^%Cardboard | ^%Claw | ^%Cockpit | ^%Cocoon | ^%Conveyor | ^%Crane | ^%Cupboard | ^%Fence | ^%Gold | ^%Handle | ^%Hook | ^%Jack | ^%Key | ^%Ladder | ^%Medical | ^%Motor | ^%Rock | ^%Rope | ^%Slide | ^%Sheet | ^%Snow | ^%Sports | ^%Spring | ^%Staircase | ^%Stretcher | ^%Tap | ^%Tipper | ^%Trailer | ^%Umbrella | ^%Winch\n"
		"Other=^%Ball | ^%Belville | ^%BigFig | ^%Die | ^%Duplo | ^%Fabuland | ^%Figure | ^%Homemaker | ^%Maxifig | ^%Microfig | ^%Mursten | ^%Quatro | ^%Scala | ^%Znap\n"
		"Panel=^%Panel | ^%Castle Wall | ^%Castle Turret\n"
		"Plant=^%Plant\n"
		"Plate=^%Plate\n"
		"Round=^%Cylinder | ^%Cone | ^%Dish | ^%Dome | ^%Hemisphere | ^%Round\n"
		"Sign and Flag=^%Flag | ^%Roadsign | ^%Streetlight | ^%Flagpost | ^%Lamppost | ^%Signpost\n"
		"Slope=^%Slope | ^%Roof\n"
		"Sticker=^%Sticker\n"
		"Support=^%Support\n"
		"Technic=^%Technic | ^%Rack\n"
		"Tile=^%Tile\n"
		"Train=^%Train | ^%Monorail | ^%Magnet\n"
		"Tyre and Wheel=^%Tyre | %^Wheel | %^Wheels | ^%Castle Wagon | ^%Axle\n"
		"Vehicle=^%Bike | ^%Canvas | ^%Car | ^%Excavator | ^%Exhaust | ^%Forklift | ^%Grab Jaw | ^%Jet | ^%Landing | ^%Motorcycle | ^%Plane | ^%Propellor | ^%Tail | ^%Tractor | ^%Vehicle | ^%Wheelbarrow\n"
		"Windscreen=^%Windscreen\n"
		"Wedge=^%Wedge\n"
		"Wing=^%Wing\n"
	};

/*** LPub3D Mod - support TENTE and VEX Categories ***/
    const char TENTEDefaultCategories[] =
    {
        "TENTE-Aire=@A\n"
        "TENTE-Alpha=@F\n"
        "TENTE-Baldosas placas=baldosa | placa\n"
        "TENTE-Basico=@B\n"
        "TENTE-Castillos=:X\n"
        "TENTE-Dark=@D\n"
        "TENTE-Escorpion=@E\n"
        "TENTE-Etiquetas=Etiqueta\n"
        "TENTE-Jacenas=jacena\n"
        "TENTE-Mar=@M\n"
        "TENTE-Micro=@C\n"
        "TENTE-Ruedas=rueda | ( eje & !bisagra ) | ( agujero & !enganche & !baldosa )\n"
        "TENTE-Ruta=@R\n"
        "TENTE-Titanium=@T\n"
        "TENTE-Vehiculos=@V \n"
        "TENTE-Ventanas=ventanas | marco | cristal\n"
    };

    const char VEXIQDefaultCategories[] =
    {
        "VEXIQ-Axles and Spacers=-Vgr1-\n"
        "VEXIQ-Beams and Plates=-Vgr2-\n"
        "VEXIQ-Connectors=-Vgr4-\n"
        "VEXIQ-Control System=-Vgr7-\n"
        "VEXIQ-Gears and Motion=-Vgr6-\n"
        "VEXIQ-Panels and Special Beams=-Vgr9-\n"
        "VEXIQ-Pins and Standoffs=-Vgr3-\n"
        "VEXIQ-Wheels and Tires=-Vgr5-\n"
        "VEXIQ-Miscellaneous Parts=-Vgr8-\n"
        "VEXIQ-Legacy Parts=-Vgr10-\n"
    };
/*** LPub3D Mod end ***/

	const char BuiltInCategories[] =
	{
		"Baseplate=^%Baseplate\n"
		"Brick=^%Brick\n"
		"Plate=^%Plate\n"
		"Slope=^%Slope\n"
		"Tile=^%Tile\n"
	};

	QByteArray Buffer;
/*** LPub3D Mod - support TENTE and VEX Categories ***/
    if (BuiltInLibrary)
        Buffer.append(BuiltInCategories, sizeof(BuiltInCategories));
    else if (Preferences::validLDrawLibrary == TENTE_LIBRARY)
        Buffer.append(TENTEDefaultCategories, sizeof(TENTEDefaultCategories));
    else if (Preferences::validLDrawLibrary == VEXIQ_LIBRARY)
        Buffer.append(VEXIQDefaultCategories, sizeof(VEXIQDefaultCategories));
    else
        Buffer.append(DefaultCategories, sizeof(DefaultCategories));
/*** LPub3D Mod end ***/

	lcLoadCategories(Buffer, Categories);
}

bool lcLoadCategories(const QString& FileName, std::vector<lcLibraryCategory>& Categories)
{
	QFile File(FileName);

	if (!File.open(QIODevice::ReadOnly))
		return false;

	QByteArray FileData = File.readAll();

	return lcLoadCategories(FileData, Categories);
}

bool lcLoadCategories(const QByteArray& Buffer, std::vector<lcLibraryCategory>& Categories)
{
	Categories.clear();

	QTextStream Stream(Buffer);

	for (QString Line = Stream.readLine(); !Line.isNull(); Line = Stream.readLine())
	{
		int Equals = Line.indexOf('=');

		if (Equals == -1)
			continue;

		QString Name = Line.left(Equals);
		QString Keywords = Line.mid(Equals + 1);

		lcLibraryCategory Category;

		Category.Name = Name;
		Category.Keywords = Keywords.toLatin1();

		Categories.emplace_back(std::move(Category));
	}

	return true;
}

bool lcSaveCategories(const QString& FileName, const std::vector<lcLibraryCategory>& Categories)
{
	QFile File(FileName);

	if (!File.open(QIODevice::WriteOnly))
		return false;

	QTextStream Stream(&File);

	return lcSaveCategories(Stream, Categories);
}

bool lcSaveCategories(QTextStream& Stream, const std::vector<lcLibraryCategory>& Categories)
{
	QString Format("%1=%2\r\n");

	for (const lcLibraryCategory& Category : Categories)
		Stream << Format.arg(Category.Name, QString::fromLatin1(Category.Keywords));

	Stream.flush();

	return true;
}

bool lcMatchCategory(const char* PieceName, const char* Expression)
{
	// Check if we need to split the test expression.
	const char* p = Expression;

	while (*p)
	{
		if (*p == '!')
		{
			return !lcMatchCategory(PieceName, p + 1);
		}
		else if (*p == '(')
		{
//			const char* Start = p;
			int c = 0;

			// Skip what's inside the parenthesis.
			do
			{
				if (*p == '(')
						c++;
				else if (*p == ')')
						c--;
				else if (*p == 0)
					return false; // Mismatched parenthesis.

				p++;
			}
			while (c);

			if (*p == 0)
				break;
		}
		else if ((*p == '|') || (*p == '&'))
		{
			std::string LeftStr(Expression, (p - Expression) - 1);
			std::string RightStr(p + 1);

			if (*p == '|')
				return lcMatchCategory(PieceName, LeftStr.c_str()) || lcMatchCategory(PieceName, RightStr.c_str());
			else
				return lcMatchCategory(PieceName, LeftStr.c_str()) && lcMatchCategory(PieceName, RightStr.c_str());
		}

		p++;
	}

	if (strchr(Expression, '('))
	{
		p = Expression;
		while (*p)
		{
			if (*p == '(')
			{
				const char* Start = p;
				int c = 0;

				// Extract what's inside the parenthesis.
				do
				{
					if (*p == '(')
							c++;
					else if (*p == ')')
							c--;
					else if (*p == 0)
						return false; // Mismatched parenthesis.

					p++;
				}
				while (c);

				std::string SubExpression(Start + 1, p - Start - 2);
				return lcMatchCategory(PieceName, SubExpression.c_str());
			}

			p++;
		}
	}

	const char* SearchStart = Expression;
	while (isspace(*SearchStart))
		SearchStart++;

	const char* SearchEnd = SearchStart + strlen(SearchStart) - 1;
	while (SearchEnd >= SearchStart && isspace(*SearchEnd))
		SearchEnd--;

	// Testing a simple case.
	std::string Search;
	if (SearchStart != SearchEnd)
		Search = std::string(SearchStart, SearchEnd - SearchStart + 1);
	const char* Word = Search.c_str();

	// Check for modifiers.
	bool WholeWord = 0;
	bool Begin = 0;

	for (;;)
	{
		if (Word[0] == '^')
			WholeWord = true;
		else if (Word[0] == '%')
			Begin = true;
		else
			break;

		Word++;
	}

	const char* Result = strcasestr(PieceName, Word);

	if (!Result)
		return false;

	if (Begin && (Result != PieceName))
	{
		if ((Result != PieceName + 1) || ((Result[-1] != '_') && (Result[-1] != '~')))
			return false;
	}

	if (WholeWord)
	{
		char End = Result[strlen(Word)];

		if ((End != 0) && (End != ' '))
			return false;

		if ((Result != PieceName) && ((Result[-1] == '_') || (Result[-1] == '~')))
			Result--;

		if ((Result != PieceName) && (Result[-1] != ' '))
			return false;
	}

	return true;
}
