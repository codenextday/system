/**
 * \file DmaDriverIOCtl.h - Contains the Driver IOCtl defines,
 *       typedefs, structures and function prototypes.
 *
 * Copyright (c) 2008-2009 Northwest Logic
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

//
// This header defines all the User defined IOCTL codes and data structures for
// the PCI Driver.  The data structures sent to/from the driver are also defined.
// (Any user defined types that are shared by the driver and app must be defined in this
// function or by the system)
//
// Define control codes for DMADriver
//
// IOCTL  Description             	Data to Driver       	Data from Driver
// -----  -----------             	-----------------     	---------------------
//  800   Get Board Config        	None                  	BOARD_CONFIG_STRUCT
//  802   Memory Read             	DO_MEM_STRUCT      		data
//  803   Memory Write            	DO_MEM_STRUCT      		data
//  806   Get DMA Engine Cap      	EngineNum (ULONG)  		DMA_CAP_STRUCT
//  807   Packet Generator Ctrl   	PACKET_GEN_CTRL_STRUCT	none
//  808   Get DMA Performance     	EngineNum (ULONG)  		DMA_STAT_STRUCT
//
//  810   Block DMA Read          	DO_DMA_STRUCT      		data
//  811   Block DMA Write         	DO_DMA_STRUCT      		data
//
//  820   Packet DMA Buf Alloc    	BUF_ALLOC_STRUCT   		RET_BUF_ALLOC_STRUCT
//  821   Packet DMA Buf Release  	BUF_DEALLOC_STRUCT   	None
//  822   Packet Receive			PACKET_RECEIVE_STRUCT  	PACKET_RET_RECEIVE
//  824   Packet Send				PACKET_SEND_STRUCT		data

#ifndef __DMADriverioctl__h_
#define __DMADriverioctl__h_

#include <sys/ioctl.h>

#define MAX_BARS                        6       // TYPE-0 configuration header values.
#define PCI_STD_CONFIG_HEADER_SIZE      64      // 64 Bytes is the standard header size


#ifndef WIN32   // Linux version ---------------------------------------------

#ifndef PACKED
#    define PACKED                      __attribute__((packed))
#endif /* PACKED */

#define PROCFS_FB_NAME             "camsys_marvin"
#define PROCFS_FB_PATH             "/dev/" PROCFS_FB_NAME

#define NWLogic_VendorID            0x19AA
#define NWLogic_PciExp_E000         0xE000
#define NWLogic_PciExp_E001         0xE001
#define NWLogic_PciExp_E002         0xE002
#define NWLogic_PciExp_E003         0xE003
#define NWLogic_PciExp_E004         0xE004

/*!
** Number of descriptors to preallocate for each DMA engine. This
** will limit the maximum amount that can be transfered. Theoretically
** the maximum will be this number times the maximum amount of a
** single DMA descriptor transfer.
*/
#define NUM_DESCRIPTORS_PER_ENGINE          8192

// General IOCTL
#define GET_BOARD_CONFIG_IOCTL_BASE         0x800
#define DO_MEM_READ_ACCESS_IOCTL_BASE       0x802
#define DO_MEM_WRITE_ACCESS_IOCTL_BASE      0x803
#define GET_DMA_ENGINE_CAP_IOCTL_BASE       0x806
#define PACKET_GENERATOR_CONTROL_IOCTL_BASE 0x807
#define GET_PERF_IOCTL_BASE                 0x808

// Block Direct DMA IOCTL
#define BLOCK_DIRECT_DO_DMA_READ_IOCTL_BASE 0x810
#define BLOCK_DIRECT_DO_DMA_WRITE_IOCTL_BASE 0x811

// Packet DMA IOCTLs
#define PACKET_BUF_ALLOC_IOCTL_BASE 	    0x820
#define PACKET_BUF_RELEASE_IOCTL_BASE       0x821
#define PACKET_RECEIVE_IOCTL_BASE		    0x822
#define PACKET_SEND_IOCTL_BASE			    0x824
//#define PACKET_FLUSH_REQUESTS_IOCTL_BASE    0x825

// Target IRQ IOCTLs
#define REGISTER_TARGET_INT_IOCTL_BASE      0x830 // configure/register/deregister target interrupt
#define WAIT_FOR_TARGET_INT_IOCTL_BASE      0x831 // wait for target interrupt (blocking call)
#define CANCEL_TARGET_INT_IOCTL_BASE        0x832 // cancel target interrupt, i.e. release waiting threads, if any



