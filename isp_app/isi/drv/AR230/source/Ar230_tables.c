#include <ebase/types.h>
#include <ebase/trace.h>
#include <ebase/builtins.h>
#include <common/return_codes.h>

#include "isi.h"
#include "isi_iss.h"
#include "isi_priv.h"
#include "AR230_priv.h"

//#define AR0230_TRIGGER

const IsiRegDescription_t AR230_g_aRegDescription[] = {
	{0x301A, 0x0001,"",eReadWrite_16},	// RESET_REGISTER
	{0x0000,0x0064,"",eDelay},// 0x00c8,"",eDelay},
#ifdef AR0230_TRIGGER
	{0x30CE, 0x0010,"",eReadWrite_16},    // RESET_REGISTER
	{0x0000, 0x000a,"",eDelay},
	{0x301A, 0x11D8,"",eReadWrite_16},    // RESET_REGISTER
#else
	{0x301A, 0x10D8,"",eReadWrite_16},    // RESET_REGISTER
#endif
	{0x3088, 0x8242,"",eReadWrite_16},    // SEQ_CTRL_PORT
	{0x3086, 0x4558,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x729B,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x4A31,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x4342,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x8E03,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x2A14,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x4578,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x7B3D,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0xFF3D,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0xFF3D,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0xEA2A,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x043D,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x102A,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x052A,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x1535,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x2A05,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x3D10,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x4558,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x2A04,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x2A14,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x3DFF,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x3DFF,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x3DEA,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x2A04,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x622A,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x288E,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x0036,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x2A08,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x3D64,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x7A3D,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x0444,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x2C4B,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x8F03,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x430D,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x2D46,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x4316,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x5F16,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x530D,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x1660,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x3E4C,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x2904,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x2984,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x8E03,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x2AFC,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x5C1D,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x5754,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x495F,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x5305,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x5307,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x4D2B,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0xF810,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x164C,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x0955,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x562B,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0xB82B,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x984E,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x1129,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x9460,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x5C19,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x5C1B,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x4548,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x4508,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x4588,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x29B6,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x8E01,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x2AF8,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x3E02,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x2AFA,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x3F09,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x5C1B,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x29B2,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x3F0C,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x3E03,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x3E15,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x5C13,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x3F11,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x3E0F,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x5F2B,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x902A,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0xF22B,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x803E,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x063F,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x0660,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x29A2,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x29A3,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x5F4D,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x1C2A,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0xFA29,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x8345,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0xA83E,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x072A,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0xFB3E,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x2945,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x8824,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x3E08,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x2AFA,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x5D29,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x9288,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x102B,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x048B,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x1686,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x8D48,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x4D4E,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x2B80,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x4C0B,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x603F,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x302A,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0xF23F,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x1029,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x8229,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x8329,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x435C,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x155F,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x4D1C,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x2AFA,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x4558,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x8E00,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x2A98,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x3F0A,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x4A0A,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x4316,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x0B43,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x168E,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x032A,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x9C45,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x783F,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x072A,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x9D3E,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x305D,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x2944,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x8810,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x2B04,"",eReadWrite_16},	// SEQ_DATA_PORT
	{0x3086, 0x530D,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x4558,"",eReadWrite_16},	// SEQ_DATA_PORT
	{0x3086, 0x3E08,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x8E01,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x2A98,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x8E00,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x769C,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x779C,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x4644,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x1616,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x907A,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x1244,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x4B18,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x4A04,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x4316,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x0643,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x1605,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x4316,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x0743,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x1658,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x4316,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x5A43,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x1645,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x588E,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x032A,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x9C45,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x787B,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x3F07,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x2A9D,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x530D,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x8B16,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x863E,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x2345,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x5825,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x3E10,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x8E01,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x2A98,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x8E00,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x3E10,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x8D60,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x1244,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x4B2C,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3086, 0x2C2C,"",eReadWrite_16},    // SEQ_DATA_PORT
	{0x3ED6, 0x34B3,"",eReadWrite_16},	// DAC_LD_10_11
	{0x2436, 0x000E,"",eReadWrite_16},    // ALTM_CONTROL_AVERAGED_L
	{0x320C, 0x0180,"",eReadWrite_16},    // ADACD_GAIN_THRESHOLD_0
	{0x320E, 0x0300,"",eReadWrite_16},    // ADACD_GAIN_THRESHOLD_1
	{0x3210, 0x0500,"",eReadWrite_16},    // ADACD_GAIN_THRESHOLD_2
	{0x3204, 0x0B6D,"",eReadWrite_16},    // ADACD_NOISE_MODEL2
	{0x30FE, 0x0080,"",eReadWrite_16},    // NOISE_PEDESTAL
	{0x3ED8, 0x7B99,"",eReadWrite_16},    // DAC_LD_12_13
	{0x3EDC, 0x9BA8,"",eReadWrite_16},    // DAC_LD_16_17
	{0x3EDA, 0x9B9B,"",eReadWrite_16},    // DAC_LD_14_15
	{0x3092, 0x006F,"",eReadWrite_16},    // ROW_NOISE_CONTROL
	{0x3EEC, 0x1C04,"",eReadWrite_16},    // DAC_LD_32_33
	{0x30BA, 0x779C,"",eReadWrite_16},    // DIGITAL_CTRL
	{0x3EF6, 0xA70F,"",eReadWrite_16},    // DAC_LD_42_43
	{0x3044, 0x0410,"",eReadWrite_16},    // DARK_CONTROL
	{0x3ED0, 0xFF44,"",eReadWrite_16},    // DAC_LD_4_5
	{0x3ED4, 0x031F,"",eReadWrite_16},    // DAC_LD_8_9
	{0x30FE, 0x0080,"",eReadWrite_16},    // NOISE_PEDESTAL
	{0x3EE2, 0x8866,"",eReadWrite_16},    // DAC_LD_22_23
	{0x3EE4, 0x6623,"",eReadWrite_16},    // DAC_LD_24_25
	{0x3EE6, 0x2263,"",eReadWrite_16},    // DAC_LD_26_27
	{0x30E0, 0x4283,"",eReadWrite_16},    // ADC_COMMAND1
	{0x30F0, 0x1283,"",eReadWrite_16},    // ADC_COMMAND1_HS
#ifdef AR0230_TRIGGER
	{0x301A, 0x11D8,"",eReadWrite_16},	// RESET_REGISTER
#else
	{0x301A, 0x10D8,"",eReadWrite_16},	// RESET_REGISTER
#endif
	{0x30B0, 0x1118,"",eReadWrite_16},    // DIGITAL_TEST
	{0x31AC, 0x0C0C,"",eReadWrite_16},    // DATA_FORMAT_BITS

#ifdef AR0230_TRIGGER
	{0x301A, 0x11D8,"",eReadWrite_16},	// RESET_REGISTER
#else
	{0x301A, 0x10D8,"",eReadWrite_16},    //RESET_REGISTER = 4312
#endif
// 74.25 Mhz
	{0x302A, 0x0008,"",eReadWrite_16},    //VT_PIX_CLK_DIV = 6
	{0x302C, 0x0001,"",eReadWrite_16},    //VT_SYS_CLK_DIV = 2
	{0x302E, 0x0008,"",eReadWrite_16},    //PRE_PLL_CLK_DIV = 2
	{0x3030, 0x00c6,"",eReadWrite_16},    //PLL_MULTIPLIER = 48
	{0x3036, 0x0006,"",eReadWrite_16},    //OP_PIX_CLK_DIV = 6
	{0x3038, 0x0001,"",eReadWrite_16},    //OP_SYS_CLK_DIV = 1
	{0x30A2, 0x0001,"",eReadWrite_16},    // X_ODD_INC
	{0x30A6, 0x0001,"",eReadWrite_16},    // Y_ODD_INC
	{0x30AE, 0x0001,"",eReadWrite_16},    // X_ODD_INC_CB
	{0x30A8, 0x0001,"",eReadWrite_16},    // Y_ODD_INC_CB
	{0x3040, 0xC000,"",eReadWrite_16},    // READ_MODE
	{0x31AE, 0x0301,"",eReadWrite_16},    // SERIAL_FORMAT
	{0x3082, 0x0009,"",eReadWrite_16},    // OPERATION_MODE_CTRL
	{0x30BA, 0x769C,"",eReadWrite_16},    // DIGITAL_CTRL

	// 640*480
	{0x3002, 0x0130,"",eReadWrite_16},    //Y_ADDR_START = 304
	{0x3004, 0x028C,"",eReadWrite_16},    //X_ADDR_START = 652
	{0x3006, 0x030F,"",eReadWrite_16},    //Y_ADDR_END = 783
	{0x3008, 0x050B,"",eReadWrite_16},    //X_ADDR_END = 1291
	{0x300A, 0x01F0,"",eReadWrite_16},    //FRAME_LENGTH_LINES = 496
	{0x300C, 0x064C,"",eReadWrite_16},    //LINE_LENGTH_PCK = 1612
	{0x3012, 0x01F0,"",eReadWrite_16},    //COARSE_INTEGRATION_TIME
	{0x3096, 0x0080,"",eReadWrite_16},    // ROW_NOISE_ADJUST_TOP
	{0x3098, 0x0080,"",eReadWrite_16},    // ROW_NOISE_ADJUST_BTM
	{0x31E0, 0x0200,"",eReadWrite_16},    // PIX_DEF_ID
	{0x318C, 0x0000,"",eReadWrite_16},    // HDR_MC_CTRL2
	{0x3176, 0x0080,"",eReadWrite_16},    // DELTA_DK_ADJUST_GREENR
	{0x3178, 0x0080,"",eReadWrite_16},    // DELTA_DK_ADJUST_RED
	{0x317A, 0x0080,"",eReadWrite_16},    // DELTA_DK_ADJUST_BLUE
	{0x317C, 0x0080,"",eReadWrite_16},    // DELTA_DK_ADJUST_GREENB
	{0x3060, 0x000B,"",eReadWrite_16},    // ANALOG_GAIN
	{0x3206, 0x0B08,"",eReadWrite_16},    // ADACD_NOISE_FLOOR1
	{0x3208, 0x1E13,"",eReadWrite_16},    // ADACD_NOISE_FLOOR2
	{0x3202, 0x0080,"",eReadWrite_16},    // ADACD_NOISE_MODEL1
	{0x3200, 0x0002,"",eReadWrite_16},    // ADACD_CONTROL
	{0x3100, 0x0000,"",eReadWrite_16},    // AECTRLREG
	{0x3200, 0x0000,"",eReadWrite_16},    // ADACD_CONTROL
	{0x31D0, 0x0000,"",eReadWrite_16},    // COMPANDING
	{0x2400, 0x0003,"",eReadWrite_16},    // ALTM_CONTROL
	{0x301E, 0x00A8,"",eReadWrite_16},    // DATA_PEDESTAL
	{0x2450, 0x0000,"",eReadWrite_16},    // ALTM_OUT_PEDESTAL
	{0x320A, 0x0080,"",eReadWrite_16},    // ADACD_PEDESTAL
	{0x3064, 0x1982,"",eReadWrite_16},    // SMIA_TEST
	{0x3064, 0x1802,"",eReadWrite_16},	// SMIA_TEST
	{0x0000, 0x0021,"",eDelay},  		  //Delay 33ms
	{0x0000, 0x0000,"TableEnd", eTableEnd}
};

