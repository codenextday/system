#ifndef __COMDEF_H__
#define __COMDEF_H__

// type define
//
typedef  unsigned char	UINT8;
typedef unsigned short  UINT16;
typedef unsigned long	UINT32;
typedef long			INT32;
typedef short			INT16;
typedef char			INT8;
typedef unsigned int uint32_t;

//#define NULL 0

#define BIT_0						0x00000001
#define BIT_1						0x00000002
#define BIT_2						0x00000004
#define BIT_3						0x00000008
#define BIT_4						0x00000010
#define BIT_5						0x00000020
#define BIT_6						0x00000040
#define BIT_7						0x00000080
#define BIT_8						0x00000100
#define BIT_9						0x00000200
#define BIT_10						0x00000400
#define BIT_11						0x00000800
#define BIT_12						0x00001000
#define BIT_13						0x00002000
#define BIT_14						0x00004000
#define BIT_15						0x00008000
#define BIT_16						0x00010000
#define BIT_17						0x00020000
#define BIT_18						0x00040000
#define BIT_19						0x00080000
#define BIT_20						0x00100000
#define BIT_21						0x00200000
#define BIT_22						0x00400000
#define BIT_23						0x00800000
#define BIT_24						0x01000000
#define BIT_25						0x02000000
#define BIT_26						0x04000000
#define BIT_27						0x08000000
#define BIT_28						0x10000000
#define BIT_29						0x20000000
#define BIT_30						0x40000000
#define BIT_31						0x80000000

//Macro define
//

#define inpb(port)        (*((volatile UINT8 *) (port)))
#define inpw(port)       (*((volatile UINT32 *) (port)))
#define inph(port)       (*((volatile UINT16 *) (port)))

#define outpb(port, val)  (*((volatile UINT8 *) (port)) = ((UINT8) (val)))
#define outpw(port, val) (*((volatile UINT32 *) (port)) = ((UINT32) (val)))
#define outph(port, val) (*((volatile UINT16 *) (port)) = ((UINT16) (val)))


#define MA_INB( io )  (UINT8) inpb( io )
#define MA_INW( io )  (UINT32) inpw( io )
#define MA_INH( io )  (UINT16) inph( io )
#define MA_INH1( io )  (UINT16) inph( io<<1 )

#define MA_INBM( io, mask ) ( inpb( io ) & (mask) )
#define MA_INWM( io, mask ) ( inpw( io ) & (mask) )
#define MA_INHM( io, mask ) ( inph( io ) & (mask) )

//#define MA_OUTB( io, val )  (void) outpb( io, (int) val)
//#define MA_OUTW( io, val )  (void) outpw( io, (int) val)
//#define MA_OUTH( io, val )  (void) outph( io, (int) val)
//#define MA_OUTH1( io, val )  (void) outph( io<<1, (int) val)

#define MA_OUTWM( io, mask, val) \
{\
	UINT32 temp;\
	(temp) =(((MA_INW(io) & (UINT32)(~(mask))) |((UINT32)((val) & (mask)))));\
	(void) outpw( io, (UINT32)(temp));\
}\

#define MA_OUTHM( io, mask, val) \
{\
	DWORD temp;\
	(temp) =(((MA_IND(io) & (UINT16)(~(mask))) |((UINT16)((val) & (mask)))));\
	(void) outph( io, (UINT16)(temp));\
}

#define SETREG32(reg,val) ( (*(volatile unsigned int *) (reg)) = ((unsigned int) (val)) )
#define GETREG32(reg)     ( *((volatile unsigned int *) (reg)) )


#endif//__COMDEF_H__