#define PCI_DEVICE_SPECIFIC_SIZE 192    // Device specific bytes [64..256].

typedef struct _PCI_CONFIG_HEADER
{
    uint16_t    VendorId;           // VendorID[15:0]
    uint16_t    DeviceId;           // DeviceID[15:
    uint16_t    Command;            // Command[15:0]
    uint16_t    Status;             // Status[15:0]
    uint8_t     RevisionId;         // RevisionId[7:0]
    uint8_t     Interface;          // Interface[7:0]
    uint8_t     SubClass;           // SubClass[7:0]
    uint8_t     BaseClass;          // BaseClass[7:0]
    uint8_t     CacheLineSize;
    uint8_t     LatencyTimer;
    uint8_t     HeaderType;
    uint8_t     BIST;
    uint32_t    BarCfg[MAX_BARS];   // BAR[5:0] Configuration
    uint32_t    CardBusCISPtr;
    uint16_t    SubsystemVendorId;  // SubsystemVendorId[15:0]
    uint16_t    SubsystemId;        // SubsystemId[15:0]
    uint32_t    ExpRomCfg;          // Expansion ROM Configuration
    uint8_t     CapabilitiesPtr;
    uint8_t     Reserved1[3];
    uint32_t    Reserved2[1];
    uint8_t     InterruptLine;
    uint8_t     InterruptPin;
    uint8_t     MinimumGrant;
    uint8_t     MaximumLatency;
    uint8_t     DeviceSpecific[PCI_DEVICE_SPECIFIC_SIZE];

    uint8_t     PCIeCapabilityID;
    uint8_t     PCIeNextCapabilityPtr;
    uint16_t    PCIeCapability;
    uint32_t    PCIeDeviceCap;      // PCIe Capability: Device Capabilities
    uint16_t    PCIeDeviceControl;  // PCIe Capability: Device Control
    uint16_t    PCIeDeviceStatus;   // PCIe Capability: Device Status
    uint32_t    PCIeLinkCap;        // PCIe Capability: Link Capabilities
    uint16_t    PCIeLinkControl;    // PCIe Capability: Link Control
    uint16_t    PCIeLinkStatus;     // PCIe Capability: Link Status
} PACKED PCI_CONFIG_HEADER, *PPCI_CONFIG_HEADER;


/*! @brief target interrupt configuration
 *
 *  passed by Ioctl(REGISTER_TARGET_INT_IOCTL_BASE)
 */
typedef struct
{
    unsigned int misAdr;        ///< target (user design) address of Masked Interrupt Status register.
    							///< Specified as byte address, i.e. should be dividable by 4.
    unsigned int cisAdr;        ///< target (user design) address of Interrupt Clear register.
                                ///< Specified as byte address, i.e. should be dividable by 4.
    unsigned int irqTimeout;    ///< Timeout in [ms] for blocking Ioctl() waiting for a user design IRQ.
    							///< Ioctl() returns after timeout, but no IRQ was signalled (useful for
    							///<  buggy hardware).
								///< A timeout set to zero will result in an infinite wait, which is still
								///< interruptible by a signal sent to the calling thread. A timeout greater
								///< than zero will result in a _non_ interruptible wait.
} tsRegisterTargetIntSetup;


// BOARD_CONFIG_STRUCT
//
//  Board Configuration Definitions
typedef struct _BOARD_CONFIG_STRUCT
{
    PCI_CONFIG_HEADER   PciConfig;
// The following fields are decoded values from the PCI configuration space.
// Make sure these start on an 8-byte boundary.
    ULONG64     BaseAddresses[MAX_BARS];
    ULONG64     BaseSizes[MAX_BARS];
    USHORT      BarType[MAX_BARS];
    USHORT      BarTransferTypes[MAX_BARS];
    ULONG       AddressSize;

    ULONG       NumDmaWriteEngines; // Number of S2C DMA Engines
    ULONG       FirstDmaWriteEngine;// Number of 1st S2C DMA Engine
    ULONG       NumDmaReadEngines;  // Number of C2S DMA Engines
    ULONG       FirstDmaReadEngine; // Number of 1st C2S DMA Engine
    ULONG       NumDmaRWEngines;    // Number of bidirectional DMA Engines
    ULONG       FirstDmaRWEngine;   // Number of 1st bidirectional DMA Engine
	UCHAR		DriverVersionMajor;	// Driver Major Revision number
	UCHAR		DriverVersionMinor;	// Driver Minor Revision number
	UCHAR		DriverVersionSubMinor; // Driver Sub Minor Revision number
	UCHAR		DriverVersionBuildNumber; // Driver Build number
} BOARD_CONFIG_STRUCT, *PBOARD_CONFIG_STRUCT;


