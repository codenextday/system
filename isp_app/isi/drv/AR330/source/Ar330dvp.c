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
 * @file AR330.c
 *
 * @brief
 *   ADD_DESCRIPTION_HERE
 *
 *****************************************************************************/
#include <ebase/types.h>
#include <ebase/trace.h>
#include <ebase/builtins.h>

#include <common/return_codes.h>
#include <common/misc.h>

#include "isi.h"
#include "isi_iss.h"
#include "isi_priv.h"

#include "AR330_priv.h"




#define AR330_SLAVE_ADDR          (0x10)
#define AR330_SLAVE_AF_ADDR       (0x0)


#define STEPS_IN_GAIN_NUMBER    ( 0 )       /**< Number of max steps_in_gain */
#define AR330_MAX_GAIN_AEC     ( 8.0f )    /**< max. gain used by the AEC (arbitrarily chosen, recommended by Omnivision) */



/*!<
 * Focus position values:
 * 65 logical positions ( 0 - 64 )
 * where 64 is the setting for infinity and 0 for macro
 * corresponding to
 * 1024 register settings (0 - 1023)
 * where 0 is the setting for infinity and 1023 for macro
 */
#define MAX_LOG   64
#define MAX_REG 1023



/*!<
 * Lens movement is triggered every 133ms (VGA, 7.5fps processed frames
 * worst case assumed, usually even much slower, see AR330 driver for
 * details). Thus the lens has to reach the requested position after
 * max. 133ms. Minimum mechanical ringing is expected with mode 1 ,
 * 100us per step. A movement over the full range needs max. 102.3ms
 * (see table 9 AD5820 datasheet).
 */
#define MDI_SLEW_RATE_CTRL 2 /* S3..0 */

#define AWB_SATURATION_ARRAY_SIZE       4
#define AWB_COLORMATRIX_ARRAY_SIZE      4

#define AWB_VIGNETTING_ARRAY_SIZE       4
#define AWB_LSCMATRIX_ARRAY_SIZE        2


#define AR330_DVP_CHIP_VERSION     (0x2604)

#define DEBUG_IN_FPGA  1

#define USE_VGA 1


/******************************************************************************
 * local type definitions
 *****************************************************************************/
CREATE_TRACER( AR330_INFO , "AR330: ", INFO, 0);
CREATE_TRACER( AR330_WARN , "AR330: ", WARNING, 1);
CREATE_TRACER( AR330_ERROR, "AR330: ", ERROR,   1);
CREATE_TRACER( AR330_DEBUG, "AR330: ", INFO, 0);

/******************************************************************************
 * local variable declarations
 *****************************************************************************/
const char AR330_g_acName[] = "AR330";
extern const IsiRegDescription_t AR330_g_aRegDescription_VGAP30[];
extern const IsiRegDescription_t AR330_g_aRegDescription_1080P30[];
const IsiSensorCaps_t AR330_g_IsiSensorDefaultConfig;


// The sensor design may not allow to alter integration time from frame to frame
// (for example the classic rolling shutter). So we remember the old integration
// time to figure out if we are changing it and to tell the upper layers how much
// frames they have to wait before executing the AE again.
// static uint32_t OldCoarseIntegrationTime = 0UL;

/* AWB specific value (from AR330_tables.c) */
extern const Isi1x1FloatMatrix_t    AR330_KFactor;
extern const Isi3x2FloatMatrix_t    AR330_PCAMatrix;
extern const Isi3x1FloatMatrix_t    AR330_SVDMeanValue;
extern const IsiLine_t              AR330_CenterLine;
extern const IsiAwbClipParm_t       AR330_AwbClipParm;
extern const IsiAwbGlobalFadeParm_t AR330_AwbGlobalFadeParm;
extern const IsiAwbFade2Parm_t      AR330_AwbFade2Parm;

/* illumination profiles */
#include "AR330_a.h"       /* CIE A - default profile */
#include "AR330_d50.h"     /* sunny (D50) */
#include "AR330_d60.h"     /* day   (D60) */
#include "AR330_d65.h"     /* CIE D65 (D65) note: indoor because of our lightbox */
#include "AR330_d75.h"     /* CIE D75 (D75) overcast daylight, 7500K */
//#include "AR330_d80.h"     /* shade (D80) */
//#include "AR330_d120.h"    /* twilight (D120) */
#include "AR330_f2.h"      /* CIE F2 (cool white flourescent CWF) */
#include "AR330_f11.h"     /* CIE F11 (TL84) */
#include "AR330_f12.h"     /* CIE F12 (TL83) */


#define AR330_ISIILLUPROFILES_DEFAULT  10
static IsiIlluProfile_t AR330_IlluProfileDefault[AR330_ISIILLUPROFILES_DEFAULT] =
{
    {
        .p_next             = NULL,

        .name               = "A",
        .id                 = ISI_CIEPROF_A,
        .DoorType           = ISI_DOOR_TYPE_INDOOR,
        .AwbType            = ISI_AWB_TYPE_AUTO,

        .bOutdoorClip       = BOOL_FALSE,

        /* legacy stuff */
        .pCrossTalkCoeff    = &AR330_XTalkCoeff_CIE_A,
        .pCrossTalkOffset   = &AR330_XTalkOffset_CIE_A,

        .pGaussMeanValue    = &AR330_GaussMeanValue_CIE_A,
        .pCovarianceMatrix  = &AR330_CovarianceMatrix_CIE_A,
        .pGaussFactor       = &AR330_GaussFactor_CIE_A,
        .pThreshold         = &AR330_Threshold_CIE_A,
        .pComponentGain     = &AR330_CompGain_CIE_A,

        .pSaturationCurve   = &AR330_SaturationCurve_CIE_A,
        .pCcMatrixTable     = &AR330_CcMatrixTable_CIE_A,
        .pCcOffsetTable     = &AR330_CcOffsetTable_CIE_A,

        .pVignettingCurve   = &AR330_VignettingCurve_CIE_A,
    },
    {
        .p_next             = NULL,

        .name               = "D50",
        .id                 = ISI_CIEPROF_D50,
        .DoorType           = ISI_DOOR_TYPE_OUTDOOR,
        .AwbType            = ISI_AWB_TYPE_AUTO,

        .bOutdoorClip       = BOOL_TRUE,

        /* legacy stuff */
        .pCrossTalkCoeff    = &AR330_XTalkCoeff_sunny,
        .pCrossTalkOffset   = &AR330_XTalkOffset_sunny,

        .pGaussMeanValue    = &AR330_GaussMeanValue_sunny,
        .pCovarianceMatrix  = &AR330_CovarianceMatrix_sunny,
        .pGaussFactor       = &AR330_GaussFactor_sunny,
        .pThreshold         = &AR330_Threshold_sunny,
        .pComponentGain     = &AR330_CompGain_sunny,

        .pSaturationCurve   = &AR330_SaturationCurve_sunny,
        .pCcMatrixTable     = &AR330_CcMatrixTable_sunny,
        .pCcOffsetTable     = &AR330_CcOffsetTable_sunny,

        .pVignettingCurve   = &AR330_VignettingCurve_sunny,
    },
    {
        .p_next             = NULL,

        .name               = "D60",
        .id                 = ISI_CIEPROF_D60,
        .DoorType           = ISI_DOOR_TYPE_OUTDOOR,
        .AwbType            = ISI_AWB_TYPE_AUTO,

        .bOutdoorClip       = BOOL_FALSE,

        /* legacy stuff */
        .pCrossTalkCoeff    = &AR330_XTalkCoeff_day,
        .pCrossTalkOffset   = &AR330_XTalkOffset_day,

        .pGaussMeanValue    = &AR330_GaussMeanValue_day,
        .pCovarianceMatrix  = &AR330_CovarianceMatrix_day,
        .pGaussFactor       = &AR330_GaussFactor_day,
        .pThreshold         = &AR330_Threshold_day,
        .pComponentGain     = &AR330_CompGain_day,

        .pSaturationCurve   = &AR330_SaturationCurve_day,
        .pCcMatrixTable     = &AR330_CcMatrixTable_day,
        .pCcOffsetTable     = &AR330_CcOffsetTable_day,

        .pVignettingCurve   = &AR330_VignettingCurve_day,
    },
    {
        .p_next             = NULL,

        .name               = "D65",
        .id                 = ISI_CIEPROF_D65,
        .DoorType           = ISI_DOOR_TYPE_INDOOR,
        .AwbType            = ISI_AWB_TYPE_AUTO,

        .bOutdoorClip       = BOOL_FALSE,

        /* legacy stuff */
        .pCrossTalkCoeff    = &AR330_XTalkCoeff_D65,
        .pCrossTalkOffset   = &AR330_XTalkOffset_D65,

        .pGaussMeanValue    = &AR330_GaussMeanValue_D65,
        .pCovarianceMatrix  = &AR330_CovarianceMatrix_D65,
        .pGaussFactor       = &AR330_GaussFactor_D65,
        .pThreshold         = &AR330_Threshold_D65,
        .pComponentGain     = &AR330_CompGain_D65,

        .pSaturationCurve   = &AR330_SaturationCurve_D65,
        .pCcMatrixTable     = &AR330_CcMatrixTable_D65,
        .pCcOffsetTable     = &AR330_CcOffsetTable_D65,

        .pVignettingCurve   = &AR330_VignettingCurve_D65,
    },
    {
        .p_next             = NULL,

        .name               = "D75",
        .id                 = ISI_CIEPROF_D75,
        .DoorType           = ISI_DOOR_TYPE_OUTDOOR,
        .AwbType            = ISI_AWB_TYPE_AUTO,

        .bOutdoorClip       = BOOL_FALSE,

        /* legacy stuff */
        .pCrossTalkCoeff    = &AR330_XTalkCoeff_D75,
        .pCrossTalkOffset   = &AR330_XTalkOffset_D75,

        .pGaussMeanValue    = &AR330_GaussMeanValue_D75,
        .pCovarianceMatrix  = &AR330_CovarianceMatrix_D75,
        .pGaussFactor       = &AR330_GaussFactor_D75,
        .pThreshold         = &AR330_Threshold_D75,
        .pComponentGain     = &AR330_CompGain_D75,

        .pSaturationCurve   = &AR330_SaturationCurve_D75,
        .pCcMatrixTable     = &AR330_CcMatrixTable_D75,
        .pCcOffsetTable     = &AR330_CcOffsetTable_D75,

        .pVignettingCurve   = &AR330_VignettingCurve_D75,
    },
    {
        .p_next             = NULL,

        .name               = "F2",
        .id                 = ISI_CIEPROF_F2,
        .DoorType           = ISI_DOOR_TYPE_INDOOR,
        .AwbType            = ISI_AWB_TYPE_AUTO,

        .bOutdoorClip       = BOOL_FALSE,

        /* legacy stuff */
        .pCrossTalkCoeff    = &AR330_XTalkCoeff_F2,
        .pCrossTalkOffset   = &AR330_XTalkOffset_F2,

        .pGaussMeanValue    = &AR330_GaussMeanValue_F2,
        .pCovarianceMatrix  = &AR330_CovarianceMatrix_F2,
        .pGaussFactor       = &AR330_GaussFactor_F2,
        .pThreshold         = &AR330_Threshold_F2,
        .pComponentGain     = &AR330_CompGain_F2,

        .pSaturationCurve   = &AR330_SaturationCurve_F2,
        .pCcMatrixTable     = &AR330_CcMatrixTable_F2,
        .pCcOffsetTable     = &AR330_CcOffsetTable_F2,

        .pVignettingCurve   = &AR330_VignettingCurve_F2,
    },
    {
        .p_next             = NULL,

        .name               = "F11",
        .id                 = ISI_CIEPROF_F11,
        .DoorType           = ISI_DOOR_TYPE_INDOOR,
        .AwbType            = ISI_AWB_TYPE_AUTO,

        .bOutdoorClip       = BOOL_FALSE,

        /* legacy stuff */
        .pCrossTalkCoeff    = &AR330_XTalkCoeff_F11,
        .pCrossTalkOffset   = &AR330_XTalkOffset_F11,

        .pGaussMeanValue    = &AR330_GaussMeanValue_F11,
        .pCovarianceMatrix  = &AR330_CovarianceMatrix_F11,
        .pGaussFactor       = &AR330_GaussFactor_F11,
        .pThreshold         = &AR330_Threshold_F11,
        .pComponentGain     = &AR330_CompGain_F11,

        .pSaturationCurve   = &AR330_SaturationCurve_F11,
        .pCcMatrixTable     = &AR330_CcMatrixTable_F11,
        .pCcOffsetTable     = &AR330_CcOffsetTable_F11,

        .pVignettingCurve   = &AR330_VignettingCurve_F11,
    },
    {
        .p_next             = NULL,

        .name               = "F12",
        .id                 = ISI_CIEPROF_F12,
        .DoorType           = ISI_DOOR_TYPE_INDOOR,
        .AwbType            = ISI_AWB_TYPE_AUTO,

        .bOutdoorClip       = BOOL_FALSE,

        /* legacy stuff */
        .pCrossTalkCoeff    = &AR330_XTalkCoeff_F12,
        .pCrossTalkOffset   = &AR330_XTalkOffset_F12,

        .pGaussMeanValue    = &AR330_GaussMeanValue_F12,
        .pCovarianceMatrix  = &AR330_CovarianceMatrix_F12,
        .pGaussFactor       = &AR330_GaussFactor_F12,
        .pThreshold         = &AR330_Threshold_F12,
        .pComponentGain     = &AR330_CompGain_F12,

        .pSaturationCurve   = &AR330_SaturationCurve_F12,
        .pCcMatrixTable     = &AR330_CcMatrixTable_F12,
        .pCcOffsetTable     = &AR330_CcOffsetTable_F12,

        .pVignettingCurve   = &AR330_VignettingCurve_F12,
    },
};



