/******************************************************************************
 *
 * Copyright 2010, Dream Chip Technologies GmbH.  All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of:
 * Dream Chip Technologies GmbH, Steinriede 10, 30827 Garbsen / Berenbostel,
 * Germany
 *
 *****************************************************************************/
/**
 * @file modulexy_mock.c
 *
 * @brief
 *   File that holds mock functions to cut the dependency to external libs.
 *   If the module gets tested no external libs are linked. Therefor we have
 *   to provide all necessary functions the module uses. With these mock
 *   functions we are able to simulate the exact behavior of the module and
 *   can test all program pathes.
 *
 *****************************************************************************/


int  mockModulexyxReturnStatus = 0;


int ModulexyxInit(void)
{
    return mockModulexyxReturnStatus;
}


int ModulexyxShutdown(void)
{
    return mockModulexyxReturnStatus;
}



