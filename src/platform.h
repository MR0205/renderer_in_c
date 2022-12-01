#if !defined( PLATFORM_H )

#include "macro_definitions.h"

struct MemoryArena
{
    uint8 * first_byte;
    size_t consumed;
    size_t size;
};

// TODO: make the disk files arena circular, so when it is full it reutilizes the entirety from the beginning . as we load the files
// transformed state into a diferent lokation we no longer need it.
// either just free the memory after file transformation is done and you no longer need it in memory.
struct GameMemory
{
    uint8 * permanent_storage;
    size_t permanent_storage_size;

    // NOTE: first bytes of permanent_storage are taken by g_GlobalState structure
    MemoryArena dynamic_storage_arena;
};

struct BitmapOutputBuffer
{
    uint32 * bits;
    uint32 width;
    uint32 height;

    int32 shift_to_center_of_bitmap_x_px;
    int32 shift_to_center_of_bitmap_y_px;

    real32 px_in_m;
};

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
    MouseEventType type;
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



struct FileReadResult
{
    uint8 * start_pointer;
    size_t file_size;
};

struct PlatformProcedures
{
    FileReadResult (*ReadFileIntoMemory) (char * file_path);
    bool32 (*FreeFileReadResultFromMemory) (FileReadResult file_read_result);
};

typedef void (* UpdateStateAndRenderPrototype)(GameMemory *, BitmapOutputBuffer *, ControlInput *,
                                               PlatformProcedures*, real64);


uint8 * 
GetMemoryFromArena(uint32 bytes_requested, MemoryArena * memory_arena);

#define PLATFORM_H
#endif
