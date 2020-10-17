
#ifndef _MIPI_DPHY_H_
#define _MIPI_DPHY_H_

#include "typedef.h"
#include <common/return_codes.h>
#include <ebase/types.h>
#include <sys/mman.h>

#define IMG_WIDTH        1280//1296//2560
#define IMG_HEIGHT        960//972//1920

#define ISP_DDR_BASE 0x80200000
#define MIPI_DAT_TYPE  0x2B //2A:raw 8, 2B:raw 10, 2C: raw 12;
#define MIPI_TEST_CHIP  1
#define MIPI_DPHY_REG_ADDR_MOD(x) (x)
/* DEBUG:3, INFO:2, ERROR:1 */
#define MIPI_DEBUG_LEVEL    3

#define MIPI_LVL_DEBUG  3
#define MIPI_LVL_INFO       2
#define MIPI_LVL_ERR        1
#define MIPI_DBG(fmt, arg...) do { \
	if (MIPI_LVL_DEBUG <= MIPI_DEBUG_LEVEL) \
		printf("MIPI dphy:" fmt, ##arg); \
	} while(0);
#define MIPI_INFO(fmt, arg...) do { \
	if (MIPI_LVL_INFO <= MIPI_DEBUG_LEVEL) \
		printf("MIPI dphy:" fmt, ##arg); \
	} while(0);
#define MIPI_ERR(fmt, arg...) do { \
	if (MIPI_LVL_ERR <= MIPI_DEBUG_LEVEL) \
		printf("MIPI dphy:" fmt, ##arg); \
	} while(0);
/*
* MIPI_CSI_CFG
*/
#define MIPI_VERSION                  0x00000000
#define MIPI_N_LANES                  0x00000004
#define MIPI_CSI2_RESET               0x00000008
#define MIPI_INT_ST_MAIN              0x0000000C
#define MIPI_DATA_IDS_1               0x00000010
#define MIPI_DATA_IDS_2               0x00000014
#define MIPI_PHY_SHUTDOWNZ            0x00000040
#define MIPI_DPHY_RSTZ                0x00000044
#define MIPI_PHY_RX                   0x00000048
#define MIPI_PHY_STOPSTATE            0x0000004C
#define MIPI_PHY_TEST_CTRL0           0x00000050
#define MIPI_PHY_TEST_CTRL1           0x00000054
#define MIPI_PHY2_TEST_CTRL0          0x00000058
#define MIPI_PHY2_TEST_CTRL1          0x0000005C
#define MIPI_IPI_MODE                 0x00000080
#define MIPI_IPI_VCID                 0x00000084
#define MIPI_IPI_DATA_TYPE            0x00000088
#define MIPI_IPI_MEM_FLUSH            0x0000008C
#define MIPI_IPI_HSA_TIME             0x00000090
#define MIPI_IPI_HBP_TIME             0x00000094
#define MIPI_IPI_HSD_TIME             0x00000098
#define MIPI_IPI_HLINE_TIME           0x0000009C
#define MIPI_IPI_VSA_LINES            0x000000B0
#define MIPI_IPI_VBP_LINES            0x000000B4
#define MIPI_IPI_VFP_LINES            0x000000B8
#define MIPI_IPI_VACTIVE_LINES        0x000000BC
#define MIPI_PHY_CAL                  0x000000CC
#define MIPI_INT_ST_PHY_FATAL         0x000000E0
#define MIPI_INT_MSK_PHY_FATAL        0x000000E4
#define MIPI_INT_FORCE_PHY_FATAL      0x000000E8
#define MIPI_INT_ST_PKT_FATAL         0x000000F0
#define MIPI_INT_MSK_PKT_FATAL        0x000000F4
#define MIPI_INT_FORCE_PKT_FATAL      0x000000F8
#define MIPI_INT_ST_FRAME_FATAL       0x00000100
#define MIPI_INT_MSK_FRAME_FATAL      0x00000104
#define MIPI_INT_FORCE_FRAME_FATAL    0x00000108
#define MIPI_INT_ST_PHY               0x00000110
#define MIPI_INT_MSK_PHY              0x00000114
#define MIPI_INT_FORCE_PHY            0x00000118
#define MIPI_INT_ST_PKT               0x00000120
#define MIPI_INT_MSK_PKT              0x00000124
#define MIPI_INT_FORCE_PKT            0x00000128
#define MIPI_INT_ST_LINE              0x00000130
#define MIPI_INT_MSK_LINE             0x00000134
#define MIPI_INT_FORCE_LINE           0x00000138
#define MIPI_INT_ST_IPI               0x00000140
#define MIPI_INT_MSK_IPI              0x00000144
#define MIPI_INT_FORCE_IPI            0x00000148

