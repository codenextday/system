/******************************************************************************
 *
 * Copyright 2010, Dream Chip Technologies GmbH. All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of:
 * Dream Chip Technologies GmbH, Steinriede 10, 30827 Garbsen / Berenbostel,
 * Germany
 *
 *****************************************************************************/
/**
 * @file AR230.c
 *
 * @brief
 * ADD_DESCRIPTION_HERE
 *
 *****************************************************************************/
#include <ebase/types.h>
#include <ebase/trace.h>
#include <ebase/builtins.h>
#include <common/return_codes.h>
#include <common/misc.h>
#include <math.h>
#include "gpio.h"
#include "isi.h"
#include "isi_iss.h"
#include "isi_priv.h"
#include "AR230_priv.h"

#define AR230_SLAVE_ADDR            0x10
#define AR230_SLAVE_AF_ADDR         0x0

//#define AR230_TRIGGER

/**< min gain step size used by GUI ( 32/(32-7) - 32/(32-6); min. reg value is 6
		as of datasheet; depending on actual gain ) */
#define SENSOR_MIN_GAIN_STEP        (1.0f / 16.0f)
/**< max. gain used by the AEC (arbitrarily chosen, recommended by Omnivision)*/
#define SENSOR_MAX_GAIN_AEC         (2.0f)

/*!<
 * Focus position values:
 * 65 logical positions ( 0 - 64 )
 * where 64 is the setting for infinity and 0 for macro
 * corresponding to
 * 1024 register settings (0 - 1023)
 * where 0 is the setting for infinity and 1023 for macro
 */
#define MAX_LOG                     64
#define MAX_REG                     1023

/*!<
 * Lens movement is triggered every 133ms (VGA, 7.5fps processed frames
 * worst case assumed, usually even much slower, see AR230 driver for
 * details). Thus the lens has to reach the requested position after
 * max. 133ms. Minimum mechanical ringing is expected with mode 1 ,
 * 100us per step. A movement over the full range needs max. 102.3ms
 * (see table 9 AD5820 datasheet).
 */
#define MDI_SLEW_RATE_CTRL          2   /* S3..0 */

#define AWB_SATURATION_ARRAY_SIZE   4
#define AWB_COLORMATRIX_ARRAY_SIZE  4

#define AWB_VIGNETTING_ARRAY_SIZE   4
#define AWB_LSCMATRIX_ARRAY_SIZE    2

#define AR230_DVP_CHIP_VERSION      0x0056

#define DEBUG_IN_FPGA               1

#define USE_VGA                     0

#ifdef ISP_FPGA
#define AR0230_RST                  2
#else
#define GPIO_PIN_AR0230_AVDD        (GPIO_GROUP_1 + 4)
#define GPIO_PIN_AR0230_IO          (GPIO_GROUP_2 + 24)
#define GPIO_PIN_AR0230_DVDD        (GPIO_GROUP_2 + 16)
#define AR0230_RST                  (GPIO_GROUP_2 + 12)
#endif

/******************************************************************************
 * local type definitions
 *****************************************************************************/
CREATE_TRACER(AR230_INFO,   "AR230_INFO ",  INFO,       0);
CREATE_TRACER(AR230_WARN,   "AR230_WARN ",  WARNING,    1);
CREATE_TRACER(AR230_ERROR,  "AR230_ERR ",   ERROR,      1);
CREATE_TRACER(AR230_DEBUG,  "AR230_DBG ",   WARNING,    0);

/******************************************************************************
 * local variable declarations
 *****************************************************************************/
extern const IsiRegDescription_t AR230_g_aRegDescription[];
extern const IsiRegDescription_t AR230_g_svga[];
extern const IsiRegDescription_t AR230_g_vga[];
extern const IsiRegDescription_t AR230_g_1080p[];

// The sensor design may not allow to alter integration time from frame to frame
// (for example the classic rolling shutter). So we remember the old integration
// time to figure out if we are changing it and to tell the upper layers how much
// frames they have to wait before executing the AE again.
// static uint32_t OldCoarseIntegrationTime = 0UL;

/* AWB specific value (from AR230_tables.c) */
extern const Isi1x1FloatMatrix_t AR230_KFactor;
extern const Isi3x2FloatMatrix_t AR230_PCAMatrix;
extern const Isi3x1FloatMatrix_t AR230_SVDMeanValue;
extern const IsiLine_t AR230_CenterLine;
extern const IsiAwbClipParm_t AR230_AwbClipParm;
extern const IsiAwbGlobalFadeParm_t AR230_AwbGlobalFadeParm;
extern const IsiAwbFade2Parm_t AR230_AwbFade2Parm;

/* illumination profiles */
#include "AR230_a.h"		/* CIE A - default profile */
#include "AR230_d50.h"		/* sunny (D50) */
#include "AR230_d60.h"		/* day   (D60) */
#include "AR230_d65.h"		/* CIE D65 (D65) note: indoor because of our lightbox */
#include "AR230_d75.h"		/* CIE D75 (D75) overcast daylight, 7500K */
//#include "AR230_d80.h"     /* shade (D80) */
//#include "AR230_d120.h"    /* twilight (D120) */
#include "AR230_f2.h"		/* CIE F2 (cool white flourescent CWF) */
#include "AR230_f11.h"		/* CIE F11 (TL84) */
#include "AR230_f12.h"		/* CIE F12 (TL83) */

#define AR230_ISIILLUPROFILES_DEFAULT  10
static IsiIlluProfile_t AR230_IlluProfileDefault[AR230_ISIILLUPROFILES_DEFAULT]
= {
	{
		.p_next             = NULL,
		.name               = "A",
		.id                 = ISI_CIEPROF_A,
		.DoorType           = ISI_DOOR_TYPE_INDOOR,
		.AwbType            = ISI_AWB_TYPE_AUTO,
		.bOutdoorClip       = BOOL_FALSE,

		/* legacy stuff */
		.pCrossTalkCoeff    = &AR230_XTalkCoeff_CIE_A,
		.pCrossTalkOffset   = &AR230_XTalkOffset_CIE_A,

		.pGaussMeanValue    = &AR230_GaussMeanValue_CIE_A,
		.pCovarianceMatrix  = &AR230_CovarianceMatrix_CIE_A,
		.pGaussFactor       = &AR230_GaussFactor_CIE_A,
		.pThreshold         = &AR230_Threshold_CIE_A,
		.pComponentGain     = &AR230_CompGain_CIE_A,
		.pSaturationCurve   = &AR230_SaturationCurve_CIE_A,
		.pCcMatrixTable     = &AR230_CcMatrixTable_CIE_A,
		.pCcOffsetTable     = &AR230_CcOffsetTable_CIE_A,
		.pVignettingCurve   = &AR230_VignettingCurve_CIE_A,
	},
	{
		.p_next             = NULL,
		.name               = "D50",
		.id                 = ISI_CIEPROF_D50,
		.DoorType           = ISI_DOOR_TYPE_OUTDOOR,
		.AwbType            = ISI_AWB_TYPE_AUTO,
		.bOutdoorClip       = BOOL_TRUE,

		/* legacy stuff */
		.pCrossTalkCoeff    = &AR230_XTalkCoeff_sunny,
		.pCrossTalkOffset   = &AR230_XTalkOffset_sunny,

		.pGaussMeanValue    = &AR230_GaussMeanValue_sunny,
		.pCovarianceMatrix  = &AR230_CovarianceMatrix_sunny,
		.pGaussFactor       = &AR230_GaussFactor_sunny,
		.pThreshold         = &AR230_Threshold_sunny,
		.pComponentGain     = &AR230_CompGain_sunny,
		.pSaturationCurve   = &AR230_SaturationCurve_sunny,
		.pCcMatrixTable     = &AR230_CcMatrixTable_sunny,
		.pCcOffsetTable     = &AR230_CcOffsetTable_sunny,
		.pVignettingCurve   = &AR230_VignettingCurve_sunny,
	},
	{
		.p_next             = NULL,
		.name               = "D60",
		.id                 = ISI_CIEPROF_D60,
		.DoorType           = ISI_DOOR_TYPE_OUTDOOR,
		.AwbType            = ISI_AWB_TYPE_AUTO,
		.bOutdoorClip       = BOOL_FALSE,

		/* legacy stuff */
		.pCrossTalkCoeff    = &AR230_XTalkCoeff_day,
		.pCrossTalkOffset   = &AR230_XTalkOffset_day,

		.pGaussMeanValue    = &AR230_GaussMeanValue_day,
		.pCovarianceMatrix  = &AR230_CovarianceMatrix_day,
		.pGaussFactor       = &AR230_GaussFactor_day,
		.pThreshold         = &AR230_Threshold_day,
		.pComponentGain     = &AR230_CompGain_day,
		.pSaturationCurve   = &AR230_SaturationCurve_day,
		.pCcMatrixTable     = &AR230_CcMatrixTable_day,
		.pCcOffsetTable     = &AR230_CcOffsetTable_day,
		.pVignettingCurve   = &AR230_VignettingCurve_day,
	},
	{
		.p_next             = NULL,
		.name               = "D65",
		.id                 = ISI_CIEPROF_D65,
		.DoorType           = ISI_DOOR_TYPE_INDOOR,
		.AwbType            = ISI_AWB_TYPE_AUTO,
		.bOutdoorClip       = BOOL_FALSE,

		/* legacy stuff */
		.pCrossTalkCoeff    = &AR230_XTalkCoeff_D65,
		.pCrossTalkOffset   = &AR230_XTalkOffset_D65,

		.pGaussMeanValue    = &AR230_GaussMeanValue_D65,
		.pCovarianceMatrix  = &AR230_CovarianceMatrix_D65,
		.pGaussFactor       = &AR230_GaussFactor_D65,
		.pThreshold         = &AR230_Threshold_D65,
		.pComponentGain     = &AR230_CompGain_D65,
		.pSaturationCurve   = &AR230_SaturationCurve_D65,
		.pCcMatrixTable     = &AR230_CcMatrixTable_D65,
		.pCcOffsetTable     = &AR230_CcOffsetTable_D65,
		.pVignettingCurve   = &AR230_VignettingCurve_D65,
	},
	{
		.p_next             = NULL,
		.name               = "D75",
		.id                 = ISI_CIEPROF_D75,
		.DoorType           = ISI_DOOR_TYPE_OUTDOOR,
		.AwbType            = ISI_AWB_TYPE_AUTO,
		.bOutdoorClip       = BOOL_FALSE,

		/* legacy stuff */
		.pCrossTalkCoeff    = &AR230_XTalkCoeff_D75,
		.pCrossTalkOffset   = &AR230_XTalkOffset_D75,

		.pGaussMeanValue    = &AR230_GaussMeanValue_D75,
		.pCovarianceMatrix  = &AR230_CovarianceMatrix_D75,
		.pGaussFactor       = &AR230_GaussFactor_D75,
		.pThreshold         = &AR230_Threshold_D75,
		.pComponentGain     = &AR230_CompGain_D75,
		.pSaturationCurve   = &AR230_SaturationCurve_D75,
		.pCcMatrixTable     = &AR230_CcMatrixTable_D75,
		.pCcOffsetTable     = &AR230_CcOffsetTable_D75,
		.pVignettingCurve   = &AR230_VignettingCurve_D75,
	},
	{
		.p_next             = NULL,
		.name               = "F2",
		.id                 = ISI_CIEPROF_F2,
		.DoorType           = ISI_DOOR_TYPE_INDOOR,
		.AwbType            = ISI_AWB_TYPE_AUTO,
		.bOutdoorClip       = BOOL_FALSE,

		/* legacy stuff */
		.pCrossTalkCoeff    = &AR230_XTalkCoeff_F2,
		.pCrossTalkOffset   = &AR230_XTalkOffset_F2,

		.pGaussMeanValue    = &AR230_GaussMeanValue_F2,
		.pCovarianceMatrix  = &AR230_CovarianceMatrix_F2,
		.pGaussFactor       = &AR230_GaussFactor_F2,
		.pThreshold         = &AR230_Threshold_F2,
		.pComponentGain     = &AR230_CompGain_F2,
		.pSaturationCurve   = &AR230_SaturationCurve_F2,
		.pCcMatrixTable     = &AR230_CcMatrixTable_F2,
		.pCcOffsetTable     = &AR230_CcOffsetTable_F2,
		.pVignettingCurve   = &AR230_VignettingCurve_F2,
	},
	{
		.p_next             = NULL,
		.name               = "F11",
		.id                 = ISI_CIEPROF_F11,
		.DoorType           = ISI_DOOR_TYPE_INDOOR,
		.AwbType            = ISI_AWB_TYPE_AUTO,
		.bOutdoorClip       = BOOL_FALSE,

		/* legacy stuff */
		.pCrossTalkCoeff    = &AR230_XTalkCoeff_F11,
		.pCrossTalkOffset   = &AR230_XTalkOffset_F11,

		.pGaussMeanValue    = &AR230_GaussMeanValue_F11,
		.pCovarianceMatrix  = &AR230_CovarianceMatrix_F11,
		.pGaussFactor       = &AR230_GaussFactor_F11,
		.pThreshold         = &AR230_Threshold_F11,
		.pComponentGain     = &AR230_CompGain_F11,
		.pSaturationCurve   = &AR230_SaturationCurve_F11,
		.pCcMatrixTable     = &AR230_CcMatrixTable_F11,
		.pCcOffsetTable     = &AR230_CcOffsetTable_F11,
		.pVignettingCurve   = &AR230_VignettingCurve_F11,
	},
	{
		.p_next             = NULL,
		.name               = "F12",
		.id                 = ISI_CIEPROF_F12,
		.DoorType           = ISI_DOOR_TYPE_INDOOR,
		.AwbType            = ISI_AWB_TYPE_AUTO,
		.bOutdoorClip       = BOOL_FALSE,

		/* legacy stuff */
		.pCrossTalkCoeff    = &AR230_XTalkCoeff_F12,
		.pCrossTalkOffset   = &AR230_XTalkOffset_F12,

		.pGaussMeanValue    = &AR230_GaussMeanValue_F12,
		.pCovarianceMatrix  = &AR230_CovarianceMatrix_F12,
		.pGaussFactor       = &AR230_GaussFactor_F12,
		.pThreshold         = &AR230_Threshold_F12,
		.pComponentGain     = &AR230_CompGain_F12,
		.pSaturationCurve   = &AR230_SaturationCurve_F12,
		.pCcMatrixTable     = &AR230_CcMatrixTable_F12,
		.pCcOffsetTable     = &AR230_CcOffsetTable_F12,
		.pVignettingCurve   = &AR230_VignettingCurve_F12,
	},
};
const char              AR230_g_acName[] = "AR230_DVP";
const IsiSensorCaps_t   AR230_g_IsiSensorDefaultConfig;

