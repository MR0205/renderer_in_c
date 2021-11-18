// TODO: add padding from what we draw
// think through the dimensions of tilemap of tiles of buffer etc (in px and may be in m)

struct global_state
{
    uint32 offset_x;
    uint32 offset_y;

} g_GlobalState;

struct BitmapOutputBuffer
{
    uint32 * bits;
    uint32 width;
    uint32 height;
};

struct TileMap
{
    uint32 tile_width_m;
    uint32 tile_width_px;

    //float32 px_in_m;

    uint32 num_tiles_x;
    uint32 num_tiles_y;
    uint32 * tiles;
};

static void
DrawRectangle(BitmapOutputBuffer * bitmap_output_buffer,
              int32 min_x, int32 min_y, int32 max_x, int32 max_y,
              uint8 r, uint8 g, uint8 b)
{
    if (min_x < 0) 
    {
        min_x = 0;
    }

    if (min_y < 0)
    {
        min_y = 0;
    }

    if (max_x > bitmap_output_buffer->width)
    {
        max_x = bitmap_output_buffer->width;
    }

    if (max_y > bitmap_output_buffer->height)
    {
        max_y = bitmap_output_buffer->height;
    }

    uint32 color = 0;
    uint32 * cur_row_start = bitmap_output_buffer->bits + 
                             min_y * bitmap_output_buffer->width;
    uint32 * cur_pixel; 
    for (int index_row = min_y; index_row < max_y; ++index_row)
    {
        cur_pixel = cur_row_start + min_x;
        for (int index_column = min_x; index_column < max_x; ++index_column)
        {
            color = ((((uint32)r) << 16) | (((BYTE)g)<< 8) | b);
            *(cur_pixel++) = color;
        }
        cur_row_start += bitmap_output_buffer->width;
    }
}

static void
RenderTileMap(TileMap * tile_map, BitmapOutputBuffer * bitmap_output_buffer)
{
    uint32 color = 0;
    for (int tile_row = 0; tile_row < tile_map->num_tiles_y; ++tile_row)
    {
        for (int tile_column = 0; tile_column < tile_map->num_tiles_x; ++tile_column)
        {
            
            uint8 r = (tile_row * (256 / tile_map->num_tiles_y)) % 256;
            uint8 g = (tile_column * (256 / tile_map->num_tiles_x)) % 256;
            uint8 b = 0;
            int min_x = tile_column * tile_map->tile_width_px;
            int max_x = min_x + tile_map->tile_width_px;
            int min_y = tile_row * tile_map->tile_width_px;
            int max_y = min_y + tile_map->tile_width_px;
            color = ((((uint32)r) << 16) | (((BYTE)g)<< 8) | b);
            DrawRectangle(bitmap_output_buffer, min_x,  min_y, max_x, max_y,
                          r, g, b);
        }
    }
}

static void 
UpdateStateAndRender(BitmapOutputBuffer * bitmap_output_buffer)
{
    uint32 tiles[10][5] = 
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

    TileMap tile_map;
    tile_map.num_tiles_x = 10;
    tile_map.num_tiles_y = 5;
    tile_map.tile_width_px = 100;
    tile_map.tiles = (uint32 *) tiles;

    RenderTileMap(&tile_map, bitmap_output_buffer);
    //DrawRectangle(bitmap_output_buffer, -100, -100, 200, 200,//200, 200, 400, 400, 
    //              0xFF, 0x0, 0x0);
    //int min_x = bitmap_output_buffer->width - 50;
    //int max_x = bitmap_output_buffer->width + 50;
    //int min_y = bitmap_output_buffer->height - 50;
    //int max_y = bitmap_output_buffer->height + 50;
    //DrawRectangle(bitmap_output_buffer, min_x, min_y, max_x, max_y,
    //              0x0, 0xFF, 0x0);
}


