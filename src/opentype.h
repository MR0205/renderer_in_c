#if !defined( OPENTYPE_H )

#include "macro_definitions.h"
#include "platform.h"
#include "bitmap.h"

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
    uint32 bearing_x;
    uint32 bearing_y;
    uint32 advance_x;
};

// I have tested these structures of OpenType don't require specification of non-standard alignment, meaning on x64 the were automatically well aligned.
// But by precaution I specify them as packed
#pragma pack(push, 1)
struct TTCHeader
{
    char tag[4];
    uint16 major_version;
    uint16 minor_version;
    uint32 num_fonts;
};

struct TableRecord
{
    char table_tag[4];
    uint32 checksum;
    uint32 offset;
    uint32 length;
};


struct TableDirectory
{
    uint32 sfnt_version;
    uint16 num_tables;
    uint16 search_range;
    uint16 entry_selector;
    uint16 range_shift;
    //TableRecord * table_records;
};

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

struct EBDTTableHeader
{
    uint16 major_version;
    uint16 minor_version;
};

struct EBLCTableHeader
{
    uint16 major_version;
    uint16 minor_version;
    uint32 num_sizes;
};

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

struct IndexSubTableArrayEntry
{
    uint16 first_glyph_index;
    uint16 last_glyph_index;
    uint32 additional_offset_to_index_subtable;
};

struct IndexSubTableHeader
{
    uint16 index_format;
    uint16 image_format;
    uint32 offset_to_subtable_ebdt_data;
};

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

struct SmallGlyphMetrics
{
    uint8 height;
    uint8 width;
    int8 bearing_x;
    int8 bearing_y;
    uint8 advance;
};

struct CmapTableHeader
{
    uint16 version;
    uint16 num_encoding_tables;
};

struct EncodingRecord
{
    uint16 platform_id;
    uint16 encoding_id;
    uint32 subtable_offset;
};

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

struct GlyphsIds
{
    uint16 * ids;
    UnicodeRange unicode_range;
};

#pragma pack(pop)

Glyphs
ParseFontFile(FileReadResult font_file, MemoryArena * memory_arena);

#define OPENTYPE_H
#endif
