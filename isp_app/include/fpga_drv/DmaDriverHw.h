/**
 * \file DmaDriverHw.h - Contains the Hardware specific defines,
 *  	 typedefs, structures.
 *
 * Copyright (c) 2008-2009 Northwest Logic
 *
 * This is the base file for the PCI Express DMA Driver.
 *
 * 	Written by Pro Code Works, LLC.  www.ProCodeWorks.com for Northwest Logic
 * 
 * There maybe some vestigial code in this file that was originally authored by
 *   TriplePoint, hence the follwing attribution:
 * Copyright (c) 2002-2007 TriplePoint Inc. -- http://www.triplepoint.com
 * All rights reserved.
 *
 * This source code may be used by anyone provided the TriplePoint copyright
 * is maintained in any derivative works.
 * 
 */

#ifndef _DMA_DRIVER_HW_H_
#define _DMA_DRIVER_HW_H_

#pragma pack(push,1)

#ifndef WIN32   // Linux version ---------------------------------------------

#ifndef PACKED
#    define PACKED                      __attribute__((packed))
#endif /* PACKED */

#else // Windows
#ifndef PACKED
#    define PACKED 
#endif /* PACKED */

// DMA Defines
#define MAX_NUM_DMA_ENGINES		64
#endif // Windows vs. Linux

#define DMA_BAR					0

#define	MSIX_MESSAGE_ID			0x11
#define	TABLE_BIR_MASK			0x07

// DMA Descriptor Defines
//
// DMA descriptors must be on a 32 byte boundary
#define DMA_NUM_DESCR           128
#define DMA_DESCR_MAX_LEN       (256 * 1024 * 1024)
#define DMA_DESCR_ALIGN_LEN     32
#define DMA_DESCR_ALIGN_REQUIREMENT	(FILE_32_BYTE_ALIGNMENT)
// Maximum number of bytes for number of descriptors (-1 for alignment)
#define DMA_MAX_TRANSFER_LENGTH ((DMA_NUM_DESCR-2) * 4 * 1024)
#define DMA_MAX_TRANSACTION_LENGTH (256 * 1024 * 1024)

// The watchdog interval.  Any request pending for this number of seconds is
// cancelled.  This is a one second granularity timer, so the timeout must be
// at least one second more than the minimum allowed.
#define CARD_WATCHDOG_INTERVAL   5

#ifndef _WIN32	// Linux Version

#define DMA_DESCR_ALIGN_VIRT_ADDR(x) ((PDMA_DESCRIPTOR_STRUCT)((((unsigned long)(x)) + (DMA_DESCR_ALIGN_LEN - 1)) & ~(DMA_DESCR_ALIGN_LEN - 1)))
#define DMA_DESCR_ALIGN_PHYS_ADDR(x) (((x) + (DMA_DESCR_ALIGN_LEN - 1)) & ~(DMA_DESCR_ALIGN_LEN - 1))

/*!
*******************************************************************************
*/
typedef struct _DMA_TRANSACTION_STRUCT
{
    struct list_head        list;

    size_t                  length;
	uint64_t		       	CardOffset;
    int                     offset;
    int                     numPages;
    struct page **          pages;
    int                     direction;
	int						status; 
	uint32_t				BytesTransfered;

	uint16_t				DMAAvailable;
    uint16_t                DMAComplete;

	wait_queue_head_t       waiting;
	wait_queue_head_t       completed;

	struct scatterlist *    pSgList;
    int                     SgNumElements;
} DMA_TRANSACTION_STRUCT, *PDMA_TRANSACTION_STRUCT;

// Interrupt status defines
#define IRQ_DMA_COMPLETE(x) 	(uint64_t) (0x00000001 << (x))

#endif // Linux Version


/*!
*******************************************************************************
** Board Register set.
*  DMA Common Control Structure
*/
typedef struct _DMA_COMMON_CONTROL_STRUCT
{
	ULONG32		ControlStatus;
	ULONG32		FpgaVersion;
} PACKED DMA_COMMON_CONTROL_STRUCT, *PDMA_COMMON_CONTROL_STRUCT;

//
// DMA Common ControlStatus defs
#define CARD_IRQ_ENABLE             (1 << 0)
#define CARD_IRQ_ACTIVE             (1 << 1)
#define CARD_IRQ_PENDING            (1 << 2)
#define CARD_IRQ_MSI                (1 << 3)
#define CARD_MAX_C2S_SIZE(x)        (1UL << ((((x) >> 8) & 0xFF)+7))
#define CARD_MAX_S2C_SIZE(x)        (1UL << ((((x) >> 12) & 0xFF)+7))


