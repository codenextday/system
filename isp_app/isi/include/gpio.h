/***************************************************************************
  Copyright (c) 2019 Huajie IMI Technology Co., Ltd.
  All rights reserved.

  @brief        camif drv
  @creator      Qianyu Liu

  @History
      When          Who             What, where, why
      ----------    ------------    ------------------------------------
      2019/05/28    Qianyu Liu      the initial version

***************************************************************************/

#ifndef __GPIO_H__
#define __GPIO_H__

#ifdef __cpluscplus
extern "C" {
#endif

#define GPIO_GROUP_0                0
#define GPIO_GROUP_1                32
#define GPIO_GROUP_2                64

enum GPIO_SET_DIR {
	GPIO_DIR_IN,
	GPIO_DIR_OUT
};
typedef uint8_t gpio_set_dir_uint8;

enum GPIO_OUT_VAL {
	GPIO_OUT_LOW,
	GPIO_OUT_HIGH
};
typedef uint8_t gpio_out_val_uint8;

typedef struct gpio_drv_s {
	int     (*pfn_export)(uint8_t gpio);
	int     (*pfn_unexport)(uint8_t gpio);
	int     (*pfn_set_dir)(uint8_t gpio, gpio_set_dir_uint8 out);
	int     (*pfn_set_value)(uint8_t gpio, gpio_out_val_uint8 value);
	char    (*pfn_get_value)(uint8_t gpio);
	int     (*pfn_set_edge)(uint8_t gpio, char *edge);
	int     (*pfn_set_output)(uint8_t gpio, uint8_t on);
	int     (*pfn_reset)(uint8_t gpio);
}gpio_drv_t;

extern const gpio_drv_t g_gpio_drv;

#ifdef __cpluscplus
}
#endif

#endif

