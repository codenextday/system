#include <ebase/types.h>
#include <ebase/trace.h>
#include <ebase/builtins.h>

#include <embUnit/embUnit.h>

#include <version/version.h>

#include <oslayer/oslayer.h>

#include <common/return_codes.h>
#include <common/picture_buffer.h>

#include <cam_engine/cam_engine_api.h>
#include <cam_engine/cam_engine_aaa_api.h>
#include <cam_engine/cam_engine_mi_api.h>
#include <cam_engine/cam_engine_jpe_api.h>

#include <som_ctrl/som_ctrl_api.h>

#include "ic_util.h"
#include "cJSON.h"

#include <uvc_display/uvc_client_api.h>
#define MAX_LEN 256

#define DEFAULT_CASE_LIST "case/list.txt"

/******************************************************************************
 * local variable declarations
 *****************************************************************************/
CREATE_TRACER(ISP_TEST_DBG,     "ISP-TEST_DBG: ",   WARNING,    1);
CREATE_TRACER(ISP_TEST_INFO,    "ISP-TEST_INFO: ",  INFO,       1);
CREATE_TRACER(ISP_TEST_WARN,    "ISP-TEST_WARN: ",  WARNING,    1);
CREATE_TRACER(ISP_TEST_ERROR,   "ISP-TEST_ERR: ",   ERROR,      1);

/******************************************************************************
 * gHalHandle
 *****************************************************************************/

#define SETTING_FILE_NAME "case/global_settings.txt"

typedef struct TestSetting_s
{
    /* SomCtrl. */
    struct Som_s {
        int enable;
    } som;
} TestSetting_t;

typedef struct TestCase_s
{
    char name[MAX_LEN];
    char description[MAX_LEN];
    CamEngineConfig_t engineConfig;
    unsigned int orientation;
    unsigned int enableDpcc;
    unsigned int enableJpeg;
    unsigned int frameNum;
    char *images;
} TestCase_t;

/******************************************************************************
 * TestCtx_t
 *****************************************************************************/
typedef struct TestCtx_s
{
    HalHandle_t hHal;
    CamEngineHandle_t hCamEngine;

    osEvent eventEngineStarted;
    osEvent eventEngineStop;
    osEvent eventEngineCaptureDone;

    somCtrlHandle_t hSomCtrl;
    osEvent eventSomStarted;
    osEvent eventSomStop;
    osEvent eventSomFinished;

    char *settingFileName;
    TestSetting_t settings;

    char *caseListFileName;
} TestCtx_t;

static CamEngineWbGains_t defaultWbGains = {1.0f, 1.0f, 1.0f, 1.0f};

static CamEngineCcMatrix_t defaultCcMatrix = {{1.0f, 0.0f, 0.0f,
                                               0.0f, 1.0f, 0.0f,
                                               0.0f, 0.0f, 1.0f}};

static CamEngineCcOffset_t defaultCcOffset = {0, 0, 0};

static CamEngineBlackLevel_t defaultBlvl = {0U, 0U, 0U, 0U};

static float defaultvGain = 0.0;
static float defaultvItime = 0.0;

//now isp is not used Jpe module
#define CamEngineEnableJpe(a, b)    0
#define CamEngineDisableJpe(a)	    0
#if 0
static CamerIcJpeConfig_t JpeConfig =
{
    CAMERIC_JPE_MODE_LARGE_CONTINUOUS,
    CAMERIC_JPE_COMPRESSION_LEVEL_HIGH,
    CAMERIC_JPE_LUMINANCE_SCALE_DISABLE,
    CAMERIC_JPE_CHROMINANCE_SCALE_DISABLE,
    1280,
    720
};
#endif
/******************************************************************************
 * cbCamEngine
 *****************************************************************************/