#define DMA_IS_BUSY(x) \
	((x)->ControlStatus & BLOCK_DMA_STAT_CHAIN_RUNNING)

//
// BLOCK_DMA_DESCRIPTOR Control defs
#define BLOCK_DESC_CTRL_IRQ_ON_COMPLETION    (1 << 0)
#define BLOCK_DESC_CTRL_IRQ_ON_SHORT_ERR     (1 << 1)
#define BLOCK_DESC_CTRL_IRQ_ON_SHORT_SW      (1 << 2)
#define BLOCK_DESC_CTRL_IRQ_ON_SHORT_HW      (1 << 3)
#define BLOCK_DESC_CTRL_SEQUENCE             (1 << 8)
#define BLOCK_DESC_CTRL_CONTINUE             (1 << 9)


#define COMMON_DMA_CTRL_IRQ_ENABLE         		0x00000001
#define COMMON_DMA_CTRL_IRQ_ACTIVE         		0x00000002

#define COMMON_USER_CTRL_IRQ_ENABLE         		0x00000010
#define COMMON_USER_CTRL_IRQ_ACTIVE         		0x00000020

#define BLOCK_DMA_CTRL_CHAIN_START        (1 << 8)
#define BLOCK_DMA_CTRL_CHAIN_STOP         (1 << 9)
#define BLOCK_DMA_STAT_CHAIN_RUNNING      (1 << 10)
#define BLOCK_DMA_STAT_CHAIN_COMPLETE     (1 << 11)
#define BLOCK_DMA_STAT_CHAIN_SHORT_ERR    (1 << 12)
#define BLOCK_DMA_STAT_CHAIN_SHORT_SW     (1 << 13)
#define BLOCK_DMA_STAT_CHAIN_SHORT_HW     (1 << 14)
#define BLOCK_DMA_STAT_DESC_ALIGN_ERR		(1 << 15)

#define PACKET_DMA_CTRL_DESC_COMPLETE			0x00000004
#define PACKET_DMA_CTRL_DESC_ALIGN_ERROR		0x00000008
#define PACKET_DMA_CTRL_DESC_FETCH_ERROR		0x00000010
#define PACKET_DMA_CTRL_DESC_SW_ABORT_ERROR		0x00000020
#define PACKET_DMA_CTRL_DESC_CHAIN_END			0x00000080
#define PACKET_DMA_CTRL_DMA_ENABLE				0x00000100
#define PACKET_DMA_CTRL_DMA_RUNNING				0x00000400
#define PACKET_DMA_CTRL_DMA_WAITING				0x00000800
#define PACKET_DMA_CTRL_DMA_WAITING_PERSIST 	0x00001000
#define PACKET_DMA_CTRL_DMA_RESET_REQUEST	 	0x00004000
#define PACKET_DMA_CTRL_DMA_RESET			 	0x00008000

#define	PACKET_DMA_INT_CTRL_IRQ_COMPLETE		0x00000000
#define PACKET_DMA_INT_CTRL_INT_EOP				0x00000002

//
// DMA Descriptor structure
typedef struct _BLOCK_DMA_DESCRIPTOR
{
	// Hardware specific entries - Do not change or reorder
    ULONG32         Control;
    ULONG32         ByteCount;
    ULONG64       	SystemAddress;
    ULONG64       	CardAddress;
    ULONG64         NextDescriptor;
	// Software specific entries
#ifndef _WIN32  /* Linux */
	PDMA_TRANSACTION_STRUCT	pDmaTrans;			// Pointer to the DMA Transaction for this buffer
#endif /* Linux */
} PACKED BLOCK_DMA_DESCRIPTOR, *PBLOCK_DMA_DESCRIPTOR;


#define PACKET_DESC_STATUS_MASK					0xFF000000
#define PACKET_DESC_COMPLETE_BYTE_COUNT_MASK	0x000FFFFF
#define PACKET_DESC_CONTROL_MASK				0xFF000000
#define PACKET_DESC_CARD_ADDRESS32_35_MASK		0x00F00000
#define PACKET_DESC_BYTE_COUNT_MASK				0x000FFFFF