// 640*480
const IsiRegDescription_t AR230_g_vga[] = {
	{0x3002, 0x0130, "AR230reg", eReadWrite_16},    //Y_ADDR_START = 304
	{0x3004, 0x028C, "AR230reg", eReadWrite_16},    //X_ADDR_START = 652
	{0x3006, 0x030F, "AR230reg", eReadWrite_16},    //Y_ADDR_END = 783
	{0x3008, 0x050B, "AR230reg", eReadWrite_16},    //X_ADDR_END = 1291
	{0x300A, 0x01F0, "AR230reg", eReadWrite_16},    //FRAME_LENGTH_LINES = 496
	{0x300C, 0x064C, "AR230reg", eReadWrite_16},    //LINE_LENGTH_PCK = 1612
	{0x3012, 0x01F0, "AR230reg", eReadWrite_16},    //COARSE_INTEGRATION_TIME

	{0x0000, 0x0000, "TableEnd", eTableEnd}
};

// 800*600
const IsiRegDescription_t AR230_g_svga[] = {
	{0x3002, 0x00F4, "AR230reg", eReadWrite_16},    //Y_ADDR_START = 244
	{0x3004, 0x02A0, "AR230reg", eReadWrite_16},    //X_ADDR_START = 672
	{0x3006, 0x034B, "AR230reg", eReadWrite_16},    //Y_ADDR_END = 843
	{0x3008, 0x05BF, "AR230reg", eReadWrite_16},    //X_ADDR_END = 1471
	{0x300A, 0x0268, "AR230reg", eReadWrite_16},    //FRAME_LENGTH_LINES = 616
	{0x300C, 0x0512, "AR230reg", eReadWrite_16},    //LINE_LENGTH_PCK = 1298
	{0x3012, 0x0115, "AR230reg", eReadWrite_16},    //COARSE_INTEGRATION_TIME= 277

	{0x0000, 0x0000, "TableEnd", eTableEnd}
};

