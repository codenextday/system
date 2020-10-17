/*******************************************************************************
  Copyright (c) 2019 Huajie IMI Technology Co., Ltd.
  All rights reserved.

  @brief        camera debug cmd process
  @creator      Qianyu Liu

  @History
      When          Who             What, where, why
      ----------    ------------    ----------------------------------------
      2019/04/12    Qianyu Liu      the initial version

*******************************************************************************/

#ifdef __cpluscplus
extern "C" {
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ebase/trace.h>
#include <ebase/builtins.h>
#include <cam_engine/cam_engine_api.h>
#include <som_ctrl/som_ctrl_api.h>
#include <fpga/altera_fpga.h>
#include "calibdb.h"
#include "cm_interface.h"
#include "cm_debug.h"
#include "isp_debug.h"
#include "camif.h"

CREATE_TRACER(ISP_DBG_DBG,  "ISP_DBG_DBG::",    INFO,       1);
CREATE_TRACER(ISP_DBG_INFO, "ISP_DBG_INFO::",   INFO,       1);
CREATE_TRACER(ISP_DBG_WARN, "ISP_DBG_WARN::",   WARNING,    1);
CREATE_TRACER(ISP_DBG_ERR,  "ISP_DBG_ERR::",    ERROR,      1);

#define DBG_SAVE_FOLDER_NAME    "/usr/lib/isp_video_out"
#define ISP_DEBUG_FPS_CMD       "print"
#define ISP_DEBUG_FPS_CMD_LEN   (sizeof(ISP_DEBUG_FPS_CMD)  - 1)

typedef struct {
	uint8_t         en;
	const char     *description;
}dbg_desc_t;

typedef struct {
	uint8_t         type;       // type of picture data
	uint8_t         layout;     // kind of data layout
	const char     *description;
}pic_fmt_desc_raw_t;

static const dbg_desc_t g_dbg_save_mode_desc_table[DBG_SAVE_MODE_BUTT] = {
	{DBG_SAVE_MODE_CLOSE,   "closed"},
	{DBG_SAVE_MODE_CAPTURE, "capture"},
	{DBG_SAVE_MODE_PREVIEW, "preview"},
	{DBG_SAVE_CAMIF_RAW,    "raw from camif"},
};

static const pic_fmt_desc_raw_t g_pic_fmt_desc_table[] = {
	{
		PIC_BUF_TYPE_DATA,
		PIC_BUF_LAYOUT_COMBINED,
		".dat"
	},
	{
		PIC_BUF_TYPE_RAW8,
		PIC_BUF_LAYOUT_COMBINED,
		"_raw8.raw"
	},
	{
		PIC_BUF_TYPE_RAW8,
		PIC_BUF_LAYOUT_BAYER_RGRGGBGB,
		"_raw8_rggb.pgm"
	},
	{
		PIC_BUF_TYPE_RAW8,
		PIC_BUF_LAYOUT_BAYER_GRGRBGBG,
		"_raw8_grbg.pgm"
	},
	{
		PIC_BUF_TYPE_RAW8,
		PIC_BUF_LAYOUT_BAYER_GBGBRGRG,
		"_raw8_gbrg.pgm"
	},
	{
		PIC_BUF_TYPE_RAW8,
		PIC_BUF_LAYOUT_BAYER_BGBGGRGR,
		"_raw8_bggr.pgm"
	},
	{
		PIC_BUF_TYPE_RAW16,
		PIC_BUF_LAYOUT_COMBINED,
		"_raw16.raw"
	},
	{
		PIC_BUF_TYPE_RAW16,
		PIC_BUF_LAYOUT_BAYER_RGRGGBGB,
		"_raw16_rggb.pgm"
	},
	{
		PIC_BUF_TYPE_RAW16,
		PIC_BUF_LAYOUT_BAYER_GRGRBGBG,
		"_raw16_grbg.pgm"
	},
	{
		PIC_BUF_TYPE_RAW16,
		PIC_BUF_LAYOUT_BAYER_GBGBRGRG,
		"_raw16_gbrg.pgm"
	},
	{
		PIC_BUF_TYPE_RAW16,
		PIC_BUF_LAYOUT_BAYER_BGBGGRGR,
		"_raw16_bggr.pgm"
	},
	{
		PIC_BUF_TYPE_JPEG,
		PIC_BUF_LAYOUT_COMBINED,
		".jpg"
	},
	{
		PIC_BUF_TYPE_RGB888,
		PIC_BUF_LAYOUT_COMBINED,
		"_rgb888.ppm"
	},
	{
		PIC_BUF_TYPE_RGB666,
		PIC_BUF_LAYOUT_COMBINED,
		"_rgb666.ppm"
	},
	{
		PIC_BUF_TYPE_RGB565,
		PIC_BUF_LAYOUT_COMBINED,
		"_rgb565.ppm"
	},
	{
		PIC_BUF_TYPE_YCbCr444,
		PIC_BUF_LAYOUT_COMBINED,
		"_yuv444"
	},
	{
		PIC_BUF_TYPE_YCbCr444,
		PIC_BUF_LAYOUT_PLANAR,
		"_yuv444_plan"
	},
	{
		PIC_BUF_TYPE_YCbCr422,
		PIC_BUF_LAYOUT_COMBINED,
		"_yuv422.yuyv"
	},
	{
		PIC_BUF_TYPE_YCbCr422,
		PIC_BUF_LAYOUT_SEMIPLANAR,
		"_yuv422_semi.lpi422h"
	},
	{
		PIC_BUF_TYPE_YCbCr422,
		PIC_BUF_LAYOUT_PLANAR,
		"_yuv422_plan.i422h"
	},
	{
		PIC_BUF_TYPE_YCbCr420,
		PIC_BUF_LAYOUT_SEMIPLANAR,
		"_yuv420_semi.nv12"
	},
	{
		PIC_BUF_TYPE_YCbCr420,
		PIC_BUF_LAYOUT_PLANAR,
		"_yuv420_plan.i420"
	},
};

static isp_dbg_ctrl_t g_isp_dbg_ctrl = { 0 };

static isp_dbg_ctrl_t *get_isp_dbg_ctrl(void)
{
	return &g_isp_dbg_ctrl;
}

dbg_save_mode_uint8 get_dbg_save_mode(void)
{
	return g_isp_dbg_ctrl.save_image.mode;
}

/*
 * get_pic_fmt_desc() - Store file's extension
 *
 * @type:   picture format type
 * @layout: picture format layout
 *
 * @return: file's extension
 */
INLINE const char *get_pic_fmt_desc(uint8_t type, uint8_t layout)
{
	uint8_t idx = 0;

	for (; idx < ARRAY_SIZE(g_pic_fmt_desc_table); idx++) {
		if ((type == g_pic_fmt_desc_table[idx].type) &&
				(layout == g_pic_fmt_desc_table[idx].layout))
			return g_pic_fmt_desc_table[idx].description;
	}

	return ".raw";
}

/*
 * dbg_get_save_mode_desc() - Store file's extension(unsafe)
 *
 * @en:     DBG_SAVE_FRAME_MODE
 * @table:  table
 *
 * @return: enum extension
 */
const char *dbg_get_save_mode_desc(uint8_t en)
{
	//tmp: reutrn directily
	return g_dbg_save_mode_desc_table[en].description;
}

/*
 * save_image_file_name() - Set save image file's name
 *
 * @t:      time stamp
 * @path:   main|self path
 * @idx:    <idx>nd frame
 * @type:   picture format type
 * @layout: picture format layout
 * @nam:    return value, save image file's name
 */
INLINE int save_image_file_name(time_t t, uint8_t path, uint16_t idx,
		uint8_t type, uint8_t layout, char nam[])
{
	char folder_nam[64];
	struct tm *lt;
	char *file_name = (char*)get_pic_fmt_desc(type, layout);

	if (!t)
		time(&t);

	lt = localtime(&t);

	sprintf(folder_nam, DBG_SAVE_FOLDER_NAME"/isp%02d%02d%02d",
			lt->tm_year % 100, lt->tm_mon, lt->tm_mday);

	if (mkdir(folder_nam, 0644) && (errno != EEXIST)) {
		TRACE(ISP_DBG_ERR, "Create %s failed[%d:%s]\n",
			folder_nam, errno, strerror(errno));
		return -1;
	}

	sprintf(nam, "%s/path%d_%02d%02d%02d_%d%s", folder_nam, path,
			lt->tm_hour, lt->tm_min, lt->tm_sec, idx, file_name);

	return 0;
}

/*
 * prepare_save_image_process() - Get save image file's id
 *
 * @path:   main|self path
 * @idx:    <idx>nd frame
 * @type:   picture format type
 * @layout: picture format layout
 *
 * @return: save image file's id
 */
static int prepare_save_image_process(uint8_t path, uint32_t idx, uint8_t type,
		uint8_t layout)
{
	char nam[128];

	if (save_image_file_name(0, path, idx++, type, layout, nam)) {
		TRACE(ISP_DBG_ERR, "create save image file failed, time[%lu] "
				"path[%d] id[%d] mode[%d] layout[%d] nam[%s]\n",
				0, path, idx, type, layout, nam);
		return -1;
	}

	return open(nam, O_CREAT | O_WRONLY);
}

/*
 * dbg_close_save_frame() - Close save image
 *
 * @void
 */
static int dbg_close_save_frame(void)
{
	save_image_t *psave_image = &g_isp_dbg_ctrl.save_image;
	int ret = 0;

	if (psave_image->fd > 0)
		ret = close(psave_image->fd);

	MEMSET(psave_image, 0, sizeof(save_image_t));
	TRACE(ISP_DBG_WARN, "close save image\n");
	return ret;
}

/*
 * isp_store_buff_process() - Get save image file's id
 *
 * @pdata:  save image struct
 */
#define STORE_BUF_DEBUG_FREQ        1
INLINE void isp_store_buff_process(isp_dbg_cb_buffer_t *pdata)
{
	static uint32_t idx = 0;
	save_image_t *psave_image = &g_isp_dbg_ctrl.save_image;
	uint8_t *pbuf = HAL_MEMORY(pdata->pdata->pbuf);
	uint32_t size = pdata->pdata->height * pdata->pdata->widthbytes;
	ssize_t ret = -2;

	if (psave_image->mode == DBG_SAVE_MODE_PREVIEW) {
		ret = write(psave_image->fd, pbuf, size);
	} else {//DBG_SAVE_MODE_CAPTURE || DBG_SAVE_CAMIF_RAW
		int fd = prepare_save_image_process(pdata->path, idx,
				pdata->type, pdata->layout);

		if (fd > 0) {
			ret = write(fd, pbuf, size);
			ret |= close(fd);
		}
	}

	if (ret < 0)
		TRACE(ISP_DBG_ERR, "save image err,ret=%d[%d,%s]",
			ret, errno, strerror(errno));

	if (!(idx % STORE_BUF_DEBUG_FREQ))
		TRACE(ISP_DBG_DBG, "save image id[%d] size[%d] write[%d]!\n",
				idx, size, ret);

	if (psave_image->frame_num && (++idx == psave_image->frame_num)) {
		idx = 0;
		dbg_close_save_frame();
	}
}


/*
 * isp_store_buff() - Start store buff to file
 *
 * @pdata:  save image struct
 */
void isp_store_buff(isp_dbg_cb_buffer_t *pdata)
{
	save_image_t *psave_image = &g_isp_dbg_ctrl.save_image;

	/* skip frames that do not need to be saved */
	if (psave_image->first_frame > 0) {
		psave_image->first_frame--;
	} else {
		psave_image->first_frame = psave_image->intval;
		isp_store_buff_process(pdata);
	}
}

/*
 * get_isp_fps() - Show frame per second
 *
 * @void
 */
#define IMI_CAMIF_FPS_MAX_BUF_LEN 300
void get_isp_fps(void)
{
	if (!g_isp_dbg_ctrl.fps.on)
		return;

	if (g_isp_dbg_ctrl.fps.print_num) {
		TRACE(ISP_DBG_WARN, "fps=%d\n", g_isp_sensor_fps);
		if (!(--g_isp_dbg_ctrl.fps.print_num)) {
			g_isp_dbg_ctrl.fps.on = BOOL_FALSE;
		}
	} else {
		char buf[IMI_CAMIF_FPS_MAX_BUF_LEN];
		unsigned char i = IMI_CAMIF_FPS_ARR_LEN;
		uint32_t *p = g_isp_sensor_fps;
		uint32_t sum = 0;
		uint32_t len = 0;
		int fd = open("/usr/lib/camfps", O_CREAT | O_WRONLY);

		if(fd < 0) {
			TRACE(ISP_DBG_ERR, "open fps file failed\n");
			return ;
		}

		MEMSET(buf, '\0', IMI_CAMIF_FPS_MAX_BUF_LEN);

		do {
			sum += *p;
			len += sprintf(buf + len, "%u\n", *p++);
		} while (--i);

		sprintf(buf + len, "ave:%u\n", sum / IMI_CAMIF_FPS_ARR_LEN);

		/* clear the old info, O_TRUNC cannot reach the goal */
		write(fd, buf, IMI_CAMIF_FPS_MAX_BUF_LEN);

		close(fd);
	}
}

/*
 * parse_cmd_set_trace_level() - Set trace level
 *
 * @level:  trace_level, no need to protect input param
 */
static void parse_cmd_set_trace_level(uint8_t level)
{
	SET_TRACE_LEVEL(level);
	TRACE(ISP_DBG_WARN, "set trace level %d\n", level);
}

/*
 * parse_cmd_get_fps() - Show frame per second
 *
 * @arg:    optarg
 */
static void parse_cmd_get_fps(const char *arg)
{
	if (strncmp(ISP_DEBUG_FPS_CMD, arg, ISP_DEBUG_FPS_CMD_LEN)) {
		g_isp_dbg_ctrl.fps.on = atoi(arg) & 1;
	}
	else {
		const char *p = &arg[ISP_DEBUG_FPS_CMD_LEN];
		g_isp_dbg_ctrl.fps.print_num = atoi(p);
	}
	TRACE(ISP_DBG_WARN, "fps is %d\n", g_isp_dbg_ctrl.fps.on);
}

/*
 * close_all_debug() - Close all debug when change path
 *
 * @void
 */
static void close_all_debug(void)
{
	dbg_close_save_frame();
	TRACE(ISP_DBG_WARN, "not support close all debug\n");
}

/*
 * free_path_safe() - Free path
 *
 * @ppath:      path
 */
static void free_path_safe(cam_path_cfg_t *ppath) {
	if (ppath && (ppath != &g_cam_path_disabled) &&
			(ppath != &g_default_path_preview))
		free(ppath);
}

/*
 * cmd_set_format_porocess() - Set format
 *
 * @ppath:  path
 * @mode:   [0:DFLT path value, !0:picture format]
 */
static int set_format_porocess(uint8_t path, int mode)
{
	struct sensor_params *psensor_param = get_sensor_param();
	cam_path_cfg_t *ppath = psensor_param->ppath_preview[path];

	if (mode) {
		uint8_t format = mode % 100;
		uint8_t layout;
		//uint16_t resolution;

		if (!ppath || (ppath == &g_cam_path_disabled) ||
				(ppath == &g_default_path_preview)) {
			ppath = (cam_path_cfg_t*)malloc(sizeof(cam_path_cfg_t));
			MEMCPY(ppath, &g_default_path_preview,
					sizeof(cam_path_cfg_t));
			psensor_param->ppath_preview[path] = ppath;
		}

		mode /= 100;
		layout = mode % 100;
		//resolution = arg / 100;

		if (layout)
			ppath->layout = (cam_midata_layout_t)layout;

		if (format)
			ppath->mode = (cam_midata_mode_t)format;
	} else {
		free_path_safe(ppath);
		ppath = (cam_path_cfg_t *)&g_default_path_preview;
	}

	if (psensor_param->ppath_preview[!path] != &g_cam_path_disabled) {
		free_path_safe(psensor_param->ppath_preview[!path]);
		psensor_param->ppath_preview[!path] =
				(cam_path_cfg_t *)&g_cam_path_disabled;
		close_all_debug();
	}

	TRACE(ISP_DBG_WARN, "set path[%d=%d], layout[%d], mode[%d]\n",
			path, mode, ppath->layout, ppath->mode);
	return 0;
}

/*
 * parse_cmd_set_format() - Set isp output picture format
 *
 * @arg:    optarg
 */
static int parse_cmd_set_format(const char *arg)
{
	uint8_t path = CAMERIC_MI_PATH_INVALID;

	if (arg[0] == 's')
		path = CAMERIC_MI_PATH_SELF;
	else //other param as main path
		path = CAMERIC_MI_PATH_MAIN;

	return set_format_porocess(path, atoi(&arg[1]));
}

/*
 * dbg_set_save_image_num() - Save image to file param
 *
 * @num:    save frame num
 */
static int dbg_set_save_frame_num(uint16_t num)
{
	save_image_t *psave_image = &g_isp_dbg_ctrl.save_image;

	psave_image->frame_num = num;

	TRACE(ISP_DBG_WARN, "set save image num=%d cur_mode=%s\n",
			psave_image->frame_num,
			dbg_get_save_mode_desc(psave_image->mode));
	return 0;
}

/*
 * prepare_save_image_video() - Save video config before start
 *
 * @void
 */
static int prepare_save_image_video(void)
{
	static uint32_t idx = 0;
	save_image_t *psave_image = &g_isp_dbg_ctrl.save_image;
	struct sensor_params *psensor_param = get_sensor_param();
	int8_t path = get_opened_path();
	cam_path_cfg_t *ppath = psensor_param->ppath_preview[path];

	psave_image->fd = prepare_save_image_process(path, idx, ppath->mode,
			ppath->layout);

	return psave_image->fd <= 0;
}

/*
 * camif_cb_buffer() - Start camif callback
 *
 * @void
 */
static void camif_cb_buffer(void)
{
	isp_dbg_cb_buffer_t arg;

	arg.pdata = g_isp_dbg_ctrl.camif;
	arg.path = CAM_ENGINE_PATH_MAX;

	isp_store_buff(&arg);
}

/*
 * camif_start() - Start camif
 *
 * @void
 */
void camif_start(void)
{
	int timeout = 1000;
	isp_dbg_ctrl_t *pisp_dbg_ctrl = get_isp_dbg_ctrl();
	imi_isp_ctrl *pisp_ctrl = get_isp_ctrl();

	while((pisp_ctrl->sensor_stream_state != IBINDER_SENSOR_STREAM_OFF) &&
			(pisp_dbg_ctrl->save_image.mode ==
			DBG_SAVE_CAMIF_RAW) && (--timeout > 0)) {
		if(g_camif_drv.pfn_irq()) {
			camif_cb_buffer();
			g_camif_drv.pfn_irq_reset();
			timeout = 1000;
		} else if (pisp_dbg_ctrl && pisp_dbg_ctrl->save_image.mode ==
				DBG_SAVE_CAMIF_RAW)
			osSleep(10);
	}

	TRACE(ISP_DBG_WARN, "camif stoped. interupt_occur_timeout[%d], "
		"stream_on[%d], camif_on[%d==%d]\n",
		timeout, pisp_ctrl->sensor_stream_state,
		pisp_dbg_ctrl->save_image.mode, DBG_SAVE_CAMIF_RAW);
}

/*
 * camif_alloc() - Config camif
 *
 * @handle:
 */
int camif_alloc(HalHandle_t handle)
{
	CamEnginePathConfig_t  *ppath;
	uint32_t                buf = 0;
	pic_buf_plane_t        *camif = NULL;

	if (g_camif_drv.pfn_base_addr_map())
		goto err;

	buf = HalAllocMemory(handle, CAMIF_BUF_NUM * CAMIF_BUF_SIZE);
	if (!buf)
		goto err;

	ppath = get_sensor_param()->ppath_preview[get_opened_path()];

	camif = (pic_buf_plane_t *)malloc(sizeof(pic_buf_plane_t));
	if (!camif)
		goto err;

	camif->pbuf = (uint8_t *)buf;
	camif->height = ppath->height;
	camif->widthbytes = ppath->width * 2;

	g_camif_drv.pfn_config(buf, camif->height, ppath->width);
	g_isp_dbg_ctrl.camif = camif;

	return 0;
err:
	TRACE(ISP_DBG_ERR, "alloc buf[0x%x] base_addr[0x%x]\n",
			buf, camif);
	return -1;
}

/*
 * camif_release_safe() - Release camif
 *
 * @handle:
 */
void camif_release_safe(HalHandle_t handle)
{
	isp_dbg_ctrl_t *pisp_dbg_ctrl = get_isp_dbg_ctrl();

	if (pisp_dbg_ctrl->camif->pbuf) {
		HalFreeMemory(handle, (uint32_t)pisp_dbg_ctrl->camif->pbuf);
		MEMSET(pisp_dbg_ctrl->camif, 0, sizeof(pic_buf_plane_t));
		free(pisp_dbg_ctrl->camif);
	}
}

/*
 * isp_start_save_frame() - Start save frame
 *
 * @arg:    optarg
 */
static int isp_start_save_frame(dbg_save_mode_uint8 save_mode)
{
	save_image_t *psave_image = &g_isp_dbg_ctrl.save_image;
	CamEnginePathConfig_t  *ppath =
			get_sensor_param()->ppath_preview[CAMERIC_MI_PATH_MAIN];

	if (psave_image->mode) {
		TRACE(ISP_DBG_ERR, "Please close save frame first\n");
		return 1;
	}

	/* create save frame path */
	if (mkdir(DBG_SAVE_FOLDER_NAME, 0644) && (errno != EEXIST)) {
		TRACE(ISP_DBG_ERR, "Start save to %s failed.[%d:%s]\n",
			DBG_SAVE_FOLDER_NAME, errno, strerror(errno));
		return 2;
	}

	switch (save_mode) {
	case DBG_SAVE_MODE_CLOSE:
		return dbg_close_save_frame();
	case DBG_SAVE_MODE_CAPTURE:
		break;
	case DBG_SAVE_MODE_PREVIEW:
		/* save preview unsupport raw format */
		if ((ppath->mode < CAMERIC_MI_DATAMODE_YUV444) ||
				(ppath->mode > CAMERIC_MI_DATAMODE_YUV400)) {
			TRACE(ISP_DBG_ERR, "Preview unsupport raw format\n");
			return 3;
		}

		if (prepare_save_image_video()) {
			return 4;
		}
		break;
	case DBG_SAVE_CAMIF_RAW:
		camif_start();
		break;
	default:
		TRACE(ISP_DBG_ERR, "Save frame mode %d must be less than %d\n",
			save_mode, DBG_SAVE_MODE_BUTT);
		return 5;
	}

	psave_image->mode = save_mode;

	TRACE(ISP_DBG_WARN, "Save frame thread %s start\n",
			dbg_get_save_mode_desc(save_mode));

	return 0;
}

/*
 * usage() - show debug help
 *
 * @argv0:
 */
static void usage(const char *argv0)
{
	fprintf(stderr, "Usage: %s [options]\n", argv0);

	/* (~((uint32_t)level - 1u)) & MAX_LEVEL */
	fprintf(stderr, " -l: set trace level=<param>."
		" [0:off all, 1:on all, 2:ERR and WARN, 4:ERR]\n");

	/* -p:printN -- print directly, auto turn off after <n> frame */
	fprintf(stderr, " -p: get fps interval. [0:turn off, 1:turn on]\n");

	/* >100 part = lqyout, eg. -s:104 -- planar & YUV422 */
	fprintf(stderr, " -f: set picture format:\n"
		"\t4--YUV422\n\t5--YUV420\n\t10--RAW10\n\t11--RAW12\n");

	fprintf(stderr, " -n: save frames num. [0~65535]\n");

	fprintf(stderr, " -s: save frames. [0:turn off, 1:capture, 2:preview]"
		"\t'-n' can let it trun off automatic\n");

	fprintf(stderr, " -t: start stream without imi_app. "
		"[0:turn off, 1:turn on]\n");

	fprintf(stderr, "\n Cmd num must be less than %d and each cmd's "
		"param len must be less than %d\n",
		ISP_DBG_MAX_CMD_NUM, ISP_DBG_MAX_LEN_PER_CMD - 1);
}

/*
 * start_stream() - Start or close stream without imi_app
 *
 * @on:     open|close stream
 */
static void start_stream(uint8_t on)
{
	struct ibinder_event event;
	int fd = open("/dev/ibinder", O_RDWR);

	if (fd > 0) {
		event.header.src_id = IBINDER_EVENT_APP;
		event.header.dst_id = IBINDER_EVENT_ISP;
		event.header.id = IBINDER_SENSOR_SET_CTL;
		event.data = (unsigned long)&on;
		event.header.len = sizeof(unsigned char);

		ioctl(fd, IBINDER_EVENT_SET, &event);
		close(fd);
	}
}

/*
 * isp_parse_cmd() - Parse cmd
 *
 * @argc:   cmd num
 * @argv:   cmd array
 */
int isp_parse_cmd(int argc, char *argv[])
{
	int opt;

	while((opt = getopt(argc, argv, "l:p:f:s:n:t:h")) != -1) {
		TRACE(ISP_DBG_DBG, "parse cmd -%c,%s\n", opt,
				optarg ? : "NULL");
		switch(opt) {
		case 'l':
			parse_cmd_set_trace_level(atoi(optarg));
			break;
		case 'p':
			parse_cmd_get_fps(optarg);
			break;
		case 'f':
			if (parse_cmd_set_format(optarg))
				goto err;
			break;
		case 's':
			if (isp_start_save_frame(atoi(optarg)))
				goto err;
			break;
		case 'n':
			if (dbg_set_save_frame_num(atoi(optarg)))
				goto err;
			break;
		case 't':
			start_stream(atoi(optarg));
			break;
		case 'h':
			usage(argv[0]);
			return 1;
		default:
			goto err;
		}
	}

	return 0;

err:
	TRACE(ISP_DBG_ERR, "invalid -%c, use '-h' to show help menu\n", opt);
	return -1;
}

/*
 * close_debug_fifo_safe() - Close cmd fifo
 *
 * @void
 */
static void close_debug_fifo_safe(void)
{
	if (g_isp_dbg_ctrl.fifo > 0) {
		close(g_isp_dbg_ctrl.fifo);
		g_isp_dbg_ctrl.fifo = 0;
		unlink(DBG_FIFO_NAME);
	}
}

/*
 * debug_thread_off() - Close cmd thread
 *
 * @void
 */
int debug_thread_off(void)
{
	close_debug_fifo_safe();
	TRACE(ISP_DBG_WARN, "recv cmd thread closed\n");
	return 0;
}

/*
 * open_debug_fifo() - Open cmd fifo
 *
 * @priv:   NULL
 */
void *open_debug_fifo(void *priv)
{
	isp_dbg_ctrl_t *pisp_dbg_ctrl = &g_isp_dbg_ctrl;
	char buf[ISP_DBG_MAX_LEN_CMD];
	int argc;
	char *argv[ISP_DBG_MAX_CMD_NUM];
	int n;

	if (mkfifo(DBG_FIFO_NAME, 0644) && (errno != EEXIST)) {
		TRACE(ISP_DBG_ERR, "Create fifo %s failed.[%d:%s]\n",
			DBG_FIFO_NAME, errno, strerror(errno));
		goto exit;
	}

	pisp_dbg_ctrl->fifo = open(DBG_FIFO_NAME, O_RDWR);
	if (pisp_dbg_ctrl->fifo <= 0) {
		TRACE(ISP_DBG_ERR, "Open fifo %s failed.[%d:%s]\n",
			DBG_FIFO_NAME, errno, strerror(errno));
		goto exit;
	}

	/* it will not exit until fifo closed */
	while ((argc = read(pisp_dbg_ctrl->fifo, buf, ISP_DBG_MAX_LEN_CMD))) {
		if (argc < ISP_DBG_MAX_LEN_PER_CMD) {
			TRACE(ISP_DBG_ERR, "Please use isp_debug "
					"app to input cmd\n");
			continue;
		}

		argc /= ISP_DBG_MAX_LEN_PER_CMD;
		for (n = 0; n < argc; n++) {
			argv[n] = &buf[n * ISP_DBG_MAX_LEN_PER_CMD];

			/* each cmd must be ended with '\0' */
			argv[n][ISP_DBG_MAX_LEN_PER_CMD - 1] = '\0';
		}

		optind = 0;
		isp_parse_cmd(n, argv);
	}

	debug_thread_off();

exit:
	return 0;
}

#ifdef __cpluscplus
}
#endif

