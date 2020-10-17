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
 * @file mapcaps.cpp
 *
 * @brief
 *   Mapping of ISI capabilities / configuration to CamerIC modes.
 *
 *****************************************************************************/
#include "mapcaps.h"


/******************************************************************************
 * local function definitions
 *****************************************************************************/
bool operator==(const CamEngineWindow_t &lhs, const CamEngineWindow_t &rhs)
{
    bool same =  (lhs.hOffset == rhs.hOffset)
              && (lhs.vOffset == rhs.vOffset)
              && (lhs.width   == rhs.width  )
              && (lhs.height  == rhs.height );
    return same;
}

bool operator!=(const CamEngineWindow_t &lhs, const CamEngineWindow_t &rhs)
{
    return !(lhs == rhs);
}


/******************************************************************************
 * local type definitions
 *****************************************************************************/
template <typename  T>
struct IsiCapsMap
{
    uint32_t    cap;
    T           value;
    const char* description;

    static const IsiCapsMap<T> map[];
};

/******************************************************************************
 * API function definitions
 *****************************************************************************/
template <typename  T>
T isiCapValue( uint32_t cap )
{
    for (unsigned int i = 1; i < (sizeof(IsiCapsMap<T>::map) / sizeof(*IsiCapsMap<T>::map)); ++i)
    {
        if ( cap == IsiCapsMap<T>::map[i].cap )
        {
            return IsiCapsMap<T>::map[i].value;
        }
    }
    return IsiCapsMap<T>::map[0].value;
}

template <typename  T>
bool isiCapValue( T& value, uint32_t cap )
{
    value = isiCapValue<T>( cap );
    return (value != IsiCapsMap<T>::map[0].value);
}

template <typename  T>
const char* isiCapDescription( uint32_t cap )
{
    for (unsigned int i = 1; i < (sizeof(IsiCapsMap<T>::map) / sizeof(*IsiCapsMap<T>::map)); ++i)
    {
        if ( cap == IsiCapsMap<T>::map[i].cap )
        {
            return IsiCapsMap<T>::map[i].description;
        }
    }
    return IsiCapsMap<T>::map[0].description;
}

template <typename  T>
bool isiCapDescription( const char* desc, uint32_t cap )
{
    desc = isiCapDescription<T>( cap );
    return (desc != IsiCapsMap<T>::map[0].description);
}

/******************************************************************************
 * local macro definitions
 *****************************************************************************/
#define instantiateCapsMapAndSpecializedFuncs(__type__, ...)                            \
    template<> const IsiCapsMap<__type__> IsiCapsMap<__type__>::map[] = {__VA_ARGS__};  \
    template   __type__ isiCapValue( uint32_t cap );                                    \
    template   bool isiCapValue( __type__& value, uint32_t cap );                       \
    template   const char* isiCapDescription<__type__>( uint32_t cap );                 \
    template   bool isiCapDescription<__type__>( const char* desc, uint32_t cap );


/******************************************************************************
 * specialized API function & internal capability maps instantiations
 *****************************************************************************/
instantiateCapsMapAndSpecializedFuncs
(
    CamerIcIspSampleEdge_t,
    {0                      , CAMERIC_ISP_SAMPLE_EDGE_INVALID   , "CAMERIC_ISP_SAMPLE_EDGE_INVALID"},
    {ISI_EDGE_FALLING       , CAMERIC_ISP_SAMPLE_EDGE_FALLING   , "CAMERIC_ISP_SAMPLE_EDGE_FALLING"},
    {ISI_EDGE_RISING        , CAMERIC_ISP_SAMPLE_EDGE_RISING    , "CAMERIC_ISP_SAMPLE_EDGE_RISING"}
)


