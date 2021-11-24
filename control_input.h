#if !defined(CONTROL_INPUT_H)
#define CONTROL_INPUT_H

enum KeyboardKey 
{
    LEFT_KEY,//=VK_LEFT,
    UP_KEY,///=VK_UP,
    RIGHT_KEY,//=VK_RIGHT,
    DOWN_KEY,//=VK_DOWN,
    KEYBOARD_KEY_SIZE,
};

enum MouseEventType
{
    NO_MOUSE_BUTTON_EVENT,
    LEFT_BUTTON_PRESSED,
    LEFT_BUTTON_RELEASED,
    RIGHT_BUTTON_PRESSED,
    RIGHT_BUTTON_RELEASED,
    MOUSE_EVENT_TYPE_SIZE,
};

struct MouseEvent 
{
    MouseEventType mouse_event_type;
    int32 x;
    int32 y;
};

struct ControlInput
{
    bool32 keys_pressed[KEYBOARD_KEY_SIZE];

    // NOTE: At the moment we keep track only of the last happened mouse event
    // during the polling of the queue, technically we overwrite the events
    // and the last in the message queue is passed to the UpdateStateAndRender
    MouseEvent last_mouse_event;

    int32 cursor_x;
    int32 cursor_y;

    int32 mouse_wheel_rotation;
};

#endif
