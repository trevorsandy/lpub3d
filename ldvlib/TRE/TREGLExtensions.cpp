#include "TREGLExtensions.h"
#include <TCFoundation/mystring.h>
#include <TCFoundation/TCUserDefaults.h>
// !!!WARNING!!!
// The following is kind of bad form, but I'm going to do it anyway.  I'm
// including a header file from a library that is dependent on this library.
// This works fine, because I'm not using any classes in the other library,
// just #defines, but it is still very strange behavior.
#include <LDLib/LDUserDefaultsKeys.h>
#include <stdio.h>

#ifdef WIN32
#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
#define new DEBUG_CLIENTBLOCK
#endif // _DEBUG
#endif // WIN32

namespace TREGLExtensionsNS
{
#ifndef GL_GLEXT_PROTOTYPES
	// GL_NV_vertex_array_range
	PFNGLVERTEXARRAYRANGENVPROC glVertexArrayRangeNV = NULL;
	// GL_EXT_multi_draw_arrays
	PFNGLMULTIDRAWELEMENTSEXTPROC glMultiDrawElementsEXT = NULL;
	// GL_ARB_vertex_buffer_object
	PFNGLBINDBUFFERARBPROC glBindBufferARB = NULL;
	PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB = NULL;
	PFNGLGENBUFFERSARBPROC glGenBuffersARB = NULL;
	PFNGLISBUFFERARBPROC glIsBufferARB = NULL;
	PFNGLBUFFERDATAARBPROC glBufferDataARB = NULL;
	PFNGLBUFFERSUBDATAARBPROC glBufferSubDataARB = NULL;
	PFNGLGETBUFFERSUBDATAARBPROC glGetBufferSubDataARB = NULL;
	PFNGLMAPBUFFERARBPROC glMapBufferARB = NULL;
	PFNGLUNMAPBUFFERARBPROC glUnmapBufferARB = NULL;
	PFNGLGETBUFFERPARAMETERIVARBPROC glGetBufferParameterivARB = NULL;
	PFNGLGETBUFFERPOINTERVARBPROC glGetBufferPointervARB = NULL;
	// GL_ARB_occlusion_query
	PFNGLGENQUERIESARBPROC glGenQueriesARB = NULL;
	PFNGLDELETEQUERIESARBPROC glDeleteQueriesARB = NULL;
	PFNGLISQUERYARBPROC glIsQueryARB = NULL;
	PFNGLBEGINQUERYARBPROC glBeginQueryARB = NULL;
	PFNGLENDQUERYARBPROC glEndQueryARB = NULL;
	PFNGLGETQUERYIVARBPROC glGetQueryivARB = NULL;
	PFNGLGETQUERYOBJECTIVARBPROC glGetQueryObjectivARB = NULL;
	PFNGLGETQUERYOBJECTUIVARBPROC glGetQueryObjectuivARB = NULL;
	// GL_EXT_framebuffer_object
	PFNGLISRENDERBUFFEREXTPROC glIsRenderbufferEXT = NULL;
	PFNGLBINDRENDERBUFFEREXTPROC glBindRenderbufferEXT = NULL;
	PFNGLDELETERENDERBUFFERSEXTPROC glDeleteRenderbuffersEXT = NULL;
	PFNGLGENRENDERBUFFERSEXTPROC glGenRenderbuffersEXT = NULL;
	PFNGLRENDERBUFFERSTORAGEEXTPROC glRenderbufferStorageEXT = NULL;
	PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC
		glGetRenderbufferParameterivEXT = NULL;
	PFNGLISFRAMEBUFFEREXTPROC glIsFramebufferEXT = NULL;
	PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebufferEXT = NULL;
	PFNGLDELETEFRAMEBUFFERSEXTPROC glDeleteFramebuffersEXT = NULL;
	PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffersEXT = NULL;
	PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatusEXT = NULL;
	PFNGLFRAMEBUFFERTEXTURE1DEXTPROC glFramebufferTexture1DEXT = NULL;
	PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2DEXT = NULL;
	PFNGLFRAMEBUFFERTEXTURE3DEXTPROC glFramebufferTexture3DEXT = NULL;
	PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC glFramebufferRenderbufferEXT = NULL;
	PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC
		glGetFramebufferAttachmentParameterivEXT = NULL;
	PFNGLGENERATEMIPMAPEXTPROC glGenerateMipmapEXT = NULL;
#endif // GL_GLEXT_PROTOTYPES

#ifdef WIN32
	// WGL_EXT_pixel_format
	PFNWGLGETPIXELFORMATATTRIBIVEXTPROC wglGetPixelFormatAttribivARB = NULL;
	PFNWGLGETPIXELFORMATATTRIBFVEXTPROC wglGetPixelFormatAttribfvARB = NULL;
	PFNWGLCHOOSEPIXELFORMATEXTPROC wglChoosePixelFormatARB = NULL;
	// WGL_ARB_extensions_string
	PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB = NULL;
	// WGL_ARB_pbuffer
	PFNWGLCREATEPBUFFERARBPROC wglCreatePbufferARB = NULL;
	PFNWGLGETPBUFFERDCARBPROC wglGetPbufferDCARB = NULL;
	PFNWGLRELEASEPBUFFERDCARBPROC wglReleasePbufferDCARB = NULL;
	PFNWGLDESTROYPBUFFERARBPROC wglDestroyPbufferARB = NULL;
	PFNWGLQUERYPBUFFERARBPROC wglQueryPbufferARB = NULL;
#endif // WIN32
	// WGL_NV_allocate_memory
	PFNWGLALLOCATEMEMORYNVPROC wglAllocateMemoryNV = NULL;
	PFNWGLFREEMEMORYNVPROC wglFreeMemoryNV = NULL;
}