#else   // Window version  ------------------------------------------------------------

// General IOCTL
#define GET_BOARD_CONFIG_IOCTL          CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED,   FILE_ANY_ACCESS)
#define DO_MEM_READ_ACCESS_IOCTL        CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)
#define DO_MEM_WRITE_ACCESS_IOCTL       CTL_CODE(FILE_DEVICE_UNKNOWN, 0x803, METHOD_IN_DIRECT,  FILE_ANY_ACCESS)
#define GET_DMA_ENGINE_CAP_IOCTL        CTL_CODE(FILE_DEVICE_UNKNOWN, 0x806, METHOD_BUFFERED,   FILE_ANY_ACCESS)
#define PACKET_GENERATOR_CONTROL_IOCTL	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x807, METHOD_BUFFERED,   FILE_ANY_ACCESS)
#define GET_PERF_IOCTL     				CTL_CODE(FILE_DEVICE_UNKNOWN, 0x808, METHOD_BUFFERED,   FILE_ANY_ACCESS)

// Block Direct DMA IOCTL
#define BLOCK_DIRECT_DO_DMA_READ_IOCTL  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x810, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)
#define BLOCK_DIRECT_DO_DMA_WRITE_IOCTL CTL_CODE(FILE_DEVICE_UNKNOWN, 0x811, METHOD_IN_DIRECT,  FILE_ANY_ACCESS)

// Packet DMA IOCTLs
#define PACKET_BUF_ALLOC_IOCTL 			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x820, METHOD_IN_DIRECT, 	FILE_ANY_ACCESS)
#define PACKET_BUF_RELEASE_IOCTL 		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x821, METHOD_IN_DIRECT,  FILE_ANY_ACCESS)
#define PACKET_RECEIVE_IOCTL  			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x822, METHOD_IN_DIRECT, 	FILE_ANY_ACCESS)
#define PACKET_SEND_IOCTL 				CTL_CODE(FILE_DEVICE_UNKNOWN, 0x824, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

// Vendor and Device IDs
//static const USHORT NWLogic_VendorID    =   0x19AA;
//static const USHORT NWLogic_PciExp_E000 =   0xE000;
//static const USHORT NWLogic_PciExp_E001 =   0xE001;
//static const USHORT NWLogic_PciExp_E002 =   0xE002;
//static const USHORT NWLogic_PciExp_E003 =   0xE003;
//static const USHORT NWLogic_PciExp_E004 =   0xE004;

#pragma pack(push,1)

#ifndef PACKED
#define PACKED
#endif /* PACKED */

