#include "macro_definitions.h"
#include "math.h"
#include "tilemap.h"
#include "platform.h"

#include "win32.h"

struct Hero
{
    v2 pos;
    real32 speed;
    v2 target_pos;
    v2 dimensions;
};

struct Bitmap
{
    uint32 width;
    uint32 height;
    uint32 * bits;
};

struct GlobalState
{
    bool32 initialized;
    Hero hero;

    v2 camera_center;

    TileMap tile_map;


    real64 game_time_ms;
    real64 time_delta_ms;

    real64 world_scaling_factor;

    real64 cursor_click_time;
    bool32 cursor_click;
    real32 cursor_x;
    real32 cursor_y;

    Bitmap * man_bitmap;

} * g_GlobalState;


internal void
GetWorldCoordinatesFromGameScreenCoordinates(BitmapOutputBuffer * bitmap_output_buffer, 
                                             int32 screen_x, int32 screen_y, 
                                             real32 * world_x, real32 * world_y)
{

    int32 shift_to_center_of_bitmap_x_px = bitmap_output_buffer->shift_to_center_of_bitmap_x_px;
    int32 shift_to_center_of_bitmap_y_px = bitmap_output_buffer->shift_to_center_of_bitmap_y_px;


    int32 screen_x_relative_to_screen_center = screen_x - shift_to_center_of_bitmap_x_px;
    int32 screen_y_relative_to_screen_center = screen_y - shift_to_center_of_bitmap_y_px;
    real32 px_in_m_scaled = bitmap_output_buffer->px_in_m * g_GlobalState->world_scaling_factor;
    //*world_x = ((real32)screen_x_relative_to_screen_center)/px_in_m_scaled;
    //*world_y = ((real32)screen_y_relative_to_screen_center)/px_in_m_scaled;

    
    real32 world_unscaled_relative_to_screen_center_x = ((real32)screen_x_relative_to_screen_center)/px_in_m_scaled;
    real32 world_unscaled_relative_to_screen_center_y = ((real32)screen_y_relative_to_screen_center)/px_in_m_scaled;


    real32 world_unscaled_unrealted_to_screen_center_x = world_unscaled_relative_to_screen_center_x + g_GlobalState->camera_center.x;
    real32 world_unscaled_unrealted_to_screen_center_y = world_unscaled_relative_to_screen_center_y + g_GlobalState->camera_center.y;
    *world_x = world_unscaled_unrealted_to_screen_center_x;
    *world_y = world_unscaled_unrealted_to_screen_center_y;

}

// TODO: comparison of int and uint when int is negative gives 'true'
// for most of the cases (which is wrong) as when int gets upconverted
// to int (it seems, need to verify) it gets a hugt value

