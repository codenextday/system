/*******************************************************************************
  Copyright (c) 2019 Huajie IMI Technology Co., Ltd.
  All rights reserved.

  @brief    camif drv
  @creator  Qianyu Liu

  @History
    When        Who             What, where, why
    ----------  -----------     --------------------------------------------
    2019/04/16  Qianyu Liu      the initial version

*******************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include "trace.h"
#include "camif.h"

#define CAMIF_FILE_PATH "/dev/camif_marvin"

CREATE_TRACER(CAMIF_DBG,    "CAMIF_DBG:",   WARNING,    1);
CREATE_TRACER(CAMIF_INFO,   "CAMIF_INFO:",  INFO,       1);
CREATE_TRACER(CAMIF_WARN,   "CAMIF_WARN:",  WARNING,    1);
CREATE_TRACER(CAMIF_ERR,    "CAMIF_ERR:",   ERROR,      1);

static volatile uint32_t *g_camif_base_addr;

static void camif_reg_write(uint32_t addr, unsigned data)
{
	g_camif_base_addr[addr >> 2] = data;
}

static unsigned int camif_reg_read(uint32_t addr)
{
	return g_camif_base_addr[addr >> 2];
}

/*
 * camif_base_addr_map() - Mmap camif reg base addr
 *
 * @void
 */
static int camif_base_addr_map(void)
{
	int fd = open(CAMIF_FILE_PATH, O_RDWR);

	if (fd <= 0) {
		TRACE(CAMIF_ERR, "open camif drv file=%d fail\n", fd);
		return -1;
	}

	g_camif_base_addr = (unsigned int*) mmap(NULL, 0x10000,
			PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	close(fd);
	TRACE(CAMIF_WARN, "mmap camif regs addr=%p\n", g_camif_base_addr);

	return (!g_camif_base_addr) || (g_camif_base_addr == MAP_FAILED);
}

/*
 * camif_config() - Config camif
 *
 * @buf:    store sensor output buffer
 * @height: picture height
 * @width:  picture width
 */
static void camif_config(uint32_t buf, uint16_t height, uint16_t width)
{
	camif_reg_write(CAMIF_CHN3_SEN_IF_0,
			(height - 1) + ((width - 1) << 12));

	camif_reg_write(CAMIF_CHN3_WIN_LEFT_UP, 0x10010);

	camif_reg_write(CAMIF_CHN3_WIN_RIGHT_DOWN, 0x108f0cf);

	camif_reg_write(CAMIF_CHN3_ADR_00, buf);

	camif_reg_write(CAMIF_CHN3_SEN_IF_1, 0x34000000);
}

/*
 * camif_irq() - Start camif
 *
 * @void
 *
 * @return: camif interrupt
 */
static bool_t camif_irq(void)
{
	return ((camif_reg_read(CAMIF_INT_STATUS) >> 30) & 0x1) == 0x1;
}

/*
 * camif_irq_reset() - camif irq handler
 *
 * @void
 */
static void camif_irq_reset(void)
{
	camif_reg_write(CAMIF_INT_ICR, 0x8000000);
}

const camif_drv_t g_camif_drv = {
	camif_base_addr_map,
	camif_config,
	camif_irq,
	camif_irq_reset
};