// BOARD_CONFIG_STRUCT
//
//  Board Configuration Definitions
typedef struct _BOARD_CONFIG_STRUCT
{
    USHORT      VendorId;           // VendorID[15:0]
    USHORT      DeviceId;           // DeviceID[15:0]
    USHORT      Command;            // Command[15:0]
    USHORT      Status;             // Status[15:0]
    UCHAR       RevisionId;         // RevisionId[7:0]
    UCHAR       Interface;          // Interface[7:0]
    UCHAR       SubClass;           // SubClass[7:0]
    UCHAR       BaseClass;          // BaseClass[7:0]
    USHORT      SubsystemVendorId;  // SubsystemVendorId[15:0]
    USHORT	    SubsystemId;        // SubsystemId[15:0]
    ULONG       BarCfg[MAX_BARS];   // BAR[5:0] Configuration
    ULONG       ExpRomCfg;          // Expansion ROM Configuration
    ULONG       PCIeDeviceCap;      // PCIe Capability: Device Capabilities
    USHORT      PCIeDeviceControl;  // PCIe Capability: Device Control
    USHORT      PCIeDeviceStatus;   // PCIe Capability: Device Status
    ULONG       PCIeLinkCap;        // PCIe Capability: Link Capabilities
    USHORT      PCIeLinkControl;    // PCIe Capability: Link Control
    USHORT      PCIeLinkStatus;     // PCIe Capability: Link Status
    ULONG       NumDmaWriteEngines; // Number of S2C DMA Engines
    ULONG       FirstDmaWriteEngine;// Number of 1st S2C DMA Engine
    ULONG       NumDmaReadEngines;  // Number of C2S DMA Engines
    ULONG       FirstDmaReadEngine; // Number of 1st C2S DMA Engine
    ULONG       NumDmaRWEngines;    // Number of bidirectional DMA Engines
    ULONG       FirstDmaRWEngine;   // Number of 1st bidirectional DMA Engine
	UCHAR		DriverVersionMajor;	// Driver Major Revision number
	UCHAR		DriverVersionMinor;	// Driver Minor Revision number
	UCHAR		DriverVersionSubMinor; // Driver Sub Minor Revision number
	UCHAR		DriverVersionBuildNumber; // Driver Build number
} BOARD_CONFIG_STRUCT, *PBOARD_CONFIG_STRUCT;



#endif  // WINDOWS version ----------------------------------------------------------



#ifdef DRIVER_ALLOCATED_RX_BUFFER_SUPPORT
#define	DEFAULT_RX_PACKET_BUFFER_SIZE		16777216	// Dafault to 16 Meg Buffer
#define DEFAULT_RX_MAX_PACKET_SIZE			65536		// Max Packet size default to 64K
#define	MAX_RX_BUFFER_ALLOCATE_SIZE			0x80000000	// Make the largest buffer 2GB.
#define MAX_RX_PACKET_SIZE					0x00200000	// Make the largest packet 2 meg.
#endif // DRIVER_ALLOCATED_RX_BUFFER_SUPPORT

// BarCfg defines
#define BAR_CFG_BAR_SIZE_OFFSET     	    (24)
#define BAR_CFG_BAR_PRESENT				    0x0001
#define BAR_CFG_BAR_TYPE_MEMORY			    0x0000
#define BAR_CFG_BAR_TYPE_IO		 		    0x0002
#define BAR_CFG_MEMORY_PREFETCHABLE		    0x0004
#define BAR_CFG_MEMORY_64BIT_CAPABLE	    0x0008

// ExpRomCfg defines
#define EXP_ROM_BAR_SIZE_OFFSET	            (24)
#define EXP_ROM_PRESENT			            0x0001
#define EXP_ROM_ENABLED			            0x0002

// Rd_Wr_n defines
#define WRITE_TO_CARD                       0x0             // S2C
#define READ_FROM_CARD                      0x1             // C2S

// Return status defines
#define STATUS_SUCCESSFUL		    	    0x00
#define STATUS_INCOMPLETE			        0x01
#define STATUS_INVALID_BARNUM		        0x02
#define STATUS_INVALID_CARDOFFSET	        0x04
#define STATUS_OVERFLOW				        0x08
#define STATUS_INVALID_BOARDNUM		        0x10			// Board number does not exist
#define STATUS_INVALID_MODE					0x20			// Mode not supported by hardware

// Return DMA status defines
#define STATUS_DMA_SUCCESSFUL				0x00
#define STATUS_DMA_INCOMPLETE				0x01
#define STATUS_DMA_INVALID_ENGINE			0x02
#define STATUS_DMA_INVALID_ENGINE_DIRECTION	0x04
#define STATUS_DMA_BUFFER_MAP_ERROR			0x08
#define STATUS_DMA_ENGINE_TIMEOUT			0x10
#define STATUS_DMA_ABORT_DESC_ALIGN_ERROR	0x20
#define STATUS_DMA_ABORT_BY_DRIVER			0x40
#define STATUS_DMA_ABORT_BY_HARDWARE		0x80

#ifndef MAX_NUM_DMA_ENGINES
#define MAX_NUM_DMA_ENGINES		            64
#endif // MAX_DMA_ENGINES