/******************************************************************************
 * local function prototypes
 *****************************************************************************/
static RESULT AR230_IsiCreateSensorIss(IsiSensorInstanceConfig_t *pConfig);
static RESULT AR230_IsiReleaseSensorIss(IsiSensorHandle_t handle);
static RESULT AR230_IsiGetCapsIss(IsiSensorHandle_t handle,
				  IsiSensorCaps_t *pIsiSensorCaps);
static RESULT AR230_IsiSetupSensorIss(IsiSensorHandle_t handle,
				      const IsiSensorConfig_t *pConfig);
static RESULT AR230_IsiSensorSetStreamingIss(IsiSensorHandle_t handle,
					     bool_t on);
static RESULT AR230_IsiCheckSensorConnectionIss(IsiSensorHandle_t handle);
static RESULT AR230_IsiGetSensorRevisionIss(IsiSensorHandle_t handle,
					    uint32_t *p_value);
static RESULT AR230_IsiGetGainLimitsIss(IsiSensorHandle_t handle,
					float *pMinGain, float *pMaxGain);
static RESULT AR230_IsiGetIntegrationTimeLimitsIss(IsiSensorHandle_t handle,
						   float *pMinIntegrationTime,
						   float *pMaxIntegrationTime);
static RESULT AR230_IsiExposureControlIss(IsiSensorHandle_t handle,
					  float newGain,
					  float newIntegrationTime,
					  uint8_t *pNumberOfFramesToSkip,
					  float *pSetGain,
					  float *pSetIntegrationTime);
/*
static RESULT AR230_IsiGetAfpsInfoHelperIss(AR230_Context_t *pAR230Ctx,
					    uint32_t Resolution,
					    IsiAfpsInfo_t *pAfpsInfo,
					    uint32_t AfpsStageIdx);
*/
static RESULT AR230_IsiGetAfpsInfoIss(IsiSensorHandle_t handle,
				      uint32_t Resolution,
				      IsiAfpsInfo_t *pAfpsInfo);
static RESULT AR230_IsiGetGainIss(IsiSensorHandle_t handle, float *pSetGain);
static RESULT AR230_IsiGetGainIncrementIss(IsiSensorHandle_t handle,
					   float *pIncr);
static RESULT AR230_IsiSetGainIss(IsiSensorHandle_t handle, float NewGain,
				  float *pSetGain);
static RESULT AR230_IsiGetIntegrationTimeIss(IsiSensorHandle_t handle,
					     float *pSetIntegrationTime);
static RESULT AR230_IsiGetIntegrationTimeIncrementIss(IsiSensorHandle_t handle,
						      float *pIncr);
static RESULT AR230_IsiSetIntegrationTimeIss(IsiSensorHandle_t handle,
					     float NewIntegrationTime,
					     float *pSetIntegrationTime,
					     uint8_t *pNumberOfFramesToSkip);
static RESULT AR230_IsiGetResolutionIss(IsiSensorHandle_t handle,
					uint32_t *pSetResolution);
static RESULT AR230_IsiRegReadIss(IsiSensorHandle_t handle,
				  const uint32_t address, uint32_t *p_value);
static RESULT AR230_IsiRegWriteIss(IsiSensorHandle_t handle,
				   const uint32_t address,
				   const uint32_t value);
static RESULT AR230_IsiGetCalibKFactor(IsiSensorHandle_t handle,
				       Isi1x1FloatMatrix_t **pIsiKFactor);
static RESULT AR230_IsiGetCalibPcaMatrix(IsiSensorHandle_t handle,
					 Isi3x2FloatMatrix_t **pIsiPcaMatrix);
static RESULT AR230_IsiGetCalibSvdMeanValue(IsiSensorHandle_t handle,
					    Isi3x1FloatMatrix_t **pIsiSvdMeanValue);
static RESULT AR230_IsiGetCalibCenterLine(IsiSensorHandle_t handle,
					  IsiLine_t **ptIsiCenterLine);
static RESULT AR230_IsiGetCalibClipParam(IsiSensorHandle_t handle,
					 IsiAwbClipParm_t **pIsiClipParam);
static RESULT AR230_IsiGetCalibGlobalFadeParam(IsiSensorHandle_t handle,
					       IsiAwbGlobalFadeParm_t **ptIsiGlobalFadeParam);
static RESULT AR230_IsiGetCalibFadeParam(IsiSensorHandle_t handle,
					 IsiAwbFade2Parm_t **ptIsiFadeParam);
static RESULT AR230_IsiGetIlluProfile(IsiSensorHandle_t handle,
				      const uint32_t CieProfile,
				      IsiIlluProfile_t **ptIsiIlluProfile);
static RESULT AR230_IsiActivateTestPattern(IsiSensorHandle_t handle,
					   const bool_t enable);

/*****************************************************************************/
/**
 *          AR230_IsiCreateSensorIss
 *
 * @brief   This function creates a new AR230 sensor instance handle.
 *
 * @param   pConfig     configuration structure to create the instance
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 * @retval  RET_OUTOFMEM
 *
 *****************************************************************************/
static RESULT AR230_IsiCreateSensorIss(IsiSensorInstanceConfig_t *pConfig)
{
	RESULT result = RET_SUCCESS;

	AR230_Context_t *pAR230Ctx;

	TRACE(AR230_INFO, "%s (enter)\n", __func__);

	if ((pConfig == NULL) || (pConfig->pSensor == NULL)) {
		result = RET_NULL_POINTER;
		goto err;
	}

	pAR230Ctx = (AR230_Context_t *)malloc(sizeof(AR230_Context_t));
	if (pAR230Ctx == NULL) {
		result = RET_OUTOFMEM;
		goto err;
	}

	result = HalAddRef(pConfig->HalHandle);
	if (result != RET_SUCCESS)
		goto err_add_hal;

	MEMSET(pAR230Ctx, 0, sizeof(AR230_Context_t));
	pAR230Ctx->IsiCtx.HalHandle = pConfig->HalHandle;
	pAR230Ctx->IsiCtx.HalDevID = pConfig->HalDevID;
	pAR230Ctx->IsiCtx.I2cBusNum = pConfig->I2cBusNum;
	pAR230Ctx->IsiCtx.SlaveAddress = (pConfig->SlaveAddr == 0U) ?
			AR230_SLAVE_ADDR : pConfig->SlaveAddr;
	pAR230Ctx->IsiCtx.NrOfAddressBytes = 2U;

	pAR230Ctx->IsiCtx.I2cAfBusNum = pConfig->I2cAfBusNum;
	pAR230Ctx->IsiCtx.SlaveAfAddress = (pConfig->SlaveAfAddr == 0U) ?
			AR230_SLAVE_AF_ADDR : pConfig->SlaveAfAddr;
	pAR230Ctx->IsiCtx.NrOfAfAddressBytes = 0U;

	pAR230Ctx->IsiCtx.pSensor = pConfig->pSensor;

	pAR230Ctx->Configured = BOOL_FALSE;
	pAR230Ctx->Streaming = BOOL_FALSE;

	pConfig->hSensor = (IsiSensorHandle_t)pAR230Ctx;

#if 0//ndef DEBUG_IN_FPGA
	result = HalSetCamConfig(pAR230Ctx->IsiCtx.HalHandle,
				pAR230Ctx->IsiCtx.HalDevID, true, true, false);
	RETURN_RESULT_IF_DIFFERENT(RET_SUCCESS, result);

	result = HalSetClock(pAR230Ctx->IsiCtx.HalHandle,
			pAR230Ctx->IsiCtx.HalDevID, 24000000UL);
#endif

	TRACE(AR230_INFO, "%s (exit)\n", __func__);
	return 0;

err_add_hal:
	free(pAR230Ctx);
err:
	TRACE(AR230_ERROR, "%d::create AR230 context failed\n", result);
	return result;
}

/*****************************************************************************/
/**
 *          AR230_IsiReleaseSensorIss
 *
 * @brief   This function destroys/releases an AR230 sensor instance.
 *
 * @param   handle      AR230 sensor instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 *
 *****************************************************************************/
static RESULT AR230_IsiReleaseSensorIss(IsiSensorHandle_t handle)
{
	AR230_Context_t *pAR230Ctx = (AR230_Context_t *)handle;

	RESULT result = RET_SUCCESS;

	TRACE(AR230_INFO, "%s (enter)\n", __func__);

	if (pAR230Ctx == NULL)
		return RET_WRONG_HANDLE;

//	AR230_IsiSensorSetStreamingIss(pAR230Ctx, BOOL_FALSE);
//	AR230_IsiSensorSetPowerIss(pAR230Ctx, BOOL_FALSE);

	result = HalDelRef(pAR230Ctx->IsiCtx.HalHandle);

	MEMSET(pAR230Ctx, 0, sizeof(AR230_Context_t));
	free(pAR230Ctx);

	TRACE(AR230_WARN, "%d::ReleaseSensor\n", result);

	TRACE(AR230_INFO, "%s (exit)\n", __func__);

	return result;
}

