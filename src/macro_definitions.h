#if !defined( MACRO_DEFINITIONS_H )




#define ASSERT(expression) if(!(expression)) {*((int *)0)=0;}

#define int8 char
#define int16 short
#define int32 int
#define int64 long long

#define uint8 unsigned char
#define uint16 unsigned short
#define uint32 unsigned int
#define uint64 unsigned long long

#define real32 float
#define real64 double

#define bool32 int

#define local_persist static
#define internal static

#define KILOBYTE (1024)
#define MEGABYTE (1024 * KILOBYTE)
#define GIGABYTE (1024 * MEGABYTE)


internal void
InvertByteOrder(uint8 * variable_pointer, size_t size_of_variable)
{
    uint8 temp;
    uint8 * forward_pointer = variable_pointer;
    uint8 * backward_pointer = forward_pointer + size_of_variable - 1;
    for (size_t i=0; i < (size_of_variable / 2); ++i)
    {
        temp = *forward_pointer;
        *forward_pointer = *backward_pointer;
        *backward_pointer = temp;
        ++forward_pointer;
        --backward_pointer;
    }
}

#define INVERT_BYTE_ORDER(var) InvertByteOrder((uint8 *) &var, sizeof(var))

#define MACRO_DEFINITIONS_H
#endif
