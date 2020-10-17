#ifndef __CM_INTERFACE_H__
#define __CM_INTERFACE_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "isi.h"
#include "isi_iss.h"
#include <sys/ioctl.h>
#include <imi-binder.h>
#include "mipi_dphy.h"

#define ARRAY_SIZE(arr)         (sizeof(arr) / sizeof((arr)[0]))
#define HAL_MEMORY(buf) (extern_mem_virtual_base + ((uint64_t)(buf) - \
		extern_mem_base))

typedef struct mipi_param_s {
   unsigned int mipi_power_on; //1: power on sensor, 0: dont execute power on function
   unsigned int mipi_power_off;//1: power off sensor when function done. 0: dont execute power off function
    unsigned int isp_src_sel; //(isp_src_sel   ) 0:mipi-0, 1:mipi-1; 2:mipi-2, 3:dvp
    unsigned int sel_ov5695 ; //1: select ov5695, 0: select ov5648. (valid only when isp_src_sel=0)
    unsigned int mipi_dt    ; //(mipi_data_type) 0x2a: 8-bit mode, 0x2b:10-bit, 0x2c:12-bit
    unsigned int mipi_lanes ; //(mipi_lane_num ) 1: use 1-MIPI-Lane, 2:use 2-MIPI-Lane
    unsigned int mipi_mbps  ; //(mipi_mbps     ), in unit of MBps, eg: mipi_mbps=400 means 400MBPS per MIPI-Lane.
    unsigned int img_width  ; //image horizontal size in pixel
    unsigned int img_height ; //image vertical size in pixel
}camsys_mipi_param_t;
#define CAMSYS_MIPI_CFG		_IOR('M',  7, camsys_mipi_param_t)

typedef struct sensor_params {
	const char             *libname;
	//not support slave
	CamEnginePathConfig_t  *ppath_preview[CAMERIC_MI_PATH_MAX];
	const uint16_t          mipi_clk;
	const bool_t            mipi_enable;
	uint8_t                 resv;
}SensorParams_t;

typedef struct SomSetting_s
{
	/* SomCtrl. */
	struct Som_s {
		int enable;
	} som;
} SomSetting_t;

typedef struct IspSensorIF_s
{
	HalHandle_t hHal;
	void* hSensorLib;
	IsiCamDrvConfig_t *pCamDrvConfig;
	IsiSensorHandle_t hSensor;
	IsiSensorCaps_t Caps;
	IsiSensorConfig_t sensorConfig;
//	void* hMipiLib;
//	ImiMipiDrvConfig_t *pImiMipiDrvConfig;
}IspSensorIF_t;


typedef struct IspCameraEngineText_s
{
	HalHandle_t hHal;
	CamEngineHandle_t hCamEngine;
	CamEngineInstanceConfig_t camInstanceConfig;
	CamEngineConfig_t camEngineConfig;

	std::string sensorDrvFile;
	std::string calibDbFile;
	CalibDb calibDb;

	osEvent eventEngineStarted;
	osEvent eventEngineStop;
	osEvent eventEngineCaptureDone;

	osEvent eventStartStreaming;
	osEvent eventStopStreaming;

	somCtrlHandle_t hSomCtrl;
	osEvent eventSomStarted;
	osEvent eventSomStop;
	osEvent eventSomFinished;

	SomSetting_t SomSetting;


}IspCameraEngineText_t;

struct imi_isp_ctrl {
	struct ibinder_sensor_format fmt;
	struct ibinder_sensor_param_info param;
	struct ibinder_sensor_reg_info reg;
	pthread_t stream_on_thd;
	pthread_mutex_t mutex;
	int sensor_stream_ack;
	int fd_addr;
	int ctrl_state;
	unsigned char sensor_stream_state;
	unsigned char isp_ctl;
	sigset_t set;
};

union sensor_info {
	unsigned char isp_ctl;
	struct ibinder_sensor_reg_info reg;
	struct ibinder_sensor_format fmt;
	struct ibinder_sensor_param_info param;
};

extern const CamEnginePathConfig_t  g_cam_path_disabled;
extern const CamEnginePathConfig_t  g_default_path_preview;

extern int      default_config(int argc, char **argv);
extern int8_t   get_opened_path(void);

extern SensorParams_t  *get_sensor_param(void);
extern imi_isp_ctrl    *get_isp_ctrl(void);

extern camsys_mipi_param_t g_mipi_param;

#ifdef __cplusplus
}
#endif

#endif
