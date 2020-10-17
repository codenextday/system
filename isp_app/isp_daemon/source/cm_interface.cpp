#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <ebase/types.h>
#include <ebase/trace.h>
#include <ebase/builtins.h>
#include <embUnit/embUnit.h>

#include <hal/hal_api.h>
#include <oslayer/oslayer.h>
#include <version/version.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <common/return_codes.h>
#include <common/picture_buffer.h>
#include <dlfcn.h>

#include "isi.h"
#include "isi_iss.h"
#include "isi_common.h"

#include <cam_engine/cam_engine_api.h>
#include <cam_engine/cam_engine_aaa_api.h>
#include <cam_engine/cam_engine_mi_api.h>
#include <cam_engine/cam_engine_jpe_api.h>
#include <cam_engine/cam_engine_cproc_api.h>
#include <cam_engine/cam_engine_isp_api.h>
#include <cam_engine/cam_engine_imgeffects_api.h>

#include <som_ctrl/som_ctrl_api.h>
#include <fpga/altera_fpga.h>
#include "calibdb.h"
#include "cm_interface.h"
#include "mapcaps.h"
#include "cm_debug.h"

/*********************************************************
* local macro definitions
*********************************************************/
CREATE_TRACER(ISP_TEST_INFO,    "ISP_TEST_INFO ",   INFO,       0);
CREATE_TRACER(ISP_TEST_WARN,    "ISP_TEST_WARN ",   WARNING,    1);
CREATE_TRACER(ISP_TEST_DEBUG,   "ISP_TEST_DBG ",    INFO,       1);
CREATE_TRACER(ISP_TEST_ERROR,   "ISP_TEST_ERR ",    ERROR,      1);

#define ISP_ADDR_PIC    "/dev/ibinder"
#define SOMCTRL_ENABLE  0
#define CAM_API_CAMENGINE_TIMEOUT 30000U // 30 s

#define CAMENGINE_STATE 0

IspSensorIF_t  SensorIf;
IspCameraEngineText_t CamEngText;

HalHandle_t HalHandle = NULL;

//static unsigned int mipi_phy_power_on(struct Imi_ImageConfig_s *p);

static void cbBuffer(CamEnginePathType_t path, MediaBuffer_t *pMediaBuffer, void *pBufferCbCtx);
static void cbSomCtrl(somCtrlCmdID_t cmdId, RESULT result,somCtrlCmdParams_t *pParams,
						  somCtrlCompletionInfo_t *pInfo, void *pUserContext);
static void prepare_hal(void);
static void release_hal(void);
static void dumpSensorCapabilityandconfig( IsiSensorCaps_t *pCaps,  IsiSensorCaps_t *pConfig);
static void CloseSensor(void);
static void SensorStreamOn(void);
static void SensorStreamOff(void);

static void cbCompletion( CamEngineCmdId_t cmdId, RESULT result, const void* pUserCtx );
static void ConfigCamEnginePara();
static void InitCameraIC();
static bool SetupCameraIC();
static void DoneCameraIC();

static bool startAec(void);
static bool startAwb(const CamEngineAwbMode_t &mode, uint32_t idx, const bool_t damp);
static bool startAdpf(void);
static bool startAdpcc(void);
static bool startAvs(void);

static bool stopAec(void);
static bool stopAwb(void);
static bool stopAdpf(void);
static bool stopAdpcc(void);
static bool stopAvs(void);

static struct imi_isp_ctrl gisp_ctrl;

imi_isp_ctrl *get_isp_ctrl(void)
{
	return &gisp_ctrl;
}

INLINE void cbBufferSaveImageProcess(CamEnginePathType_t path, uint16_t width,
		PicBufMetaData_t *pPicBufMetaData)
{
	isp_dbg_cb_buffer_t arg;
	pic_buf_plane_t    data;

	data.pbuf = pPicBufMetaData->Data.YCbCr.combined.pBuffer;
	data.height = pPicBufMetaData->Data.YCbCr.combined.PicHeightPixel;
	data.widthbytes = width;

	arg.pdata = &data;
	arg.path = path;
	arg.type = pPicBufMetaData->Type;
	arg.layout = pPicBufMetaData->Layout;

	isp_store_buff(&arg);
}

INLINE void cbBufferSaveImage(CamEnginePathType_t path, uint16_t width,
		PicBufMetaData_t *pPicBufMetaData)
{
	dbg_save_mode_uint8 mode = get_dbg_save_mode();

	if ((mode < DBG_SAVE_MODE_CAPTURE) || (mode > DBG_SAVE_MODE_PREVIEW))
		return;

	cbBufferSaveImageProcess(path, width, pPicBufMetaData);
}

static void cbBuffer(CamEnginePathType_t path,
		MediaBuffer_t *pMediaBuffer, void *pBufferCbCtx)
{
	IspCameraEngineText_t *pCtx = (IspCameraEngineText_t *)pBufferCbCtx;
	PicBufMetaData_t *pPicBufMetaData =
			( PicBufMetaData_t *)(pMediaBuffer->pMetaData);
	struct imi_isp_ctrl *ctrl = &gisp_ctrl;
	struct ibinder_buf_set bset;
	int width = pPicBufMetaData->Data.YCbCr.planar.Y.PicWidthBytes +
			pPicBufMetaData->Data.YCbCr.planar.Cb.PicWidthBytes +
			pPicBufMetaData->Data.YCbCr.planar.Cr.PicWidthBytes;

	TRACE(ISP_TEST_INFO, "%s (enter)\n", __FUNCTION__);

	bset.buf.phy = (uint64_t)pPicBufMetaData->Data.YCbCr.combined.pBuffer;
	bset.buf.size = pPicBufMetaData->Data.YCbCr.combined.PicHeightPixel *
			width;
	bset.stream_id = IBINDER_STREAM_ISP_YUV;
	//bset.buf.callback = IBINDER_EVENT_ISP;

	TRACE(ISP_TEST_DEBUG, "%s %llu:addr=0x%lx size=0x%lx time=%lld\n",
			__FUNCTION__, osGetTick(), bset.buf.phy, bset.buf.size,
			pPicBufMetaData->TimeStampUs);

	if (ioctl(ctrl->fd_addr, IBINDER_BUF_SET, &bset) < 0) {
		TRACE(ISP_TEST_ERROR, "IBINDER_BUF_SET failed %d:%s\n",
				errno, strerror(errno));
		return;
	}

	get_isp_fps();
	cbBufferSaveImage(path, width, pPicBufMetaData);

	if (pCtx->SomSetting.som.enable) {
		TRACE(ISP_TEST_ERROR, "pCtx->SomSetting.som.enable \n");
		somCtrlStoreBuffer(pCtx->hSomCtrl, pMediaBuffer);
	}
}