// GL_NV_vertex_array_range
PFNGLVERTEXARRAYRANGENVPROC TREGLExtensions::sm_glVertexArrayRangeNV = NULL;
// GL_EXT_multi_draw_arrays
PFNGLMULTIDRAWELEMENTSEXTPROC TREGLExtensions::sm_glMultiDrawElementsEXT = NULL;
// GL_ARB_vertex_buffer_object
PFNGLBINDBUFFERARBPROC TREGLExtensions::sm_glBindBufferARB = NULL;
PFNGLDELETEBUFFERSARBPROC TREGLExtensions::sm_glDeleteBuffersARB = NULL;
PFNGLGENBUFFERSARBPROC TREGLExtensions::sm_glGenBuffersARB = NULL;
PFNGLISBUFFERARBPROC TREGLExtensions::sm_glIsBufferARB = NULL;
PFNGLBUFFERDATAARBPROC TREGLExtensions::sm_glBufferDataARB = NULL;
PFNGLBUFFERSUBDATAARBPROC TREGLExtensions::sm_glBufferSubDataARB = NULL;
PFNGLGETBUFFERSUBDATAARBPROC TREGLExtensions::sm_glGetBufferSubDataARB = NULL;
PFNGLMAPBUFFERARBPROC TREGLExtensions::sm_glMapBufferARB = NULL;
PFNGLUNMAPBUFFERARBPROC TREGLExtensions::sm_glUnmapBufferARB = NULL;
PFNGLGETBUFFERPARAMETERIVARBPROC TREGLExtensions::sm_glGetBufferParameterivARB =
	NULL;