#define PACKET_DESC_C2S_CTRL_IRQ_ON_COMPLETE	0x01000000;
#define PACKET_DESC_C2S_CTRL_IRQ_ON_ERROR		0x02000000

#define PACKET_DESC_C2S_STAT_START_OF_PACKET	0x80000000
#define PACKET_DESC_C2S_STAT_END_OF_PACKET		0x40000000
#define PACKET_DESC_C2S_STAT_ERROR				0x10000000
#define PACKET_DESC_C2S_STAT_USER_STAT_HI_0		0x08000000
#define PACKET_DESC_C2S_STAT_USER_STAT_LO_0		0x04000000
#define PACKET_DESC_C2S_STAT_SHORT				0x02000000
#define PACKET_DESC_C2S_STAT_COMPLETE			0x01000000
//
//  Packet Mdoe DMA Descriptor (C2S) Read
typedef struct _PACKET_DESCRIPTOR_C2S_STRUCT
{
	// Hardware specific entries - Do not change or reorder
	ULONG32		StatusFlags_BytesCompleted;		// C2S StatusFlags and Bytes count completed       
	ULONG64		UserStatus;                     // C2S User Status - Passed from Users design
	ULONG32		CardAddress;                    // C2S Card Address (offset) Not used in FIFO Mode 
	ULONG32		ControlFlags_ByteCount;         // C2S Control Flags and Bytes Count to transfer   
	ULONG64 	SystemAddressPhys;              // C2S System Physical Buffer Address to transfer  
	ULONG32		NextDescriptorPhys;             // C2S Physical Address to Next DMA Descriptor     
	// Software specific entries
#ifdef _WIN32  /* Windows */
	PSCATTER_GATHER_LIST pScatterGatherList;	// Pointer to the Scatter List for this set of descriptors
	PVOID *				SystemAddressVirt;		// User address for the SystemAddress
	PMDL				pMdl;					// Pointer to the MDL for this buffer.
#else /* Linux */
	PDMA_TRANSACTION_STRUCT	pDmaTrans;			// Pointer to the DMA Transaction for this buffer
//	struct scatterlist * pScatterGatherList;	// Pointer to the Scatter List for this set of descriptors
	UCHAR *				SystemAddressVirt;		// User address for the SystemAddress
#endif /* Windows vs Linux */
} PACKED PACKET_DESCRIPTOR_C2S_STRUCT, *PPACKET_DESCRIPTOR_C2S_STRUCT;


#define PACKET_DESC_S2C_CTRL_IRQ_ON_COMPLETE	0x01000000
#define PACKET_DESC_S2C_CTRL_IRQ_ON_ERROR		0x02000000
#define PACKET_DESC_S2C_CTRL_END_OF_PACKET		0x40000000
#define PACKET_DESC_S2C_CTRL_START_OF_PACKET	0x80000000

#define PACKET_DESC_S2C_STAT_COMPLETE			0x01000000
#define PACKET_DESC_S2C_STAT_SHORT				0x02000000
#define PACKET_DESC_S2C_STAT_ERROR				0x10000000
//
//  Packet Mdoe DMA Descriptor (S2C) Write
typedef struct _PACKET_DESCRIPTOR_S2C_STRUCT
{
	// Hardware specific entries - Do not change or reorder
	ULONG32		StatusFlags_BytesCompleted;		// S2C StatusFlags and Bytes count completed
	ULONG64		UserControl;					// S2C User Control - passed to Users design
	ULONG32		CardAddress;					// S2C Card Address (offset) Not used in FIFO Mode
	ULONG32		ControlFlags_ByteCount;			// S2C Control Flags and Bytes Count to transfer
	ULONG64 	SystemAddressPhys;				// S2C System Physical Buffer Address to transfer
	ULONG32		NextDescriptorPhys;				// S2C Physical Address to Next DMA Descriptor
	// Software specific entries
#ifdef _WIN32
	WDFDMATRANSACTION 		DmaTransaction;		// Contains the DMA Transaction associated to this descriptor
#else  /* Linux */
	PDMA_TRANSACTION_STRUCT pDmaTransaction;	// Pointer the DMA Transaction struct
#endif /* Windows vs. Linux */
} PACKED PACKET_DESCRIPTOR_S2C_STRUCT, *PPACKET_DESCRIPTOR_S2C_STRUCT;

#define	DESC_FLAGS_HW_OWNED						0x00000001
#define	DESC_FLAGS_SW_OWNED						0x00000002
#define	DESC_FLAGS_SW_FREED						0x00000004