static void cbSomCtrl(somCtrlCmdID_t cmdId, RESULT result, somCtrlCmdParams_t *pParams,
		somCtrlCompletionInfo_t *pInfo, void *pUserContext)
{
	TRACE(ISP_TEST_INFO, "%s (enter)\n", __FUNCTION__);
	IspCameraEngineText_t *pCtx = (IspCameraEngineText_t *)pUserContext;

	if (pCtx != NULL) {
		switch (cmdId) {
		case SOM_CTRL_CMD_START:
			if (RET_PENDING == result) {
				osEventSignal(&pCtx->eventSomStarted);
			} else {
				osEventSignal(&pCtx->eventSomFinished);
			}
		break;

		case SOM_CTRL_CMD_STOP:
			osEventSignal(&pCtx->eventSomStop);
			break;

		default:
			TRACE(ISP_TEST_ERROR, "%s (invalid cmd)\n", __FUNCTION__);
			break;
		}
	}

	TRACE(ISP_TEST_INFO, "%s (exit)\n", __FUNCTION__);
}

static void prepare_hal(void)
{
	TRACE(ISP_TEST_INFO, "%s (enter)\n", __FUNCTION__);
	/* prepare hal */
	HalHandle = HalOpen();

	SensorIf.hHal = HalHandle;
	CamEngText.hHal = HalHandle;

	TRACE(ISP_TEST_INFO, "%s (exit)\n", __FUNCTION__);
	return;
}

static void release_hal(void)
{
	TRACE(ISP_TEST_INFO, "%s (enter)\n", __FUNCTION__);

	HalClose(HalHandle);
	HalHandle = NULL;

	SensorIf.hHal = NULL;
	CamEngText.hHal = NULL;

	TRACE(ISP_TEST_INFO, "%s (exit)\n", __FUNCTION__);
	return;
}

static void dumpSensorCapabilityandconfig( IsiSensorCaps_t *pCaps,
		IsiSensorCaps_t *pConfig)
{
	TRACE(ISP_TEST_INFO, "%s (enter)\n", __FUNCTION__);

	TRACE(ISP_TEST_DEBUG, "Caps.BusWidth        : 0x%08x \n", pCaps->BusWidth         );
	TRACE(ISP_TEST_DEBUG, "Caps.Mode            : 0x%08x \n", pCaps->Mode             );
	TRACE(ISP_TEST_DEBUG, "Caps.FieldSelection  : 0x%08x \n", pCaps->FieldSelection   );
	TRACE(ISP_TEST_DEBUG, "Caps.YCSequence      : 0x%08x \n", pCaps->YCSequence       );
	TRACE(ISP_TEST_DEBUG, "Caps.Conv422         : 0x%08x \n", pCaps->Conv422          );
	TRACE(ISP_TEST_DEBUG, "Caps.BPat            : 0x%08x \n", pCaps->BPat             );
	TRACE(ISP_TEST_DEBUG, "Caps.HPol            : 0x%08x \n", pCaps->HPol             );
	TRACE(ISP_TEST_DEBUG, "Caps.VPol            : 0x%08x \n", pCaps->VPol             );
	TRACE(ISP_TEST_DEBUG, "Caps.Edge            : 0x%08x \n", pCaps->Edge             );
	TRACE(ISP_TEST_DEBUG, "Caps.Bls             : 0x%08x \n", pCaps->Bls              );
	TRACE(ISP_TEST_DEBUG, "Caps.Gamma           : 0x%08x \n", pCaps->Gamma            );
	TRACE(ISP_TEST_DEBUG, "Caps.CConv           : 0x%08x \n", pCaps->CConv            );
	TRACE(ISP_TEST_DEBUG, "Caps.Resolution      : 0x%08x \n", pCaps->Resolution       );
	TRACE(ISP_TEST_DEBUG, "Caps.DwnSz           : 0x%08x \n", pCaps->DwnSz            );
	TRACE(ISP_TEST_DEBUG, "Caps.BLC             : 0x%08x \n", pCaps->BLC              );
	TRACE(ISP_TEST_DEBUG, "Caps.AGC             : 0x%08x \n", pCaps->AGC              );
	TRACE(ISP_TEST_DEBUG, "Caps.AWB             : 0x%08x \n", pCaps->AWB              );
	TRACE(ISP_TEST_DEBUG, "Caps.AEC             : 0x%08x \n", pCaps->AEC              );
	TRACE(ISP_TEST_DEBUG, "Caps.DPCC            : 0x%08x \n", pCaps->DPCC             );
	TRACE(ISP_TEST_DEBUG, "Caps.CieProfile      : 0x%08x \n", pCaps->CieProfile       );
	TRACE(ISP_TEST_DEBUG, "Caps.SmiaMode        : 0x%08x \n", pCaps->SmiaMode         );
	TRACE(ISP_TEST_DEBUG, "Caps.MipiMode        : 0x%08x \n", pCaps->MipiMode         );
	TRACE(ISP_TEST_DEBUG, "Caps.AfpsResolutions : 0x%08x \n", pCaps->AfpsResolutions  );
	TRACE(ISP_TEST_DEBUG, "pConfig.BusWidth        : 0x%08x \n", pConfig->BusWidth         );
	TRACE(ISP_TEST_DEBUG, "pConfig.Mode            : 0x%08x \n", pConfig->Mode             );
	TRACE(ISP_TEST_DEBUG, "pConfig.FieldSelection  : 0x%08x \n", pConfig->FieldSelection   );
	TRACE(ISP_TEST_DEBUG, "pConfig.YCSequence      : 0x%08x \n", pConfig->YCSequence       );
	TRACE(ISP_TEST_DEBUG, "pConfig.Conv422         : 0x%08x \n", pConfig->Conv422          );
	TRACE(ISP_TEST_DEBUG, "pConfig.BPat            : 0x%08x \n", pConfig->BPat             );
	TRACE(ISP_TEST_DEBUG, "pConfig.HPol            : 0x%08x \n", pConfig->HPol             );
	TRACE(ISP_TEST_DEBUG, "pConfig.VPol            : 0x%08x \n", pConfig->VPol             );
	TRACE(ISP_TEST_DEBUG, "pConfig.Edge            : 0x%08x \n", pConfig->Edge             );
	TRACE(ISP_TEST_DEBUG, "pConfig.Bls             : 0x%08x \n", pConfig->Bls              );
	TRACE(ISP_TEST_DEBUG, "pConfig.Gamma           : 0x%08x \n", pConfig->Gamma            );
	TRACE(ISP_TEST_DEBUG, "pConfig.CConv           : 0x%08x \n", pConfig->CConv            );
	TRACE(ISP_TEST_DEBUG, "pConfig.Resolution      : 0x%08x \n", pConfig->Resolution       );
	TRACE(ISP_TEST_DEBUG, "pConfig.DwnSz           : 0x%08x \n", pConfig->DwnSz            );
	TRACE(ISP_TEST_DEBUG, "pConfig.BLC             : 0x%08x \n", pConfig->BLC              );
	TRACE(ISP_TEST_DEBUG, "pConfig.AGC             : 0x%08x \n", pConfig->AGC              );
	TRACE(ISP_TEST_DEBUG, "pConfig.AWB             : 0x%08x \n", pConfig->AWB              );
	TRACE(ISP_TEST_DEBUG, "pConfig.AEC             : 0x%08x \n", pConfig->AEC              );
	TRACE(ISP_TEST_DEBUG, "pConfig.DPCC            : 0x%08x \n", pConfig->DPCC             );
	TRACE(ISP_TEST_DEBUG, "pConfig.CieProfile      : 0x%08x \n", pConfig->CieProfile       );
	TRACE(ISP_TEST_DEBUG, "pConfig.SmiaMode        : 0x%08x \n", pConfig->SmiaMode         );
	TRACE(ISP_TEST_DEBUG, "pConfig.MipiMode        : 0x%08x \n", pConfig->MipiMode         );
	TRACE(ISP_TEST_DEBUG, "pConfig.AfpsResolutions : 0x%08x \n", pConfig->AfpsResolutions  );

	TRACE(ISP_TEST_INFO, "%s (exit)\n", __FUNCTION__);
}