/******************************************************************************
 * local function prototypes
 *****************************************************************************/
static RESULT AR330_IsiCreateSensorIss( IsiSensorInstanceConfig_t *pConfig );
static RESULT AR330_IsiReleaseSensorIss( IsiSensorHandle_t handle );
static RESULT AR330_IsiGetCapsIss( IsiSensorHandle_t handle, IsiSensorCaps_t *pIsiSensorCaps );
static RESULT AR330_IsiSetupSensorIss( IsiSensorHandle_t handle, const IsiSensorConfig_t *pConfig );
static RESULT AR330_IsiSensorSetStreamingIss( IsiSensorHandle_t handle, bool_t on );
static RESULT AR330_IsiSensorSetPowerIss( IsiSensorHandle_t handle, bool_t on );
static RESULT AR330_IsiCheckSensorConnectionIss( IsiSensorHandle_t handle );
static RESULT AR330_IsiGetSensorRevisionIss( IsiSensorHandle_t handle, uint32_t *p_value);

static RESULT AR330_IsiGetGainLimitsIss( IsiSensorHandle_t handle, float *pMinGain, float *pMaxGain);
static RESULT AR330_IsiGetIntegrationTimeLimitsIss( IsiSensorHandle_t handle, float *pMinIntegrationTime, float *pMaxIntegrationTime );
static RESULT AR330_IsiExposureControlIss( IsiSensorHandle_t handle, float newGain, float newIntegrationTime, uint8_t *pNumberOfFramesToSkip, float *pSetGain, float *pSetIntegrationTime );
static RESULT AR330_IsiGetAfpsInfoIss ( IsiSensorHandle_t handle, uint32_t Resolution, IsiAfpsInfo_t* pAfpsInfo);
static RESULT AR330_IsiGetGainIss( IsiSensorHandle_t handle, float *pSetGain );
static RESULT AR330_IsiGetGainIncrementIss( IsiSensorHandle_t handle, float *pIncr );
static RESULT AR330_IsiSetGainIss( IsiSensorHandle_t handle, float NewGain, float *pSetGain );
static RESULT AR330_IsiGetIntegrationTimeIss( IsiSensorHandle_t handle, float *pSetIntegrationTime );
static RESULT AR330_IsiGetIntegrationTimeIncrementIss( IsiSensorHandle_t handle, float *pIncr );
static RESULT AR330_IsiSetIntegrationTimeIss( IsiSensorHandle_t handle, float NewIntegrationTime, float *pSetIntegrationTime, uint8_t *pNumberOfFramesToSkip );
static RESULT AR330_IsiGetResolutionIss( IsiSensorHandle_t handle, uint32_t *pSetResolution );

static RESULT AR330_IsiRegReadIss( IsiSensorHandle_t handle, const uint32_t address, uint32_t *p_value );
static RESULT AR330_IsiRegWriteIss( IsiSensorHandle_t handle, const uint32_t address, const uint32_t value );

static RESULT AR330_IsiGetCalibKFactor( IsiSensorHandle_t handle, Isi1x1FloatMatrix_t **pIsiKFactor );
static RESULT AR330_IsiGetCalibPcaMatrix( IsiSensorHandle_t handle, Isi3x2FloatMatrix_t **pIsiPcaMatrix );
static RESULT AR330_IsiGetCalibSvdMeanValue( IsiSensorHandle_t handle, Isi3x1FloatMatrix_t **pIsiSvdMeanValue );
static RESULT AR330_IsiGetCalibCenterLine( IsiSensorHandle_t handle, IsiLine_t **ptIsiCenterLine );
static RESULT AR330_IsiGetCalibClipParam( IsiSensorHandle_t handle, IsiAwbClipParm_t **pIsiClipParam );
static RESULT AR330_IsiGetCalibGlobalFadeParam( IsiSensorHandle_t handle, IsiAwbGlobalFadeParm_t **ptIsiGlobalFadeParam );
static RESULT AR330_IsiGetCalibFadeParam( IsiSensorHandle_t handle, IsiAwbFade2Parm_t **ptIsiFadeParam );
static RESULT AR330_IsiGetIlluProfile( IsiSensorHandle_t   handle, const uint32_t CieProfile, IsiIlluProfile_t **ptIsiIlluProfile );

static RESULT AR330_IsiActivateTestPattern( IsiSensorHandle_t handle, const bool_t enable );




/*****************************************************************************/
/**
 *          floor
 *
 * @brief   Implements floor function to avoid include math-lib
 *
 *****************************************************************************/
#if 0
static double floor( const double f )
{
    if ( f < 0 )
    {
        return ( (double)((int32_t)f - 1L) );
    }
    else
    {
        return ( (double)((uint32_t)f) );
    }
}
#endif

/*****************************************************************************/
/**
 *          AR330_IsiCreateSensorIss
 *
 * @brief   This function creates a new AR330 sensor instance handle.
 *
 * @param   pConfig     configuration structure to create the instance
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 * @retval  RET_OUTOFMEM
 *
 *****************************************************************************/
static RESULT AR330_IsiCreateSensorIss
(
    IsiSensorInstanceConfig_t   *pConfig
)
{
    RESULT result = RET_SUCCESS;

    AR330_Context_t *pAR330Ctx;

    TRACE( AR330_INFO, "%s (enter)\n", __FUNCTION__);

    if ( (pConfig == NULL) || (pConfig->pSensor ==NULL) )
    {
        return ( RET_NULL_POINTER );
    }

    pAR330Ctx = ( AR330_Context_t * )malloc ( sizeof (AR330_Context_t) );
    if ( pAR330Ctx == NULL )
    {
        TRACE( AR330_ERROR,  "%s: Can't allocate AR330 context\n",  __FUNCTION__ );
        return ( RET_OUTOFMEM );
    }
    MEMSET( pAR330Ctx, 0, sizeof( AR330_Context_t ) );

    result = HalAddRef( pConfig->HalHandle );
    if ( result != RET_SUCCESS )
    {
        free ( pAR330Ctx );
        return ( result );
    }

    pAR330Ctx->IsiCtx.HalHandle           = pConfig->HalHandle;
    pAR330Ctx->IsiCtx.HalDevID            = pConfig->HalDevID;
    pAR330Ctx->IsiCtx.I2cBusNum           = pConfig->I2cBusNum;
    pAR330Ctx->IsiCtx.SlaveAddress        = ( pConfig->SlaveAddr == 0U ) ? AR330_SLAVE_ADDR : pConfig->SlaveAddr;
    pAR330Ctx->IsiCtx.NrOfAddressBytes    = 2U;

    pAR330Ctx->IsiCtx.I2cAfBusNum         = pConfig->I2cAfBusNum;
    pAR330Ctx->IsiCtx.SlaveAfAddress      = ( pConfig->SlaveAfAddr == 0U ) ? AR330_SLAVE_AF_ADDR : pConfig->SlaveAfAddr;
    pAR330Ctx->IsiCtx.NrOfAfAddressBytes  = 0U;

    pAR330Ctx->IsiCtx.pSensor             = pConfig->pSensor;

    pAR330Ctx->Configured          = BOOL_FALSE;
    pAR330Ctx->Streaming           = BOOL_FALSE;

    pConfig->hSensor = ( IsiSensorHandle_t )pAR330Ctx;
	
#ifndef DEBUG_IN_FPGA
    result = HalSetCamConfig( pAR330Ctx->IsiCtx.HalHandle, pAR330Ctx->IsiCtx.HalDevID, true, true, false );
    RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );

    result = HalSetClock( pAR330Ctx->IsiCtx.HalHandle, pAR330Ctx->IsiCtx.HalDevID, 24000000UL );
