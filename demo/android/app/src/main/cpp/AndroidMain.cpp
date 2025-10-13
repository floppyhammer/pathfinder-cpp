#include <android/log.h>

#include "native_engine.h"

NativeEngine *native_engine = nullptr;

// Process the next main command.
void handle_cmd(android_app *app, int32_t cmd) {
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            // The window is being shown, get it ready.
            native_engine = new NativeEngine(app);
            native_engine->init_app(false);
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
        if (ALooper_pollOnce(0, nullptr, &events, (void **)&source) >= 0) {
            if (source != nullptr) {
                source->process(app, source);
            }
        }

        // Render if the engine is ready.
        if (native_engine && native_engine->is_ready()) {
            native_engine->draw_frame();
        }
    } while (app->destroyRequested == 0);
}