bool loadCalibrationData(const char *pFileName)
{
	TRACE(ISP_TEST_INFO, "%s (enter)\n", __FUNCTION__);

	if (NULL != pFileName) {
		// reset calibration database
		//m_pCamEngine->sensorDrvFile = std::string();
		CamEngText.calibDbFile = std::string();
		CamEngText.calibDb = CalibDb();

		CamEngText.calibDbFile = std::string(pFileName);

		QFile file (QString::fromAscii( CamEngText.calibDbFile.c_str()));
		return CamEngText.calibDb.CreateCalibDb(&file);
	}

	TRACE(ISP_TEST_INFO, "%s (exit)\n", __FUNCTION__);

	return false;
}

static RESULT OpenSensor(const char *pFileName)
{
	RESULT result = -1;
	uint32_t revId;
	IsiSensorInstanceConfig_t Config;

	TRACE(ISP_TEST_INFO, "%s (enter)\n", __FUNCTION__);

	/*
	*  Sensor Calibration data analysis conversion the Xml
	*/
	CamEngText.sensorDrvFile = std::string(pFileName);

	/* load calibration data */
	QString str = QString::fromAscii(pFileName);
	str.replace(QString(".drv"), QString(".xml"));
	if (true != loadCalibrationData(str.toAscii().constData())) {
		TRACE(ISP_TEST_ERROR, "%s (can't load the calib data: %s)\n",
			__FUNCTION__, str.toAscii().constData());
		return -1;
	}

	/* read system configuration */
	CamCalibSystemData_t systemData;
	MEMSET(&systemData, 0, sizeof(CamCalibSystemData_t));
	result = CamCalibDbGetSystemData(CamEngText.calibDb.GetCalibDbHandle(),
			&systemData);
	if (RET_SUCCESS != result) {
		TRACE(ISP_TEST_ERROR, "%s (can't read the system config: %s)\n",
			__FUNCTION__, str.toAscii().constData());
		return result;
	}

	/* load Sensor driver lib */
	SensorIf.hSensorLib = dlopen( pFileName, RTLD_LAZY);
	if (SensorIf.hSensorLib == NULL) {
		TRACE(ISP_TEST_ERROR, "%s Load module \"%s\"error:%s\n",
				__FUNCTION__, pFileName, dlerror());
		return -1;
	}
#if 0
	/* load Mipi to Dvp contorl driver lib */
	SensorIf.hMipiLib = dlopen("mipi_dphy.drv", RTLD_LAZY);
	if (SensorIf.hMipiLib == NULL) {
		TRACE(ISP_TEST_ERROR, "%s Load module mipi_dphy.drv error:%s\n",
				__FUNCTION__, dlerror());
		return -1;
	}
#endif
	/* get sensor isi func */
	SensorIf.pCamDrvConfig = (IsiCamDrvConfig_t *)dlsym(SensorIf.hSensorLib,
			"IsiCamDrvConfig" );
	if (SensorIf.pCamDrvConfig == NULL) {
		TRACE(ISP_TEST_ERROR, "%s dlsym failed \n",__FUNCTION__);
		return -1;
	}
#if 0
	/* get mipi phy func*/
	SensorIf.pImiMipiDrvConfig = (ImiMipiDrvConfig_t *)dlsym(
			SensorIf.hMipiLib, "ImiMipiDrvConfig" );
	if (SensorIf.pImiMipiDrvConfig == NULL) {
		TRACE(ISP_TEST_ERROR, "%s dlsym mipi phy fail\n", __FUNCTION__);
		return -1;
	}
#endif
	SensorIf.pCamDrvConfig->pfIsiGetSensorIss(
			&(SensorIf.pCamDrvConfig->IsiSensor));
	TRACE(ISP_TEST_DEBUG,  "Sensor Name (%s)\n",
			SensorIf.pCamDrvConfig->IsiSensor.pszName );
#if 0
	SensorIf.pImiMipiDrvConfig->pfImiGetMipiIss
			(&(SensorIf.pImiMipiDrvConfig->ImiMipiToDvpIss));
#endif
	// setup sensor instance
	MEMSET(&Config, 0, sizeof(IsiSensorInstanceConfig_t));
	Config.hSensor      = NULL;
	Config.HalHandle    = SensorIf.hHal;
	Config.pSensor      = &SensorIf.pCamDrvConfig->IsiSensor;
	Config.HalDevID     = HAL_DEVID_CAM_1;

	result = IsiCreateSensorIss(&Config);
	if (result != RET_SUCCESS){
		TRACE(ISP_TEST_ERROR, "Create sensor faild\n");
		return result;
	}

	SensorIf.hSensor = Config.hSensor;

	//power up
	result = IsiSensorSetPowerIss(Config.hSensor, BOOL_TRUE);
	if (result != RET_SUCCESS){
		TRACE(ISP_TEST_ERROR, "power up sensor faild\n");
		return result;
	}

	// chip ID
	result = IsiGetSensorRevisionIss(Config.hSensor, &revId);
	if (result != RET_SUCCESS){
		TRACE(ISP_TEST_ERROR, "%d, read chip id faild\n", result);
		return result;
	}

	TRACE(ISP_TEST_DEBUG, "read chip id succuss chipID id 0x%08x\n", revId);

	//capability
	result = IsiGetCapsIss(Config.hSensor, &SensorIf.Caps);
	if (result != RET_SUCCESS){
		TRACE(ISP_TEST_ERROR, "get cap faild\n");
		return result;
	}

	MEMCPY( &SensorIf.sensorConfig,
			SensorIf.pCamDrvConfig->IsiSensor.pIsiSensorCaps,
			sizeof(IsiSensorCaps_t));

	//dump sensor cap and config
	dumpSensorCapabilityandconfig(&SensorIf.Caps, &SensorIf.sensorConfig);
#if 0
	//get regs vaddr for mipi phy
	result = SensorIf.pImiMipiDrvConfig->
		ImiMipiToDvpIss.pImi_GetMipiRegVaddr(device_file);
	if (result < 0) {
		close(device_file);
		device_file = 0;
		TRACE(ISP_TEST_ERROR,"Get MipiRegVaddr failed >_<, result=%d\n",
				result);
		return result;
	}
	TRACE(ISP_TEST_DEBUG,"Get MipiRegVaddr success *_*\n");

	//get mipiimage width & height and clk
	SensorIf.pImiMipiDrvConfig->Imi_ImageConfig.height =
			g_default_path_preview.width;
	SensorIf.pImiMipiDrvConfig->Imi_ImageConfig.width =
			g_default_path_preview.height;
	SensorIf.pImiMipiDrvConfig->Imi_ImageConfig.clk =
			get_sensor_param()->mipi_clk;

	TRACE(ISP_TEST_DEBUG, "%s, mipi_height:%d mipi_width:%d mipi_clk:%d\n",
			__FUNCTION__,
			SensorIf.pImiMipiDrvConfig->Imi_ImageConfig.height,
			SensorIf.pImiMipiDrvConfig->Imi_ImageConfig.width,
			SensorIf.pImiMipiDrvConfig->Imi_ImageConfig.clk);
#endif
	result = IsiSetupSensorIss(Config.hSensor, &SensorIf.sensorConfig);
	if (result != RET_SUCCESS) {
		TRACE(ISP_TEST_ERROR, "setup sensor faild\n");
		return result;
	}
	TRACE(ISP_TEST_WARN, "setup sensor succuss\n");

	TRACE(ISP_TEST_INFO, "%s (exit)\n", __FUNCTION__);

	return 0;
}