// TODO: replace the separate channels with a single packed
// color value, as this obligation forces unpack 
// the users of this function the color when its gets 
// passed to them in packed form, which leads to 
// double work: pack, unpack, and pack in this function again
// when we could have just straight up used the packed value
// and the packing is done only once at the beggining of the chain
internal void
DrawRectangle(BitmapOutputBuffer * bitmap_output_buffer,
              real32 min_x_real, real32 min_y_real, real32 max_x_real, real32 max_y_real,
              uint32 color) //uint8 r, uint8 g, uint8 b)
{
    min_x_real -= g_GlobalState->camera_center.x;
    min_y_real -= g_GlobalState->camera_center.y;
    max_x_real -= g_GlobalState->camera_center.x;
    max_y_real -= g_GlobalState->camera_center.y;
    real32 px_in_m_scaled = bitmap_output_buffer->px_in_m * g_GlobalState->world_scaling_factor;
    int32 shift_to_center_of_bitmap_x_px = bitmap_output_buffer->shift_to_center_of_bitmap_x_px;
    int32 shift_to_center_of_bitmap_y_px = bitmap_output_buffer->shift_to_center_of_bitmap_y_px;
    int32 min_x = RoundReal32ToInt32(min_x_real * px_in_m_scaled) + shift_to_center_of_bitmap_x_px;
    int32 min_y = RoundReal32ToInt32(min_y_real * px_in_m_scaled) + shift_to_center_of_bitmap_y_px;
    int32 max_x = RoundReal32ToInt32(max_x_real * px_in_m_scaled) + shift_to_center_of_bitmap_x_px;
    int32 max_y = RoundReal32ToInt32(max_y_real * px_in_m_scaled) + shift_to_center_of_bitmap_y_px;

    if (min_x < 0) 
    {
        min_x = 0;
    }

    if (min_y < 0)
    {
        min_y = 0;
    }

    if (max_x < 0)
    {
        max_x = 0;
    }

    if (max_y < 0)
    {
        max_y = 0;
    }

    if (max_x > bitmap_output_buffer->width)
    {
        max_x = bitmap_output_buffer->width;
    }

    if (max_y > bitmap_output_buffer->height)
    {
        max_y = bitmap_output_buffer->height;
    }

    //uint32 color = 0;
    uint32 * cur_row_start = bitmap_output_buffer->bits + 
                             min_y * bitmap_output_buffer->width;
    uint32 * cur_pixel; 
    for (int32 index_row = min_y; index_row < max_y; ++index_row)
    {
        cur_pixel = cur_row_start + min_x;
        for (int32 index_column = min_x; index_column < max_x; ++index_column)
        {
            // TODO: dont understand why green gets casted into uint8,
            // when it is already uint8, and why does the shift produces
            // correct value when it theory it must zero out the uin8
            // when shifting 8 bits.
            //color = ((((uint32)r) << 16) | (((uint8)g) << 8) | b);
            //color = ((((uint32)r) << 16) | (((uint32)g) << 8) | (uint32)b);
            *(cur_pixel++) = color;
        }
        cur_row_start += bitmap_output_buffer->width;
    }
}

internal void
RenderHero(BitmapOutputBuffer * bitmap_output_buffer)
{
    v2 hero_pos_min = g_GlobalState->hero.pos - 0.5 * g_GlobalState->hero.dimensions; //  - g_GlobalState->camera_center

    v2 hero_pos_max = hero_pos_min + g_GlobalState->hero.dimensions;

    uint32 hero_color = 0x000000FF;
    real32 hero_border_width = 0.05f;
    uint32 hero_border_color = 0x00FFFFFF;
    DrawRectangle(bitmap_output_buffer, hero_pos_min.x, hero_pos_min.y,
                  hero_pos_max.x, hero_pos_max.y, hero_border_color);
    DrawRectangle(bitmap_output_buffer, hero_pos_min.x + hero_border_width,
                  hero_pos_min.y + hero_border_width, hero_pos_max.x - hero_border_width,
                  hero_pos_max.y - hero_border_width, hero_color);
}

internal void
RenderTileMap(BitmapOutputBuffer * bitmap_output_buffer)
{
    TileMap * tile_map =  &g_GlobalState->tile_map;
    uint32 color = 0;


    for (int tile_row = 0; tile_row < tile_map->num_tiles_y; ++tile_row)
    {
        for (int tile_column = 0; tile_column < tile_map->num_tiles_x; ++tile_column)
        {
            real32 tile_length = g_GlobalState->tile_map.tile_length; 
            v2 tile_min = {(real32) tile_column, (real32) tile_row};
            tile_min = tile_length * tile_min;//  - g_GlobalState->camera_center;
            v2 tile_max = tile_min + v2{tile_length, tile_length}; 

            color = tile_map->tiles[tile_row][tile_column];
            DrawRectangle(bitmap_output_buffer, tile_min.x,  tile_min.y, 
                          tile_max.x, tile_max.y, color);
        }
    }
}

