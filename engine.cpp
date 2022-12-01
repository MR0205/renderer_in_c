#include "macro_definitions.h"
#include "math.h"
#include "tilemap.h"
#include "platform.h"
#include "bitmap.h"
#include "bmp_parser.h"
#include "opentype.h"
//#include "win32.h"

struct GlobalState
{
    bool32 initialized;
    Entity hero;

    v2 camera_center;

    TileMap tile_map;


    real64 game_time_ms;
    real64 time_delta_ms;

    real64 world_scaling_factor;

    real64 right_button_click_timestamp;
    bool32 draw_right_button_click;
    real32 right_button_click_x;
    real32 right_button_click_y;

    bool32 draw_mouse_selection;
    real32 mouse_selection_min_x;
    real32 mouse_selection_min_y;
    real32 cursor_cur_x; // mouse_selection_max_x;
    real32 cursor_cur_y; // mouse_selection_max_y;

    Bitmap * man_bitmap;
    Glyphs glyphs;
    //Bitmap ** glyphs_bitmaps_pointers_array;
    //uint32 number_of_glyphs;

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

internal void
DrawRectangle(BitmapOutputBuffer * bitmap_output_buffer,
              real32 min_x_real, real32 min_y_real, real32 max_x_real, real32 max_y_real,
              uint32 color) //uint8 r, uint8 g, uint8 b)
{
    if (g_GlobalState->draw_mouse_selection && g_GlobalState->game_time_ms >= 3000.0f) {
        int a = 1;
    }
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

    int temp_min;
    int temp_max;

    temp_min = min_x;
    temp_max = max_x;
    min_x = temp_min < temp_max ? temp_min : temp_max;
    max_x = temp_min > temp_max ? temp_min : temp_max;

    temp_min = min_y;
    temp_max = max_y;
    min_y = temp_min < temp_max ? temp_min : temp_max;
    max_y = temp_min > temp_max ? temp_min : temp_max;

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
DrawBitmapAtScreenCoords(Bitmap * bitmap, BitmapOutputBuffer * bitmap_output_buffer, uint32 top_left_x, uint32 top_left_y)
{
    uint32 min_x = top_left_x;
    uint32 min_y = top_left_y;
    uint32 max_x = min_x + bitmap->width;
    uint32 max_y = min_y + bitmap->height;

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
DrawBitmap(Bitmap * bitmap, BitmapOutputBuffer * bitmap_output_buffer, real32 center_x_real, real32 center_y_real)
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
RenderEntity(BitmapOutputBuffer * bitmap_output_buffer)
{
    //v2 hero_pos_min = g_GlobalState->hero.pos - 0.5 * g_GlobalState->hero.dimensions;
    //v2 hero_pos_max = hero_pos_min + g_GlobalState->hero.dimensions;

    //uint32 hero_color = 0x000000FF;
    //real32 hero_border_width = 0.05f;
    //uint32 hero_border_color = 0x00FFFFFF;
    DrawBitmap(g_GlobalState->man_bitmap, bitmap_output_buffer, g_GlobalState->hero.pos.x, g_GlobalState->hero.pos.y);
    // DrawRectangle(bitmap_output_buffer, hero_pos_min.x, hero_pos_min.y,
    //               hero_pos_max.x, hero_pos_max.y, hero_border_color);
    // DrawRectangle(bitmap_output_buffer, hero_pos_min.x + hero_border_width,
    //               hero_pos_min.y + hero_border_width, hero_pos_max.x - hero_border_width,
    //               hero_pos_max.y - hero_border_width, hero_color);
}

internal void
RenderTileMap(BitmapOutputBuffer * bitmap_output_buffer)
{
    TileMap * tile_map =  &g_GlobalState->tile_map;
    uint32 color = 0;


    for (int entity_index = 0; entity_index < tile_map->num_entities; ++entity_index)
    {
        Entity * cur_tile_entity = tile_map->entities + entity_index;

        if (cur_tile_entity->entity_type != rectangle_entity_type)
        {
            continue;
        }
        v2 tile_min = cur_tile_entity->pos - (cur_tile_entity->dimensions * 0.5f);
        v2 tile_max = cur_tile_entity->pos + (cur_tile_entity->dimensions * 0.5f);

        color = cur_tile_entity->color;
        DrawRectangle(bitmap_output_buffer, tile_min.x,  tile_min.y, 
                      tile_max.x, tile_max.y, color);
    }
}

internal void
RenderRightButtonClick(BitmapOutputBuffer * bitmap_output_buffer)
{
    if (g_GlobalState->draw_right_button_click && 
        (g_GlobalState->game_time_ms - g_GlobalState->right_button_click_timestamp < 100.0f))
    {
        real32 right_button_click_x = g_GlobalState->right_button_click_x;
        real32 right_button_click_y = g_GlobalState->right_button_click_y;

        real32 right_click_square_highlight_edge_lenth = 0.1f;

        DrawRectangle(bitmap_output_buffer, 
                      right_button_click_x - right_click_square_highlight_edge_lenth/2,
                      right_button_click_y - right_click_square_highlight_edge_lenth/2,
                      right_button_click_x + right_click_square_highlight_edge_lenth/2,
                      right_button_click_y + right_click_square_highlight_edge_lenth/2,
                      0x00888888);
    }
}


// TODO; transparent selection
internal void
RenderMouseSelection(BitmapOutputBuffer * bitmap_output_buffer)
{
    if (g_GlobalState->draw_mouse_selection)
    {

        real32 selection_rectangle_border_thickness = 0.05f;
        uint32 selection_rectangle_border_color = 0x0000FF00;

        uint32 selection_rectangle_color = 0x00FFFFFF;

        DrawRectangle(bitmap_output_buffer, 
                      g_GlobalState->mouse_selection_min_x,
                      g_GlobalState->mouse_selection_min_y,
                      g_GlobalState->cursor_cur_x,
                      g_GlobalState->cursor_cur_y,
                      selection_rectangle_color);
    }
}

// TODO: didn't want to pass here bitmap_output_buffer as we don't need to change it
// but we keep using its px_to_m to convert coordinates
// may be think of how we can restructure the code and move out probably this information
// to somthing globally available, to keep arguments lists short

// for mouse buttons events we could just store in global state the px values of the screen positions
// when calling the redner function we will have acess to bitmap_output_buffer where we could
// get the coordinates in the world position coordinates and then draw rectangle with them.
// havent yet thought it trough completely though

// for the other events need to do the analysis

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

    ///////////////////////////////////////////////
    ///////////////// HANDLING MOUSE BUTTON EVENTS
    MouseEvent * last_mouse_event = &control_input->last_mouse_event;

    int32 screen_x;
    int32 screen_y;
    real32 world_x;
    real32 world_y;

    // NOTE: we pass the mouse move event coordinates to the handling code of all cases
    // even though we get also in cases of OS button events another set of coordinates
    // does this make practical difference? 
    // i prefer it as it is, as it simplifies my code here, though it makes completely redundant mouse_event.x and mouse_event.y
    screen_x = control_input->cursor_x;
    screen_y = control_input->cursor_y;
    GetWorldCoordinatesFromGameScreenCoordinates(bitmap_output_buffer,
                                                 screen_x, screen_y,
                                                 &world_x, &world_y);
    g_GlobalState->cursor_cur_x = world_x;
    g_GlobalState->cursor_cur_y = world_y;

    if (last_mouse_event->type == RIGHT_BUTTON_PRESSED)
    {
        g_GlobalState->draw_right_button_click = true;
        g_GlobalState->right_button_click_x = world_x;
        g_GlobalState->right_button_click_y = world_y;
        g_GlobalState->right_button_click_timestamp = g_GlobalState->game_time_ms;

        g_GlobalState->hero.target_pos = v2{world_x, world_y};
    }

    /////////////////////////////////////////////////

    if (last_mouse_event->type == LEFT_BUTTON_PRESSED)
    {
        g_GlobalState->draw_mouse_selection = true;
        g_GlobalState->mouse_selection_min_x = world_x;
        g_GlobalState->mouse_selection_min_y = world_y;
    }


    if (last_mouse_event->type == LEFT_BUTTON_RELEASED)
    {
        g_GlobalState->draw_mouse_selection = false;
    }

    /////////////////////////////////////////////////

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

internal void
RenderText(char text[], BitmapOutputBuffer * bitmap_output_buffer, uint32 pen_start_pos_x, uint32 pen_start_pos_y)
{
    uint32 pen_pos_x = pen_start_pos_x;
    uint32 pen_pos_y = pen_start_pos_y; 

    //for (size_t char_index=0; char_index < text.char_number; ++char_index)
    int16 question_mark_code = 0x3F;

    for (size_t char_index = 0; text[char_index] != '\0'; ++char_index)
    {
        size_t bitmap_index = 0;
        uint16 char_code = text[char_index];
        if (char_code < g_GlobalState->glyphs.unicode_range.start || char_code > g_GlobalState->glyphs.unicode_range.end)
        {
            bitmap_index = question_mark_code - g_GlobalState->glyphs.unicode_range.start;
        } else
        {
            bitmap_index = char_code - g_GlobalState->glyphs.unicode_range.start;
        }

        Bitmap * cur_glyph_bitmap_pointer = g_GlobalState->glyphs.bitmaps + bitmap_index;

        
        uint32 top_left_x = pen_pos_x + g_GlobalState->glyphs.bearing_x;
        uint32 top_left_y = pen_pos_y - g_GlobalState->glyphs.bearing_y;

        DrawBitmapAtScreenCoords(cur_glyph_bitmap_pointer, bitmap_output_buffer, top_left_x, top_left_y);
        
        pen_pos_x += g_GlobalState->glyphs.advance_x;
    }

}

internal void
RenderFontPreview(BitmapOutputBuffer * bitmap_output_buffer)
{
    for (size_t glyph_index=0; glyph_index < g_GlobalState->glyphs.unicode_range.number; ++glyph_index)
    {
        Bitmap * cur_glyph_bitmap_pointer = g_GlobalState->glyphs.bitmaps + glyph_index;
        DrawBitmap(cur_glyph_bitmap_pointer, bitmap_output_buffer, glyph_index * 0.5, 0);
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

    size_t memory_distributed = 0;
    ASSERT(game_memory->permanent_storage_size >= sizeof(GlobalState));
    g_GlobalState = (GlobalState *) game_memory->permanent_storage + memory_distributed;
    memory_distributed += sizeof(GlobalState);

    if (!g_GlobalState->initialized)
    {
        // size_t file_cache_arena_size = 256 * MEGABYTE; 
        // ASSERT(game_memory->permanent_storage_size >= memory_distributed + file_cache_arena_size);
        // game_memory->file_cache_arena = {};
        // game_memory->file_cache_arena.first_byte = game_memory->permanent_storage + memory_distributed;
        // game_memory->file_cache_arena.size = file_cache_arena_size;
        // memory_distributed += file_cache_arena_size;

        size_t rest_of_the_permanent_storage_size = game_memory->permanent_storage_size - memory_distributed;
        size_t dynamic_storage_arena_size = rest_of_the_permanent_storage_size;
        ASSERT(game_memory->permanent_storage_size >= memory_distributed + dynamic_storage_arena_size);
        game_memory->dynamic_storage_arena = {};
        game_memory->dynamic_storage_arena.first_byte = game_memory->permanent_storage + memory_distributed;
        game_memory->dynamic_storage_arena.size = dynamic_storage_arena_size;
        memory_distributed += dynamic_storage_arena_size;

        FileReadResult man_bmp_file = platform_procedures->ReadFileIntoMemory("..\\assets\\develop\\man\\man2.bmp");
        g_GlobalState->man_bitmap = ReadLoadedBmp(man_bmp_file, &game_memory->dynamic_storage_arena);
        platform_procedures->FreeFileReadResultFromMemory(man_bmp_file);

        FileReadResult msgothic_font_file = platform_procedures->ReadFileIntoMemory("..\\assets\\msgothic.ttc");
        g_GlobalState->glyphs = ParseFontFile(msgothic_font_file, &game_memory->dynamic_storage_arena);
        platform_procedures->FreeFileReadResultFromMemory(msgothic_font_file);


        //TODO: for file sizes and memory sizes use portable data type size_t
        //TODO: implement freeing memory after reading file and parsing it. Need function Free Memory in win32 layer that will be passed
        // in the same way as ReadFileIntoMemory.

        // TODO: implement reading the font file
        //void * font_file_bytes = platform_procedures->ReadFileIntoMemory("D:\\c_proj\\assets\\cozette_bitmap.ttf", &game_memory->file_cache_arena);

        g_GlobalState->hero.pos = {1.0f, 1.0f};
        g_GlobalState->hero.target_pos = g_GlobalState->hero.pos;
        //g_GlobalState->hero.dimensions = {0.8f, 1.4f};
        g_GlobalState->hero.speed = 0.0015f;
        g_GlobalState->hero.entity_type = hero_entity_type;


        g_GlobalState->camera_center = {5.0f, 2.5f};

        g_GlobalState->game_time_ms = 0.0f;
        g_GlobalState->time_delta_ms = 0.0f;
        g_GlobalState->world_scaling_factor = 1.0f;


        v2ui num_tiles = {30, 30};
        uint32 tiles_memory_size = num_tiles.x * num_tiles.y * sizeof(Entity);
        Entity * tiles = (Entity *)GetMemoryFromArena(tiles_memory_size, &game_memory->dynamic_storage_arena);

        uint32 color = 0;
        real32 tile_length = 1.0f;
        real32 tile_width = tile_length;
        real32 tile_height = tile_length;

        for (int tile_row = 0; tile_row < num_tiles.y; ++tile_row)
        {
            for (int tile_column = 0; tile_column < num_tiles.x; ++tile_column)
            {
                
                uint8 r = (tile_row * (256 / num_tiles.y)) % 256;
                uint8 g = (tile_column * (256 / num_tiles.x)) % 256;
                uint8 b = 255;

                color = ((((uint32)r) << 16) | (((uint32)g)<< 8) | b);
                Entity * current_tile_pointer = tiles + tile_row * num_tiles.x + tile_column;

                v2 tile_pos = {(real32) tile_column * tile_width, (real32) tile_row * tile_height};
                tile_pos += v2{tile_width/2.0f, tile_height/2.0f};

                current_tile_pointer->pos = tile_pos; 
                current_tile_pointer->speed = 0.0f; 
                current_tile_pointer->target_pos = v2{0.0f, 0.0f};

                current_tile_pointer->dimensions = v2{tile_width, tile_height};
                current_tile_pointer->entity_type = rectangle_entity_type;
                current_tile_pointer->color = color;

                if (tile_row % 2 || tile_column % 2)
                {
                    current_tile_pointer->entity_type = no_entity_type;
                }
            }
        }

        TileMap tile_map;
        tile_map.num_entities = num_tiles.x * num_tiles.y;
        tile_map.entities = tiles;

        g_GlobalState->tile_map = tile_map;

        g_GlobalState->initialized = true;
    }

    g_GlobalState->game_time_ms += time_delta_ms;
    g_GlobalState->time_delta_ms = time_delta_ms;


    ProcessControlInput(control_input, bitmap_output_buffer);

    SimulateTimeStep();

    RenderTileMap(bitmap_output_buffer);
    RenderEntity(bitmap_output_buffer);
    RenderRightButtonClick(bitmap_output_buffer);
    RenderMouseSelection(bitmap_output_buffer);
    //RenderFontPreview(bitmap_output_buffer);
    RenderText("HELLO ILYA!", bitmap_output_buffer, 50, 20);
    RenderText("Some simple text test.", bitmap_output_buffer, 20, 20);
    RenderText("1+2-3*4=5_6 7[8,9] {10,11}", bitmap_output_buffer, 20, 40);
    //RenderBitmap2(g_GlobalState->man_bitmap, bitmap_output_buffer, 0.0f, 0.0f);
    //uint32 my_color = ((((uint32)255) << 16) | (((uint32)255)<< 8) | 255);
    //DrawRectangle(bitmap_output_buffer, 1, 1, 2, 2, my_color);
}

} // extern "C"