#endif
    TRACE( AR330_INFO, "%s (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          AR330_IsiReleaseSensorIss
 *
 * @brief   This function destroys/releases an AR330 sensor instance.
 *
 * @param   handle      AR330 sensor instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 *
 *****************************************************************************/
static RESULT AR330_IsiReleaseSensorIss
(
    IsiSensorHandle_t   handle
)
{
    AR330_Context_t *pAR330Ctx = (AR330_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( AR330_INFO, "%s (enter)\n", __FUNCTION__);

    if ( pAR330Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    AR330_IsiSensorSetStreamingIss( pAR330Ctx, BOOL_FALSE );
    AR330_IsiSensorSetPowerIss( pAR330Ctx, BOOL_FALSE );

    HalDelRef( pAR330Ctx->IsiCtx.HalHandle );

    MEMSET( pAR330Ctx, 0, sizeof( AR330_Context_t ) );
    free ( pAR330Ctx );

    TRACE( AR330_INFO, "%s (exit)\n", __FUNCTION__);

    return ( result );

}



/*****************************************************************************/
/**
 *          AR330_IsiGetCapsIss
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
static RESULT AR330_IsiGetCapsIss
(
    IsiSensorHandle_t   handle,
    IsiSensorCaps_t     *pIsiSensorCaps
)
{
    AR330_Context_t *pAR330Ctx = (AR330_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( AR330_INFO, "%s (enter)\n", __FUNCTION__);

    if ( pAR330Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pIsiSensorCaps == NULL )
    {
        return ( RET_NULL_POINTER );
    }
    else
    {
        pIsiSensorCaps->BusWidth        = ISI_BUSWIDTH_10BIT_EX;
        pIsiSensorCaps->Mode            = ISI_MODE_BAYER;
        pIsiSensorCaps->FieldSelection  = ISI_FIELDSEL_BOTH;
        pIsiSensorCaps->YCSequence      = ISI_YCSEQ_YCBYCR;           /**< only Bayer supported, will not be evaluated */
        pIsiSensorCaps->Conv422         = ISI_CONV422_NOCOSITED;
        pIsiSensorCaps->BPat            = ISI_BPAT_GBGBRGRG;//ISI_BPAT_GRGRBGBG;//ISI_BPAT_BGBGGRGR;
        pIsiSensorCaps->HPol            = ISI_HPOL_REFPOS;
        pIsiSensorCaps->VPol            = ISI_VPOL_POS;
        pIsiSensorCaps->Edge            = ISI_EDGE_RISING;
        pIsiSensorCaps->BLC             = ISI_BLC_AUTO;
        pIsiSensorCaps->Gamma           = ISI_GAMMA_OFF;
        pIsiSensorCaps->CConv           = ISI_CCONV_OFF;

        pIsiSensorCaps->Resolution      = ISI_RES_VGA; 

        pIsiSensorCaps->AGC             =  ISI_AGC_OFF ;
        pIsiSensorCaps->AWB             =  ISI_AWB_OFF ;
        pIsiSensorCaps->AEC             =  ISI_AEC_OFF;
        pIsiSensorCaps->DPCC            = ( ISI_DPCC_OFF | ISI_DPCC_OFF );

        pIsiSensorCaps->DwnSz           = ISI_DWNSZ_SUBSMPL;
        pIsiSensorCaps->CieProfile      = ( ISI_CIEPROF_A
										  | ISI_CIEPROF_D50
                                          | ISI_CIEPROF_D65
                                          | ISI_CIEPROF_D75
                                          | ISI_CIEPROF_F2
                                          | ISI_CIEPROF_F11
                                          | ISI_CIEPROF_F12 );
        pIsiSensorCaps->SmiaMode        = ISI_SMIA_OFF;
        pIsiSensorCaps->MipiMode        = ISI_MIPI_OFF;
        pIsiSensorCaps->AfpsResolutions = ISI_AFPS_NOTSUPP;
        pIsiSensorCaps->SensorOutputMode = ISI_SENSOR_OUTPUT_MODE_RAW;
    }

    TRACE( AR330_INFO, "%s (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          AR330_g_IsiSensorDefaultConfig
 *
 * @brief   recommended default configuration for application use via call
 *          to IsiGetSensorIss()
 *
 *****************************************************************************/
const IsiSensorCaps_t AR330_g_IsiSensorDefaultConfig =
{
    ISI_BUSWIDTH_8BIT_EX,      // BusWidth
    ISI_MODE_BAYER,             // Mode
    ISI_FIELDSEL_BOTH,          // FieldSelection
    ISI_YCSEQ_YCBYCR,           // YCSequence
    ISI_CONV422_INTER,      // Conv422
    ISI_BPAT_GRGRBGBG,          // BPat
    ISI_HPOL_SYNCPOS,            // HPol
    ISI_VPOL_POS,               // VPol
    ISI_EDGE_RISING,            // Edge
    ISI_BLS_OFF,                // Bls
    ISI_GAMMA_OFF,              // Gamma
    ISI_CCONV_OFF,              // CConv
    ISI_RES_VGA,          // Resolution
    ISI_DWNSZ_SUBSMPL,          // DwnSz
    ISI_BLC_AUTO,               // BLC
    ISI_AGC_OFF,                // AGC
    ISI_AWB_OFF,                // AWB
    ISI_AEC_OFF,                // AEC
    ISI_DPCC_OFF,               // DPCC
    ISI_CIEPROF_F11,            // CieProfile, this is also used as start profile for AWB (if not altered by menu settings)
    ISI_SMIA_OFF,               // SmiaMode
    ISI_MIPI_OFF,               // MipiMode
    ISI_AFPS_NOTSUPP,            // AfpsResolutions
    ISI_SENSOR_OUTPUT_MODE_RAW,
    0,
};

/*****************************************************************************/
/**
 *          AR330_SetupOutputWindow
 *
 * @brief   Setup of the image sensor considering the given configuration.
 *
 * @param   handle      AR330 sensor instance handle
 * @param   pConfig     pointer to sensor configuration structure
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT AR330_SetupOutputWindow
(
    AR330_Context_t        *pAR330Ctx,
    const IsiSensorConfig_t *pConfig
)
{
	RESULT result = RET_SUCCESS;
	uint32_t usXStart   = 0U;
	uint32_t usYStart   = 0U;
	uint32_t usXEnd     = 0U;
	uint32_t usYEnd     = 0U;
	uint32_t usHSize    = 0U;
	uint32_t usVSize    = 0U;

	uint32_t usLineLengthPck = 0U;     //see AR330_resolution_size_vga.xls
	uint32_t usFrameLengthLines = 0U;
	float    rVtPixClkFreq      = 0.0f;

	TRACE( AR330_INFO, "%s (enter)\n", __FUNCTION__);

	/* resolution */
	switch ( pConfig->Resolution )
	{
		case ISI_RES_VGA:
			{
				//see AR330_clocktree_V02_framefun_25mhz_vga.xls
				//
				//The system is able to process about 8fps (with 2 lines of short debugging output). The cam has to
				//run at 16fps to achieve this (every 2nd frame is skipped with the current processing loop).
				//Unfortunately the max. exposure value drops with higher fps. So we target for 15fps giving 5fps
				//overall performance (measured).
				//
				//HSize = 640
				//VSize = 480
				//
				//LINE_LENGTH_PCK    = 2 * HSize + 640 = 1920 (0x3022/0x3023)  valid calculation for RPCLKdiv=2 (0x30B3[1:0]=1)
				//FRAME_LENGTH_LINES =     VSize +  36 =  516 (0x3020/0x3021)
				//
				//fps = 2 * DVP_PCLK / LINE_LENGTH_PCK / FRAME_LENGTH_LINES) = 15.14 valid calculation for RPCLKdiv=2 (0x30B3[1:0]=1)
				//
				//50Hz/60Hz banding filter from Excel sheet (just for completeness, we use our own AEC anyway):
				//BD50ST = 0x0027 (0x305C/0x305D)
				//BD60ST = 0x0020 (0x305E/0x305F)
				//
				//input clock = 10 MHz
				//
				//from Excel sheet:
				//Sdiv_m and Ldiv just for completeness (we do not use MIPI):
				//PreDiv   = 1, Div_cnt6b = 18 => R_PLL1 = 0x2E (0x300E)
				//Sdiv     = 6, Sdiv_m    = 2  => R_PLL2 = 0x51 (0x300F)
				//Ldiv     = 2, Div45     = 5  => R_PLL3 = 0x07 (0x3010)  PLL charge pump current control from default settings, Bit 3 is reserved
				//CLK_Div  = 2                 => R_PLL4 = 0x40 (0x3011)  Bits [5:0] are reserved
				//RPCLKdiv = 2                 => DSIO_3[1:0]=1 (0x30B3)
				//
				//DVP_PCLK =  input clock / PreDiv * Div_cnt6b * Div45 / Sdiv / Div45 / CLK_Div / RPCLKdiv = 7.5 MHz

				TRACE( AR330_INFO, "%s: Resolution VGA\n", __FUNCTION__ );

				// see calculations in AR330_resolution_size_vga.xls
				usXStart =  192;
				usYStart =   48;
				usXEnd   = 831;               //subsampling/skipping 1:4
				usYEnd   = 527;
				usHSize  =  640;               //VGA_SIZE_H (use hard coded values here for consistency with other hard coded values)
				usVSize  =  480;               //VGA_SIZE_V

				// remember that the next three values are also handed over to the AEC module
				rVtPixClkFreq      = 24000000; //video timing for AEC, scan frequency of sensor array
				usLineLengthPck    = 1242;     //see AR330_resolution_size_vga.xls
				usFrameLengthLines =  644;

				//            usDspHSizeIn = 1284;           //see AR330_resolution_size_vga.xls
				//            usDspVSizeIn =  484;

				/*            usBandStep50Hz = 0x0027;
							  usBandStep60Hz = 0x0020;

							  ucHVPad  = 0x20;

							  ucR_PLL1 = 0x2E;
							  ucR_PLL2 = 0x51;
							  ucR_PLL3 = 0x07;
							  ucR_PLL4 = 0x40;
							  ucDSIO_3 = 0x09; //[3] enable, [2] reserved, [1:0] divider
							  */
				break;
			}

		case ISI_RES_TV720P30:
			{
				TRACE( AR330_INFO, "%s: Resolution 720p30\n", __FUNCTION__ );
				break;
			}

		case ISI_RES_TV720P60:
			{
				TRACE( AR330_INFO, "%s: Resolution 720P60\n", __FUNCTION__ );
				break;
			}

		case ISI_RES_TV1080P24:
			{
				TRACE( AR330_INFO, "%s: Resolution 1080P24\n", __FUNCTION__ );

				break;
			}

		case ISI_RES_TV1080P30:
			{
				TRACE( AR330_INFO, "%s: Resolution 1080P30\n", __FUNCTION__ );

				break;
			}
		default:
			{
				TRACE( AR330_ERROR, "%s: Resolution not supported\n", __FUNCTION__ );
				return ( RET_NOTSUPP );
			}
	}

	TRACE( AR330_INFO, "%s: Resolution %dx%d\n", __FUNCTION__, usHSize, usVSize );
	// set camera output margins
	result = AR330_IsiRegWriteIss ( pAR330Ctx, X_ADDR_START, usXStart);
	RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );
	result = AR330_IsiRegWriteIss ( pAR330Ctx, X_ADDR_END, usXEnd);
	RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );
	result = AR330_IsiRegWriteIss ( pAR330Ctx, Y_ADDR_START, usYStart);
	RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );
	result = AR330_IsiRegWriteIss ( pAR330Ctx, Y_ADDR_END, usYEnd);

	// set line and frame length
	RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );
	result = AR330_IsiRegWriteIss ( pAR330Ctx,LINE_LENGTH_PCK, usLineLengthPck );
	RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );
	result = AR330_IsiRegWriteIss ( pAR330Ctx,FRAME_LENGTH_LINES, usFrameLengthLines);
	RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );

	pAR330Ctx->VtPixClkFreq     = rVtPixClkFreq;
	pAR330Ctx->LineLengthPck    = usLineLengthPck;
	pAR330Ctx->FrameLengthLines = usFrameLengthLines;

	TRACE( AR330_INFO, "%s (exit)\n", __FUNCTION__);

	return ( result );
}



