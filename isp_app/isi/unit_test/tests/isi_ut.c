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
 * @file cam_engine_ut.c
 *
 * @brief
 *   ADD_DESCRIPTION_HERE
 *
 *****************************************************************************/
/**
 *
 * @page Template_Module_Unittest UnitTest Documentation
 *
 * Welcome to the NAME UnitTest Documentation of the Template Module
 *
 * Doc-Id: xx-xxx-xxx-xxx (NAME Validation Spec)\n
 * Author: YOUR_NAME
 *
 * The test suite depends on the following test concept:
 *
 * Describe the test concept in detail...
 *
 *
 * The sw validation of the module_name is separated into the following
 * test groups to grant the best possible overview and test coverage.
 *
 * - @ref module_name_testgroup1_page
 *
 */
/**
 * @page module_name_testgroup1_page Module Name Test Group 1
 * This module is the test group 1 for the NAME. DESCRIBE IN DETAIL / ADD USECASES...
 *
 * for a detailed list of test functions refer to:
 * - @ref module_name_testgroup1
 */
/*****************************************************************************/
/**
 * @defgroup version_testgroup1 Test group 1
 *
 * @brief The  version_testgroup1 contains fundamental function tests of the
 *        Version module.
 *
 *****************************************************************************/
#include <ebase/types.h>
#include <ebase/trace.h>
#include <ebase/builtins.h>
#include <embUnit/embUnit.h>

#include <hal/hal_api.h>
#include <oslayer/oslayer.h>
#include <version/version.h>

#include <common/return_codes.h>
#include <common/picture_buffer.h>

#include <dlfcn.h>

#include "isi.h"
#include "isi_iss.h"


CREATE_TRACER( ISI_TEST_INFO , "ISI-TEST: ", INFO,    1);
CREATE_TRACER( ISI_TEST_WARN , "ISI-TEST: ", WARNING, 1);
CREATE_TRACER( ISI_TEST_ERROR, "ISI-TEST: ", ERROR,   1);


// compiletime selection of driver to load
char *libName = "ov2715.drv";
//char *libName = "ov5630.drv";
//char *libName = "ov8810.drv";
//char *libName = "ov14825.drv";


/******************************************************************************
 * local variable declarations
 *****************************************************************************/

/******************************************************************************
 * gHalHandle
 *****************************************************************************/
HalHandle_t gHalHandle = NULL;


/******************************************************************************
 * gTestCtx
 *****************************************************************************/
typedef struct TestGroupCtx_s
{
    osEvent             StartEvent;
    osEvent             StopEvent;
} TestGroupCtx_t;


/*****************************************************************************/
/**
 * setUp()
 *
 * @brief This function is called before every test case.
 *
 *****************************************************************************/
static void setUp(void)
{
    TRACE( ISI_TEST_INFO, "%s (enter)\n", __FUNCTION__);

    /* prepare test framework */
    gHalHandle = HalOpen();
    TEST_ASSERT( gHalHandle != 0);

    TRACE( ISI_TEST_INFO, "%s (exit)\n", __FUNCTION__);
}



/*****************************************************************************/
/**
 * tearDown()
 *
 * @brief This function is called after every test case.
 *
 *****************************************************************************/
static void tearDown(void)
{
    TRACE( ISI_TEST_INFO, "%s (enter)\n", __FUNCTION__);

    HalClose(gHalHandle);
    gHalHandle = NULL;

    TRACE( ISI_TEST_INFO, "%s (exit)\n", __FUNCTION__);
}



/*****************************************************************************/
/**
 * getResName()
 *
 * @brief get name of sensor resolution.
 *
 *****************************************************************************/
static char* getResName
(
    uint32_t Resolution
)
{
    char *szResolution = NULL;
    RESULT result = IsiGetResolutionName( Resolution, &szResolution );
    if ( ((RET_SUCCESS != result) && (RET_OUTOFRANGE != result)) || (szResolution == NULL) )
    {
        szResolution = "error getting name";
    }

    return szResolution;
}


/*****************************************************************************/
/**
 * dumpSensorCapabilities()
 *
 * @brief Dump capabilities of sensor.
 *
 *****************************************************************************/
