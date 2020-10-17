/***************************************************************************
  Copyright (c) 2019 Huajie IMI Technology Co., Ltd.
  All rights reserved.

  @brief        camif drv
  @creator      Qianyu Liu

  @History
      When          Who             What, where, why
      ----------    ------------    ------------------------------------
      2019/04/16    Qianyu Liu      the initial version

***************************************************************************/

#ifndef __CAMIF_H__
#define __CAMIF_H__

#ifdef __cpluscplus
extern "C" {
#endif

#define CAMIF_CHN0_SEN_IF_0         0x0
#define CAMIF_CHN0_SEN_IF_1         0x4
#define CAMIF_CHN0_WIN_LEFT_UP      0x10
#define CAMIF_CHN0_WIN_RIGHT_DOWN   0x14
#define CAMIF_CHN0_ADR_00           0x30
#define CAMIF_CHN1_SEN_IF_0         0x100
#define CAMIF_CHN1_SEN_IF_1         0x104
#define CAMIF_CHN1_WIN_LEFT_UP      0x110
#define CAMIF_CHN1_WIN_RIGHT_DOWN   0x114
#define CAMIF_CHN1_ADR_00           0x130
#define CAMIF_CHN2_SEN_IF_0         0x200
#define CAMIF_CHN2_SEN_IF_1         0x204
#define CAMIF_CHN2_WIN_LEFT_UP      0x210
#define CAMIF_CHN2_WIN_RIGHT_DOWN   0x214
#define CAMIF_CHN2_ADR_00           0x230
#define CAMIF_CHN3_SEN_IF_0         0x300
#define CAMIF_CHN3_SEN_IF_1         0x304
#define CAMIF_CHN3_WIN_LEFT_UP      0x310
#define CAMIF_CHN3_WIN_RIGHT_DOWN   0x314
#define CAMIF_CHN3_OUT_CTRL         0x32C
#define CAMIF_CHN3_ADR_00           0x330
#define CAMIF_CHN3_ADR_10           0x338
#define CAMIF_INT_STATUS            0xF0
#define CAMIF_INT_ICR               0xE4

#define CAMIF_BUF_NUM               1
#define CAMIF_BUF_SIZE              (4 * 1024 * 1024)

typedef struct camif_drv_s {
	int     (*pfn_base_addr_map)(void);
	void    (*pfn_config)(uint32_t buf, uint16_t height, uint16_t width);
	bool_t  (*pfn_irq)(void);
	void    (*pfn_irq_reset)(void);
}camif_drv_t;

extern const camif_drv_t g_camif_drv;

#ifdef __cpluscplus
}
#endif

#endif

