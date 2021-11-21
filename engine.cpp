
struct GameMemory
{
    uint8 * permanent_storage;
    uint32 permanent_storage_size;
} g_GameMemory;

struct TileMap
{
    real32 tile_length;
    uint32 num_tiles_x;
    uint32 num_tiles_y;
    //uint32 * tiles;
    uint32 tiles[5][10];
};

struct global_state
{
    real32 hero_x;
    real32 hero_y;

    real32 camera_center_x;
    real32 camera_center_y;

    TileMap tile_map;

    real64 game_time_ms;
} * g_GlobalState;

struct BitmapOutputBuffer
{
    uint32 * bits;
    uint32 width;
    uint32 height;

    // offset in game units of measurement of the center of the game screen
    // from the origin (0, 0)
    //real32 center_of_bitmap_x_m;
    //real32 center_of_bitmap_y_m;

    int32 shift_to_center_of_bitmap_x_px;
    int32 shift_to_center_of_bitmap_y_px;

    real32 px_in_m;
};

struct ControlInput
{
};

// TODO: handle correctly negative range rounding 
internal inline int32
RoundReal32ToInt32(real32 val)
{
    if (val < 0) {
        return (int32) (val - 0.5f);
    } else 
    {
        return (int32) (val + 0.5f);
    }
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
    real32 px_in_m = bitmap_output_buffer->px_in_m;
    int32 shift_to_center_of_bitmap_x_px = bitmap_output_buffer->shift_to_center_of_bitmap_x_px;
    int32 shift_to_center_of_bitmap_y_px = bitmap_output_buffer->shift_to_center_of_bitmap_y_px;
    int32 min_x = RoundReal32ToInt32(min_x_real * px_in_m) + shift_to_center_of_bitmap_x_px;
    int32 min_y = RoundReal32ToInt32(min_y_real * px_in_m) + shift_to_center_of_bitmap_y_px;
    int32 max_x = RoundReal32ToInt32(max_x_real * px_in_m) + shift_to_center_of_bitmap_x_px;
    int32 max_y = RoundReal32ToInt32(max_y_real * px_in_m) + shift_to_center_of_bitmap_y_px;

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
    real32 hero_width = 0.5f;
    real32 hero_height = 1.5f;

    real32 camera_center_x = g_GlobalState->camera_center_x;
    real32 camera_center_y = g_GlobalState->camera_center_y;

    real32 hero_min_x = g_GlobalState->hero_x - 0.5 * hero_width - camera_center_x;
    real32 hero_max_x = hero_min_x + hero_width;
    real32 hero_min_y = g_GlobalState->hero_y - 0.5 * hero_height - camera_center_y;
    real32 hero_max_y = hero_min_y + hero_height;
    uint32 hero_color = 0x00FFFFFF;
    DrawRectangle(bitmap_output_buffer, hero_min_x, hero_min_y,
                  hero_max_x, hero_max_y, hero_color);
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
            
            real32 tile_length = tile_map->tile_length;

            real32 camera_center_x = g_GlobalState->camera_center_x;
            real32 camera_center_y = g_GlobalState->camera_center_y;

            real32 min_x = (tile_column * tile_length) - camera_center_x;
            real32 max_x = (min_x + tile_length);
            real32 min_y = (tile_row * tile_length) - camera_center_y;
            real32 max_y = (min_y + tile_length);

            color = tile_map->tiles[tile_row][tile_column];
            DrawRectangle(bitmap_output_buffer, min_x,  min_y, max_x, max_y, color);
        }
    }
}

internal void
ProcessControlInput(ControlInput * control_input)
{
    real32 delta = 0.1f;
    return;
}

internal void 
UpdateStateAndRender(BitmapOutputBuffer * bitmap_output_buffer, 
                     ControlInput * control_input,
                     real64 time_delta_ms)
{
    local_persist bool32 initialized = false;
    if (!initialized)
    {
        g_GlobalState = (global_state *) g_GameMemory.permanent_storage;
        g_GlobalState->hero_x = 1.0f;
        g_GlobalState->hero_y = 1.0f;

        g_GlobalState->camera_center_x = 5.0f;
        g_GlobalState->camera_center_y = 2.5f;

        g_GlobalState->game_time_ms = 0.0f;

        TileMap tile_map;

        tile_map.num_tiles_x = 10;
        tile_map.num_tiles_y = 5;
        tile_map.tile_length = 1.0f;

        uint32 color = 0;
        for (int tile_row = 0; tile_row < tile_map.num_tiles_y; ++tile_row)
        {
            for (int tile_column = 0; tile_column < tile_map.num_tiles_x; ++tile_column)
            {
                
                uint8 r = (tile_row * (256 / tile_map.num_tiles_y)) % 256;
                uint8 g = (tile_column * (256 / tile_map.num_tiles_x)) % 256;
                uint8 b = 0;

                color = ((((uint32)r) << 16) | (((BYTE)g)<< 8) | b);
                tile_map.tiles[tile_row][tile_column] = color;
            }
        }

        g_GlobalState->tile_map = tile_map;

        initialized = true;
    }

    g_GlobalState->game_time_ms += time_delta_ms;

    ProcessControlInput(control_input);

    RenderTileMap(bitmap_output_buffer);
    RenderHero(bitmap_output_buffer);
}


