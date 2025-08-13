// Copyright 2016 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include <android/log.h>

#include "native_engine_gl.h"
#include "native_engine_vk.h"

NativeEngine *native_engine = nullptr;

// Process the next main command.
void handle_cmd(android_app *app, int32_t cmd) {
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            // The window is being shown, get it ready.
            native_engine = new NativeEngineVk(app);
            native_engine->init_app();
            break;
        case APP_CMD_TERM_WINDOW:
            // The window is being hidden or closed, clean it up.
            delete native_engine;
            break;
        default:
            __android_log_print(ANDROID_LOG_INFO, "Pathfinder", "Event not handled: %d", cmd);
    }
}

void android_main(struct android_app *app) {
    // Set the callback to process system events.
    app->onAppCmd = handle_cmd;

    // Used to poll the events in the main loop.
    int events;
    android_poll_source *source;

    // Main loop.
    do {
        if (ALooper_pollAll(1000, nullptr, &events, (void **)&source) >= 0) {
            if (source != NULL) {
                source->process(app, source);
            }
        }

        // Render if the engine is ready.
        if (native_engine && native_engine->is_ready()) {
            native_engine->draw_frame();
        }
    } while (app->destroyRequested == 0);
}