static void
cbCamEngine(
    CamEngineCmdId_t cmdId,
    RESULT           result,
    const void*      pUserContext
    )
{
    TRACE( ISP_TEST_INFO, "%s (enter)\n", __FUNCTION__);

    if (pUserContext != NULL)
    {
        TestCtx_t *pCtx = (TestCtx_t *)pUserContext;

        TRACE(ISP_TEST_WARN, "%s: cmd %d(<=%d) completed\n", __FUNCTION__,
                cmdId, CAM_ENGINE_CMD_STOP_STREAMING);
        switch (cmdId)
        {
        case CAM_ENGINE_CMD_START:
            osEventSignal(&pCtx->eventEngineStarted);
            break;

        case CAM_ENGINE_CMD_STOP:
            osEventSignal(&pCtx->eventEngineStop);
            break;

        case CAM_ENGINE_CMD_STOP_STREAMING:
            osEventSignal(&pCtx->eventEngineCaptureDone);
            break;
#if 0
        case CAM_ENGINE_CMD_HW_DMA_FINISHED:
            osEventSignal(&pCtx->eventEngineCaptureDone);
            break;
#endif
        default:
            break;
        }
    }

    TRACE( ISP_TEST_INFO, "%s (exit)\n", __FUNCTION__);
}

static void
cbBuffer(
    CamEnginePathType_t path,
    MediaBuffer_t *pMediaBuffer,
    void *pBufferCbCtx
    )
{
    TestCtx_t *pCtx = pBufferCbCtx;

    TRACE(ISP_TEST_INFO, "%s (enter)\n", __FUNCTION__);

    TRACE(ISP_TEST_INFO, "Path type: %d, pMediaBuffer: %p, pBufferCbCtx: %p.\n",
          path, (void *)pMediaBuffer, pBufferCbCtx);

    if (pCtx->settings.som.enable)
    {
        somCtrlStoreBuffer(pCtx->hSomCtrl, pMediaBuffer);
    }

    TRACE(ISP_TEST_INFO, "%s (exit)\n", __FUNCTION__);
}

static void
cbSomCtrl(
    somCtrlCmdID_t          cmdId,
    RESULT                  result,
    somCtrlCmdParams_t      *pParams,
    somCtrlCompletionInfo_t *pInfo,
    void                    *pUserContext
    )
{
    TRACE(ISP_TEST_INFO, "%s (enter)\n", __FUNCTION__);

    TestCtx_t *pCtx = pUserContext;

    TRACE(ISP_TEST_DBG, "%s prtx[%p] cmdid=%d param_ret=%d\n",
            __FUNCTION__, pCtx, cmdId, result);

    if (pCtx != NULL)
    {
        switch (cmdId)
        {
        case SOM_CTRL_CMD_START:
            if (RET_PENDING == result)
            {
                osEventSignal(&pCtx->eventSomStarted);
            }
            else
            {
                osEventSignal(&pCtx->eventSomFinished);
            }
            break;

        case SOM_CTRL_CMD_STOP:
            osEventSignal(&pCtx->eventSomStop);
            break;

        default:
            //osEventSignal(&pCtx->eventSomFinished);
            break;
        }
    }

    TRACE(ISP_TEST_INFO, "%s (exit)\n", __FUNCTION__);
}

static void
loadGlobalSetting(
    char *fileName,
    TestSetting_t *pSetting
    )
{
    pSetting->som.enable = 1;
}

/*****************************************************************************/
/**
 * init()
 *
 * @brief This function is called before every test case.
 *
 *****************************************************************************/