typedef struct _PACKET_DMA_DESCRIPTOR_STRUCT
{
	union
	{
		PACKET_DESCRIPTOR_S2C_STRUCT	S2C;	// HW System To Card Descriptor components
		PACKET_DESCRIPTOR_C2S_STRUCT	C2S;	// HW Card To System Descriptor components
	}; // Direction;								// Direction specific Hardware descriptor
	ULONG32				DescriptorNumber;		// "Token" for this descriptor
	ULONG32				DescFlags;				// Flags for this decriptor
	PVOID *				pNextDescriptorVirt;	// Driver uable pointer to the next descriptor
#ifdef _WIN32 /* Windows */
	PHYSICAL_ADDRESS	pDescPhys;				// Physical address for this descriptor
#else /* Linux */
	ULONG32				pDescPhys;				// Physical address for this descriptor
#endif /* Windows vs. Linux */
} PACKED PACKET_DMA_DESCRIPTOR_STRUCT, *PPACKET_DMA_DESCRIPTOR_STRUCT;


//
// DMA Engine Control Structure
typedef struct _DMA_ENGINE_STRUCT
{
	ULONG32		Capabilities;  					// Common Capabilities
	volatile ULONG32 ControlStatus;				// Common Control and Status
	union
	{
		struct
		{
			ULONG64		Descriptor;				// Block Mode Descriptor pointer
			ULONG32		HardwareTime;			// Block Mode Hardware timer
			ULONG32		ChainCompleteByteCount;	// Block Mode Completed Byte Count
		} Block;
		struct
		{
			ULONG32		NextDescriptorPtr;		// Packet Mode Next Descriptor Pointer (Physical)
			ULONG32		SoftwareDescriptorPtr;	// Packet Mode Software Owned Descriptor pointer (Physical)
			ULONG32		CompletedDescriptorPtr;	// Packet Mode Completed Packet Descriptor Pointer (Physical)
			ULONG32		DMAActiveTime;			// Amount of time the DMA was active / second (4ns resolution)
			ULONG32		DMAWaitTime;			// Amount of time the DMA was inactive / second (4ns resolution)
			ULONG32		DMACompletedByteCount;	// Amount DMA byte transfers / second (4 byte resolution)
			ULONG32 	InterruptControl;		// New Firmware Interrupt Control register
		}Packet;
	};
	UCHAR		Reserved[0x100-(9*sizeof(ULONG32))];	// Reserved size
} PACKED DMA_ENGINE_STRUCT, *PDMA_ENGINE_STRUCT;

#ifdef _X86_
	#define	RESERVED_SIZE	64					// Assume a 32 bit machine
#else
	#define	RESERVED_SIZE	128					// 64 bit pointers increase the size beyond 64.
#endif

/*!
*******************************************************************************
** Descriptor engine register set.
*/

#   define      DMA_CTRL_IRQ_ENABLE                    (1 << 0)
#   define      DMA_CTRL_IRQ_ACTIVE                    (1 << 1)
#   define      DMA_CTRL_CHAIN_START                   (1 << 8)
#   define      DMA_CTRL_CHAIN_STOP                    (1 << 9)
#   define      DMA_STAT_CHAIN_RUNNING                 (1 << 10)
#   define      DMA_STAT_CHAIN_COMPLETE                (1 << 11)
#   define      DMA_STAT_CHAIN_SHORT_ERR               (1 << 12)
#   define      DMA_STAT_CHAIN_SHORT_SW                (1 << 13)
#   define      DMA_STAT_CHAIN_SHORT_HW                (1 << 14)

//
// DMA_DESCRIPTOR_MAX_SIZE is used only for a size reference when allocating space
//  for the descriptor pool
typedef struct _DMA_DESCRIPTOR_STRUCT
{
	union
	{
		PACKET_DMA_DESCRIPTOR_STRUCT	Packet;
		BLOCK_DMA_DESCRIPTOR			Block;
	};
	UCHAR	reserved[RESERVED_SIZE - sizeof(PACKET_DMA_DESCRIPTOR_STRUCT)];
} PACKED DMA_DESCRIPTOR_STRUCT, *PDMA_DESCRIPTOR_STRUCT;


#define	PACKET_GENERATOR_ENGINE_OFFSET	0x0000a000

