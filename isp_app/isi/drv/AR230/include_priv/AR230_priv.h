#ifndef __AR230_PRIV_H__
#define __AR230_PRIV_H__

#include <ebase/types.h>
#include <common/return_codes.h>
#include <hal/hal_api.h>



#ifdef __cplusplus
extern "C"
{
#endif




#define AR230_RESET_REGISTER      (0x301A)

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
#define AR230_Test_Pattern        (0x3070)  //val 0:No test pattern; 1: solid color; 2:Vertical color bar;
					    //3:Fade to Gray Vertical color bar; 256: walking 1s test pattern;
#define AR230_DGC                (0x30BA)   // R0X30BA[5] Bit5=1 use digtal gain; default is 0
#define AR230_AGC                (0x3060)   // R0X3060[3:0]:fine gain; 0,2,4,6,8,10
					    // R0X3060[5:4]:Coarse gain,
					    //val=0, 1x;
					    //val=1, 2x;
					    //val=2, 4x;
					    //val=3, 8x;

typedef struct AR230_Context_s
{
    IsiSensorContext_t  IsiCtx;                 /**< common context of ISI and ISI driver layer; @note: MUST BE FIRST IN DRIVER CONTEXT */

    //// modify below here ////

    IsiSensorConfig_t   Config;                 /**< sensor configuration */
    bool_t              Configured;             /**< flags that config was applied to sensor */
    bool_t              Streaming;              /**< flags that sensor is streaming data */
    bool_t              TestPattern;            /**< flags that sensor is streaming test-pattern */

    bool_t              isAfpsRun;              /**< if true, just do anything required for Afps parameter calculation, but DON'T access SensorHW! */

    bool_t              GroupHold;

    float               VtPixClkFreq;           /**< pixel clock */
    uint16_t            LineLengthPck;          /**< line length with blanking */
    uint16_t            FrameLengthLines;       /**< frame line length */

    float               AecMaxGain;
    float               AecMinGain;
    float               AecMaxIntegrationTime;
    float               AecMinIntegrationTime;

    float               AecIntegrationTimeIncrement; /**< _smallest_ increment the sensor/driver can handle (e.g. used for sliders in the application) */
    float               AecGainIncrement;            /**< _smallest_ increment the sensor/driver can handle (e.g. used for sliders in the application) */

    float               AecCurGain;
    float               AecCurIntegrationTime;

    uint16_t            OldMultiplier;               /**< gain multiplier */
    uint8_t             OldBase;                     /**< fine gain */
    uint32_t            OldCoarseIntegrationTime;
    uint32_t            OldFineIntegrationTime;
    uint32_t            OldIntegrationTime;
    IsiSensorMipiInfo   IsiSensorMipiInfo;
} AR230_Context_t;


#ifdef __cplusplus
}
#endif

#endif