/*****************************************************************************/
/**
 *          AR230_IsiGetCapsIss
 *
 * @brief   fills in the correct pointers for the sensor description struct
 *
 * @param   param1      pointer to sensor capabilities structure
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT AR230_IsiGetCapsIss
	(IsiSensorHandle_t handle, IsiSensorCaps_t *pIsiSensorCaps)
{
	AR230_Context_t *pAR230Ctx = (AR230_Context_t *)handle;

	RESULT result = RET_SUCCESS;

	TRACE(AR230_INFO, "%s (enter)\n", __func__);

	if (pAR230Ctx == NULL)
		return RET_WRONG_HANDLE;

	if (pIsiSensorCaps == NULL) {
		return RET_NULL_POINTER;
	} else {
		pIsiSensorCaps->BusWidth = ISI_BUSWIDTH_8BIT_ZZ; //12bit interface
		pIsiSensorCaps->Mode = ISI_MODE_BAYER;
		pIsiSensorCaps->FieldSelection = ISI_FIELDSEL_BOTH;
		pIsiSensorCaps->YCSequence = ISI_YCSEQ_YCBYCR;		  /**< only Bayer supported, will not be evaluated */
		pIsiSensorCaps->Conv422 = ISI_CONV422_NOCOSITED;
		pIsiSensorCaps->BPat = ISI_BPAT_GRGRBGBG; //GRGR/BGBG
		pIsiSensorCaps->HPol = ISI_HPOL_REFPOS;
		pIsiSensorCaps->VPol = ISI_VPOL_POS;
		pIsiSensorCaps->Edge = ISI_EDGE_RISING;
		pIsiSensorCaps->BLC = ISI_BLC_AUTO;
		pIsiSensorCaps->Gamma = ISI_GAMMA_OFF;
		pIsiSensorCaps->CConv = ISI_CCONV_OFF;

#if USE_VGA
		pIsiSensorCaps->Resolution = ISI_RES_VGA;
#else
		pIsiSensorCaps->Resolution = ISI_RES_TV1080P10;
#endif

		pIsiSensorCaps->AGC = ISI_AGC_AUTO;
		pIsiSensorCaps->AWB = ISI_AWB_OFF;
		pIsiSensorCaps->AEC = ISI_AEC_AUTO;
		pIsiSensorCaps->DPCC = (ISI_DPCC_OFF | ISI_DPCC_OFF);

		pIsiSensorCaps->DwnSz = ISI_DWNSZ_SUBSMPL;
		pIsiSensorCaps->CieProfile = (ISI_CIEPROF_A
						  | ISI_CIEPROF_D50
						  | ISI_CIEPROF_D65
						  | ISI_CIEPROF_D75
						  | ISI_CIEPROF_F2
						  | ISI_CIEPROF_F11
						  | ISI_CIEPROF_F12);
		pIsiSensorCaps->SmiaMode = ISI_SMIA_OFF;
		pIsiSensorCaps->MipiMode = ISI_MIPI_OFF;
		pIsiSensorCaps->AfpsResolutions = ISI_AFPS_NOTSUPP;
	}

	TRACE(AR230_INFO, "%s (exit)\n", __func__);

	return result;
}

/*****************************************************************************/
/**
 *          AR230_g_IsiSensorDefaultConfig
 *
 * @brief   recommended default configuration for application use via call
 *          to IsiGetSensorIss()
 *
 *****************************************************************************/
const IsiSensorCaps_t AR230_g_IsiSensorDefaultConfig = {
	ISI_BUSWIDTH_8BIT_ZZ ,	// BusWidth 12bit_interface
	ISI_MODE_BAYER,		// Mode
	ISI_FIELDSEL_BOTH,	// FieldSelection
	ISI_YCSEQ_YCBYCR,	// YCSequence
	ISI_CONV422_INTER,	// Conv422
	ISI_BPAT_GRGRBGBG,	// BPat---GRGR/BGBG
	ISI_HPOL_SYNCPOS,	// HPol
	ISI_VPOL_POS,		// VPol
	ISI_EDGE_RISING,	// Edge
	ISI_BLS_OFF,		// Bls
	ISI_GAMMA_OFF,		// Gamma
	ISI_CCONV_OFF,		// CConv
#if USE_VGA
	ISI_RES_VGA,		// Resolution
#else
	ISI_RES_TV1080P10,
#endif
	ISI_DWNSZ_SUBSMPL,	// DwnSz
	ISI_BLC_AUTO,		// BLC
	ISI_AGC_OFF,		// AGC
	ISI_AWB_OFF,		// AWB
	ISI_AEC_OFF,		// AEC
	ISI_DPCC_OFF,		// DPCC
	ISI_CIEPROF_F11,	// CieProfile, this is also used as start profile for AWB (if not altered by menu settings)
	ISI_SMIA_OFF,		// SmiaMode
	ISI_MIPI_OFF,		// MipiMode
	ISI_AFPS_NOTSUPP	// AfpsResolutions
};

/*****************************************************************************/
/**
 *          AR230_SetupOutputWindow
 *
 * @brief   Setup of the image sensor considering the given configuration.
 *
 * @param   handle      AR230 sensor instance handle
 * @param   pConfig     pointer to sensor configuration structure
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT AR230_SetupOutputWindow
	(AR230_Context_t *pAR230Ctx, const IsiSensorConfig_t *pConfig)
{
	RESULT result = RET_SUCCESS;
	float rVtPixClkFreq = 0.0f;
	uint16_t usLineLengthPck = 0U;
	uint16_t usFrameLengthLines = 0U;

	TRACE(AR230_INFO, "%s (enter)\n", __func__);

	/* resolution */
	switch (pConfig->Resolution) {
	case ISI_RES_VGA: //640*480
		{
			if ((result = IsiRegDefaultsApply((IsiSensorHandle_t)pAR230Ctx,
									AR230_g_vga)) != RET_SUCCESS) {
				TRACE(AR230_ERROR, "%s failed to set ISI_RES_VGA\n", __func__);
			}

			usLineLengthPck = 0x064C;
			usFrameLengthLines = 0x01F0;
			break;
		}
	case ISI_RES_SVGAP30: //800*600
		{
			if ((result = IsiRegDefaultsApply((IsiSensorHandle_t)pAR230Ctx,
									AR230_g_svga)) != RET_SUCCESS) {
				TRACE(AR230_ERROR, "%s failed to set ISI_RES_SVGAP30\n", __func__);
			}

			usLineLengthPck = 0x0512;
			usFrameLengthLines = 0x0268;
			break;
		}

	case ISI_RES_TV1080P10: //1920*1080
		{
			if ((result = IsiRegDefaultsApply((IsiSensorHandle_t)pAR230Ctx,
									AR230_g_1080p)) != RET_SUCCESS) {
				TRACE(AR230_ERROR, "%s failed to set ISI_RES_TV1080P10\n", __func__);
			}

			//usLineLengthPck = 0x0469;
			usLineLengthPck = 0x045e;//0x0452;//0x046c;
			usFrameLengthLines = 0x0452;//0x044c;

/*			usLineLengthPck = 0x0469;
			usFrameLengthLines = 0x0448;
*/
			break;
		}
	default:
		{
			TRACE(AR230_ERROR, "%s: Resolution not supported\n",
				  __func__);
			return RET_NOTSUPP;
		}
	}

	rVtPixClkFreq = 74250000;
	pAR230Ctx->VtPixClkFreq = rVtPixClkFreq;
	pAR230Ctx->LineLengthPck = usLineLengthPck;
	pAR230Ctx->FrameLengthLines = usFrameLengthLines;
	pAR230Ctx->AecMaxIntegrationTime = (((float)pAR230Ctx->FrameLengthLines - 4)
										* (float)pAR230Ctx->LineLengthPck) /
										pAR230Ctx->VtPixClkFreq;

	TRACE(AR230_INFO, "%s (exit)\n", __func__);

	return result;
}

/*****************************************************************************/
/**
 *          AR230_SetupImageControl
 *
 * @brief   Sets the image control functions (BLC, AGC, AWB, AEC, DPCC ...)
 *
 * @param   handle      AR230 sensor instance handle
 * @param   pConfig     pointer to sensor configuration structure
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT AR230_SetupImageControl
	(AR230_Context_t *pAR230Ctx, const IsiSensorConfig_t *pConfig)
{
	RESULT result = RET_SUCCESS;

	TRACE(AR230_INFO, "%s (enter)\n", __func__);

	switch (pConfig->Bls) {
	case ISI_BLS_OFF:
		break;
	default:
		{
			TRACE(AR230_ERROR, "%s: Black level not support\n",
						__func__);
			return RET_NOTSUPP;
		}

	}

	/* black level compensation */
	switch (pConfig->BLC) {
	case ISI_BLC_OFF:
		break;
	case ISI_BLC_AUTO:
		break;
	default:
		{
			TRACE(AR230_ERROR, "%s: BLC not supported\n",
				  __func__);
			return RET_NOTSUPP;
		}
	}

	/* automatic gain control */
	switch (pConfig->AGC) {
	case ISI_AGC_OFF:
			break;
	case ISI_AGC_AUTO:
			break;
	default:
		{
			TRACE(AR230_ERROR, "%s: AGC not supported\n",
				  __func__);
			return RET_NOTSUPP;
		}
	}

	/* automatic white balance */
	switch (pConfig->AWB) {
	case ISI_AWB_OFF:
		break;
	case ISI_AWB_AUTO:
		break;
	default:
		{
			TRACE(AR230_ERROR, "%s: AWB not supported\n",
				  __func__);
			return RET_NOTSUPP;
		}
	}

	switch (pConfig->AEC) {
	case ISI_AEC_OFF:
		break;
	case ISI_AEC_AUTO:
		break;
	default:
		{
			TRACE(AR230_ERROR, "%s: AEC not supported\n",
				  __func__);
			return RET_NOTSUPP;
		}
	}


	switch(pConfig->DPCC) {
	case ISI_DPCC_OFF:
		break;
	case ISI_DPCC_AUTO:
		break;
	default:
		{
			TRACE(AR230_ERROR, "%s: DPCC not supported\n",
				  __func__);
			return RET_NOTSUPP;
		}
	}

	return result;
}

