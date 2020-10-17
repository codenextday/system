/***************************************************************************
  Copyright (c) 2019 Huajie IMI Technology Co., Ltd.
  All rights reserved.

  @brief        camera debug cmd process
  @creator      Qianyu Liu

  @History
      When          Who             What, where, why
      ----------    ------------    ------------------------------------
      2019/04/12    Qianyu Liu      the initial version

***************************************************************************/

#ifndef _ISP_DEBUG_H__
#define _ISP_DEBUG_H__

#ifdef __cpluscplus
extern "C" {
#endif

/* isp_debug app & cm_debug.cpp */
#define DBG_FIFO_NAME           "/tmp/isp_fifo"
#define ISP_DBG_MAX_LEN_PER_CMD 32
#define ISP_DBG_MAX_CMD_NUM     8
#define ISP_DBG_MAX_LEN_CMD     (ISP_DBG_MAX_CMD_NUM * ISP_DBG_MAX_LEN_PER_CMD)

#ifdef __cpluscplus
}
#endif

#endif