PFNGLGETBUFFERPOINTERVARBPROC TREGLExtensions::sm_glGetBufferPointervARB = NULL;
// GL_ARB_occlusion_query
PFNGLGENQUERIESARBPROC TREGLExtensions::sm_glGenQueriesARB = NULL;
PFNGLDELETEQUERIESARBPROC TREGLExtensions::sm_glDeleteQueriesARB = NULL;
PFNGLISQUERYARBPROC TREGLExtensions::sm_glIsQueryARB = NULL;
PFNGLBEGINQUERYARBPROC TREGLExtensions::sm_glBeginQueryARB = NULL;
PFNGLENDQUERYARBPROC TREGLExtensions::sm_glEndQueryARB = NULL;
PFNGLGETQUERYIVARBPROC TREGLExtensions::sm_glGetQueryivARB = NULL;
PFNGLGETQUERYOBJECTIVARBPROC TREGLExtensions::sm_glGetQueryObjectivARB = NULL;
PFNGLGETQUERYOBJECTUIVARBPROC TREGLExtensions::sm_glGetQueryObjectuivARB = NULL;
// GL_EXT_framebuffer_object
PFNGLISRENDERBUFFEREXTPROC TREGLExtensions::sm_glIsRenderbufferEXT = NULL;
PFNGLBINDRENDERBUFFEREXTPROC TREGLExtensions::sm_glBindRenderbufferEXT = NULL;
PFNGLDELETERENDERBUFFERSEXTPROC TREGLExtensions::sm_glDeleteRenderbuffersEXT =
	NULL;
PFNGLGENRENDERBUFFERSEXTPROC TREGLExtensions::sm_glGenRenderbuffersEXT = NULL;
PFNGLRENDERBUFFERSTORAGEEXTPROC TREGLExtensions::sm_glRenderbufferStorageEXT =
	NULL;
PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC
	TREGLExtensions::sm_glGetRenderbufferParameterivEXT = NULL;
PFNGLISFRAMEBUFFEREXTPROC TREGLExtensions::sm_glIsFramebufferEXT = NULL;
PFNGLBINDFRAMEBUFFEREXTPROC TREGLExtensions::sm_glBindFramebufferEXT = NULL;
PFNGLDELETEFRAMEBUFFERSEXTPROC TREGLExtensions::sm_glDeleteFramebuffersEXT =
	NULL;
PFNGLGENFRAMEBUFFERSEXTPROC TREGLExtensions::sm_glGenFramebuffersEXT = NULL;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC
	TREGLExtensions::sm_glCheckFramebufferStatusEXT = NULL;
PFNGLFRAMEBUFFERTEXTURE1DEXTPROC TREGLExtensions::sm_glFramebufferTexture1DEXT =
	NULL;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC TREGLExtensions::sm_glFramebufferTexture2DEXT =
	NULL;
PFNGLFRAMEBUFFERTEXTURE3DEXTPROC TREGLExtensions::sm_glFramebufferTexture3DEXT =
	NULL;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC
	TREGLExtensions::sm_glFramebufferRenderbufferEXT = NULL;
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC
	TREGLExtensions::sm_glGetFramebufferAttachmentParameterivEXT = NULL;
PFNGLGENERATEMIPMAPEXTPROC TREGLExtensions::sm_glGenerateMipmapEXT = NULL;


StringSet TREGLExtensions::sm_glExtensions;
GLfloat TREGLExtensions::sm_maxAnisoLevel = 1.0f;
bool TREGLExtensions::sm_rendererIsMesa = false;
bool TREGLExtensions::sm_tempDisable = false;
std::string TREGLExtensions::sm_version;
int TREGLExtensions::sm_versionMaj = -1;
int TREGLExtensions::sm_versionMin = -1;

TREGLExtensions::TREGLExtensionsCleanup TREGLExtensions::sm_extensionsCleanup;

#ifdef WIN32
#define GET_EXTENSION(name) wglGetProcAddress(#name)
#else
#define GET_EXTENSION(name) (name)
#endif

TREGLExtensions::TREGLExtensionsCleanup::~TREGLExtensionsCleanup(void)
{
	TREGLExtensions::cleanup();
}

void TREGLExtensions::disableAll(bool disable)
{
	sm_tempDisable = disable;
}

void TREGLExtensions::cleanup(void)
{
	sm_glExtensions.clear();
}