/*****************************************************************************/
/**
 *          AR230_AecSetModeParameters
 *
 * @brief   This function fills in the correct parameters in AR230-Instances
 *          according to AEC mode selection in IsiSensorConfig_t.
 *
 * @note    It is assumed that IsiSetupOutputWindow has been called before
 *          to fill in correct values in instance structure.
 *
 * @param   handle      AR230 context
 * @param   pConfig     pointer to sensor configuration structure
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT AR230_AecSetModeParameters
	(AR230_Context_t *pAR230Ctx, const IsiSensorConfig_t *pConfig)
{
	RESULT result = RET_SUCCESS;

	TRACE(AR230_INFO, "%s%s (enter) Res: 0x%x  0x%x\n", __func__, pAR230Ctx->isAfpsRun?"(AFPS)":"",
		 pAR230Ctx->Config.Resolution, pConfig->Resolution);

	 if (pAR230Ctx->VtPixClkFreq == 0.0f) {
		 TRACE(AR230_INFO, "%s%s: Division by zero!\n", __func__);
		 return RET_OUTOFRANGE;
	 }

	 pAR230Ctx->AecMaxIntegrationTime = (((float)pAR230Ctx->FrameLengthLines - 4) * (float)pAR230Ctx->LineLengthPck) / pAR230Ctx->VtPixClkFreq;
	 pAR230Ctx->AecMinIntegrationTime = 0.0001f;

	 pAR230Ctx->AecMaxGain = SENSOR_MAX_GAIN_AEC;
	 pAR230Ctx->AecMinGain = 1.0f;

	 pAR230Ctx->AecIntegrationTimeIncrement = ((float)pAR230Ctx->LineLengthPck) / pAR230Ctx->VtPixClkFreq;
	 pAR230Ctx->AecGainIncrement = SENSOR_MIN_GAIN_STEP;

	 pAR230Ctx->AecCurGain = pAR230Ctx->AecMinGain;
	 pAR230Ctx->AecCurIntegrationTime = 0.0f;
	 pAR230Ctx->OldCoarseIntegrationTime = 0;
	 pAR230Ctx->OldFineIntegrationTime = 0;

	 TRACE(AR230_INFO, "%s%s (exit)\n AecMaxIntegrationTime:%f\n AecMinIntegrationTime:%f\n FrameLengthLines:%d\n LineLengthPck:%d\n VtPixClkFreq:%f\n",
	 __func__, pAR230Ctx->isAfpsRun?"(AFPS)":"",
	 pAR230Ctx->AecMaxIntegrationTime,
	 pAR230Ctx->AecMinIntegrationTime,
	 pAR230Ctx->FrameLengthLines,
	 pAR230Ctx->LineLengthPck,
	 pAR230Ctx->VtPixClkFreq);

	 return result;
}

/*****************************************************************************/
/**
 *          AR230_IsiSetupSensorIss
 *
 * @brief   Setup of the image sensor considering the given configuration.
 *
 * @param   handle      AR230 sensor instance handle
 * @param   pConfig     pointer to sensor configuration structure
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT AR230_IsiSetupSensorIss
	(IsiSensorHandle_t handle, const IsiSensorConfig_t *pConfig)
{
	AR230_Context_t *pAR230Ctx = (AR230_Context_t *)handle;

	RESULT result = RET_SUCCESS;

	TRACE(AR230_INFO, "%s (enter)\n", __func__);

	if (pAR230Ctx == NULL) {
		TRACE(AR230_ERROR,
			  "%s: Invalid sensor handle (NULL pointer detected)\n",
			  __func__);
		return RET_WRONG_HANDLE;
	}

	if (pConfig == NULL) {
		TRACE(AR230_ERROR,
			  "%s: Invalid configuration (NULL pointer detected)\n",
			  __func__);
		return RET_NULL_POINTER;
	}

	if (pAR230Ctx->Streaming != BOOL_FALSE) {
		TRACE(AR230_ERROR, "%s: Streaming failed\n", __func__);
		return RET_WRONG_STATE;
	}

	MEMCPY(&pAR230Ctx->Config, pConfig, sizeof(IsiSensorConfig_t));

	result = IsiRegDefaultsApply(pAR230Ctx, AR230_g_aRegDescription);
	if (result != RET_SUCCESS) {
		TRACE(AR230_ERROR, "%s: AR230_g_aRegDescription failed\n",
				__func__);
		return result;
	}

	/* sleep a while, that sensor can take over new default values */
	osSleep(100);

	result = AR230_SetupOutputWindow(pAR230Ctx, pConfig);
	if (result != RET_SUCCESS) {
		TRACE(AR230_ERROR, "%s: SetupOutputWindow failed.\n",
			  __func__);
		return result;
	}

	result = AR230_SetupImageControl(pAR230Ctx, pConfig);
	if (result != RET_SUCCESS) {
		TRACE(AR230_ERROR, "%s: SetupImageControl failed.\n",
			  __func__);
		return result;
	}

	result = AR230_AecSetModeParameters(pAR230Ctx, pConfig);
	if (result != RET_SUCCESS) {
		TRACE(AR230_ERROR, "%s: AecSetModeParameters failed.\n",
			  __func__);
		return result;
	}

	if (result == RET_SUCCESS)
		pAR230Ctx->Configured = BOOL_TRUE;

	TRACE(AR230_INFO, "%s: (exit)\n", __func__);

	return result;
}

/*****************************************************************************/
/**
 *          AR230_IsiChangeSensorResolutionIss
 *
 * @brief   Change image sensor resolution while keeping all other static settings.
 *          Dynamic settings like current exposure, gain & integration time are
 *          kept as close as possible.
 *
 * @note    Re-read current & min/max values as they will probably have changed!
 *
 * @param   handle                  Sensor instance handle
 * @param   Resolution              new resolution ID
 * @param   pNumberOfFramesToSkip   reference to storage for number of frames to skip
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_WRONG_STATE
 * @retval  RET_OUTOFRANGE
 *
 *****************************************************************************/
static RESULT AR230_IsiChangeSensorResolutionIss
	(IsiSensorHandle_t handle,
		uint32_t Resolution, uint8_t *pNumberOfFramesToSkip)
{
	AR230_Context_t *pAR230Ctx = (AR230_Context_t *)handle;

	RESULT result = RET_SUCCESS;

	TRACE(AR230_INFO, "%s (enter)\n", __func__);

	if (pAR230Ctx == NULL)
		return RET_WRONG_HANDLE;
	if (pNumberOfFramesToSkip == NULL)
		return RET_NULL_POINTER;

	if ((pAR230Ctx->Configured != BOOL_TRUE) || (pAR230Ctx->Streaming != BOOL_FALSE))
		return RET_WRONG_STATE;

	IsiSensorCaps_t Caps;
	Caps.Resolution = 0;
	while (AR230_IsiGetCapsIss(handle, &Caps) == RET_SUCCESS) {
		if (Resolution == Caps.Resolution)
			break;
	}

	if ((Resolution & Caps.Resolution) == 0)
		return RET_OUTOFRANGE;

	if (Resolution == pAR230Ctx->Config.Resolution) {
		// well, no need to worry
		*pNumberOfFramesToSkip = 0;
	} else {
		// change resolution
		char *szResName = NULL;
		result = IsiGetResolutionName(Resolution, &szResName);
		TRACE(AR230_DEBUG, "%s: NewRes=0x%08x (%s)\n", __func__, Resolution, szResName);

		// update resolution in copy of config in context
		pAR230Ctx->Config.Resolution = Resolution;

		// tell sensor about that
		result = AR230_SetupOutputWindow(pAR230Ctx, &pAR230Ctx->Config);
		if (result != RET_SUCCESS) {
			TRACE(AR230_ERROR, "%s: SetupOutputWindow failed.\n", __func__);
			return result;
		}

		// remember old exposure values
		float OldGain = pAR230Ctx->AecCurGain;
		float OldIntegrationTime = pAR230Ctx->AecCurIntegrationTime;

		// update limits & stuff (reset current & old settings)
		result = AR230_AecSetModeParameters(pAR230Ctx, &pAR230Ctx->Config);
		if (result != RET_SUCCESS) {
			TRACE( AR230_ERROR, "%s: AecSetModeParameters failed.\n", __func__);
			return result;
		}

		// restore old exposure values (at least within new exposure values' limits)
		uint8_t NumberOfFramesToSkip = 0;
		float	DummySetGain;
		float	DummySetIntegrationTime;
		result = AR230_IsiExposureControlIss(handle, OldGain, OldIntegrationTime, &NumberOfFramesToSkip, &DummySetGain, &DummySetIntegrationTime);
		if (result != RET_SUCCESS) {
			TRACE(AR230_ERROR, "%s: AR230_IsiExposureControlIss failed.\n", __func__);
			return result;
		}

		// return number of frames that aren't exposed correctly
		*pNumberOfFramesToSkip = NumberOfFramesToSkip + 1;
	}

	TRACE(AR230_INFO, "%s (exit)\n", __func__);

	return result;
}

/*****************************************************************************/
/**
 *          AR230_IsiSensorSetStreamingIss
 *
 * @brief   Enables/disables streaming of sensor data, if possible.
 *
 * @param   handle      Sensor instance handle
 * @param   on          new streaming state (BOOL_TRUE=on, BOOL_FALSE=off)
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_WRONG_STATE
 *
 *****************************************************************************/
static RESULT AR230_IsiSensorSetStreamingIss
	(IsiSensorHandle_t handle, bool_t on)
{
	AR230_Context_t *pAR230Ctx = (AR230_Context_t *)handle;

	RESULT result = RET_SUCCESS;

	TRACE(AR230_INFO, "%s (enter)\n", __func__);

	if (pAR230Ctx == NULL)
		return RET_WRONG_HANDLE;

	if ((pAR230Ctx->Configured != BOOL_TRUE) ||
			(pAR230Ctx->Streaming == on))
		return RET_WRONG_STATE;

	result = AR230_IsiRegWriteIss(pAR230Ctx, AR230_RESET_REGISTER,
			on == BOOL_TRUE ? 0x10DC : 0x10D8);

#ifdef AR230_TRIGGER
	result = AR230_IsiRegWriteIss(pAR230Ctx, AR230_RESET_REGISTER,
			on == BOOL_TRUE ? 0x11DC : 0x11D8);
#endif

	if (result == RET_SUCCESS)
		pAR230Ctx->Streaming = on;

	TRACE(AR230_INFO, "%s (exit)\n", __func__);

	return result;
}

#ifdef ISP_FPGA

/******************************************************************************
 * AR230PowerCtrl() - Control AR230 power
 *
 * @void:
 *****************************************************************************/
static RESULT AR230PowerCtrl(void)
{
	return 0;
}

#else

/******************************************************************************
 * AR230PowerCtrl() - Control AR230 power
 *
 * @void:
 *****************************************************************************/
static RESULT AR230PowerCtrl(uint8_t on)
{
	RESULT ret = 0;
	ret |= g_gpio_drv.pfn_set_output(GPIO_PIN_AR0230_AVDD, on);
	ret |= g_gpio_drv.pfn_set_output(GPIO_PIN_AR0230_DVDD, on);
	ret |= g_gpio_drv.pfn_set_output(GPIO_PIN_AR0230_IO, on);
	return ret;
}

#endif

/******************************************************************************
 * AR230Reset() - Reset AR230
 *
 * @void:
 *****************************************************************************/
static RESULT AR230Reset(void)
{
	return g_gpio_drv.pfn_reset(AR0230_RST);
}

/*****************************************************************************/
/**
 *          AR230IsiSensorSetup
 *
 * @brief   Set the camera to idle state or power down
 *
 * @param   handle      AR230 sensor instance handle
 * @param   on          new power state (BOOL_TRUE=on, BOOL_FALSE=off)
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT AR230IsiSensorSetup(IsiSensorHandle_t handle, bool_t on)
{
	AR230_Context_t *pAR230Ctx = (AR230_Context_t *)handle;
	RESULT result = RET_SUCCESS;

	TRACE(AR230_INFO, "%s (enter)\n", __func__);

	if (pAR230Ctx == NULL) {
		result = RET_WRONG_HANDLE;
		goto exit;
	}

	pAR230Ctx->Configured = BOOL_FALSE;

	result = AR230PowerCtrl(on);

	/* not care about power down when failed */
	if (on && !result)
		result = AR230Reset();

exit:
	TRACE(AR230_WARN, "%s %d::ops=%d\n", __func__, result, on);
	TRACE(AR230_INFO, "%s (exit)\n", __func__);

	return result;
}

