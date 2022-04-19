#ifndef PATHFINDER_INPUT_SERVER_H
#define PATHFINDER_INPUT_SERVER_H

#include "../../../common/math/vec2.h"

#include <cstdint>
#include <vector>

namespace Pathfinder {
    enum class InputEventType {
        MouseButton = 0,
        MouseMotion,
        MouseScroll,
        Key,
        Max,
    };

    enum class KeyCode {

    };

    class InputEvent {
    public:
        InputEventType type = InputEventType::Max;

        union Args {
            struct {
                KeyCode key;
                bool pressed;
            } key{};
            struct {
                uint8_t button;
                bool pressed;
                Vec2<float> position;
            } mouse_button;
            struct {
                float delta;
            } mouse_scroll;
            struct {
                Vec2<float> relative;
                Vec2<float> position;
            } mouse_motion;
        } args;
    };

    class InputServer {
    public:
        static InputServer &get_singleton() {
            static InputServer singleton;
            return singleton;
        }

        Vec2<float> cursor_position;

        std::vector<Pathfinder::InputEvent> input_queue;

        void clear_queue();
    };
}

#endif //PATHFINDER_INPUT_SERVER_H