#define HIGH         1
#define LOW          0
#define TX           1
#define RX           0
#define DIRECT       1
#define REVERSE      0

#define IMG_V_CROP    0 //16
#define IMG_H_CROP    0

#define LANE_NUM     2
#define MIPI_BPS     420 //480 //600Mbps
#define ISP_PIXEL_BYTES 2

#define VI_CSI_0_FRAME_SIZE           0x00000140
#define VI_CSI_1_FRAME_SIZE           0x00000144
#define VI_CSI_2_FRAME_SIZE           0x00000148

#define MIPI_DPHY0_BASE_ADDR   0x32800000  // 0x32800000 ~ 0x328FFFFF (1MB)  Peripheral 8
#define MIPI_DPHY1_BASE_ADDR   0x32900000  // 0x32900000 ~ 0x329FFFFF (1MB)  Peripheral 9
#define MIPI_DPHY2_BASE_ADDR   0x32C00000  // 0x32C00000 ~ 0x32CFFFFF (1MB)  Peripheral 12

#define VI_BASE_ADDR 0x32a00000

#define VI_CSI_1_DPHY_CTRL            0x00000054
#define VI_CSI_1_DPHY_LANE0           0x00000058
#define VI_CSI_1_DPHY_PLL_REG0        0x0000005C
#define VI_CSI_1_DPHY_PLL_REG1        0x00000060
#define VI_CSI_1_DPHY_PLL_REG2        0x00000064
#define VI_CSI_2_DPHY_LANE0           0x0000006C
#define VI_CSI_0_DPHY_LANE0           0x00000044
#define VI_CSI_0_DPHY_CTRL            0x00000040
#define VI_CSI_2_DPHY_CTRL            0x00000068

#define MIPI_DHPY_BASE_ADDR 0x32900000

#define VI_TOP_CTRL 0x00000000
#define VI_MIPI_OUT_EN 0x0000002c

#define REG_WRITE32(addr, val)             WRITE_DWORD(addr,val)
#define REG_READ32(addr)                   READ_DWORD(addr)

#define MIPI_REGISTER_SPACE_SIZE 512*1024

struct Imi_ImageConfig_s
{
	unsigned int height;
	unsigned int width;
	unsigned int clk;
};

typedef struct Imi_ImageConfig_s  Imi_ImageConfig_t;

typedef int (Imi_Mipi_To_Dvp_t)(bool_t on, struct Imi_ImageConfig_s *p);

typedef unsigned int (Imi_GetMipiRegVaddr_t)(int fd);

struct ImiMipiToDvp_s
{
	Imi_Mipi_To_Dvp_t	*pImi_Mipi_To_Dvp;
	Imi_GetMipiRegVaddr_t	*pImi_GetMipiRegVaddr;
};

typedef struct ImiMipiToDvp_s ImiMipiToDvpIss_t;

typedef RESULT (ImiGetMipiIss_t)(ImiMipiToDvpIss_t *pImiMipiToDvpIss);

typedef struct ImiMipiDrvConfig_s
{
	ImiGetMipiIss_t         *pfImiGetMipiIss;
	ImiMipiToDvpIss_t       ImiMipiToDvpIss;
	Imi_ImageConfig_t       Imi_ImageConfig;
} ImiMipiDrvConfig_t;

extern void isp_test_pattern_mode (struct Imi_ImageConfig_s *p);
extern int imi_Mipi_To_Dvp(bool_t on, struct Imi_ImageConfig_s *p);
extern volatile unsigned int* mipi_vi_regs_mmap;
extern volatile unsigned int* mipi_dphy2_regs_mmap;
extern volatile unsigned int* mipi_dphy1_regs_mmap;
extern volatile unsigned int* mipi_dphy0_regs_mmap;
#endif