void TREGLExtensions::initExtensions(
	StringSet &extensions,
	const char *extensionsString)
{
	int count;
	char **components = componentsSeparatedByString(extensionsString, " ",
		count);

	for (int i = 0; i < count; i++)
	{
		extensions.insert(components[i]);
	}
	deleteStringArray(components, count);
}

void TREGLExtensions::setup(void)
{
	cleanup();
	sm_version = (const char *)glGetString(GL_VERSION);
	if (sscanf(sm_version.c_str(), "%d.%d", &sm_versionMaj, &sm_versionMin) !=
		2)
	{
		sm_versionMaj = -1;
		sm_versionMin = -1;
	}
	initExtensions(sm_glExtensions, (const char*)glGetString(GL_EXTENSIONS));
	// Note that when we load the function pointers, don't want to pay
	// attention to any ignore flags in the registry, so all the checks for
	// extensions have the force flag set to true.  Otherwise, if the
	// program starts with the ignore flag set, and it later gets cleared,
	// the function pointers won't be loaded.
	if (haveVARExtension(true))
	{
#ifdef WIN32
		// NOTE: I should really change the __APPLE__ path to do their
		// equivalent of wglGetProcAddress.  However, since this extension won't
		// work anyway without a bunch of glX stuff that I'm not going to do,
		// I'm not going to make that change just for this.
		sm_glVertexArrayRangeNV = (PFNGLVERTEXARRAYRANGENVPROC)
			GET_EXTENSION(glVertexArrayRangeNV);
#endif // WIN32
	}
	const char *renderer = (const char *)glGetString(GL_RENDERER);
	if (stringHasCaseInsensitivePrefix(renderer, "Mesa "))
	{
		sm_rendererIsMesa = true;
	}
	if (haveMultiDrawArraysExtension(true))
	{
		sm_glMultiDrawElementsEXT = (PFNGLMULTIDRAWELEMENTSEXTPROC)
			GET_EXTENSION(glMultiDrawElementsEXT);
	}
	if (haveVBOExtension(true))
	{
		sm_glBindBufferARB = (PFNGLBINDBUFFERARBPROC)
			GET_EXTENSION(glBindBufferARB);
		sm_glDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC)
			GET_EXTENSION(glDeleteBuffersARB);
		sm_glGenBuffersARB = (PFNGLGENBUFFERSARBPROC)
			GET_EXTENSION(glGenBuffersARB);
		sm_glIsBufferARB = (PFNGLISBUFFERARBPROC)
			GET_EXTENSION(glIsBufferARB);
		sm_glBufferDataARB = (PFNGLBUFFERDATAARBPROC)
			GET_EXTENSION(glBufferDataARB);
		sm_glBufferSubDataARB = (PFNGLBUFFERSUBDATAARBPROC)
			GET_EXTENSION(glBufferSubDataARB);
		sm_glGetBufferSubDataARB = (PFNGLGETBUFFERSUBDATAARBPROC)
			GET_EXTENSION(glGetBufferSubDataARB);
		sm_glMapBufferARB = (PFNGLMAPBUFFERARBPROC)
			GET_EXTENSION(glMapBufferARB);
		sm_glUnmapBufferARB = (PFNGLUNMAPBUFFERARBPROC)
			GET_EXTENSION(glUnmapBufferARB);
		sm_glGetBufferParameterivARB = (PFNGLGETBUFFERPARAMETERIVARBPROC)
			GET_EXTENSION(glGetBufferParameterivARB);
		sm_glGetBufferPointervARB = (PFNGLGETBUFFERPOINTERVARBPROC)
			GET_EXTENSION(glGetBufferPointervARB);
	}
	if (haveOcclusionQueryExtension(true))
	{
		sm_glGenQueriesARB = (PFNGLGENQUERIESARBPROC)
			GET_EXTENSION(glGenQueriesARB);
		sm_glDeleteQueriesARB = (PFNGLDELETEQUERIESARBPROC)
			GET_EXTENSION(glDeleteQueriesARB);
		sm_glIsQueryARB = (PFNGLISQUERYARBPROC)
			GET_EXTENSION(glIsQueryARB);
		sm_glBeginQueryARB = (PFNGLBEGINQUERYARBPROC)
			GET_EXTENSION(glBeginQueryARB);
		sm_glEndQueryARB = (PFNGLENDQUERYARBPROC)
			GET_EXTENSION(glEndQueryARB);
		sm_glGetQueryivARB = (PFNGLGETQUERYIVARBPROC)
			GET_EXTENSION(glGetQueryivARB);
		sm_glGetQueryObjectivARB = (PFNGLGETQUERYOBJECTIVARBPROC)
			GET_EXTENSION(glGetQueryObjectivARB);
		sm_glGetQueryObjectuivARB = (PFNGLGETQUERYOBJECTUIVARBPROC)
			GET_EXTENSION(glGetQueryObjectuivARB);
	}
	if (haveAnisoExtension(true))
	{
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &sm_maxAnisoLevel);
	}
	else
	{
		sm_maxAnisoLevel = 1.0f;
	}
	if (haveFramebufferObjectExtension(true))
	{
		sm_glIsRenderbufferEXT = (PFNGLISRENDERBUFFEREXTPROC)
			GET_EXTENSION(glIsRenderbufferEXT);
		sm_glBindRenderbufferEXT = (PFNGLBINDRENDERBUFFEREXTPROC)
			GET_EXTENSION(glBindRenderbufferEXT);
		sm_glDeleteRenderbuffersEXT = (PFNGLDELETERENDERBUFFERSEXTPROC)
			GET_EXTENSION(glDeleteRenderbuffersEXT);
		sm_glGenRenderbuffersEXT = (PFNGLGENRENDERBUFFERSEXTPROC)
			GET_EXTENSION(glGenRenderbuffersEXT);
		sm_glRenderbufferStorageEXT = (PFNGLRENDERBUFFERSTORAGEEXTPROC)
			GET_EXTENSION(glRenderbufferStorageEXT);
		sm_glGetRenderbufferParameterivEXT = (PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC)
			GET_EXTENSION(glGetRenderbufferParameterivEXT);
		sm_glIsFramebufferEXT = (PFNGLISFRAMEBUFFEREXTPROC)
			GET_EXTENSION(glIsFramebufferEXT);
		sm_glBindFramebufferEXT = (PFNGLBINDFRAMEBUFFEREXTPROC)
			GET_EXTENSION(glBindFramebufferEXT);
		sm_glDeleteFramebuffersEXT = (PFNGLDELETEFRAMEBUFFERSEXTPROC)
			GET_EXTENSION(glDeleteFramebuffersEXT);
		sm_glGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC)
			GET_EXTENSION(glGenFramebuffersEXT);
		sm_glCheckFramebufferStatusEXT = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)
			GET_EXTENSION(glCheckFramebufferStatusEXT);
		sm_glFramebufferTexture1DEXT = (PFNGLFRAMEBUFFERTEXTURE1DEXTPROC)
			GET_EXTENSION(glFramebufferTexture1DEXT);
		sm_glFramebufferTexture2DEXT = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)
			GET_EXTENSION(glFramebufferTexture2DEXT);
		sm_glFramebufferTexture3DEXT = (PFNGLFRAMEBUFFERTEXTURE3DEXTPROC)
			GET_EXTENSION(glFramebufferTexture3DEXT);
		sm_glFramebufferRenderbufferEXT = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)
			GET_EXTENSION(glFramebufferRenderbufferEXT);
		sm_glGetFramebufferAttachmentParameterivEXT =
			(PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC)
			GET_EXTENSION(glGetFramebufferAttachmentParameterivEXT);
		sm_glGenerateMipmapEXT = (PFNGLGENERATEMIPMAPEXTPROC)
			GET_EXTENSION(glGenerateMipmapEXT);
	}
