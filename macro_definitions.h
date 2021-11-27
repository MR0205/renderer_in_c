#if !defined( MACRO_DEFINITIONS_H )
#define MACRO_DEFINITIONS_H




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





#endif