/*****************************************************************************/
/**
 *          AR230_IsiCheckSensorConnectionIss
 *
 * @brief   Checks the I2C-Connection to sensor by reading sensor revision id.
 *
 * @param   handle      AR230 sensor instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT AR230_IsiCheckSensorConnectionIss(IsiSensorHandle_t handle)
{
	uint32_t RevId;
	uint32_t value = 0UL;

	RESULT result = RET_SUCCESS;

	TRACE(AR230_INFO, "%s (enter)\n", __func__);

	if (handle == NULL)
		return RET_WRONG_HANDLE;

	RevId = AR230_DVP_CHIP_VERSION;

	result = AR230_IsiGetSensorRevisionIss(handle, &value);
	if ((result != RET_SUCCESS) || (RevId != value)) {
		TRACE(AR230_ERROR, "%s RevId = 0x%08x, value = 0x%08x\n",
				__func__, RevId, value);
		return RET_FAILURE;
	}
	TRACE(AR230_DEBUG, "%s RevId = 0x%08x, value = 0x%08x\n",
				__func__, RevId, value);
	TRACE(AR230_INFO, "%s (exit)\n", __func__);

	return result;
}

/*****************************************************************************/
/**
 *          AR230_IsiGetSensorRevisionIss
 *
 * @brief   reads the sensor revision register and returns this value
 *
 * @param   handle      pointer to sensor description struct
 * @param   p_value     pointer to storage value
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT AR230_IsiGetSensorRevisionIss
	(IsiSensorHandle_t handle, uint32_t *p_value)
{
	RESULT result = RET_SUCCESS;

	TRACE(AR230_INFO, "%s (enter)\n", __func__);

	if (handle == NULL)
		return RET_WRONG_HANDLE;

	if (p_value == NULL)
		return RET_NULL_POINTER;

	*p_value = 0U;
	result = AR230_IsiRegReadIss(handle, CHIP_VERSION, p_value);

	TRACE(AR230_INFO, "%s (exit)\n", __func__);

	return result;
}

/*****************************************************************************/
/**
 *          AR230_IsiRegReadIss
 *
 * @brief   grants user read access to the camera register
 *
 * @param   handle      pointer to sensor description struct
 * @param   address     sensor register to write
 * @param   p_value     pointer to value
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT AR230_IsiRegReadIss
	(IsiSensorHandle_t handle, const uint32_t address, uint32_t *p_value)
{
	RESULT result = RET_SUCCESS;

	TRACE(AR230_INFO, "%s (enter)\n", __func__);

	if (handle == NULL)
		return RET_WRONG_HANDLE;

	if (p_value == NULL) {
		return RET_NULL_POINTER;
	} else {
		uint8_t NrOfBytes =
			IsiGetNrDatBytesIss(address, AR230_g_aRegDescription);
		if (!NrOfBytes) {
			NrOfBytes = 2;	//i2c read16
		}

		*p_value = 0U;

		result = IsiI2cReadRegister(handle, address, p_value, NrOfBytes,
				BOOL_TRUE);
	}

	TRACE(AR230_DEBUG, "%s (exit: 0x%08x 0x%08x ret=%d)\n", __func__,
			address, *p_value, result);

	return result;
}

/*****************************************************************************/
/**
 *          AR230_IsiRegWriteIss
 *
 * @brief   grants user write access to the camera register
 *
 * @param   handle      pointer to sensor description struct
 * @param   address     sensor register to write
 * @param   value       value to write
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 *
 *****************************************************************************/
static RESULT AR230_IsiRegWriteIss
	(IsiSensorHandle_t handle, uint32_t address, uint32_t value)
{
	RESULT result = RET_SUCCESS;

	uint8_t NrOfBytes;

	TRACE(AR230_INFO, "%s (enter)\n", __func__);

	if (handle == NULL)
		return RET_WRONG_HANDLE;

	NrOfBytes = IsiGetNrDatBytesIss(address, AR230_g_aRegDescription);
	if (!NrOfBytes) {
		NrOfBytes = 2;	//i2c write16
	}

	result =
		IsiI2cWriteRegister(handle, address, &value, NrOfBytes, BOOL_TRUE);
	TRACE(AR230_INFO, "%s (exit: 0x%08x 0x%08x)\n", __func__, address,
		  value);

	return result;
}