internal void
RenderCursorClick(BitmapOutputBuffer * bitmap_output_buffer)
{
    if (g_GlobalState->cursor_click && 
        (g_GlobalState->game_time_ms - g_GlobalState->cursor_click_time < 100.0f))
    {
        real32 cursor_x = g_GlobalState->cursor_x;
        real32 cursor_y = g_GlobalState->cursor_y;

        real32 splah_square_edge_length = 1.1f;

        DrawRectangle(bitmap_output_buffer, 
                      cursor_x - splah_square_edge_length/2,
                      cursor_y - splah_square_edge_length/2,
                      cursor_x + splah_square_edge_length/2,
                      cursor_y + splah_square_edge_length/2,
                      0x00888888);
    }
}
// didn't want to pass here bitmap_output_buffer as we don't need to change it
// but we keep using its px_to_m to convert coordinates
// may be think of how we can restructure the code and move out probably this information
// to somthing globally available, to keep arguments lists short
internal void
ProcessControlInput(ControlInput * control_input,
                    BitmapOutputBuffer * bitmap_output_buffer)
{

    real32 delta = 0.1f;

    for (int key_index = 0; key_index < KEYBOARD_KEY_SIZE; ++key_index)
    {
        if (control_input->keys_pressed[key_index])
        {
            switch (key_index)
            {
                case LEFT_KEY:
                {
                    g_GlobalState->camera_center.x -= delta;
                } break;

                case UP_KEY:
                {
                    g_GlobalState->camera_center.y -= delta;
                } break;

                case RIGHT_KEY:
                {
                    g_GlobalState->camera_center.x += delta;
                } break;

                case DOWN_KEY:
                {
                    g_GlobalState->camera_center.y += delta;
                } break;
            }
        }
    }

    ////////////////////////

    uint32 mouse_screen_scroll_region_length_px = 10;
    if (control_input->cursor_x < mouse_screen_scroll_region_length_px)
    {
        g_GlobalState->camera_center.x -= delta;
    } else if (control_input->cursor_x >= bitmap_output_buffer->width - mouse_screen_scroll_region_length_px)
    {
        g_GlobalState->camera_center.x += delta;
    } else if (control_input->cursor_y < mouse_screen_scroll_region_length_px)
    {
        g_GlobalState->camera_center.y -= delta;
    } else if (control_input->cursor_y >= bitmap_output_buffer->height - mouse_screen_scroll_region_length_px)
    {
        g_GlobalState->camera_center.y += delta;
    } 

    ////////////////////////

    real32 world_scaling_factor_delta = (real32)control_input->mouse_wheel_rotation / 100.0f; 
    g_GlobalState->world_scaling_factor += world_scaling_factor_delta; 

    if (control_input->last_mouse_event.mouse_event_type == RIGHT_BUTTON_PRESSED)
    {
        int32 screen_x = control_input->last_mouse_event.x;
        int32 screen_y = control_input->last_mouse_event.y;
        real32 world_x;
        real32 world_y;
        GetWorldCoordinatesFromGameScreenCoordinates(bitmap_output_buffer,
                                                     screen_x, screen_y,
                                                     &world_x, &world_y);
        g_GlobalState->cursor_click = true;
        g_GlobalState->cursor_x = world_x;
        g_GlobalState->cursor_y = world_y;
        g_GlobalState->cursor_click_time = g_GlobalState->game_time_ms;

        g_GlobalState->hero.target_pos = v2{world_x, world_y};
    }

//            case VK_PRIOR: // PageUp
//            {
//                //bitmap_output_buffer->px_in_m -= 0.1f;
//                OutputDebugStringA("PageUp: down\n");
//            } break;
//
//            case VK_NEXT: // PageDown
//            {
//                //bitmap_output_buffer->px_in_m += 0.1f;
//                OutputDebugStringA("PageDown: down\n");
//            } break;
    return;
}