static void CloseLib(void** SensorLib)
{
	RESULT result;
	result = dlclose(*SensorLib);
	if (result)
		TRACE(ISP_TEST_ERROR, "%s %d::%s, addr=%p\n",
				__FUNCTION__, result, dlerror(), *SensorLib);
	else
		*SensorLib = NULL;
}

static void CloseSensor(void)
{
	RESULT result;

	TRACE(ISP_TEST_INFO, "%s (enter)\n", __FUNCTION__);

	// TODO: MIPI

	result = IsiSensorSetPowerIss(SensorIf.hSensor, BOOL_FALSE);
	if (result != RET_SUCCESS)
		TRACE(ISP_TEST_ERROR, "%d::set power off faild\n", result);

	result = IsiReleaseSensorIss(SensorIf.hSensor);
	if (result != RET_SUCCESS)
		TRACE(ISP_TEST_ERROR, "%d::release sensor faild\n", result);

	CloseLib(&SensorIf.hSensorLib);
	//CloseLib(&SensorIf.hMipiLib);

	TRACE(ISP_TEST_INFO, "%s (exit)\n", __FUNCTION__);
}
#if 0
static unsigned int mipi_phy_power_on(struct Imi_ImageConfig_s *p)
{
	//enable mipi to dvp interface
	RESULT result;

	result = SensorIf.pImiMipiDrvConfig->ImiMipiToDvpIss.
		pImi_Mipi_To_Dvp(get_sensor_param()->mipi_enable, p);
	TRACE(ISP_TEST_INFO, "CamSensorParam mipi to dvp enable = (%d)\n",
			get_sensor_param()->mipi_enable);

	return result;
}
#endif
static void SensorStreamOn(void)
{
	RESULT result;

	TRACE(ISP_TEST_INFO, "%s (enter)\n", __FUNCTION__);

	result = IsiSensorSetStreamingIss(SensorIf.hSensor, BOOL_TRUE);
	if (result != RET_SUCCESS) {
		TRACE(ISP_TEST_ERROR, "set stream on faild\n");
	} else {
		TRACE(ISP_TEST_DEBUG, "set stream on succuss\n");
	}
#if 0
	result = mipi_phy_power_on(&SensorIf.pImiMipiDrvConfig->Imi_ImageConfig);
	if (!result) {
		TRACE(ISP_TEST_ERROR, "set mipi to dvp controller success\n");
	} else {
		TRACE(ISP_TEST_DEBUG,"set mipi to dvp controller failed\n");
	}
#endif
	TRACE(ISP_TEST_INFO, "%s (exit)\n", __FUNCTION__);
	return;
}

static void SensorStreamOff(void)
{
	RESULT result;

	TRACE(ISP_TEST_INFO, "%s (enter)\n", __FUNCTION__);
	result = IsiSensorSetStreamingIss(SensorIf.hSensor, BOOL_FALSE);
	if (result != RET_SUCCESS) {
		TRACE(ISP_TEST_ERROR, "set stream off faild\n");
	} else {
		TRACE(ISP_TEST_INFO, "set stream off succuss\n");
	}

	TRACE(ISP_TEST_INFO, "%s (exit)\n", __FUNCTION__);
	return;
}

void cbCompletion( CamEngineCmdId_t cmdId, RESULT result, const void* pUserCtx )
{
	IspCameraEngineText_t* P_CamEngText = NULL;

	TRACE(ISP_TEST_INFO, "%s (enter)\n", __FUNCTION__);
	if (pUserCtx != NULL) {
		P_CamEngText = (IspCameraEngineText_t* )pUserCtx;
	} else {
		TRACE(ISP_TEST_ERROR, "%s para error\n", __FUNCTION__);
		return;
	}

	switch (cmdId) {
	case CAM_ENGINE_CMD_START:
		(void)osEventSignal( &P_CamEngText->eventEngineStarted );
		TRACE(ISP_TEST_DEBUG, "%s (CAM_ENGINE_CMD_START)\n", __FUNCTION__);
		break;

	case CAM_ENGINE_CMD_STOP:
		(void)osEventSignal( &P_CamEngText->eventEngineStop );
		TRACE(ISP_TEST_DEBUG, "%s (CAM_ENGINE_CMD_STOP)\n", __FUNCTION__);
		break;

	case CAM_ENGINE_CMD_START_STREAMING:
		(void)osEventSignal( &P_CamEngText->eventStartStreaming );
		TRACE(ISP_TEST_DEBUG, "%s (CAM_ENGINE_CMD_START_STREAMING)\n", __FUNCTION__);
		break;

	case CAM_ENGINE_CMD_STOP_STREAMING:
		TRACE(ISP_TEST_DEBUG, "%s (CAM_ENGINE_CMD_STOP_STREAMING)\n", __FUNCTION__);
		(void)osEventSignal( &P_CamEngText->eventStopStreaming );
		osEventSignal(&P_CamEngText->eventEngineCaptureDone);
		break;

	default:
		TRACE(ISP_TEST_ERROR, "%s (invalid cmd)\n", __FUNCTION__);
		break;
	}

	TRACE(ISP_TEST_INFO, "%s (exit)\n", __FUNCTION__);
}

