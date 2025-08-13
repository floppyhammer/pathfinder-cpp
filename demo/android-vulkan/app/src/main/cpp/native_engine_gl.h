#pragma once

#include <EGL/egl.h>

#include "native_engine.h"

class NativeEngineGl : public NativeEngine {
public:
    NativeEngineGl(struct android_app *app);

    bool init_app() override;

private:
    bool init_display();

    bool init_surface();

    bool init_context();

    EGLDisplay mEglDisplay;
    EGLSurface mEglSurface;
    EGLContext mEglContext;
    EGLConfig mEglConfig;
};