#ifndef GL_GLEXT_PROTOTYPES
	using namespace TREGLExtensionsNS;
	glVertexArrayRangeNV = sm_glVertexArrayRangeNV;
	// GL_EXT_multi_draw_arrays
	glMultiDrawElementsEXT = sm_glMultiDrawElementsEXT;
	// GL_ARB_vertex_buffer_object
	glBindBufferARB = sm_glBindBufferARB;
	glDeleteBuffersARB = sm_glDeleteBuffersARB;
	glGenBuffersARB = sm_glGenBuffersARB;
	glIsBufferARB = sm_glIsBufferARB;
	glBufferDataARB = sm_glBufferDataARB;
	glBufferSubDataARB = sm_glBufferSubDataARB;
	glGetBufferSubDataARB = sm_glGetBufferSubDataARB;
	glMapBufferARB = sm_glMapBufferARB;
	glUnmapBufferARB = sm_glUnmapBufferARB;
	glGetBufferParameterivARB = sm_glGetBufferParameterivARB;
	glGetBufferPointervARB = sm_glGetBufferPointervARB;
	// GL_ARB_occlusion_query
	glGenQueriesARB = sm_glGenQueriesARB;
	glDeleteQueriesARB = sm_glDeleteQueriesARB;
	glIsQueryARB = sm_glIsQueryARB;
	glBeginQueryARB = sm_glBeginQueryARB;
	glEndQueryARB = sm_glEndQueryARB;
	glGetQueryivARB = sm_glGetQueryivARB;
	glGetQueryObjectivARB = sm_glGetQueryObjectivARB;
	glGetQueryObjectuivARB = sm_glGetQueryObjectuivARB;
	// GL_EXT_framebuffer_object
	glIsRenderbufferEXT = sm_glIsRenderbufferEXT;
	glBindRenderbufferEXT = sm_glBindRenderbufferEXT;
	glDeleteRenderbuffersEXT = sm_glDeleteRenderbuffersEXT;
	glGenRenderbuffersEXT = sm_glGenRenderbuffersEXT;
	glRenderbufferStorageEXT = sm_glRenderbufferStorageEXT;
	glGetRenderbufferParameterivEXT = sm_glGetRenderbufferParameterivEXT;
	glIsFramebufferEXT = sm_glIsFramebufferEXT;
	glBindFramebufferEXT = sm_glBindFramebufferEXT;
	glDeleteFramebuffersEXT = sm_glDeleteFramebuffersEXT;
	glGenFramebuffersEXT = sm_glGenFramebuffersEXT;
	glCheckFramebufferStatusEXT = sm_glCheckFramebufferStatusEXT;
	glFramebufferTexture1DEXT = sm_glFramebufferTexture1DEXT;
	glFramebufferTexture2DEXT = sm_glFramebufferTexture2DEXT;
	glFramebufferTexture3DEXT = sm_glFramebufferTexture3DEXT;
	glFramebufferRenderbufferEXT = sm_glFramebufferRenderbufferEXT;
	glGetFramebufferAttachmentParameterivEXT =
		sm_glGetFramebufferAttachmentParameterivEXT;
	glGenerateMipmapEXT = sm_glGenerateMipmapEXT;