static void InitCameraIC()
{
	RESULT result;

	CamEngineInstanceConfig_t *camInstanceConfig;

	TRACE(ISP_TEST_INFO, "%s (enter)\n", __FUNCTION__);
	/* enable SOM. setting */
	if( SOMCTRL_ENABLE == 1 )
		CamEngText.SomSetting.som.enable = 1;
	else
		CamEngText.SomSetting.som.enable = 0;
	/* Setup engine. */
	osEventInit(&CamEngText.eventEngineStarted, 1, 0);
	osEventInit(&CamEngText.eventEngineStop, 1, 0);
	osEventInit(&CamEngText.eventEngineCaptureDone, 1, 0);
	osEventInit(&CamEngText.eventStartStreaming, 1, 0);
	osEventInit(&CamEngText.eventStopStreaming, 1, 0);

	camInstanceConfig = &CamEngText.camInstanceConfig;
	MEMSET(camInstanceConfig, 0, sizeof(CamEngineInstanceConfig_t));

	camInstanceConfig->maxPendingCommands = 6;
	camInstanceConfig->isSystem3D         = (bool_t)false;
	camInstanceConfig->cbCompletion       = cbCompletion;
	camInstanceConfig->pUserCbCtx         = (void *)&CamEngText;
	camInstanceConfig->hHal               = CamEngText.hHal;
	camInstanceConfig->hCamEngine         = NULL;


	result = CamEngineInit(camInstanceConfig);
	if (result != RET_SUCCESS) {
		TRACE(ISP_TEST_ERROR, "camengineinit failed\n");
	} else {
		TRACE(ISP_TEST_DEBUG, "camengineinit succuss\n");
	}

	CamEngText.hCamEngine = camInstanceConfig->hCamEngine;

	CamEngineRegisterBufferCb(CamEngText.hCamEngine, cbBuffer, &CamEngText);

	/* Setup SOM. */
	if (CamEngText.SomSetting.som.enable) {
		somCtrlConfig_t somConfig;

		memset(&somConfig, 0, sizeof(somCtrlConfig_t));
		somConfig.MaxPendingCommands = 8;
		somConfig.MaxBuffers         = 4;
		somConfig.somCbCompletion    = cbSomCtrl;
		somConfig.pUserContext       = (void *)&CamEngText;
		somConfig.HalHandle          = CamEngText.hHal;
		somConfig.somCtrlHandle      = NULL;

		result = somCtrlInit(&somConfig);
		if (result != RET_SUCCESS) {
			TRACE(ISP_TEST_ERROR, "Failed to initialize SomCtrll: %d\n",result);
			return;
		}
		CamEngText.hSomCtrl = somConfig.somCtrlHandle;
		if (!CamEngText.hSomCtrl) {
			TRACE(ISP_TEST_DEBUG, "hSomCtrl is NULL\n");
			return;
		} else {
			TRACE(ISP_TEST_DEBUG, "hSomCtrl is succuss\n");
		}
	}

	TRACE(ISP_TEST_INFO, "%s (exit)\n", __FUNCTION__);
	return;
}

static void destroyCameraIC()
{
	TRACE(ISP_TEST_INFO, "%s (enter)\n", __FUNCTION__);
	if (CamEngText.SomSetting.som.enable) {
		somCtrlStop(CamEngText.hSomCtrl);
		osEventWait(&CamEngText.eventSomStop);

		osEventDestroy(&CamEngText.eventSomFinished);
		osEventDestroy(&CamEngText.eventSomStop);
		osEventDestroy(&CamEngText.eventSomStarted);

		somCtrlShutDown(CamEngText.hSomCtrl);
		CamEngText.hSomCtrl = NULL;
	}

	if (RET_SUCCESS != CamEngineShutDown(CamEngText.hCamEngine)) {
		TRACE(ISP_TEST_ERROR, "CamEngineShutDown failed\n");
	} else {
		TRACE(ISP_TEST_DEBUG, "CamEngineShutDown succuss\n");
	}

	CamEngText.hCamEngine = NULL;

	(void)osEventDestroy(&CamEngText.eventStartStreaming);
	(void)osEventDestroy(&CamEngText.eventStopStreaming);

	(void)osEventDestroy(&CamEngText.eventEngineStarted);
	(void)osEventDestroy(&CamEngText.eventEngineStop );

	TRACE(ISP_TEST_INFO, "%s (exit)\n", __FUNCTION__);
}

static void ConfigCamEnginePara()
{
	CamEngText.camEngineConfig.mode = CAM_ENGINE_MODE_SENSOR_2D;
	struct sensor_params *psensor_param = get_sensor_param();

	CamEngText.camEngineConfig.pathConfigMaster[CAMERIC_MI_PATH_MAIN] =
			*(psensor_param->ppath_preview[CAMERIC_MI_PATH_MAIN]);

	CamEngText.camEngineConfig.pathConfigMaster[CAMERIC_MI_PATH_SELF] =
			*(psensor_param->ppath_preview[CAMERIC_MI_PATH_SELF]);

	CamEngText.camEngineConfig.pathConfigSlave[CAMERIC_MI_PATH_MAIN] =
			g_cam_path_disabled;
	CamEngText.camEngineConfig.pathConfigSlave[CAMERIC_MI_PATH_SELF] =
			g_cam_path_disabled;

	return;
}

static void DisableModule()
{
	RESULT result;

	result = CamEngineCprocDisable(CamEngText.hCamEngine);
	if (result != RET_SUCCESS)
		TRACE(ISP_TEST_ERROR, "CamEngineCprocDisable failed\n");
	else
		TRACE(ISP_TEST_DEBUG, "CamEngineCprocDisable succuss\n");

	result = CamEngineDisableImageEffect(CamEngText.hCamEngine);
	if (result != RET_SUCCESS)
		TRACE(ISP_TEST_ERROR, "CamEngineDisableImageEffect failed\n");
	else
		TRACE(ISP_TEST_DEBUG, "CamEngineDisableImageEffect succuss\n");
}

