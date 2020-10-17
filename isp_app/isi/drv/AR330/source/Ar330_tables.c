#include <ebase/types.h>
#include <ebase/trace.h>
#include <ebase/builtins.h>

#include <common/return_codes.h>

#include "isi.h"
#include "isi_iss.h"
#include "isi_priv.h"

#include "AR330_priv.h"


const IsiRegDescription_t AR330_g_aRegDescription_1080P30[] =
{
	//{ulAddr,    ulDefaultValue,     pszName,         ulFlags}
    { 0x301A,   0x10DD,           "AR330reg",      eReadWrite_16},
//    { 0x31AE,	0x0301, 		  "AR330reg",	   eReadWrite_16},    
    { 0x301A,   0x0058,           "AR330reg",      eReadWrite_16},
//    { 0x301A,	0x0058, 		  "AR330reg",	   eReadWrite_16},    
    { 0x301A,   0x00D8,           "AR330reg",      eReadWrite_16},
    { 0x301A,   0x10D8,           "AR330reg",      eReadWrite_16},
    { 0x3064,   0x1802,           "AR330reg",      eReadWrite_16},
    { 0x3078,   0x0001,           "AR330reg",      eReadWrite_16},
	{ 0x3046,	0x4038, 		  "AR330reg",	   eReadWrite_16},
	{ 0x3048,	0x8480, 		  "AR330reg",	   eReadWrite_16},
	{ 0x3ED2,	0x0146, 		  "AR330reg",	   eReadWrite_16},
	{ 0x3ED6,	0x66CC, 		  "AR330reg",	   eReadWrite_16},
    { 0x3ED8,	0x8C42, 		  "AR330reg",	   eReadWrite_16},
    { 0x302A,	0x0006, 		  "AR330reg",	   eReadWrite_16},
    { 0x302C,	0x0001, 		  "AR330reg",	   eReadWrite_16},
    { 0x302E,	0x0002, 		  "AR330reg",	   eReadWrite_16},
    { 0x3030,	0x0020, 		  "AR330reg",	   eReadWrite_16},
    { 0x3036,	0x000C, 		  "AR330reg",	   eReadWrite_16},
    { 0x3038,	0x0001, 		  "AR330reg",	   eReadWrite_16},
    { 0x301A,	0x10D8, 		  "AR330reg",	   eReadWrite_16},
    { 0x301A,	0x10DC, 		  "AR330reg",	   eReadWrite_16},
    { 0x306E,	0x9C10, 		  "AR330reg",	   eReadWrite_16},
    { 0x306E,	0xFC00, 		  "AR330reg",	   eReadWrite_16},
    { 0x3012,	0x0010, 		  "AR330reg",	   eReadWrite_16},  //ExposureLine
    { 0x301A,	0x10DC, 		  "AR330reg",	   eReadWrite_16},
    { 0x31AE,	0x0301, 		  "AR330reg",	   eReadWrite_16},
    { 0x31E0,	0x0703, 		  "AR330reg",	   eReadWrite_16},
    { 0x3004,	0x00C6, 		  "AR330reg",	   eReadWrite_16},  //X_ADDR_START
    { 0x3008,	0x0845, 		  "AR330reg",	   eReadWrite_16},  //X_ADDR_END
    { 0x3002,	0x00EA, 		  "AR330reg",	   eReadWrite_16},  //Y_ADDR_START
//    { 0x3006,	0x0521, 		  "AR330reg",	   eReadWrite_16},  //Y_ADDR_END
    { 0x3006,	0x0529, 		  "AR330reg",	   eReadWrite_16},  //Y_ADDR_END
    { 0x30A2,	0x0001, 		  "AR330reg",	   eReadWrite_16},
    { 0x30A6,	0x0001, 		  "AR330reg",	   eReadWrite_16},
    { 0x3040,	0x0000, 		  "AR330reg",	   eReadWrite_16},	// | 0X8000 | 0X4000    
    { 0x300C,	0x04DA, 		  "AR330reg",	   eReadWrite_16},  // npixels
    { 0x300A,	0x0467, 		  "AR330reg",	   eReadWrite_16},  //line  
    { 0x3014,	0x0000, 		  "AR330reg",	   eReadWrite_16},
    { 0x30BA,	0x002C, 		  "AR330reg",	   eReadWrite_16},
//    { 0x305E,	0x0085, 		  "AR330reg",	   eReadWrite_16},         //Digital Gain
//    { 0x3060,	0x20 | 0x0A, 		  "AR330reg",	   eReadWrite_16},     //Analog Gain
//    { 0x3086,	0x0253, 		  "AR330reg",	   eReadWrite_16},
    { 0x301A,	0x10D8, 		  "AR330reg",	   eReadWrite_16},
    { 0x3088,	0x80BA, 		  "AR330reg",	   eReadWrite_16},    
    { 0x3086,	0x0253, 		  "AR330reg",	   eReadWrite_16},
    {0x0000,    0x0000,           "TableEnd",      eTableEnd}
};