static void dumpSensorCapabilities
(
    IsiSensorCaps_t *pCaps
)
{
    TRACE( ISI_TEST_INFO, "Caps.BusWidth        : 0x%08x\n", pCaps->BusWidth );
    TRACE( ISI_TEST_INFO, "Caps.Mode            : 0x%08x\n", pCaps->Mode );
    TRACE( ISI_TEST_INFO, "Caps.FieldSelection  : 0x%08x\n", pCaps->FieldSelection );
    TRACE( ISI_TEST_INFO, "Caps.YCSequence      : 0x%08x\n", pCaps->YCSequence );
    TRACE( ISI_TEST_INFO, "Caps.Conv422         : 0x%08x\n", pCaps->Conv422 );
    TRACE( ISI_TEST_INFO, "Caps.BPat            : 0x%08x\n", pCaps->BPat );
    TRACE( ISI_TEST_INFO, "Caps.HPol            : 0x%08x\n", pCaps->HPol );
    TRACE( ISI_TEST_INFO, "Caps.VPol            : 0x%08x\n", pCaps->VPol );
    TRACE( ISI_TEST_INFO, "Caps.Edge            : 0x%08x\n", pCaps->Edge );
    TRACE( ISI_TEST_INFO, "Caps.Bls             : 0x%08x\n", pCaps->Bls );
    TRACE( ISI_TEST_INFO, "Caps.Gamma           : 0x%08x\n", pCaps->Gamma );
    TRACE( ISI_TEST_INFO, "Caps.CConv           : 0x%08x\n", pCaps->CConv );
    TRACE( ISI_TEST_INFO, "Caps.Resolution      : 0x%08x\n", pCaps->Resolution );
    TRACE( ISI_TEST_INFO, "Caps.DwnSz           : 0x%08x\n", pCaps->DwnSz );
    TRACE( ISI_TEST_INFO, "Caps.BLC             : 0x%08x\n", pCaps->BLC );
    TRACE( ISI_TEST_INFO, "Caps.AGC             : 0x%08x\n", pCaps->AGC );
    TRACE( ISI_TEST_INFO, "Caps.AWB             : 0x%08x\n", pCaps->AWB );
    TRACE( ISI_TEST_INFO, "Caps.AEC             : 0x%08x\n", pCaps->AEC );
    TRACE( ISI_TEST_INFO, "Caps.DPCC            : 0x%08x\n", pCaps->DPCC );
    TRACE( ISI_TEST_INFO, "Caps.CieProfile      : 0x%08x\n", pCaps->CieProfile );
    TRACE( ISI_TEST_INFO, "Caps.SmiaMode        : 0x%08x\n", pCaps->SmiaMode );
    TRACE( ISI_TEST_INFO, "Caps.MipiMode        : 0x%08x\n", pCaps->MipiMode );
    TRACE( ISI_TEST_INFO, "Caps.AfpsResolutions : 0x%08x\n", pCaps->AfpsResolutions );
}



/*****************************************************************************/
/**
 * dumpSensorCapabilitiesAndConfig()
 *
 * @brief Dump capabilities and config of sensor side by side.
 *
 *****************************************************************************/