/*****************************************************************************/
/**
 *          AR230_IsiGetGainLimitsIss
 *
 * @brief   Returns the exposure minimal and maximal values of an
 *          AR230 instance
 *
 * @param   handle       AR230 sensor instance handle
 * @param   pMinExposure Pointer to a variable receiving minimal exposure value
 * @param   pMaxExposure Pointer to a variable receiving maximal exposure value
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT AR230_IsiGetGainLimitsIss
	(IsiSensorHandle_t handle, float *pMinGain, float *pMaxGain)
{
	AR230_Context_t *pAR230Ctx = (AR230_Context_t *)handle;

	RESULT result = RET_SUCCESS;

	TRACE(AR230_INFO, "%s: (enter)\n", __func__);

	if (pAR230Ctx == NULL) {
		TRACE(AR230_ERROR,
			  "%s: Invalid sensor handle (NULL pointer detected)\n",
			  __func__);
		return RET_WRONG_HANDLE;
	}

	if ((pMinGain == NULL) || (pMaxGain == NULL)) {
		TRACE(AR230_ERROR, "%s: NULL pointer received!!\n");
		return RET_NULL_POINTER;
	}

	*pMinGain = pAR230Ctx->AecMinGain;
	*pMaxGain = pAR230Ctx->AecMaxGain;

	TRACE(AR230_INFO, "%s: (enter)\n", __func__);

	return result;
}

/*****************************************************************************/
/**
 *          AR230_IsiGetIntegrationTimeLimitsIss
 *
 * @brief   Returns the minimal and maximal integration time values of an
 *          AR230 instance
 *
 * @param   handle       AR230 sensor instance handle
 * @param   pMinExposure Pointer to a variable receiving minimal exposure value
 * @param   pMaxExposure Pointer to a variable receiving maximal exposure value
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT AR230_IsiGetIntegrationTimeLimitsIss
	(IsiSensorHandle_t handle,
	 float *pMinIntegrationTime, float *pMaxIntegrationTime)
{
	AR230_Context_t *pAR230Ctx = (AR230_Context_t *)handle;

	RESULT result = RET_SUCCESS;

	TRACE(AR230_INFO, "%s: (enter)\n", __func__);

	if (pAR230Ctx == NULL) {
		TRACE(AR230_ERROR,
			  "%s: Invalid sensor handle (NULL pointer detected)\n",
			  __func__);
		return RET_WRONG_HANDLE;
	}

	if ((pMinIntegrationTime == NULL) || (pMaxIntegrationTime == NULL)) {
		TRACE(AR230_ERROR, "%s: NULL pointer received!!\n");
		return RET_NULL_POINTER;
	}

	*pMinIntegrationTime = pAR230Ctx->AecMinIntegrationTime;
	*pMaxIntegrationTime = pAR230Ctx->AecMaxIntegrationTime;

	TRACE(AR230_INFO, "%s: (enter)\n", __func__);

	return result;
}

/*****************************************************************************/
/**
 *          AR230_IsiGetGainIncrementIss
 *
 * @brief   Get smallest possible gain increment.
 *
 * @param   handle                  AR230 sensor instance handle
 * @param   pIncr                   increment
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT AR230_IsiGetGainIncrementIss(IsiSensorHandle_t handle, float *pIncr)
{
	AR230_Context_t *pAR230Ctx = (AR230_Context_t *)handle;

	RESULT result = RET_SUCCESS;

	TRACE(AR230_INFO, "%s: (enter)\n", __func__);

	if (pAR230Ctx == NULL) {
		TRACE(AR230_ERROR,
			  "%s: Invalid sensor handle (NULL pointer detected)\n",
			  __func__);
		return RET_WRONG_HANDLE;
	}

	if (pIncr == NULL)
		return RET_NULL_POINTER;
	//_smallest_ increment the sensor/driver can handle (e.g. used for sliders in the application)
	*pIncr = pAR230Ctx->AecGainIncrement;

	TRACE(AR230_INFO, "%s: (exit)\n", __func__);

	return result;
}

/*****************************************************************************/
/**
 *          AR230_IsiExposureControlIss
 *
 * @brief   Camera hardware dependent part of the exposure control loop.
 *          Calculates appropriate register settings from the new exposure
 *          values and writes them to the image sensor module.
 *
 * @param   handle                  AR230 sensor instance handle
 * @param   NewGain                 newly calculated gain to be set
 * @param   NewIntegrationTime      newly calculated integration time to be set
 * @param   pNumberOfFramesToSkip   number of frames to skip until AE is
 *                                  executed again
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT AR230_IsiExposureControlIss
	(IsiSensorHandle_t handle,
	 float NewGain,
	 float NewIntegrationTime,
	 uint8_t *pNumberOfFramesToSkip,
	 float *pSetGain, float *pSetIntegrationTime)
{
	AR230_Context_t *pAR230Ctx = (AR230_Context_t *)handle;

	RESULT result = RET_SUCCESS;

	TRACE(AR230_INFO, "%s: (enter)\n", __func__);

	if (pAR230Ctx == NULL) {
		TRACE(AR230_ERROR,
			  "%s: Invalid sensor handle (NULL pointer detected)\n",
			  __func__);
		return RET_WRONG_HANDLE;
	}

	if ((pNumberOfFramesToSkip == NULL)
		|| (pSetGain == NULL)
		|| (pSetIntegrationTime == NULL)) {
		TRACE(AR230_ERROR,
			  "%s: Invalid parameter (NULL pointer detected)\n",
			  __func__);
		return RET_NULL_POINTER;
	}

	TRACE(AR230_DEBUG, "%s: g=%f, Ti=%f\n", __func__, NewGain,
		  NewIntegrationTime);

	pAR230Ctx->GroupHold = BOOL_TRUE;
	result = AR230_IsiSetIntegrationTimeIss(handle,
						NewIntegrationTime,
						pSetIntegrationTime,
						pNumberOfFramesToSkip);
	result = AR230_IsiSetGainIss(handle, NewGain, pSetGain);
	pAR230Ctx->GroupHold = BOOL_FALSE;

	TRACE(AR230_DEBUG, "%s: set: g=%f, Ti=%f, skip=%d\n", __func__, *pSetGain, *pSetIntegrationTime, *pNumberOfFramesToSkip);
	TRACE(AR230_INFO, "%s: (exit)\n", __func__);

	return result;
}

/*****************************************************************************/
/**
 *          AR230_IsiSetIntegrationTimeIss
 *
 * @brief   Writes gain and integration time values to the image sensor module.
 *
 * @param   handle                  AR230 sensor instance handle
 * @param   NewIntegrationTime      integration time to be set
 * @param   pSetIntegrationTime     set integration time
 * @param   pNumberOfFramesToSkip   number of frames to skip until AE is
 *                                  executed again
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT AR230_IsiSetIntegrationTimeIss
	(IsiSensorHandle_t handle,
	 float NewIntegrationTime,
	 float *pSetIntegrationTime,
	 uint8_t *pNumberOfFramesToSkip)
{
	AR230_Context_t *pAR230Ctx = (AR230_Context_t *)handle;

	RESULT result = RET_SUCCESS;

	uint32_t CoarseIntegrationTime = 0;
	float ShutterWidthPck = 0.0f;

	TRACE(AR230_INFO, "%s: (enter)\n", __func__);

	if (pAR230Ctx == NULL) {
		TRACE(AR230_ERROR,
			  "%s: Invalid sensor handle(NULL pointer detected)\n",
			  __func__);
		return RET_WRONG_HANDLE;
	}

	if (pSetIntegrationTime == NULL || pNumberOfFramesToSkip == NULL) {
		TRACE(AR230_ERROR, "%s: NULL pointer received!!\n",
			  __func__);
		return RET_NULL_POINTER;
	}

	if (NewIntegrationTime < pAR230Ctx->AecMinIntegrationTime)
		NewIntegrationTime = pAR230Ctx->AecMinIntegrationTime;
	if (NewIntegrationTime > pAR230Ctx->AecMaxIntegrationTime)
		NewIntegrationTime = pAR230Ctx->AecMaxIntegrationTime;

	if (pAR230Ctx->LineLengthPck == 0) {
		TRACE(AR230_ERROR, "%s: Division by zero!\n", __func__);
		return RET_DIVISION_BY_ZERO;
	}

	ShutterWidthPck = NewIntegrationTime * ((float)pAR230Ctx->VtPixClkFreq);
	CoarseIntegrationTime =
		(uint32_t)(ShutterWidthPck /
		((float)pAR230Ctx->LineLengthPck) + 0.5f);
	if (CoarseIntegrationTime != pAR230Ctx->OldCoarseIntegrationTime) {
		result = AR230_IsiRegWriteIss(handle, COARSE_INTEGRATION_TIME,
					 CoarseIntegrationTime);
		TRACE(AR230_ERROR, "%s: exposure is setting %d !\n", __func__, CoarseIntegrationTime);
		RETURN_RESULT_IF_DIFFERENT(RET_SUCCESS, result);
		pAR230Ctx->OldCoarseIntegrationTime = CoarseIntegrationTime;
		*pNumberOfFramesToSkip = 1;
	} else {
		*pNumberOfFramesToSkip = 0;
	}

	pAR230Ctx->AecCurIntegrationTime =
		((float)CoarseIntegrationTime * (float)pAR230Ctx->LineLengthPck) /
		pAR230Ctx->VtPixClkFreq;
	*pSetIntegrationTime = pAR230Ctx->AecCurIntegrationTime;

	TRACE(AR230_DEBUG, "%s: Ti=%f TimeReg\n", __func__,
			*pSetIntegrationTime, CoarseIntegrationTime);
	TRACE(AR230_INFO, "%s: (exit)\n", __func__);

	return result;
}

/*****************************************************************************/
/**
 *          AR230_IsiGetGainIss
 *
 * @brief   Reads gain values from the image sensor module.
 *
 * @param   handle                  AR230 sensor instance handle
 * @param   pSetGain                set gain
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT AR230_IsiGetGainIss(IsiSensorHandle_t handle, float *pSetGain)
{
	AR230_Context_t *pAR230Ctx = (AR230_Context_t *)handle;

	RESULT result = RET_SUCCESS;

	TRACE(AR230_INFO, "%s: (enter)\n", __func__);

	if (pAR230Ctx == NULL) {
		TRACE(AR230_ERROR,
			  "%s: Invalid sensor handle (NULL pointer detected)\n",
			  __func__);
		return RET_WRONG_HANDLE;
	}

	if (pSetGain == NULL)
		return RET_NULL_POINTER;

	*pSetGain = pAR230Ctx->AecCurGain;

	TRACE(AR230_INFO, "%s: (exit)\n", __func__);

	return result;
}

/*****************************************************************************/
/**
 *          AR230_IsiGetResolutionIss
 *
 * @brief   Reads integration time values from the image sensor module.
 *
 * @param   handle                  sensor instance handle
 * @param   pSettResolution         set resolution
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT AR230_IsiGetResolutionIss
	(IsiSensorHandle_t handle, uint32_t *pSetResolution)
{
	AR230_Context_t *pAR230Ctx = (AR230_Context_t *)handle;

	RESULT result = RET_SUCCESS;

	TRACE(AR230_INFO, "%s: (enter)\n", __func__);

	if (pAR230Ctx == NULL) {
		TRACE(AR230_ERROR,
			  "%s: Invalid sensor handle (NULL pointer detected)\n",
			  __func__);
		return RET_WRONG_HANDLE;
	}

	if (pSetResolution == NULL)
		return RET_NULL_POINTER;

	*pSetResolution = pAR230Ctx->Config.Resolution;

	TRACE(AR230_INFO, "%s: (enter)\n", __func__);

	return result;
}

/*****************************************************************************/
/**
 *          AR230_IsiSetGainIss
 *
 * @brief   Writes gain values to the image sensor module.
 *
 * @param   handle                  AR230 sensor instance handle
 * @param   NewGain                 gain to be set
 * @param   pSetGain                set gain
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT AR230_IsiSetGainIss
	(IsiSensorHandle_t handle, float NewGain, float *pSetGain)
{
	AR230_Context_t *pAR230Ctx = (AR230_Context_t *)handle;

	RESULT result = RET_SUCCESS;
	uint16_t usGain = 0;
	float calc = 0;

	TRACE(AR230_INFO, "%s: (enter)\n", __func__);

	if (pAR230Ctx == NULL) {
		TRACE(AR230_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __func__);
		return RET_WRONG_HANDLE;
	}

	if (pSetGain == NULL) {
		TRACE(AR230_ERROR, "%s: Invalid parameter (NULL pointer detected)\n", __func__);
		return RET_NULL_POINTER;
	}

	if (NewGain < pAR230Ctx->AecMinGain)
		NewGain = pAR230Ctx->AecMinGain;
	if (NewGain > pAR230Ctx->AecMaxGain)
		NewGain = pAR230Ctx->AecMaxGain;

	usGain = (uint16_t)((sqrt(0.02078 * 0.02078 - 4.0 * 0.002404 * (1.012 - NewGain)) - 0.02078) / (2.0 * 0.002404) + 0.5);
	usGain = usGain & 0x003f;

	//write new gain into sensor registers, do not write if nothing has changed
	result = AR230_IsiRegWriteIss(handle, AR230_AGC, usGain);
	RETURN_RESULT_IF_DIFFERENT(RET_SUCCESS, result);

	calc = (float)usGain;
	pAR230Ctx->AecCurGain = 0.002404 * calc * calc + 0.02078 * calc + 1.012;

	//return current state
	*pSetGain = pAR230Ctx->AecCurGain;

	TRACE(AR230_INFO, "%s:NewGain:%f pSetGain:%f usGain:%f\n", __FUNCTION__, NewGain, *pSetGain, usGain);

	TRACE(AR230_INFO, "%s: (exit)\n", __FUNCTION__);

	return result;
}

/*****************************************************************************/
/**
 *          AR230_IsiGetIntegrationTimeIss
 *
 * @brief   Reads integration time values from the image sensor module.
 *
 * @param   handle                  AR230 sensor instance handle
 * @param   pSetIntegrationTime     set integration time
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT AR230_IsiGetIntegrationTimeIss
	(IsiSensorHandle_t handle, float *pSetIntegrationTime)
{
	AR230_Context_t *pAR230Ctx = (AR230_Context_t *)handle;

	RESULT result = RET_SUCCESS;

	TRACE(AR230_INFO, "%s: (enter)\n", __func__);

	if (pAR230Ctx == NULL) {
		TRACE(AR230_ERROR,
			  "%s: Invalid sensor handle (NULL pointer detected)\n",
			  __func__);
		return RET_WRONG_HANDLE;
	}

	if (pSetIntegrationTime == NULL)
		return RET_NULL_POINTER;

	*pSetIntegrationTime = pAR230Ctx->AecCurIntegrationTime;

	TRACE(AR230_INFO, "%s: (exit)\n", __func__);

	return result;
}

/*****************************************************************************/
/**
 *          AR230_IsiGetIntegrationTimeIncrementIss
 *
 * @brief   Get smallest possible integration time increment.
 *
 * @param   handle                  AR230 sensor instance handle
 * @param   pIncr                   increment
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT AR230_IsiGetIntegrationTimeIncrementIss
	(IsiSensorHandle_t handle, float *pIncr)
{
	AR230_Context_t *pAR230Ctx = (AR230_Context_t *)handle;

	RESULT result = RET_SUCCESS;

	TRACE(AR230_INFO, "%s: (enter)\n", __func__);

	if (pAR230Ctx == NULL) {
		TRACE(AR230_ERROR,
			  "%s: Invalid sensor handle (NULL pointer detected)\n",
			  __func__);
		return RET_WRONG_HANDLE;
	}

	if (pIncr == NULL)
		return RET_NULL_POINTER;
	*pIncr = pAR230Ctx->AecIntegrationTimeIncrement;

	TRACE(AR230_INFO, "%s: (enter)\n", __func__);

	return result;
}

/*****************************************************************************/
/**
*          AR230_IsiGetAfpsInfoHelperIss
*
* @brief   Calc AFPS sub resolution settings for thee given resolution
*
* @param  pAR230Ctx           AR230 sensor instance (dummy!) xontext
* @param  Resolution          Any supported resolution to query AFPS params for
* @param  pAfpsInfo           Reference of AFPS info structure to write the
*                             results to
* @param  AfpsStageIdx        Index of current AFPS stage to use
*
* @return  Return the result of the function call.
* @retval  RET_SUCCESS
******************************************************************************/
/*
static RESULT AR230_IsiGetAfpsInfoHelperIss(
	AR230_Context_t *pAR230Ctx,
	uint32_t         Resolution,
	IsiAfpsInfo_t   *pAfpsInfo,
	uint32_t         AfpsStageIdx
)
{
	RESULT result = RET_SUCCESS;

	TRACE(AR230_INFO, "%s (enter)\n", __func__);
	if (pAR230Ctx == NULL) {
		TRACE(AR230_ERROR,
			  "%s: Invalid sensor handle (NULL pointer detected)\n",
			  __func__);
		return RET_WRONG_HANDLE;
	}

	if (pAfpsInfo == NULL)
		return RET_NULL_POINTER;

	if (AfpsStageIdx <= ISI_NUM_AFPS_STAGES)
		return RET_OUTOFRANGE;

	// update resolution in copy of config in context
	pAR230Ctx->Config.Resolution = Resolution;

	// tell sensor about that
	result = AR230_SetupOutputWindow(pAR230Ctx, &pAR230Ctx->Config);
	if (result != RET_SUCCESS) {
		TRACE(AR230_ERROR,
		"%s: SetupOutputWindow failed for Resolution ID0x%08x.\n",
		__func__, Resolution);
		return result;
	}

	// update limit & stuff (reset current & old settings)
	result = AR230_AecSetModeParameters(pAR230Ctx, &pAR230Ctx->Config);
	if (result != RET_SUCCESS) {
		TRACE(AR230_ERROR,
		"%s: AecSetModeParamters failed for Resolution ID 0x%08x.\n",
		__func__, Resolution);
		return result;
	}

	// take over params
	pAfpsInfo->Stage[AfpsStageIdx].Resolution = Resolution;
	pAfpsInfo->Stage[AfpsStageIdx].MaxIntTime =
					pAR230Ctx->AecMaxIntegrationTime;
	pAfpsInfo->AecMinGain           = pAR230Ctx->AecMinGain;
	pAfpsInfo->AecMaxGain           = pAR230Ctx->AecMaxGain;
	pAfpsInfo->AecMinIntTime        = pAR230Ctx->AecMinIntegrationTime;
	pAfpsInfo->AecMaxIntTime        = pAR230Ctx->AecMaxIntegrationTime;
	pAfpsInfo->AecSlowestResolution = Resolution;

	TRACE(AR230_INFO, "%s (exit)\n", __func__);
	return result;
}
*/

/*****************************************************************************/
/**
 *          AR230_IsiGetAfpsInfoIss
 *
 * @brief   Returns the possible AFPS sub resolution settings for the given resolution series
 *
 * @param   handle                  AR230 sensor instance handle
 * @param   Resolution              Any resolution within the AFPS group to query;
 *                                  0 (zero) to use the currently configured resolution
 * @param   pAfpsInfo               Reference of AFPS info structure to store the results
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 * @retval  RET_NOTSUPP
 *
 *****************************************************************************/
