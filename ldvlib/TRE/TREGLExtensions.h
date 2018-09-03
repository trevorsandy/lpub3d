#ifndef __TREGLEXTENSIONS_H__
#define __TREGLEXTENSIONS_H__

#include <TCFoundation/TCObject.h>
#include <TCFoundation/TCStlIncludes.h>
#include <TRE/TREGL.h>

// Just in case.
#ifndef GL_CLAMP_TO_BORDER
#define GL_CLAMP_TO_BORDER                0x812D
#endif // GL_CLAMP_TO_BORDER
#ifndef GL_TEXTURE_FILTER_CONTROL_EXT
#define GL_TEXTURE_FILTER_CONTROL_EXT     0x8500
#endif // GL_TEXTURE_FILTER_CONTROL_EXT
#ifndef GL_TEXTURE_LOD_BIAS_EXT
#define GL_TEXTURE_LOD_BIAS_EXT           0x8501
#endif // GL_TEXTURE_LOD_BIAS_EXT

struct TREVertex;
class TREVertexArray;
class TCVector;

typedef std::set<std::string> StringSet;

namespace TREGLExtensionsNS
{
#ifndef GL_GLEXT_PROTOTYPES
	// GL_NV_vertex_array_range
	extern PFNGLVERTEXARRAYRANGENVPROC glVertexArrayRangeNV;
	// GL_EXT_multi_draw_arrays
	extern PFNGLMULTIDRAWELEMENTSEXTPROC glMultiDrawElementsEXT;
	// GL_ARB_vertex_buffer_object
	extern PFNGLBINDBUFFERARBPROC glBindBufferARB;
	extern PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB;
	extern PFNGLGENBUFFERSARBPROC glGenBuffersARB;
	extern PFNGLISBUFFERARBPROC glIsBufferARB;
	extern PFNGLBUFFERDATAARBPROC glBufferDataARB;
	extern PFNGLBUFFERSUBDATAARBPROC glBufferSubDataARB;
	extern PFNGLGETBUFFERSUBDATAARBPROC glGetBufferSubDataARB;
	extern PFNGLMAPBUFFERARBPROC glMapBufferARB;
	extern PFNGLUNMAPBUFFERARBPROC glUnmapBufferARB;
	extern PFNGLGETBUFFERPARAMETERIVARBPROC glGetBufferParameterivARB;
	extern PFNGLGETBUFFERPOINTERVARBPROC glGetBufferPointervARB;
	// GL_ARB_occlusion_query
	extern PFNGLGENQUERIESARBPROC glGenQueriesARB;
	extern PFNGLDELETEQUERIESARBPROC glDeleteQueriesARB;
	extern PFNGLISQUERYARBPROC glIsQueryARB;
	extern PFNGLBEGINQUERYARBPROC glBeginQueryARB;
	extern PFNGLENDQUERYARBPROC glEndQueryARB;
	extern PFNGLGETQUERYIVARBPROC glGetQueryivARB;
	extern PFNGLGETQUERYOBJECTIVARBPROC glGetQueryObjectivARB;
	extern PFNGLGETQUERYOBJECTUIVARBPROC glGetQueryObjectuivARB;
	// GL_EXT_framebuffer_object
	extern PFNGLISRENDERBUFFEREXTPROC glIsRenderbufferEXT;
	extern PFNGLBINDRENDERBUFFEREXTPROC glBindRenderbufferEXT;
	extern PFNGLDELETERENDERBUFFERSEXTPROC glDeleteRenderbuffersEXT;
	extern PFNGLGENRENDERBUFFERSEXTPROC glGenRenderbuffersEXT;
	extern PFNGLRENDERBUFFERSTORAGEEXTPROC glRenderbufferStorageEXT;
	extern PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC
		glGetRenderbufferParameterivEXT;
	extern PFNGLISFRAMEBUFFEREXTPROC glIsFramebufferEXT;
	extern PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebufferEXT;
	extern PFNGLDELETEFRAMEBUFFERSEXTPROC glDeleteFramebuffersEXT;
	extern PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffersEXT;
	extern PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatusEXT;
	extern PFNGLFRAMEBUFFERTEXTURE1DEXTPROC glFramebufferTexture1DEXT;
	extern PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2DEXT;
	extern PFNGLFRAMEBUFFERTEXTURE3DEXTPROC glFramebufferTexture3DEXT;
	extern PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC glFramebufferRenderbufferEXT;
	extern PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC
		glGetFramebufferAttachmentParameterivEXT;
	extern PFNGLGENERATEMIPMAPEXTPROC glGenerateMipmapEXT;
#endif // GL_GLEXT_PROTOTYPES

#ifdef WIN32
	// WGL_EXT_pixel_format
	extern PFNWGLGETPIXELFORMATATTRIBIVEXTPROC wglGetPixelFormatAttribivARB;
	extern PFNWGLGETPIXELFORMATATTRIBFVEXTPROC wglGetPixelFormatAttribfvARB;
	extern PFNWGLCHOOSEPIXELFORMATEXTPROC wglChoosePixelFormatARB;
	// WGL_ARB_extensions_string
	extern PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB;
	// WGL_ARB_pbuffer
	extern PFNWGLCREATEPBUFFERARBPROC wglCreatePbufferARB;
	extern PFNWGLGETPBUFFERDCARBPROC wglGetPbufferDCARB;
	extern PFNWGLRELEASEPBUFFERDCARBPROC wglReleasePbufferDCARB;
	extern PFNWGLDESTROYPBUFFERARBPROC wglDestroyPbufferARB;
	extern PFNWGLQUERYPBUFFERARBPROC wglQueryPbufferARB;
#endif // WIN32
	// Windows doesn't define GL_GLEXT_PROTOTYPES, and none of the WGL function
	// pointers will exist outside Windows, so always define.
	// WGL_NV_allocate_memory
	extern PFNWGLALLOCATEMEMORYNVPROC wglAllocateMemoryNV;
	extern PFNWGLFREEMEMORYNVPROC wglFreeMemoryNV;
}

