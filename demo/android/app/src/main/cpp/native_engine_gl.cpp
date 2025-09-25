#include "native_engine_gl.h"

#include <pathfinder/gpu/gl/window_builder.h>

NativeEngineGl::NativeEngineGl(struct android_app *app) : NativeEngine(app) {
    mEglDisplay = EGL_NO_DISPLAY;
    mEglSurface = EGL_NO_SURFACE;
    mEglContext = EGL_NO_CONTEXT;
    mEglConfig = nullptr;
}

bool NativeEngineGl::init_display() {
    if (mEglDisplay != EGL_NO_DISPLAY) {
        return true;
    }

    mEglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (EGL_FALSE == eglInitialize(mEglDisplay, nullptr, nullptr)) {
        LOGE("NativeEngine: failed to init display, error %d", eglGetError());
        return false;
    }
    return true;
}

bool NativeEngineGl::init_surface() {
    assert(mEglDisplay != EGL_NO_DISPLAY);
    if (mEglSurface != EGL_NO_SURFACE) {
        return true;
    }

    EGLint numConfigs;
    const EGLint attribs[] = {EGL_RENDERABLE_TYPE,
                              EGL_OPENGL_ES3_BIT, // Request OpenGL ES 3.0
                              EGL_SURFACE_TYPE,
                              EGL_WINDOW_BIT,
                              EGL_BLUE_SIZE,
                              8,
                              EGL_GREEN_SIZE,
                              8,
                              EGL_RED_SIZE,
                              8,
                              EGL_DEPTH_SIZE,
                              16,
                              EGL_NONE};

    // Pick the first EGLConfig that matches.
    eglChooseConfig(mEglDisplay, attribs, &mEglConfig, 1, &numConfigs);
    mEglSurface = eglCreateWindowSurface(mEglDisplay, mEglConfig, mAppCtx->window, nullptr);
    if (mEglSurface == EGL_NO_SURFACE) {
        LOGE("Failed to create EGL surface, EGL error %d", eglGetError());
        return false;
    }
    return true;
}

bool NativeEngineGl::init_context() {
    assert(mEglDisplay != EGL_NO_DISPLAY);
    if (mEglContext != EGL_NO_CONTEXT) {
        return true;
    }

    EGLint attribList[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
    mEglContext = eglCreateContext(mEglDisplay, mEglConfig, nullptr, attribList);
    if (mEglContext == EGL_NO_CONTEXT) {
        LOGE("Failed to create EGL context, EGL error %d", eglGetError());
        return false;
    }
    return true;
}

bool NativeEngineGl::init_app() {
    init_display();
    init_surface();
    init_context();

    // This is required.
    set_context();

    auto window_size =
        Pathfinder::Vec2I(ANativeWindow_getWidth(mAppCtx->window), ANativeWindow_getHeight(mAppCtx->window));

    window_builder = std::make_shared<Pathfinder::WindowBuilderGl>(window_size);

    init_app_common(window_size);

    return true;
}

void NativeEngineGl::set_context() {
    eglMakeCurrent(mEglDisplay, mEglSurface, mEglSurface, mEglContext);
}

void NativeEngineGl::draw_frame() {
    NativeEngine::draw_frame();

    eglSwapBuffers(mEglDisplay, mEglSurface);
}