static void dumpSensorCapabilitiesAndConfig
(
    IsiSensorCaps_t *pCaps,
    IsiSensorCaps_t *pConfig
)
{
    TRACE( ISI_TEST_INFO, "Caps.BusWidth        : 0x%08x --> 0x%08x : Config.BusWidth       \n", pCaps->BusWidth       , pConfig->BusWidth        );
    TRACE( ISI_TEST_INFO, "Caps.Mode            : 0x%08x --> 0x%08x : Config.Mode           \n", pCaps->Mode           , pConfig->Mode            );
    TRACE( ISI_TEST_INFO, "Caps.FieldSelection  : 0x%08x --> 0x%08x : Config.FieldSelection \n", pCaps->FieldSelection , pConfig->FieldSelection  );
    TRACE( ISI_TEST_INFO, "Caps.YCSequence      : 0x%08x --> 0x%08x : Config.YCSequence     \n", pCaps->YCSequence     , pConfig->YCSequence      );
    TRACE( ISI_TEST_INFO, "Caps.Conv422         : 0x%08x --> 0x%08x : Config.Conv422        \n", pCaps->Conv422        , pConfig->Conv422         );
    TRACE( ISI_TEST_INFO, "Caps.BPat            : 0x%08x --> 0x%08x : Config.BPat           \n", pCaps->BPat           , pConfig->BPat            );
    TRACE( ISI_TEST_INFO, "Caps.HPol            : 0x%08x --> 0x%08x : Config.HPol           \n", pCaps->HPol           , pConfig->HPol            );
    TRACE( ISI_TEST_INFO, "Caps.VPol            : 0x%08x --> 0x%08x : Config.VPol           \n", pCaps->VPol           , pConfig->VPol            );
    TRACE( ISI_TEST_INFO, "Caps.Edge            : 0x%08x --> 0x%08x : Config.Edge           \n", pCaps->Edge           , pConfig->Edge            );
    TRACE( ISI_TEST_INFO, "Caps.Bls             : 0x%08x --> 0x%08x : Config.Bls            \n", pCaps->Bls            , pConfig->Bls             );
    TRACE( ISI_TEST_INFO, "Caps.Gamma           : 0x%08x --> 0x%08x : Config.Gamma          \n", pCaps->Gamma          , pConfig->Gamma           );
    TRACE( ISI_TEST_INFO, "Caps.CConv           : 0x%08x --> 0x%08x : Config.CConv          \n", pCaps->CConv          , pConfig->CConv           );
    TRACE( ISI_TEST_INFO, "Caps.Resolution      : 0x%08x --> 0x%08x : Config.Resolution (%s)\n", pCaps->Resolution     , pConfig->Resolution      , getResName(pConfig->Resolution) );
    TRACE( ISI_TEST_INFO, "Caps.DwnSz           : 0x%08x --> 0x%08x : Config.DwnSz          \n", pCaps->DwnSz          , pConfig->DwnSz           );
    TRACE( ISI_TEST_INFO, "Caps.BLC             : 0x%08x --> 0x%08x : Config.BLC            \n", pCaps->BLC            , pConfig->BLC             );
    TRACE( ISI_TEST_INFO, "Caps.AGC             : 0x%08x --> 0x%08x : Config.AGC            \n", pCaps->AGC            , pConfig->AGC             );
    TRACE( ISI_TEST_INFO, "Caps.AWB             : 0x%08x --> 0x%08x : Config.AWB            \n", pCaps->AWB            , pConfig->AWB             );
    TRACE( ISI_TEST_INFO, "Caps.AEC             : 0x%08x --> 0x%08x : Config.AEC            \n", pCaps->AEC            , pConfig->AEC             );
    TRACE( ISI_TEST_INFO, "Caps.DPCC            : 0x%08x --> 0x%08x : Config.DPCC           \n", pCaps->DPCC           , pConfig->DPCC            );
    TRACE( ISI_TEST_INFO, "Caps.CieProfile      : 0x%08x --> 0x%08x : Config.CieProfile     \n", pCaps->CieProfile     , pConfig->CieProfile      );
    TRACE( ISI_TEST_INFO, "Caps.SmiaMode        : 0x%08x --> 0x%08x : Config.SmiaMode       \n", pCaps->SmiaMode       , pConfig->SmiaMode        );
    TRACE( ISI_TEST_INFO, "Caps.MipiMode        : 0x%08x --> 0x%08x : Config.MipiMode       \n", pCaps->MipiMode       , pConfig->MipiMode        );
    TRACE( ISI_TEST_INFO, "Caps.AfpsResolutions : 0x%08x --> 0x%08x : Config.AfpsResolutions\n", pCaps->AfpsResolutions, pConfig->AfpsResolutions );
}



/*****************************************************************************/
/**
 * dumpSensorCapabilities()
 *
 * @brief Dump capabilities of sensor.
 *
 *****************************************************************************/
static void dumpAfpsInfo
(
    IsiAfpsInfo_t *pInfo
)
{
    uint32_t idx = 0;

    if (pInfo->Stage[0].Resolution == 0)
    {
        TRACE( ISI_TEST_INFO, "AfpsInfo empty\n" );
        return;
    }

    TRACE( ISI_TEST_INFO, "AfpsInfo.AecMinGain           : %f\n", pInfo->AecMinGain    );
    TRACE( ISI_TEST_INFO, "AfpsInfo.AecMaxGain           : %f\n", pInfo->AecMaxGain    );
    TRACE( ISI_TEST_INFO, "AfpsInfo.AecMinIntTime        : %f\n", pInfo->AecMinIntTime );
    TRACE( ISI_TEST_INFO, "AfpsInfo.AecMaxIntTime        : %f\n", pInfo->AecMaxIntTime );

    for (idx=0; idx<ISI_NUM_AFPS_STAGES; idx++)
    {
        if (pInfo->Stage[idx].Resolution == 0)
        {
            break;
        }

        TRACE( ISI_TEST_INFO, "AfpsInfo.Stage[%02d].Resolution : 0x%08x (%s)\n", idx, pInfo->Stage[idx].Resolution, getResName(pInfo->Stage[idx].Resolution) );
        TRACE( ISI_TEST_INFO, "AfpsInfo.Stage[%02d].MaxIntTime : %f\n",          idx, pInfo->Stage[idx].MaxIntTime );
    }
}



/*****************************************************************************/
/**
 * @ingroup
 *
 * @brief
 *
 * @test
 *
 *****************************************************************************/