#endif // GL_GLEXT_PROTOTYPES
#ifdef WIN32
	using namespace TREGLExtensionsNS;
	if (haveVARExtension(true))
	{
		wglAllocateMemoryNV = (PFNWGLALLOCATEMEMORYNVPROC)
			wglGetProcAddress("wglAllocateMemoryNV");
		wglFreeMemoryNV = (PFNWGLFREEMEMORYNVPROC)
			wglGetProcAddress("wglFreeMemoryNV");
	}
#endif // WIN32
}

bool TREGLExtensions::haveNvMultisampleFilterHintExtension(bool force)
{
	bool ignore = TCUserDefaults::boolForKey(IGNORE_MS_FILTER_HINT_KEY, false,
		false);

	return (!ignore || force) &&
		checkForExtension("GL_NV_multisample_filter_hint", force);
}

bool TREGLExtensions::haveVARExtension(bool force)
{
	bool ignore = TCUserDefaults::boolForKey(IGNORE_VAR_KEY, false, false);

	using namespace TREGLExtensionsNS;
	return (!ignore || force) && checkForExtension("GL_NV_vertex_array_range",
		force);// && wglAllocateMemoryNV != NULL && wglFreeMemoryNV != NULL;
}

bool TREGLExtensions::haveMultiDrawArraysExtension(bool force)
{
	if (sm_rendererIsMesa)
	{
		// This extension apparently sucks on Mesa.  As near as I can tell, it
		// won't work inside a display list.
		return false;
	}
#ifdef __APPLE__
	int one = 1;
	TCByte binary[] = {1,0,0,0};

	if (one != *(int*)binary)
	{
		// Doesn't work on PPC Mac (at least in Rosetta).  The above checks to
		// see if the current architecture is big endian or little endian.  If
		// it's big endian (PPC), we'll get here.
		return false;
	}
#endif
	bool ignore = TCUserDefaults::boolForKey(IGNORE_MULTI_DRAW_ARRAYS_KEY,
		false, false);

	return (!ignore || force) && checkForExtension("GL_EXT_multi_draw_arrays",
		force);
}