static bool SetupCameraIC()
{
	RESULT result = 0;

	TRACE(ISP_TEST_INFO, "%s (enter)\n",__FUNCTION__);

	ConfigCamEnginePara();

	switch ( CamEngText.camEngineConfig.mode) {
	case CAM_ENGINE_MODE_SENSOR_2D:
		CamEngText.camEngineConfig.data.sensor.hSensor =
				SensorIf.hSensor;

		if (SensorIf.sensorConfig.SmiaMode != ISI_SMIA_OFF)
			CamEngText.camEngineConfig.data.sensor.ifSelect =
					CAMERIC_ITF_SELECT_SMIA;
		else {
			/* isp mipi phy is not used, instead by mipi_to_dvp */
			//CamEngText.camEngineConfig.data.sensor.ifSelect =
			//		CAMERIC_ITF_SELECT_MIPI;

			CamEngText.camEngineConfig.data.sensor.ifSelect =
					CAMERIC_ITF_SELECT_PARALLEL;

			if ((SensorIf.sensorConfig.MipiMode != ISI_MIPI_OFF) &&
				!isiCapValue(
				CamEngText.camEngineConfig.data.sensor.mipiMode,
				SensorIf.sensorConfig.MipiMode)) {
				result = -1;
				goto err;
			}
		}

		if (!isiCapValue(
			CamEngText.camEngineConfig.data.sensor.sampleEdge,
			SensorIf.sensorConfig.Edge)) {
			result = -2;
			goto err;
		}

		if (!isiCapValue(
			CamEngText.camEngineConfig.data.sensor.hSyncPol,
			SensorIf.sensorConfig.HPol)) {
			result = -3;
			goto err;
		}

		if (!isiCapValue(
			CamEngText.camEngineConfig.data.sensor.vSyncPol,
			SensorIf.sensorConfig.VPol)) {
			result = -4;
			goto err;
		}

		if (!isiCapValue(
			CamEngText.camEngineConfig.data.sensor.bayerPattern,
			SensorIf.sensorConfig.BPat)) {
			result = -5;
			goto err;
		}

		if (!isiCapValue(
			CamEngText.camEngineConfig.data.sensor.subSampling,
			SensorIf.sensorConfig.Conv422)) {
			result = -6;
			goto err;
		}

		if (!isiCapValue(
			CamEngText.camEngineConfig.data.sensor.seqCCIR,
			SensorIf.sensorConfig.YCSequence)) {
			result = -7;
			goto err;
		}

		if (!isiCapValue(
			CamEngText.camEngineConfig.data.sensor.fieldSelection,
			SensorIf.sensorConfig.FieldSelection)) {
			result = -8;
			goto err;
		}

		if (!isiCapValue(
			CamEngText.camEngineConfig.data.sensor.inputSelection,
			SensorIf.sensorConfig.BusWidth)) {
			result = -9;
			goto err;
		}

		if (!isiCapValue(
			CamEngText.camEngineConfig.data.sensor.mode,
			SensorIf.sensorConfig.Mode)) {
			result = -10;
			goto err;
		}

		if (!isiCapValue(
			CamEngText.camEngineConfig.data.sensor.acqWindow,
			SensorIf.sensorConfig.Resolution)) {
			result = -11;
			goto err;
		}

		MEMCPY(&CamEngText.camEngineConfig.data.sensor.outWindow,
			&CamEngText.camEngineConfig.data.sensor.acqWindow,
			sizeof(CamEngineWindow_t));
		MEMCPY(&CamEngText.camEngineConfig.data.sensor.isWindow,
			&CamEngText.camEngineConfig.data.sensor.acqWindow,
			sizeof(CamEngineWindow_t));

		CamEngText.camEngineConfig.data.sensor.enableTestpattern =
				BOOL_FALSE;
		CamEngText.camEngineConfig.data.sensor.flickerPeriod =
				CAM_ENGINE_FLICKER_100HZ;
		CamEngText.camEngineConfig.data.sensor.enableAfps = BOOL_TRUE;
		break;

		default:
			goto err;
	}

	CamEngText.camEngineConfig.hCamCalibDb =
			CamEngText.calibDb.GetCalibDbHandle();

	result = CamEngineStart(CamEngText.hCamEngine,
			&CamEngText.camEngineConfig);
	if (RET_PENDING == result) {
		if (OSLAYER_OK !=
				osEventTimedWait(&CamEngText.eventEngineStarted,
				CAM_API_CAMENGINE_TIMEOUT))
			goto err_stop_engine;
	} else if (RET_SUCCESS != result)
		goto err;

	DisableModule();

	TRACE(ISP_TEST_INFO, "%s (exit)\n",__FUNCTION__);

	return true;

err_stop_engine:
	CamEngineStop(CamEngText.hCamEngine);
err:
	TRACE(ISP_TEST_ERROR, "CamEngineStart failed[%d]\n", result);
	return false;
}

static bool SensorStartStream()
{
	RESULT result;
	unsigned int frameNum = 0;

	TRACE(ISP_TEST_INFO, "%s (enter)\n",__FUNCTION__);

	if (ioctl(device_file, CAMSYS_MIPI_CFG, &g_mipi_param) < 0)
		TRACE(ISP_TEST_ERROR, "%s open mipi failed\n", __FUNCTION__);

	SensorStreamOn();

	if (CamEngText.SomSetting.som.enable) {
		somCtrlCmdParams_t params;

		char acBaseFileName[300] = "";
		snprintf( acBaseFileName, sizeof(acBaseFileName), "raw_pic");

		osEventInit(&CamEngText.eventSomStarted, 1, 0);
		osEventInit(&CamEngText.eventSomStop, 1, 0);
		osEventInit(&CamEngText.eventSomFinished, 1, 0);
		params.Start.szBaseFileName = acBaseFileName;
		params.Start.NumOfFrames = 1;
		/* Just save the last frame. */
		params.Start.NumSkipFrames = 0;
		params.Start.AverageFrames = false;
		params.Start.ForceRGBOut = (bool_t)true;
		params.Start.ExtendName  = (bool_t)false;

		result = somCtrlStart(CamEngText.hSomCtrl, &params);
		if (!((RET_SUCCESS == result) || ((RET_PENDING == result) &&
				(osEventWait(&CamEngText.eventSomStarted) ==
					OSLAYER_OK)))) {
			TRACE(ISP_TEST_ERROR, "%s %d(can't start snapshot)\n",
					__FUNCTION__, result);
			return result;
		}
	}

	//CamEngText.state = Idle;

	/* try to start AEC module*/
	(void)startAec();

	/* start AWB module */
	(void)startAwb( CAM_ENGINE_AWB_MODE_AUTO, 0, BOOL_TRUE );

	/* start ADPF module */
	(void)startAdpf();

	/* try to start ADPCC module */
	(void)startAdpcc();

	/* try to start AVS module */
	if (CAM_ENGINE_MODE_SENSOR_2D_IMGSTAB == CamEngText.camEngineConfig.mode)
		(void)startAvs();

	TRACE(ISP_TEST_DEBUG, "%s: start 3A ok, and CamEngineStartStreaming\n",
			__func__);

	/* Start capturing. */
	result = CamEngineStartStreaming(CamEngText.hCamEngine, frameNum);
	if (!((RET_SUCCESS == result) || ((RET_PENDING == result) &&
			(osEventWait(&CamEngText.eventStartStreaming) ==
				OSLAYER_OK)))) {
		TRACE(ISP_TEST_ERROR, "%s %d(can't start streaming)\n",
				__FUNCTION__, result);
		return result;
	}
	TRACE(ISP_TEST_DEBUG, "CamEngineStartStreaming success\n");
	osEventWait(&CamEngText.eventStopStreaming);

	if (CamEngText.SomSetting.som.enable)
		osEventWait(&CamEngText.eventSomFinished);

	TRACE(ISP_TEST_INFO, "%s (exit)\n",__FUNCTION__);
	return true;
}

static int sensor_stop_stream(struct imi_isp_ctrl *gctrl)
{
	int ret;

	ret = CamEngineStopStreaming(CamEngText.hCamEngine);
	if (RET_PENDING == ret) {
		if (OSLAYER_OK != osEventWait(&CamEngText.eventStopStreaming)) {
			TRACE(ISP_TEST_ERROR, "stop streaming timed out\n");
			return -1;
		} else {
			TRACE(ISP_TEST_DEBUG, "stop streaming success\n");
		}
	} else if (RET_SUCCESS != ret){
		TRACE(ISP_TEST_ERROR, "can not stop streaming\n");
		return -1;
	}

	SensorStreamOff();

	gctrl->sensor_stream_state = IBINDER_SENSOR_STREAM_OFF;
	TRACE(ISP_TEST_INFO, "sensor_stop_stream SUCCESS\n");

	return ret;
}

