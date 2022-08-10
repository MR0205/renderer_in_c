#if !defined( BMP_PARSER_H )

//#include "macro_definitions.h"
//#include "platform.h"
//#include "bitmap.h"

#pragma pack(push, 1)
//#pragma pack(1)
struct BmpFileHeader
{
    char file_type[2];
    uint32 file_size;
    uint16 empty_space0;
    uint16 empty_space1;
    uint32 offset_to_bits;
};

//#pragma pack(1)
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


//#pragma pack(1)
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
#pragma pack(pop)

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

#define BMP_PARSER_H
#endif