bool TREGLExtensions::haveOcclusionQueryExtension(bool force)
{
	bool ignore = TCUserDefaults::boolForKey(IGNORE_OCCLUSION_QUERY_KEY, false,
		false);

	return (!ignore || force) && checkForExtension("GL_ARB_occlusion_query",
		force);
}

bool TREGLExtensions::haveFramebufferObjectExtension(bool force)
{
	bool ignore = TCUserDefaults::boolForKey(IGNORE_FRAMEBUFFER_OBJECT_KEY,
		false, false);

	return (!ignore || force) && checkForExtension("GL_EXT_framebuffer_object",
		force);
}

bool TREGLExtensions::haveClampToBorderExtension(bool force)
{
	bool ignore = TCUserDefaults::boolForKey(IGNORE_CLAMP_TO_BORDER, false,
		false);

	return (!ignore || force) && checkForOGLVersion(1, 3, force);
}

bool TREGLExtensions::haveAnisoExtension(bool force)
{
	bool ignore = TCUserDefaults::boolForKey(IGNORE_ANISO_KEY, false, false);

	return (!ignore || force) &&
		checkForExtension("GL_EXT_texture_filter_anisotropic", force);
}

bool TREGLExtensions::haveVBOExtension(bool force)
{
	bool ignore = TCUserDefaults::boolForKey(IGNORE_VBO_KEY, false, false);

	return (!ignore || force) &&
		checkForExtension("GL_ARB_vertex_buffer_object", force);
}

bool TREGLExtensions::checkForExtension(
	const StringSet &extensions,
	const char* extension,
	bool force)
{
	bool ignore = TCUserDefaults::boolForKey(IGNORE_ALL_OGL_EXTENSIONS, false,
		false) || sm_tempDisable;

	if (!ignore || force)
	{
		return extensions.find(extension) != extensions.end();
	}
	return false;
}

bool TREGLExtensions::checkForExtension(const char* extension, bool force)
{
	return checkForExtension(sm_glExtensions, extension, force);
}

bool TREGLExtensions::checkForOGLVersion(
	int major,
	int minor,
	bool force /*= false*/)
{
	bool ignore = TCUserDefaults::boolForKey(IGNORE_ALL_OGL_EXTENSIONS, false,
		false) || sm_tempDisable;

	if (!ignore || force)
	{
		return sm_versionMaj > major ||
			(sm_versionMaj == major && sm_versionMin >= minor);
	}
	return false;
}

GLfloat TREGLExtensions::getMaxAnisoLevel(void)
{
	if (haveAnisoExtension())
	{
		return sm_maxAnisoLevel;
	}
	else
	{
		return 1.0f;
	}
}