static void DoneCameraIC()
{
	RESULT result;

	/* stop AEC module */
	(void)stopAec();
	/* stop AWB module */
	(void)stopAwb();
	/* stop ADPF module */
	(void)stopAdpf();
	/* stop ADPCC module */
	(void)stopAdpcc();
	/* stop AVS module */
	if (CAM_ENGINE_MODE_SENSOR_2D_IMGSTAB == CamEngText.camEngineConfig.mode)
		(void)stopAvs();

	result = CamEngineStop(CamEngText.hCamEngine);
	if (RET_PENDING == result) {
		if (OSLAYER_OK != osEventTimedWait(&CamEngText.eventEngineStop, CAM_API_CAMENGINE_TIMEOUT)) {
			TRACE(ISP_TEST_ERROR, "CamEngineStop cam-engine instance timed out\n");
		} else {
			TRACE(ISP_TEST_INFO, "CamEngineStop cam-engine instance succuss\n");
		}
	} else if (RET_SUCCESS != result) {
		TRACE(ISP_TEST_ERROR, "CamEngineStop failed, result = %d\n", result);
	} else if (RET_SUCCESS == result) {
		TRACE(ISP_TEST_INFO, "CamEngineStop succuss result = %d\n", result);
	}

	return;
}

static int sensor_format_set(struct imi_isp_ctrl *ctrl)
{
	int ret = 0;

	TRACE(ISP_TEST_INFO, "format:%d x %d\n", ctrl->fmt.width, ctrl->fmt.height);
	//TODO: set format

	return ret;
}

static int sensor_param_set(struct imi_isp_ctrl *ctrl)
{
	int ret = 0;

	TRACE(ISP_TEST_INFO, "%s\n", __func__);
	//TODO: set param

	return ret;
}

static int sensor_reg_set(struct imi_isp_ctrl *ctrl)
{
	int ret = 0;

	TRACE(ISP_TEST_INFO, "%s\n", __func__);
	//TODO: set reg

	return ret;
}

static void sensor_read_cmos_reg(struct ibinder_sensor_reg_info reg)
{
	TRACE(ISP_TEST_INFO, "%s\n", __func__);
	//TODO: read cmos reg
}

static void *stream_on_thread(void *priv)
{
	int signum;
	struct imi_isp_ctrl *ctrl = (struct imi_isp_ctrl *)priv;

	while(1) {
		TRACE(ISP_TEST_WARN, "stream_on_thread sigwait\n");
		sigwait(&ctrl->set, &signum);
		if (ctrl->isp_ctl == 1 && SIGUSR1 == signum
			&& ctrl->sensor_stream_state == IBINDER_SENSOR_STREAM_OFF) {
			ctrl->sensor_stream_state = IBINDER_SENSOR_STREAM_ON;
			SensorStartStream();
		}
	}
}

static void sensor_start_stream(struct imi_isp_ctrl *gctrl)
{
	pthread_kill(gctrl->stream_on_thd, SIGUSR1);
}

static void isp_send_event(struct ibinder_event_get eget)
{
	int ret;
	struct imi_isp_ctrl *ctrl = &gisp_ctrl;
	struct ibinder_event *event = &eget.event;
	event->header.src_id = IBINDER_EVENT_ISP;
	event->header.dst_id = IBINDER_EVENT_APP;

	TRACE(ISP_TEST_DEBUG, "%s header.id=%d\n.", __func__, event->header.id);
	switch(event->header.id) {
	case IBINDER_SENSOR_GET_CTL:
		event->data = (unsigned long)&ctrl->isp_ctl;
		event->header.len = sizeof(ctrl->isp_ctl);
		break;
	case IBINDER_SENSOR_GET_FMT: {
		sensor_param_t *psensor_param = get_sensor_param();
		CamEnginePathConfig_t *ppath =
			psensor_param->ppath_preview[get_opened_path()];

		ctrl->fmt.width = ppath->width;
		ctrl->fmt.height = ppath->height;
		ctrl->fmt.pixelformat = ppath->mode;
		event->data = (unsigned long)&ctrl->fmt;
		event->header.len = sizeof(ctrl->fmt);
		break;
	}
	case IBINDER_SENSOR_GET_PARAM:
		event->data = (unsigned long)&ctrl->param;
		event->header.len = sizeof(ctrl->param);
		break;
	case IBINDER_SENSOR_GET_REG:
		sensor_read_cmos_reg(ctrl->reg);
		event->data = (unsigned long)&ctrl->reg;
		event->header.len = sizeof(ctrl->reg);
		break;
	default:
		TRACE(ISP_TEST_ERROR, "%s invalid id=%d\n", __func__, event->header.id);
		break;
	}

	ret = ioctl(ctrl->fd_addr, IBINDER_EVENT_SET, event);
	if (ret < 0) {
		TRACE(ISP_TEST_ERROR, "IBINDER_EVENT_SET failed: %s (%d)\n", strerror(errno), errno);
		return;
	}
	TRACE(ISP_TEST_DEBUG, "%s set event success\n", __func__);
}

static void isp_recv_event_thread(struct imi_isp_ctrl *gctrl)
{
	struct ibinder_event_get eget;
	struct imi_isp_ctrl ctrl;
	union sensor_info info;
	int ret;

	gctrl->sensor_stream_state = IBINDER_SENSOR_STREAM_OFF;

	while(1) {
		TRACE(ISP_TEST_DEBUG, "%s\n", __func__);
		eget.event.header.src_id = IBINDER_EVENT_ALL;
		eget.event.header.dst_id = IBINDER_EVENT_ISP;
		eget.event.data = (unsigned long)&info;
		eget.timeout = -1;
		ret = ioctl(gctrl->fd_addr, IBINDER_EVENT_GET, &eget);
		if (ret < 0) {
			TRACE(ISP_TEST_ERROR, "IBINDER_EVENT_GET failed: %s (%d)\n",
				strerror(errno), errno);
		}
		TRACE(ISP_TEST_DEBUG, "eget.event.header.id=%d,len=%d\n",
			eget.event.header.id, eget.event.header.len);

		switch(eget.event.header.id) {
		case IBINDER_SENSOR_SET_CTL:
			if (eget.event.header.len != sizeof(unsigned char))
				break;
			memcpy(&gctrl->isp_ctl, (void *)eget.event.data, sizeof(unsigned char));
			if (gctrl->isp_ctl == IBINDER_SENSOR_STREAM_ON
				&& gctrl->sensor_stream_state == IBINDER_SENSOR_STREAM_OFF)
				sensor_start_stream(gctrl);
			else if (gctrl->isp_ctl == IBINDER_SENSOR_STREAM_OFF
				&& gctrl->sensor_stream_state == IBINDER_SENSOR_STREAM_ON)
				sensor_stop_stream(gctrl);
			break;
		case IBINDER_SENSOR_SET_FMT:
			if (eget.event.header.len != sizeof(struct ibinder_sensor_format))
				break;
			memcpy(&ctrl.fmt, (void *)eget.event.data, sizeof(struct ibinder_sensor_format));
			sensor_format_set(&ctrl);
			break;
		case IBINDER_SENSOR_SET_PARAM:
			if (eget.event.header.len != sizeof(struct ibinder_sensor_param_info))
				break;
			memcpy(&ctrl.param, (void *)eget.event.data, sizeof(struct ibinder_sensor_param_info));
			sensor_param_set(&ctrl);
			break;
		case IBINDER_SENSOR_SET_REG:
			if (eget.event.header.len != sizeof(struct ibinder_sensor_reg_info))
				break;
			memcpy(&ctrl.reg, (void *)eget.event.data, sizeof(struct ibinder_sensor_reg_info));
			sensor_reg_set(&ctrl);
			break;
		default:
			break;
		}

		if ((eget.event.header.id == IBINDER_SENSOR_GET_CTL)
			|| eget.event.header.id == IBINDER_SENSOR_GET_FMT
			|| eget.event.header.id == IBINDER_SENSOR_GET_PARAM
			|| eget.event.header.id == IBINDER_SENSOR_GET_REG) {
			isp_send_event(eget);
		}
	}
}

