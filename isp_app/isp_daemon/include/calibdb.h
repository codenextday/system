/******************************************************************************
 *
 * Copyright 2011, Dream Chip Technologies GmbH. All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of:
 * Dream Chip Technologies GmbH, Steinriede 10, 30827 Garbsen / Berenbostel,
 * Germany
 *
 *****************************************************************************/
/**
 * @file    calibtreewidget.h
 *
 *
 *****************************************************************************/
#ifndef __CALIBDB_H__
#define __CALIBDB_H__

#include <QDomDocument>
#include <QtXml>

class CalibDb
{
public:
	CalibDb( );
	~CalibDb( );

	CamCalibDbHandle_t GetCalibDbHandle( void )
	{
		return ( m_CalibDbHandle );
	}

	bool CreateCalibDb( const QDomElement & );
	bool CreateCalibDb( QFile *device );

private:
	typedef bool (CalibDb::*parseCellContent)(const QDomElement&, void *param);

	// parse helper
	bool parseEntryCell( const QDomElement&, int, parseCellContent, void *param = NULL );

	// parse Header
	bool parseEntryHeader( const QDomElement&, void *param = NULL );
	bool parseEntryResolution( const QDomElement&, void *param = NULL );

	bool parseEntryFramerates( const QDomElement&, void *param = NULL );

	// parse Sensor
	bool parseEntrySensor( const QDomElement&, void *param = NULL );

	// parse Sensor-AWB
	bool parseEntryAwb( const QDomElement&, void *param = NULL );
	bool parseEntryAwbGlobals( const QDomElement&, void *param = NULL );
	bool parseEntryAwbIllumination( const QDomElement&, void *param = NULL );
	bool parseEntryAwbIlluminationAlsc( const QDomElement&, void *param = NULL );
	bool parseEntryAwbIlluminationAcc( const QDomElement&, void *param = NULL );

	// parse Sensor-AEC
	bool parseEntryAec( const QDomElement&, void *param = NULL );
	bool parseEntryAecEcm( const QDomElement&, void *param = NULL );
	bool parseEntryAecEcmPriorityScheme( const QDomElement&, void *param = NULL );

	// parse Sensor-LSC
	bool parseEntryLsc( const QDomElement&, void *param = NULL );

	// parse Sensor-CC
	bool parseEntryCc( const QDomElement&, void *param = NULL );

	// parse Sensor-BLS
	bool parseEntryBls( const QDomElement&, void *param = NULL );

	// parse Sensor-CAC
	bool parseEntryCac( const QDomElement&, void *param = NULL );

	// parse Sensor-DPF
	bool parseEntryDpf( const QDomElement&, void *param = NULL );

	// parse Sensor-DPCC
	bool parseEntryDpcc( const QDomElement&, void *param = NULL );
	bool parseEntryDpccRegisters( const QDomElement&, void *param = NULL );

	// parse System
	bool parseEntrySystem( const QDomElement&, void *param = NULL );

private:

	CamCalibDbHandle_t  m_CalibDbHandle;
};


#endif /* __CALIBDB_H__ */
