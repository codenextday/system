/*******************************************************************************
  Copyright (c) 2019 Huajie IMI Technology Co., Ltd.
  All rights reserved.

  @brief        camera default config
  @creator      Qianyu Liu

  @History
      When          Who             What, where, why
      ----------    ------------    ----------------------------------------
      2019/04/12    Qianyu Liu      the initial version

*******************************************************************************/

#ifdef __cpluscplus
extern "C" {
#endif

#include <ebase/trace.h>
#include <cam_engine/cam_engine_api.h>
#include <som_ctrl/som_ctrl_api.h>
#include <fpga/altera_fpga.h>
#include "calibdb.h"
#include "cm_interface.h"
#include "cm_debug.h"

CREATE_TRACER(ISP_CFG_DBG,  "ISP_CFG_DBG:",     WARNING,    1);
CREATE_TRACER(ISP_CFG_INFO, "ISP_CFG_INFO:",    INFO,       1);
CREATE_TRACER(ISP_CFG_WARN, "ISP_CFG_WARN:",    WARNING,    1);
CREATE_TRACER(ISP_CFG_ERR,  "ISP_CFG_ERR:",     ERROR,      1);

//#define IMI_SENSOR_OV5695
//#define IMI_SENSOR_OV5648
//#define IMI_SENSOR_AR330
#define IMI_SENSOR_AR230

const cam_path_cfg_t g_cam_path_disabled =
{
	0U,					// width
	0U,					// height
	CAMERIC_MI_DATAMODE_DISABLED,		// mode
	CAMERIC_MI_DATASTORAGE_SEMIPLANAR,	// layout
	BOOL_FALSE,				// dcEnable
	{ 0U, 0U, 0U, 0U }			// dcWin
};

#ifdef  IMI_SENSOR_AR230

static sensor_param_t g_cam_sensor_param = {
	"/usr/lib/ar230.drv",
	{
		(cam_path_cfg_t*)&g_default_path_preview,
		(cam_path_cfg_t*)&g_cam_path_disabled
	},
	0, //Mhz
	BOOL_FALSE, //Enable mipi to dvp
};

camsys_mipi_param_t g_mipi_param = {0, 0, 3, 0, 0, 0, 0, 0, 0};

const cam_path_cfg_t g_default_path_preview =
{
	1920U,
	1080U,
	CAMERIC_MI_DATAMODE_YUV422, // mode outputmode:YUV422
	// CAMERIC_MI_DATASTORAGE_SEMIPLANAR,       // layout
	CAMERIC_MI_DATASTORAGE_INTERLEAVED,
	BOOL_FALSE, // dcEnable
	{ 0U, 0U, 1920U, 1080U }  // dcWin
};

#elif defined  IMI_SENSOR_OV5648

static sensor_param_t g_cam_sensor_param = {
	"/usr/lib/ov5648.drv",
	{
		(cam_path_cfg_t*)&g_default_path_preview,
		(cam_path_cfg_t*)&g_cam_path_disabled
	},
	420, //Mhz
	BOOL_TRUE, //Enable mipi to dvp
};

camsys_mipi_param_t g_mipi_param = {0, 0, 0, 0, 0x2b, 2, 420, 1280, 960};

const cam_path_cfg_t g_default_path_preview =
{
	1280U,
	960U,
	CAMERIC_MI_DATAMODE_YUV422, // mode outputmode:YUV422
	// CAMERIC_MI_DATASTORAGE_SEMIPLANAR,       // layout
	CAMERIC_MI_DATASTORAGE_INTERLEAVED,
	BOOL_FALSE, // dcEnable
	{ 0U, 0U, 1280U, 960U }  // dcWin
};

#elif defined  IMI_SENSOR_OV5695

static sensor_param_t g_cam_sensor_param = {
	"/usr/lib/ov5695.drv",
	{
		(cam_path_cfg_t*)&g_default_path_preview,
		(cam_path_cfg_t*)&g_cam_path_disabled
	},
	420, //Mhz
	BOOL_TRUE, //Enable mipi to dvp
};

camsys_mipi_param_t g_mipi_param = {0, 0, 0, 1, 0x2b, 2, 420, 1920, 1080};

const cam_path_cfg_t g_default_path_preview =
{
	1920U,
	1080U,
	CAMERIC_MI_DATAMODE_YUV422, // mode outputmode:YUV422
	// CAMERIC_MI_DATASTORAGE_SEMIPLANAR,       // layout
	CAMERIC_MI_DATASTORAGE_INTERLEAVED,
	BOOL_FALSE, // dcEnable
	{ 0U, 0U, 1920U, 1080U }  // dcWin
};

#else //IMI_SENSOR_AR330

static sensor_param_t g_cam_sensor_param = {
	"/usr/lib/ar330.drv",
	{
		(cam_path_cfg_t*)&g_default_path_preview,
		(cam_path_cfg_t*)&g_cam_path_disabled
	},
	0,
	BOOL_FALSE, // disable mipi to dvp
};

camsys_mipi_param_t g_mipi_param = {0, 0, 3, 0, 0, 0, 0, 0, 0};

const cam_path_cfg_t g_default_path_preview =
{
	640U,   // width
	480U,   // height
	CAMERIC_MI_DATAMODE_YUV422, // mode outputmode:YUV422
	// CAMERIC_MI_DATASTORAGE_SEMIPLANAR,       // layout
	CAMERIC_MI_DATASTORAGE_INTERLEAVED,
	BOOL_FALSE, // dcEnable
	{ 0U, 0U, 640U, 480U }  // dcWin
};
#endif

sensor_param_t *get_sensor_param(void)
{
	return &g_cam_sensor_param;
}

/*
 * get_opened_path() - get opened path. only one path
 *
 * @void
 *
 * @return: opened path or main path when failed
 */
int8_t get_opened_path(void)
{
	struct sensor_params *psensor_param = get_sensor_param();
	int8_t path = CAMERIC_MI_PATH_MAIN;

	for (; path < CAMERIC_MI_PATH_MAX; path++)
		if (psensor_param->ppath_preview[path] != &g_cam_path_disabled)
			return path;

	TRACE(ISP_CFG_ERR, "get opened path failed\n");
	return CAMERIC_MI_PATH_MAIN;
}

/*
 * default_config() - Default config
 *
 * @argc:   cmd num
 * @argv:   cmd array
 */
int default_config(int argc, char **argv)
{
	SET_TRACE_LEVEL(2);

	return isp_parse_cmd(argc, argv);
}

#ifdef __cpluscplus
}
#endif

