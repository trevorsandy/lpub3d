#ifndef __TREGL_H__
#define __TREGL_H__

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#else // WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif // WIN32

#ifndef WIN32
//#ifndef __APPLE__
#define GL_GLEXT_PROTOTYPES
//#endif
#endif // WIN32

#ifdef __APPLE__
#  define GL_GLEXT_LEGACY
#  include <GLUT/GLUT.h>
#  define APIENTRY
#  ifdef _OSMESA
#    include <GL/gl.h>
#    include <GL/glext.h>
#    include <GL/glu.h>
#  else // _OSMESA
#    include <OpenGL/gl.h>
#    include "../include/GL/glext.h"
#    include <OpenGL/OpenGL.h>
#  endif // _OSMESA
#else	// __APPLE__
#  include <GL/gl.h>
#  include <GL/glext.h>
#  include <GL/glu.h>
#endif	// __APPLE__

#include <TCFoundation/TCDefines.h>

//Solaris
#if (defined (__SVR4) && defined (__sun)) 
#define APIENTRY
#endif

#ifdef WIN32
#include <GL/wglext.h>
#else // WIN32

#ifndef __APPLE__
#ifndef GL_ARB_vertex_buffer_object
typedef int GLsizeiptrARB;
#endif // !GL_ARB_vertex_buffer_object
#endif // !APPLE

//	WGL ext stuff
typedef void * (APIENTRY * PFNWGLALLOCATEMEMORYNVPROC) (int size, float readfreq, float writefreq, float priority);
typedef void (APIENTRY * PFNWGLFREEMEMORYNVPROC) (void *pointer);

//	ext stuff
typedef void (APIENTRY * PFNGLFLUSHVERTEXARRAYRANGENVPROC) (void);
typedef void (APIENTRY * PFNGLVERTEXARRAYRANGENVPROC) (GLsizei size, const GLvoid *pointer);
typedef void (APIENTRY * PFNGLBINDBUFFERARBPROC) (GLenum target, GLuint buffer);
typedef void (APIENTRY * PFNGLDELETEBUFFERSARBPROC) (GLsizei n, const GLuint *buffers);
typedef void (APIENTRY * PFNGLGENBUFFERSARBPROC) (GLsizei n, GLuint *buffers);
typedef void (APIENTRY * PFNGLBUFFERDATAARBPROC) (GLenum target, GLsizeiptrARB size, const GLvoid *data, GLenum usage);

//Solaris
#if (defined (__SVR4) && defined (__sun))
typedef GLvoid (APIENTRY * PFNGLMULTIDRAWELEMENTSEXTPROC) (GLenum mode, GLsizei *count, GLenum type, const GLvoid **indices, GLsizei primcount);
#endif

#define GL_ARRAY_BUFFER_ARB 0x8892
#define GL_STATIC_DRAW_ARB 0x88E4

#ifndef GL_VERTEX_ARRAY_RANGE_NV
#define GL_VERTEX_ARRAY_RANGE_NV 0x851D
#endif

#endif // WIN32

#ifdef LDVIEW_DOUBLES
#define treGlMultMatrixf glMultMatrixd
#define treGlTranslatef glTranslated
#define treGlVertex3fv glVertex3dv
#define treGlVertex3f glVertex3d
#define treGlGetFloatv glGetDoublev
#define treGlTexCoord2f glTexCoord2d
#define treGlRotatef glRotated
#define TRE_GL_FLOAT GL_DOUBLE
#else // LDVIEW_DOUBLES
#define treGlMultMatrixf glMultMatrixf
#define treGlTranslatef glTranslatef
#define treGlVertex3fv glVertex3fv
#define treGlVertex3f glVertex3f
#define treGlGetFloatv glGetFloatv
#define treGlTexCoord2f glTexCoord2f
#define treGlRotatef glRotatef
#define TRE_GL_FLOAT GL_FLOAT
#endif // LDVIEW_DOUBLES

#endif // __TREGL_H__