// DMA Engine Capability Defines
#define DMA_CAP_ENGINE_NOT_PRESENT			0x00000000
#define DMA_CAP_ENGINE_PRESENT				0x00000001
#define DMA_CAP_DIRECTION_MASK              0x00000006
#define DMA_CAP_SYSTEM_TO_CARD				0x00000000
#define DMA_CAP_CARD_TO_SYSTEM				0x00000002
#define DMA_CAP_BIDIRECTIONAL				0x00000006
#define DMA_CAP_ENGINE_TYPE_MASK            0x00000030
#define DMA_CAP_BLOCK_DMA                   0x00000000
#define DMA_CAP_PACKET_DMA                  0x00000010
#define DMA_CAP_ENGINE_NUMBER_MASK          0x0000FF00
#define	DMA_CAP_CARD_ADDRESS_SIZE_MASK		0x007F0000
#define DMA_CAP_MAX_DESCRIPTOR_BYTE_MASK	0x3F000000
#define DMA_CAP_ENGINE_NUMBER_SHIFT_SIZE    8
#define DMA_CAP_ENGINE_NUMBER(x)            (((x) & DMA_CAP_ENGINE_NUMBER_MASK) >> DMA_CAP_ENGINE_NUMBER_SHIFT_SIZE)
#define DMA_CAP_ENGINE_DIRECTION(X)		    ((X)& DMA_CAP_DIRECTION_MASK)
#define DMA_CAP_CARD_ADDR_SIZE_SHIFT		16
#define DMA_CAP_CARD_ADDR_SIZE_MASK			(0xff)
#define DMA_CAP_CARD_ADDR_SIZE(x)			\
	(1ULL << (((x) >> DMA_CAP_CARD_ADDR_SIZE_SHIFT) & DMA_CAP_CARD_ADDR_SIZE_MASK))


// DMA_STAT_STRUCT
//
// DMA Status Structure - Status Information from a DMA IOCTL transaction
typedef struct _DMA_STAT_STRUCT
{
#ifndef WIN32
    ULONG32	    EngineNum;          // DMA Engine number to use
#endif  // Linux Only
    ULONGLONG   CompletedByteCount; // Number of bytes transferred
    ULONGLONG   DriverTime;         // Number of nanoseconds for driver
    ULONGLONG   HardwareTime;       // Number of nanoseconds for hardware
    ULONGLONG   IntsPerSecond;      // Number of interrupts per second
    ULONGLONG   DPCsPerSecond;      // Number of DPCs/Tasklets per second
} DMA_STAT_STRUCT, *PDMA_STAT_STRUCT;

// DO_MEM_STRUCT
//
// Do Memory Structure - Information for performing a Memory Transfer
typedef struct _DO_MEM_STRUCT
{
	ULONG		Rd_Wr_n;            // 1 == Read, 0 == Write
	ULONG		BarNum;             // Base Addres Register (BAR) to access
	ULONGLONG	Offset;             // Byte starting offset in application buffer
	ULONGLONG	CardOffset;         // Byte starting offset in BAR
	ULONGLONG	Length;             // Transaction length in bytes
#ifndef WIN32
    unsigned char * Buffer;         // Buffer Pointer
#endif  // Linux Only
} DO_MEM_STRUCT, *PDO_MEM_STRUCT;

// DMA_CAP_STRUCT
//
// DMA Capabilities Structure
typedef struct _DMA_CAP_STRUCT
{
#ifndef WIN32
    ULONG32	    EngineNum;          // DMA Engine number to use
#endif  // Linux Only
	ULONG	    DmaCapabilities;    // DMA Capabilities
} DMA_CAP_STRUCT, *PDMA_CAP_STRUCT;

//*************************************************************
// Block Mode defines and Structures

// DO_DMA_STRUCT
//
// Do DMA Structure - Information for performing a BLOCK DMA Transfer
typedef struct _DO_DMA_STRUCT
{
	ULONG32	    EngineNum;          // DMA Engine number to use
//	ULONG64 	Offset;             // Byte starting offset in application buffer
	ULONG64		CardOffset;         // Byte starting offset in DMA Card Memory
	ULONG64		Length;             // Transaction length in bytes
#ifndef WIN32 // Linux variant
    void *      BufferAddress;      // Buffer Address for data transfer
#endif // Linux -
} DO_DMA_STRUCT, *PDO_DMA_STRUCT;

//*************************************************************
// Packet Mode defines and Structures

