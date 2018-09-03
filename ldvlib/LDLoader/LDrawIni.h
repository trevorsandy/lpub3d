/* LDrawIni.h  Include file for LDrawIni.c */

/* Copyright (c) 2004-2008  Lars C. Hassing (SP.lars@AM.hassings.dk)
If you make any changes to this file, please contact me, and I'll probably
adopt the changes for the benefit of other users.                            */

/*****************************************************************************
040228 lch First version of LDrawIni handling routines
040307 lch Added LgeoDir
040319 lch Added LDrawInP.h (Really LDrawIniP.h but 8+3 name...)
041202 lch Changed parameter ModelDir to ModelPath in LDrawIniComputeRealDirs
041229 lch Removed LgeoDir
050527 lch Added defines LDRAWINI_BEGIN_STDC/LDRAWINI_END_STDC for extern "C"
071120 lch Added LDrawIniReadSectionKey
080412 lch Added LDrawIniSetFileCaseCallback from Travis Cobbs
080910 lch Try typical locations for LDrawDir and see if they have P and PARTS
080915 lch Added LDrawDirOrigin and SearchDirsOrigin
******************************************************************************/

#ifndef LDRAWINI_INCLUDED
#define LDRAWINI_INCLUDED

/* Hide curly brackets {} in defines to avoid Beautifier indenting whole file */
#ifdef __cplusplus
#define LDRAWINI_BEGIN_STDC extern "C" {
#define LDRAWINI_END_STDC   }
typedef bool LDrawIniBoolT;
#else
#define LDRAWINI_BEGIN_STDC
#define LDRAWINI_END_STDC
typedef char LDrawIniBoolT;
#ifndef false
#define false 0
#endif
#ifndef true
#define true (!false)
#endif
#endif

LDRAWINI_BEGIN_STDC

typedef LDrawIniBoolT(*LDrawIniFileCaseCallbackF) (char *Path);

struct LDrawSearchDirS
{
   int            Flags;        /* Parsed and known flags LDSDF_XXX          */
   char          *UnknownFlags; /* Any unknown flags <XXX>                   */
   char          *Dir;          /* The full path of a search dir             */
};
struct LDrawIniPrivateDataS;    /* Defined in LDrawInP.h                     */

struct LDrawIniS
{
   /* The LDRAWDIR containing the P, PARTS and MODELS directories */
   char          *LDrawDir;

   /* The origin of the LDRAWDIR setting, may be path or text */
   char          *LDrawDirOrigin;

   /* The LDrawSearch directories ready to use */
   int            nSearchDirs;
   struct LDrawSearchDirS *SearchDirs;

   /* The origin of the SearchDirs setting, may be path or text */
   char          *SearchDirsOrigin;

   /* The dir extracted from ModelPath in last LDrawIniComputeRealDirs call */
   char          *ModelDir;

   /* Private date for the LDrawIni routines */
   struct LDrawIniPrivateDataS *PrivateData;
};

/* LDrawSearchDir Flags */
#define LDSDF_HIDE     0x0001   /* Hide directory                            */
#define LDSDF_SKIP     0x0002   /* Skip directory                            */
#define LDSDF_DEFPART  0x0004   /* Default filetype: Part                    */
#define LDSDF_DEFPRIM  0x0008   /* Default filetype: Primitive               */
#define LDSDF_MODELDIR 0x0010   /* <MODELDIR>                                */
#define LDSDF_UNOFFIC  0x0020   /* Unofficial directory                      */

/*
Initialize and read all settings.
If the argument LDrawDir is not NULL
it will override the normal initialization of the LDraw directory.
In that case you should explain the origin in LDrawDirOrigin, else pass NULL.
If ErrorCode is not NULL it will return a code telling why
LDrawIniGet returned NULL.
If all is OK then a pointer to struct LDrawIniS is returned.
You should then call LDrawIniComputeRealDirs to obtain the search dirs.
Remember to free the struct by calling LDrawIniFree.
*/
struct LDrawIniS *LDrawIniGet(const char *LDrawDir,
                              const char *LDrawDirOrigin,
                              int *ErrorCode);
/*
Sets the callback function to use for converting an input path to match the case
of the actual path on the filesystem.  When you implement such a function, you
must change the contents of the string passed into the function so that it
matches the case of the actual filesystem files.  This is needed on case
sensitive file systems, since LDraw files aren't case sensitive.  The callback
function must return false if no file could be found, and true if a file was
successfully found.
The function is called from LDrawIniComputeRealDirs when OnlyValidDirs
is set to true.
LCH note: If the user specifies search directories in the proper case,
then the function is unnecessary. File case fixing is relevant later
when finding parts. However, may also be necessary for testing
typical LDrawDir locations.
*/
void LDrawIniSetFileCaseCallback(LDrawIniFileCaseCallbackF FileCaseCallback);
/*
Compute Real Dirs by substituting <LDRAWDIR> and <MODELDIR> in
the Symbolic Dirs read from the env vars or ini files.
If OnlyValidDirs is true then non-existing search dirs is skipped
If AddTrailingSlash is true then the search dirs will have a slash/backslash appended.
If ModelPath is NULL then search dir <MODELDIR> is skipped.
Returns 1 if OK, 0 on error
*/
int LDrawIniComputeRealDirs(struct LDrawIniS * LDrawIni,
                            int OnlyValidDirs,
                            int AddTrailingSlash,
                            const char *ModelPath);
/*
Reset search dirs to default if LDrawSearch is NULL
or to the dirs specified in LDrawSearch delimited by |.
Returns 1 if OK, 0 on error
*/
int LDrawIniResetSearchDirs(struct LDrawIniS * LDrawIni,
                            const char *LDrawSearch);
/*
Free the LDrawIni data
*/
void LDrawIniFree(struct LDrawIniS * LDrawIni);

/*
Read Section/Key value.
This function is used internally, but has been made public so you can get
other LDraw related values, like e.g. "LDraw"/"LgeoDirectory".
If IniFile is specified then that file is read (Windows Ini format).
On the Mac if IniFile is "applicationID.plist", like "org.ldraw.plist",
then the various sources of application defaults are searched.
If IniFile is an empty string it is assumed to be a buffer of size sizeofIniFile
which will receive the path of the inifile actually read.
Returns 1 if OK, 0 if Section/Key not found or error
*/
int LDrawIniReadSectionKey(struct LDrawIniS * LDrawIni,
                           const char *Section, const char *Key,
                           char *Str, int sizeofStr,
                           char *IniFile, int sizeofIniFile);

/* Error codes returned by LDrawIniGet */
#define LDRAWINI_ERROR_OUT_OF_MEMORY     1
#define LDRAWINI_ERROR_LDRAWDIR_NOT_SET  2

LDRAWINI_END_STDC

#endif
