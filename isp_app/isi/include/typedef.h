#ifndef __TYPEDEF_H__
#define __TYPEDEF_H__

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef char s8;
typedef short s16;
typedef int s32;

/* Define integer types for 8/16 bit registers */
#ifndef BYTE
typedef unsigned char BYTE;
#endif
#ifndef WORD
typedef unsigned short WORD;
#endif
#ifndef DWORD
typedef unsigned int DWORD;
#endif

/* Define standard constants */
#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef NULL
#define NULL  0
#endif

#ifndef bool
//typedef unsigned char bool;
#endif

#ifndef true
#define true (1)
#endif

#ifndef false
#define false (0)
#endif

/* Register read/write macros */
#define READ_BYTE(addr)        (*((volatile BYTE *)(addr)))
#define READ_WORD(addr)        (*((volatile WORD *)(addr)))
#define READ_DWORD(addr)       (*((volatile unsigned int *)(addr)))
#define WRITE_BYTE(addr,data)  (*((volatile BYTE *)(addr)) = (data))
#define WRITE_WORD(addr,data)  (*((volatile WORD *)(addr)) = (data))
#define WRITE_DWORD(addr,data) (*((volatile unsigned int *)(addr))  = (data))

#endif