static int
init(
    TestCtx_t *pCtx
    )
{
    CamEngineInstanceConfig_t insConfig;

    RESULT result;
    int ret = 0;

    TRACE(ISP_TEST_INFO, "%s (enter)\n", __FUNCTION__);

    /* Get the HAL handle. */
    pCtx->hHal = HalOpen();
    if (!pCtx->hHal)
    {
        TRACE(ISP_TEST_ERROR, "Failed to get the HAL handle.\n");
        return -ENODEV;
    }

    loadGlobalSetting(pCtx->settingFileName, &pCtx->settings);

    /* Setup engine. */
    osEventInit(&pCtx->eventEngineStarted, 1, 0);
    osEventInit(&pCtx->eventEngineStop, 1, 0);
    osEventInit(&pCtx->eventEngineCaptureDone, 1, 0);

    /* uvc display init*/
#if IMI_UVC_DISPLAY_ENABLE
    result = uvc_display_init();
#endif

    /* create and initialize the cam-engine instance */
    insConfig.maxPendingCommands = 4;
    insConfig.isSystem3D         = false;
    insConfig.cbCompletion       = cbCamEngine;
    insConfig.pUserCbCtx         = (void *)pCtx;
    insConfig.hHal               = pCtx->hHal;
    insConfig.hCamEngine         = NULL;

    result = CamEngineInit(&insConfig);
    pCtx->hCamEngine = insConfig.hCamEngine;
    if (result != RET_SUCCESS)
    {
        TRACE(ISP_TEST_ERROR, "Failed to init CamEngine, ret=%d\n", result);
        return -ENODEV;
    }

    CamEngineRegisterBufferCb(pCtx->hCamEngine, cbBuffer, pCtx);

    /* Setup SOM. */
    if (pCtx->settings.som.enable)
    {
        somCtrlConfig_t somConfig;

        memset(&somConfig, 0, sizeof(somCtrlConfig_t));

        somConfig.MaxPendingCommands = 20;
        somConfig.MaxBuffers         = 10;
        somConfig.somCbCompletion    = cbSomCtrl;
        somConfig.pUserContext       = (void *)pCtx;
        somConfig.HalHandle          = pCtx->hHal;
        somConfig.somCtrlHandle      = NULL;

        result = somCtrlInit(&somConfig);
        if (result != RET_SUCCESS)
        {
            TRACE(ISP_TEST_ERROR, "Failed to init SomCtrll:%d\n", result);
            goto err_shutdown_engine;
        }

        pCtx->hSomCtrl = somConfig.somCtrlHandle;
        if (!pCtx->hSomCtrl)
        {
            TRACE(ISP_TEST_ERROR, "hSomCtrl is NULL.\n");
            goto err_shutdown_engine;
        }
    }

    TRACE( ISP_TEST_INFO, "%s (exit)\n", __FUNCTION__);

    return 0;
#if 0
err_shutdown_som:
    if (pCtx->hSomCtrl)
    {
        somCtrlShutDown(pCtx->hSomCtrl);
        pCtx->hSomCtrl = NULL;
    }
#endif
err_shutdown_engine:
    if (pCtx->hCamEngine)
    {
        CamEngineShutDown(pCtx->hCamEngine);
        pCtx->hCamEngine = NULL;
    }

    osEventDestroy(&pCtx->eventEngineStarted);
    osEventDestroy(&pCtx->eventEngineStop);
    osEventDestroy(&pCtx->eventEngineCaptureDone);

//err_close_hal:
    if (pCtx->hHal)
    {
        HalClose(pCtx->hHal);
        pCtx->hHal = NULL;
    }

    TRACE(ISP_TEST_INFO, "%s (exit)\n", __FUNCTION__);

    return ret;
}

/*****************************************************************************/
/**
 * fini()
 *
 * @brief This function is called after every test case.
 *
 *****************************************************************************/
static void
fini(
    TestCtx_t *pCtx
    )
{
    TRACE( ISP_TEST_INFO, "%s (enter)\n", __FUNCTION__);

    /* Shutdown SomCtrl. */
    if (pCtx->settings.som.enable)
    {
        somCtrlStop(pCtx->hSomCtrl);
        osEventWait(&pCtx->eventSomStop);

        osEventDestroy(&pCtx->eventSomFinished);
        osEventDestroy(&pCtx->eventSomStop);
        osEventDestroy(&pCtx->eventSomStarted);

        somCtrlShutDown(pCtx->hSomCtrl);
        pCtx->hSomCtrl = NULL;
    }

    /* Shutdown enigne. */
    CamEngineShutDown(pCtx->hCamEngine);
    //pCtx->hCamEngine;

    /* release test-context */
    osEventDestroy(&pCtx->eventEngineStarted);
    osEventDestroy(&pCtx->eventEngineStop);
    osEventDestroy(&pCtx->eventEngineCaptureDone);

    /* Close HAL. */
    HalClose(pCtx->hHal);
    pCtx->hHal = NULL;

    TRACE( ISP_TEST_INFO, "%s (exit)\n", __FUNCTION__);
}

/*****************************************************************************/
/**
 *
 *
 *****************************************************************************/
