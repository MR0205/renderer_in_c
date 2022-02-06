#include "macro_definitions.h"
#include "math.h"
#include "tilemap.h"
#include "platform.h"

//#include "win32.h"


struct Bitmap
{
    uint32 width;
    uint32 height;
    uint32 * bits;
};

struct UnicodeRange
{
    uint16 start = 0x20;
    uint16 end = 0x7E;
    uint32 number = (uint32) (end - start + 1);
};

struct Glyphs
{
    UnicodeRange unicode_range;
    Bitmap * bitmaps;
    //SmallGlyphMetrics * metrics;
};

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
    uint16 bits_per_pixel;
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
    uint16 bits_per_pixel;
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
ReadLoadedBmp(FileReadResult bmp_file, MemoryArena * memory_arena)
{
    uint8 * bmp_file_base_pointer = bmp_file.start_pointer;
    BmpFileHeader bmp_file_header = *((BmpFileHeader *) bmp_file_base_pointer);

    uint32 bitmap_height;
    uint32 bitmap_width;
    uint32 bits_per_pixel;

    uint32 bitmap_header_size = *((uint32 *) (bmp_file_base_pointer + sizeof(BmpFileHeader)));
    if (bitmap_header_size == 40)
    {
        BmpBitmapV1Header bmp_bitmap_v1_header = *((BmpBitmapV1Header *) (bmp_file_base_pointer + sizeof(BmpFileHeader)));
        bitmap_height = bmp_bitmap_v1_header.bitmap_height;
        bitmap_width = bmp_bitmap_v1_header.bitmap_width;
        bits_per_pixel = bmp_bitmap_v1_header.bits_per_pixel;
    } else if (bitmap_header_size == 124)
    {
        BmpBitmapV5Header bmp_bitmap_v5_header = *((BmpBitmapV5Header *) (bmp_file_base_pointer + sizeof(BmpFileHeader)));
        //debug
        uint32 size_of_header_as_defined = sizeof(BmpBitmapV5Header);
        bitmap_height = bmp_bitmap_v5_header.bitmap_height;
        bitmap_width = bmp_bitmap_v5_header.bitmap_width;
        bits_per_pixel = bmp_bitmap_v5_header.bits_per_pixel;

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
    return_bitmap->bits = (uint32 *) GetMemoryFromArena(bits_per_pixel * bitmap_width * bitmap_height, memory_arena);

    uint32 * src = (uint32 *) (bmp_file_base_pointer + bmp_file_header.offset_to_bits);
    uint32 * dest = (uint32 *) (return_bitmap->bits + bitmap_width * (bitmap_height - 1));
    uint32 * src_px;
    uint32 * dest_px;

    // TODO: bad naming, row_index, column_index relative to what? it is relative to the displacement
    // from the start of the data in the source bitmap, which corresponds iterating bottom-up, left-to-right
    // but it is not evident. I implement parsing of a font with different names
    for (uint32 row_index = 0; row_index < bitmap_height; ++row_index)
    {
        uint32 * src_row = src + row_index * bitmap_width;
        uint32 * dest_row = dest - row_index * bitmap_width;
        for (uint32 column_index = 0; column_index < bitmap_width; ++column_index)
        {
            src_px = src_row + column_index;
            dest_px = dest_row + column_index;
            *dest_px = *src_px;
        }
    }

    return return_bitmap;
}

#pragma pack(1)
struct TTCHeader
{
    char tag[4];
    uint16 major_version;
    uint16 minor_version;
    uint32 num_fonts;
};

#pragma pack(1)
struct TableRecord
{
    char table_tag[4];
    uint32 checksum;
    uint32 offset;
    uint32 length;
};


#pragma pack(1)
struct TableDirectory
{
    uint32 sfnt_version;
    uint16 num_tables;
    uint16 search_range;
    uint16 entry_selector;
    uint16 range_shift;
    //TableRecord * table_records;
};

#pragma pack(1)
struct FontHeaderTable
{
    uint16 major_version;
    uint16 minor_version;
    uint32 font_revision;
    uint32 checksum_adjustment;
    uint32 magic_number;
    uint16 flags;
    uint16 units_per_em;
    int64 created;
    int64 modified;
    int16 x_min;
    int16 y_min;
    int16 x_max;
    int16 y_max;
    uint16 mac_style;
    uint16 smallest_readable_size_px;
    int16 font_direction_hint;
    int16 index_to_loc_format;
    int16 glyph_data_format;
};

#pragma pack(1)
struct EBDTTableHeader
{
    uint16 major_version;
    uint16 minor_version;
};

#pragma pack(1)
struct EBLCTableHeader
{
    uint16 major_version;
    uint16 minor_version;
    uint32 num_sizes;
};

#pragma pack(1)
struct SbitLineMetricsRecord
{
    int8 ascender;
    int8 descender;
    uint8 width_max;
    int8 caret_slope_numerator;
    int8 caret_slope_denominator;
    int8 caret_offset;
    int8 min_origin_sb;
    int8 min_advance_sb;
    int8 max_before_bl;
    int8 min_after_bl;
    int8 pad1;
    int8 pad2;
};

#pragma pack(1)
struct BitmapSizeRecord
{
    uint32 index_subtable_array_offset;
    uint32 index_tables_size;
    uint32 number_of_index_subtables;
    uint32 color_ref;
    SbitLineMetricsRecord hori;
    SbitLineMetricsRecord vert;
    uint16 start_glyph_index;
    uint16 end_glyph_index;
    uint8 ppem_x;
    uint8 ppem_y;
    uint8 bit_depth;
    int8 flags;
};

#pragma pack(1)
struct IndexSubTableArrayEntry
{
    uint16 first_glyph_index;
    uint16 last_glyph_index;
    uint32 additional_offset_to_index_subtable;
};

#pragma pack(1)
struct IndexSubTableHeader
{
    uint16 index_format;
    uint16 image_format;
    uint32 offset_to_subtable_ebdt_data;
};

#pragma pack(1)
struct BigGlyphMetrics
{
    uint8 height;
    uint8 width;
    int8 hori_bearing_x;
    int8 hori_bearing_y;
    uint8 hori_advance;
    int8 vert_bearing_x;
    int8 vert_bearing_y;
    uint8 vert_advance;
};

#pragma pack(1)
struct SmallGlyphMetrics
{
    uint8 height;
    uint8 width;
    int8 bearing_x;
    int8 bearing_y;
    uint8 advance;
};

#pragma pack(1)
struct CmapTableHeader
{
    uint16 version;
    uint16 num_encoding_tables;
};

#pragma pack(1)
struct EncodingRecord
{
    uint16 platform_id;
    uint16 encoding_id;
    uint32 subtable_offset;
};

#pragma pack(1)
struct Format4EncodingSubtable 
{
    uint16 format;
    uint16 length;
    uint16 language;
    uint16 seg_count_x2;
    uint16 search_range;
    uint16 entry_selector;
    uint16 range_shift;
};






struct Format4EncodingSubtableSegment
{
    uint16 end_code;
    uint16 start_code;
    int16 id_delta;
    uint16 id_range_offset;
};







// #pragma pack(1)
// struct IndexSubTableFormat2
// {
//     IndexSubTableHeader index_subtable_header;
//     uint32 image_size;
//     BigGlyphMetrics big_metrics;
// };

bool32 StringsPrefixesEqual(char * str1, char * str2, size_t prefix_len)
{
    int index=0;
    while (str1[index] == str2[index] && index < prefix_len)
    {
        ++index;
    }
    if(index == prefix_len)
    {
        return true;
    } else
    {
        return false;
    }
}

internal int32
CalculateChecksum(uint8 * base_pointer, size_t length)
{
    uint32 checksum_calculated = 0;
    size_t byte_index=0;
    for (; byte_index < length; byte_index+=4)
    {
        if (byte_index + 4 <= length)
        {
            uint32 cur_addend = *((uint32 *)(base_pointer + byte_index));
            INVERT_BYTE_ORDER(cur_addend);
            checksum_calculated += cur_addend;
        }
    }


    if (byte_index != length)
    {
        byte_index -= 4;
        uint32 last_addend = 0;
        for (; byte_index < length; ++byte_index)
        {
            last_addend = (last_addend << 8) | (*((uint8 *)(base_pointer + byte_index)));
            last_addend = (*((uint8 *)(base_pointer + byte_index)));
        }
        last_addend <<= 8 * (4 - length%4);
        checksum_calculated += last_addend;
    }
    return checksum_calculated;
}








internal uint32 
GetChecksumAdjustmentFromHeadTable(uint8 * table_pointer)
{
    FontHeaderTable font_header_table = *((FontHeaderTable *) table_pointer);
    INVERT_BYTE_ORDER(font_header_table.major_version);
    INVERT_BYTE_ORDER(font_header_table.minor_version);
    INVERT_BYTE_ORDER(font_header_table.font_revision);
    INVERT_BYTE_ORDER(font_header_table.checksum_adjustment);
    INVERT_BYTE_ORDER(font_header_table.magic_number);
    INVERT_BYTE_ORDER(font_header_table.flags);
    INVERT_BYTE_ORDER(font_header_table.units_per_em);
    INVERT_BYTE_ORDER(font_header_table.created);
    INVERT_BYTE_ORDER(font_header_table.modified);
    INVERT_BYTE_ORDER(font_header_table.x_min);
    INVERT_BYTE_ORDER(font_header_table.y_min);
    INVERT_BYTE_ORDER(font_header_table.x_max);
    INVERT_BYTE_ORDER(font_header_table.y_max);
    INVERT_BYTE_ORDER(font_header_table.mac_style);
    INVERT_BYTE_ORDER(font_header_table.smallest_readable_size_px);
    INVERT_BYTE_ORDER(font_header_table.font_direction_hint);
    INVERT_BYTE_ORDER(font_header_table.index_to_loc_format);
    INVERT_BYTE_ORDER(font_header_table.glyph_data_format);

    //checksum_calculated -= font_header_table.checksum_adjustment;
    uint32 font_head_checksum_adjustment = font_header_table.checksum_adjustment;
    return font_head_checksum_adjustment;
}


internal EBDTTableHeader
ReadEBDTTableHeader(uint8 * ebdt_table_pointer)
{
    EBDTTableHeader ebdt_table_header = *((EBDTTableHeader *) ebdt_table_pointer);
    INVERT_BYTE_ORDER(ebdt_table_header.major_version);
    INVERT_BYTE_ORDER(ebdt_table_header.minor_version);
    return ebdt_table_header;
}

// TODO: refactor the structs

struct GlyphsIds
{
    uint16 * ids;
    UnicodeRange unicode_range;
};


// NOTE: the assumption that we make is that the whole unicode_range that we search for
// is contained within a single segment of cmap table. it simplifies our implementation
// If on the other hand, there will be the case where we will have a range that spans multiple segments
// it the font file, for one reason or another, we will have to add searcning across all the spans
// which is easy, we only need to persist the state of found glyphs.
// There is also a related problem of searching not for a continous unicode range, but for an arbitrary set of unicode codes
// that are not neccesarly ordered.
// So this code does not pretend to provide a proper implementation of the parsing of the 
// cmap table
//
internal GlyphsIds 
GetGlyphIdsFromCmapTable(uint8 * cmap_table_pointer, UnicodeRange unicode_range, MemoryArena * memory_arena)
{
    GlyphsIds ascii_glyphs_ids;
    ascii_glyphs_ids.unicode_range = unicode_range;
    // TODO: do allocations on the stack wherever possible, refactor this when learn what combination of "const" and "static" allows using the member
    // of a struct as a specifier of an array size in its stack allocation.
    // This ascii_glyphs_ids from now on pollutes the memory, as it is only used once as a return from this function. Replace it with an array?
    ascii_glyphs_ids.ids = (uint16 *) GetMemoryFromArena( sizeof(uint16) * unicode_range.number, memory_arena);

    CmapTableHeader cmap_table_header = *((CmapTableHeader *) cmap_table_pointer);
    INVERT_BYTE_ORDER(cmap_table_header.version);
    INVERT_BYTE_ORDER(cmap_table_header.num_encoding_tables);

    
    for (size_t encoding_index=0; encoding_index<cmap_table_header.num_encoding_tables; ++encoding_index)
    {
        EncodingRecord encoding_record = *((EncodingRecord *)(cmap_table_pointer + sizeof(CmapTableHeader) + encoding_index * sizeof(EncodingRecord)));
        INVERT_BYTE_ORDER(encoding_record.platform_id);
        INVERT_BYTE_ORDER(encoding_record.encoding_id);
        INVERT_BYTE_ORDER(encoding_record.subtable_offset);

        // format 4
        // uint16 UNICODE_PLATFORM_ID = 0;
        //uint16 BMP_ONLY_UNICODE_ENCODING_ID = 3; // BMP = basic multilingual plane


        // format 4
        uint16 WINDOWS_PLATFORM_ID = 3;
        uint16 WINDOWS_UNICODE_BMP_ONLY_ENCODING_ID = 1;

        //if (encoding_record.platform_id == UNICODE_PLATFORM_ID && encoding_record.encoding_id == BMP_ONLY_UNICODE_ENCODING_ID)
        if (encoding_record.platform_id == WINDOWS_PLATFORM_ID && encoding_record.encoding_id == WINDOWS_UNICODE_BMP_ONLY_ENCODING_ID)
        {
            uint8 * bmp_unicode_subtable_pointer = cmap_table_pointer + encoding_record.subtable_offset;
            uint16 subtable_format = *((uint16 *) bmp_unicode_subtable_pointer);
            INVERT_BYTE_ORDER(subtable_format);
            int a = 1;

            ASSERT(subtable_format == 4);

            Format4EncodingSubtable format4_bmp_unicode_subtable = *((Format4EncodingSubtable *) (cmap_table_pointer + encoding_record.subtable_offset));
            INVERT_BYTE_ORDER(format4_bmp_unicode_subtable.format);
            INVERT_BYTE_ORDER(format4_bmp_unicode_subtable.length);
            INVERT_BYTE_ORDER(format4_bmp_unicode_subtable.language);
            INVERT_BYTE_ORDER(format4_bmp_unicode_subtable.seg_count_x2);
            INVERT_BYTE_ORDER(format4_bmp_unicode_subtable.search_range);
            INVERT_BYTE_ORDER(format4_bmp_unicode_subtable.entry_selector);
            INVERT_BYTE_ORDER(format4_bmp_unicode_subtable.range_shift);

            uint32 seg_count = format4_bmp_unicode_subtable.seg_count_x2 / 2;

            // NOTE: Although we have one of the arrays of int16 with the elements of the size uint16, we do calculate its size like it had uint16 elements
            // it could be confusing, but provides nice looking code which is easier to look at.
            uint16 * end_code_array =          (uint16 *) (cmap_table_pointer + encoding_record.subtable_offset + sizeof(format4_bmp_unicode_subtable));
            uint16 * start_code_array =        (uint16 *) (cmap_table_pointer + encoding_record.subtable_offset + sizeof(format4_bmp_unicode_subtable) + 1 * seg_count * sizeof(uint16) + sizeof(uint16));
            int16 * id_delta_array =           ( int16 *) (cmap_table_pointer + encoding_record.subtable_offset + sizeof(format4_bmp_unicode_subtable) + 2 * seg_count * sizeof(uint16) + sizeof(uint16));
            uint16 * id_range_offsets_array =  (uint16 *) (cmap_table_pointer + encoding_record.subtable_offset + sizeof(format4_bmp_unicode_subtable) + 3 * seg_count * sizeof(uint16) + sizeof(uint16)); 
            uint16 * glyph_id_array =          (uint16 *) (cmap_table_pointer + encoding_record.subtable_offset + sizeof(format4_bmp_unicode_subtable) + 4 * seg_count * sizeof(uint16) + sizeof(uint16)); 

            // looking for a first segment that should contained the whole unicode_range
            for (size_t seg_index = 0; seg_index < seg_count; ++seg_index)
            {
                Format4EncodingSubtableSegment segment;
                segment.end_code = *(end_code_array + seg_index);
                segment.start_code = *(start_code_array + seg_index);
                segment.id_delta = *(id_delta_array + seg_index);
                segment.id_range_offset = *(id_range_offsets_array + seg_index);

                INVERT_BYTE_ORDER(segment.end_code);
                INVERT_BYTE_ORDER(segment.start_code);
                INVERT_BYTE_ORDER(segment.id_delta);
                INVERT_BYTE_ORDER(segment.id_range_offset);

                // exloiting the assumption that the whole unicode range with unicode_range.end >= unicode_range.start must be contained
                // within a single segment
                if (unicode_range.end <= segment.end_code)
                {
                    if(unicode_range.start >= segment.start_code)
                    {
                        for (uint16 unicode_code = unicode_range.start, within_range_index=0; unicode_code <= unicode_range.end; ++unicode_code, ++within_range_index)
                        {
                            if (segment.id_range_offset != 0)
                            {
                                uint16 glyph_id  = *(id_range_offsets_array + seg_index + segment.id_range_offset/(sizeof(*id_range_offsets_array)/sizeof(uint8)) + unicode_code - segment.start_code);
                                if (glyph_id != 0)
                                {
                                    ascii_glyphs_ids.ids[within_range_index] = (glyph_id + segment.id_delta) % 0xFFFF;
                                } else
                                {
                                    ascii_glyphs_ids.ids[within_range_index] = 0;
                                }
                            } else
                            {
                                ascii_glyphs_ids.ids[within_range_index] = (unicode_code + segment.id_delta) % 0xFFFF;
                            }
                        }
                        // all the unicode_range must have been search for corresponding ids.
                        break;
                    }
                    else
                    {
                        // our assumption that the font and our specified unicode range is broken
                        // she segment is suppossesd to contain the WHOLE searched for unicode_range, but it doesn't
                        ASSERT(0);
                        break;
                    } 
                }

                int a = 1;
            }
        }
    }

    bool32 ascii_glyps_ids_are_sorted = true;
    ASSERT(ascii_glyphs_ids.unicode_range.number > 0);
    for (size_t glyph_index=1; glyph_index<ascii_glyphs_ids.unicode_range.number; ++glyph_index)
    {
        if (ascii_glyphs_ids.ids[glyph_index-1] > ascii_glyphs_ids.ids[glyph_index])
        {
            ascii_glyps_ids_are_sorted = false;
        } else if (ascii_glyphs_ids.ids[glyph_index-1] == ascii_glyphs_ids.ids[glyph_index]) 
        {
            if (ascii_glyphs_ids.ids[glyph_index] != 0)
            {
                ascii_glyps_ids_are_sorted = false;
            }

        } else //ascii_glyphs_ids.ids[glyph_index-1] < ascii_glyphs_ids.ids[glyph_index]
        {
            continue;
        }
    }
    ASSERT(ascii_glyps_ids_are_sorted);
    return ascii_glyphs_ids;
}

internal void
InvertByteOrderInBitmapSizeRecord(BitmapSizeRecord * bitmap_size_record)
{
    INVERT_BYTE_ORDER(bitmap_size_record->index_subtable_array_offset);
    INVERT_BYTE_ORDER(bitmap_size_record->index_tables_size);
    INVERT_BYTE_ORDER(bitmap_size_record->number_of_index_subtables);
    INVERT_BYTE_ORDER(bitmap_size_record->color_ref);
    INVERT_BYTE_ORDER(bitmap_size_record->start_glyph_index);
    INVERT_BYTE_ORDER(bitmap_size_record->end_glyph_index);
    INVERT_BYTE_ORDER(bitmap_size_record->ppem_x);
    INVERT_BYTE_ORDER(bitmap_size_record->ppem_y);
    INVERT_BYTE_ORDER(bitmap_size_record->bit_depth);
    INVERT_BYTE_ORDER(bitmap_size_record->flags);

    INVERT_BYTE_ORDER(bitmap_size_record->hori.ascender);
    INVERT_BYTE_ORDER(bitmap_size_record->hori.descender);
    INVERT_BYTE_ORDER(bitmap_size_record->hori.width_max);
    INVERT_BYTE_ORDER(bitmap_size_record->hori.caret_slope_numerator);
    INVERT_BYTE_ORDER(bitmap_size_record->hori.caret_slope_denominator);
    INVERT_BYTE_ORDER(bitmap_size_record->hori.caret_offset);
    INVERT_BYTE_ORDER(bitmap_size_record->hori.min_origin_sb);
    INVERT_BYTE_ORDER(bitmap_size_record->hori.min_advance_sb);
    INVERT_BYTE_ORDER(bitmap_size_record->hori.max_before_bl);
    INVERT_BYTE_ORDER(bitmap_size_record->hori.min_after_bl);
    INVERT_BYTE_ORDER(bitmap_size_record->hori.pad1);
    INVERT_BYTE_ORDER(bitmap_size_record->hori.pad2);

    INVERT_BYTE_ORDER(bitmap_size_record->vert.ascender);
    INVERT_BYTE_ORDER(bitmap_size_record->vert.descender);
    INVERT_BYTE_ORDER(bitmap_size_record->vert.width_max);
    INVERT_BYTE_ORDER(bitmap_size_record->vert.caret_slope_numerator);
    INVERT_BYTE_ORDER(bitmap_size_record->vert.caret_slope_denominator);
    INVERT_BYTE_ORDER(bitmap_size_record->vert.caret_offset);
    INVERT_BYTE_ORDER(bitmap_size_record->vert.min_origin_sb);
    INVERT_BYTE_ORDER(bitmap_size_record->vert.min_advance_sb);
    INVERT_BYTE_ORDER(bitmap_size_record->vert.max_before_bl);
    INVERT_BYTE_ORDER(bitmap_size_record->vert.min_after_bl);
    INVERT_BYTE_ORDER(bitmap_size_record->vert.pad1);
    INVERT_BYTE_ORDER(bitmap_size_record->vert.pad2);

}


// currently taking the glyphs from the first strike  
// may be it is better idea to implement a different interface
// for example search for a certain glyph size etc.
// could be that the different sizes have different glyphs represented?
// which affects the corecness of our search, and thus makes
// the assumption of using the first strike erroneous
//

// currently assuming that the whole range of glyph_ids is situated in the single rangle subtable in the eblc strike.
internal Glyphs 
ExtractGlyphsFromEBDTUsingEBLCTable(GlyphsIds glyphs_ids, uint8 * eblc_table_pointer, uint8 * ebdt_table_pointer, MemoryArena * memory_arena)
{
    EBDTTableHeader ebdt_table_header = ReadEBDTTableHeader(ebdt_table_pointer);
    Glyphs glyphs;
    glyphs.unicode_range = glyphs_ids.unicode_range;

    glyphs.bitmaps = (Bitmap *) GetMemoryFromArena(sizeof(Bitmap) * glyphs_ids.unicode_range.number, memory_arena);
    //glyphs.metrics = (SmallGlyphMetrics *) GetMemoryFromArena(sizeof(SmallGlyphMetrics) * glyphs_ids.unicode_range.number);

    // parse_font_file_result.glyphs_bitmaps_pointers_array = (Bitmap **) GetMemoryFromArena(sizeof(Bitmap *) * number_of_glyphs_in_strike, memory_arena);
    // parse_font_file_result.number_of_glyphs = 0;

    EBLCTableHeader eblc_table_header = *((EBLCTableHeader *) eblc_table_pointer);
    INVERT_BYTE_ORDER(eblc_table_header.major_version);
    INVERT_BYTE_ORDER(eblc_table_header.minor_version);
    INVERT_BYTE_ORDER(eblc_table_header.num_sizes);

    size_t num_glyphs_found = 0;

    for (size_t strike_index=0; strike_index<eblc_table_header.num_sizes; ++strike_index)
    {
        BitmapSizeRecord cur_bitmap_size_record = *((BitmapSizeRecord *) (eblc_table_pointer + sizeof(EBLCTableHeader) + strike_index*sizeof(BitmapSizeRecord)));
        InvertByteOrderInBitmapSizeRecord(&cur_bitmap_size_record);

        int8 HORIZONTAL_METRICS = 0x01;
        ASSERT(cur_bitmap_size_record.flags == HORIZONTAL_METRICS);
        ASSERT(cur_bitmap_size_record.bit_depth == 1);

        for (size_t range_subtable_index=0; range_subtable_index<cur_bitmap_size_record.number_of_index_subtables;++range_subtable_index)
        {
            IndexSubTableArrayEntry cur_index_subtable_array_entry = *((IndexSubTableArrayEntry *) (eblc_table_pointer + cur_bitmap_size_record.index_subtable_array_offset + range_subtable_index*sizeof(IndexSubTableArrayEntry) ));
            INVERT_BYTE_ORDER(cur_index_subtable_array_entry.first_glyph_index);
            INVERT_BYTE_ORDER(cur_index_subtable_array_entry.last_glyph_index);
            INVERT_BYTE_ORDER(cur_index_subtable_array_entry.additional_offset_to_index_subtable);

            if (cur_index_subtable_array_entry.first_glyph_index > glyphs_ids.ids[0] || cur_index_subtable_array_entry.last_glyph_index < glyphs_ids.ids[glyphs_ids.unicode_range.number-1])
            {
                // range doesn't contain the entire looked for glyphs_ids 
                continue;
            }

            IndexSubTableHeader cur_index_subtable_header = *((IndexSubTableHeader *) (eblc_table_pointer + cur_bitmap_size_record.index_subtable_array_offset + cur_index_subtable_array_entry.additional_offset_to_index_subtable));

            INVERT_BYTE_ORDER(cur_index_subtable_header.index_format);
            INVERT_BYTE_ORDER(cur_index_subtable_header.image_format);
            INVERT_BYTE_ORDER(cur_index_subtable_header.offset_to_subtable_ebdt_data);


            SmallGlyphMetrics cur_glyph_metrics;
            uint32 glyph_image_size;
            uint32 * eblc_subtable_format1_glyph_data_offsets_array_pointer = 0;
            // NOTE: there is no implementation for the format 1 of handling of missing glyph ids in the range (which must be implemented: verification that the next glyph image data offset is bigger than the current glyph image data offset (EBDT table))
            // subtable format 2 doesnt provide method, would that mean that the offset into ebdt would point to a 0id glyph if the index of glyph is to be missing from the eblc subtable range? don't know the answer

            // subtable 5 format for handling sparsness:
            uint32 num_present_glyphs_format5 = 0;
            uint16 * present_glyphs_ids_array_format5 = 0;
            if (cur_index_subtable_header.index_format==1)
            {
                eblc_subtable_format1_glyph_data_offsets_array_pointer = (uint32 *) (eblc_table_pointer + cur_bitmap_size_record.index_subtable_array_offset + cur_index_subtable_array_entry.additional_offset_to_index_subtable + sizeof(IndexSubTableHeader));

            } else if (cur_index_subtable_header.index_format==2 || cur_index_subtable_header.index_format==5)
            {
                // exploiting the layout of format 2 index subtable
                glyph_image_size = *((uint32 *) (eblc_table_pointer + cur_bitmap_size_record.index_subtable_array_offset + cur_index_subtable_array_entry.additional_offset_to_index_subtable + sizeof(IndexSubTableHeader)));
                BigGlyphMetrics big_glyph_metrics = *((BigGlyphMetrics *) (eblc_table_pointer + cur_bitmap_size_record.index_subtable_array_offset + cur_index_subtable_array_entry.additional_offset_to_index_subtable + sizeof(IndexSubTableHeader) + sizeof(uint32)));

                INVERT_BYTE_ORDER(glyph_image_size);
                INVERT_BYTE_ORDER(big_glyph_metrics.height);
                INVERT_BYTE_ORDER(big_glyph_metrics.width);
                INVERT_BYTE_ORDER(big_glyph_metrics.hori_bearing_x);
                INVERT_BYTE_ORDER(big_glyph_metrics.hori_bearing_y);
                INVERT_BYTE_ORDER(big_glyph_metrics.hori_advance);

                cur_glyph_metrics.height = big_glyph_metrics.height;
                cur_glyph_metrics.width = big_glyph_metrics.width;
                cur_glyph_metrics.bearing_x = big_glyph_metrics.hori_bearing_x;
                cur_glyph_metrics.bearing_y = big_glyph_metrics.hori_bearing_y;
                cur_glyph_metrics.advance = big_glyph_metrics.hori_advance;

                if (cur_index_subtable_header.index_format==5)
                {
                    num_present_glyphs_format5  = *((uint32 *) (eblc_table_pointer + cur_bitmap_size_record.index_subtable_array_offset + cur_index_subtable_array_entry.additional_offset_to_index_subtable + sizeof(IndexSubTableHeader) + sizeof(uint32) + sizeof(BigGlyphMetrics)));
                    present_glyphs_ids_array_format5  = (uint16 *) (eblc_table_pointer + cur_bitmap_size_record.index_subtable_array_offset + cur_index_subtable_array_entry.additional_offset_to_index_subtable + sizeof(IndexSubTableHeader) + sizeof(uint32) + sizeof(BigGlyphMetrics) + sizeof(uint32));
                    INVERT_BYTE_ORDER(num_present_glyphs_format5);
                }

            } else
            {
                // subtable format not implemented
                ASSERT(0);
            }


            if (cur_index_subtable_header.index_format==5)
            {
                size_t cur_glyph_index = 0;
                // TODO: the index could be searched with binary search 
                // according to the specification the glyph ids in the subtables indexes are stored in order
                // which allows us to search for our contiguous or non-contiguous range in one pass;
                for (size_t subtable_glyph_index=0; subtable_glyph_index < num_present_glyphs_format5 && cur_glyph_index < glyphs_ids.unicode_range.number; ++subtable_glyph_index)
                {
                    uint16 cur_glyph_id = present_glyphs_ids_array_format5[subtable_glyph_index];
                    INVERT_BYTE_ORDER(cur_glyph_id);
                    if (cur_glyph_id < glyphs_ids.ids[cur_glyph_index])
                    {
                        continue;
                    } else if (cur_glyph_id > glyphs_ids.ids[cur_glyph_index])
                    {
                        // the searched glyph_id is not represented in the current font. very unlikely?
                        ASSERT(0);
                    }

                    uint8 * glyph_ebdt_data_pointer = ebdt_table_pointer + cur_index_subtable_header.offset_to_subtable_ebdt_data + glyph_image_size * subtable_glyph_index;


                    Bitmap * glyph_bitmap = glyphs.bitmaps + cur_glyph_index;
                    glyph_bitmap->width = cur_glyph_metrics.width;
                    glyph_bitmap->height = cur_glyph_metrics.height;

                    // current bitmap renderer assumes 32 bit per pixelformat
                    uint32 bits_per_pixel = 32;
                    glyph_bitmap->bits = (uint32 *) GetMemoryFromArena(bits_per_pixel * glyph_bitmap->width * glyph_bitmap->height, memory_arena);

                    // row_index assumes top-down view of the source bitmap data embedded into font,
                   //  column_index - left-to-right

                    uint8 * glyph_image_data_pointer = glyph_ebdt_data_pointer;
                    uint32 * dest_pixel_pointer = glyph_bitmap->bits;
                    for (uint32 row_index = 0; row_index <  glyph_bitmap->height; ++row_index)
                    {
                        for (uint32 column_index = 0; column_index < glyph_bitmap->width; ++column_index)
                        {
                            int32 flat_index_of_a_bit = row_index * (glyph_bitmap->width) + column_index;
                            uint8 flat_index_of_source_byte = flat_index_of_a_bit / 8;
                            uint8 source_byte = *(glyph_image_data_pointer + flat_index_of_source_byte);
                            //INVERT_BYTE_ORDER(source_byte);
                            uint8 wanted_bit_extracted = (source_byte >> (7 - (flat_index_of_a_bit % 8))) & 0x01;
                            if (wanted_bit_extracted == 0x01)
                            {
                                *dest_pixel_pointer = 0x00000000;
                            } else if (wanted_bit_extracted == 0x00)
                            {
                                *dest_pixel_pointer = 0xFFFFFFFF;
                            } else
                            {
                                // unexpected
                                ASSERT(0);
                            }
                            ++dest_pixel_pointer;
                        }
                    }

                    ++cur_glyph_index;
                    ++num_glyphs_found;
                }

            } else  // else we iterate over dense array
            {
                // not implemented
                ASSERT(0);
            }

            // assuming that the searched range is contained in the single range lblc subtable we could break out as soon as we parsed first range where our range is present
            break;
        }

        //if (num_glyphs_found == glyphs_ids.unicode_range.number)
        //{
        //    break;
        //}
    //    break; // taking only the first strike of the eblc table
    }
    return glyphs;
}


// dense formats for eblc subtables
// {
// 
//             {
//                 for (size_t strike_glyph_index=cur_index_subtable_array_entry.first_glyph_index, subtable_glyph_index=0; strike_glyph_index<= cur_index_subtable_array_entry.last_glyph_index; ++strike_glyph_index, ++subtable_glyph_index)
//                 {
// 
//                     // if (strike_glyph_index < 661 || strike_glyph_index > 686)
//                     // {
//                     //     continue;
//                     // }
//                     //
//                     // TODO: stopped on iterating through glyph index subtables
//                     // the question of need completeness and exaustiveness of understanding of every
//                     // details and every byte of layout of these two tables is an open question still
//                     //
//                     ASSERT(ebdt_table_pointer != 0);
//                     // these are verification of the corectness of the file format, and not our code
//                     ASSERT(strike_glyph_index >= cur_bitmap_size_record.start_glyph_index);
//                     ASSERT(strike_glyph_index <= cur_bitmap_size_record.end_glyph_index);
//                     //uint8 * glyph_ebdt_data_pointer = ebdt_table_pointer + cur_index_subtable_header.offset_to_subtable_ebdt_data; 
// 
//                     if (cur_index_subtable_header.index_format==1)
//                     {
//                         ASSERT(eblc_subtable_format1_glyph_data_offsets_array_pointer);
//                         uint32 offset_into_ebdt_glyph_data = *(eblc_subtable_format1_glyph_data_offsets_array_pointer + subtable_glyph_index);
//                         uint32 offset_into_ebdt_glyph_data_for_next_glyph = *(eblc_subtable_format1_glyph_data_offsets_array_pointer + subtable_glyph_index + 1);
//                         INVERT_BYTE_ORDER(offset_into_ebdt_glyph_data);
//                         INVERT_BYTE_ORDER(offset_into_ebdt_glyph_data_for_next_glyph);
// 
//                         //never used
//                         size_t glyph_ebdt_data_size = offset_into_ebdt_glyph_data_for_next_glyph - offset_into_ebdt_glyph_data;
// 
//                         // BUG
//                         //glyph_ebdt_data_pointer += offset_into_ebdt_glyph_data;
//                         uint8 * glyph_ebdt_data_pointer = ebdt_table_pointer + cur_index_subtable_header.offset_to_subtable_ebdt_data + offset_into_ebdt_glyph_data;
// 
//                         if (cur_index_subtable_header.image_format == 2)
//                         {
//                             cur_glyph_metrics = *((SmallGlyphMetrics * ) glyph_ebdt_data_pointer);
// 
//                             INVERT_BYTE_ORDER(cur_glyph_metrics.height);
//                             INVERT_BYTE_ORDER(cur_glyph_metrics.width);
//                             INVERT_BYTE_ORDER(cur_glyph_metrics.bearing_x);
//                             INVERT_BYTE_ORDER(cur_glyph_metrics.bearing_y);
//                             INVERT_BYTE_ORDER(cur_glyph_metrics.advance);
// 
//                             Bitmap * glyph_bitmap = (Bitmap *) GetMemoryFromArena(sizeof(Bitmap), memory_arena);
//                             glyph_bitmap->width = cur_glyph_metrics.width;
//                             glyph_bitmap->height = cur_glyph_metrics.height;
// 
//                             // current bitmap renderer assumes 32 bit per pixelformat
//                             uint32 bits_per_pixel = 32;
//                             glyph_bitmap->bits = (uint32 *) GetMemoryFromArena(bits_per_pixel * glyph_bitmap->width * glyph_bitmap->height, memory_arena);
// 
//                             // TODO: stopped on trying to output the results of parsing. the double indirection why???
//                             // anyway it doesnt work. the Bitmap * of the glyphs dont get saved into the memory pointed to by parse_font_file_result.glyphs_bitmaps_pointers_array  
//                             // 
//                             *(parse_font_file_result.glyphs_bitmaps_pointers_array + parse_font_file_result.number_of_glyphs++) = glyph_bitmap;
// 
//                             // row_index assumes top-down view of the source bitmap data embedded into font,
//                            //  column_index - left-to-right
// 
//                             uint8 * glyph_image_data_pointer = glyph_ebdt_data_pointer + sizeof(SmallGlyphMetrics);
//                             uint32 * dest_pixel_pointer = glyph_bitmap->bits;
//                             for (uint32 row_index = 0; row_index <  glyph_bitmap->height; ++row_index)
//                             {
//                                 for (uint32 column_index = 0; column_index < glyph_bitmap->width; ++column_index)
//                                 {
//                                     uint8 flat_index_of_source_byte = (row_index * (glyph_bitmap->width) + column_index) / 8;
//                                     uint8 source_byte = (*glyph_image_data_pointer + flat_index_of_source_byte);
//                                     //INVERT_BYTE_ORDER(source_byte);
//                                     uint8 wanted_bit_extracted = (source_byte >> (7 - (flat_index_of_source_byte % 8))) & 0x01;
//                                     if (wanted_bit_extracted == 0x01)
//                                     {
//                                         *dest_pixel_pointer = 0xFFFFFFFF;
//                                         //*dest_pixel_pointer = 0x00000000;
//                                     } else if (wanted_bit_extracted == 0x00)
//                                     {
//                                         //*dest_pixel_pointer = 0xFFFFFFFF;
//                                         *dest_pixel_pointer = 0x00000000;
//                                     } else
//                                     {
//                                         // unexpected
//                                         ASSERT(0);
//                                     }
//                                     ++dest_pixel_pointer;
//                                 }
//                             }
// 
//                             int a = 1;
//                         } else 
//                         {
//                             //not implemented
//                             //ASSERT(0);
//                         }
// 
//                     } else if (cur_index_subtable_header.index_format==2)
//                     {
//                         //glyph_ebdt_data_size = glyph_image_size;
//                         uint8 * glyph_ebdt_data_pointer = ebdt_table_pointer + cur_index_subtable_header.offset_to_subtable_ebdt_data + glyph_image_size * subtable_glyph_index;
// 
// 
//                         ////////////////////////////////// start copy paste from here
//                         if (cur_index_subtable_header.image_format == 5)
// 
//                         {
//                             Bitmap * glyph_bitmap = (Bitmap *) GetMemoryFromArena(sizeof(Bitmap), memory_arena);
//                             glyph_bitmap->width = cur_glyph_metrics.width;
//                             glyph_bitmap->height = cur_glyph_metrics.height;
// 
//                             // current bitmap renderer assumes 32 bit per pixelformat
//                             uint32 bits_per_pixel = 32;
//                             glyph_bitmap->bits = (uint32 *) GetMemoryFromArena(bits_per_pixel * glyph_bitmap->width * glyph_bitmap->height, memory_arena);
// 
//                             *(parse_font_file_result.glyphs_bitmaps_pointers_array + parse_font_file_result.number_of_glyphs++) = glyph_bitmap;
// 
//                             // row_index assumes top-down view of the source bitmap data embedded into font,
//                            //  column_index - left-to-right
// 
//                             uint8 * glyph_image_data_pointer = glyph_ebdt_data_pointer;
//                             uint32 * dest_pixel_pointer = glyph_bitmap->bits;
//                             for (uint32 row_index = 0; row_index <  glyph_bitmap->height; ++row_index)
//                             {
//                                 for (uint32 column_index = 0; column_index < glyph_bitmap->width; ++column_index)
//                                 {
//                                     uint8 flat_index_of_source_byte = (row_index * (glyph_bitmap->width) + column_index) / 8;
//                                     uint8 source_byte = *(glyph_image_data_pointer + flat_index_of_source_byte);
//                                     //INVERT_BYTE_ORDER(source_byte);
//                                     uint8 wanted_bit_extracted = (source_byte >> (7 - (flat_index_of_source_byte % 8))) & 0x01;
//                                     if (wanted_bit_extracted == 0x01)
//                                     {
//                                         *dest_pixel_pointer = 0x00000000;
//                                     } else if (wanted_bit_extracted == 0x00)
//                                     {
//                                         *dest_pixel_pointer = 0xFFFFFFFF;
//                                     } else
//                                     {
//                                         // unexpected
//                                         ASSERT(0);
//                                     }
//                                     ++dest_pixel_pointer;
//                                 }
//                             }
//                         }
//                         //////////////////////////// end copy paste
// 
// }

// TODO: i could implement verification of fonts' checksums in a different function
// legal values for sfnt_version are: 0x00010000 for TrueType outlines, 'ttcf' - for ;'otto' - for Compact Font Format (CFF) data
// currently supportin only the first one
internal Glyphs
ParseFontFile(FileReadResult font_file, MemoryArena * memory_arena)
{
    Glyphs glyphs = {};
    TTCHeader ttc_header = *( (TTCHeader *) font_file.start_pointer);
    INVERT_BYTE_ORDER(ttc_header.tag);
    INVERT_BYTE_ORDER(ttc_header.major_version);
    INVERT_BYTE_ORDER(ttc_header.minor_version);
    INVERT_BYTE_ORDER(ttc_header.num_fonts);

    for (size_t font_index=0; font_index<ttc_header.num_fonts; ++font_index)
    {
        uint32 cur_font_table_directory_offset = *((uint32 *) (font_file.start_pointer + sizeof(TTCHeader) + sizeof(uint32) * font_index));
        INVERT_BYTE_ORDER(cur_font_table_directory_offset);
        uint8 * cur_font_table_directory_pointer = font_file.start_pointer + cur_font_table_directory_offset;


        uint32 first_4bytes = *((uint32 *) cur_font_table_directory_pointer);
        INVERT_BYTE_ORDER(first_4bytes);
        uint32 k_true_type_outlines_sfnt_version = 0x00010000;
        ASSERT(first_4bytes == k_true_type_outlines_sfnt_version);

        TableDirectory table_directory = *((TableDirectory *) cur_font_table_directory_pointer);
        INVERT_BYTE_ORDER(table_directory.sfnt_version);
        INVERT_BYTE_ORDER(table_directory.num_tables);
        INVERT_BYTE_ORDER(table_directory.search_range);
        INVERT_BYTE_ORDER(table_directory.entry_selector);
        INVERT_BYTE_ORDER(table_directory.range_shift);

        uint8 * cmap_table_pointer = 0;
        uint8 * head_table_pointer = 0;
        uint8 * eblc_table_pointer = 0;
        uint8 * ebdt_table_pointer = 0;
        
        for (int table_index=0; table_index<table_directory.num_tables; ++table_index)
        {

            TableRecord * cur_table_record_pointer = (TableRecord *) (cur_font_table_directory_pointer + sizeof(TableDirectory) + table_index * sizeof(TableRecord));
            TableRecord cur_table_record = *cur_table_record_pointer;
            INVERT_BYTE_ORDER(cur_table_record.checksum);
            INVERT_BYTE_ORDER(cur_table_record.offset);
            INVERT_BYTE_ORDER(cur_table_record.length);

            //uint8 * table_pointer = cur_font_table_directory_pointer + cur_table_record.offset;
            uint8 * table_pointer = font_file.start_pointer + cur_table_record.offset;
            uint32 checksum_calculated = CalculateChecksum(table_pointer, cur_table_record.length);

            if (StringsPrefixesEqual(cur_table_record.table_tag, "cmap", 4))
            {
                if ( ! cmap_table_pointer)
                {
                    cmap_table_pointer = table_pointer;
                } else
                {
                    // multiple cmap tables in the font
                    ASSERT(0);
                }
            } else if (StringsPrefixesEqual(cur_table_record.table_tag, "head", 4))
            {
                if ( ! head_table_pointer)
                {
                    head_table_pointer = table_pointer;
                } else
                {
                    // multiple cmap tables in the font
                    ASSERT(0);
                }
            } else if (StringsPrefixesEqual(cur_table_record.table_tag, "EBDT", 4))
            {
                if ( ! ebdt_table_pointer)
                {
                    ebdt_table_pointer = table_pointer;
                } else
                {
                    // multiple cmap tables in the font
                    ASSERT(0);
                }
            } else if (StringsPrefixesEqual(cur_table_record.table_tag, "EBLC", 4))
            {
                if ( ! eblc_table_pointer)
                {
                    eblc_table_pointer = table_pointer;
                } else
                {
                    // multiple cmap tables in the font
                    ASSERT(0);
                }
            }
        }

        if ( cmap_table_pointer == 0 || head_table_pointer == 0 || eblc_table_pointer == 0 || ebdt_table_pointer == 0) {
            ASSERT(0);
        }

        UnicodeRange unicode_range;
        GlyphsIds ascii_glyphs_ids = GetGlyphIdsFromCmapTable(cmap_table_pointer, unicode_range, memory_arena);
        glyphs = ExtractGlyphsFromEBDTUsingEBLCTable(ascii_glyphs_ids, eblc_table_pointer, ebdt_table_pointer, memory_arena);
        break; // looking only in the first font
    }


    //uint32 entire_font_checksum = CalculateChecksum(font_file.start_pointer, font_file.file_size); 
    //ASSERT(entire_font_checksum == 0xB1B0AFBA);
    return glyphs;
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

        FileReadResult man_bmp_file = platform_procedures->ReadFileIntoMemory("D:\\c_proj\\assets\\develop\\man\\man2.bmp");
        g_GlobalState->man_bitmap = ReadLoadedBmp(man_bmp_file, &game_memory->dynamic_storage_arena);
        platform_procedures->FreeFileReadResultFromMemory(man_bmp_file);

        FileReadResult msgothic_font_file = platform_procedures->ReadFileIntoMemory("D:\\c_proj\\assets\\msgothic.ttc");
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
                //uint8 b = 0;
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
    RenderFontPreview(bitmap_output_buffer);
    //RenderBitmap2(g_GlobalState->man_bitmap, bitmap_output_buffer, 0.0f, 0.0f);
}

} // extern "C"
