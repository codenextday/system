#ifndef __AR330_PRIV_H__
#define __AR330_PRIV_H__

#include <ebase/types.h>
#include <common/return_codes.h>
#include <hal/hal_api.h>



#ifdef __cplusplus
extern "C"
{
#endif




#define AR330_RESET_REGISTER      (0x301A)

#define CHIP_VERSION              (0x3000)
#define DATA_FORMAT_BITS          (0x31AC)
#define X_ADDR_START              (0x3004)
#define X_ADDR_END                (0x3008)
#define Y_ADDR_START              (0x3002)
#define Y_ADDR_END                (0x3006)
#define ANALOG_GAIN               (0x3060)
#define GLOBAL_GAIN               (0x305E)  //digital gain
#define FLASH                     (0x3046)
#define FLASH2                    (0x3048)
#define FRAME_COUNT               (0x303A)
#define DATAPATH_STATUS           (0x306A)
#define TEST_DATA_RED             (0x3072)
#define REVISION_NUMBER           (0x300E)
#define PRE_PLL_CLK_DIV           (0x302E)
#define PLL_MULTIPLIER            (0x3030)
#define VT_SYS_CLK_DIV            (0x302C)
#define VT_PIX_CLK_DIV            (0x302A)
#define LINE_LENGTH_PCK           (0x300C)
#define READ_MODE                 (0x3040)
#define FRAME_LENGTH_LINES        (0x300A)
#define SERIAL_FORMAT             (0x31AE)
#define COARSE_INTEGRATION_TIME   (0x3012)


typedef struct AR330_Context_s
{
    IsiSensorContext_t  IsiCtx;                 /**< common context of ISI and ISI driver layer; @note: MUST BE FIRST IN DRIVER CONTEXT */

    //// modify below here ////

    IsiSensorConfig_t   Config;                 /**< sensor configuration */
    bool_t              Configured;             /**< flags that config was applied to sensor */
    bool_t              Streaming;              /**< flags that csensor is streaming data */

    float               VtPixClkFreq;           /**< pixel clock */
    uint16_t            LineLengthPck;          /**< line length with blanking */
    uint16_t            FrameLengthLines;       /**< frame line length */

    float               AecMinGain;
    float               AecMaxGain;
    float               AecMinIntegrationTime;
    float               AecMaxIntegrationTime;

    float               AecCurGain;
    float               AecCurIntegrationTime;

    uint32_t            OldCoarseIntegrationTime;
} AR330_Context_t;


#ifdef __cplusplus
}
#endif

#endif




