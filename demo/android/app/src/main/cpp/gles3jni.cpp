/*
 * Copyright 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <jni.h>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <android/asset_manager_jni.h>

#include "gles3jni.h"

static void printGlString(const char* name, GLenum s) {
    const char* v = (const char*)glGetString(s);
    ALOGV("GL %s: %s\n", name, v);
}

// ----------------------------------------------------------------------------

Renderer::Renderer(AAssetManager* p_asset_manager) {
    asset_manager = p_asset_manager;
}

Renderer::~Renderer() = default;

void Renderer::resize(int w, int h) {
}

void Renderer::step() {
}

void Renderer::render() {
    draw();
}

// ----------------------------------------------------------------------------

static Renderer* g_renderer = nullptr;

extern "C" {
    JNIEXPORT void JNICALL Java_com_android_gles3jni_GLES3JNILib_init(JNIEnv* env, jobject obj, jint width, jint height, jobject asset_manager);
    JNIEXPORT void JNICALL Java_com_android_gles3jni_GLES3JNILib_resize(JNIEnv* env, jobject obj, jint width, jint height);
    JNIEXPORT void JNICALL Java_com_android_gles3jni_GLES3JNILib_step(JNIEnv* env, jobject obj);
};

#if !defined(DYNAMIC_ES3)
static GLboolean gl3stubInit() {
    return GL_TRUE;
}
#endif

JNIEXPORT void JNICALL
Java_com_android_gles3jni_GLES3JNILib_init(JNIEnv* env, jobject obj, jint width, jint height, jobject asset_manager) {
    if (g_renderer) {
        delete g_renderer;
        g_renderer = nullptr;
    }

    printGlString("Version", GL_VERSION);
    printGlString("Vendor", GL_VENDOR);
    printGlString("Renderer", GL_RENDERER);
    printGlString("Extensions", GL_EXTENSIONS);

    AAssetManager* asset_manager0 = AAssetManager_fromJava(env, asset_manager);

    const char* versionStr = (const char*)glGetString(GL_VERSION);
    if (strstr(versionStr, "OpenGL ES 3.") && gl3stubInit()) {
        g_renderer = createES3Renderer(width, height, asset_manager0);
    } else if (strstr(versionStr, "OpenGL ES 2.")) {
        g_renderer = createES2Renderer(width, height, asset_manager0);
    } else {
        ALOGE("Unsupported OpenGL ES version");
    }
}

JNIEXPORT void JNICALL
Java_com_android_gles3jni_GLES3JNILib_resize(JNIEnv* env, jobject obj, jint width, jint height) {
    if (g_renderer) {
        g_renderer->resize(width, height);
    }
}

JNIEXPORT void JNICALL
Java_com_android_gles3jni_GLES3JNILib_step(JNIEnv* env, jobject obj) {
    if (g_renderer) {
        g_renderer->render();
    }
}
