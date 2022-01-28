#if !defined(TILEMAP_H)

// struct WorldCoord
// {
//     v2ui on_tilemap;
//     v2 in_tile;
// };

// struct Tile
// {
//     bool32 walkable;
//     uint32 color;
// };

enum EntityType
{
    no_entity_type,
    hero_entity_type,
    rectangle_entity_type,
    last_enity_type,
};

struct Entity
{
    v2 pos;
    real32 speed;
    v2 target_pos;

    v2 dimensions;
    EntityType entity_type;
    uint32 color;
};

struct TileMap
{
    //real32 tile_length;
    //v2ui num_tiles;
    //Tile * tiles;
    uint32 num_entities;
    Entity * entities;
    //uint32 tiles[30][30];
};


#define TILEMAP_H
#endif