class TREGLExtensions
{
public:
	static void setup(void);

	static bool checkForExtension(const char* extension,
		bool force = false);
	static bool checkForOGLVersion(int major, int minor, bool force = false);
	static bool checkForExtension(const StringSet &extensions,
		const char* extension, bool force = false);
	static bool haveNvMultisampleFilterHintExtension(bool force = false);
	static bool haveVARExtension(bool force = false);
	static bool haveMultiDrawArraysExtension(bool force = false);
	static bool haveVBOExtension(bool force = false);
	static bool haveAnisoExtension(bool force = false);
	static bool haveOcclusionQueryExtension(bool force = false);
	static bool haveFramebufferObjectExtension(bool force = false);
	static bool haveClampToBorderExtension(bool force = false);
	static GLfloat getMaxAnisoLevel(void);
	static void disableAll(bool disable);
	static void initExtensions(StringSet &extensions,
		const char *extensionsString);

protected:
	static void cleanup(void);

	// GL_NV_vertex_array_range
	static PFNGLVERTEXARRAYRANGENVPROC sm_glVertexArrayRangeNV;
	// GL_EXT_multi_draw_arrays
	static PFNGLMULTIDRAWELEMENTSEXTPROC sm_glMultiDrawElementsEXT;
	// GL_ARB_vertex_buffer_object
	static PFNGLBINDBUFFERARBPROC sm_glBindBufferARB;
	static PFNGLDELETEBUFFERSARBPROC sm_glDeleteBuffersARB;
	static PFNGLGENBUFFERSARBPROC sm_glGenBuffersARB;
	static PFNGLISBUFFERARBPROC sm_glIsBufferARB;
	static PFNGLBUFFERDATAARBPROC sm_glBufferDataARB;
	static PFNGLBUFFERSUBDATAARBPROC sm_glBufferSubDataARB;
	static PFNGLGETBUFFERSUBDATAARBPROC sm_glGetBufferSubDataARB;
	static PFNGLMAPBUFFERARBPROC sm_glMapBufferARB;
	static PFNGLUNMAPBUFFERARBPROC sm_glUnmapBufferARB;
	static PFNGLGETBUFFERPARAMETERIVARBPROC sm_glGetBufferParameterivARB;
	static PFNGLGETBUFFERPOINTERVARBPROC sm_glGetBufferPointervARB;
	// GL_ARB_occlusion_query
	static PFNGLGENQUERIESARBPROC sm_glGenQueriesARB;
	static PFNGLDELETEQUERIESARBPROC sm_glDeleteQueriesARB;
	static PFNGLISQUERYARBPROC sm_glIsQueryARB;
	static PFNGLBEGINQUERYARBPROC sm_glBeginQueryARB;
	static PFNGLENDQUERYARBPROC sm_glEndQueryARB;
	static PFNGLGETQUERYIVARBPROC sm_glGetQueryivARB;
	static PFNGLGETQUERYOBJECTIVARBPROC sm_glGetQueryObjectivARB;
	static PFNGLGETQUERYOBJECTUIVARBPROC sm_glGetQueryObjectuivARB;
	// GL_EXT_framebuffer_object
	static PFNGLISRENDERBUFFEREXTPROC sm_glIsRenderbufferEXT;
	static PFNGLBINDRENDERBUFFEREXTPROC sm_glBindRenderbufferEXT;
	static PFNGLDELETERENDERBUFFERSEXTPROC sm_glDeleteRenderbuffersEXT;
	static PFNGLGENRENDERBUFFERSEXTPROC sm_glGenRenderbuffersEXT;
	static PFNGLRENDERBUFFERSTORAGEEXTPROC sm_glRenderbufferStorageEXT;
	static PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC
		sm_glGetRenderbufferParameterivEXT;
	static PFNGLISFRAMEBUFFEREXTPROC sm_glIsFramebufferEXT;
	static PFNGLBINDFRAMEBUFFEREXTPROC sm_glBindFramebufferEXT;
	static PFNGLDELETEFRAMEBUFFERSEXTPROC sm_glDeleteFramebuffersEXT;
	static PFNGLGENFRAMEBUFFERSEXTPROC sm_glGenFramebuffersEXT;
	static PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC sm_glCheckFramebufferStatusEXT;
	static PFNGLFRAMEBUFFERTEXTURE1DEXTPROC sm_glFramebufferTexture1DEXT;
	static PFNGLFRAMEBUFFERTEXTURE2DEXTPROC sm_glFramebufferTexture2DEXT;
	static PFNGLFRAMEBUFFERTEXTURE3DEXTPROC sm_glFramebufferTexture3DEXT;
	static PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC sm_glFramebufferRenderbufferEXT;
	static PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC
		sm_glGetFramebufferAttachmentParameterivEXT;
	static PFNGLGENERATEMIPMAPEXTPROC sm_glGenerateMipmapEXT;

	static StringSet sm_glExtensions;
	static GLfloat sm_maxAnisoLevel;
	static bool sm_rendererIsMesa;
	static bool sm_tempDisable;
	static std::string sm_version;
	static int sm_versionMaj;
	static int sm_versionMin;

	static class TREGLExtensionsCleanup
	{
	public:
		~TREGLExtensionsCleanup(void);
	} sm_extensionsCleanup;
	friend class TREGLExtensionsCleanup;
};

#endif // __TREGLEXTENSIONS_H__
