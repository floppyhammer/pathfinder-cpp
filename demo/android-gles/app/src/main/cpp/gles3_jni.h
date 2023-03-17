#ifndef GLES3_JNI_H
#define GLES3_JNI_H 1

#include <android/log.h>
#include <android/asset_manager.h>
#include <cmath>

#if DYNAMIC_ES3
#include "gl3stub.h"
#else
// Include the latest possible header file( GL version header )
#if __ANDROID_API__ >= 24
#include <GLES3/gl32.h>

#elif __ANDROID_API__ >= 21
#include <GLES3/gl31.h>
#else

#include <GLES3/gl3.h>

#endif

#endif

#define LOG_TAG "GLES3JNI"
#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define ALOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)

// ----------------------------------------------------------------------------
// Interface to the ES3 renderers, used by JNI code.

class RendererES {
public:
    virtual ~RendererES();

    void resize(int width, int height);

    virtual void render();

protected:
    RendererES(AAssetManager *_asset_manager);

    AAssetManager *asset_manager;
};

extern RendererES *createES3Renderer(int width, int height, AAssetManager *_asset_manager);

#endif // GLES3JNI_H