RESULT AR230_IsiGetAfpsInfoIss(IsiSensorHandle_t handle,
				   uint32_t Resolution, IsiAfpsInfo_t *pAfpsInfo)
{
	AR230_Context_t *pAR230Ctx = (AR230_Context_t *)handle;

	RESULT result = RET_SUCCESS;

	TRACE(AR230_INFO, "%s: (enter)\n", __func__);

	if (pAR230Ctx == NULL) {
		TRACE(AR230_ERROR,
			  "%s: Invalid sensor handle (NULL pointer detected)\n",
			  __func__);
		return RET_WRONG_HANDLE;
	}

	if (pAfpsInfo == NULL)
		return RET_NULL_POINTER;
	// use currently configured resolution?
	if (Resolution == 0)
		Resolution = pAR230Ctx->Config.Resolution;

	TRACE(AR230_INFO, "%s: (exit)\n", __func__);

	return result;
}

/*****************************************************************************/
/**
 *          AR230_IsiGetCurrentExposureIss
 *
 * @brief   Returns the currently adjusted AE values
 *
 * @param   handle                  AR230 sensor instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT AR230_IsiGetCurrentExposureIss
	(IsiSensorHandle_t handle, float *pSetGain, float *pSetIntegrationTime)
{
	AR230_Context_t *pAR230Ctx = (AR230_Context_t *)handle;

	RESULT result = RET_SUCCESS;

	TRACE(AR230_INFO, "%s: (enter)\n", __func__);

	if (pAR230Ctx == NULL) {
		TRACE(AR230_ERROR,
			  "%s: Invalid sensor handle (NULL pointer detected)\n",
			  __func__);
		return RET_WRONG_HANDLE;
	}

	if ((pSetGain == NULL) || (pSetIntegrationTime == NULL))
		return RET_NULL_POINTER;

	*pSetGain = pAR230Ctx->AecCurGain;
	*pSetIntegrationTime = pAR230Ctx->AecCurIntegrationTime;

	TRACE(AR230_INFO, "%s: (enter)\n", __func__);

	return result;
}

/*****************************************************************************/
/**
 *          AR230_IsiGetCalibKFactor
 *
 * @brief   Returns the AR230 specific K-Factor
 *
 * @param   handle       AR230 sensor instance handle
 * @param   pIsiKFactor  Pointer to Pointer receiving the memory address
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT AR230_IsiGetCalibKFactor
	(IsiSensorHandle_t handle, Isi1x1FloatMatrix_t **pIsiKFactor)
{
	AR230_Context_t *pAR230Ctx = (AR230_Context_t *)handle;

	RESULT result = RET_SUCCESS;

	TRACE(AR230_INFO, "%s: (enter)\n", __func__);

	if (pAR230Ctx == NULL)
		return RET_WRONG_HANDLE;

	if (pIsiKFactor == NULL)
		return RET_NULL_POINTER;

	*pIsiKFactor = (Isi1x1FloatMatrix_t *)&AR230_KFactor;

	TRACE(AR230_INFO, "%s: (exit)\n", __func__);

	return result;
}

/*****************************************************************************/
/**
 *          AR230_IsiGetCalibPcaMatrix
 *
 * @brief   Returns the AR230 specific PCA-Matrix
 *
 * @param   handle          AR230 sensor instance handle
 * @param   pIsiPcaMatrix   Pointer to Pointer receiving the memory address
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT AR230_IsiGetCalibPcaMatrix
	(IsiSensorHandle_t handle, Isi3x2FloatMatrix_t **pIsiPcaMatrix)
{
	AR230_Context_t *pAR230Ctx = (AR230_Context_t *)handle;

	RESULT result = RET_SUCCESS;

	TRACE(AR230_INFO, "%s: (enter)\n", __func__);

	if (pAR230Ctx == NULL)
		return RET_WRONG_HANDLE;

	if (pIsiPcaMatrix == NULL)
		return RET_NULL_POINTER;

	*pIsiPcaMatrix = (Isi3x2FloatMatrix_t *)&AR230_PCAMatrix;

	TRACE(AR230_INFO, "%s: (exit)\n", __func__);

	return result;
}

/*****************************************************************************/
/**
 *          AR230_IsiGetCalibSvdMeanValue
 *
 * @brief   Returns the sensor specific SvdMean-Vector
 *
 * @param   handle              AR230 sensor instance handle
 * @param   pIsiSvdMeanValue    Pointer to Pointer receiving the memory address
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT AR230_IsiGetCalibSvdMeanValue
	(IsiSensorHandle_t handle, Isi3x1FloatMatrix_t **pIsiSvdMeanValue)
{
	AR230_Context_t *pAR230Ctx = (AR230_Context_t *)handle;

	RESULT result = RET_SUCCESS;

	TRACE(AR230_INFO, "%s: (enter)\n", __func__);

	if (pAR230Ctx == NULL)
		return RET_WRONG_HANDLE;

	if (pIsiSvdMeanValue == NULL)
		return RET_NULL_POINTER;

	*pIsiSvdMeanValue = (Isi3x1FloatMatrix_t *)&AR230_SVDMeanValue;

	TRACE(AR230_INFO, "%s: (exit)\n", __func__);

	return result;
}

/*****************************************************************************/
/**
 *          AR230_IsiGetCalibCenterLine
 *
 * @brief   Returns a pointer to the sensor specific centerline, a straight
 *          line in Hesse normal form in Rg/Bg colorspace
 *
 * @param   handle              AR230 sensor instance handle
 * @param   pIsiSvdMeanValue    Pointer to Pointer receiving the memory address
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT AR230_IsiGetCalibCenterLine
	(IsiSensorHandle_t handle, IsiLine_t **ptIsiCenterLine)
{
	AR230_Context_t *pAR230Ctx = (AR230_Context_t *)handle;

	RESULT result = RET_SUCCESS;

	TRACE(AR230_INFO, "%s: (enter)\n", __func__);

	if (pAR230Ctx == NULL)
		return RET_WRONG_HANDLE;

	if (ptIsiCenterLine == NULL)
		return RET_NULL_POINTER;

	*ptIsiCenterLine = (IsiLine_t *)&AR230_CenterLine;

	TRACE(AR230_INFO, "%s: (exit)\n", __func__);

	return result;
}

/*****************************************************************************/
/**
 *          AR230_IsiGetCalibClipParam
 *
 * @brief   Returns a pointer to the sensor specific arrays for Rg/Bg color
 *          space clipping
 *
 * @param   handle              AR230 sensor instance handle
 * @param   pIsiSvdMeanValue    Pointer to Pointer receiving the memory address
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT AR230_IsiGetCalibClipParam
	(IsiSensorHandle_t handle, IsiAwbClipParm_t **pIsiClipParam)
{
	AR230_Context_t *pAR230Ctx = (AR230_Context_t *)handle;

	RESULT result = RET_SUCCESS;

	TRACE(AR230_INFO, "%s: (enter)\n", __func__);

	if (pAR230Ctx == NULL)
		return RET_WRONG_HANDLE;

	if (pIsiClipParam == NULL)
		return RET_NULL_POINTER;

	*pIsiClipParam = (IsiAwbClipParm_t *)&AR230_AwbClipParm;

	TRACE(AR230_INFO, "%s: (exit)\n", __func__);

	return result;
}

/*****************************************************************************/
/**
 *          AR230_IsiGetCalibGlobalFadeParam
 *
 * @brief   Returns a pointer to the sensor specific arrays for AWB out of
 *          range handling
 *
 * @param   handle              AR230 sensor instance handle
 * @param   pIsiSvdMeanValue    Pointer to Pointer receiving the memory address
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT AR230_IsiGetCalibGlobalFadeParam
	(IsiSensorHandle_t handle, IsiAwbGlobalFadeParm_t **ptIsiGlobalFadeParam)
{
	AR230_Context_t *pAR230Ctx = (AR230_Context_t *)handle;

	RESULT result = RET_SUCCESS;

	TRACE(AR230_INFO, "%s: (enter)\n", __func__);

	if (pAR230Ctx == NULL)
		return RET_WRONG_HANDLE;

	if (ptIsiGlobalFadeParam == NULL)
		return RET_NULL_POINTER;

	*ptIsiGlobalFadeParam =
		(IsiAwbGlobalFadeParm_t *)&AR230_AwbGlobalFadeParm;

	TRACE(AR230_INFO, "%s: (exit)\n", __func__);

	return result;
}

/*****************************************************************************/
/**
 *          AR230_IsiGetCalibFadeParam
 *
 * @brief   Returns a pointer to the sensor specific arrays for near white
 *          pixel parameter calculations
 *
 * @param   handle              AR230 sensor instance handle
 * @param   pIsiSvdMeanValue    Pointer to Pointer receiving the memory address
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT AR230_IsiGetCalibFadeParam
	(IsiSensorHandle_t handle, IsiAwbFade2Parm_t **ptIsiFadeParam)
{
	AR230_Context_t *pAR230Ctx = (AR230_Context_t *)handle;

	RESULT result = RET_SUCCESS;

	TRACE(AR230_INFO, "%s: (enter)\n", __func__);

	if (pAR230Ctx == NULL)
		return RET_WRONG_HANDLE;

	if (ptIsiFadeParam == NULL)
		return RET_NULL_POINTER;

	*ptIsiFadeParam = (IsiAwbFade2Parm_t *)&AR230_AwbFade2Parm;

	TRACE(AR230_INFO, "%s: (exit)\n", __func__);

	return result;
}

/*****************************************************************************/
/**
 *          AR230_IsiGetIlluProfile
 *
 * @brief   Returns a pointer to illumination profile idetified by CieProfile
 *          bitmask
 *
 * @param   handle              sensor instance handle
 * @param   CieProfile
 * @param   ptIsiIlluProfile    Pointer to Pointer receiving the memory address
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT AR230_IsiGetIlluProfile
	(IsiSensorHandle_t handle,
	 const uint32_t CieProfile, IsiIlluProfile_t **ptIsiIlluProfile)
{
	AR230_Context_t *pAR230Ctx = (AR230_Context_t *)handle;

	RESULT result = RET_SUCCESS;

	TRACE(AR230_INFO, "%s: (enter)\n", __func__);

	if (pAR230Ctx == NULL)
		return RET_WRONG_HANDLE;

	if (ptIsiIlluProfile == NULL) {
		return RET_NULL_POINTER;
	} else {
		uint16_t i;

		*ptIsiIlluProfile = NULL;

		/* check if we've a default profile */
		for (i = 0U; i < AR230_ISIILLUPROFILES_DEFAULT; i++) {
			if (AR230_IlluProfileDefault[i].id == CieProfile) {
				*ptIsiIlluProfile =
					&AR230_IlluProfileDefault[i];
				break;
			}
		}

		result =
			(*ptIsiIlluProfile !=
			 NULL) ? RET_SUCCESS : RET_NOTAVAILABLE;
	}

	TRACE(AR230_INFO, "%s: (exit)\n", __func__);

	return result;
}