instantiateCapsMapAndSpecializedFuncs
(
    CamerIcIspPolarity_t,
    {0                      , CAMERIC_ISP_POLARITY_INVALID      , "CAMERIC_ISP_POLARITY_INVALID"},
    {ISI_HPOL_SYNCPOS       , CAMERIC_ISP_POLARITY_HIGH         , "CAMERIC_ISP_POLARITY_HIGH"},
    {ISI_HPOL_SYNCNEG       , CAMERIC_ISP_POLARITY_LOW          , "CAMERIC_ISP_POLARITY_LOW"},
    {ISI_HPOL_REFPOS        , CAMERIC_ISP_POLARITY_HIGH         , "CAMERIC_ISP_POLARITY_HIGH"},
    {ISI_HPOL_REFNEG        , CAMERIC_ISP_POLARITY_LOW          , "CAMERIC_ISP_POLARITY_LOW"},
    {ISI_VPOL_POS           , CAMERIC_ISP_POLARITY_HIGH         , "CAMERIC_ISP_POLARITY_HIGH"},
    {ISI_VPOL_NEG           , CAMERIC_ISP_POLARITY_LOW          , "CAMERIC_ISP_POLARITY_LOW"}
)


instantiateCapsMapAndSpecializedFuncs
(
    CamerIcIspBayerPattern_t,
    {0                      , CAMERIC_ISP_BAYER_PATTERN_INVALID , "INVALID"},
    {ISI_BPAT_RGRGGBGB      , CAMERIC_ISP_BAYER_PATTERN_RGRGGBGB, "RGRGGBGB"},
    {ISI_BPAT_GRGRBGBG      , CAMERIC_ISP_BAYER_PATTERN_GRGRBGBG, "GRGRBGBG"},
    {ISI_BPAT_GBGBRGRG      , CAMERIC_ISP_BAYER_PATTERN_GBGBRGRG, "GBGBRGRG"},
    {ISI_BPAT_BGBGGRGR      , CAMERIC_ISP_BAYER_PATTERN_BGBGGRGR, "BGBGGRGR"}
)


instantiateCapsMapAndSpecializedFuncs
(
    CamerIcIspColorSubsampling_t,
    {0                      , CAMERIC_ISP_CONV422_INVALID       , "CAMERIC_ISP_CONV422_INVALID"},
    {ISI_CONV422_COSITED    , CAMERIC_ISP_CONV422_COSITED       , "CAMERIC_ISP_CONV422_COSITED"},
    {ISI_CONV422_INTER      , CAMERIC_ISP_CONV422_INTERLEAVED   , "CAMERIC_ISP_CONV422_INTERLEAVED"},
    {ISI_CONV422_NOCOSITED  , CAMERIC_ISP_CONV422_NONCOSITED    , "CAMERIC_ISP_CONV422_NONCOSITED"}
)


instantiateCapsMapAndSpecializedFuncs
(
    CamerIcIspCCIRSequence_t,
    {0                      , CAMERIC_ISP_CCIR_SEQUENCE_INVALID , "CAMERIC_ISP_CCIR_SEQUENCE_INVALID"},
    {ISI_YCSEQ_YCBYCR       , CAMERIC_ISP_CCIR_SEQUENCE_YCbYCr  , "CAMERIC_ISP_CCIR_SEQUENCE_YCbYCr"},
    {ISI_YCSEQ_YCRYCB       , CAMERIC_ISP_CCIR_SEQUENCE_YCrYCb  , "CAMERIC_ISP_CCIR_SEQUENCE_YCrYCb"},
    {ISI_YCSEQ_CBYCRY       , CAMERIC_ISP_CCIR_SEQUENCE_CbYCrY  , "CAMERIC_ISP_CCIR_SEQUENCE_CbYCrY"},
    {ISI_YCSEQ_CRYCBY       , CAMERIC_ISP_CCIR_SEQUENCE_CrYCbY  , "CAMERIC_ISP_CCIR_SEQUENCE_CrYCbY"}
)