const IsiRegDescription_t AR330_g_aRegDescription_VGAP30[] =
{
    //{ulAddr,    ulDefaultValue,     pszName,         ulFlags}
    { 0x301A,   0x10D9,           "AR330reg",      eReadWrite_16},
    { 0x3052,   0xA114,           "AR330reg",      eReadWrite_16},
    { 0x304A,   0x0070,           "AR330reg",      eReadWrite_16},
    { 0x31AE,   0x0301,           "AR330reg",      eReadWrite_16},
    { 0x301A,   0x10D8,           "AR330reg",      eReadWrite_16},
#if 1
	{ 0x301A,	0x10D0, 		  "AR330reg",	   eReadWrite_16},
	{ 0x301E,	0x0000, 		  "AR330reg",	   eReadWrite_16}, //by zhangyanna//red purple
#endif
    { 0x301A,   0x10D8,           "AR330reg",      eReadWrite_16}, 
    { 0x3064,   0x1802,           "AR330reg",      eReadWrite_16},
    { 0x3078,	0x0001, 		  "AR330reg",	   eReadWrite_16},
    { 0x31E0,	0x0200, 		  "AR330reg",	   eReadWrite_16},
    { 0x302A,	0x0006, 		  "AR330reg",	   eReadWrite_16},
    { 0x302C,	0x0001, 		  "AR330reg",	   eReadWrite_16},
    { 0x302E,	0x0005, 		  "AR330reg",	   eReadWrite_16},
    { 0x3030,	0x0020, 		  "AR330reg",	   eReadWrite_16},  //by eric 0x0043  -> 0x0020
    { 0x3036,	0x000C, 		  "AR330reg",	   eReadWrite_16},
    { 0x3038,	0x0001, 		  "AR330reg",	   eReadWrite_16},
    { 0x31AC,	0x0C0C, 		  "AR330reg",	   eReadWrite_16},//12-bits
    { 0x31B0,	0x0027, 		  "AR330reg",	   eReadWrite_16},
    { 0x31B2,	0x0010, 		  "AR330reg",	   eReadWrite_16},
    { 0x31B4,	0x2B34, 		  "AR330reg",	   eReadWrite_16},
    { 0x31B6,	0x310A, 		  "AR330reg",	   eReadWrite_16},
    { 0x31B8,	0x2089, 		  "AR330reg",	   eReadWrite_16},
    { 0x31BA,	0x0185, 		  "AR330reg",	   eReadWrite_16},
    { 0x31BC,	0x8004, 		  "AR330reg",	   eReadWrite_16},
    { 0x31BE,	0x2003, 		  "AR330reg",	   eReadWrite_16},

    { 0x3002,	0x0030, 		  "AR330reg",	   eReadWrite_16},
    { 0x3004,	0x00C0, 		  "AR330reg",	   eReadWrite_16},
    { 0x3006,	0x020F, 		  "AR330reg",	   eReadWrite_16},
    { 0x3008,	0x033F, 		  "AR330reg",	   eReadWrite_16},

    { 0x30A2,	0x0001, 		  "AR330reg",	   eReadWrite_16},
    { 0x30A6,	0x0001, 		  "AR330reg",	   eReadWrite_16},
    { 0x300C,	0x04DA, 		  "AR330reg",	   eReadWrite_16},
    { 0x300A,	0x0284, 		  "AR330reg",	   eReadWrite_16},
    { 0x3014,	0x0000, 		  "AR330reg",	   eReadWrite_16},

    { 0x3012,	0x0064, 		  "AR330reg",	   eReadWrite_16},

    { 0x3042,	0x0000, 		  "AR330reg",	   eReadWrite_16},
    { 0x30BA,	0x002C, 		  "AR330reg",	   eReadWrite_16},
    { 0x3ED2,	0x0146, 		  "AR330reg",	   eReadWrite_16},

    { 0x3ED4,	0x8F6C, 		  "AR330reg",	   eReadWrite_16},
    { 0x3ED6,	0x66CC, 		  "AR330reg",	   eReadWrite_16},
    { 0x3ECC,	0x0E0D, 		  "AR330reg",	   eReadWrite_16},

    { 0x301A,	0x10D8, 		  "AR330reg",	   eReadWrite_16},
    { 0x3088,	0x80BA, 		  "AR330reg",	   eReadWrite_16},
    { 0x3086,	0xE653, 		  "AR330reg",	   eReadWrite_16},

    { 0x3060,	0x0010, 		  "AR330reg",	   eReadWrite_16},
    #if 0
    //blue_gain
    { 0x3058,	0x00F0, 		  "AR330reg",	   eReadWrite_16},
    //red_gain
    { 0x305A,	0x0104, 		  "AR330reg",	   eReadWrite_16},
    #else
	//green1_gain
	{ 0x3056,	0x0080, 		  "AR330reg",	   eReadWrite_16},/*0x0080, default*/
    //blue_gain
    { 0x3058,	0x0080, 		  "AR330reg",	   eReadWrite_16},
    //red_gain
    { 0x305A,	0x00c0, 		  "AR330reg",	   eReadWrite_16},
    //green2_gain
    { 0x305C,	0x0080, 		  "AR330reg",	   eReadWrite_16},
	#endif

	//read_mode
	//[15]: read_mode_vert_flip
	//[14]: read_mode_horiz_mirror
	//[13]: read_mode_col_bin
	//[12]: read_mode_row_bin
	//[11]: read_mode_col_bin_cb
	//[10]: read_mode_row_bin_cb
	//[9]: read_mode_col_sf_bin_en
	//[8]:read_mode_col_sf_bin_en_cb
	//7:6]: reserved
	//[5]: read_mode_col_sum
	//4:0]: reserved
    #if 0
    { 0x3040,	0x8000, 		  "AR330reg",	   eReadWrite_16},
    #else
    { 0x3040,	0xc000, 		  "AR330reg",	   eReadWrite_16},
	#endif
	//test_pattern_mode_
    { 0x3070,  0x0000,			 "AR330reg",	   eReadWrite_16},

    {0x0000,    0x0000,           "TableEnd",      eTableEnd}
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
// Important: This value is determinde for AR330_1_CLKIN = 10000000 MHz and
//            need to be changed for other values
const Isi1x1FloatMatrix_t AR330_KFactor =
{
    { 3.944625f }   // or 3.94f (to be checked)
};


// PCA matrix
const Isi3x2FloatMatrix_t AR330_PCAMatrix =
{
    {
        -0.66582f,  -0.07638f,  0.74220f,
         0.47261f,  -0.81292f,  0.34031f
    }
};


// mean values from SVD
const Isi3x1FloatMatrix_t AR330_SVDMeanValue =
{
    {
        0.32893f,  0.37670f,  0.29437f
    }
};


// exposure where the probability for inside and outside lighting is equal (reference sensor)
const Isi1x1FloatMatrix_t AR330_GExpMiddle =
{
    { 0.033892f }
};


// standard deviation of the normal distribution of the inside light sources (sigmaI)
const Isi1x1FloatMatrix_t AR330_VarDistrIn =
{
    { 0.931948f }
};


// mean value of the normal distribution of the inside light sources (muI)
const Isi1x1FloatMatrix_t AR330_MeanDistrIn =
{
    { -3.529613f }
};


// variance of the normal distribution of the outside light sources (sigmaO)
const Isi1x1FloatMatrix_t AR330_VarDistrOut =
{
    { 1.545416f }
};


// mean value of the normal distribution of the outside light sources (muO)
const Isi1x1FloatMatrix_t AR330_MeanDistrOut =
{
    { -6.464629f }
};



/*****************************************************************************
 * Rg/Bg color space (clipping and out of range)
 *****************************************************************************/
/* Center line of polygons { f_N0_Rg, f_N0_Bg, f_d } */
const IsiLine_t AR330_CenterLine =
{
    .f_N0_Rg    = -0.7597481694833567f,
    .f_N0_Bg    = -0.6502174397589539f,
    .f_d        = -1.8497657347968266f
};


/* parameter arrays for Rg/Bg color space clipping */
#define AWB_CLIP_PARM_ARRAY_SIZE_1 16
#define AWB_CLIP_PARM_ARRAY_SIZE_2 16

// top right (clipping area)
float afRg2[AWB_CLIP_PARM_ARRAY_SIZE_2] =
{
    0.95000f,   1.10076f,   1.14336f,   1.18854f,
    1.25372f,   1.31546f,   1.34483f,   1.39015f,
    1.43547f,   1.48079f,   1.52610f,   1.57142f,
    1.61674f,   1.66206f,   1.70738f,   1.75269f
};

float afMaxDist2[AWB_CLIP_PARM_ARRAY_SIZE_2] =
{
    0.08862f,   0.05392f,   0.11547f,   0.14087f,
    0.06364f,  -0.01615f,  -0.01701f,  -0.01825f,
   -0.01774f,  -0.01532f,  -0.01073f,  -0.00379f,
    0.00570f,   0.01789f,   0.03299f,   0.06072f
};

//bottom left (clipping area)
float afRg1[AWB_CLIP_PARM_ARRAY_SIZE_1] =
{
    0.95000f,   1.08378f,   1.15729f,   1.20968f,
    1.25420f,   1.29951f,   1.34483f,   1.39015f,
    1.43547f,   1.48079f,   1.52610f,   1.57142f,
    1.61674f,   1.66206f,   1.70738f,   1.75269f
};

float afMaxDist1[AWB_CLIP_PARM_ARRAY_SIZE_1] =
{
    0.00004f,   0.04196f,   0.03743f,   0.03620f,
    0.01975f,   0.02413f,   0.02701f,   0.02825f,
    0.02774f,   0.02532f,   0.02073f,   0.01379f,
    0.00430f,  -0.00789f,  -0.02299f,  -0.05072f
};

// structure holding pointers to above arrays
// and their sizes
const IsiAwbClipParm_t AR330_AwbClipParm =
{
    .pRg1       = &afRg1[0],
    .pMaxDist1  = &afMaxDist1[0],
    .ArraySize1 = AWB_CLIP_PARM_ARRAY_SIZE_1,
    .pRg2       = &afRg2[0],
    .pMaxDist2  = &afMaxDist2[0],
    .ArraySize2 = AWB_CLIP_PARM_ARRAY_SIZE_2
};



/* parameter arrays for AWB out of range handling */
#define AWB_GLOBAL_FADE1_ARRAY_SIZE 16
#define AWB_GLOBAL_FADE2_ARRAY_SIZE 16

//top right
float afGlobalFade2[AWB_GLOBAL_FADE2_ARRAY_SIZE] =
{
    0.80064f,   0.88574f,   0.96285f,   1.01108f,
    1.05638f,   1.11035f,   1.18078f,   1.28386f,
    1.33501f,   1.41174f,   1.46283f,   1.50743f,
    1.57952f,   1.62709f,   1.72651f,   1.79269f
};

float afGlobalGainDistance2[AWB_GLOBAL_FADE2_ARRAY_SIZE] =
{
    0.15477f,   0.13279f,   0.12127f,   0.12486f,
    0.11925f,   0.17622f,   0.19771f,   0.08278f,
    0.08465f,   0.08746f,   0.10719f,   0.12248f,
    0.10881f,   0.09918f,   0.13533f,   0.17657f
};

//bottom left
float afGlobalFade1[AWB_GLOBAL_FADE1_ARRAY_SIZE] =
{
    0.80291f,   0.86210f,   0.92531f,   1.01277f,
    1.05696f,   1.15824f,   1.21259f,   1.25626f,
    1.32019f,   1.38210f,   1.44976f,   1.52407f,
    1.57684f,   1.66033f,   1.72651f,   1.79269f
};

float afGlobalGainDistance1[AWB_GLOBAL_FADE1_ARRAY_SIZE] =
{
   -0.01758f,  -0.00363f,   0.03190f,   0.09184f,
    0.12491f,   0.19927f,   0.09898f,   0.09375f,
    0.09136f,   0.09258f,   0.09133f,   0.09382f,
    0.09054f,   0.08761f,   0.06467f,   0.02343f
};

// structure holding pointers to above arrays and their sizes
const IsiAwbGlobalFadeParm_t AR330_AwbGlobalFadeParm =
{
    .pGlobalFade1           = &afGlobalFade1[0],
    .pGlobalGainDistance1   = &afGlobalGainDistance1[0],
    .ArraySize1             = AWB_GLOBAL_FADE1_ARRAY_SIZE,
    .pGlobalFade2           = &afGlobalFade2[0],
    .pGlobalGainDistance2   = &afGlobalGainDistance2[0],
    .ArraySize2             = AWB_GLOBAL_FADE2_ARRAY_SIZE
};


/*****************************************************************************
 * Near white pixel discrimination
 *****************************************************************************/
// parameter arrays for near white pixel parameter calculations
#define AWB_FADE2_ARRAY_SIZE 6

float afFade2[AWB_FADE2_ARRAY_SIZE] =
{
    0.50000f,   1.00125f, 1.318535f,
    1.45159f,   1.55305f, 1.640000f
};

float afCbMinRegionMax[AWB_FADE2_ARRAY_SIZE] =
{
    114.000f,   114.000f,   105.000f,
     95.000f,    95.000f,    90.000f
};

float afCrMinRegionMax[AWB_FADE2_ARRAY_SIZE] =
{
     83.000f,    83.000f,   110.000f,
    120.000f,   122.000f,   128.000f
};

float afMaxCSumRegionMax[AWB_FADE2_ARRAY_SIZE] =
{
     28.000f,    27.000f,    18.000f,
     16.000f,     9.000f,     9.000f
};

float afCbMinRegionMin[AWB_FADE2_ARRAY_SIZE] =
{
    123.000f,   123.000f,   123.000f,
    123.000f,   123.000f,   120.000f
};

float afCrMinRegionMin[AWB_FADE2_ARRAY_SIZE] =
{
    123.000f,   123.000f,   123.000f,
    123.000f,   123.000f,   126.000f
};

float afMaxCSumRegionMin[AWB_FADE2_ARRAY_SIZE] =
{
      5.000f,     5.000f,     5.000f,
      5.000f,     5.000f,     5.000f
};

// structure holding pointers to above arrays and their sizes
const IsiAwbFade2Parm_t AR330_AwbFade2Parm =
{
    .pFade              = &afFade2[0],
    .pCbMinRegionMax    = &afCbMinRegionMax[0],
    .pCrMinRegionMax    = &afCrMinRegionMax[0],
    .pMaxCSumRegionMax  = &afMaxCSumRegionMax[0],
    .pCbMinRegionMin    = &afCbMinRegionMin[0],
    .pCrMinRegionMin    = &afCrMinRegionMin[0],
    .pMaxCSumRegionMin  = &afMaxCSumRegionMin[0],
    .ArraySize          = AWB_FADE2_ARRAY_SIZE
};