/*****************************************************************************/
/**
 *          AR230_IsiGetLscMatrixTable
 *
 * @brief   Returns a pointer to illumination profile idetified by CieProfile
 *          bitmask
 *
 * @param   handle              sensor instance handle
 * @param   CieProfile
 * @param   ptIsiIlluProfile    Pointer to Pointer receiving the memory address
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT AR230_IsiGetLscMatrixTable
	(IsiSensorHandle_t handle,
	 const uint32_t CieProfile, IsiLscMatrixTable_t **pLscMatrixTable)
{
	AR230_Context_t *pAR230Ctx = (AR230_Context_t *)handle;

	RESULT result = RET_SUCCESS;

	TRACE(AR230_INFO, "%s: (enter)\n", __func__);

	if (pAR230Ctx == NULL)
		return RET_WRONG_HANDLE;

	if (pLscMatrixTable == NULL) {
		return RET_NULL_POINTER;
	} else {
		switch (CieProfile) {
		case ISI_CIEPROF_A:
		default:
			{
				if ((pAR230Ctx->Config.Resolution ==
					 ISI_RES_TV720P30)
					|| (pAR230Ctx->Config.Resolution ==
					ISI_RES_TV720P60)) {
					*pLscMatrixTable =
						&AR230_LscMatrixTable_CIE_A_1280x720;
				} else
					if ((pAR230Ctx->Config.Resolution ==
					 ISI_RES_TV1080P24)
					|| (pAR230Ctx->Config.Resolution ==
						ISI_RES_TV1080P10)) {
					*pLscMatrixTable =
						&AR230_LscMatrixTable_CIE_A_1920x1080;
				} else if (pAR230Ctx->Config.Resolution ==
					   ISI_RES_2592_1944) {
					*pLscMatrixTable =
						&AR230_LscMatrixTable_CIE_A_2592x1944;
				} else {
					TRACE(AR230_ERROR,
						  "%s: Resolution not supported\n",
						  __func__);
					*pLscMatrixTable = NULL;
				}

				break;
			}

#if 0
		default:
			{
				TRACE(AR230_ERROR,
					  "%s: Illumination not supported\n",
					  __func__);
				*pLscMatrixTable = NULL;
				break;
			}
#endif
		}

		result =
			(*pLscMatrixTable != NULL) ? RET_SUCCESS : RET_NOTAVAILABLE;
	}

	TRACE(AR230_INFO, "%s: (exit)\n", __func__);

	return result;
}

/*****************************************************************************/
/**
 *          AR230_IsiActivateTestPattern
 *
 * @brief   Triggers a forced calibration of the focus hardware.
 *
 * @param   handle          AR230 sensor instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT AR230_IsiActivateTestPattern(IsiSensorHandle_t handle,
					   const bool_t enable)
{
	AR230_Context_t *pAR230Ctx = (AR230_Context_t *)handle;

	RESULT result = RET_SUCCESS;

	uint32_t ulRegValue = 0UL;

	TRACE(AR230_INFO, "%s: (enter)\n", __func__);

	if (pAR230Ctx == NULL)
		return RET_WRONG_HANDLE;

	if (enable == BOOL_TRUE) {
		/* enable test-pattern */
		result =
			AR230_IsiRegReadIss(pAR230Ctx, AR230_Test_Pattern,
					&ulRegValue);
		RETURN_RESULT_IF_DIFFERENT(RET_SUCCESS, result);

		ulRegValue |= 0x02U;

		result =
			AR230_IsiRegWriteIss(pAR230Ctx, AR230_Test_Pattern,
					 ulRegValue);
		RETURN_RESULT_IF_DIFFERENT(RET_SUCCESS, result);
	} else {
		/* disable test-pattern */
		result =
			AR230_IsiRegReadIss(pAR230Ctx, AR230_Test_Pattern,
					&ulRegValue);
		RETURN_RESULT_IF_DIFFERENT(RET_SUCCESS, result);

		ulRegValue &= 0x00U;

		result =
			AR230_IsiRegWriteIss(pAR230Ctx, AR230_Test_Pattern,
					 ulRegValue);
		RETURN_RESULT_IF_DIFFERENT(RET_SUCCESS, result);
	}

	TRACE(AR230_INFO, "%s: (exit)\n", __func__);

	return result;
}

/*****************************************************************************/
/**
 *          AR230_IsiGetSensorIss
 *
 * @brief   fills in the correct pointers for the sensor description struct
 *
 * @param   param1      pointer to sensor description struct
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT AR230_IsiGetSensorIss(IsiSensor_t *pIsiSensor)
{
	RESULT result = RET_SUCCESS;

	TRACE(AR230_INFO, "%s (enter)\n", __func__);

	if (pIsiSensor != NULL) {
		pIsiSensor->pszName = AR230_g_acName;
		pIsiSensor->pRegisterTable = AR230_g_aRegDescription;
		pIsiSensor->pIsiSensorCaps = &AR230_g_IsiSensorDefaultConfig;

		pIsiSensor->pIsiCreateSensorIss = AR230_IsiCreateSensorIss;
		pIsiSensor->pIsiReleaseSensorIss = AR230_IsiReleaseSensorIss;
		pIsiSensor->pIsiGetCapsIss = AR230_IsiGetCapsIss;
		pIsiSensor->pIsiSetupSensorIss = AR230_IsiSetupSensorIss;
		pIsiSensor->pIsiChangeSensorResolutionIss =
			AR230_IsiChangeSensorResolutionIss;
		pIsiSensor->pIsiSensorSetStreamingIss =
			AR230_IsiSensorSetStreamingIss;
		pIsiSensor->pIsiSensorSetPowerIss = AR230IsiSensorSetup;
		pIsiSensor->pIsiCheckSensorConnectionIss =
			AR230_IsiCheckSensorConnectionIss;
		pIsiSensor->pIsiGetSensorRevisionIss =
			AR230_IsiGetSensorRevisionIss;
		pIsiSensor->pIsiRegisterReadIss = AR230_IsiRegReadIss;
		pIsiSensor->pIsiRegisterWriteIss = AR230_IsiRegWriteIss;

		/* AEC functions */
		pIsiSensor->pIsiExposureControlIss =
			AR230_IsiExposureControlIss;
		pIsiSensor->pIsiGetGainLimitsIss = AR230_IsiGetGainLimitsIss;
		pIsiSensor->pIsiGetIntegrationTimeLimitsIss =
			AR230_IsiGetIntegrationTimeLimitsIss;
		pIsiSensor->pIsiGetCurrentExposureIss =
			AR230_IsiGetCurrentExposureIss;
		pIsiSensor->pIsiGetGainIss = AR230_IsiGetGainIss;
		pIsiSensor->pIsiGetGainIncrementIss =
			AR230_IsiGetGainIncrementIss;
		pIsiSensor->pIsiSetGainIss = AR230_IsiSetGainIss;
		pIsiSensor->pIsiGetIntegrationTimeIss =
			AR230_IsiGetIntegrationTimeIss;
		pIsiSensor->pIsiGetIntegrationTimeIncrementIss =
			AR230_IsiGetIntegrationTimeIncrementIss;
		pIsiSensor->pIsiSetIntegrationTimeIss =
			AR230_IsiSetIntegrationTimeIss;
		pIsiSensor->pIsiGetResolutionIss = AR230_IsiGetResolutionIss;
		pIsiSensor->pIsiGetAfpsInfoIss = AR230_IsiGetAfpsInfoIss;

		/* AWB specific functions */
		pIsiSensor->pIsiGetCalibKFactor = AR230_IsiGetCalibKFactor;
		pIsiSensor->pIsiGetCalibPcaMatrix = AR230_IsiGetCalibPcaMatrix;
		pIsiSensor->pIsiGetCalibSvdMeanValue =
			AR230_IsiGetCalibSvdMeanValue;
		pIsiSensor->pIsiGetCalibCenterLine =
			AR230_IsiGetCalibCenterLine;
		pIsiSensor->pIsiGetCalibClipParam = AR230_IsiGetCalibClipParam;
		pIsiSensor->pIsiGetCalibGlobalFadeParam =
			AR230_IsiGetCalibGlobalFadeParam;
		pIsiSensor->pIsiGetCalibFadeParam = AR230_IsiGetCalibFadeParam;
		pIsiSensor->pIsiGetIlluProfile = AR230_IsiGetIlluProfile;
		pIsiSensor->pIsiGetLscMatrixTable = AR230_IsiGetLscMatrixTable;

		/* Testpattern */
		pIsiSensor->pIsiActivateTestPattern =
			AR230_IsiActivateTestPattern;
	} else {
		result = RET_NULL_POINTER;
	}

	TRACE(AR230_INFO, "%s (exit)\n", __func__);

	return result;
}

/******************************************************************************
 * See header file for detailed comment.
 *****************************************************************************/

/*****************************************************************************/
/**
 */
/*****************************************************************************/
IsiCamDrvConfig_t IsiCamDrvConfig = {
	0,
	AR230_IsiGetSensorIss,
	{
	 0,			/**< IsiSensor_t.pszName */
	 0,			/**< IsiSensor_t.pRegisterTable */
	 0,			/**< IsiSensor_t.pIsiSensorCaps */
	 0,			/**< IsiSensor_t.pIsiCreateSensorIss */
	 0,			/**< IsiSensor_t.pIsiReleaseSensorIss */
	 0,			/**< IsiSensor_t.pIsiGetCapsIss */
	 0,			/**< IsiSensor_t.pIsiSetupSensorIss */
	 0,			/**< IsiSensor_t.pIsiChangeSensorResolutionIss */
	 0,			/**< IsiSensor_t.pIsiSensorSetStreamingIss */
	 0,			/**< IsiSensor_t.pIsiSensorSetPowerIss */
	 0,			/**< IsiSensor_t.pIsiCheckSensorConnectionIss */
	 0,			/**< IsiSensor_t.pIsiGetSensorRevisionIss */
	 0,			/**< IsiSensor_t.pIsiRegisterReadIss */
	 0,			/**< IsiSensor_t.pIsiRegisterWriteIss */

	 0,			/**< IsiSensor_t.pIsiExposureControlIss */
	 0,			/**< IsiSensor_t.pIsiGetGainLimitsIss */
	 0,			/**< IsiSensor_t.pIsiGetIntegrationTimeLimitsIss */
	 0,			/**< IsiSensor_t.pIsiGetCurrentExposureIss */
	 0,			/**< IsiSensor_t.pIsiGetGainIss */
	 0,			/**< IsiSensor_t.pIsiGetGainIncrementIss */
	 0,			/**< IsiSensor_t.pIsiSetGainIss */
	 0,			/**< IsiSensor_t.pIsiGetIntegrationTimeIss */
	 0,			/**< IsiSensor_t.pIsiGetIntegrationTimeIncrementIss */
	 0,			/**< IsiSensor_t.pIsiSetIntegrationTimeIss */
	 0,			/**< IsiSensor_t.pIsiGetResolutionIss */
	 0,			/**< IsiSensor_t.pIsiGetAfpsInfoIss */

	 0,			/**< IsiSensor_t.pIsiGetCalibKFactor */
	 0,			/**< IsiSensor_t.pIsiGetCalibPcaMatrix */
	 0,			/**< IsiSensor_t.pIsiGetCalibSvdMeanValue */
	 0,			/**< IsiSensor_t.pIsiGetCalibCenterLine */
	 0,			/**< IsiSensor_t.pIsiGetCalibClipParam */
	 0,			/**< IsiSensor_t.pIsiGetCalibGlobalFadeParam */
	 0,			/**< IsiSensor_t.pIsiGetCalibFadeParam */
	 0,			/**< IsiSensor_t.pIsiGetIlluProfile */
	 0,			/**< IsiSensor_t.pIsiGetLscMatrixTable */

	 0,			/**< IsiSensor_t.pIsiMdiInitMotoDriveMds */
	 0,			/**< IsiSensor_t.pIsiMdiSetupMotoDrive */
	 0,			/**< IsiSensor_t.pIsiMdiFocusSet */
	 0,			/**< IsiSensor_t.pIsiMdiFocusGet */
	 0,			/**< IsiSensor_t.pIsiMdiFocusCalibrate */

	 0,			/**< IsiSensor_t.pIsiGetSensorMipiInfoIss */

	 0,			/**< IsiSensor_t.pIsiActivateTestPattern */
	 }
};