instantiateCapsMapAndSpecializedFuncs
(
    CamerIcIspFieldSelection_t,
    {0                      , CAMERIC_ISP_FIELD_SELECTION_INVALID, "CAMERIC_ISP_FIELD_SELECTION_INVALID"},
    {ISI_FIELDSEL_BOTH      , CAMERIC_ISP_FIELD_SELECTION_BOTH  , "CAMERIC_ISP_FIELD_SELECTION_BOTH"},
    {ISI_FIELDSEL_EVEN      , CAMERIC_ISP_FIELD_SELECTION_EVEN  , "CAMERIC_ISP_FIELD_SELECTION_EVEN"},
    {ISI_FIELDSEL_ODD       , CAMERIC_ISP_FIELD_SELECTION_ODD   , "CAMERIC_ISP_FIELD_SELECTION_ODD"}
)


instantiateCapsMapAndSpecializedFuncs
(
    CamerIcIspInputSelection_t,
    {0                      , CAMERIC_ISP_INPUT_INVALID         , "INVALID"},
    {ISI_BUSWIDTH_12BIT     , CAMERIC_ISP_INPUT_12BIT           , "12BIT"},
    {ISI_BUSWIDTH_10BIT_ZZ  , CAMERIC_ISP_INPUT_10BIT_ZZ        , "10BIT_ZZ"},
    {ISI_BUSWIDTH_10BIT_EX  , CAMERIC_ISP_INPUT_10BIT_EX        , "10BIT_EX"},
    {ISI_BUSWIDTH_8BIT_ZZ   , CAMERIC_ISP_INPUT_8BIT_ZZ         , "8BIT_ZZ"},
    {ISI_BUSWIDTH_8BIT_EX   , CAMERIC_ISP_INPUT_8BIT_EX         , "8BIT_EX"}
)


instantiateCapsMapAndSpecializedFuncs
(
    CamerIcIspMode_t,
    {0                      , CAMERIC_ISP_MODE_INVALID          , "CAMERIC_ISP_MODE_INVALID"},
    {ISI_MODE_PICT          , CAMERIC_ISP_MODE_RAW              , "CAMERIC_ISP_MODE_RAW"},
    {ISI_MODE_BT656         , CAMERIC_ISP_MODE_656              , "CAMERIC_ISP_MODE_656"},
    {ISI_MODE_BT601         , CAMERIC_ISP_MODE_601              , "CAMERIC_ISP_MODE_601"},
    {ISI_MODE_BAYER         , CAMERIC_ISP_MODE_BAYER_RGB        , "CAMERIC_ISP_MODE_BAYER_RGB"},
    {ISI_MODE_DATA          , CAMERIC_ISP_MODE_DATA             , "CAMERIC_ISP_MODE_DATA"},
    {ISI_MODE_BAY_BT656     , CAMERIC_ISP_MODE_RGB656           , "CAMERIC_ISP_MODE_RGB656"},
    {ISI_MODE_RAW_BT656     , CAMERIC_ISP_MODE_RAW656           , "CAMERIC_ISP_MODE_RAW656"},
    {ISI_MODE_MIPI          , CAMERIC_ISP_MODE_BAYER_RGB        , "CAMERIC_ISP_MODE_BAYER_RGB"}
)


