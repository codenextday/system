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

#ifndef __CM_DEBUG_H__
#define __CM_DEBUG_H__

#ifdef __cpluscplus
extern "C" {
#endif

typedef CamEnginePathConfig_t   cam_path_cfg_t;
typedef SensorParams_t          sensor_param_t;
typedef CamerIcMiDataLayout_t   cam_midata_layout_t;
typedef CamerIcMiDataMode_t     cam_midata_mode_t;

#define IMI_CAMIF_FPS_ARR_LEN   32      /* svn&app must be same */

typedef enum {
	DBG_SAVE_MODE_CLOSE = 0,
	DBG_SAVE_MODE_CAPTURE,
	DBG_SAVE_MODE_PREVIEW,
	DBG_SAVE_CAMIF_RAW,
	DBG_SAVE_MODE_BUTT
}DBG_SAVE_MODE;
typedef uint8_t dbg_save_mode_uint8;

/* not same with PicBufPlane_t */
typedef struct pic_buf_plane_s {
    uint8_t            *pbuf;
    uint16_t            height;
    uint16_t            widthbytes;
}pic_buf_plane_t;

typedef struct {
	pic_buf_plane_t    *pdata;
	uint8_t             type;
	uint8_t             layout;
	uint8_t             path;
	uint8_t             resv;
}isp_dbg_cb_buffer_t;

typedef struct save_image {
	int                 fd;
	uint32_t            frame_num;
	int                 first_frame;
	int                 intval;
	dbg_save_mode_uint8 mode;
	uint8_t             resv[3];
}save_image_t;

typedef struct fps_debug {
	uint8_t             print_num;
	uint8_t             on;
}fps_debug_t;

typedef struct {
	pthread_t           thd;
	int                 fifo;
	save_image_t        save_image;
	pic_buf_plane_t    *camif;
	fps_debug_t         fps;            //2 bytes
}isp_dbg_ctrl_t;

extern uint32_t g_isp_sensor_fps[];

extern void    *open_debug_fifo(void *priv);
extern void     get_isp_fps(void);
extern void     isp_store_buff(isp_dbg_cb_buffer_t *pdata);
extern int      isp_parse_cmd(int argc, char *argv[]);
extern int      debug_thread_off(void);
extern int      camif_alloc(HalHandle_t handle);
extern void     camif_release_safe(HalHandle_t handle);

extern dbg_save_mode_uint8 get_dbg_save_mode(void);

#ifdef __cpluscplus
}
#endif

#endif