/*****************************************************************************/
/**
 *          AR330_SetupImageControl
 *
 * @brief   Sets the image control functions (BLC, AGC, AWB, AEC, DPCC ...)
 *
 * @param   handle      AR330 sensor instance handle
 * @param   pConfig     pointer to sensor configuration structure
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT AR330_SetupImageControl
(
    AR330_Context_t        *pAR330Ctx,
    const IsiSensorConfig_t *pConfig
)
{
    RESULT result = RET_SUCCESS;

    TRACE( AR330_INFO, "%s (enter)\n", __FUNCTION__);

   /* black level compensation */
    switch ( pConfig->BLC )
    {
		case ISI_BLC_OFF:
			break;
        case ISI_BLC_AUTO:
            break;
        default:
		{
			TRACE( AR330_ERROR, "%s: BLC not supported\n", __FUNCTION__ );
			return ( RET_NOTSUPP );
		}
	}

    /* automatic gain control */
    switch ( pConfig->AGC )
    {
        case ISI_AGC_OFF:
        case ISI_AGC_AUTO:
        default:
        {
            TRACE( AR330_ERROR, "%s: AGC not supported\n", __FUNCTION__ );
            return ( RET_NOTSUPP );
        }
    }

    /* automatic white balance */
    switch( pConfig->AWB )
    {
        case ISI_AWB_OFF:
            break;
        case ISI_AWB_AUTO:
            break;

        default:
		{
			TRACE( AR330_ERROR, "%s: AWB not supported\n", __FUNCTION__ );
			return ( RET_NOTSUPP );
		}
	}

    switch( pConfig->AEC )
    {
        case ISI_AEC_OFF:
            break;

        case ISI_AEC_AUTO:
            break;

        default:
		{
			TRACE( AR330_ERROR, "%s: AEC not supported\n", __FUNCTION__ );
			return ( RET_NOTSUPP );
		}
	}


    switch( pConfig->DPCC )
    {
        case ISI_DPCC_OFF:
            break;

        case ISI_DPCC_AUTO:
            break;

        default:
        {
            TRACE( AR330_ERROR, "%s: DPCC not supported\n", __FUNCTION__ );
            return ( RET_NOTSUPP );
        }
    }

    return ( result );
}



/*****************************************************************************/
/**
 *          AR330_AecSetModeParameters
 *
 * @brief   This function fills in the correct parameters in AR330-Instances
 *          according to AEC mode selection in IsiSensorConfig_t.
 *
 * @note    It is assumed that IsiSetupOutputWindow has been called before
 *          to fill in correct values in instance structure.
 *
 * @param   handle      AR330 context
 * @param   pConfig     pointer to sensor configuration structure
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT AR330_AecSetModeParameters
(
    AR330_Context_t        *pAR330Ctx,
    const IsiSensorConfig_t *pConfig
)
{
    RESULT result = RET_SUCCESS;

    TRACE( AR330_INFO, "%s: (enter)\n", __FUNCTION__);

    pAR330Ctx->OldCoarseIntegrationTime = 0UL; // this variable reflects the state of the sensor register, it is assumed that
                                                // the sensor initialization sets the exposure register to 0

    if ( pAR330Ctx->VtPixClkFreq == 0.0f )
    {
        TRACE( AR330_ERROR, "%s: AR330_AecSetModeParameters: VtPixClkFreq=0.0f (Division by zero !!)\n", __FUNCTION__  );
        return ( RET_OUTOFRANGE );
    }

    // there are no limits defined in the datasheet, so we assume the max. frame heigth/width is applicable
    // (formula is usually MaxIntTime = (CoarseMax * LineLength + FineMax) / Clk
    //                     MinIntTime = (CoarseMin * LineLength + FineMin) / Clk )

    pAR330Ctx->AecMinIntegrationTime = 0.0;
    pAR330Ctx->AecMaxIntegrationTime = ( ((float)pAR330Ctx->FrameLengthLines)
                                        * ((float)pAR330Ctx->LineLengthPck) )
                                        / pAR330Ctx->VtPixClkFreq;

    pAR330Ctx->AecMinGain = 1;
    pAR330Ctx->AecMaxGain = AR330_MAX_GAIN_AEC;

    TRACE( AR330_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          AR330_IsiSetupSensorIss
 *
 * @brief   Setup of the image sensor considering the given configuration.
 *
 * @param   handle      AR330 sensor instance handle
 * @param   pConfig     pointer to sensor configuration structure
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT AR330_IsiSetupSensorIss
(
    IsiSensorHandle_t       handle,
    const IsiSensorConfig_t *pConfig
)
{
    AR330_Context_t *pAR330Ctx = (AR330_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( AR330_INFO, "%s (enter)\n", __FUNCTION__);

    if ( pAR330Ctx == NULL )
    {
        TRACE( AR330_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( pConfig == NULL )
    {
        TRACE( AR330_ERROR, "%s: Invalid configuration (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_NULL_POINTER );
    }

    if ( pAR330Ctx->Streaming != BOOL_FALSE )
    {
        return RET_WRONG_STATE;
    }

    MEMCPY( &pAR330Ctx->Config, pConfig, sizeof( IsiSensorConfig_t ) );

    /* 1.) SW reset of image sensor (via I2C register interface)  be careful, bits 6..0 are reserved, reset bit is not sticky */
//    result = AR330_IsiRegReadIss ( pAR330Ctx, AR330_SYS, &RegValue);
	
    if ( result != RET_SUCCESS )
    {
        TRACE( AR330_ERROR, "%s: Can't read AR330 System Register\n", __FUNCTION__ );
        return ( result );
    }

//    result = AR330_IsiRegWriteIss ( pAR330Ctx, AR330_SYS, (RegValue| 0x80) );
    if ( result != RET_SUCCESS )
    {
        TRACE( AR330_ERROR, "%s: Can't write AR330 System Register (sensor reset failed)\n", __FUNCTION__ );
        return ( result );
    }

    TRACE( AR330_INFO, "%s: AR330 System-Reset executed\n", __FUNCTION__);