// 1920*1080
const IsiRegDescription_t AR230_g_1080p[] = {
	{0x3002, 0x0004, "AR230reg", eReadWrite_16}, // Y_ADDR_START
	{0x3004, 0x000c, "AR230reg", eReadWrite_16}, // X_ADDR_START
	{0x3006, 0x043b, "AR230reg", eReadWrite_16}, // Y_ADDR_END
	{0x3008, 0x078b, "AR230reg", eReadWrite_16}, // X_ADDR_END
	//74.25
	{0x300A, 0x0452, "AR230reg", eReadWrite_16}, // FRAME_LENGTH_LINES
	{0x300C, 0x045e, "AR230reg", eReadWrite_16}, // LINE_LENGTH_PCK

	{0x3012, 0x019e, "AR230reg", eReadWrite_16}, // COARSE_INTEGRATION_TIME
	{0x3028, 0x0020, "AR230reg", eReadWrite_16}, // COARSE_INTEGRATION_TIME
	{0x0000, 0x0000, "TableEnd", eTableEnd}
};

/*****************************************************************************
 * AWB-Calibration data
 *****************************************************************************/

// Calibration (e.g. Matlab) is done in double precision. Parameters are then stored and handle as float
// types here in the software. Finally these parameters are written to hardware registers with fixed
// point precision.
// Some thoughts about the error between a real value, rounded to a constant with a finite number of
// fractional digits, and the resulting binary fixed point value:
// The total absolute error is the absolute error of the conversion real to constant plus the absolute
// error of the conversion from the constant to fixed point.
// For example the distance between two figures of a a fixed point value with 8 bit fractional part
// is 1/256. The max. absolute error is half of that, thus 1/512. So 3 digits fractional part could
// be chosen for the constant with an absolut error of 1/2000. The total absolute error would then be
// 1/2000 + 1/512.
// To avoid any problems we take one more digit. And another one to account for error propagation in
// the calculations of the SLS algorithms. Finally we end up with reasonable 5 digits fractional part.