instantiateCapsMapAndSpecializedFuncs
(
    MipiDataType_t,
    {0                            , MIPI_DATA_TYPE_MAX            , "INVALID"},
    {ISI_MIPI_OFF                 , MIPI_DATA_TYPE_MAX            , "OFF"},
    {ISI_MIPI_MODE_YUV420_8       , MIPI_DATA_TYPE_YUV420_8       , "YUV420_8"},
    {ISI_MIPI_MODE_YUV420_10      , MIPI_DATA_TYPE_YUV420_10      , "YUV420_10"},
    {ISI_MIPI_MODE_LEGACY_YUV420_8, MIPI_DATA_TYPE_LEGACY_YUV420_8, "LEGACY_YUV420_8"},
    {ISI_MIPI_MODE_YUV420_CSPS_8  , MIPI_DATA_TYPE_YUV420_8_CSPS  , "YUV420_8_CSPS"},
    {ISI_MIPI_MODE_YUV420_CSPS_10 , MIPI_DATA_TYPE_YUV420_10_CSPS , "YUV420_10_CSPS"},
    {ISI_MIPI_MODE_YUV422_8       , MIPI_DATA_TYPE_YUV422_8       , "YUV422_8"},
    {ISI_MIPI_MODE_YUV422_10      , MIPI_DATA_TYPE_YUV422_10      , "YUV422_10"},
    {ISI_MIPI_MODE_RGB444         , MIPI_DATA_TYPE_RGB444         , "RGB444"},
    {ISI_MIPI_MODE_RGB555         , MIPI_DATA_TYPE_RGB555         , "RGB555"},
    {ISI_MIPI_MODE_RGB565         , MIPI_DATA_TYPE_RGB565         , "RGB565"},
    {ISI_MIPI_MODE_RGB666         , MIPI_DATA_TYPE_RGB666         , "RGB666"},
    {ISI_MIPI_MODE_RGB888         , MIPI_DATA_TYPE_RGB888         , "RGB888"},
    {ISI_MIPI_MODE_RAW_6          , MIPI_DATA_TYPE_RAW_6          , "RAW_6"},
    {ISI_MIPI_MODE_RAW_7          , MIPI_DATA_TYPE_RAW_7          , "RAW_7"},
    {ISI_MIPI_MODE_RAW_8          , MIPI_DATA_TYPE_RAW_8          , "RAW_8"},
    {ISI_MIPI_MODE_RAW_10         , MIPI_DATA_TYPE_RAW_10         , "RAW_10"},
    {ISI_MIPI_MODE_RAW_12         , MIPI_DATA_TYPE_RAW_12         , "RAW_12"}
)


instantiateCapsMapAndSpecializedFuncs
(
    CamEngineWindow_t,
    {0                , {0, 0, 0, 0}          , "RES_INVALID"},
    {ISI_RES_VGA      , {0, 0, 640 ,  480}    , "RES_VGA"},
    {ISI_RES_VGAP30   , {0, 0, 640 ,  480}    , "RES_VGAP30"},
    {ISI_RES_TV720P5  , {0, 0, 1280,  720}    , "RES_TV720P"},
    {ISI_RES_TV720P15 , {0, 0, 1280,  720}    , "RES_TV720P"},
    {ISI_RES_TV720P30 , {0, 0, 1280,  720}    , "RES_TV720P"},
    {ISI_RES_TV720P60 , {0, 0, 1280,  720}    , "RES_TV720P"},
    {ISI_RES_1280_960P30 , {0, 0, 1280,  960} , "RES_1280_960P"},
    {ISI_RES_TV1080P5 , {0, 0, 1920, 1080}    , "RES_TV1080P"},
    {ISI_RES_TV1080P6 , {0, 0, 1920, 1080}    , "RES_TV1080P"},
    {ISI_RES_TV1080P10, {0, 0, 1920, 1080}    , "RES_TV1080P"},
    {ISI_RES_TV1080P12, {0, 0, 1920, 1080}    , "RES_TV1080P"},
    {ISI_RES_TV1080P15, {0, 0, 1920, 1080}    , "RES_TV1080P"},
    {ISI_RES_TV1080P20, {0, 0, 1920, 1080}    , "RES_TV1080P"},
    {ISI_RES_TV1080P24, {0, 0, 1920, 1080}    , "RES_TV1080P"},
    {ISI_RES_TV1080P25, {0, 0, 1920, 1080}    , "RES_TV1080P"},
    {ISI_RES_TV1080P30, {0, 0, 1920, 1080}    , "RES_TV1080P"},
    {ISI_RES_TV1080P50, {0, 0, 1920, 1080}    , "RES_TV1080P"},
    {ISI_RES_TV1080P60, {0, 0, 1920, 1080}    , "RES_TV1080P"},
    {ISI_RES_2592_1944, {0, 0, 2592, 1944}    , "RES_2592_1944"},
    {ISI_RES_3264_2448, {0, 0, 3264, 2448}    , "RES_3264_2448"},
    {ISI_RES_4416_3312, {0, 0, 4416, 3312}    , "RES_4416_3312"}
)