//  result = AR330_IsiRegWriteIss ( pAR330Ctx, AR330_SYS, RegValue );
    if ( result != RET_SUCCESS )
    {
        TRACE( AR330_ERROR, "%s: Can't remove AR330 System Reset (sensor reset failed)\n", __FUNCTION__ );
        return ( result );
    }

    /* 2.) write default values derived from datasheet and evaluation kit (static setup altered by dynamic setup further below) */
    result = IsiRegDefaultsApply( pAR330Ctx, AR330_g_aRegDescription_VGAP30);
    if ( result != RET_SUCCESS )
    {
        return ( result );
    }

    /* sleep a while, that sensor can take over new default values */
    osSleep( 100 );

    result = AR330_SetupOutputWindow( pAR330Ctx, pConfig );
    if ( result != RET_SUCCESS )
    {
        TRACE( AR330_ERROR, "%s: SetupOutputWindow failed.\n", __FUNCTION__);
        return ( result );
    }

    result = AR330_AecSetModeParameters( pAR330Ctx, pConfig );
    if ( result != RET_SUCCESS )
    {
        TRACE( AR330_ERROR, "%s: AecSetModeParameters failed.\n", __FUNCTION__);
        return ( result );
    }

    if (result == RET_SUCCESS)
    {
        pAR330Ctx->Configured = BOOL_TRUE;
    }

    TRACE( AR330_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          AR330_IsiChangeSensorResolutionIss
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
static RESULT AR330_IsiChangeSensorResolutionIss
(
    IsiSensorHandle_t   handle,
    uint32_t            Resolution,
    uint8_t             *pNumberOfFramesToSkip
)
{
    AR330_Context_t *pAR330Ctx = (AR330_Context_t *)handle;

    RESULT result = RET_SUCCESS;
#if 0
	float     rVtPixClkFreq      = 0.0f;
    uint16_t  usLineLengthPck    = 0;
    uint16_t  usFrameLengthLines = 0;
#endif

    TRACE( AR330_INFO, "%s (enter)\n", __FUNCTION__);

    if ( pAR330Ctx == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    if ( (pAR330Ctx->Configured != BOOL_TRUE) || (pAR330Ctx->Streaming != BOOL_FALSE) )
    {
        return RET_WRONG_STATE;
    }

    IsiSensorCaps_t Caps;
    result = AR330_IsiGetCapsIss( handle, &Caps);
    if (RET_SUCCESS != result)
    {
        return result;
    }

    if ( (Resolution & Caps.Resolution) == 0 )
    {
        return RET_OUTOFRANGE;
    }

    if ( Resolution == pAR330Ctx->Config.Resolution )
    {
        // well, no need to worry
        *pNumberOfFramesToSkip = 0;
    }
    else
    {
        // change resolution

        //TODO: implement change of resolution while keeping exposure, gain & integration time
        //...

        // return number of frames that aren't exposed correctly
        *pNumberOfFramesToSkip = 0;

        return RET_OUTOFRANGE;
    }

    TRACE( AR330_INFO, "%s (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          AR330_IsiSensorSetStreamingIss
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
static RESULT AR330_IsiSensorSetStreamingIss
(
    IsiSensorHandle_t   handle,
    bool_t              on
)
{
    AR330_Context_t *pAR330Ctx = (AR330_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( AR330_INFO, "%s (enter)\n", __FUNCTION__);

    if ( pAR330Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( (pAR330Ctx->Configured != BOOL_TRUE) || (pAR330Ctx->Streaming == on) )
    {
        return RET_WRONG_STATE;
    }

    if (on == BOOL_TRUE)
    {
        /* enable streaming */
        result = AR330_IsiRegWriteIss ( pAR330Ctx, AR330_RESET_REGISTER, (0x10DC) );
        RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );
    }
    else
    {
        /* disable streaming */
        result = AR330_IsiRegWriteIss ( pAR330Ctx, AR330_RESET_REGISTER, (0x10D8) );
        RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );
    }

    if (result == RET_SUCCESS)
    {
        pAR330Ctx->Streaming = on;
    }

    TRACE( AR330_INFO, "%s (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          AR330_IsiSensorSetPowerIss
 *
 * @brief   Performs the power-up/power-down sequence of the camera, if possible.
 *
 * @param   handle      AR330 sensor instance handle
 * @param   on          new power state (BOOL_TRUE=on, BOOL_FALSE=off)
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT AR330_IsiSensorSetPowerIss
(
    IsiSensorHandle_t   handle,
    bool_t              on
)
{
    AR330_Context_t *pAR330Ctx = (AR330_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( AR330_INFO, "%s (enter)\n", __FUNCTION__);

    if ( pAR330Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    pAR330Ctx->Configured = BOOL_FALSE;
    pAR330Ctx->Streaming  = BOOL_FALSE;

    // need to optimization after all hal function finish
#if 0
	TRACE( AR330_INFO, "%s power off \n", __FUNCTION__);
	result = HalSetPower( pAR330Ctx->IsiCtx.HalHandle, pAR330Ctx->IsiCtx.HalDevID, false );
	RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );

	TRACE( AR330_INFO, "%s reset on\n", __FUNCTION__);
	result = HalSetReset( pAR330Ctx->IsiCtx.HalHandle, pAR330Ctx->IsiCtx.HalDevID, true );
	RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );

	if (on == BOOL_TRUE)
	{
		TRACE( AR330_INFO, "%s power on \n", __FUNCTION__);
		result = HalSetPower( pAR330Ctx->IsiCtx.HalHandle, pAR330Ctx->IsiCtx.HalDevID, true );
		RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );

		osSleep( 10 );

		TRACE( AR330_INFO, "%s reset off \n", __FUNCTION__);
		result = HalSetReset( pAR330Ctx->IsiCtx.HalHandle, pAR330Ctx->IsiCtx.HalDevID, false );
		RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );

		osSleep( 10 );

		TRACE( AR330_INFO, "%s reset on \n", __FUNCTION__);
		result = HalSetReset( pAR330Ctx->IsiCtx.HalHandle, pAR330Ctx->IsiCtx.HalDevID, true );
		RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );

		osSleep( 10 );

		TRACE( AR330_INFO, "%s reset off \n", __FUNCTION__);
		result = HalSetReset( pAR330Ctx->IsiCtx.HalHandle, pAR330Ctx->IsiCtx.HalDevID, false );

		osSleep( 50 );
	}
#else
    HalSensorReset(pAR330Ctx->IsiCtx.HalHandle, pAR330Ctx->IsiCtx.HalDevID, true);
#endif

    TRACE( AR330_INFO, "%s (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          AR330_IsiCheckSensorConnectionIss
 *
 * @brief   Checks the I2C-Connection to sensor by reading sensor revision id.
 *
 * @param   handle      AR330 sensor instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT AR330_IsiCheckSensorConnectionIss
(
    IsiSensorHandle_t   handle
)
{
    uint32_t RevId;
    uint32_t value = 0UL;

    RESULT result = RET_SUCCESS;

    TRACE( AR330_INFO, "%s (enter)\n", __FUNCTION__);

    if ( handle == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    RevId = AR330_DVP_CHIP_VERSION;

    result = AR330_IsiGetSensorRevisionIss( handle, &value );
    if ( (result != RET_SUCCESS) || (RevId != value) )
    {
        return ( RET_FAILURE );
    }

    TRACE( AR330_INFO, "%s (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          AR330_IsiGetSensorRevisionIss
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
static RESULT AR330_IsiGetSensorRevisionIss
(
    IsiSensorHandle_t   handle,
    uint32_t            *p_value
)
{
    RESULT result = RET_SUCCESS;

    TRACE( AR330_INFO, "%s (enter)\n", __FUNCTION__);

    if ( handle == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( p_value == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    *p_value = 0U;
    result = AR330_IsiRegReadIss ( handle, CHIP_VERSION, p_value );

    TRACE( AR330_INFO, "%s (exit)\n", __FUNCTION__);

    return ( result );
}


/*****************************************************************************/
/**
 *          AR330_IsiRegReadIss
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
static RESULT AR330_IsiRegReadIss
(
    IsiSensorHandle_t   handle,
    const uint32_t      address,
    uint32_t            *p_value
)
{
    RESULT result = RET_SUCCESS;

    TRACE( AR330_INFO, "%s (enter)\n", __FUNCTION__);

    if ( handle == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( p_value == NULL )
    {
        return ( RET_NULL_POINTER );
    }
    else
    {
#if 0    
        uint8_t NrOfBytes = IsiGetNrDatBytesIss( address, AR330_g_aRegDescription_init );
        if ( !NrOfBytes )
        {
            NrOfBytes = 2;   //i2c read16
        }
#else
        uint8_t NrOfBytes; 
        NrOfBytes = 2;   //i2c read16
#endif
        *p_value = 0U;

        result = IsiI2cReadRegister( handle, address, p_value, NrOfBytes, BOOL_TRUE );
    }

    TRACE( AR330_INFO, "%s (exit: 0x%08x 0x%08x)\n", __FUNCTION__, address, *p_value);

    return ( result );
}



/*****************************************************************************/
/**
 *          AR330_IsiRegWriteIss
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
static RESULT AR330_IsiRegWriteIss
(
    IsiSensorHandle_t   handle,
    uint32_t      address,
    uint32_t      value
)
{
    RESULT result = RET_SUCCESS;

    uint8_t NrOfBytes;

    TRACE( AR330_INFO, "%s (enter)\n", __FUNCTION__);

    if ( handle == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }
#if 0
    NrOfBytes = IsiGetNrDatBytesIss( address, AR330_g_aRegDescription_init );
    if ( !NrOfBytes )
    {
        NrOfBytes = 2;     //i2c write16
    }
#else
    NrOfBytes = 2;     //i2c write16
#endif

    result = IsiI2cWriteRegister( handle, address, &value, NrOfBytes, BOOL_TRUE );
	TRACE( AR330_INFO, "%s (exit: 0x%08x 0x%08x)\n", __FUNCTION__, address, value);

    return ( result );
}
/*****************************************************************************/
/**
 *          AR330_IsiGetGainLimitsIss
 *
 * @brief   Returns the exposure minimal and maximal values of an
 *          AR330 instance
 *
 * @param   handle       AR330 sensor instance handle
 * @param   pMinExposure Pointer to a variable receiving minimal exposure value
 * @param   pMaxExposure Pointer to a variable receiving maximal exposure value
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT AR330_IsiGetGainLimitsIss
(
    IsiSensorHandle_t   handle,
    float               *pMinGain,
    float               *pMaxGain
)
{
    AR330_Context_t *pAR330Ctx = (AR330_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( AR330_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pAR330Ctx == NULL )
    {
        TRACE( AR330_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( (pMinGain == NULL) || (pMaxGain == NULL) )
    {
        TRACE( AR330_ERROR, "%s: NULL pointer received!!\n" );
        return ( RET_NULL_POINTER );
    }

    *pMinGain = 1.0f;
    *pMaxGain = pAR330Ctx->AecMaxGain;

    TRACE( AR330_INFO, "%s: (enter)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          AR330_IsiGetIntegrationTimeLimitsIss
 *
 * @brief   Returns the minimal and maximal integration time values of an
 *          AR330 instance
 *
 * @param   handle       AR330 sensor instance handle
 * @param   pMinExposure Pointer to a variable receiving minimal exposure value
 * @param   pMaxExposure Pointer to a variable receiving maximal exposure value
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT AR330_IsiGetIntegrationTimeLimitsIss
(
    IsiSensorHandle_t   handle,
    float               *pMinIntegrationTime,
    float               *pMaxIntegrationTime
)
{
    AR330_Context_t *pAR330Ctx = (AR330_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( AR330_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pAR330Ctx == NULL )
    {
        TRACE( AR330_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( (pMinIntegrationTime == NULL) || (pMaxIntegrationTime == NULL) )
    {
        TRACE( AR330_ERROR, "%s: NULL pointer received!!\n" );
        return ( RET_NULL_POINTER );
    }

    *pMinIntegrationTime = 0.0f;
    *pMaxIntegrationTime = (float)(pAR330Ctx->FrameLengthLines * pAR330Ctx->LineLengthPck) / pAR330Ctx->VtPixClkFreq;

    TRACE( AR330_INFO, "%s: (enter)\n", __FUNCTION__);

    return ( result );
}


/*****************************************************************************/
/**
 *          AR330_IsiGetGainIncrementIss
 *
 * @brief   Get smallest possible gain increment.
 *
 * @param   handle                  AR330 sensor instance handle
 * @param   pIncr                   increment
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT AR330_IsiGetGainIncrementIss
(
    IsiSensorHandle_t   handle,
    float               *pIncr
)
{
    AR330_Context_t *pAR330Ctx = (AR330_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( AR330_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pAR330Ctx == NULL )
    {
        TRACE( AR330_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( pIncr == NULL)
    {
        return ( RET_NULL_POINTER );
    }

    //_smallest_ increment the sensor/driver can handle (e.g. used for sliders in the application)
    *pIncr = pAR330Ctx->AecGainIncrement;

    TRACE( AR330_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}
/*****************************************************************************/
/**
 *          AR330_IsiExposureControlIss
 *
 * @brief   Camera hardware dependent part of the exposure control loop.
 *          Calculates appropriate register settings from the new exposure
 *          values and writes them to the image sensor module.
 *
 * @param   handle                  AR330 sensor instance handle
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
RESULT AR330_IsiExposureControlIss
(
    IsiSensorHandle_t   handle,
    float               NewGain,
    float               NewIntegrationTime,
    uint8_t             *pNumberOfFramesToSkip,
    float               *pSetGain,
    float               *pSetIntegrationTime
)
{
    AR330_Context_t *pAR330Ctx = (AR330_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( AR330_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pAR330Ctx == NULL )
    {
        TRACE( AR330_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( (pNumberOfFramesToSkip == NULL)
            || (pSetGain == NULL)
            || (pSetIntegrationTime == NULL) )
    {
        TRACE( AR330_ERROR, "%s: Invalid parameter (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_NULL_POINTER );
    }

    TRACE( AR330_DEBUG, "%s: g=%f, Ti=%f\n", __FUNCTION__, NewGain, NewIntegrationTime );

//  pAR330Ctx->GroupHold = BOOL_TRUE;
    result = AR330_IsiSetIntegrationTimeIss( handle, NewIntegrationTime, pSetIntegrationTime, pNumberOfFramesToSkip );
    result = AR330_IsiSetGainIss( handle, NewGain, pSetGain );
//  pAR330Ctx->GroupHold = BOOL_FALSE;

    //TRACE( AR330_DEBUG, "%s: set: g=%f, Ti=%f, skip=%d\n", __FUNCTION__, *pSetGain, *pSetIntegrationTime, *pNumberOfFramesToSkip );
    TRACE( AR330_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          AR330_IsiSetIntegrationTimeIss
 *
 * @brief   Writes gain and integration time values to the image sensor module.
 *
 * @param   handle                  AR330 sensor instance handle
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
RESULT AR330_IsiSetIntegrationTimeIss
(
    IsiSensorHandle_t   handle,
    float               NewIntegrationTime,
    float               *pSetIntegrationTime,
    uint8_t             *pNumberOfFramesToSkip
)
{
    AR330_Context_t *pAR330Ctx = (AR330_Context_t *)handle;


	uint32_t CoarseIntegrationTime = 0;

    RESULT result = RET_SUCCESS;
	float ShutterWidthPck = 0.0f; //shutter width in pixel clock periods

	TRACE( AR330_INFO, "%s: (enter) NewIntegrationTime: %f (min: %f	 max: %f)\n", __FUNCTION__,
	NewIntegrationTime, pAR330Ctx->AecMinIntegrationTime, pAR330Ctx->AecMaxIntegrationTime);


	if ( pAR330Ctx == NULL )
	{
		TRACE( AR330_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __FUNCTION__ );
		return ( RET_WRONG_HANDLE );
	}

	if ( (pSetIntegrationTime == NULL) || (pNumberOfFramesToSkip == NULL) )
	{
		TRACE( AR330_ERROR, "%s: Invalid parameter (NULL pointer detected)\n", __FUNCTION__ );
		return ( RET_NULL_POINTER );
	}

	 /*
	  * maximum and minimum integration time is limited by the sensor, if this limit is not
      * considered, the exposure control loop needs lots of time to return to a new state
      * so limit to allowed range
      */
	if ( NewIntegrationTime > pAR330Ctx->AecMaxIntegrationTime ) NewIntegrationTime = pAR330Ctx->AecMaxIntegrationTime;
	if ( NewIntegrationTime < pAR330Ctx->AecMinIntegrationTime ) NewIntegrationTime = pAR330Ctx->AecMinIntegrationTime;

	ShutterWidthPck = NewIntegrationTime * ( (float)pAR330Ctx->VtPixClkFreq );

    // avoid division by zero
	if ( pAR330Ctx->LineLengthPck == 0 )
	{
		TRACE( AR330_ERROR, "%s: Division by zero!\n", __FUNCTION__ );
		return ( RET_DIVISION_BY_ZERO );
 	}

	 CoarseIntegrationTime = (uint32_t)( ShutterWidthPck / ((float)pAR330Ctx->LineLengthPck) + 0.5f );

	/* 
	 * write new integration time into sensor registers
	 * do not write if nothing has changed
	 * write new integration time into sensor registers
     * do not write if nothing has changed
     */
	if( CoarseIntegrationTime != pAR330Ctx->OldCoarseIntegrationTime )
	{
		TRACE( AR330_DEBUG, "%s: rina >>>>EXP =0x%4x\n", __FUNCTION__, CoarseIntegrationTime);
		result = AR330_IsiRegWriteIss( pAR330Ctx, 0x3012, CoarseIntegrationTime & 0xffff);

		pAR330Ctx->OldCoarseIntegrationTime = CoarseIntegrationTime;   // remember current integration time
		*pNumberOfFramesToSkip = 1U; //skip 1 frame
	} else {
		*pNumberOfFramesToSkip = 0U; //no frame skip
	}

	pAR330Ctx->AecCurIntegrationTime = ((float)CoarseIntegrationTime) * ((float)pAR330Ctx->LineLengthPck) / pAR330Ctx->VtPixClkFreq;

	//return current state
	*pSetIntegrationTime = pAR330Ctx->AecCurIntegrationTime;

    TRACE( AR330_DEBUG, "%s: Ti=%f\n", __FUNCTION__, *pSetIntegrationTime );
    TRACE( AR330_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}

/*****************************************************************************/
/**
 *          AR330_IsiGetGainIss
 *
 * @brief   Reads gain values from the image sensor module.
 *
 * @param   handle                  AR330 sensor instance handle
 * @param   pSetGain                set gain
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT AR330_IsiGetGainIss
(
    IsiSensorHandle_t   handle,
    float               *pSetGain
)
{
    AR330_Context_t *pAR330Ctx = (AR330_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( AR330_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pAR330Ctx == NULL )
    {
        TRACE( AR330_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( pSetGain == NULL)
    {
        return ( RET_NULL_POINTER );
    }

    *pSetGain = pAR330Ctx->AecCurGain;

    TRACE( AR330_INFO, "%s: (enter)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          AR330_IsiGetResolutionIss
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
RESULT AR330_IsiGetResolutionIss
(
    IsiSensorHandle_t   handle,
    uint32_t            *pSetResolution
)
{
    AR330_Context_t *pAR330Ctx = (AR330_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( AR330_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pAR330Ctx == NULL )
    {
        TRACE( AR330_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( pSetResolution == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    *pSetResolution = pAR330Ctx->Config.Resolution;

    TRACE( AR330_INFO, "%s: (enter)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          AR330_IsiSetGainIss
 *
 * @brief   Writes gain values to the image sensor module.
 *
 * @param   handle                  AR330 sensor instance handle
 * @param   NewGain                 gain to be set
 * @param   pSetGain                set gain
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT AR330_IsiSetGainIss
(
    IsiSensorHandle_t   handle,
    float               NewGain,
    float               *pSetGain
)
{
	AR330_Context_t *pAR330Ctx = (AR330_Context_t *)handle;

	RESULT result = RET_SUCCESS;

	/* variables for gain */
	float	Gain					= 0.0f;

	uint8_t  ucMultiplier			= 0U;
	uint8_t  ucGain 				= 0U;
	uint32_t ulGainRegister 		= 0U;

	TRACE( AR330_INFO, "%s: (enter)\n", __FUNCTION__);

	if ( pAR330Ctx == NULL )
	{
		TRACE( AR330_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __FUNCTION__ );
		return ( RET_WRONG_HANDLE );
	}

	if ( pSetGain == NULL)
	{
		return ( RET_NULL_POINTER );
	}

	// The maximum and minimum gain is limited by the sensor.
	if ( NewGain > pAR330Ctx->AecMaxGain )
	{
		Gain = pAR330Ctx->AecMaxGain;
	}
	else if ( NewGain < 1.0f )
	{
		ucMultiplier = 0U;
		Gain = 1.0f;
	}
	else
	{
		Gain = NewGain;
	}

	// Calculate and set new gain register settings.
	// See mail from Omnivision FAE, analog gain = (Bit[6]+1)*(Bit[5]+1)*(Bit[4]+1)*(Bit[3:0]/16+1).
	// Lower multiplier bits have to be set before higher bits are set.

	if ( Gain < 2.0f )
	{
		ucMultiplier = 0U;					// 2*0 = x1
	}
	else if ( Gain < 4.0f )
	{
		ucMultiplier = 0x01;				// 2*1 = x2 //COARSA GAIN
		//Gain /= 2.0f;                 //fine_gain
	}
	else  if ( Gain < 8.0f )
	{
		ucMultiplier = 0x02; 		// 2*2 = x4
		//Gain /= 4.0f;           //fine fain
	}
	else
	{
		ucMultiplier = 0x20 + 0x10;	// 2*2*2 = x8
		//Gain /= 8.0f;  //fine fain
	}

	//Gain  = 16 * (Gain - 1.0f) + 0.5f;

	ucGain = (uint8_t)Gain;
	if ( ucGain > 0x0F )
	{
		ucGain = 0x0F; // avoid overflow due to limited floating point precision
	}

	result = AR330_IsiRegReadIss( pAR330Ctx, AR330_AGC, &ulGainRegister ); //0x3060 bit[5:4]

	ucMultiplier &= 0xffff;

	ulGainRegister |= (ulGainRegister & (ucMultiplier << 4));


	TRACE( AR330_DEBUG, "AR330_ADDR[0x%04x], AR330_VAL[0x%04x]:\n", AR330_AGC, ulGainRegister);
	result = AR330_IsiRegWriteIss( pAR330Ctx, AR330_AGC, ulGainRegister );



	*pSetGain			 = NewGain;

	pAR330Ctx->AecCurGain			   = NewGain;

	TRACE( AR330_DEBUG, "%s: g=%f\n", __FUNCTION__, *pSetGain );
	TRACE( AR330_INFO, "%s: (enter)\n", __FUNCTION__);

	return ( result );
}




/*****************************************************************************/
/**
 *          AR330_IsiGetIntegrationTimeIss
 *
 * @brief   Reads integration time values from the image sensor module.
 *
 * @param   handle                  AR330 sensor instance handle
 * @param   pSetIntegrationTime     set integration time
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT AR330_IsiGetIntegrationTimeIss
(
    IsiSensorHandle_t   handle,
    float               *pSetIntegrationTime
)
{
    AR330_Context_t *pAR330Ctx = (AR330_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( AR330_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pAR330Ctx == NULL )
    {
        TRACE( AR330_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( pSetIntegrationTime == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    *pSetIntegrationTime = pAR330Ctx->AecCurIntegrationTime;

    TRACE( AR330_INFO, "%s: (enter)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          AR330_IsiGetIntegrationTimeIncrementIss
 *
 * @brief   Get smallest possible integration time increment.
 *
 * @param   handle                  AR330 sensor instance handle
 * @param   pIncr                   increment
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT AR330_IsiGetIntegrationTimeIncrementIss
(
    IsiSensorHandle_t   handle,
    float               *pIncr
)
{
    AR330_Context_t *pAR330Ctx = (AR330_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( AR330_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pAR330Ctx == NULL )
    {
        TRACE( AR330_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( pIncr == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    *pIncr = (float)pAR330Ctx->LineLengthPck / (float)pAR330Ctx->VtPixClkFreq;

    TRACE( AR330_INFO, "%s: (enter)\n", __FUNCTION__);

    return ( result );
}

/*****************************************************************************/
/**
 *          AR330_IsiGetAfpsInfoIss
 *
 * @brief   Returns the possible AFPS sub resolution settings for the given resolution series
 *
 * @param   handle                  AR330 sensor instance handle
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
RESULT AR330_IsiGetAfpsInfoIss(
    IsiSensorHandle_t   handle,
    uint32_t            Resolution,
    IsiAfpsInfo_t*      pAfpsInfo
)
{
    AR330_Context_t *pAR330Ctx = (AR330_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( AR330_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pAR330Ctx == NULL )
    {
        TRACE( AR330_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( pAfpsInfo == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    // use currently configured resolution?
    if (Resolution == 0)
    {
        Resolution = pAR330Ctx->Config.Resolution;
    }

    // set current resolution data in info struct
    pAfpsInfo->CurrResolution = pAR330Ctx->Config.Resolution;
    pAfpsInfo->CurrMinIntTime = pAR330Ctx->AecMinIntegrationTime;
    pAfpsInfo->CurrMaxIntTime = pAR330Ctx->AecMaxIntegrationTime;

    // allocate dummy context used for Afps parameter calculation as a copy of current context
    AR330_Context_t *pDummyCtx = (AR330_Context_t*) malloc( sizeof(AR330_Context_t) );
    if ( pDummyCtx == NULL )
    {
        TRACE( AR330_ERROR,  "%s: Can't allocate dummy AR330 context\n",  __FUNCTION__ );
        return ( RET_OUTOFMEM );
    }
    *pDummyCtx = *pAR330Ctx;

    // set AFPS mode in dummy context
    pDummyCtx->isAfpsRun = BOOL_TRUE;

#define AFPSCHECKANDADD(_res_) \
    if ( (pAR330Ctx->Config.AfpsResolutions & (_res_)) != 0 ) \
    { \
        RESULT lres = AR330_IsiGetAfpsInfoHelperIss( pDummyCtx, _res_, pAfpsInfo, idx ); \
        if ( lres == RET_SUCCESS ) \
        { \
            ++idx; \
        } \
        else \
        { \
            UPDATE_RESULT( result, lres ); \
        } \
    }

    // check which AFPS series is requested and build its params list for the enabled AFPS resolutions
    switch(Resolution)
    {
        default:
            TRACE( AR330_DEBUG,  "%s: Resolution %08x not supported by AFPS\n",  __FUNCTION__, Resolution );
            result = RET_NOTSUPP;
            break;
        // check next series here...
    }

    // release dummy context again
    free(pDummyCtx);

    TRACE( AR330_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}


/*****************************************************************************/
/**
 *          AR330_IsiGetCurrentExposureIss
 *
 * @brief   Returns the currently adjusted AE values
 *
 * @param   handle                  AR330 sensor instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
RESULT AR330_IsiGetCurrentExposureIss
(
    IsiSensorHandle_t   handle,
    float               *pSetGain,
    float               *pSetIntegrationTime
)
{
    AR330_Context_t *pAR330Ctx = (AR330_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( AR330_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pAR330Ctx == NULL )
    {
        TRACE( AR330_ERROR, "%s: Invalid sensor handle (NULL pointer detected)\n", __FUNCTION__ );
        return ( RET_WRONG_HANDLE );
    }

    if ( (pSetGain == NULL) || (pSetIntegrationTime) )
    {
        return ( RET_NULL_POINTER );
    }

    *pSetGain            = pAR330Ctx->AecCurGain;
    *pSetIntegrationTime = pAR330Ctx->AecCurIntegrationTime;

    TRACE( AR330_INFO, "%s: (enter)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          AR330_IsiGetCalibKFactor
 *
 * @brief   Returns the AR330 specific K-Factor
 *
 * @param   handle       AR330 sensor instance handle
 * @param   pIsiKFactor  Pointer to Pointer receiving the memory address
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT AR330_IsiGetCalibKFactor
(
    IsiSensorHandle_t   handle,
    Isi1x1FloatMatrix_t **pIsiKFactor
)
{
    AR330_Context_t *pAR330Ctx = (AR330_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( AR330_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pAR330Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pIsiKFactor == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    *pIsiKFactor = (Isi1x1FloatMatrix_t *)&AR330_KFactor;

    TRACE( AR330_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}


/*****************************************************************************/
/**
 *          AR330_IsiGetCalibPcaMatrix
 *
 * @brief   Returns the AR330 specific PCA-Matrix
 *
 * @param   handle          AR330 sensor instance handle
 * @param   pIsiPcaMatrix   Pointer to Pointer receiving the memory address
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT AR330_IsiGetCalibPcaMatrix
(
    IsiSensorHandle_t   handle,
    Isi3x2FloatMatrix_t **pIsiPcaMatrix
)
{
    AR330_Context_t *pAR330Ctx = (AR330_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( AR330_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pAR330Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pIsiPcaMatrix == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    *pIsiPcaMatrix = (Isi3x2FloatMatrix_t *)&AR330_PCAMatrix;

    TRACE( AR330_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          AR330_IsiGetCalibSvdMeanValue
 *
 * @brief   Returns the sensor specific SvdMean-Vector
 *
 * @param   handle              AR330 sensor instance handle
 * @param   pIsiSvdMeanValue    Pointer to Pointer receiving the memory address
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT AR330_IsiGetCalibSvdMeanValue
(
    IsiSensorHandle_t   handle,
    Isi3x1FloatMatrix_t **pIsiSvdMeanValue
)
{
    AR330_Context_t *pAR330Ctx = (AR330_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( AR330_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pAR330Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pIsiSvdMeanValue == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    *pIsiSvdMeanValue = (Isi3x1FloatMatrix_t *)&AR330_SVDMeanValue;

    TRACE( AR330_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          AR330_IsiGetCalibCenterLine
 *
 * @brief   Returns a pointer to the sensor specific centerline, a straight
 *          line in Hesse normal form in Rg/Bg colorspace
 *
 * @param   handle              AR330 sensor instance handle
 * @param   pIsiSvdMeanValue    Pointer to Pointer receiving the memory address
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT AR330_IsiGetCalibCenterLine
(
    IsiSensorHandle_t   handle,
    IsiLine_t           **ptIsiCenterLine
)
{
    AR330_Context_t *pAR330Ctx = (AR330_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( AR330_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pAR330Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( ptIsiCenterLine == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    *ptIsiCenterLine = (IsiLine_t*)&AR330_CenterLine;

    TRACE( AR330_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          AR330_IsiGetCalibClipParam
 *
 * @brief   Returns a pointer to the sensor specific arrays for Rg/Bg color
 *          space clipping
 *
 * @param   handle              AR330 sensor instance handle
 * @param   pIsiSvdMeanValue    Pointer to Pointer receiving the memory address
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT AR330_IsiGetCalibClipParam
(
    IsiSensorHandle_t   handle,
    IsiAwbClipParm_t    **pIsiClipParam
)
{
    AR330_Context_t *pAR330Ctx = (AR330_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( AR330_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pAR330Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pIsiClipParam == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    *pIsiClipParam = (IsiAwbClipParm_t *)&AR330_AwbClipParm;

    TRACE( AR330_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          AR330_IsiGetCalibGlobalFadeParam
 *
 * @brief   Returns a pointer to the sensor specific arrays for AWB out of
 *          range handling
 *
 * @param   handle              AR330 sensor instance handle
 * @param   pIsiSvdMeanValue    Pointer to Pointer receiving the memory address
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT AR330_IsiGetCalibGlobalFadeParam
(
    IsiSensorHandle_t       handle,
    IsiAwbGlobalFadeParm_t  **ptIsiGlobalFadeParam
)
{
    AR330_Context_t *pAR330Ctx = (AR330_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( AR330_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pAR330Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( ptIsiGlobalFadeParam == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    *ptIsiGlobalFadeParam = (IsiAwbGlobalFadeParm_t *)&AR330_AwbGlobalFadeParm;

    TRACE( AR330_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          AR330_IsiGetCalibFadeParam
 *
 * @brief   Returns a pointer to the sensor specific arrays for near white
 *          pixel parameter calculations
 *
 * @param   handle              AR330 sensor instance handle
 * @param   pIsiSvdMeanValue    Pointer to Pointer receiving the memory address
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT AR330_IsiGetCalibFadeParam
(
    IsiSensorHandle_t   handle,
    IsiAwbFade2Parm_t   **ptIsiFadeParam
)
{
    AR330_Context_t *pAR330Ctx = (AR330_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( AR330_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pAR330Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( ptIsiFadeParam == NULL )
    {
        return ( RET_NULL_POINTER );
    }

    *ptIsiFadeParam = (IsiAwbFade2Parm_t *)&AR330_AwbFade2Parm;

    TRACE( AR330_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          AR330_IsiGetIlluProfile
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
static RESULT AR330_IsiGetIlluProfile
(
    IsiSensorHandle_t   handle,
    const uint32_t      CieProfile,
    IsiIlluProfile_t    **ptIsiIlluProfile
)
{
    AR330_Context_t *pAR330Ctx = (AR330_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( AR330_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pAR330Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( ptIsiIlluProfile == NULL )
    {
        return ( RET_NULL_POINTER );
    }
    else
    {
        uint16_t i;

        *ptIsiIlluProfile = NULL;

        /* check if we've a default profile */
        for ( i=0U; i<AR330_ISIILLUPROFILES_DEFAULT; i++ )
        {
            if ( AR330_IlluProfileDefault[i].id == CieProfile )
            {
                *ptIsiIlluProfile = &AR330_IlluProfileDefault[i];
                break;
            }
        }

        result = ( *ptIsiIlluProfile != NULL ) ?  RET_SUCCESS : RET_NOTAVAILABLE;
    }

    TRACE( AR330_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *          AR330_IsiGetLscMatrixTable
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
static RESULT AR330_IsiGetLscMatrixTable
(
    IsiSensorHandle_t   handle,
    const uint32_t      CieProfile,
    IsiLscMatrixTable_t **pLscMatrixTable
)
{
    AR330_Context_t *pAR330Ctx = (AR330_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    TRACE( AR330_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pAR330Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( pLscMatrixTable == NULL )
    {
        return ( RET_NULL_POINTER );
    }
    else
    {
        switch ( CieProfile )
        {
            case ISI_CIEPROF_A:
            default:
            {
                if (  (pAR330Ctx->Config.Resolution == ISI_RES_TV720P30)
                        || (pAR330Ctx->Config.Resolution == ISI_RES_TV720P60) )
                {
                    *pLscMatrixTable = &AR330_LscMatrixTable_CIE_A_1280x720;
                }
                else if ( (pAR330Ctx->Config.Resolution == ISI_RES_TV1080P24)
                        || (pAR330Ctx->Config.Resolution == ISI_RES_TV1080P30) )
                {
                    *pLscMatrixTable = &AR330_LscMatrixTable_CIE_A_1920x1080;
                }
                else if ( pAR330Ctx->Config.Resolution == ISI_RES_2592_1944 )
                {
                    *pLscMatrixTable = &AR330_LscMatrixTable_CIE_A_2592x1944;
                }
                else
                {
                    TRACE( AR330_ERROR, "%s: Resolution not supported\n", __FUNCTION__ );
                    *pLscMatrixTable = NULL;
                }

                break;
            }

#if 0
            default:
            {
                TRACE( AR330_ERROR, "%s: Illumination not supported\n", __FUNCTION__ );
                *pLscMatrixTable = NULL;
                break;
            }
#endif
        }

        result = ( *pLscMatrixTable != NULL ) ?  RET_SUCCESS : RET_NOTAVAILABLE;
    }

    TRACE( AR330_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}




/*****************************************************************************/
/**
 *          AR330_IsiActivateTestPattern
 *
 * @brief   Triggers a forced calibration of the focus hardware.
 *
 * @param   handle          AR330 sensor instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_NULL_POINTER
 *
 *****************************************************************************/
static RESULT AR330_IsiActivateTestPattern(IsiSensorHandle_t handle, const bool_t enable)
{
    AR330_Context_t *pAR330Ctx = (AR330_Context_t *)handle;

    RESULT result = RET_SUCCESS;

    uint32_t ulRegValue = 0UL;

    TRACE( AR330_INFO, "%s: (enter)\n", __FUNCTION__);

    if ( pAR330Ctx == NULL )
    {
        return ( RET_WRONG_HANDLE );
    }

    if ( BOOL_TRUE == enable )
    {
        /* enable test-pattern */
        result = AR330_IsiRegReadIss( pAR330Ctx, AR330_Test_Pattern, &ulRegValue );
        RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );

        ulRegValue &=  0x02U;

        result = AR330_IsiRegWriteIss( pAR330Ctx, AR330_Test_Pattern, ulRegValue );
        RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );
    }
    else
    {
        /* disable test-pattern */
        result = AR330_IsiRegReadIss( pAR330Ctx, AR330_Test_Pattern, &ulRegValue );
        RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );

        ulRegValue &=  0x00U;

        result = AR330_IsiRegWriteIss( pAR330Ctx, AR330_Test_Pattern, ulRegValue );
        RETURN_RESULT_IF_DIFFERENT( RET_SUCCESS, result );
    }

    TRACE( AR330_INFO, "%s: (exit)\n", __FUNCTION__);

    return ( result );
}



/*****************************************************************************/
/**
 *
 */
/*****************************************************************************/



/*****************************************************************************/
/**
 *          AR330_IsiGetSensorIss
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
RESULT AR330_IsiGetSensorIss
(
    IsiSensor_t *pIsiSensor
)
{
    RESULT result = RET_SUCCESS;

    TRACE( AR330_INFO, "%s (enter)\n", __FUNCTION__);

    if ( pIsiSensor != NULL )
    {
        pIsiSensor->pszName                         = AR330_g_acName;
#if USE_VGA
        pIsiSensor->pRegisterTable                  = AR330_g_aRegDescription_VGAP30;
#else
        pIsiSensor->pRegisterTable                  = AR330_g_aRegDescription_1080P30;
#endif
        pIsiSensor->pIsiSensorCaps                  = &AR330_g_IsiSensorDefaultConfig;

        pIsiSensor->pIsiCreateSensorIss             = AR330_IsiCreateSensorIss;
        pIsiSensor->pIsiReleaseSensorIss            = AR330_IsiReleaseSensorIss;
        pIsiSensor->pIsiGetCapsIss                  = AR330_IsiGetCapsIss;
        pIsiSensor->pIsiSetupSensorIss              = AR330_IsiSetupSensorIss;
        pIsiSensor->pIsiChangeSensorResolutionIss   = AR330_IsiChangeSensorResolutionIss;
        pIsiSensor->pIsiSensorSetStreamingIss       = AR330_IsiSensorSetStreamingIss;
        pIsiSensor->pIsiSensorSetPowerIss           = AR330_IsiSensorSetPowerIss;
        pIsiSensor->pIsiCheckSensorConnectionIss    = AR330_IsiCheckSensorConnectionIss;
        pIsiSensor->pIsiGetSensorRevisionIss        = AR330_IsiGetSensorRevisionIss;
        pIsiSensor->pIsiRegisterReadIss             = AR330_IsiRegReadIss;
        pIsiSensor->pIsiRegisterWriteIss            = AR330_IsiRegWriteIss;

        /* AEC functions */
        pIsiSensor->pIsiExposureControlIss          = AR330_IsiExposureControlIss;
        pIsiSensor->pIsiGetGainLimitsIss            = AR330_IsiGetGainLimitsIss;
        pIsiSensor->pIsiGetIntegrationTimeLimitsIss = AR330_IsiGetIntegrationTimeLimitsIss;
        pIsiSensor->pIsiGetCurrentExposureIss       = AR330_IsiGetCurrentExposureIss;
        pIsiSensor->pIsiGetGainIss                      = AR330_IsiGetGainIss;
        pIsiSensor->pIsiGetGainIncrementIss             = AR330_IsiGetGainIncrementIss;
        pIsiSensor->pIsiSetGainIss                      = AR330_IsiSetGainIss;
        pIsiSensor->pIsiGetIntegrationTimeIss           = AR330_IsiGetIntegrationTimeIss;
        pIsiSensor->pIsiGetIntegrationTimeIncrementIss  = AR330_IsiGetIntegrationTimeIncrementIss;
        pIsiSensor->pIsiSetIntegrationTimeIss           = AR330_IsiSetIntegrationTimeIss;
        pIsiSensor->pIsiGetResolutionIss                = AR330_IsiGetResolutionIss;
        pIsiSensor->pIsiGetAfpsInfoIss                  = AR330_IsiGetAfpsInfoIss;

        /* AWB specific functions */
        pIsiSensor->pIsiGetCalibKFactor             = AR330_IsiGetCalibKFactor;
        pIsiSensor->pIsiGetCalibPcaMatrix           = AR330_IsiGetCalibPcaMatrix;
        pIsiSensor->pIsiGetCalibSvdMeanValue        = AR330_IsiGetCalibSvdMeanValue;
        pIsiSensor->pIsiGetCalibCenterLine          = AR330_IsiGetCalibCenterLine;
        pIsiSensor->pIsiGetCalibClipParam           = AR330_IsiGetCalibClipParam;
        pIsiSensor->pIsiGetCalibGlobalFadeParam     = AR330_IsiGetCalibGlobalFadeParam;
        pIsiSensor->pIsiGetCalibFadeParam           = AR330_IsiGetCalibFadeParam;
        pIsiSensor->pIsiGetIlluProfile              = AR330_IsiGetIlluProfile;
        pIsiSensor->pIsiGetLscMatrixTable           = AR330_IsiGetLscMatrixTable;

        /*AF_functions */
        pIsiSensor->pIsiMdiInitMotoDriveMds         = NULL;

        /* MIPI */
        pIsiSensor->pIsiGetSensorMipiInfoIss        = NULL;

        /* Testpattern */
        pIsiSensor->pIsiActivateTestPattern         = AR330_IsiActivateTestPattern;
    }
    else
    {
        result = RET_NULL_POINTER;
    }

    TRACE( AR330_INFO, "%s (exit)\n", __FUNCTION__);

    return ( result );
}



/******************************************************************************
 * See header file for detailed comment.
 *****************************************************************************/

/*****************************************************************************/
/**
 */
/*****************************************************************************/
IsiCamDrvConfig_t IsiCamDrvConfig =
{
    0,
    AR330_IsiGetSensorIss,
#if 0
    {
        0,                      /**< IsiSensor_t.pszName */
        0,                      /**< IsiSensor_t.pRegisterTable */
        0,                      /**< IsiSensor_t.pIsiSensorCaps */
        0,                      /**< IsiSensor_t.pIsiCreateSensorIss */
        0,                      /**< IsiSensor_t.pIsiReleaseSensorIss */
        0,                      /**< IsiSensor_t.pIsiGetCapsIss */
        0,                      /**< IsiSensor_t.pIsiSetupSensorIss */
        0,                      /**< IsiSensor_t.pIsiChangeSensorResolutionIss */
        0,                      /**< IsiSensor_t.pIsiSensorSetStreamingIss */
        0,                      /**< IsiSensor_t.pIsiSensorSetPowerIss */
        0,                      /**< IsiSensor_t.pIsiCheckSensorConnectionIss */
        0,                      /**< IsiSensor_t.pIsiGetSensorRevisionIss */
        0,                      /**< IsiSensor_t.pIsiRegisterReadIss */
        0,                      /**< IsiSensor_t.pIsiRegisterWriteIss */

        0,                      /**< IsiSensor_t.pIsiExposureControlIss */
        0,                      /**< IsiSensor_t.pIsiGetGainLimitsIss */
        0,                      /**< IsiSensor_t.pIsiGetIntegrationTimeLimitsIss */
        0,                      /**< IsiSensor_t.pIsiGetCurrentExposureIss */
        0,                      /**< IsiSensor_t.pIsiGetGainIss */
        0,                      /**< IsiSensor_t.pIsiGetGainIncrementIss */
        0,                      /**< IsiSensor_t.pIsiSetGainIss */
        0,                      /**< IsiSensor_t.pIsiGetIntegrationTimeIss */
        0,                      /**< IsiSensor_t.pIsiGetIntegrationTimeIncrementIss */
        0,                      /**< IsiSensor_t.pIsiSetIntegrationTimeIss */
        0,                      /**< IsiSensor_t.pIsiGetResolutionIss */
        0,                      /**< IsiSensor_t.pIsiGetAfpsInfoIss */

        0,                      /**< IsiSensor_t.pIsiGetCalibKFactor */
        0,                      /**< IsiSensor_t.pIsiGetCalibPcaMatrix */
        0,                      /**< IsiSensor_t.pIsiGetCalibSvdMeanValue */
        0,                      /**< IsiSensor_t.pIsiGetCalibCenterLine */
        0,                      /**< IsiSensor_t.pIsiGetCalibClipParam */
        0,                      /**< IsiSensor_t.pIsiGetCalibGlobalFadeParam */
        0,                      /**< IsiSensor_t.pIsiGetCalibFadeParam */
        0,                      /**< IsiSensor_t.pIsiGetIlluProfile */
        0,                      /**< IsiSensor_t.pIsiGetLscMatrixTable */

        0,                      /**< IsiSensor_t.pIsiMdiInitMotoDriveMds */
        0,                      /**< IsiSensor_t.pIsiMdiSetupMotoDrive */
        0,                      /**< IsiSensor_t.pIsiMdiFocusSet */
        0,                      /**< IsiSensor_t.pIsiMdiFocusGet */
        0,                      /**< IsiSensor_t.pIsiMdiFocusCalibrate */

        0,                      /**< IsiSensor_t.pIsiGetSensorMipiInfoIss */

        0,                      /**< IsiSensor_t.pIsiActivateTestPattern */
#endif
   {
        0,// const char                          *pszName;                       /**< name of the camera-sensor */
        0,//   const IsiRegDescription_t           *pRegisterTable;                /**< pointer to register table */
        0, // const IsiSensorCaps_t               *pIsiSensorCaps;                /**< pointer to sensor capabilities */

        0, // IsiGetSensorIsiVer_t                *pIsiGetSensorIsiVer;
        0, // IsiGetSensorTuningXmlVersion_t      *pIsiGetSensorTuningXmlVersion;
        0, // IsiWhiteBalanceIlluminationChk_t    *pIsiWhiteBalanceIlluminationChk;//ddl@rock-chips.com
        0, // IsiWhiteBalanceIlluminationSet_t    *pIsiWhiteBalanceIlluminationSet;//ddl@rock-chips.com
        0, // IsiCheckOTPInfo_t                   *pIsiCheckOTPInfo;  //for OTP,zyc
        0, // IsiSetSensorOTPInfo_t               *pIsiSetSensorOTPInfo; //zyl
        0, // IsiEnableSensorOTP_t                *pIsiEnableSensorOTP; //zyl

        0, // IsiCreateSensorIss_t                *pIsiCreateSensorIss;           /**< create a sensor handle */
        0, // IsiReleaseSensorIss_t               *pIsiReleaseSensorIss;          /**< release a sensor handle */
        0, // IsiGetCapsIss_t                     *pIsiGetCapsIss;                /**< get sensor capabilities */
        0, // IsiSetupSensorIss_t                 *pIsiSetupSensorIss;            /**< setup sensor capabilities */
        0, // IsiChangeSensorResolutionIss_t      *pIsiChangeSensorResolutionIss; /**< change sensor resolution */
        0, // IsiSensorSetStreamingIss_t          *pIsiSensorSetStreamingIss;     /**< enable/disable streaming of data once sensor is configured */
        0, // IsiSensorSetPowerIss_t              *pIsiSensorSetPowerIss;         /**< turn sensor power on/off */
        0, // IsiCheckSensorConnectionIss_t       *pIsiCheckSensorConnectionIss;

        0, // IsiGetSensorRevisionIss_t           *pIsiGetSensorRevisionIss;      /**< read sensor revision register (if available) */
        0, // IsiRegisterReadIss_t                *pIsiRegisterReadIss;           /**< read sensor register */
        0, // IsiRegisterWriteIss_t               *pIsiRegisterWriteIss;          /**< write sensor register */

         /* AEC functions */
        0, // IsiExposureControlIss_t             *pIsiExposureControlIss;
        0, // IsiGetGainLimitsIss_t               *pIsiGetGainLimitsIss;
        0, // IsiGetIntegrationTimeLimitsIss_t    *pIsiGetIntegrationTimeLimitsIss;
        0, // IsiGetCurrentExposureIss_t          *pIsiGetCurrentExposureIss;     /**< get the currenntly adjusted ae values (gain and integration time) */
        0, // IsiGetGainIss_t                     *pIsiGetGainIss;
        0, // IsiGetGainIncrementIss_t            *pIsiGetGainIncrementIss;
        0, // IsiSetGainIss_t                     *pIsiSetGainIss;
        0, // IsiGetIntegrationTimeIss_t          *pIsiGetIntegrationTimeIss;
        0, // IsiGetIntegrationTimeIncrementIss_t *pIsiGetIntegrationTimeIncrementIss;
        0, // IsiSetIntegrationTimeIss_t          *pIsiSetIntegrationTimeIss;
        0, // IsiGetResolutionIss_t               *pIsiGetResolutionIss;
        0, // IsiGetAfpsInfoIss_t                 *pIsiGetAfpsInfoIss;

        /* AWB functions */
        0, // IsiGetCalibKFactor_t                *pIsiGetCalibKFactor;           /**< get sensor specific K-Factor (comes from calibration) */
        0, // IsiGetCalibPcaMatrix_t              *pIsiGetCalibPcaMatrix;         /**< get sensor specific PCA-Matrix (comes from calibration) */
        0, // IsiGetCalibSvdMeanValue_t           *pIsiGetCalibSvdMeanValue;      /**< get sensor specific SVD-Means (comes from calibration) */
        0, // IsiGetCalibCenterLine_t             *pIsiGetCalibCenterLine;
        0, // IsiGetCalibClipParam_t              *pIsiGetCalibClipParam;
        0, // IsiGetCalibGlobalFadeParam_t        *pIsiGetCalibGlobalFadeParam;
        0, // IsiGetCalibFadeParam_t              *pIsiGetCalibFadeParam;
        0, // IsiGetIlluProfile_t                 *pIsiGetIlluProfile;
        0, // IsiGetLscMatrixTable_t              *pIsiGetLscMatrixTable;

        /* AF functions */
        0, // IsiMdiInitMotoDriveMds_t            *pIsiMdiInitMotoDriveMds;
        0, // IsiMdiSetupMotoDrive_t              *pIsiMdiSetupMotoDrive;
        0, // IsiMdiFocusSet_t                    *pIsiMdiFocusSet;
        0, // IsiMdiFocusGet_t                    *pIsiMdiFocusGet;
        0, // IsiMdiFocusCalibrate_t              *pIsiMdiFocusCalibrate;

         /*MIPI*/
        0, // IsiGetSensorMipiInfoIss_t           *pIsiGetSensorMipiInfoIss;

        /* Testpattern */
        0, // IsiActivateTestPattern_t            *pIsiActivateTestPattern;       /**< enable/disable test-pattern */
        0, // IsiSetSensorFrameRateLimitIss_t     *pIsiSetSensorFrameRateLimit;
        0, // IsiGetSensorColorIss_t              *pIsiGetColorIss;
    },
    0,//    Sensor_IsiGetSensorI2cInfo,
};