cJSON *
jsonObjFromFile(
    char *fileName
    )
{
    FILE *fp;
    long len;
    char *data;
    size_t num;
    cJSON *json = NULL;

    if (!fileName)
    {
        TRACE(ISP_TEST_ERROR, "%s: File name is not set.\n", __FUNCTION__);
        return NULL;
    }

    /* Open config file. */
    fp = fopen(fileName, "r");
    if (!fp)
    {
        TRACE(ISP_TEST_ERROR, "%s: Failed to open case file %s: %s.\n",
                __FUNCTION__, fileName, strerror(errno));
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    len = ftell(fp);

    data = (char *)malloc(len + 1);
    if (!data)
    {
        TRACE(ISP_TEST_ERROR, "Failed to allocate memory with size of %d.\n",
                len + 1);
        goto out_file;
    }

    rewind(fp);

    num = fread(data, len, 1, fp);
    if (num != 1)
    {
        TRACE(ISP_TEST_ERROR, "Failed to read file.\n");
        goto out;
    }

    data[len] = '\0';

    /* Parse. */
    json = cJSON_Parse(data);
    if (!json)
    {
        TRACE(ISP_TEST_ERROR, "Json error: %s.\n", cJSON_GetErrorPtr());
        goto out;
    }

out:
    if (data)
    {
        free(data);
    }

out_file:
    if (fp)
    {
        fclose(fp);
    }

    return json;
}

static int
parseCaseConfig(
    cJSON *json,
    TestCase_t *pTestCase
    )
{
    cJSON *caseConfig;
    cJSON *engineConfig;
    cJSON *mpConfig, *spConfig;
    //cJSON *images;
    cJSON *item;

    CamEngineConfig_t *pConfig = &pTestCase->engineConfig;

    if (!json)
    {
        TRACE(ISP_TEST_ERROR, "Json object is NULL.\n");
        return -EINVAL;
    }

    caseConfig = cJSON_GetObjectItem(json, "CaseConfig");
    if (caseConfig)
    {
        item = cJSON_GetObjectItem(caseConfig, "name");
        if (item)
        {
            char *caseName;

            caseName = cJSON_GetObjectString(item);
            if (caseName)
            {
                strncpy(pTestCase->name, caseName, MAX_LEN);
            }
        }

        engineConfig = cJSON_GetObjectItem(caseConfig, "engineConfig");
        if (engineConfig)
        {
            /* Input type: sensor or image. */
            item = cJSON_GetObjectItem(engineConfig, "pathMode");
            if (item)
            {
                pConfig->mode = cJSON_GetObjectValue(item);
            }

            /* Set gains. */
            if (pConfig->mode == CAM_ENGINE_MODE_IMAGE_PROCESSING) {
                pConfig->data.image.pWbGains  	= &defaultWbGains;
                pConfig->data.image.pCcMatrix 	= &defaultCcMatrix;
                pConfig->data.image.pCcOffset 	= &defaultCcOffset;
                pConfig->data.image.pBlvl     	= &defaultBlvl;
                pConfig->data.image.vGain		= defaultvGain;
                pConfig->data.image.vItime		= defaultvItime;
            }

            /* Main path configuration. */
            mpConfig = cJSON_GetObjectItem(engineConfig, "mainPathConfig");
            if (mpConfig)
            {
                item = cJSON_GetObjectItem(mpConfig, "width");
                if (item)
                {
                    pConfig->pathConfigMaster[CAM_ENGINE_PATH_MAIN].width =
                            cJSON_GetObjectValue(item);
                }

                item = cJSON_GetObjectItem(mpConfig, "height");
                if (item)
                {
                    pConfig->pathConfigMaster[CAM_ENGINE_PATH_MAIN].height =
                            cJSON_GetObjectValue(item);
                }

                item = cJSON_GetObjectItem(mpConfig, "mode");
                if (item)
                {
                    pConfig->pathConfigMaster[CAM_ENGINE_PATH_MAIN].mode =
                            cJSON_GetObjectValue(item);
                }

                item = cJSON_GetObjectItem(mpConfig, "layout");
                if (item)
                {
                    pConfig->pathConfigMaster[CAM_ENGINE_PATH_MAIN].layout =
                            cJSON_GetObjectValue(item);
                }
            }

            /* Self path configuration. */
            spConfig = cJSON_GetObjectItem(engineConfig, "selfPathConfig");
            if (spConfig)
            {
                item = cJSON_GetObjectItem(spConfig, "width");
                if (item)
                {
                    pConfig->pathConfigMaster[CAM_ENGINE_PATH_SELF].width =
                            cJSON_GetObjectValue(item);
                }

                item = cJSON_GetObjectItem(spConfig, "height");
                if (item)
                {
                    pConfig->pathConfigMaster[CAM_ENGINE_PATH_SELF].height =
                            cJSON_GetObjectValue(item);
                }

                item = cJSON_GetObjectItem(spConfig, "mode");
                if (item)
                {
                    pConfig->pathConfigMaster[CAM_ENGINE_PATH_SELF].mode =
                            cJSON_GetObjectValue(item);
                }

                item = cJSON_GetObjectItem(spConfig, "layout");
                if (item)
                {
                    pConfig->pathConfigMaster[CAM_ENGINE_PATH_SELF].layout =
                            cJSON_GetObjectValue(item);
                }
            }
        }

        item = cJSON_GetObjectItem(caseConfig, "rotateAngle");
        if (item)
        {
            pTestCase->orientation = cJSON_GetObjectValue(item);
        }

        item = cJSON_GetObjectItem(caseConfig, "enableDpcc");
        if (item)
        {
            pTestCase->enableDpcc = cJSON_GetObjectValue(item);
        }

        item = cJSON_GetObjectItem(caseConfig, "enableJpeg");
        if (item)
        {
            pTestCase->enableJpeg = cJSON_GetObjectValue(item);
        }

        item = cJSON_GetObjectItem(caseConfig, "frameNum");
        if (item)
        {
            pTestCase->frameNum = cJSON_GetObjectValue(item);
        }

        if (pConfig->mode == CAM_ENGINE_MODE_IMAGE_PROCESSING)
        {
            cJSON *images;

            images = cJSON_GetObjectItem(caseConfig, "images");
            if (images)
            {
                int num, i;
                cJSON *image;

                num = cJSON_GetArraySize(images);
                pTestCase->images = malloc(MAX_LEN * num);

                for (i = 0; i < num; i++)
                {
                    image = cJSON_GetArrayItem(images, i);
                    if (image)
                    {
                        item = cJSON_GetObjectItem(image, "fileName");
                        if (item)
                        {
                            strncpy(&pTestCase->images[i * MAX_LEN],
                                    cJSON_GetObjectString(item), MAX_LEN);
                        }
                    }
                }
            }
        }
    }

    return 0;
}

void
freeConfigResource(
    TestCase_t *pTestCase
    )
{
    if (!pTestCase)
        return;

    if (pTestCase->images)
    {
        free(pTestCase->images);
        pTestCase->images = NULL;
    }
}

static int
loadCaseList(
    const char *fileName,
    char ***caseList,
    int *caseNum
    )
{
    FILE *fp;
    char caseName[MAX_LEN];
    char **list = NULL;
    int num = 0;

    int ret = 0;

    if (!caseList || !caseNum)
    {
        return -EINVAL;
    }

    if ((fileName == NULL) || (strlen(fileName) == 0))
    {
        fp = fopen(DEFAULT_CASE_LIST, "r");
    }
    else
    {
        fp = fopen(fileName, "r");
    }

    if (!fp)
    {
        return -ENODEV;
    }

    num = 0;

    while (fgets(caseName, MAX_LEN, fp))
    {
        if ((caseName[0] == '\n') || (caseName[0] == '#'))
            continue;

        num++;
    }

    list = malloc(num * sizeof(char *));

    fseek(fp, 0, SEEK_SET);

    num = 0;

    while (fgets(caseName, MAX_LEN, fp))
    {
        if (caseName[0] == '\n' || caseName[0] == '#')
        {
            continue;
        }

        if (caseName[strlen(caseName) - 1] == '\n')
        {
            caseName[strlen(caseName) - 1] = '\0';
        }

        list[num] = malloc(MAX_LEN);

        if (list[num])
        {
            strncpy(list[num], caseName, MAX_LEN);
        }

        num++;
    }

    *caseList = list;
    *caseNum  = num;

    fclose(fp);

    return ret;
}

static void
freeCaseList(
    char **caseList,
    int caseNum
    )
{
    int i;

    if (!caseList || !caseNum)
        return;

    for (i = 0; i < caseNum; i++)
    {
        if (caseList[i])
        {
            free(caseList[i]);
        }
    }

    free(caseList);

    return;
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
static void
run(
    TestCtx_t *pCtx
    )
{
    char **caseList = NULL;
    int caseNum = 0;
    char caseName[MAX_LEN];
    cJSON *json;
    CamEngineConfig_t *pConfig;
    somCtrlCmdParams_t params;
    TestCase_t testCase;
    char imageFileName[MAX_LEN];
    PicBufMetaData_t picBuf;
    uint8_t *pBuffer = NULL;

    RESULT result;
    int i, ret = 0;

    TRACE(ISP_TEST_INFO, "%s (enter)\n", __FUNCTION__);

#if 0
    /* Parse calibration data. */
    memset(&caliData, 0, sizeof(caliData));

    json = jsonObjFromFile("case/cali_data.txt");
    if (parseCaseConfig(json, &testCase) < 0)
    {
        TRACE(ISP_TEST_ERROR, "Failed to load case.\n");
        return;
    }
#endif

    ret = loadCaseList("case/list.txt", &caseList, &caseNum);
    if (ret)
    {
        TRACE(ISP_TEST_ERROR, "Failed to read case/list.txt,ret=%d\n", ret);
        return;
    }

    for (i = 0; i < caseNum; i++)
    {
        memset(caseName, 0, MAX_LEN);
        snprintf(caseName, MAX_LEN, "case/%s", caseList[i]);

        TRACE(ISP_TEST_INFO, "CASE [%03d/%03d]: %s\n", i + 1, caseNum,
                caseList[i]);

        /* Parse the case. */
        memset(&testCase, 0, sizeof(testCase));

        json = jsonObjFromFile(caseName);
        if (parseCaseConfig(json, &testCase) < 0)
        {
            TRACE(ISP_TEST_ERROR, "Failed to load case.\n");
            return;
        }

        pConfig = &testCase.engineConfig;

        /* Load the data from file. */
        if (pConfig->mode == CAM_ENGINE_MODE_IMAGE_PROCESSING)
        {
            snprintf(imageFileName, MAX_LEN, "%s/%s", "resource",
                    &testCase.images[0]);
            TRACE(ISP_TEST_INFO, "Load raw file: %s.\n", imageFileName);
            ret = icoUtil_LoadPGMRaw(imageFileName, &picBuf);
            if (ret < 0)
            {
                TRACE(ISP_TEST_ERROR, "%d, fail to load PGM raw image\n", ret);
                return;
            }

            pConfig->data.image.type   = picBuf.Type;
            pConfig->data.image.layout = picBuf.Layout;

            switch (picBuf.Type)
            {
            case PIC_BUF_TYPE_RAW8:
            case PIC_BUF_TYPE_RAW16:
                pConfig->data.image.pBuffer = picBuf.Data.raw.pBuffer;
                pConfig->data.image.width   = picBuf.Data.raw.PicWidthPixel;
                pConfig->data.image.height  = picBuf.Data.raw.PicHeightPixel;

                pBuffer = picBuf.Data.raw.pBuffer;
                break;
            default:
                TRACE(ISP_TEST_ERROR, "Not supported buffer type: %d.\n",
                        picBuf.Type);
                ret = -EINVAL;
                goto err_free_buf;
            }
        }

        /* start the cam-engine instance  */
        result = CamEngineStart(pCtx->hCamEngine, pConfig);
        TEST_ASSERT_EQUAL_INT(RET_PENDING, result);

        /* wait for test-event <= complition of start command */
        TEST_ASSERT_EQUAL_INT(OSLAYER_OK,
                osEventWait(&pCtx->eventEngineStarted));

        /* enable dpcc. */
        if (testCase.enableDpcc)
        {
            result = CamEngineAdpccStart(pCtx->hCamEngine);
            TEST_ASSERT_EQUAL_INT(RET_SUCCESS, result);
        }

        /* Enable Jpe. */
        if (testCase.enableJpeg)
        {
            result = CamEngineEnableJpe(pCtx->hCamEngine, &JpeConfig);
            TEST_ASSERT_EQUAL_INT(RET_SUCCESS, result);
        }

        /* Start SomCtrl. */
        osEventInit(&pCtx->eventSomStarted, 1, 0);
        osEventInit(&pCtx->eventSomStop, 1, 0);
        osEventInit(&pCtx->eventSomFinished, 1, 0);

        params.Start.szBaseFileName = testCase.name;
        params.Start.NumOfFrames = testCase.frameNum;
        /* Just save the last frame. */
        //params.Start.NumSkipFrames = testCase.frameNum - 1;
        params.Start.NumSkipFrames = 0;
        params.Start.AverageFrames = 0;
        params.Start.ForceRGBOut = BOOL_TRUE;
        params.Start.ExtendName  = BOOL_FALSE;

        result = somCtrlStart(pCtx->hSomCtrl, &params);
        if (RET_PENDING == result)
        {
            if (OSLAYER_OK != osEventWait(&pCtx->eventSomStarted))
            {
                TRACE(ISP_TEST_ERROR,
                      "%s (start snapshot output timed out)\n", __FUNCTION__);

                goto err_stop_engine;
            }
        }
        else if (RET_SUCCESS != result)
        {
            TRACE(ISP_TEST_ERROR,
                  "%s (can't start snapshot output)\n", __FUNCTION__);

            goto err_stop_engine;
        }

        /* Start capturing. */
        result = CamEngineStartStreaming(pCtx->hCamEngine, testCase.frameNum);
        TEST_ASSERT_EQUAL_INT(RET_PENDING, result);

        TRACE(ISP_TEST_WARN, "rotation id=%d(<=6)\n", testCase.orientation);

        /* enable Rotation&&Flip */
        switch(testCase.orientation)
        {
        case 1:
            CamEngineSetPictureOrientation(pCtx->hCamEngine, 1);
            break;

        case 2:
            CamEngineSetPictureOrientation(pCtx->hCamEngine, 2);
            break;

        case 3:
            CamEngineSetPictureOrientation(pCtx->hCamEngine, 3);
            break;

        case 4:
            CamEngineSetPictureOrientation(pCtx->hCamEngine, 4);
            break;

        case 5:
            CamEngineSetPictureOrientation(pCtx->hCamEngine, 5);
            break;

        case 6:
            CamEngineSetPictureOrientation(pCtx->hCamEngine, 6);
            break;

        default:
            break;
        }

        osEventWait(&pCtx->eventEngineCaptureDone);

        TRACE( ISP_TEST_INFO, "%s: eventEngineCaptureDone\n", __FUNCTION__);

        osEventWait(&pCtx->eventSomFinished);

        TRACE( ISP_TEST_INFO, "%s: eventSomFinished\n", __FUNCTION__);

        /* stop the cam-engine Adpcc instance */
        if (testCase.enableDpcc)
        {
            result = CamEngineAdpccStop(pCtx->hCamEngine);
            TEST_ASSERT_EQUAL_INT(RET_SUCCESS, result);
        }

        /* disable Jpeg */
        if (testCase.enableJpeg)
        {
            result = CamEngineDisableJpe(pCtx->hCamEngine);
            TEST_ASSERT_EQUAL_INT(RET_SUCCESS, result);
        }

        /* stop the cam-engine instance */
        result = CamEngineStop(pCtx->hCamEngine);
        TEST_ASSERT_EQUAL_INT(RET_PENDING, result);

        /* wait for test-event <= complition of stop command */
        TEST_ASSERT_EQUAL_INT(OSLAYER_OK, osEventWait(&pCtx->eventEngineStop));

        freeConfigResource(&testCase);

        if (pBuffer)
            free(pBuffer);
    }

    freeCaseList(caseList, caseNum);

    TRACE( ISP_TEST_INFO, "%s (exit)\n", __FUNCTION__);

    return;

err_stop_engine:
    CamEngineStop(pCtx->hCamEngine);

err_free_buf:
    if (pBuffer)
        free(pBuffer);

//err_free_list:
    freeCaseList(caseList, caseNum);

    TRACE(ISP_TEST_INFO, "%s (exit)\n", __FUNCTION__);

    return;
}

int main()
{
    TestCtx_t *pCtx;
    int ret = 0;

    pCtx = malloc(sizeof(TestCtx_t));
    if (!pCtx)
    {
        TRACE(ISP_TEST_ERROR, "Failed to alloc TestCtx\n");
        return -ENOMEM;
    }
    memset(pCtx, 0, sizeof(TestCtx_t));

    /* Set global setting file. */
    pCtx->settingFileName = SETTING_FILE_NAME;

    /* Setup. */
    ret = init(pCtx);
    if (ret < 0)
    {
        TRACE(ISP_TEST_ERROR, "Init TestCtx failed,ret=%d\n", ret);
        return ret;
    }

    /* Run test. */
    run(pCtx);

    /* Tear down. */
    fini(pCtx);

    /* uvc display free shared buffer*/
#if IMI_UVC_DISPLAY_ENABLE
    if (deinit_shared_memory() == -1) {
        perror("deinit_shared_memory error");
        return -1;
    }
#endif

    return 0;
}