#define		ALLOC_FLAGS_APPLICATION_ALLOCATE	0x0000
#define		ALLOC_FLAGS_DRIVER_ALLOCATE			0x0001

//
// BUF_ALLOC_STRUCT
//
// Buffer Allocate Structure - Information for performing the Buffer Allocation task
typedef struct _BUF_ALLOC_STRUCT
{
	ULONG32     EngineNum;          // DMA Engine number to use
	ULONG32	    AllocationMode;     // Allocation type Flags
	ULONG32		MaxPacketSize;  	// Maximum Packet Size (Must be dividable into PoolSize)
	ULONG32		Length;				// Length of Application buffer or size to Allocate
#ifndef WIN32 // Linux variant
    void *      BufferAddress;      // Buffer Address for data transfer
#endif // Linux -
} BUF_ALLOC_STRUCT, *PBUF_ALLOC_STRUCT;

#ifdef WIN32 // Linux variant
//
// RET_BUF_ALLOC_STRUCT
//
// Return Buffer Allocate Structure - Information returned by the Rx Buffer Allocation task
typedef struct _RET_BUF_ALLOC_STRUCT
{
	ULONG64		RxBufferAddress;  	// Returned buffer address.
	ULONG32 	Length;        		// Length of Allocated buffer
	ULONG32		MaxPacketSize;  	// Maximum Packet Size
} RET_BUF_ALLOC_STRUCT, *PRET_BUF_ALLOC_STRUCT;
#endif // Only used on Windows version

// RX_BUF_DEALLOC_STRUCT
//
// Rx Buffer DeAllocate Structure - Information for performing the Rx Buffer Release task
typedef struct _BUF_DEALLOC_STRUCT
{
	ULONG32     EngineNum;          // DMA Engine number to use
	ULONG32	    AllocationMode;     // Allocation type Flags
	ULONG64		RxBufferAddress;  	// Buffer address to Release
} BUF_DEALLOC_STRUCT, *PBUF_DEALLOC_STRUCT;

#ifdef WIN32    /* Windows Version of Packet Structures */
// PACKET_RECEIVE_STRUCT
//
//  Packet Receive Structure - Information for the PacketReceive function
typedef struct _PACKET_RECEIVE_STRUCT
{
	ULONG32     EngineNum;          // DMA Engine number to use
	ULONG32	    RxReleaseToken;     // Recieve Token of buffer to release (if any)
} PACKET_RECEIVE_STRUCT, *PPACKET_RECEIVE_STRUCT;

// PACKET_RET_RECEIVE_STRUCT
//
//  Packet Return Receive Structure - Information from the PacketReceive function
typedef struct _PACKET_RET_RECEIVE_STRUCT
{
	ULONG32		RxToken;  			// Token for this recieve.
	ULONG64		Address;			// Address of data buffer for the receive
	ULONG32 	Length;        		// Length of packet
} PACKET_RET_RECEIVE_STRUCT, *PPACKET_RET_RECEIVE_STRUCT;
#else   /* Linux version of Packet Recieve structures */

//  Packet Receive Structure - Information for the PacketReceive function
typedef struct _PACKET_RECEIVE_STRUCT
{
	ULONG32     EngineNum;          // DMA Engine number to use
	ULONG32	    RxToken;            // Recieve Token of buffer to release (if any)
    ULONG64		Address;			// Address of data buffer for the receive
    ULONG32 	Length;        		// Length of packet
} PACKET_RECEIVE_STRUCT, *PPACKET_RECEIVE_STRUCT;

#endif  /* Windows vs. Linux Packet Recieve structure(s) */

// PACKET_SEND_STRUCT
//
//  Packet Send Structure - Information for the PacketSend function
typedef struct _PACKET_SEND_STRUCT
{
	ULONG32     EngineNum;          // DMA Engine number to use
    ULONG64		CardOffset;         // Byte starting offset in DMA Card Memory
	ULONG32 	Length;        		// Length of packet
#ifndef WIN32 // Linux variant
    void *      BufferAddress;      // Buffer Address for data transfer
#endif // Linux -
} PACKET_SEND_STRUCT, *PPACKET_SEND_STRUCT;


