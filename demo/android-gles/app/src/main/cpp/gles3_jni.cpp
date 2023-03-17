#include <jni.h>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <android/asset_manager_jni.h>

#include "gles3_jni.h"

static void print_gl_string(const char *name, GLenum s) {
    const char *v = (const char *) glGetString(s);
    ALOGV("GL %s: %s\n", name, v);
}

// ----------------------------------------------------------------------------

RendererES::RendererES(AAssetManager *_asset_manager) {
    asset_manager = _asset_manager;
}

RendererES::~RendererES() = default;

void RendererES::resize(int width, int height) {}

void RendererES::render() {}

// ----------------------------------------------------------------------------

static RendererES *g_renderer = nullptr;

extern "C" {
JNIEXPORT void JNICALL
Java_graphics_pathfinder_gles_GLES3JNILib_init(JNIEnv *env, jobject obj, jint width, jint height,
                                               jobject asset_manager);
JNIEXPORT void JNICALL
Java_graphics_pathfinder_gles_GLES3JNILib_resize(JNIEnv *env, jobject obj, jint width, jint height);
JNIEXPORT void JNICALL Java_graphics_pathfinder_gles_GLES3JNILib_step(JNIEnv *env, jobject obj);
}

#if !defined(DYNAMIC_ES3)

static GLboolean gl3stubInit() {
    return GL_TRUE;
}

#endif

JNIEXPORT void JNICALL
Java_graphics_pathfinder_gles_GLES3JNILib_init(JNIEnv *env, jobject obj, jint width, jint height,
                                               jobject asset_manager) {
    if (g_renderer) {
        delete g_renderer;
        g_renderer = nullptr;
    }

    print_gl_string("Version", GL_VERSION);
    print_gl_string("Vendor", GL_VENDOR);
    print_gl_string("Renderer", GL_RENDERER);
    print_gl_string("Extensions", GL_EXTENSIONS);

    AAssetManager *asset_manager0 = AAssetManager_fromJava(env, asset_manager);

    const char *versionStr = (const char *) glGetString(GL_VERSION);

    if (strstr(versionStr, "OpenGL ES 3.") && gl3stubInit()) {
        g_renderer = createES3Renderer(width, height, asset_manager0);
    } else if (strstr(versionStr, "OpenGL ES 2.")) {
        ALOGE("OpenGL ES 2 is not supported");
    } else {
        ALOGE("Unsupported OpenGL ES version");
    }
}

JNIEXPORT void JNICALL
Java_graphics_pathfinder_gles_GLES3JNILib_resize(JNIEnv *env, jobject obj, jint width,
                                                 jint height) {
    if (g_renderer) {
        g_renderer->resize(width, height);
    }
}

JNIEXPORT void JNICALL
Java_graphics_pathfinder_gles_GLES3JNILib_step(JNIEnv *env, jobject obj) {
    if (g_renderer) {
        g_renderer->render();
    }
}
