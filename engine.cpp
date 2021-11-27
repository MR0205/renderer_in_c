#include "macro_definitions.h"
#include "math.h"
#include "tilemap.h"
#include "platform.h"

#include "win32.h"

struct Hero
{
    //v2 pos;
    WorldCoord pos;
    real32 speed;
    //v2 target_pos;
    WorldCoord target_pos;
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

    WorldCoord cursor_pos;
    //real32 cursor_x;
    //real32 cursor_y;

    Bitmap * man_bitmap;

} * g_GlobalState;


internal void
GetWorldCoordinatesFromGameScreenCoordinates(BitmapOutputBuffer * bitmap_output_buffer, 
                                             int32 screen_x, int32 screen_y, 
                                             WorldCoord * pos)
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
DrawBitmap(Bitmap * bitmap, BitmapOutputBuffer * bitmap_output_buffer,
              real32 center_x_real, real32 center_y_real)
{
    center_x_real -= g_GlobalState->camera_center.x;
    center_y_real -= g_GlobalState->camera_center.y;
    real32 px_in_m_scaled = bitmap_output_buffer->px_in_m * g_GlobalState->world_scaling_factor;
    int32 shift_to_center_of_bitmap_x_px = bitmap_output_buffer->shift_to_center_of_bitmap_x_px;
    int32 shift_to_center_of_bitmap_y_px = bitmap_output_buffer->shift_to_center_of_bitmap_y_px;
    int32 center_x = RoundReal32ToInt32(center_x_real * px_in_m_scaled) + shift_to_center_of_bitmap_x_px;
    int32 center_y = RoundReal32ToInt32(center_y_real * px_in_m_scaled) + shift_to_center_of_bitmap_y_px;

    // substracting the remainder so that in the case of odd width the center coordinate center_x corresponds exactly to the center of the bitmap and not one pixel left
    int32 min_x = center_x - bitmap->width/2 - bitmap->width%2; 
    int32 max_x = min_x + bitmap->width;

    int32 min_y = center_y - bitmap->height/2 - bitmap->height%2; 
    int32 max_y = min_y + bitmap->height;

    int32 num_pixels_cut_left = 0;
    int32 num_pixels_cut_top = 0;

    if (min_x < 0) 
    {
        num_pixels_cut_left = -min_x;
        min_x = 0;
    }

    if (min_y < 0)
    {
        num_pixels_cut_top = -min_y;
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

    uint32 * dst_start = bitmap_output_buffer->bits;
    uint32 * src_start = bitmap->bits; 
    uint32 * src_row;
    uint32 * dst_row;
    uint32 * src_px_pointer;
    uint32 * dst_px_pointer;
    uint32 alpha_blend_color;
    // TODO: implement the case when bitmap is timmed because it goes beyond the canvas;
    for (int32 index_row = 0; index_row < max_y - min_y; ++index_row)
    {
        src_row = src_start + bitmap->width * (index_row + num_pixels_cut_top);
        dst_row = dst_start + bitmap_output_buffer->width * (min_y + index_row);

        for (int32 index_column = 0; index_column < max_x - min_x; ++index_column)
        {
            src_px_pointer = src_row + index_column + num_pixels_cut_left;
            dst_px_pointer = dst_row + min_x + index_column;

            uint32 src_px = *src_px_pointer;
            uint32 dst_px = *dst_px_pointer;

            real32 alpha = (real32)(src_px >> 24);

            if (index_row == 0 && index_column == 0) {
                int b = 1;
            }

            if (alpha > 0.0f && src_px > 0xFF000000)
            {
                int b = 255;
            }
            real32 alpha_remainder = 255.0f - alpha;

            real32 r_src_alpha_accounted = ((real32)(src_px >> 16 & 0xFF) * alpha/255.0f);
            real32 r_dst_alpha_accounted = ((real32)(dst_px >> 16 & 0xFF) * alpha_remainder/255.0f);
            uint32  r_alpha_blend = RoundReal32ToInt32(r_src_alpha_accounted) + RoundReal32ToInt32(r_dst_alpha_accounted);

            real32 g_src_alpha_accounted = ((real32)(src_px >> 8 & 0xFF) * alpha/255.0f);
            real32 g_dst_alpha_accounted = ((real32)(dst_px >> 8 & 0xFF) * alpha_remainder/255.0f);
            uint32  g_alpha_blend = RoundReal32ToInt32(g_src_alpha_accounted) + RoundReal32ToInt32(g_dst_alpha_accounted);

            real32 b_src_alpha_accounted = ((real32)(src_px >> 0 & 0xFF) * alpha/255.0f);
            real32 b_dst_alpha_accounted = ((real32)(dst_px >> 0 & 0xFF) * alpha_remainder/255.0f);
            uint32  b_alpha_blend = RoundReal32ToInt32(b_src_alpha_accounted) + RoundReal32ToInt32(b_dst_alpha_accounted);
            alpha_blend_color = (r_alpha_blend << 16) | (g_alpha_blend << 8) | (b_alpha_blend);

            *dst_px_pointer = alpha_blend_color;
        }
    }
}

internal void
RenderHero(BitmapOutputBuffer * bitmap_output_buffer)
{
    //v2 hero_pos_min = g_GlobalState->hero.pos - 0.5 * g_GlobalState->hero.dimensions; //  - g_GlobalState->camera_center

    //v2 hero_pos_max = hero_pos_min + g_GlobalState->hero.dimensions;

    uint32 hero_color = 0x000000FF;
    real32 hero_border_width = 0.05f;
    uint32 hero_border_color = 0x00FFFFFF;
    DrawBitmap(g_GlobalState->man_bitmap, bitmap_output_buffer, g_GlobalState->hero.pos);
}

internal void
RenderTileMap(BitmapOutputBuffer * bitmap_output_buffer)
{
    TileMap * tile_map =  &g_GlobalState->tile_map;
    uint32 color = 0;


    for (int tile_row = 0; tile_row < tile_map->num_tiles.y; ++tile_row)
    {
        for (int tile_column = 0; tile_column < tile_map->num_tiles.x; ++tile_column)
        {
            real32 tile_length = tile_map->tile_length; 
            v2 tile_min = {(real32) tile_column, (real32) tile_row};
            tile_min = tile_length * tile_min;//  - g_GlobalState->camera_center;
            v2 tile_max = tile_min + v2{tile_length, tile_length}; 

            Tile * current_tile_pointer = tile_map->tiles + tile_row * tile_map->num_tiles.x + tile_column;
            color = current_tile_pointer->color;
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
        //real32 cursor_x = g_GlobalState->cursor_x;
        //real32 cursor_y = g_GlobalState->cursor_y;

        WorldCoord cursor_pos = g_GlobalState->cursor_pos;
        // TODO: render the curor click

        real32 cursor_highlight_square_edge_length = 0.1f;

        // DrawRectangle(bitmap_output_buffer, 
        //               cursor_x - cursor_highlight_square_edge_length/2,
        //               cursor_y - cursor_highlight_square_edge_length/2,
        //               cursor_x + cursor_highlight_square_edge_length/2,
        //               cursor_y + cursor_highlight_square_edge_length/2,
        //               0x00888888);
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
        WorldCoord target_pos;
        // TODO: reimplement the function
        // GetWorldCoordinatesFromGameScreenCoordinates(bitmap_output_buffer,
        //                                              screen_x, screen_y,
        //                                              &target_pos);
        // g_GlobalState->cursor_click = true;
        // g_GlobalState->cursor_pos = target_pos;

        // g_GlobalState->cursor_click_time = g_GlobalState->game_time_ms;

        // g_GlobalState->hero.target_pos = target_pos;
    }

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
struct BmpBitmapV1Header
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


#pragma pack(1)
struct BmpBitmapV5Header
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
    uint32 red_mask;
    uint32 green_mask;
    uint32 blue_mask;
    uint32 alpha_mask;
    uint8 other_staff[68];
};






internal Bitmap *
ReadLoadedBmp(uint8 * bmp_file, MemoryArena * memory_arena)
{
    BmpFileHeader bmp_file_header = *((BmpFileHeader *) bmp_file);

    uint32 bitmap_height;
    uint32 bitmap_width;
    uint32 bits_per_color;

    uint32 bitmap_header_size = *((uint32 *) (bmp_file + sizeof(BmpFileHeader)));
    if (bitmap_header_size == 40)
    {
        BmpBitmapV1Header bmp_bitmap_v1_header = *((BmpBitmapV1Header *) (bmp_file + sizeof(BmpFileHeader)));
        bitmap_height = bmp_bitmap_v1_header.bitmap_height;
        bitmap_width = bmp_bitmap_v1_header.bitmap_width;
        bits_per_color = bmp_bitmap_v1_header.bits_per_color;
    } else if (bitmap_header_size == 124)
    {
        BmpBitmapV5Header bmp_bitmap_v5_header = *((BmpBitmapV5Header *) (bmp_file + sizeof(BmpFileHeader)));
        //debug
        uint32 size_of_header_as_defined = sizeof(BmpBitmapV5Header);
        bitmap_height = bmp_bitmap_v5_header.bitmap_height;
        bitmap_width = bmp_bitmap_v5_header.bitmap_width;
        bits_per_color = bmp_bitmap_v5_header.bits_per_color;

        // verifiyng our assumptions about the format
        // if any of these fails we need to write different interpretation code of the bytes
        // as it will no longer be the same as raw format.
        ASSERT(bmp_bitmap_v5_header.compression == 3); // specifies the presence of the masks in the corresponding files of the structure;
        // the default location of the channels, which is the same as in v1 version of bitmap format;
        ASSERT(bmp_bitmap_v5_header.alpha_mask == 0xFF000000); 
        ASSERT(bmp_bitmap_v5_header.red_mask   == 0x00FF0000); 
        ASSERT(bmp_bitmap_v5_header.green_mask == 0x0000FF00); 
        ASSERT(bmp_bitmap_v5_header.blue_mask  == 0x000000FF); 
    } else 
    {
        ASSERT(0);// unsupprted bmp format
    }

    ASSERT(bitmap_height > 0); // current implementation assumes bottom-up bitmap file


    
    Bitmap * return_bitmap = (Bitmap *) GetMemoryFromArena(sizeof(Bitmap), memory_arena);
    return_bitmap->width = bitmap_width;
    return_bitmap->height = bitmap_height;
    return_bitmap->bits = (uint32 *) GetMemoryFromArena(bits_per_color * bitmap_width * bitmap_height, memory_arena);

    uint32 * src = (uint32 *) (bmp_file + bmp_file_header.offset_to_bits);
    uint32 * dest = (uint32 *) (return_bitmap->bits + bitmap_width * (bitmap_height - 1));
    uint32 * src_px;
    uint32 * dest_px;

    for (uint32 row_index = 0; row_index < bitmap_height; ++row_index)
    {
        uint32 * src_row = src + row_index * bitmap_width;
        uint32 * dest_row = dest - row_index * bitmap_width;
        for (uint32 column_index = 0; column_index < bitmap_width; ++column_index)
        {

            // debug
            if (row_index == 50 && column_index == 24) {
                int b = 1;
            }
            src_px = src_row + column_index;
            dest_px = dest_row + column_index;
            *dest_px = *src_px;
        }
    }

    return return_bitmap;
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

    uint32 memory_distributed = 0;
    ASSERT(game_memory->permanent_storage_size >= sizeof(GlobalState));
    g_GlobalState = (GlobalState *) game_memory->permanent_storage + memory_distributed;
    memory_distributed += sizeof(GlobalState);

    if (!g_GlobalState->initialized)
    {

        uint32 file_cache_arena_size = 256 * MEGABYTE; 
        ASSERT(game_memory->permanent_storage_size >= memory_distributed + file_cache_arena_size);
        //game_memory->file_cache_arena = {};
        game_memory->file_cache_arena.first_byte = game_memory->permanent_storage + memory_distributed;
        game_memory->file_cache_arena.size = file_cache_arena_size;
        memory_distributed += file_cache_arena_size;

        uint32 rest_of_the_permanent_storage_size = game_memory->permanent_storage_size - memory_distributed;
        uint32 dynamic_storage_arena_size = rest_of_the_permanent_storage_size;
        ASSERT(game_memory->permanent_storage_size >= memory_distributed + dynamic_storage_arena_size);
        //game_memory->dynamic_storage_arena = {};
        game_memory->dynamic_storage_arena.first_byte = game_memory->permanent_storage + memory_distributed;
        game_memory->dynamic_storage_arena.size = dynamic_storage_arena_size;
        memory_distributed += dynamic_storage_arena_size;

        uint8 * man_bmp_file = platform_procedures->ReadFileIntoMemory("D:\\c_proj\\assets\\develop\\man\\man2.bmp", &game_memory->file_cache_arena);
        g_GlobalState->man_bitmap = ReadLoadedBmp(man_bmp_file, &game_memory->dynamic_storage_arena);

        WorldCoord hero_pos = {};
        hero_pos.on_tilemap = {0, 0};
        hero_pos.in_tile = {1.0f, 1.0f};

        g_GlobalState->hero.pos = hero_pos;

        g_GlobalState->hero.target_pos = g_GlobalState->hero.pos;
        g_GlobalState->hero.dimensions = {0.8f, 1.4f};
        g_GlobalState->hero.speed = 0.0015f;


        g_GlobalState->camera_center = {5.0f, 2.5f};

        g_GlobalState->game_time_ms = 0.0f;
        g_GlobalState->time_delta_ms = 0.0f;
        g_GlobalState->world_scaling_factor = 1.0f;


        v2ui num_tiles = {30, 30};
        uint32 tiles_memory_size = num_tiles.x * num_tiles.y * sizeof(Tile);
        Tile * tiles = (Tile *)GetMemoryFromArena(tiles_memory_size, &game_memory->dynamic_storage_arena);

        uint32 color = 0;
        for (int tile_row = 0; tile_row < num_tiles.y; ++tile_row)
        {
            for (int tile_column = 0; tile_column < num_tiles.x; ++tile_column)
            {
                
                uint8 r = (tile_row * (256 / num_tiles.y)) % 256;
                uint8 g = (tile_column * (256 / num_tiles.x)) % 256;
                //uint8 b = 0;
                uint8 b = 255;

                color = ((((uint32)r) << 16) | (((uint32)g)<< 8) | b);
                Tile * current_tile_pointer = tiles + tile_row * num_tiles.x + tile_column;
                *current_tile_pointer = {1, color}; // making all tiles walkable
            }
        }

        TileMap tile_map;
        tile_map.num_tiles = num_tiles;
        tile_map.tile_length = 1.0f;
        tile_map.tiles = tiles;

        g_GlobalState->tile_map = tile_map;

        g_GlobalState->initialized = true;
    }

    g_GlobalState->game_time_ms += time_delta_ms;
    g_GlobalState->time_delta_ms = time_delta_ms;


    ProcessControlInput(control_input, bitmap_output_buffer);

    //SimulateTimeStep();

    // TODO: reimplement the function
    //RenderTileMap(bitmap_output_buffer);
    RenderHero(bitmap_output_buffer);
    //TODO
    //RenderCursorClick(bitmap_output_buffer);
    //RenderBitmap2(g_GlobalState->man_bitmap, bitmap_output_buffer, 0.0f, 0.0f);
}

} // extern "C"
