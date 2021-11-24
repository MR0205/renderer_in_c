#if !defined(TILEMAP_H)
#define TILEMAP_H



struct TileMap
{
    real32 tile_length;
    uint32 num_tiles_x;
    uint32 num_tiles_y;
    //uint32 * tiles;
    uint32 tiles[5][10];
};

#endif