/*****************************************************************************
 *
 *****************************************************************************/
// K-Factor
// calibration factor to map exposure of current sensor to the exposure of the
// reference sensor
//
// Important: This value is determinde for AR230_1_CLKIN = 10000000 MHz and
//            need to be changed for other values
const Isi1x1FloatMatrix_t AR230_KFactor = {
	{4.5676f}		// or 3.94f (to be checked)
};

// PCA matrix
const Isi3x2FloatMatrix_t AR230_PCAMatrix = {
	{
	 -0.7660542806120959f, 0.1383423852294128f, 0.6277118953826835f,
	 0.2825376184201862f, -0.8046912768979814f, 0.5221536584777949f}
};

// mean values from SVD
const Isi3x1FloatMatrix_t AR230_SVDMeanValue = {
	{0.3278795832272791f, 0.4189362029226504f, 0.2531842138500700f}
};

// exposure where the probability for inside and outside lighting is equal (reference sensor)
const Isi1x1FloatMatrix_t AR230_GExpMiddle = {
	{0.033892f}
};

// standard deviation of the normal distribution of the inside light sources (sigmaI)
const Isi1x1FloatMatrix_t AR230_VarDistrIn = {
	{0.931948f}
};

// mean value of the normal distribution of the inside light sources (muI)
const Isi1x1FloatMatrix_t AR230_MeanDistrIn = {
	{-3.529613f}
};