// Packet Generator Control DWORD Defines
#define PACKET_GEN_ENABLE					0x00000001
#define PACKET_GEN_LOOPBACK_ENABLE			0x00000002
#define PACKET_GEN_TABLE_ENTRIES_MASK		0x00000030
#define PACKET_GEN_DATA_PATTERN_MASK		0x00000700
#define PACKET_GEN_DATA_PATTERN_CONSTANT	0x00000000
#define PACKET_GEN_DATA_PATTERN_INC_BYTE	0x00000100
#define PACKET_GEN_DATA_PATTERN_LFSR		0x00000200
#define PACKET_GEN_DATA_PATTERN_INC_DWORD	0x00000300
#define PACKET_GEN_CONT_DATA_PATTERN		0x00000800
#define PACKET_GEN_USER_PATTERN_MASK		0x00007000
#define PACKET_GEN_USER_PATTERN_CONSTANT	0x00000000
#define PACKET_GEN_USER_PATTERN_INC_BYTE	0x00001000
#define PACKET_GEN_USER_PATTERN_LFSR		0x00002000
#define PACKET_GEN_USER_PATTERN_INC_DWORD	0x00003000
#define PACKET_GEN_CONT_USER_PATTERN		0x00008000
#define PACKET_GEN_ACTIVE_CLOCK_MASK		0x00FF0000
#define PACKET_GEN_INACTIVE_CLOCK_MASK		0xFF000000

// PACKET_GEN_CTRL_STRUCT
//
//  Packet Generator ControlStructure - Information for the Packet Generator
//  *********************THIS IS USED FOR TESTING ONLY**********************
typedef struct _PACKET_GEN_CTRL_STRUCT
{
	ULONG32     EngineNum;          // DMA Engine number to use
	ULONG32		Control;			// Packet Generator Control DWORD
	ULONG32		NumPackets;			// Count of packet to generate, 0 = infinite
	ULONG32		DataSeed;			// Data Seed pattern
	ULONG32		UserCtrlStatSeed;	// Seed for the User Control/Status fields
	ULONG32 	PacketLength[4];	// Packet Length array
} PACKET_GEN_CTRL_STRUCT, *PPACKET_GEN_CTRL_STRUCT;

typedef struct camsys_irqsta_s {
    unsigned int mis;					//Masked interrupt status
} camsys_irqsta_t;

typedef struct camsys_irqcnnt_s {
    unsigned int mis;
    unsigned int icr;
    unsigned int timeout;				//us
} camsys_irqcnnt_t;

typedef struct camsys_i2c_info_s {
    unsigned char     bus_num;
    unsigned short    slave_addr;
    unsigned int      reg_addr;			//i2c device register address
    unsigned int      reg_size;			//register address size
    unsigned int      val;
    unsigned int      val_size;			//register value size
    unsigned int      i2cbuf_directly;
    unsigned int      i2cbuf_bytes;
    unsigned int      speed;			//100000 == 100KHz
} camsys_i2c_info_t;

typedef struct camsys_getmembase_s {
    unsigned long phy_addr;					//Masked interrupt status
} camsys_getmembase_t;

/*
 *	I O C T L   C O D E S   F O R    H J I M I   C A M S Y S   D E V I C E S
 *
 */

#define CAMSYS_IOC_MAGIC	'M'
#define CAMSYS_IOC_MAXNR	14

#define CAMSYS_I2CRD			_IOWR(CAMSYS_IOC_MAGIC, 0, camsys_i2c_info_t)
#define CAMSYS_I2CWR			_IOW(CAMSYS_IOC_MAGIC,  1, camsys_i2c_info_t)
#define CAMSYS_IRQCONNECT		_IOW(CAMSYS_IOC_MAGIC,  2, camsys_irqcnnt_t)
#define CAMSYS_IRQWAIT			_IOWR(CAMSYS_IOC_MAGIC, 3, int)
#define CAMSYS_IRQDISCONNECT	_IOW(CAMSYS_IOC_MAGIC,  4, camsys_irqcnnt_t)
#define CAMSYS_IRQCANCEL		_IOW(CAMSYS_IOC_MAGIC,  5, camsys_irqcnnt_t)
#define CAMSYS_GETMEMBASE		_IOR(CAMSYS_IOC_MAGIC,  6, camsys_getmembase_t)

#ifdef WIN32   // Windows version
#pragma pack(pop)
#endif // Windows version

#endif /* __DMADriverioctl__h_ */