// PACKET_GENERATOR_STRUCT
//
//  Packet Generator Structure - Information for the Packet Generator
typedef struct _PACKET_GENENRATOR_STRUCT
{
	ULONG32		Control;			// Packet Generator Control DWORD
	ULONG32		NumPackets;			// Count of packet to generate, 0 = infinite
	ULONG32		DataSeed;			// Data Seed pattern
	ULONG32		UserCtrlStatSeed;	// Seed for the User Control/Status fields
	ULONG32		reserved[4];		// reserved
	ULONG32 	PacketLength[4];	// Packet Length array
	UCHAR		Reserved[0x100-(12*sizeof(ULONG32))];	// Reserved size
} PACKED PACKET_GENENRATOR_STRUCT, *PPACKET_GENENRATOR_STRUCT;

#ifndef WIN32   // Linux version ---------------------------------------------

static const uint32_t  BAR_TYPE_MASK       = 0x1;
static const uint32_t  BAR_TYPE_MEM        = 0x0;
static const uint32_t  BAR_TYPE_IO         = 0x1;
static const uint32_t  BAR_MEM_ADDR_MASK   = 0x6;
static const uint32_t  BAR_MEM_ADDR_32     = 0x0;
static const uint32_t  BAR_MEM_ADDR_1M     = 0x2;
static const uint32_t  BAR_MEM_ADDR_64     = 0x4;
static const uint32_t  BAR_MEM_CACHE_MASK  = 0x8;
static const uint32_t  BAR_MEM_CACHABLE    = 0x8;

#define BAR_MEM_MASK        (~0x0F)
#define BAR_IO_MASK         (~0x03)
#define IS_IO_BAR(x)            (((x) & BAR_TYPE_MASK)      == BAR_TYPE_IO)
#define IS_MEMORY_BAR(x)        (((x) & BAR_TYPE_MASK)      == BAR_TYPE_MEM)
#define IS_MEMORY_32BIT(x)      (((x) & BAR_MEM_ADDR_MASK)  == BAR_MEM_ADDR_32)
#define IS_MEMORY_64BIT(x)      (((x) & BAR_MEM_ADDR_MASK)  == BAR_MEM_ADDR_64)
#define IS_MEMORY_BELOW1M(x)    (((x) & BAR_MEM_ADDR_MASK)  == BAR_MEM_ADDR_1M)
#define IS_MEMORY_CACHABLE(x)   (((x) & BAR_MEM_CACHE_MASK) == BAR_MEM_CACHABLE)

/* Bar Types
*/
#define REGISTER_MEM_PCI_BAR_TYPE  0
#define RAM_MEM_PCI_BAR_TYPE       1
#define REGISTER_IO_PCI_BAR_TYPE   2
#define DISABLED_PCI_BAR_TYPE      3

#define TRANSFER_SIZE_8_BIT        0x01    // 8 Bit transfer
#define TRANSFER_SIZE_16_BIT       0x02    // 16 Bit transfer
#define TRANSFER_SIZE_32_BIT       0x04    // 32 Bit transfer
#define TRANSFER_SIZE_64_BIT       0x08    // 64 Bit transfer (not currently supported)
#define TRANSFER_SIZE_32_BIT_DMA   0x10    // 32 Bit transfer
#define TRANSFER_SIZE_64_BIT_DMA   0x20    // 64 Bit transfer

#endif  // Linux version ---------------------------------------------

//
// Register Structure located at BAR0
typedef struct _BAR0_REGISTER_MAP_STRUCT
{
	DMA_ENGINE_STRUCT		dmaEngine[MAX_NUM_DMA_ENGINES];	// Pointer to the base of the DMA Engines in BAR0 space
	DMA_COMMON_CONTROL_STRUCT	commonControl;		// Pointer to the common control struct in BAR0 space
	UCHAR					Reserved[PACKET_GENERATOR_ENGINE_OFFSET - 
								((sizeof(DMA_ENGINE_STRUCT) * MAX_NUM_DMA_ENGINES) + 
								 sizeof(DMA_COMMON_CONTROL_STRUCT))];	// Reserved size
	PACKET_GENENRATOR_STRUCT	packetGen[MAX_NUM_DMA_ENGINES];
} PACKED BAR0_REGISTER_MAP_STRUCT, *PBAR0_REGISTER_MAP_STRUCT;


#pragma pack(pop)

#endif // _DMA_DRIVER_HW_H_