static void TestIsiApi_generic( void )
{
    void * handle;

    RESULT                      result;
    uint32_t                    revId;

    IsiCamDrvConfig_t           *pIsiCamDrvConfig;

    IsiSensorInstanceConfig_t   Config;
    IsiSensorHandle_t           hSensor;

    IsiSensorCaps_t             Caps;
    IsiSensorConfig_t           ConfigSensor;

    bool_t selSuccess = true;

    uint8_t NumFramesToSkip = 0;

    TRACE( ISI_TEST_INFO, "%s (enter)\n", __FUNCTION__);
    TRACE( ISI_TEST_INFO, "Software Version (%s)\n", GetVersion() );

    handle = dlopen( libName, RTLD_LAZY);
    TEST_ASSERT ( handle != NULL );

    pIsiCamDrvConfig = (IsiCamDrvConfig_t *)dlsym( handle, "IsiCamDrvConfig" );
    TEST_ASSERT( pIsiCamDrvConfig != NULL );

    pIsiCamDrvConfig->pfIsiGetSensorIss( &(pIsiCamDrvConfig->IsiSensor) );
    TRACE( ISI_TEST_INFO, "Sensor Name (%s)\n", pIsiCamDrvConfig->IsiSensor.pszName );

    /* left sensor */
    Config.hSensor      = NULL;
    Config.HalHandle    = gHalHandle;
    Config.HalDevID     = HAL_DEVID_CAM_1;
    Config.I2cBusNum    = HAL_I2C_BUS_CAM_1;
    Config.SlaveAddr    = 0U;
    Config.I2cAfBusNum  = HAL_I2C_BUS_CAM_1;
    Config.SlaveAfAddr  = 0U;
    Config.pSensor      = &pIsiCamDrvConfig->IsiSensor;

    result = IsiCreateSensorIss( &Config );
    TEST_ASSERT_EQUAL_INT( RET_SUCCESS, result );

    hSensor = Config.hSensor;
    TEST_ASSERT( hSensor != NULL );

    result = IsiSensorSetPowerIss( hSensor, BOOL_TRUE );
    TEST_ASSERT_EQUAL_INT( RET_SUCCESS, result );

    result = IsiGetSensorRevisionIss( hSensor, &revId );
    TEST_ASSERT_EQUAL_INT( RET_SUCCESS, result );
    TRACE( ISI_TEST_INFO, "Sensor ID = 0x%04x\n", revId );

    result = IsiGetCapsIss( hSensor, &Caps );
    TEST_ASSERT_EQUAL_INT( RET_SUCCESS, result );

    MEMCPY( &ConfigSensor, pIsiCamDrvConfig->IsiSensor.pIsiSensorCaps, sizeof( IsiSensorCaps_t ) );

    /* common sensor setup */
    uint32_t ResolutionCaps[] = { // in descending priority order
        ISI_RES_TV1080P60,
        ISI_RES_TV1080P50,
        ISI_RES_TV1080P30,
        ISI_RES_TV1080P25,
        ISI_RES_TV1080P24,
        ISI_RES_TV1080P20,
        ISI_RES_TV1080P15,
        ISI_RES_TV1080P12,
        ISI_RES_TV1080P10,
        ISI_RES_TV1080P6 ,
        ISI_RES_TV1080P5 ,
        ISI_RES_TV720P60 ,
        ISI_RES_TV720P30 ,
        ISI_RES_TV720P15 ,
        ISI_RES_TV720P5  ,
        ISI_RES_VGA      ,
        0 // must be last, marks end of array/list
    };
    selSuccess &= IsiTryToSetConfigFromPreferredCaps( &ConfigSensor.Resolution, ResolutionCaps, Caps.Resolution );

    uint32_t BusWidthCaps[] = { // in descending priority order
        ISI_BUSWIDTH_12BIT,
        ISI_BUSWIDTH_10BIT,
        ISI_BUSWIDTH_10BIT_EX,
        ISI_BUSWIDTH_10BIT_ZZ,
        ISI_BUSWIDTH_8BIT_EX,
        ISI_BUSWIDTH_8BIT_ZZ,
        0 // must be last, marks end of array/list
    };
    selSuccess &= IsiTryToSetConfigFromPreferredCaps( &ConfigSensor.BusWidth, BusWidthCaps, Caps.BusWidth);

    uint32_t ModeCaps[] = { // in descending priority order
        ISI_MODE_MIPI,
        ISI_MODE_BAYER,
        ISI_MODE_BAY_BT656,
        ISI_MODE_BT601,
        ISI_MODE_BT656,
        0 // must be last, marks end of array/list
    };
    selSuccess &= IsiTryToSetConfigFromPreferredCaps( &ConfigSensor.Mode, ModeCaps, Caps.Mode );

    selSuccess &= IsiTryToSetConfigFromPreferredCap( &ConfigSensor.FieldSelection, ISI_FIELDSEL_BOTH    , Caps.FieldSelection );
    /*weak req.*/ IsiTryToSetConfigFromPreferredCap( &ConfigSensor.YCSequence    , ISI_YCSEQ_YCBYCR     , Caps.YCSequence     );
    /*weak req.*/ IsiTryToSetConfigFromPreferredCap( &ConfigSensor.Conv422       , ISI_CONV422_NOCOSITED, Caps.Conv422        );
    /*weak req.*/ IsiTryToSetConfigFromPreferredCap( &ConfigSensor.BPat          , ISI_BPAT_BGBGGRGR    , Caps.BPat           );
    /*weak req.*/ IsiTryToSetConfigFromPreferredCap( &ConfigSensor.HPol          , ISI_HPOL_REFPOS      , Caps.HPol           );
    /*weak req.*/ IsiTryToSetConfigFromPreferredCap( &ConfigSensor.VPol          , ISI_VPOL_NEG         , Caps.VPol           );
    /*weak req.*/ IsiTryToSetConfigFromPreferredCap( &ConfigSensor.Edge          , ISI_EDGE_FALLING     , Caps.Edge           );
    selSuccess &= IsiTryToSetConfigFromPreferredCap( &ConfigSensor.Bls           , ISI_BLS_OFF          , Caps.Bls            );
    selSuccess &= IsiTryToSetConfigFromPreferredCap( &ConfigSensor.Gamma         , ISI_GAMMA_OFF        , Caps.Gamma          );
    selSuccess &= IsiTryToSetConfigFromPreferredCap( &ConfigSensor.CConv         , ISI_CCONV_OFF        , Caps.CConv          );
    selSuccess &= IsiTryToSetConfigFromPreferredCap( &ConfigSensor.DwnSz         , ISI_DWNSZ_SUBSMPL    , Caps.DwnSz          );
    /*weak req.*/ IsiTryToSetConfigFromPreferredCap( &ConfigSensor.BLC           , ISI_BLC_OFF          , Caps.BLC            );
    selSuccess &= IsiTryToSetConfigFromPreferredCap( &ConfigSensor.AGC           , ISI_AGC_OFF          , Caps.AGC            );
    selSuccess &= IsiTryToSetConfigFromPreferredCap( &ConfigSensor.AWB           , ISI_AWB_OFF          , Caps.AWB            );
    selSuccess &= IsiTryToSetConfigFromPreferredCap( &ConfigSensor.AEC           , ISI_AEC_OFF          , Caps.AEC            );
    selSuccess &= IsiTryToSetConfigFromPreferredCap( &ConfigSensor.DPCC          , ISI_DPCC_OFF         , Caps.DPCC           );
    selSuccess &= IsiTryToSetConfigFromPreferredCap( &ConfigSensor.CieProfile    , ISI_CIEPROF_F11      , Caps.CieProfile     );
    selSuccess &= IsiTryToSetConfigFromPreferredCap( &ConfigSensor.SmiaMode      , ISI_SMIA_OFF         , Caps.SmiaMode       );

    if (ConfigSensor.Mode == ISI_MODE_MIPI)
    {
        uint32_t MipiModeCaps[] = { // in descending priority order
            ISI_MIPI_MODE_RAW_12,
            ISI_MIPI_MODE_RAW_10,
            0 // must be last, marks end of array/list
        };
        selSuccess &= IsiTryToSetConfigFromPreferredCaps( &ConfigSensor.MipiMode, MipiModeCaps, Caps.MipiMode );
    }
    else
    {
        ConfigSensor.MipiMode = ISI_MIPI_OFF;
    }

    ConfigSensor.AfpsResolutions = Caps.AfpsResolutions; // enable all AFPS resolutions that are supported

    TEST_ASSERT( selSuccess == true );
    dumpSensorCapabilitiesAndConfig( &Caps, &ConfigSensor );

    result = IsiSetupSensorIss( hSensor, &ConfigSensor );
    TEST_ASSERT_EQUAL_INT( RET_SUCCESS, result );

    result = IsiSensorSetStreamingIss( hSensor, BOOL_TRUE );
    TEST_ASSERT_EQUAL_INT( RET_SUCCESS, result );

    osSleep(500);

    result = IsiMdiInitMotoDrive( hSensor );
    if (RET_SUCCESS != result)
    {
        TEST_ASSERT_EQUAL_INT( RET_NOTSUPP, result );
        TRACE( ISI_TEST_INFO, "AF not supported!\n" );
    }
    else
    {
        uint32_t MaxSteps = 0;
        result = IsiMdiSetupMotoDrive( hSensor, &MaxSteps);

        uint32_t NewPos, CurPos;

        TRACE( ISI_TEST_INFO, "AF supported!\n" );

        NewPos = 0;
        result = IsiMdiFocusSet( hSensor, NewPos );
        TEST_ASSERT_EQUAL_INT( RET_SUCCESS, result );

        result = IsiMdiFocusGet( hSensor, &CurPos );
        TEST_ASSERT_EQUAL_INT( RET_SUCCESS, result );
        TRACE( ISI_TEST_INFO, "AF Position = 0x%04x\n", CurPos );
        TEST_ASSERT_EQUAL_INT( NewPos, CurPos );

        NewPos = MaxSteps/2;
        result = IsiMdiFocusSet( hSensor, NewPos );
        TEST_ASSERT_EQUAL_INT( RET_SUCCESS, result );

        result = IsiMdiFocusGet( hSensor, &CurPos );
        TEST_ASSERT_EQUAL_INT( RET_SUCCESS, result );
        TRACE( ISI_TEST_INFO, "AF Position = 0x%04x\n", CurPos );
        TEST_ASSERT_EQUAL_INT( NewPos, CurPos );

        NewPos = MaxSteps;
        result = IsiMdiFocusSet( hSensor, NewPos );
        TEST_ASSERT_EQUAL_INT( RET_SUCCESS, result );

        result = IsiMdiFocusGet( hSensor, &CurPos );
        TEST_ASSERT_EQUAL_INT( RET_SUCCESS, result );
        TRACE( ISI_TEST_INFO, "AF Position = 0x%04x\n", CurPos );
        TEST_ASSERT_EQUAL_INT( NewPos, CurPos );
    }

    result = IsiSensorSetStreamingIss( hSensor, BOOL_FALSE );
    TEST_ASSERT_EQUAL_INT( RET_SUCCESS, result );

    result = IsiChangeSensorResolutionIss( hSensor, ConfigSensor.Resolution, &NumFramesToSkip ); // dummy, just tells us if resolution change is supported
    if (RET_SUCCESS != result)
    {
        TEST_ASSERT_EQUAL_INT( RET_NOTSUPP, result );
        TRACE( ISI_TEST_INFO, "ChangeSensorResolution not supported!\n" );
    }
    else
    {
        TRACE( ISI_TEST_INFO, "ChangeSensorResolution supported!\n" );

        // try to automatically select a resolution different than the current one for resolution change testing
        uint32_t ResCaps = Caps.Resolution & ~ConfigSensor.Resolution; // all but currently selected resolution
        uint32_t NewRes = 0;
        uint32_t i;
        for( i=0; i < 32; i++)
        {
            uint32_t TestRes = (2<<i);
            if ((ResCaps & TestRes) == TestRes)
            {
                NewRes = TestRes;
                break;
            }
        }
        TEST_ASSERT( IsiTryToSetConfigFromPreferredCap( &ConfigSensor.Resolution, NewRes, ResCaps ) );

        TRACE( ISI_TEST_INFO, "Caps.Resolution     : 0x%08x --> 0x%08x : Config.Resolution (%s)\n", Caps.Resolution, ConfigSensor.Resolution, getResName(ConfigSensor.Resolution) );

        result = IsiChangeSensorResolutionIss( hSensor, ConfigSensor.Resolution, &NumFramesToSkip );
        TEST_ASSERT_EQUAL_INT( RET_SUCCESS, result );

        result = IsiSensorSetStreamingIss( hSensor, BOOL_TRUE );
        TEST_ASSERT_EQUAL_INT( RET_SUCCESS, result );

        osSleep(500);

        result = IsiSensorSetStreamingIss( hSensor, BOOL_FALSE );
        TEST_ASSERT_EQUAL_INT( RET_SUCCESS, result );
    }

    if ( Caps.AfpsResolutions == ISI_AFPS_NOTSUPP )
    {
        TRACE( ISI_TEST_INFO, "AFPS not supported!\n" );
    }
    else
    {
        IsiAfpsInfo_t AfpsInfo;

        // automatically select an AFPS supported resolution for AFPS testing
        uint32_t ResCaps = Caps.AfpsResolutions;
        uint32_t AfpsRes = 0;
        uint32_t i;
        for( i=0; i < 32; i++)
        {
            uint32_t TestRes = (2<<i);
            if ((ResCaps & TestRes) == TestRes)
            {
                AfpsRes = TestRes;
                break;
            }
        }

        result = IsiGetAfpsInfoIss( hSensor, AfpsRes, &AfpsInfo );
        if (RET_NOTSUPP == result)
        {
            TRACE( ISI_TEST_ERROR, "AFPS not supported for resolution 0x%08x (%s) or in this mode!\n", AfpsRes, getResName(AfpsRes) );
        }
        else
        {
            TRACE( ISI_TEST_INFO, "AFPS supported for resolution 0x%08x (%s)!\n", AfpsRes, getResName(AfpsRes) );
            TEST_ASSERT_EQUAL_INT( RET_SUCCESS, result );
            dumpAfpsInfo(&AfpsInfo);
        }
    }

    result = IsiSensorSetPowerIss( hSensor, BOOL_FALSE );
    TEST_ASSERT_EQUAL_INT( RET_SUCCESS, result );

    result = IsiReleaseSensorIss( hSensor );
    TEST_ASSERT_EQUAL_INT( RET_SUCCESS, result );

    dlclose(handle);

    TRACE( ISI_TEST_INFO, "%s (exit)\n", __FUNCTION__);
}



