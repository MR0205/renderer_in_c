#if !defined(TILEMAP_H)

struct WorldCoord
{
    v2ui on_tilemap;
    v2 in_tile;
};

struct Tile
{
    bool32 walkable;
    uint32 color;
};

struct TileMap
{
    real32 tile_length;
    v2ui num_tiles;
    Tile * tiles;
    //uint32 tiles[30][30];
};


#define TILEMAP_H
#endif
