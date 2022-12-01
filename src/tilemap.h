#if !defined(TILEMAP_H)

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
    uint32 num_entities;
    Entity * entities;
};


#define TILEMAP_H
#endif
