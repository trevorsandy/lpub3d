/****************************************************************************
**
** Copyright (C) 2018 - 2020 Trevor SANDY. All rights reserved.
**
** This file may be used under the terms of the
** GNU General Public Liceense (GPL) version 3.0
** which accompanies this distribution, and is
** available at http://www.gnu.org/licenses/gpl.html
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "LDVExtensionsSetup.h"
#include <TCFoundation/mystring.h>
#include <TCFoundation/TCUserDefaults.h>
#include <TRE/TREShapeGroup.h>
#include <TRE/TREGLExtensions.h>
#include <LDLib/LDUserDefaultsKeys.h>

#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
#define new DEBUG_CLIENTBLOCK
#endif // _DEBUG

#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
#define new DEBUG_CLIENTBLOCK
#endif

// WGL_EXT_pixel_format
PFNWGLGETPIXELFORMATATTRIBIVEXTPROC
	LDVExtensionsSetup::sm_wglGetPixelFormatAttribivARB = NULL;
PFNWGLGETPIXELFORMATATTRIBFVEXTPROC
	LDVExtensionsSetup::sm_wglGetPixelFormatAttribfvARB = NULL;
PFNWGLCHOOSEPIXELFORMATEXTPROC
	LDVExtensionsSetup::sm_wglChoosePixelFormatARB = NULL;
// WGL_ARB_extensions_string
PFNWGLGETEXTENSIONSSTRINGARBPROC
	LDVExtensionsSetup::sm_wglGetExtensionsStringARB = NULL;
// WGL_ARB_pbuffer
PFNWGLCREATEPBUFFERARBPROC LDVExtensionsSetup::sm_wglCreatePbufferARB = NULL;
PFNWGLGETPBUFFERDCARBPROC LDVExtensionsSetup::sm_wglGetPbufferDCARB = NULL;
PFNWGLRELEASEPBUFFERDCARBPROC LDVExtensionsSetup::sm_wglReleasePbufferDCARB = NULL;
PFNWGLDESTROYPBUFFERARBPROC LDVExtensionsSetup::sm_wglDestroyPbufferARB = NULL;
PFNWGLQUERYPBUFFERARBPROC LDVExtensionsSetup::sm_wglQueryPbufferARB = NULL;
// WGL_NV_allocate_memory
PFNWGLALLOCATEMEMORYNVPROC LDVExtensionsSetup::sm_wglAllocateMemoryNV = NULL;
PFNWGLFREEMEMORYNVPROC LDVExtensionsSetup::sm_wglFreeMemoryNV = NULL;

StringSet LDVExtensionsSetup::sm_wglExtensions;
char *LDVExtensionsSetup::sm_wglExtensionsString = NULL;
bool LDVExtensionsSetup::sm_performedInitialSetup = false;
bool LDVExtensionsSetup::sm_stencilPresent = false;
bool LDVExtensionsSetup::sm_alphaPresent = false;
TCIntArray *LDVExtensionsSetup::sm_fsaaModes = NULL;
TCIntArrayArray *LDVExtensionsSetup::sm_pfIntValues = NULL;
LDVExtensionsSetup *LDVExtensionsSetup::sm_extensionsSetup = NULL;

LDVExtensionsSetup::LDVExtensionsSetupCleanup
	LDVExtensionsSetup::sm_extensionsSetupCleanup;

static char const *pfIntAttribNames[] =
{
	"WGL_MAX_PBUFFER_WIDTH_ARB",
	"WGL_DRAW_TO_WINDOW_ARB",
	"WGL_DRAW_TO_BITMAP_ARB",
	"WGL_ACCELERATION_ARB",
	"WGL_NEED_PALETTE_ARB",
	"WGL_NEED_SYSTEM_PALETTE_ARB",
	"WGL_SWAP_LAYER_BUFFERS_ARB",
	"WGL_SWAP_METHOD_ARB",
	"WGL_NUMBER_OVERLAYS_ARB",
	"WGL_NUMBER_UNDERLAYS_ARB",
	"WGL_TRANSPARENT_ARB",
	"WGL_TRANSPARENT_RED_VALUE_ARB",
	"WGL_TRANSPARENT_GREEN_VALUE_ARB",
	"WGL_TRANSPARENT_BLUE_VALUE_ARB",
	"WGL_TRANSPARENT_ALPHA_VALUE_ARB",
	"WGL_TRANSPARENT_INDEX_VALUE_ARB",
	"WGL_SHARE_DEPTH_ARB",
	"WGL_SHARE_STENCIL_ARB",
	"WGL_SHARE_ACCUM_ARB",
	"WGL_SUPPORT_GDI_ARB",
	"WGL_SUPPORT_OPENGL_ARB",
	"WGL_DOUBLE_BUFFER_ARB",
	"WGL_STEREO_ARB",
	"WGL_PIXEL_TYPE_ARB",
	"WGL_COLOR_BITS_ARB",
	"WGL_RED_BITS_ARB",
	"WGL_RED_SHIFT_ARB",
	"WGL_GREEN_BITS_ARB",
	"WGL_GREEN_SHIFT_ARB",
	"WGL_BLUE_BITS_ARB",
	"WGL_BLUE_SHIFT_ARB",
	"WGL_ALPHA_BITS_ARB",
	"WGL_ALPHA_SHIFT_ARB",
	"WGL_ACCUM_BITS_ARB",
	"WGL_ACCUM_RED_BITS_ARB",
	"WGL_ACCUM_GREEN_BITS_ARB",
	"WGL_ACCUM_BLUE_BITS_ARB",
	"WGL_ACCUM_ALPHA_BITS_ARB",
	"WGL_DEPTH_BITS_ARB",
	"WGL_STENCIL_BITS_ARB",
	"WGL_AUX_BUFFERS_ARB",
	"WGL_DRAW_TO_PBUFFER_ARB",
	"WGL_SAMPLE_BUFFERS_EXT",
	"WGL_SAMPLES_EXT",
};
static int pfIntAttribs[] =
{
	WGL_MAX_PBUFFER_WIDTH_ARB,
	WGL_DRAW_TO_WINDOW_ARB,
	WGL_DRAW_TO_BITMAP_ARB,
	WGL_ACCELERATION_ARB,
	WGL_NEED_PALETTE_ARB,
	WGL_NEED_SYSTEM_PALETTE_ARB,
	WGL_SWAP_LAYER_BUFFERS_ARB,
	WGL_SWAP_METHOD_ARB,
	WGL_NUMBER_OVERLAYS_ARB,
	WGL_NUMBER_UNDERLAYS_ARB,
	WGL_TRANSPARENT_ARB,
	WGL_TRANSPARENT_RED_VALUE_ARB,
	WGL_TRANSPARENT_GREEN_VALUE_ARB,
	WGL_TRANSPARENT_BLUE_VALUE_ARB,
	WGL_TRANSPARENT_ALPHA_VALUE_ARB,
	WGL_TRANSPARENT_INDEX_VALUE_ARB,
	WGL_SHARE_DEPTH_ARB,
	WGL_SHARE_STENCIL_ARB,
	WGL_SHARE_ACCUM_ARB,
	WGL_SUPPORT_GDI_ARB,
	WGL_SUPPORT_OPENGL_ARB,
	WGL_DOUBLE_BUFFER_ARB,
	WGL_STEREO_ARB,
	WGL_PIXEL_TYPE_ARB,
	WGL_COLOR_BITS_ARB,
	WGL_RED_BITS_ARB,
	WGL_RED_SHIFT_ARB,
	WGL_GREEN_BITS_ARB,
	WGL_GREEN_SHIFT_ARB,
	WGL_BLUE_BITS_ARB,
	WGL_BLUE_SHIFT_ARB,
	WGL_ALPHA_BITS_ARB,
	WGL_ALPHA_SHIFT_ARB,
	WGL_ACCUM_BITS_ARB,
	WGL_ACCUM_RED_BITS_ARB,
	WGL_ACCUM_GREEN_BITS_ARB,
	WGL_ACCUM_BLUE_BITS_ARB,
	WGL_ACCUM_ALPHA_BITS_ARB,
	WGL_DEPTH_BITS_ARB,
	WGL_STENCIL_BITS_ARB,
	WGL_AUX_BUFFERS_ARB,
	WGL_DRAW_TO_PBUFFER_ARB,
	WGL_SAMPLE_BUFFERS_EXT,
	WGL_SAMPLES_EXT,
};
static int pfIntAttribCount = COUNT_OF(pfIntAttribs);

LDVExtensionsSetup::LDVExtensionsSetupCleanup::~LDVExtensionsSetupCleanup(void)
{
	LDVExtensionsSetup::sm_wglExtensions.clear();
	delete LDVExtensionsSetup::sm_wglExtensionsString;
	LDVExtensionsSetup::sm_wglExtensionsString = NULL;
	if (LDVExtensionsSetup::sm_fsaaModes)
	{
		LDVExtensionsSetup::sm_fsaaModes->release();
	}
	if (LDVExtensionsSetup::sm_extensionsSetup)
	{
        LDVExtensionsSetup::sm_extensionsSetup->release();
		LDVExtensionsSetup::sm_extensionsSetup = NULL;
	}
	if (LDVExtensionsSetup::sm_pfIntValues)
	{
		LDVExtensionsSetup::sm_pfIntValues->release();
		LDVExtensionsSetup::sm_pfIntValues = NULL;
	}
}


LDVExtensionsSetup::LDVExtensionsSetup(HWND hWnd, HINSTANCE hInstance ):
    hInstance(hInstance),
    hdc(GetDC(hWnd)),
    hWnd(hWnd)
{
}

LDVExtensionsSetup::~LDVExtensionsSetup(void)
{
}

BOOL LDVExtensionsSetup::initExtensions(void)
{
    if (sm_extensionsSetup)
    {
		GLint intValue;

		TREGLExtensions::setup();
		if (!sm_wglExtensionsString)
		{
			sm_wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)
				wglGetProcAddress("wglGetExtensionsStringARB");

			if (!sm_wglGetExtensionsStringARB)
			{
				sm_wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)
					wglGetProcAddress("wglGetExtensionsStringEXT");
			}
			if (sm_wglGetExtensionsStringARB)
			{
				sm_wglExtensionsString = copyString(
					sm_wglGetExtensionsStringARB(hdc));
				TREGLExtensions::initExtensions(sm_wglExtensions,
					sm_wglExtensionsString);
			}
		}
		// Note that when we load the function pointers, don't want to pay
		// attention to any ignore flags in the registry, so all the checks for
		// extensions have the force flag set to true.  Otherwise, if the
		// program starts with the ignore flag set, and it later gets cleared,
		// the function pointers won't be loaded.
        if (checkForWGLExtension((char *)"WGL_ARB_pixel_format", true))
		{
			sm_wglGetPixelFormatAttribivARB = (PFNWGLGETPIXELFORMATATTRIBIVEXTPROC)
				wglGetProcAddress("wglGetPixelFormatAttribivARB");
			sm_wglGetPixelFormatAttribfvARB = (PFNWGLGETPIXELFORMATATTRIBFVEXTPROC)
				wglGetProcAddress("wglGetPixelFormatAttribfvARB");
			sm_wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATEXTPROC)
				wglGetProcAddress("wglChoosePixelFormatARB");
		}
		if (havePixelBufferExtension(true))
		{
			sm_wglCreatePbufferARB = (PFNWGLCREATEPBUFFERARBPROC)
				wglGetProcAddress("wglCreatePbufferARB");
			sm_wglGetPbufferDCARB = (PFNWGLGETPBUFFERDCARBPROC)
				wglGetProcAddress("wglGetPbufferDCARB");
			sm_wglReleasePbufferDCARB = (PFNWGLRELEASEPBUFFERDCARBPROC)
				wglGetProcAddress("wglReleasePbufferDCARB");
			sm_wglDestroyPbufferARB = (PFNWGLDESTROYPBUFFERARBPROC)
				wglGetProcAddress("wglDestroyPbufferARB");
			sm_wglQueryPbufferARB = (PFNWGLQUERYPBUFFERARBPROC)
				wglGetProcAddress("wglQueryPbufferARB");
		}
		glGetIntegerv(GL_STENCIL_BITS, &intValue);
		if (intValue)
		{
			sm_stencilPresent = true;
		}
		glGetIntegerv(GL_ALPHA_BITS, &intValue);
		if (intValue)
		{
			sm_alphaPresent = true;
		}
		recordPixelFormats();
		scanFSAAModes();

		using namespace TREGLExtensionsNS;
		// WGL_EXT_pixel_format
		wglGetPixelFormatAttribivARB = sm_wglGetPixelFormatAttribivARB;
		wglGetPixelFormatAttribfvARB = sm_wglGetPixelFormatAttribfvARB;
		wglChoosePixelFormatARB = NULL;
		// WGL_ARB_extensions_string
		wglGetExtensionsStringARB = sm_wglGetExtensionsStringARB;
		// WGL_ARB_pbuffer
		wglCreatePbufferARB = sm_wglCreatePbufferARB;
		wglGetPbufferDCARB = sm_wglGetPbufferDCARB;
		wglReleasePbufferDCARB = sm_wglReleasePbufferDCARB;
		wglDestroyPbufferARB = sm_wglDestroyPbufferARB;
		wglQueryPbufferARB = sm_wglQueryPbufferARB;
		// WGL_NV_allocate_memory
		wglAllocateMemoryNV = sm_wglAllocateMemoryNV;
		wglFreeMemoryNV = sm_wglFreeMemoryNV;
		return TRUE;
    }
    return FALSE;
}

void LDVExtensionsSetup::recordPixelFormats(void)
{
	if (havePixelFormatExtension() && !sm_pfIntValues)
	{
		GLint intValues[] = {
			WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
			WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
			WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
			WGL_COLOR_BITS_ARB, 16,
			WGL_DEPTH_BITS_ARB, 16,
			0, 0
		};
		GLfloat floatValues[] = { 0.0f, 0.0f };
		GLint indexes[1024];
		GLuint count;

		sm_pfIntValues = new TCIntArrayArray;
		// Get a list of all acceptable pixel formats.
		if (sm_wglChoosePixelFormatARB(hdc, intValues,
			floatValues, COUNT_OF(indexes), indexes, &count)
			&& count)
		{
			GLuint i;
			GLint *values = new GLint[pfIntAttribCount];

			// Record info about all pixel formats.
			for (i = 0; i < count; i++)
			{
				TCIntArray *valueArray = new TCIntArray;
				int j;

				memset(values, -1, pfIntAttribCount * sizeof(GLint));
				sm_wglGetPixelFormatAttribivARB(hdc, indexes[i], 0,
					pfIntAttribCount, pfIntAttribs, values);
				for (j = 0; j < pfIntAttribCount; j++)
				{
					valueArray->addValue(values[j]);
				}
				valueArray->addValue(indexes[i]);
				sm_pfIntValues->addObject(valueArray);
				valueArray->release();
			}
            delete[] values;
		}
	}
}

void LDVExtensionsSetup::scanFSAAModes(void)
{
	if (sm_fsaaModes)
	{
		sm_fsaaModes->removeAll();
	}
	else
	{
		sm_fsaaModes = new TCIntArray;
	}
	if (haveMultisampleExtension())
	{
		GLint intValues[] = {
			WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
			WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
			WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
			WGL_COLOR_BITS_ARB, 16,
			WGL_DEPTH_BITS_ARB, 16,
			0, 0
		};
		GLfloat floatValues[] = { 0.0f, 0.0f };
		GLint indexes[1024];
		GLuint count;

		// Get a list of all acceptable pixel formats.
		if (sm_wglChoosePixelFormatARB(hdc, intValues,
			floatValues, COUNT_OF(indexes), indexes, &count)
			&& count)
		{
			GLuint i;
			GLint attributes[2] = { WGL_SAMPLE_BUFFERS_EXT, WGL_SAMPLES_EXT };
			GLint values[2];

			// Scan the list for multisample pixel formats.
			for (i = 0; i < count; i++)
			{
				sm_wglGetPixelFormatAttribivARB(hdc, indexes[i], 0, 2, attributes,
					values);
				if (values[0] && values[1] > 1)
				{
					int value = values[1];

					if (sm_fsaaModes->indexOfValue(value) == -1)
					{
						sm_fsaaModes->addValue(value);
					}
				}
			}
		}
		sm_fsaaModes->sort();
	}
}

bool LDVExtensionsSetup::haveMultisampleExtension(bool force)
{
	bool ignore = TCUserDefaults::longForKey(IGNORE_MULTISAMPLE_KEY, 0, false)
		!= 0;

    return (!ignore || force) && checkForWGLExtension((char*)"WGL_ARB_multisample");
}

bool LDVExtensionsSetup::havePixelBufferExtension(bool force)
{
	bool ignore = TCUserDefaults::longForKey(IGNORE_PBUFFER_KEY, 0, false) != 0;

    return (!ignore || force) && checkForWGLExtension((char*)"WGL_ARB_pbuffer", force);
}

bool LDVExtensionsSetup::havePixelFormatExtension(bool force)
{
	bool ignore = TCUserDefaults::longForKey(IGNORE_PIXEL_FORMAT_KEY, 0, false)
		!= 0;

    return (!ignore || force) && checkForWGLExtension((char*)"WGL_ARB_pixel_format");
}

bool LDVExtensionsSetup::checkForWGLExtension(char* extension, bool force)
{
	return TREGLExtensions::checkForExtension(sm_wglExtensions, extension, force);
}

void LDVExtensionsSetup::setup(HWND hWnd, HINSTANCE hInstance)
{
	if (!sm_performedInitialSetup)
	{
        sm_extensionsSetup = new LDVExtensionsSetup(hWnd, hInstance);
        sm_extensionsSetup->initExtensions();
		sm_performedInitialSetup = true;
	}
}

int LDVExtensionsSetup::matchPixelFormat(int *intValues)
{
	int count = sm_pfIntValues->getCount();
	int i;

	for (i = 0; i < count; i++)
	{
		int index = pixelFormatMatches(i, intValues);

		if (index != -1)
		{
			return index;
		}
	}
	return -1;
}

int LDVExtensionsSetup::pixelFormatMatches(int index, int *intValues)
{
	if (sm_pfIntValues)
	{
		int i, j;
		TCIntArray *pfValues = (*sm_pfIntValues)[index];

		for (i = 0; intValues[i]; i += 2)
		{
			int attrib = intValues[i];
			int value = intValues[i + 1];

			if (value)
			{
				for (j = 0; j < pfIntAttribCount; j++)
				{
					if (pfIntAttribs[j] == attrib)
					{
						if ((*pfValues)[j] < value)
						{
							return -1;
						}
						break;
					}
				}
			}
		}
		return (*pfValues)[pfIntAttribCount];
	}
	else
	{
		return -1;
	}
}

int LDVExtensionsSetup::choosePixelFormat(HDC hdc, GLint customValues[])
{
	GLint commonValues[] = {
		WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
		WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
		WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
		WGL_COLOR_BITS_ARB, 16,
		WGL_DEPTH_BITS_ARB, 16,
		0, 0
	};
	GLint *intValues;
	GLfloat floatValues[] = { 0.0f, 0.0f };
	GLint indexes[100];
	GLuint count;
	int customSize;
	int retValue = -1;
	int i;

	for (count = 0; customValues[count]; count++)
	{
		// Do nothing; we just want to know how many values there are.
	}
	customSize = count * sizeof(GLint);
	intValues = new GLint[customSize + sizeof(commonValues)];
	memcpy(intValues, customValues, customSize);
	memcpy(intValues + count, commonValues, sizeof(commonValues));
	for (i = 0; intValues[i]; i += 2)
	{
		debugPrintf("%d %d\n", intValues[i], intValues[i + 1]);
	}
	debugPrintf("%d %d\n", intValues[i], intValues[i + 1]);
	for (i = 0; floatValues[i]; i += 2)
	{
		debugPrintf("%g %g\n", floatValues[i], floatValues[i + 1]);
	}
	debugPrintf("%g %g\n", floatValues[i], floatValues[i + 1]);
	if (sm_wglChoosePixelFormatARB(hdc, intValues,
		floatValues, 100, indexes, &count))
	{
		if (count)
		{
			printPixelFormats(indexes, count);
			printPixelFormat(hdc, indexes[0]);
			retValue = indexes[0];
		}
		else
		{
			// There seems to be a bug in my ATI drivers that causes
			// sm_wglChoosePixelFormatARB to stop working, so use my own matching
			// code if the standard matching code fails.
			retValue = matchPixelFormat(intValues);
			debugPrintf("matchPixelFormat returned: %d\n", retValue);
			printPixelFormat(hdc, indexes[0]);
		}
	}
	delete intValues;
	return retValue;
}

void LDVExtensionsSetup::printPixelFormats(int *indexes, GLuint count)
{
	debugPrintf("Got %d pixel formats.\n", count);
	for (unsigned int i = 0; i < count; i++)
	{
		debugPrintf("%5d", indexes[i]);
	}
	debugPrintf("\n");
}

void LDVExtensionsSetup::printPixelFormat(HDC hdc, int index)
{
	int i;
	int values[1024];

	memset(values, -1, sizeof(values));
	sm_wglGetPixelFormatAttribivARB(hdc, index, 0, pfIntAttribCount, pfIntAttribs,
		values);
	for (i = 0; i < pfIntAttribCount; i++)
	{
		debugPrintf("%31s: 0x%08X %d\n", pfIntAttribNames[i], values[i],
			values[i]);
	}
}