/*****************************************************************************/
/**
 * @ingroup
 *
 * @brief
 *
 * @test
 *
 *****************************************************************************/
static void TestIsiApi_dual( void )
{
    void * handle;

    RESULT result;
    uint32_t revId;
    uint32_t position;

    IsiCamDrvConfig_t *pIsiCamDrvConfig;

    IsiSensorInstanceConfig_t   Config;
    IsiSensorHandle_t           hSensorLeft;
    IsiSensorHandle_t           hSensorRight;

    IsiSensorCaps_t             CapsLeft;
    IsiSensorCaps_t             CapsRight;

    IsiSensorConfig_t           ConfigLeft;

    TRACE( ISI_TEST_INFO, "%s (enter)\n", __FUNCTION__);
    TRACE( ISI_TEST_INFO, "Software Version (%s)\n", GetVersion() );

    handle = dlopen( libName, RTLD_LAZY);
    TEST_ASSERT ( handle != NULL );

    pIsiCamDrvConfig = (IsiCamDrvConfig_t *)dlsym( handle, "IsiCamDrvConfig" );
    TEST_ASSERT( pIsiCamDrvConfig != NULL );

    pIsiCamDrvConfig->pfIsiGetSensorIss( &(pIsiCamDrvConfig->IsiSensor) );
    TRACE( ISI_TEST_INFO, "Sensor Name (%s)\n", pIsiCamDrvConfig->IsiSensor.pszName );

    /* left sensor */
    Config.HalHandle    = gHalHandle;
    Config.HalDevID     = HAL_DEVID_CAM_1;
    Config.I2cBusNum    = HAL_I2C_BUS_CAM_1;
    Config.SlaveAddr    = 0U;
    Config.I2cAfBusNum  = HAL_I2C_BUS_CAM_1;
    Config.SlaveAfAddr  = 0U;
    Config.pSensor      = &pIsiCamDrvConfig->IsiSensor;

    result = IsiCreateSensorIss( &Config );
    TEST_ASSERT_EQUAL_INT( RET_SUCCESS, result );

    hSensorLeft = Config.hSensor;
    TEST_ASSERT( hSensorLeft != NULL );

    /* power-on only left sensor */
    result = IsiSensorSetPowerIss( hSensorLeft, BOOL_TRUE );
    TEST_ASSERT_EQUAL_INT( RET_SUCCESS, result );

    /* right sensor */
    Config.HalHandle    = gHalHandle;
    Config.HalDevID     = HAL_DEVID_CAM_1;
    Config.I2cBusNum    = HAL_I2C_BUS_CAM_2;
    Config.SlaveAddr    = 0U;
    Config.I2cAfBusNum  = HAL_I2C_BUS_CAM_2;
    Config.SlaveAfAddr  = 0U;
    Config.pSensor      = &pIsiCamDrvConfig->IsiSensor;

    result = IsiCreateSensorIss( &Config );
    TEST_ASSERT_EQUAL_INT( RET_SUCCESS, result );

    hSensorRight = Config.hSensor;
    TEST_ASSERT( hSensorRight != NULL );

    /* read revision of left sensor */
    result = IsiGetSensorRevisionIss( hSensorLeft, &revId );
    TEST_ASSERT_EQUAL_INT( RET_SUCCESS, result );
    TRACE( ISI_TEST_INFO, "Sensor ID = 0x%04x\n", revId );

    /* read revision of right sensor */
    result = IsiGetSensorRevisionIss( hSensorRight, &revId );
    TEST_ASSERT_EQUAL_INT( RET_SUCCESS, result );
    TRACE( ISI_TEST_INFO, "Sensor ID = 0x%04x\n", revId );

#if 0
    result = IsiGetCapsIss( hSensorLeft, &CapsLeft );
    TEST_ASSERT_EQUAL_INT( RET_SUCCESS, result );
    dumpSensorCapabilities( &CapsLeft );

    ConfigLeft.BusWidth         = ISI_BUSWIDTH_10BIT_EX;
    ConfigLeft.Mode             = ISI_MODE_BAYER;
    ConfigLeft.FieldSelection   = ISI_FIELDSEL_BOTH;
    ConfigLeft.YCSequence       = ISI_YCSEQ_YCBYCR;
    ConfigLeft.Conv422          = ISI_CONV422_NOCOSITED;
    ConfigLeft.BPat             = ISI_BPAT_BGBGGRGR;
    ConfigLeft.HPol             = ISI_HPOL_REFPOS;
    ConfigLeft.VPol             = ISI_VPOL_NEG;
    ConfigLeft.Edge             = ISI_EDGE_RISING;
    ConfigLeft.Bls              = ISI_BLS_OFF;
    ConfigLeft.Gamma            = ISI_GAMMA_OFF;
    ConfigLeft.CConv            = ISI_CCONV_OFF;
    ConfigLeft.Resolution       = ISI_RES_2592_1944;
    ConfigLeft.DwnSz            = ISI_DWNSZ_SUBSMPL;
    ConfigLeft.BLC              = ISI_BLC_AUTO;
    ConfigLeft.AGC              = ISI_AGC_OFF;
    ConfigLeft.AWB              = ISI_AWB_OFF;
    ConfigLeft.AEC              = ISI_AEC_OFF;
    ConfigLeft.AecMode          = ISI_AEC_MODE_NORMAL;
    ConfigLeft.DPCC             = ISI_DPCC_OFF;
    ConfigLeft.CieProfile       = ISI_CIEPROF_F11;
    ConfigLeft.SmiaMode         = ISI_SMIA_OFF;
    ConfigLeft.MipiMode         = ISI_MIPI_OFF;

    result = IsiSetupSensorIss( hSensorLeft, &ConfigLeft );
    TEST_ASSERT_EQUAL_INT( RET_SUCCESS, result );

    result = IsiSensorSetStreamingIss( hSensorLeft, BOOL_TRUE );
    TEST_ASSERT_EQUAL_INT( RET_SUCCESS, result );

    result = IsiGetCapsIss( hSensorLeft, &CapsRight );
    TEST_ASSERT_EQUAL_INT( RET_SUCCESS, result );
    dumpSensorCapabilities( &CapsLeft );

    result = IsiMdiFocusSet( hSensorLeft, 64 );
    TEST_ASSERT_EQUAL_INT( RET_SUCCESS, result );

    result = IsiMdiFocusGet( hSensorLeft, &position );
    TEST_ASSERT_EQUAL_INT( RET_SUCCESS, result );
    TRACE( ISI_TEST_INFO, "AF Position = 0x%04x\n", position );

    result = IsiMdiFocusGet( hSensorLeft, &position );
    TEST_ASSERT_EQUAL_INT( RET_SUCCESS, result );
    TRACE( ISI_TEST_INFO, "AF Position = 0x%04x\n", position );

    result = IsiMdiFocusGet( hSensorLeft, &position );
    TEST_ASSERT_EQUAL_INT( RET_SUCCESS, result );
    TRACE( ISI_TEST_INFO, "AF Position = 0x%04x\n", position );
#endif

    result = IsiReleaseSensorIss( hSensorLeft );
    TEST_ASSERT_EQUAL_INT( RET_SUCCESS, result );

    result = IsiReleaseSensorIss( hSensorRight );
    TEST_ASSERT_EQUAL_INT( RET_SUCCESS, result );

    dlclose(handle);

    TRACE( ISI_TEST_INFO, "%s (exit)\n", __FUNCTION__);
}




/*****************************************************************************/
/**
 * @ingroup
 *
 * @brief
 *
 * @test
 *
 *****************************************************************************/
TestRef isi_tests_api(void)
{
    EMB_UNIT_TESTFIXTURES(fixtures)
    {
        new_TestFixture( "TestIsiApi_generic", TestIsiApi_generic ),
////        new_TestFixture( "TestIsiApi_dual", TestIsiApi_dual ),
    };
    EMB_UNIT_TESTCALLER( isi_tests_api, "isi_tests", setUp, tearDown, fixtures);

    return (TestRef)&isi_tests_api;
}
