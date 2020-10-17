/*******************************************************************************
  Copyright (c) 2019 Huajie IMI Technology Co., Ltd.
  All rights reserved.

  @brief        gpio
  @creator      Qianyu Liu

  @History
      When          Who             What, where, why
      ----------    ------------    ----------------------------------------
      2019/05/27    Qianyu Liu      the initial version

*******************************************************************************/

#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "types.h"
#include "trace.h"
#include "oslayer.h"
#include "gpio.h"

#define SYS_GPIO_DIR        "/sys/class/gpio/"
#define GPIO_WRITE_BUF_LEN  32
#define GPIO_FILE_BUF_LEN   64

#define GPIO_DIR_OUT_VAL    "out"
#define GPIO_DIR_IN_VAL     "in"

#define GPIO_DIR_OUT_HIGH   "1"
#define GPIO_DIR_OUT_LOW    "0"

CREATE_TRACER(GPIO_DBG,     "GPIO_DBG ",    WARNING,    1);
CREATE_TRACER(GPIO_INFO,    "GPIO_INFO ",   INFO,       1);
CREATE_TRACER(GPIO_WARN,    "GPIO_WARN ",   WARNING,    1);
CREATE_TRACER(GPIO_ERR,     "GPIO_ERR ",    ERROR,      1);

/*
 * gpio_write_to_file() - Write buffer to gpio_file
 *
 * @file:   gpio file name
 * @buf:    write value
 * @len:    buf len
 */
static int gpio_write_to_file(char *file, char *buf, int len)
{
	int ret = -1;
	int fd;

	fd = open(file, O_WRONLY);
	if(fd <= 0)
		goto exit;

	/* len > "" */
	ret = (len < 1) || (write(fd, buf, len) != len);
	ret |= close(fd);

exit:
	if (ret)
		TRACE(GPIO_ERR, "%s %d:%s fd=%d file=%s buf=%s len=%d ret=%d\n",
				__func__, errno, strerror(errno), fd, file,
				buf, len, ret);
	return ret;
}

/*
 * gpio_export() - Export or unexport gpio
 *
 * @gpio:   gpio num
 */
static int gpio_export(uint8_t gpio)
{
	int len = 0;
	char buf[GPIO_WRITE_BUF_LEN];

	len = snprintf(buf, sizeof(buf), "%d", gpio);

	return gpio_write_to_file(SYS_GPIO_DIR"export", buf, len);
}

/*
 * gpio_export() - Unexport or unexport gpio
 *
 * @gpio:   gpio num
 */
static int gpio_unexport(uint8_t gpio)
{
	int len = 0;
	char buf[GPIO_WRITE_BUF_LEN];

	len = snprintf(buf, sizeof(buf), "%d", gpio);

	return gpio_write_to_file(SYS_GPIO_DIR"unexport", buf, len);
}

/*
 * gpio_set_dir() - Set gpio direction out or in
 *
 * @gpio:   gpio num
 * @out:    enum GPIO_SET_DIR
 */
static int gpio_set_dir(uint8_t gpio, gpio_set_dir_uint8 out)
{
	char file[GPIO_FILE_BUF_LEN];

	snprintf(file, sizeof(file), SYS_GPIO_DIR"gpio%d/direction", gpio);

	if (out)
		return gpio_write_to_file(file, GPIO_DIR_OUT_VAL,
				sizeof(GPIO_DIR_OUT_VAL));
	else //in
		return gpio_write_to_file(file, GPIO_DIR_IN_VAL,
				sizeof(GPIO_DIR_IN_VAL));
}

/*
 * gpio_set_edge() - Set gpio interrupt
 *
 * @gpio:   gpio num
 * @edge:
 */
static int gpio_set_edge(uint8_t gpio, char *edge)
{
	char file[GPIO_FILE_BUF_LEN];
	snprintf(file, sizeof(file), SYS_GPIO_DIR"gpio%d/edge", gpio);
	return gpio_write_to_file(file, edge, strlen(edge) + 1);
}

/*
 * gpio_set_value() - Gpio set high or low electrical level
 *
 * @gpio:   gpio num
 * @value:  enum GPIO_OUT_VAL
 */
static int gpio_set_value(uint8_t gpio, gpio_out_val_uint8 value)
{
	char file[GPIO_FILE_BUF_LEN];

	snprintf(file, sizeof(file), SYS_GPIO_DIR"gpio%d/value", gpio);

	if (value)
		return gpio_write_to_file(file, GPIO_DIR_OUT_HIGH,
				sizeof(GPIO_DIR_OUT_HIGH));
	else //low electrical level
		return gpio_write_to_file(file, GPIO_DIR_OUT_LOW,
				sizeof(GPIO_DIR_OUT_LOW));
}

/*
 * gpio_set_value() - Get gpio set high or low electrical level
 *
 * @gpio:   gpio num
 *
 * @return: enum GPIO_OUT_VAL, -1=failed
 */
static char gpio_get_value(uint8_t gpio)
{
	char file[GPIO_FILE_BUF_LEN];
	int fd;
	char value = -1;
	int ret = -1;

	snprintf(file, sizeof(file), SYS_GPIO_DIR"gpio%d/value", gpio);

	fd = open(file, O_WRONLY);
	if(fd <= 0)
		goto exit;

	ret = (read(fd, &value, sizeof(char)) != sizeof(char));
	ret |= close(fd);

	if (value == '0')
		return GPIO_OUT_LOW;
	else if (value == '1')
		return GPIO_OUT_HIGH;
	else
		ret = -1;

exit:
	if (ret)
		TRACE(GPIO_ERR, "%s %d:%s, gpio=%d fd=%d val=%d ret=%d\n",
			__func__, errno, strerror(errno), gpio, fd, value, ret);

	return ret;
}

/*
 * gpio_set_output() - Export and set gpio output electrical level
 *
 * @gpio:   gpio num
 * @on:     mode[1:out 0:in]
 */
static int gpio_set_output(uint8_t gpio, uint8_t on)
{
	int ret = 0;

	ret |= gpio_export(gpio);
	osSleep(10);

	if (on) {
		gpio_set_dir(gpio, GPIO_DIR_OUT);
		osSleep(10);
	}

	gpio_set_value(gpio, on);
	osSleep(10);

	ret |= gpio_unexport(gpio);

	return ret;
}

/*
 * gpio_reset() - Set gpio 0 then set gpio 1
 *
 * @gpio:   gpio num
 */
static int gpio_reset(uint8_t gpio)
{
	int ret = 0;
	ret |= gpio_export(gpio);
	osSleep(10);
	ret |= gpio_set_dir(gpio, GPIO_DIR_OUT);
	osSleep(10);
	ret |= gpio_set_value(gpio, GPIO_OUT_LOW);
	osSleep(20);
	ret |= gpio_set_value(gpio, GPIO_OUT_HIGH);
	osSleep(10);
	ret |= gpio_unexport(gpio);
	osSleep(10);
	return ret;
}

const gpio_drv_t g_gpio_drv = {
	gpio_export,
	gpio_unexport,
	gpio_set_dir,
	gpio_set_value,
	gpio_get_value,
	gpio_set_edge,
	gpio_set_output,
	gpio_reset,
};

