#pragma once

#include "native_engine.h"
#include "vulkan_wrapper.h"

class NativeEngineVk : public NativeEngine {
public:
    NativeEngineVk(struct android_app *app);

    bool init_app() override;
};