/******************************************************************************
 * startAec
 *****************************************************************************/
static bool startAec(void)
{
	#if CAMENGINE_STATE
	if (Invalid == CamEngText.state)
		return false;
	#endif

	if (RET_SUCCESS != CamEngineAecStart(CamEngText.hCamEngine)) {
		TRACE(ISP_TEST_ERROR, "%s: CamEngineAecStart err\n", __func__);
		return false;
	}

	return true;
}

/******************************************************************************
 * startAwb
 *****************************************************************************/
static bool startAwb(const CamEngineAwbMode_t &mode, uint32_t idx, const bool_t damp)
{
	#if CAMENGINE_STATE
	if (Invalid == CamEngText->state)
		return false;
	#endif

	if (RET_SUCCESS != CamEngineAwbStart(CamEngText.hCamEngine, mode, idx,
						damp)) {
		TRACE(ISP_TEST_ERROR, "%s: CamEngineAwbStart err\n", __func__);
		return false;
	}

	return true;
}

/******************************************************************************
 * startAdpf
 *****************************************************************************/
static bool startAdpf(void)
{
	#if CAMENGINE_STATE
	if (Invalid == CamEngText->state)
		return false;
	#endif

	if (RET_SUCCESS != CamEngineAdpfStart(CamEngText.hCamEngine)) {
		TRACE(ISP_TEST_ERROR, "%s: CamEngineAdpfStart err\n", __func__);
		return false;
	}

	return true;
}

/******************************************************************************
 * startAdpcc
 *****************************************************************************/
static bool startAdpcc(void)
{
	#if CAMENGINE_STATE
	if (Invalid == CamEngText->state)
	return false;
	#endif

	if (RET_SUCCESS != CamEngineAdpccStart(CamEngText.hCamEngine)) {
		TRACE(ISP_TEST_ERROR, "%s: CamEngineAdpccStart err\n", __func__);
		return false;
	}

	return true;
}

/******************************************************************************
 * startAvs
 *****************************************************************************/
static bool startAvs(void)
{
	#if CAMENGINE_STATE
	if (Invalid == CamEngText->state)
		return false;
	#endif

	if (RET_SUCCESS != CamEngineAvsStart(CamEngText.hCamEngine)) {
		TRACE(ISP_TEST_ERROR, "%s: CamEngineAvsStart err\n", __func__);
		return false;
	}

	return true;
}

/******************************************************************************
 * stopAec
 *****************************************************************************/
static bool stopAec(void)
{
	#if CAMENGINE_STATE
	if (Invalid == CamEngText->state)
		return false;
	#endif

	if (RET_SUCCESS != CamEngineAecStop(CamEngText.hCamEngine)) {
		TRACE(ISP_TEST_ERROR, "%s: CamEngineAecStop err\n", __func__);
		return false;
	}

	return true;
}

/******************************************************************************
 * stopAwb
 *****************************************************************************/
static bool stopAwb(void)
{
	#if CAMENGINE_STATE
	if (Invalid == CamEngText->state)
		return false;
	#endif

	if (RET_SUCCESS != CamEngineAwbStop(CamEngText.hCamEngine)) {
		TRACE(ISP_TEST_ERROR, "%s: CamEngineAwbStop err\n", __func__);
		return false;
	}

	return true;
}
/******************************************************************************
 * stopAdpf
 *****************************************************************************/
static bool stopAdpf(void)
{
	#if CAMENGINE_STATE
	if (Invalid == CamEngText->state)
		return false;
	#endif

	if (RET_SUCCESS != CamEngineAdpfStop(CamEngText.hCamEngine)) {
		TRACE(ISP_TEST_ERROR, "%s: CamEngineAdpfStop err\n", __func__);
		return false;
	}

	return true;
}
/******************************************************************************
 * stopAdpcc
 *****************************************************************************/
static bool stopAdpcc(void)
{
	#if CAMENGINE_STATE
	if (Invalid == CamEngText->state)
		return false;
	#endif

	if (RET_SUCCESS != CamEngineAdpccStop(CamEngText.hCamEngine)) {
		TRACE(ISP_TEST_ERROR, "%s: CamEngineAdpccStop err\n", __func__);
		return false;
	}

	return true;
}
/******************************************************************************
 * stopAvs
 *****************************************************************************/
static bool stopAvs(void)
{
	#if CAMENGINE_STATE
	if (Invalid == CamEngText->state)
		return false;
	#endif

	if (RET_SUCCESS != CamEngineAvsStop(CamEngText.hCamEngine)) {
		TRACE(ISP_TEST_ERROR, "%s: CamEngineAVsStop err\n", __func__);
		return false;
	}

	return true;
}

int main(int argc, char **argv)
{
	struct imi_isp_ctrl *ctrl = &gisp_ctrl;

	if (default_config(argc, argv))
		return -1;

	sigemptyset(&ctrl->set);
	sigaddset(&ctrl->set, SIGUSR1);
	sigprocmask(SIG_SETMASK, &ctrl->set, NULL);

	prepare_hal();

	ctrl->fd_addr = open(ISP_ADDR_PIC, O_RDWR);
	if(!ctrl->fd_addr) {
		TRACE(ISP_TEST_ERROR, "open %s failed, ret =%d\n",
				ISP_ADDR_PIC, ctrl->fd_addr);
	}

	InitCameraIC();
	if (OpenSensor(get_sensor_param()->libname))
		return -1;

	//IsiActivateTestPattern(SensorIf.hSensor, BOOL_TRUE);

	SetupCameraIC();

	/* camif default opened */
	//if (!camif_alloc(HalHandle))
	{
		pthread_t thd;

		pthread_create(&thd, NULL, open_debug_fifo, NULL);
		pthread_create(&ctrl->stream_on_thd, NULL,
				stream_on_thread, (void*)ctrl);

		isp_recv_event_thread(ctrl);

		pthread_join(thd, NULL);
		pthread_join(ctrl->stream_on_thd, NULL);
	}

	TRACE(ISP_TEST_INFO, "main thread exit\n");

	camif_release_safe(HalHandle);
	debug_thread_off();
	DoneCameraIC();
	destroyCameraIC();
	CloseSensor();
	release_hal();

	return 0;
}