// variance of the normal distribution of the outside light sources (sigmaO)
const Isi1x1FloatMatrix_t AR230_VarDistrOut = {
	{1.545416f}
};

// mean value of the normal distribution of the outside light sources (muO)
const Isi1x1FloatMatrix_t AR230_MeanDistrOut = {
	{-6.464629f}
};

/*****************************************************************************
 * Rg/Bg color space (clipping and out of range)
 *****************************************************************************/
/* Center line of polygons { f_N0_Rg, f_N0_Bg, f_d } */
const IsiLine_t AR230_CenterLine = {
	.f_N0_Rg = -0.7248305072753748f,
	.f_N0_Bg = -0.6889272354341370f,
	.f_d = -2.1688399600854176f
};

/* parameter arrays for Rg/Bg color space clipping */
#define AWB_CLIP_PARM_ARRAY_SIZE_1 16
#define AWB_CLIP_PARM_ARRAY_SIZE_2 16

// top right (clipping area)
float afRg2[AWB_CLIP_PARM_ARRAY_SIZE_2] = {
	1.06666f, 1.10293f, 1.12503f, 1.15103f,
	1.19279f, 1.24176f, 1.29234f, 1.33601f,
	1.37792f, 1.41074f, 1.45642f, 1.50874f,
	1.56137f, 1.59956f, 1.63327f, 1.64000f
};

float afMaxDist2[AWB_CLIP_PARM_ARRAY_SIZE_2] = {
	0.10656f, 0.12370f, 0.15198f, 0.19169f,
	0.23905f, 0.29745f, 0.32003f, 0.31018f,
	0.31953f, 0.32044f, 0.32460f, 0.32798f,
	0.30996f, 0.26987f, 0.20656f, 0.05814f
};

//bottom left (clipping area)
float afRg1[AWB_CLIP_PARM_ARRAY_SIZE_1] = {
	1.07000f, 1.10800f, 1.14600f, 1.18400f,
	1.22200f, 1.26000f, 1.29800f, 1.33600f,
	1.37400f, 1.41200f, 1.45000f, 1.48800f,
	1.52600f, 1.56400f, 1.60200f, 1.64000f
};

float afMaxDist1[AWB_CLIP_PARM_ARRAY_SIZE_1] = {
	0.06571f, 0.06187f, 0.05994f, 0.05834f,
	0.05705f, 0.05606f, 0.05536f, 0.05496f,
	0.05485f, 0.05503f, 0.05549f, 0.05623f,
	0.05721f, 0.05851f, 0.06004f, 0.06186f
};

// structure holding pointers to above arrays
// and their sizes
const IsiAwbClipParm_t AR230_AwbClipParm = {
	.pRg1 = &afRg1[0],
	.pMaxDist1 = &afMaxDist1[0],
	.ArraySize1 = AWB_CLIP_PARM_ARRAY_SIZE_1,
	.pRg2 = &afRg2[0],
	.pMaxDist2 = &afMaxDist2[0],
	.ArraySize2 = AWB_CLIP_PARM_ARRAY_SIZE_2
};

/* parameter arrays for AWB out of range handling */
#define AWB_GLOBAL_FADE1_ARRAY_SIZE 16
#define AWB_GLOBAL_FADE2_ARRAY_SIZE 16

//top right
float afGlobalFade2[AWB_GLOBAL_FADE2_ARRAY_SIZE] = {
	0.80000f, 0.85867f, 0.91733f, 0.97600f,
	1.03467f, 1.07903f, 1.14651f, 1.20472f,
	1.26253f, 1.32973f, 1.38784f, 1.45723f,
	1.52793f, 1.57999f, 1.63164f, 1.68000f
};

