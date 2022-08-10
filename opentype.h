#if !defined( OPENTYPE_H )

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

// struct Text
// {
//     uint16 * unicode_values;
//     size_t char_number;
// };


/// However as I have tested these structures of OpenType don't require specification of non-standard alignment, meaning on x64 the were automatically well aligned.
// But by precaution I specify them as packed nevertheless.
//#pragma pack(1) // doesn't save a value in a stack, what we want is to restore the default value (16 bytes for x64) after we finished with packed staructures
//#pragma pack(show)
#pragma pack(push, 1)
struct TTCHeader
{
    char tag[4];
    uint16 major_version;
    uint16 minor_version;
    uint32 num_fonts;
};

//#pragma pack(1)
struct TableRecord
{
    char table_tag[4];
    uint32 checksum;
    uint32 offset;
    uint32 length;
};


//#pragma pack(1)
struct TableDirectory
{
    uint32 sfnt_version;
    uint16 num_tables;
    uint16 search_range;
    uint16 entry_selector;
    uint16 range_shift;
    //TableRecord * table_records;
};

//#pragma pack(1)
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

//#pragma pack(1)
struct EBDTTableHeader
{
    uint16 major_version;
    uint16 minor_version;
};

//#pragma pack(1)
struct EBLCTableHeader
{
    uint16 major_version;
    uint16 minor_version;
    uint32 num_sizes;
};

//#pragma pack(1)
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

//#pragma pack(1)
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

//#pragma pack(1)
struct IndexSubTableArrayEntry
{
    uint16 first_glyph_index;
    uint16 last_glyph_index;
    uint32 additional_offset_to_index_subtable;
};

//#pragma pack(1)
struct IndexSubTableHeader
{
    uint16 index_format;
    uint16 image_format;
    uint32 offset_to_subtable_ebdt_data;
};

//#pragma pack(1)
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

//#pragma pack(1)
struct SmallGlyphMetrics
{
    uint8 height;
    uint8 width;
    int8 bearing_x;
    int8 bearing_y;
    uint8 advance;
};

//#pragma pack(1)
struct CmapTableHeader
{
    uint16 version;
    uint16 num_encoding_tables;
};

//#pragma pack(1)
struct EncodingRecord
{
    uint16 platform_id;
    uint16 encoding_id;
    uint32 subtable_offset;
};

//#pragma pack(1)
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

#pragma pack(pop)





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


            //SmallGlyphMetrics cur_glyph_metrics;
            uint32 glyph_bitmap_width;
            uint32 glyph_bitmap_height;
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

                // cur_glyph_metrics.height = big_glyph_metrics.height;
                // cur_glyph_metrics.width = big_glyph_metrics.width;
                // cur_glyph_metrics.bearing_x = big_glyph_metrics.hori_bearing_x;
                // cur_glyph_metrics.bearing_y = big_glyph_metrics.hori_bearing_y;
                // cur_glyph_metrics.advance = big_glyph_metrics.hori_advance;

                glyphs.bearing_x = big_glyph_metrics.hori_bearing_x;
                glyphs.bearing_y = big_glyph_metrics.hori_bearing_y;
                glyphs.advance_x = big_glyph_metrics.hori_advance;

                glyph_bitmap_width = big_glyph_metrics.width;
                glyph_bitmap_height = big_glyph_metrics.height;

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
                    glyph_bitmap->width = glyph_bitmap_width;
                    glyph_bitmap->height = glyph_bitmap_height;

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
                                //*dest_pixel_pointer = 0x00000000;
                                *dest_pixel_pointer = 0xFFFFFFFF;
                            } else if (wanted_bit_extracted == 0x00)
                            {
                                *dest_pixel_pointer = 0x00FFFFFF;
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

#define OPENTYPE_H
#endif
