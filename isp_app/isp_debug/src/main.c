/*******************************************************************************
  Copyright (c) 2019 Huajie IMI Technology Co., Ltd.
  All rights reserved.

  @brief        isp debug app
  @creator      Qianyu Liu

  @History
      When          Who             What, where, why
      ----------    ------------    ----------------------------------------
      2019/04/16    Qianyu Liu      the initial version

*******************************************************************************/

#ifdef __cpluscplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "isp_debug.h"

/*
 * to_byte_array() - Transfer argv[] to array
 *
 * @argc:   cmd num
 * @argv:   cmd array
 * @buf:    return fifo buffer
 */
static void to_byte_array(int argc, char *argv[], char *buf)
{
	int i = 0;

	for (; i < argc; i++) {
		strncpy(buf, argv[i], ISP_DBG_MAX_LEN_PER_CMD - 1);
		buf[ISP_DBG_MAX_LEN_PER_CMD - 1] = '\0';
		buf += ISP_DBG_MAX_LEN_PER_CMD;
	}
}

int main(int argc, char *argv[])
{
	int fd = 0;
	char buf[ISP_DBG_MAX_LEN_CMD];
	size_t len = argc * ISP_DBG_MAX_LEN_PER_CMD;
	ssize_t ret = -1;

	if (argc < 2 || argc > ISP_DBG_MAX_CMD_NUM) {
		printf("Param num must be between %d and %d, but now the param"
				" num is %d. use '-h' to show help menu\n",
				1, ISP_DBG_MAX_CMD_NUM, argc);
		return -1;
	}

	fd = open(DBG_FIFO_NAME, O_WRONLY);
	if (fd > 0) {
		to_byte_array(argc, argv, buf);
		ret = write(fd, buf, len);
		ret |= close(fd);
	}

	if (ret != len) {
		printf("Connect to isp_app err %d:%s[fifo=%d, write_len=%d, "
				"ret=%d]. Please make sure isp_app is on\n",
				errno, strerror(errno), fd, len, ret);
		return -2;
	}

	return 0;
}

#ifdef __cpluscplus
}
#endif