float afGlobalGainDistance2[AWB_GLOBAL_FADE2_ARRAY_SIZE] = {
	0.16036f, 0.17740f, 0.18354f, 0.18895f,
	0.19349f, 0.22386f, 0.27771f, 0.36115f,
	0.39476f, 0.41540f, 0.42761f, 0.42424f,
	0.40648f, 0.37665f, 0.29699f, 0.19597f
};

//bottom left
float afGlobalFade1[AWB_GLOBAL_FADE1_ARRAY_SIZE] = {
	0.80000f, 0.85867f, 0.91733f, 0.97600f,
	1.03467f, 1.09333f, 1.15200f, 1.21067f,
	1.26933f, 1.32800f, 1.38667f, 1.44533f,
	1.50400f, 1.56267f, 1.62133f, 1.68000f
};

float afGlobalGainDistance1[AWB_GLOBAL_FADE1_ARRAY_SIZE] = {
	0.23964f, 0.22260f, 0.21646f, 0.21105f,
	0.20651f, 0.20270f, 0.19953f, 0.19741f,
	0.19584f, 0.19503f, 0.19488f, 0.19542f,
	0.19661f, 0.19841f, 0.20095f, 0.20403f
};

// structure holding pointers to above arrays and their sizes
const IsiAwbGlobalFadeParm_t AR230_AwbGlobalFadeParm = {
	.pGlobalFade1 = &afGlobalFade1[0],
	.pGlobalGainDistance1 = &afGlobalGainDistance1[0],
	.ArraySize1 = AWB_GLOBAL_FADE1_ARRAY_SIZE,
	.pGlobalFade2 = &afGlobalFade2[0],
	.pGlobalGainDistance2 = &afGlobalGainDistance2[0],
	.ArraySize2 = AWB_GLOBAL_FADE2_ARRAY_SIZE
};

/*****************************************************************************
 * Near white pixel discrimination
 *****************************************************************************/
// parameter arrays for near white pixel parameter calculations
#define AWB_FADE2_ARRAY_SIZE 6

float afFade2[AWB_FADE2_ARRAY_SIZE] = {
	0.9f, 1.3f, 1.4f,
	1.5f, 1.6f, 1.7f
};

float afCbMinRegionMax[AWB_FADE2_ARRAY_SIZE] = {
	132.000f, 130.000f, 128.000f,
	128.000f, 128.000f, 126.000f
};

float afCrMinRegionMax[AWB_FADE2_ARRAY_SIZE] = {
	123.000f, 123.000f, 125.000f,
	128.000f, 130.000f, 132.000f
};

float afMaxCSumRegionMax[AWB_FADE2_ARRAY_SIZE] = {
	16.000f, 14.000f, 14.000f,
	12.000f, 10.000f, 10.000f
};

float afCbMinRegionMin[AWB_FADE2_ARRAY_SIZE] = {
	123.000f, 123.000f, 123.000f,
	123.000f, 123.000f, 123.000f
};

float afCrMinRegionMin[AWB_FADE2_ARRAY_SIZE] = {
	123.000f, 123.000f, 123.000f,
	123.000f, 123.000f, 123.000f
};

float afMaxCSumRegionMin[AWB_FADE2_ARRAY_SIZE] = {
	5.000f, 5.000f, 5.000f,
	5.000f, 5.000f, 5.000f
};

// structure holding pointers to above arrays and their sizes
const IsiAwbFade2Parm_t AR230_AwbFade2Parm = {
	.pFade = &afFade2[0],
	.pCbMinRegionMax = &afCbMinRegionMax[0],
	.pCrMinRegionMax = &afCrMinRegionMax[0],
	.pMaxCSumRegionMax = &afMaxCSumRegionMax[0],
	.pCbMinRegionMin = &afCbMinRegionMin[0],
	.pCrMinRegionMin = &afCrMinRegionMin[0],
	.pMaxCSumRegionMin = &afMaxCSumRegionMin[0],
	.ArraySize = AWB_FADE2_ARRAY_SIZE
};