internal void
SimulateTimeStep()
{
    
    if ( g_GlobalState->hero.pos != g_GlobalState->hero.target_pos)
    {
        v2 direction = g_GlobalState->hero.target_pos - g_GlobalState->hero.pos;
        v2 direction_noralised = direction;

        normalize(&direction_noralised);

        v2 full_speed_distance_step = direction_noralised * (g_GlobalState->time_delta_ms * g_GlobalState->hero.speed);

        v2 new_pos;
        real32 norm_full_speed_distance_step = norm(full_speed_distance_step);
        real32 norm_direction = norm(direction);
        if (norm_full_speed_distance_step <= norm_direction)
        {
            new_pos = g_GlobalState->hero.pos + full_speed_distance_step;

        } else
        {
            new_pos = g_GlobalState->hero.target_pos;
        }

        g_GlobalState->hero.pos = new_pos;
    }
}


#pragma pack(1)
struct BmpFileHeader
{
    char file_type[2];
    uint32 file_size;
    uint16 empty_space0;
    uint16 empty_space1;
    uint32 offset_to_bits;
};

#pragma pack(1)
struct BmpBitmapHeader
{
    uint32 header_size;
    int32 bitmap_width;
    int32 bitmap_height; // cant be negative
    uint16 num_planes; // always 1
    uint16 bits_per_color;
    uint32 compression;
    uint32 size_of_buffer_for_compressed; // used for compressed formats
    int32 horizontal_resolution_px_per_m;
    int32 vertical_resolution_px_per_m;
    uint32 num_colors_in_color_table;// expecting 0
    uint32 num_colors_required_for_displaying; // expecting 0
};


internal Bitmap *
ReadLoadedBmp(uint8 * bmp_file, MemoryArena * memory_arena)
{
    BmpFileHeader bmp_file_header = *((BmpFileHeader *) bmp_file);

    uint32 bitmap_header_size = *((uint32 *) (bmp_file + sizeof(BmpFileHeader)));
    ASSERT(bitmap_header_size == 40); // we currently don't implement support for v4 and v5 headers (and format?)
    BmpBitmapHeader bmp_bitmap_header = *((BmpBitmapHeader *) (bmp_file + sizeof(BmpFileHeader)));
    ASSERT(bmp_bitmap_header.bitmap_height > 0); // current implementation assumes bottom-up bitmap file


    
    Bitmap * return_bitmap = (Bitmap *) GetMemoryFromArena(sizeof(Bitmap), memory_arena);
    return_bitmap->width = bmp_bitmap_header.bitmap_width;
    return_bitmap->height = bmp_bitmap_header.bitmap_height;
    return_bitmap->bits = (uint32 *) GetMemoryFromArena(bmp_bitmap_header.bits_per_color * bmp_bitmap_header.bitmap_width * bmp_bitmap_header.bitmap_height, memory_arena);

    uint32 * src = (uint32 *) (bmp_file + bmp_file_header.offset_to_bits);
    uint32 * dest = (uint32 *) (return_bitmap->bits + bmp_bitmap_header.bitmap_width * (bmp_bitmap_header.bitmap_height - 1));
    uint32 * src_px;
    uint32 * dest_px;

    for (uint32 row_index = 0; row_index < bmp_bitmap_header.bitmap_height; ++row_index)
    {
        uint32 * src_row = src + row_index * bmp_bitmap_header.bitmap_width;
        uint32 * dest_row = dest - row_index * bmp_bitmap_header.bitmap_width;
        for (uint32 column_index = 0; column_index < bmp_bitmap_header.bitmap_width; ++column_index)
        {

            src_px = src_row + column_index;
            dest_px = dest_row + column_index;
            *dest_px = *src_px;
        }
    }

    return return_bitmap;
}

internal void
RenderBitmap(Bitmap * bitmap, BitmapOutputBuffer * bitmap_output_buffer)
{
    int32 min_x = 0;
    int32 max_x = bitmap->width;
    int32 min_y = 0;
    int32 max_y = bitmap->height;

    if (max_x > bitmap_output_buffer->width)
    {
        max_x = bitmap_output_buffer->width;
    }

    if (max_y > bitmap_output_buffer->height)
    {
        max_y = bitmap_output_buffer->height;
    }
    
    uint32 * dest_start = bitmap_output_buffer->bits + min_y * bitmap_output_buffer->width + min_x;
    uint32 * src_start = bitmap->bits; 
    uint32 * src_row;
    uint32 * dest_row;
    uint32 * src_px;
    uint32 *dest_px;
    // TODO: implement the case when bitmap is timmed because it goes beyond the canvas;
    for (int32 index_row = min_y; index_row < max_y; ++index_row)
    {
        src_row = src_start + bitmap->width * index_row;
        dest_row = dest_start + bitmap_output_buffer->width * index_row;

        for (int32 index_column = min_x; index_column < max_x; ++index_column)
        {
            src_px = src_row + index_column;
            dest_px = dest_row + index_column;
            *dest_px = *src_px;
            //*dest_px = 0xFFFFFFFF;
        }
    }
}

extern "C"
{
__declspec(dllexport) void
UpdateStateAndRender(GameMemory * game_memory,
                     BitmapOutputBuffer * bitmap_output_buffer, 
                     ControlInput * control_input,
                     PlatformProcedures * platform_procedures,
                     real64 time_delta_ms)
{

    ASSERT(game_memory->permanent_storage_size >= sizeof(GlobalState));
    g_GlobalState = (GlobalState *) game_memory->permanent_storage;
    if (!g_GlobalState->initialized)
    {
        game_memory->disk_files_memory_arena.first_byte = game_memory->permanent_storage + sizeof(GlobalState);
        game_memory->disk_files_memory_arena.size = game_memory->permanent_storage_size - sizeof(GlobalState);

        uint8 * man_bmp_file = platform_procedures->ReadFileIntoMemory("D:\\c_proj\\assets\\develop\\man\\man.bmp", &game_memory->disk_files_memory_arena);
        g_GlobalState->man_bitmap = ReadLoadedBmp(man_bmp_file, &game_memory->disk_files_memory_arena);

        g_GlobalState->hero.pos = {1.0f, 1.0f};
        g_GlobalState->hero.target_pos = g_GlobalState->hero.pos;
        g_GlobalState->hero.dimensions = {0.8f, 1.4f};
        g_GlobalState->hero.speed = 0.0015f;


        g_GlobalState->camera_center = {5.0f, 2.5f};

        g_GlobalState->game_time_ms = 0.0f;
        g_GlobalState->time_delta_ms = 0.0f;
        g_GlobalState->world_scaling_factor = 1.0f;
        TileMap tile_map;

        tile_map.num_tiles_x = 30;
        tile_map.num_tiles_y = 30;
        tile_map.tile_length = 1.0f;

        uint32 color = 0;
        for (int tile_row = 0; tile_row < tile_map.num_tiles_y; ++tile_row)
        {
            for (int tile_column = 0; tile_column < tile_map.num_tiles_x; ++tile_column)
            {
                
                uint8 r = (tile_row * (256 / tile_map.num_tiles_y)) % 256;
                uint8 g = (tile_column * (256 / tile_map.num_tiles_x)) % 256;
                //uint8 b = 0;
                uint8 b = 255;

                color = ((((uint32)r) << 16) | (((uint32)g)<< 8) | b);
                tile_map.tiles[tile_row][tile_column] = color;
            }
        }

        g_GlobalState->tile_map = tile_map;

        g_GlobalState->initialized = true;
    }

    g_GlobalState->game_time_ms += time_delta_ms;
    g_GlobalState->time_delta_ms = time_delta_ms;


    ProcessControlInput(control_input, bitmap_output_buffer);

    SimulateTimeStep();

    RenderTileMap(bitmap_output_buffer);
    RenderHero(bitmap_output_buffer);
    RenderCursorClick(bitmap_output_buffer);
    RenderBitmap(g_GlobalState->man_bitmap, bitmap_output_buffer);
}

} // extern "C"
