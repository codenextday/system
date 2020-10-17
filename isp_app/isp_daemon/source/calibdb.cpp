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
 * @file    calibdb.cpp
 *
 *****************************************************************************/
#include <ebase/builtins.h>
#include <ebase/dct_assert.h>
#include <ebase/trace.h>
#include <common/return_codes.h>
#include <common/cam_types.h>
#include <cam_calibdb/cam_calibdb_api.h>

#include <QtDebug>

#include "calibdb.h"
#include "calibtags.h"
#include "xmltags.h"


/******************************************************************************
 * local macro definitions
 *****************************************************************************/
CREATE_TRACER(CALIBDB_INFO,	"CALIBDB:", INFO,	1);
CREATE_TRACER(CALIBDB_WARN,	"CALIBDB:", WARNING,	1);
CREATE_TRACER(CALIBDB_ERROR,	"CALIBDB:", ERROR,	1);
CREATE_TRACER(CALIBDB_DEBUG,	"CALIBDB:", INFO,	1);

/******************************************************************************
 * ParseFloatArray
 *****************************************************************************/
static int ParseFloatArray
(
    const char  *c_string,          /**< trimmed c string */
    float       *values,            /**< pointer to memory */
    const int   num                 /**< number of expected float values */
)
{
	float *value = values;
	char *str = (char *)c_string;

	int last = strlen( str );

	/* check for beginning/closing parenthesis */
	if ( (str[0]!='[') || (str[last-1]!=']') )
	{
		return ( -1 );
	}

	/* calc. end adress of string */
	char *str_last = str + (last-1);

	/* skipped left parenthesis */
	str++;

	/* skip spaces */
	while ( *str == 0x20 )
	{
		str++;
	}

	int cnt = 0;
	int scanned;
	float f;

	/* parse the c-string */
	while ( (str != str_last) && (cnt<num) )
	{
		scanned = sscanf(str, "%f", &f);
		if ( scanned != 1 )
		{
			goto err1;
		}
		else
		{
			value[cnt] = f;
			cnt++;
		}

		/* remove detected float */
		while ( (*str != 0x20)	&& (*str != ',') && (*str != ']') )
		{
			str++;
		}

		/* skip spaces and comma */
		while ( (*str == 0x20) || (*str == ',') )
		{
			str++;
		}
	}

	return ( cnt );

err1:
	MEMSET( values, 0, (sizeof(float) * num) );

	return ( 0 );

}



/******************************************************************************
 * ParseUshortArray
 *****************************************************************************/
static int ParseUshortArray
(
    const char  *c_string,          /**< trimmed c string */
    uint16_t    *values,            /**< pointer to memory */
    const int   num                 /**< number of expected float values */
)
{
	uint16_t *value = values;
	char *str = (char *)c_string;

	int last = strlen( str );

	/* check for beginning/closing parenthesis */
	if ( (str[0]!='[') || (str[last-1]!=']') )
	{
		return ( -1 );
	}

	/* calc. end adress of string */
	char *str_last = str + (last-1);

	/* skipped left parenthesis */
	str++;

	/* skip spaces */
	while ( *str == 0x20 )
	{
		str++;
	}

	int cnt = 0;
	int scanned;
	uint16_t f;

	/* parse the c-string */
	while ( (str != str_last) && (cnt<num) )
	{
		scanned = sscanf(str, "%hu", &f);
		if ( scanned != 1 )
		{
			goto err1;
		}
		else
		{
			value[cnt] = f;
			cnt++;
		}

		/* remove detected float */
		while ( (*str != 0x20)	&& (*str != ',') && (*str != ']') )
		{
			str++;
		}

		/* skip spaces and comma */
		while ( (*str == 0x20) || (*str == ',') )
		{
			str++;
		}
	}

	return ( cnt );

err1:
	MEMSET( values, 0, (sizeof(uint16_t) * num) );

	return ( 0 );

}


/******************************************************************************
 * ParseShortArray
 *****************************************************************************/
static int ParseShortArray
(
    const char  *c_string,          /**< trimmed c string */
    int16_t     *values,            /**< pointer to memory */
    const int   num                 /**< number of expected float values */
)
{
	int16_t *value	= values;
	char *str		= (char *)c_string;

	int last = strlen( str );

	/* check for beginning/closing parenthesis */
	if ( (str[0]!='[') || (str[last-1]!=']') )
	{
		return ( -1 );
	}

	/* calc. end adress of string */
	char *str_last = str + (last-1);

	/* skipped left parenthesis */
	str++;

	/* skip spaces */
	while ( *str == 0x20 )
	{
		str++;
	}

	int cnt = 0;
	int scanned;
	int16_t f;

	/* parse the c-string */
	while ( (str != str_last) && (cnt<num) )
	{
		scanned = sscanf(str, "%hd", &f);
		if ( scanned != 1 )
		{
			goto err1;
		}
		else
		{
			value[cnt] = f;
			cnt++;
		}

		/* remove detected float */
		while ( (*str != 0x20)	&& (*str != ',') && (*str != ']') )
		{
			str++;
		}

		/* skip spaces and comma */
		while ( (*str == 0x20) || (*str == ',') )
		{
			str++;
		}
	}

	return ( cnt );

err1:
	MEMSET( values, 0, (sizeof(uint16_t) * num) );

	return ( 0 );

}



/******************************************************************************
 * ParseByteArray
 *****************************************************************************/
static int ParseByteArray
(
    const char  *c_string,          /**< trimmed c string */
    uint8_t     *values,            /**< pointer to memory */
    const int   num                 /**< number of expected float values */
)
{
	uint8_t *value	= values;
	char *str		= (char *)c_string;

	int last = strlen( str );

	/* check for beginning/closing parenthesis */
	if ( (str[0]!='[') || (str[last-1]!=']') )
	{
		return ( -1 );
	}

	/* calc. end adress of string */
	char *str_last = str + (last-1);

	/* skipped left parenthesis */
	str++;

	/* skip spaces */
	while ( *str == 0x20 )
	{
		str++;
	}

	int cnt = 0;
	int scanned;
	uint16_t f;

	/* parse the c-string */
	while ( (str != str_last) && (cnt<num) )
	{
		scanned = sscanf(str, "%hu", &f);
		if ( scanned != 1 )
		{
			goto err1;
		}
		else
		{
			value[cnt] = (uint8_t)f;
			cnt++;
		}

		/* remove detected float */
		while ( (*str != 0x20)	&& (*str != ',') && (*str != ']') )
		{
			str++;
		}

		/* skip spaces and comma */
		while ( (*str == 0x20) || (*str == ',') )
		{
			str++;
		}
	}

	return ( cnt );

err1:
	MEMSET( values, 0, (sizeof(uint8_t) * num) );

	return ( 0 );

}


/******************************************************************************
 * ParseCcProfileArray
 *****************************************************************************/
static int ParseCcProfileArray
(
    const char          *c_string,          /**< trimmed c string */
    CamCcProfileName_t  values[],           /**< pointer to memory */
    const int           num                 /**< number of expected float values */
)
{
    char *str = (char *)c_string;

	int last = strlen( str );

	/* calc. end adress of string */
	char *str_last = str + (last-1);

	/* skip beginning spaces */
	while ( *str == 0x20 )
	{
		str++;
	}

	/* skip ending spaces */
	while ( *str_last == 0x20 )
	{
		str_last--;
	}

	int cnt = 0;
	int scanned;
	CamCcProfileName_t f;
	memset( f, 0, sizeof(f) );

	/* parse the c-string */
	while ( (str != str_last) && (cnt<num) )
	{
		scanned = sscanf(str, "%s", f);
		if ( scanned != 1 )
		{
			goto err1;
		}
		else
		{
			strncpy( values[cnt], f, strlen( f ) );
			cnt++;
		}

		/* remove detected string */
		while ( (*str != 0x20)	&& (*str != ',') && (*str != ']') && (str!=str_last) )
		{
			str++;
		}

		if ( str != str_last )
		{
			/* skip spaces and comma */
			while ( (*str == 0x20) || (*str == ',') )
			{
				str++;
			}
		}

		memset( f, 0, sizeof(f) );
	}

	return ( cnt );

err1:
	memset( values, 0, (sizeof(uint16_t) * num) );

	return ( 0 );
}



/******************************************************************************
 * ParseLscProfileArray
 *****************************************************************************/
static int ParseLscProfileArray
(
    const char          *c_string,          /**< trimmed c string */
    CamLscProfileName_t values[],           /**< pointer to memory */
    const int           num                 /**< number of expected float values */
)
{
	char *str = (char *)c_string;

	int last = strlen( str );

	/* calc. end adress of string */
	char *str_last = str + (last-1);

	/* skip beginning spaces */
	while ( *str == 0x20 )
	{
		str++;
	}

	/* skip ending spaces */
	while ( *str_last == 0x20 )
	{
		str_last--;
	}

	int cnt = 0;
	int scanned;
	CamLscProfileName_t f;
	memset( f, 0, sizeof(f) );

	/* parse the c-string */
	while ( (str != str_last) && (cnt<num) )
	{
		scanned = sscanf(str, "%s", f);
		if ( scanned != 1 )
		{
			goto err1;
		}
		else
		{
			strncpy( values[cnt], f, strlen( f ) );
			cnt++;
		}

		/* remove detected string */
		while ( (*str != 0x20)	&& (*str != ',') && (*str != ']') && (str!=str_last) )
		{
			str++;
		}

		if ( str != str_last )
		{
			/* skip spaces and comma */
			while ( (*str == 0x20) || (*str == ',') )
			{
				str++;
			}
		}

		memset( f, 0, sizeof(f) );
	}

	return ( cnt );

err1:
	memset( values, 0, (sizeof(uint16_t) * num) );

	return ( 0 );
}




/******************************************************************************
 * CalibDb::CalibDb
 *****************************************************************************/
CalibDb::CalibDb
(
)
{
	m_CalibDbHandle = NULL;
}



/******************************************************************************
 * CalibReader::CalibReader
 *****************************************************************************/
CalibDb::~CalibDb()
{
	if ( m_CalibDbHandle != NULL )
	{
		RESULT result = CamCalibDbRelease( &m_CalibDbHandle );
		DCT_ASSERT( result == RET_SUCCESS );
	}
}



/******************************************************************************
 * CalibDb::CalibDb
 *****************************************************************************/
bool CalibDb::CreateCalibDb
(
    const QDomElement  &root
)
{
	bool res = true;

	// create calibration-database (c-code)
	RESULT result = CamCalibDbCreate( &m_CalibDbHandle );
	DCT_ASSERT( result == RET_SUCCESS );

	TRACE(CALIBDB_INFO, "%s (enter)\n", __func__);
	// get and parse header section
	QDomElement header = root.firstChildElement( CALIB_HEADER_TAG );
	if ( !header.isNull() )
	{
		res = parseEntryHeader( header.toElement(), NULL );
		if ( !res )
		{
			return ( res );
		}
	}

	// get and parse sensor section
	QDomElement sensor = root.firstChildElement( CALIB_SENSOR_TAG );
	if ( !sensor.isNull() )
	{
		res = parseEntrySensor( sensor.toElement(), NULL );
		if ( !res )
		{
			return ( res );
		}
	}

	// get and parse system section
	QDomElement system = root.firstChildElement( CALIB_SYSTEM_TAG );
	if ( !system.isNull() )
	{
		res = parseEntrySystem( system.toElement(), NULL );
		if ( !res )
		{
			return ( res );
		}
	}

	TRACE(CALIBDB_INFO, "%s (exit)\n", __func__);
	return ( true );
}



/******************************************************************************
 * CalibDb::readFile
 *****************************************************************************/
bool CalibDb::CreateCalibDb
(
    QFile *device
)
{
	QString errorString;
	int errorLine;
	int errorColumn;

	QDomDocument doc;

	bool res = true;

	TRACE(CALIBDB_INFO, "%s (enter)\n", __func__);
	RESULT result = CamCalibDbCreate( &m_CalibDbHandle );
	DCT_ASSERT( result == RET_SUCCESS );

	if ( !doc.setContent( device, false, &errorString, &errorLine, &errorColumn) )
	{
		TRACE(CALIBDB_ERROR, "%s Error: Parse error at line %d, column"
					" %d: %s\n", __func__, errorLine, errorColumn,
					qPrintable(errorString));
		return ( false);
	}

	QDomElement root = doc.documentElement();
	if ( root.tagName() != CALIB_FILESTART_TAG )
	{
		TRACE(CALIBDB_ERROR, "%s Error: Not a calibration data file\n",
					__func__);
		return ( false );
	}

	// parse header section
	QDomElement header = root.firstChildElement( CALIB_HEADER_TAG );
	if ( !header.isNull() )
	{
		res = parseEntryHeader( header.toElement(), NULL );
		if ( !res )
		{
			return ( res );
		}
	}

	// parse sensor section
	QDomElement sensor = root.firstChildElement( CALIB_SENSOR_TAG );
	if ( !sensor.isNull() )
	{
		res = parseEntrySensor( sensor.toElement(), NULL );
		if ( !res )
		{
			return ( res );
		}
	}

	// parse system section
	QDomElement system = root.firstChildElement( CALIB_SYSTEM_TAG );
	if ( !system.isNull() )
	{
		res = parseEntrySystem( system.toElement(), NULL );
		if ( !res )
		{
			return ( res );
		}
	}

	TRACE(CALIBDB_INFO, "%s (exit)\n", __func__);
	return ( res );
}




/******************************************************************************
 * CalibDb::parseEntryCell
 *****************************************************************************/
bool CalibDb::parseEntryCell
(
    const QDomElement   &element,
    int                 noElements,
    parseCellContent    func,
    void                *param
)
{
	int cnt = 0;

	TRACE(CALIBDB_INFO, "%s (enter)\n", __func__);
	QDomNode child = element.firstChild();
	while ( !child.isNull() && (cnt < noElements) )
	{
		XmlCellTag tag = XmlCellTag( child.toElement() );
		if ( child.toElement().tagName() == CALIB_CELL_TAG )
		{
			bool result = (this->*func)( child.toElement(), param );
			if ( !result )
			{
				return ( result );
			}
		}
		else
		{
			TRACE(CALIBDB_ERROR, "%s unknown cell tag: %s\n",
					__func__, child.toElement().tagName());
			return ( false );
		}

		child = child.nextSibling();
		cnt ++;
	}

	TRACE(CALIBDB_INFO, "%s (exit)\n", __func__);
	return ( true );
}



/******************************************************************************
 * CalibDb::parseEntryHeader
 *****************************************************************************/
bool CalibDb::parseEntryHeader
(
    const QDomElement   &element,
    void                *param
)
{
	(void)param;

	TRACE(CALIBDB_INFO, "%s (enter)\n", __func__);
	CamCalibDbMetaData_t meta_data;
	MEMSET( &meta_data, 0, sizeof( meta_data ) );

	QDomNode child = element.firstChild();
	while ( !child.isNull() )
	{
		XmlTag tag = XmlTag( child.toElement() );
		QString value = tag.Value();

		if ( (child.toElement().tagName() == CALIB_HEADER_CREATION_DATE_TAG)
				&& (tag.isType( XmlTag::TAG_TYPE_CHAR ))
				&& (tag.Size() > 0) )
		{
			strncpy( meta_data.cdate, value.toAscii().constData(), sizeof( meta_data.cdate ) );
		}
		else if ( (child.toElement().tagName() == CALIB_HEADER_CREATOR_TAG)
					&& (tag.isType( XmlTag::TAG_TYPE_CHAR ))
					&& (tag.Size() > 0) )
		{
			strncpy( meta_data.cname, value.toAscii().constData(), sizeof( meta_data.cname ) );
		}
		else if ( (child.toElement().tagName() == CALIB_HEADER_GENERATOR_VERSION_TAG)
					&& (tag.isType( XmlTag::TAG_TYPE_CHAR ))
					&& (tag.Size() > 0) )
		{
			strncpy( meta_data.cversion, value.toAscii().constData(), sizeof( meta_data.cversion ) );
		}
		else if ( (child.toElement().tagName() == CALIB_HEADER_SENSOR_NAME_TAG)
					&& (tag.isType( XmlTag::TAG_TYPE_CHAR ))
					&& (tag.Size() > 0) )
		{
			strncpy( meta_data.sname, value.toAscii().constData(), sizeof( meta_data.sname ) );
		}
		else if ( (child.toElement().tagName() == CALIB_HEADER_SAMPLE_NAME_TAG)
					&& (tag.isType( XmlTag::TAG_TYPE_CHAR ))
					&& (tag.Size() > 0) )
		{
			strncpy( meta_data.sid, value.toAscii().constData(), sizeof( meta_data.sid ) );
		}
		else if ( child.toElement().tagName() == CALIB_HEADER_RESOLUTION_TAG )
		{
			if ( !parseEntryCell( child.toElement(), tag.Size(), &CalibDb::parseEntryResolution ) )
			{
				TRACE(CALIBDB_ERROR, "%s parse error in header"
						"resolution section(%s)\n", __func__, child.toElement().tagName());
				return ( false );
			}
		}
		else
		{
			TRACE(CALIBDB_ERROR, "%s parse error in header section"
						"(unknown tag: %s)\n", __func__,
						child.toElement().tagName());
			return ( false );
		}

		child = child.nextSibling();
	}

	RESULT result = CamCalibDbSetMetaData( m_CalibDbHandle, &meta_data );
	DCT_ASSERT( result == RET_SUCCESS );

	TRACE(CALIBDB_INFO, "%s (exit)\n", __func__);
	return ( true );
}



/******************************************************************************
 * CalibDb::parseEntryResolution
 *****************************************************************************/
bool CalibDb::parseEntryResolution
(
    const QDomElement   &element,
    void                *param
)
{
	(void)param;

	TRACE(CALIBDB_INFO, "%s (enter)\n", __func__);
	CamResolution_t resolution;
	MEMSET( &resolution, 0, sizeof( resolution ) );
	ListInit( &resolution.framerates );

	QDomNode child = element.firstChild();
	while ( !child.isNull() )
	{
		XmlTag tag = XmlTag( child.toElement() );
		QString value = tag.Value();

		if ( (child.toElement().tagName() == CALIB_HEADER_RESOLUTION_NAME_TAG)
				&& (tag.isType( XmlTag::TAG_TYPE_CHAR ))
				&& (tag.Size() > 0) )
		{
			strncpy( resolution.name, value.toAscii().constData(), sizeof( resolution.name ) );
		}
		else if ( (child.toElement().tagName() == CALIB_HEADER_RESOLUTION_WIDTH_TAG)
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0) )
		{
			int no = ParseUshortArray( value.toAscii().constData(),  &resolution.width, 1 );
			DCT_ASSERT( (no == 1) );
		}
		else if ( (child.toElement().tagName() == CALIB_HEADER_RESOLUTION_HEIGHT_TAG)
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0) )
		{
			int no = ParseUshortArray( value.toAscii().constData(), &resolution.height, 1 );
			DCT_ASSERT( (no == 1) );
		}
		else if ( (child.toElement().tagName() == CALIB_HEADER_RESOLUTION_FRATE_TAG)
					&& (tag.isType( XmlTag::TAG_TYPE_CELL ))
					&& (tag.Size() > 0) )
		{
			if ( !parseEntryCell( child.toElement(), tag.Size(), &CalibDb::parseEntryFramerates, &resolution) )
			{
				TRACE(CALIBDB_ERROR, "%s parse error in DPF"
						"section (unknown tag:%s)\n",
						__func__,
						child.toElement().tagName());
				return ( false );
			}
		}
		else if ( child.toElement().tagName() == CALIB_HEADER_RESOLUTION_ID_TAG )
		{
			bool ok;

			resolution.id = tag.ValueToUInt( &ok );
			if ( !ok )
			{
				TRACE(CALIBDB_ERROR, "%s parse error:invalid"
						"resolution %s / %d\n",__func__,
						child.toElement().tagName(),
						tag.Value());
				return ( false );
			}
		}
		else
		{
			TRACE(CALIBDB_ERROR, "%s unknown tag: %s\n", __func__,
					child.toElement().tagName());
			return ( false );
		}

		child = child.nextSibling();
	}

	RESULT result = CamCalibDbAddResolution( m_CalibDbHandle, &resolution );
	DCT_ASSERT( result == RET_SUCCESS );

	// free linked framerates
	List *l = ListRemoveHead( &resolution.framerates );
	while ( l )
	{
		free( l );
		l = ListRemoveHead( l );
	}

	TRACE(CALIBDB_INFO, "%s (exit)\n", __func__);
	return ( true );
}


/******************************************************************************
 * CalibDb::parseEntryDpccRegisters
 *****************************************************************************/
bool CalibDb::parseEntryFramerates
(
    const QDomElement   &element,
    void                *param
)
{
	(void)param;

	TRACE(CALIBDB_INFO, "%s (enter)\n", __func__);
	CamResolution_t *pResolution = (CamResolution_t *)param;
	CamFrameRate_t *pFrate = (CamFrameRate_t *) malloc( sizeof(CamFrameRate_t) );
	if ( !pFrate )
	{
		return false;
	}
	MEMSET( pFrate, 0, sizeof(*pFrate) );

	QDomNode child = element.firstChild();
	while ( !child.isNull() )
	{
		XmlTag tag = XmlTag( child.toElement() );
		QString value = tag.Value();

		if ( (child.toElement().tagName() == CALIB_HEADER_RESOLUTION_FRATE_NAME_TAG)
				&& (tag.isType( XmlTag::TAG_TYPE_CHAR ))
				&& (tag.Size() > 0) )
		{
			snprintf( pFrate->name, CAM_FRAMERATE_NAME, "%s_%s",
						pResolution->name, value.toAscii().constData() );
		}
		else if ( (child.toElement().tagName() == CALIB_HEADER_RESOLUTION_FRATE_FPS_TAG)
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0) )
		{
			int no = ParseFloatArray( value.toAscii().constData(), &pFrate->fps, 1 );
			DCT_ASSERT( (no == 1) );
		}
		else
		{
			TRACE(CALIBDB_ERROR, "%s parse error in framerate"
					"section (unknown tag: %s)\n", __func__,
					child.toElement().tagName());
			return ( false );
		}

		child = child.nextSibling();
	}

	if ( pResolution )
	{
		ListPrepareItem( pFrate );
		ListAddTail( &pResolution->framerates, pFrate );
	}

	TRACE(CALIBDB_INFO, "%s (exit)\n", __func__);
	return ( true );
}


/******************************************************************************
 * CalibDb::parseEntrySensor
 *****************************************************************************/
bool CalibDb::parseEntrySensor
(
    const QDomElement   &element,
    void                *param
)
{
	(void)param;

	TRACE(CALIBDB_INFO, "%s (enter)\n", __func__);
	QDomNode child = element.firstChild();
	while ( !child.isNull() )
	{
		XmlTag tag = XmlTag( child.toElement() );

		if ( child.toElement().tagName() == CALIB_SENSOR_AWB_TAG )
		{
			if ( !parseEntryAwb( child.toElement() ) )
			{
				return ( false );
			}
		}
		else if ( child.toElement().tagName() == CALIB_SENSOR_LSC_TAG )
		{
			if ( !parseEntryCell( child.toElement(), tag.Size(), &CalibDb::parseEntryLsc ) )
			{
				return ( false );
			}
		}
		else if ( child.toElement().tagName() == CALIB_SENSOR_CC_TAG )
		{
			if ( !parseEntryCell( child.toElement(), tag.Size(), &CalibDb::parseEntryCc ) )
			{
				return ( false );
			}
		}
		else if ( child.toElement().tagName() == CALIB_SENSOR_AF_TAG )
		{
			TRACE(CALIBDB_INFO, "%s tag: %s\n",
						child.toElement().tagName().trimmed());
		}
		else if ( child.toElement().tagName() == CALIB_SENSOR_AEC_TAG )
		{
			TRACE(CALIBDB_INFO, "%s tag: %s\n", __func__,
					child.toElement().tagName());
			if ( !parseEntryAec( child.toElement() ) )
			{
				return ( false );
			}
		}
		else if ( child.toElement().tagName() == CALIB_SENSOR_BLS_TAG )
		{
			if ( !parseEntryCell( child.toElement(), tag.Size(), &CalibDb::parseEntryBls ) )
			{
				TRACE(CALIBDB_ERROR, "%s parse error in BLS"
						"section (unknown tag: %s)\n",
						__func__,
						child.toElement().tagName());
				return ( false );
			}
		}
		else if ( child.toElement().tagName() == CALIB_SENSOR_DEGAMMA_TAG )
		{
			TRACE(CALIBDB_INFO, "%s tag: %s\n", __func__,
					child.toElement().tagName());
		}
		else if ( child.toElement().tagName() == CALIB_SENSOR_WDR_TAG )
		{
			TRACE(CALIBDB_INFO, "%s tag: %s\n", __func__,
					child.toElement().tagName());
		}
		else if ( child.toElement().tagName() == CALIB_SENSOR_CAC_TAG )
		{
			if ( !parseEntryCell( child.toElement(), tag.Size(), &CalibDb::parseEntryCac ) )
			{
				TRACE(CALIBDB_ERROR, "%s parse error in CAC"
						"section (unknown tag:%s)\n",
						__func__,
						child.toElement().tagName());
				return ( false );
			}
		}
		else if ( child.toElement().tagName() == CALIB_SENSOR_DPF_TAG )
		{
			if ( !parseEntryCell( child.toElement(), tag.Size(), &CalibDb::parseEntryDpf ) )
			{
				TRACE(CALIBDB_ERROR, "%s parse error in DPF"
						"section (unknown tag:%s)\n",
						__func__,
						child.toElement().tagName());
				return ( false );
			}
		}
		else if ( child.toElement().tagName() == CALIB_SENSOR_DPCC_TAG )
		{
			if ( !parseEntryCell( child.toElement(), tag.Size(), &CalibDb::parseEntryDpcc ) )
			{
				TRACE(CALIBDB_ERROR, "%s parse error in DPCC"
						"section (unknown tag:%s)\n",
						__func__,
						child.toElement().tagName());
				return ( false );
			}
		}
		else
		{
			TRACE(CALIBDB_ERROR, "%s parse error in header"
					"section (unknown tag:%s)\n",
					__func__,
					child.toElement().tagName());
			return ( false );
		}

		child = child.nextSibling();
	}

	TRACE(CALIBDB_INFO, "%s (exit)\n", __func__);

	return ( true );
}



/******************************************************************************
 * CalibDb::parseEntryAec
 *****************************************************************************/
bool CalibDb::parseEntryAec
(
    const QDomElement   &element,
    void                *param
)
{
	(void)param;

	TRACE(CALIBDB_INFO, "%s (enter)\n", __func__);

	CamCalibAecGlobal_t aec_data;

	QDomNode child = element.firstChild();
	while ( !child.isNull() )
	{
		XmlTag tag = XmlTag( child.toElement() );

		if ( (child.toElement().tagName() == CALIB_SENSOR_AEC_SETPOINT_TAG)
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0) )
		{
			int no = ParseFloatArray( tag.Value().toAscii().constData(), &aec_data.SetPoint , 1 );
			DCT_ASSERT( (no == tag.Size()) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_AEC_CLM_TOLERANCE_TAG)
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0) )
		{
			int no = ParseFloatArray( tag.Value().toAscii().constData(), &aec_data.ClmTolerance, 1 );
			DCT_ASSERT( (no == tag.Size()) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_AEC_DAMP_OVER_TAG )
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0) )
		{
			int no = ParseFloatArray( tag.Value().toAscii().constData(), &aec_data.DampOverStill, 1 );
			DCT_ASSERT( (no == tag.Size()) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_AEC_DAMP_UNDER_TAG )
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0) )
		{
			int no = ParseFloatArray( tag.Value().toAscii().constData(), &aec_data.DampUnderStill, 1 );
			DCT_ASSERT( (no == tag.Size()) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_AEC_DAMP_OVER_VIDEO_TAG )
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0) )
		{
			int no = ParseFloatArray( tag.Value().toAscii().constData(), &aec_data.DampOverVideo, 1 );
			DCT_ASSERT( (no == tag.Size()) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_AEC_DAMP_UNDER_VIDEO_TAG )
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0) )
		{
			int no = ParseFloatArray( tag.Value().toAscii().constData(), &aec_data.DampUnderVideo, 1 );
			DCT_ASSERT( (no == tag.Size()) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_AEC_AFPS_MAX_GAIN_TAG )
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0) )
		{
			int no = ParseFloatArray( tag.Value().toAscii().constData(), &aec_data.AfpsMaxGain, 1 );
			DCT_ASSERT( (no == tag.Size()) );
		}
		else if ( child.toElement().tagName() == CALIB_SENSOR_AEC_ECM_TAG )
		{
			if ( !parseEntryCell( child.toElement(), tag.Size(), &CalibDb::parseEntryAecEcm ) )
			{
				TRACE(CALIBDB_ERROR, "%s parse error in AEC"
						"section (%s)\n", __func__,
						child.toElement().tagName());
				return ( false );
			}
		}
		else
		{
			TRACE(CALIBDB_ERROR, "%s parse error in AEC section"
					"(unknown tag: %s)\n", __func__,
					child.toElement().tagName());
			//return ( false );
		}

		child = child.nextSibling();
	}

	RESULT result = CamCalibDbAddAecGlobal( m_CalibDbHandle, &aec_data );
	DCT_ASSERT( result == RET_SUCCESS );

	TRACE(CALIBDB_INFO, "%s (exit)\n", __func__);

	return ( true );
}



/******************************************************************************
 * CalibDb::parseEntryAecEcm
 *****************************************************************************/
bool CalibDb::parseEntryAecEcm
(
    const QDomElement& element,
    void *param
)
{
	(void)param;

	TRACE(CALIBDB_INFO, "%s (enter)\n", __func__);

	CamEcmProfile_t EcmProfile;
	MEMSET( &EcmProfile, 0, sizeof(EcmProfile) );
	ListInit( &EcmProfile.ecm_scheme );

	QDomNode child = element.firstChild();
	while ( !child.isNull() )
	{
		XmlTag tag = XmlTag( child.toElement() );

		if ( (child.toElement().tagName() == CALIB_SENSOR_AEC_ECM_NAME_TAG )
				&& (tag.isType( XmlTag::TAG_TYPE_CHAR ))
				&& (tag.Size() > 0) )
		{
			QString value = tag.Value().toUpper();
			strncpy( EcmProfile.name, value.toAscii().constData(), sizeof( EcmProfile.name ) );
		}
		else if ( child.toElement().tagName() == CALIB_SENSOR_AEC_ECM_SCHEMES_TAG )
		{
			if ( !parseEntryCell( child.toElement(), tag.Size(), &CalibDb::parseEntryAecEcmPriorityScheme, &EcmProfile ) )
			{
				TRACE(CALIBDB_ERROR, "%s parse error in ECM"
						"section (%s)\n", __func__,
						child.toElement().tagName());
				return ( false );
			}
		}
		else
		{
			TRACE(CALIBDB_ERROR, "%s parse error in ECM section"
					"(unknown tag: %s)\n", __func__,
					child.toElement().tagName());
			return ( false );
		}

		child = child.nextSibling();
	}

	RESULT result = CamCalibDbAddEcmProfile( m_CalibDbHandle, &EcmProfile );
	DCT_ASSERT( result == RET_SUCCESS );

	// free linked ecm_schemes
	List *l = ListRemoveHead( &EcmProfile.ecm_scheme );
	while ( l )
	{
		free( l );
		l = ListRemoveHead( l );
	}

	TRACE(CALIBDB_INFO, "%s (exit)\n", __func__);

	return ( true );
}



/******************************************************************************
 * CalibDb::parseEntryAecEcmPriorityScheme
 *****************************************************************************/
bool CalibDb::parseEntryAecEcmPriorityScheme
(
    const QDomElement& element,
    void *param
)
{
	CamEcmProfile_t *pEcmProfile = (CamEcmProfile_t *)param;

	TRACE(CALIBDB_INFO, "%s (enter)\n", __func__);

	CamEcmScheme_t *pEcmScheme = (CamEcmScheme_t*) malloc( sizeof(CamEcmScheme_t) );
	if ( !pEcmScheme )
	{
		return false;
	}
	MEMSET( pEcmScheme, 0, sizeof(*pEcmScheme) );

	QDomNode child = element.firstChild();
	while ( !child.isNull() )
	{
		XmlTag tag = XmlTag( child.toElement() );

		if ( (child.toElement().tagName() == CALIB_SENSOR_AEC_ECM_SCHEME_NAME_TAG )
				&& (tag.isType( XmlTag::TAG_TYPE_CHAR ))
				&& (tag.Size() > 0) )
		{
			QString value = tag.Value().toUpper();
			strncpy( pEcmScheme->name, value.toAscii().constData(), sizeof( pEcmScheme->name ) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_AEC_ECM_SCHEME_OFFSETT0FAC_TAG )
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0) )
		{
			int no = ParseFloatArray( tag.Value().toAscii().constData(), &pEcmScheme->OffsetT0Fac, 1 );
			DCT_ASSERT( (no == tag.Size()) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_AEC_ECM_SCHEME_SLOPEA0_TAG )
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0) )
		{
			int no = ParseFloatArray( tag.Value().toAscii().constData(), &pEcmScheme->SlopeA0, 1 );
			DCT_ASSERT( (no == tag.Size()) );
		}
		else
		{
			TRACE(CALIBDB_ERROR, "%s parse error in ECM section"
					"(unknown tag: %s)\n", __func__,
					child.toElement().tagName());
			free(pEcmScheme);
			pEcmScheme = NULL;

			//return ( false );
		}

		child = child.nextSibling();
	}

	if ( pEcmScheme )
	{
		ListPrepareItem( pEcmScheme );
		ListAddTail( &pEcmProfile->ecm_scheme, pEcmScheme );
	}

	TRACE(CALIBDB_INFO, "%s (exit)\n", __func__);

	return ( true );
}



/******************************************************************************
 * CalibDb::parseEntryAwb
 *****************************************************************************/
bool CalibDb::parseEntryAwb
(
    const QDomElement   &element,
    void                *param
)
{
	(void)param;

	TRACE(CALIBDB_INFO, "%s (enter)\n", __func__);

	QDomNode child = element.firstChild();
	while ( !child.isNull() )
	{
		XmlTag tag = XmlTag( child.toElement() );

		if ( child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_TAG )
		{
			if ( !parseEntryCell( child.toElement(), tag.Size(), &CalibDb::parseEntryAwbGlobals ) )
			{
				TRACE(CALIBDB_ERROR, "%s parse error in AWB"
						"globals (%s)\n", __func__,
						child.toElement().tagName());
				return ( false );
			}
		}
		else if ( child.toElement().tagName() == CALIB_SENSOR_AWB_ILLUMINATION_TAG )
		{
			if ( !parseEntryCell( child.toElement(), tag.Size(), &CalibDb::parseEntryAwbIllumination ) )
			{
				TRACE(CALIBDB_ERROR, "%s parse error in AWB"
						"illumination (%s)\n", __func__,
						child.toElement().tagName());
				return ( false );
			}
		}
		else
		{
			TRACE(CALIBDB_ERROR, "%s parse error in AWB section"
					"(unknown tag: %s)\n", __func__,
					child.toElement().tagName());
			//return ( false );
		}

		child = child.nextSibling();
	}

	TRACE(CALIBDB_INFO, "%s (exit)\n", __func__);

	return ( true );
}


/******************************************************************************
 * CalibDb::parseEntryAwbGlobals
 *****************************************************************************/
bool CalibDb::parseEntryAwbGlobals
(
    const QDomElement   &element,
    void                *param
)
{
	(void)param;

	TRACE(CALIBDB_INFO, "%s (enter)\n", __func__);

	CamCalibAwbGlobal_t awb_data;

	/* CamAwbClipParm_t */
	float *pRg1 		= NULL; int nRg1		= 0;
	float *pMaxDist1	= NULL; int nMaxDist1	= 0;
	float *pRg2 		= NULL; int nRg2		= 0;
	float *pMaxDist2	= NULL; int nMaxDist2	= 0;

	/* CamAwbGlobalFadeParm_t */
	float *pGlobalFade1 		= NULL; int nGlobalFade1		 = 0;
	float *pGlobalGainDistance1 = NULL; int nGlobalGainDistance1 = 0;
	float *pGlobalFade2 		= NULL; int nGlobalFade2		 = 0;
	float *pGlobalGainDistance2 = NULL; int nGlobalGainDistance2 = 0;

	/* CamAwbFade2Parm_t */
	float *pFade				= NULL; int nFade				 = 0;
	float *pCbMinRegionMax		= NULL; int nCbMinRegionMax 	 = 0;
	float *pCrMinRegionMax		= NULL; int nCrMinRegionMax 	 = 0;
	float *pMaxCSumRegionMax	= NULL; int nMaxCSumRegionMax	 = 0;
	float *pCbMinRegionMin		= NULL; int nCbMinRegionMin 	 = 0;
	float *pCrMinRegionMin		= NULL; int nCrMinRegionMin 	 = 0;
	float *pMaxCSumRegionMin	= NULL; int nMaxCSumRegionMin	 = 0;

	QDomNode child = element.firstChild();
	while ( !child.isNull() )
	{
		XmlTag tag = XmlTag( child.toElement() );
		QString value = tag.Value();

		if ( (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_NAME_TAG)
				&& (tag.isType( XmlTag::TAG_TYPE_CHAR ))
				&& (tag.Size() > 0) )
		{
			strncpy( awb_data.name, value.toAscii().constData(), sizeof( awb_data.name ) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_RESOLUTION_TAG)
					&& (tag.isType( XmlTag::TAG_TYPE_CHAR ))
					&& (tag.Size() > 0) )
		{
			strncpy( awb_data.resolution, value.toAscii().constData(), sizeof( awb_data.resolution ) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_SENSOR_FILENAME_TAG)
					&& (tag.isType( XmlTag::TAG_TYPE_CHAR ))
					&& (tag.Size() > 0) )
		{
			// do nothing
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_SVDMEANVALUE_TAG)
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0) )

		{
			int i = ( sizeof(awb_data.SVDMeanValue) / sizeof(awb_data.SVDMeanValue.fCoeff[0]) );
			int no = ParseFloatArray( value.toAscii().constData(), awb_data.SVDMeanValue.fCoeff, i );

			DCT_ASSERT( (no == tag.Size()) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_PCAMATRIX_TAG)
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0) )
		{
			int i = ( sizeof(awb_data.PCAMatrix) / sizeof(awb_data.PCAMatrix.fCoeff[0]) );
			int no = ParseFloatArray( tag.Value().toAscii().constData(), awb_data.PCAMatrix.fCoeff, i );

			DCT_ASSERT( (no == tag.Size()) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_CENTERLINE_TAG)
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0) )
		{
			int i = ( sizeof(awb_data.CenterLine) / sizeof(awb_data.CenterLine.f_N0_Rg) );
			int no = ParseFloatArray( tag.Value().toAscii().constData(), &awb_data.CenterLine.f_N0_Rg, i );

			DCT_ASSERT( (no == tag.Size()) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_KFACTOR_TAG)
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0) )
		{
			int i = ( sizeof(awb_data.KFactor) / sizeof(awb_data.KFactor.fCoeff[0]) );
			int no = ParseFloatArray( tag.Value().toAscii().constData(), awb_data.KFactor.fCoeff, i );

			DCT_ASSERT( (no == tag.Size()) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_RG1_TAG)
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0)
					&& (NULL == pRg1) )
		{
			nRg1 = tag.Size();
			pRg1 = (float *)malloc( sizeof(float) * nRg1 );

			int no = ParseFloatArray( tag.Value().toAscii().constData(), pRg1, nRg1 );
			DCT_ASSERT( (no == nRg1) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_MAXDIST1_TAG)
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0)
					&& (NULL == pMaxDist1) )
		{
			nMaxDist1 = tag.Size();
			pMaxDist1 = (float *)malloc( sizeof(float) * nMaxDist1 );

			int no = ParseFloatArray( tag.Value().toAscii().constData(), pMaxDist1, nMaxDist1 );
			DCT_ASSERT( (no == nRg1) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_RG2_TAG)
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0)
					&& (NULL == pRg2) )
		{
			nRg2 = tag.Size();
			pRg2 = (float *)malloc( sizeof(float) * nRg2 );

			int no = ParseFloatArray( tag.Value().toAscii().constData(), pRg2, nRg2 );
			DCT_ASSERT( (no == nRg2) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_MAXDIST2_TAG)
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0)
					&& (NULL == pMaxDist2) )
		{
			nMaxDist2 = tag.Size();
			pMaxDist2 = (float *)malloc( sizeof(float) * nMaxDist2 );

			int no = ParseFloatArray( tag.Value().toAscii().constData(), pMaxDist2, nMaxDist2 );
			DCT_ASSERT( (no == nMaxDist2) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_GLOBALFADE1_TAG)
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0)
					&& (NULL == pGlobalFade1) )
		{
			nGlobalFade1 = tag.Size();
			pGlobalFade1 = (float *)malloc( sizeof(float) * nGlobalFade1 );

			int no = ParseFloatArray( tag.Value().toAscii().constData(), pGlobalFade1, nGlobalFade1 );
			DCT_ASSERT( (no == nGlobalFade1) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_GLOBALGAINDIST1_TAG)
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0)
					&& (NULL == pGlobalGainDistance1) )
		{
			nGlobalGainDistance1 = tag.Size();
			pGlobalGainDistance1 = (float *)malloc( sizeof(float) * nGlobalGainDistance1 );

			int no = ParseFloatArray( tag.Value().toAscii().constData(), pGlobalGainDistance1, nGlobalGainDistance1 );
			DCT_ASSERT( (no == nGlobalGainDistance1) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_GLOBALFADE2_TAG)
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0)
					&& (NULL == pGlobalFade2) )
		{
			nGlobalFade2 = tag.Size();
			pGlobalFade2 = (float *)malloc( sizeof(float) * nGlobalFade2 );

			int no = ParseFloatArray( tag.Value().toAscii().constData(), pGlobalFade2, nGlobalFade2 );
			DCT_ASSERT( (no == nGlobalFade2) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_GLOBALGAINDIST2_TAG)
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0)
					&& (NULL == pGlobalGainDistance2) )
		{
			nGlobalGainDistance2 = tag.Size();
			pGlobalGainDistance2 = (float *)malloc( sizeof(float) * nGlobalGainDistance2 );

			int no = ParseFloatArray( tag.Value().toAscii().constData(), pGlobalGainDistance2, nGlobalGainDistance2 );
			DCT_ASSERT( (no == nGlobalGainDistance2) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_FADE2_TAG)
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0)
					&& (NULL == pFade) )
		{
			nFade = tag.Size();
			pFade = (float *)malloc( sizeof(float) * nFade );

			int no = ParseFloatArray( tag.Value().toAscii().constData(), pFade, nFade );
			DCT_ASSERT( (no == nFade) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_CB_MIN_REGIONMAX_TAG)
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0)
					&& (NULL == pCbMinRegionMax) )
		{
			nCbMinRegionMax = tag.Size();
			pCbMinRegionMax = (float *)malloc( sizeof(float) * nCbMinRegionMax );

			int no = ParseFloatArray( tag.Value().toAscii().constData(), pCbMinRegionMax, nCbMinRegionMax );
			DCT_ASSERT( (no == nCbMinRegionMax) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_CR_MIN_REGIONMAX_TAG)
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0)
					&& (NULL == pCrMinRegionMax) )
		{
			nCrMinRegionMax = tag.Size();
			pCrMinRegionMax = (float *)malloc( sizeof(float) * nCrMinRegionMax );

			int no = ParseFloatArray( tag.Value().toAscii().constData(), pCrMinRegionMax, nCrMinRegionMax );
			DCT_ASSERT( (no == nCrMinRegionMax) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_MAX_CSUM_REGIONMAX_TAG)
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0)
					&& (NULL == pMaxCSumRegionMax) )
		{
			nMaxCSumRegionMax = tag.Size();
			pMaxCSumRegionMax = (float *)malloc( sizeof(float) * nMaxCSumRegionMax );

			int no = ParseFloatArray( tag.Value().toAscii().constData(), pMaxCSumRegionMax, nMaxCSumRegionMax );
			DCT_ASSERT( (no == nMaxCSumRegionMax) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_CB_MIN_REGIONMIN_TAG)
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0)
					&& (NULL == pCbMinRegionMin) )
		{
			nCbMinRegionMin = tag.Size();
			pCbMinRegionMin = (float *)malloc( sizeof(float) * nCbMinRegionMin );

			int no = ParseFloatArray( tag.Value().toAscii().constData(), pCbMinRegionMin, nCbMinRegionMin );
			DCT_ASSERT( (no == nCbMinRegionMin) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_CR_MIN_REGIONMIN_TAG)
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0)
					&& (NULL == pCrMinRegionMin) )
		{
			nCrMinRegionMin = tag.Size();
			pCrMinRegionMin = (float *)malloc( sizeof(float) * nCrMinRegionMin );

			int no = ParseFloatArray( tag.Value().toAscii().constData(), pCrMinRegionMin, nCrMinRegionMin );
			DCT_ASSERT( (no == nCrMinRegionMin) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_MAX_CSUM_REGIONMIN_TAG)
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0)
					&& (NULL == pMaxCSumRegionMin) )
		{
			nMaxCSumRegionMin = tag.Size();
			pMaxCSumRegionMin = (float *)malloc( sizeof(float) * nMaxCSumRegionMin );

			int no = ParseFloatArray( tag.Value().toAscii().constData(), pMaxCSumRegionMin, nMaxCSumRegionMin );
			DCT_ASSERT( (no == nMaxCSumRegionMin) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_RGPROJ_INDOOR_MIN_TAG)
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0) )
		{
			int no = ParseFloatArray( tag.Value().toAscii().constData(), &awb_data.fRgProjIndoorMin, 1 );
			DCT_ASSERT( (no == tag.Size()) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_RGPROJ_OUTDOOR_MIN_TAG )
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0) )
		{
			int no = ParseFloatArray( tag.Value().toAscii().constData(), &awb_data.fRgProjOutdoorMin, 1 );
			DCT_ASSERT( (no == tag.Size()) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_RGPROJ_MAX_TAG )
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0) )
		{
			int no = ParseFloatArray( tag.Value().toAscii().constData(), &awb_data.fRgProjMax, 1 );
			DCT_ASSERT( (no == tag.Size()) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_RGPROJ_MAX_SKY_TAG )
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0) )
		{
			int no = ParseFloatArray( tag.Value().toAscii().constData(), &awb_data.fRgProjMaxSky, 1 );
			DCT_ASSERT( (no == tag.Size()) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_CLIP_OUTDOOR )
					&& (tag.isType( XmlTag::TAG_TYPE_CHAR ))
					&& (tag.Size() > 0) )
		{
			QString value = tag.Value().toUpper();
			strncpy( awb_data.outdoor_clipping_profile,
						value.toAscii().constData(), sizeof( awb_data.outdoor_clipping_profile ) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_REGION_SIZE )
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0) )
		{
			int no = ParseFloatArray( tag.Value().toAscii().constData(), &awb_data.fRegionSize, 1 );
			DCT_ASSERT( (no == tag.Size()) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_REGION_SIZE_INC )
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0) )
		{
			int no = ParseFloatArray( tag.Value().toAscii().constData(), &awb_data.fRegionSizeInc, 1 );
			DCT_ASSERT( (no == tag.Size()) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_REGION_SIZE_DEC )
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0) )
		{
			int no = ParseFloatArray( tag.Value().toAscii().constData(), &awb_data.fRegionSizeDec, 1 );
			DCT_ASSERT( (no == tag.Size()) );
		}
		else if ( child.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_IIR )
		{
			QDomNode subchild = child.toElement().firstChild();
			while ( !subchild.isNull() )
			{
				XmlTag tag = XmlTag( subchild.toElement() );
				if ( (subchild.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_IIR_DAMP_COEF_ADD)
						&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
						&& (tag.Size() > 0) )
				{
					int no = ParseFloatArray( tag.Value().toAscii().constData(), &awb_data.IIR.fIIRDampCoefAdd, 1 );
					DCT_ASSERT( (no == tag.Size()) );
				}
				else if ( (subchild.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_IIR_DAMP_COEF_SUB)
						   && (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
						   && (tag.Size() > 0) )
				{
					int no = ParseFloatArray( tag.Value().toAscii().constData(), &awb_data.IIR.fIIRDampCoefSub, 1 );
					DCT_ASSERT( (no == tag.Size()) );
				}
				else if ( (subchild.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_IIR_DAMP_FILTER_THRESHOLD)
						   && (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
						   && (tag.Size() > 0) )
				{
					int no = ParseFloatArray( tag.Value().toAscii().constData(), &awb_data.IIR.fIIRDampFilterThreshold, 1 );
					DCT_ASSERT( (no == tag.Size()) );
				}
				else if ( (subchild.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_IIR_DAMPING_COEF_MIN)
						   && (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
						   && (tag.Size() > 0) )
				{
					int no = ParseFloatArray( tag.Value().toAscii().constData(), &awb_data.IIR.fIIRDampingCoefMin, 1 );
					DCT_ASSERT( (no == tag.Size()) );
				}
				else if ( (subchild.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_IIR_DAMPING_COEF_MAX)
						   && (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
						   && (tag.Size() > 0) )
				{
					int no = ParseFloatArray( tag.Value().toAscii().constData(), &awb_data.IIR.fIIRDampingCoefMax, 1 );
					DCT_ASSERT( (no == tag.Size()) );
				}
				else if ( (subchild.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_IIR_DAMPING_COEF_INIT)
						   && (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
						   && (tag.Size() > 0) )
				{
					int no = ParseFloatArray( tag.Value().toAscii().constData(), &awb_data.IIR.fIIRDampingCoefInit, 1 );
					DCT_ASSERT( (no == tag.Size()) );
				}
				else if ( (subchild.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_IIR_EXP_PRIOR_FILTER_SIZE_MAX)
						   && (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
						   && (tag.Size() > 0) )
				{
					int no = ParseUshortArray( tag.Value().toAscii().constData(), &awb_data.IIR.IIRFilterSize, 1 );
					DCT_ASSERT( (no == tag.Size()) );
				}
				else if ( (subchild.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_IIR_EXP_PRIOR_FILTER_SIZE_MIN)
							&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
							&& (tag.Size() > 0) )
				{
				}
				else if ( (subchild.toElement().tagName() == CALIB_SENSOR_AWB_GLOBALS_IIR_EXP_PRIOR_MIDDLE)
						   && (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
						   && (tag.Size() > 0) )
				{
					int no = ParseFloatArray( tag.Value().toAscii().constData(), &awb_data.IIR.fIIRFilterInitValue, 1 );
					DCT_ASSERT( (no == tag.Size()) );
				}
				else
				{
					TRACE(CALIBDB_ERROR, "%s parse error in"
						"AWB GLOBALS - IIR section"
						"(unknown tag:%s)\n", __func__,
						subchild.toElement().tagName());
					return ( false );
				}

				subchild = subchild.nextSibling();
			}
		}
		else
		{
			TRACE(CALIBDB_ERROR, "%s parse error in AWB section"
					"(unknown tag:%s)\n", __func__,
					child.toElement().tagName());
			return ( false );
		}

		child = child.nextSibling();
	}

	DCT_ASSERT( (nRg1 == nMaxDist1) );
	DCT_ASSERT( (nRg2 == nMaxDist2) );

	DCT_ASSERT( (nGlobalFade1 == nGlobalGainDistance1) );
	DCT_ASSERT( (nGlobalFade2 == nGlobalGainDistance2) );

	DCT_ASSERT( (nFade == nCbMinRegionMax) );
	DCT_ASSERT( (nFade == nCrMinRegionMax) );
	DCT_ASSERT( (nFade == nMaxCSumRegionMax) );
	DCT_ASSERT( (nFade == nCbMinRegionMin) );
	DCT_ASSERT( (nFade == nCrMinRegionMin) );
	DCT_ASSERT( (nFade == nMaxCSumRegionMin) );

	/* CamAwbClipParm_t */
	awb_data.AwbClipParam.ArraySize1	= nRg1;
	awb_data.AwbClipParam.pRg1		= pRg1;
	awb_data.AwbClipParam.pMaxDist1 	= pMaxDist1;
	awb_data.AwbClipParam.ArraySize2	= nRg2;
	awb_data.AwbClipParam.pRg2		= pRg2;
	awb_data.AwbClipParam.pMaxDist2 	= pMaxDist2;

	/* CamAwbGlobalFadeParm_t */
	awb_data.AwbGlobalFadeParm.ArraySize1		= nGlobalFade1;
	awb_data.AwbGlobalFadeParm.pGlobalFade1 	= pGlobalFade1;
	awb_data.AwbGlobalFadeParm.pGlobalGainDistance1 = pGlobalGainDistance1;
	awb_data.AwbGlobalFadeParm.ArraySize2		= nGlobalFade2;
	awb_data.AwbGlobalFadeParm.pGlobalFade2 	= pGlobalFade2;
	awb_data.AwbGlobalFadeParm.pGlobalGainDistance2 = pGlobalGainDistance2;

	/* CamAwbFade2Parm_t */
	awb_data.AwbFade2Parm.ArraySize 	= nFade;
	awb_data.AwbFade2Parm.pFade 		= pFade;
	awb_data.AwbFade2Parm.pCbMinRegionMax	= pCbMinRegionMax;
	awb_data.AwbFade2Parm.pCrMinRegionMax	= pCrMinRegionMax;
	awb_data.AwbFade2Parm.pMaxCSumRegionMax = pMaxCSumRegionMax;
	awb_data.AwbFade2Parm.pCbMinRegionMin	= pCbMinRegionMin;
	awb_data.AwbFade2Parm.pCrMinRegionMin	= pCrMinRegionMin;
	awb_data.AwbFade2Parm.pMaxCSumRegionMin = pMaxCSumRegionMin;

	RESULT result = CamCalibDbAddAwbGlobal( m_CalibDbHandle, &awb_data );
	DCT_ASSERT( result == RET_SUCCESS );

	/* cleanup */
	free( pRg1 );
	free( pMaxDist1 );
	free( pRg2 );
	free( pMaxDist2 );

	free( pGlobalFade1 );
	free( pGlobalGainDistance1 );
	free( pGlobalFade2 );
	free( pGlobalGainDistance2 );

	free( pFade );
	free( pCbMinRegionMax );
	free( pCrMinRegionMax );
	free( pMaxCSumRegionMax );
	free( pCbMinRegionMin );
	free( pCrMinRegionMin );
	free( pMaxCSumRegionMin );

	TRACE(CALIBDB_INFO, "%s (exit)\n", __func__);

	return ( true );
}



/******************************************************************************
 * CalibDb::parseEntryAwbIllumination
 *****************************************************************************/
bool CalibDb::parseEntryAwbIllumination
(
    const QDomElement   &element,
    void                *param
)
{
	(void)param;

	TRACE(CALIBDB_INFO, "%s (enter)\n", __func__);

	CamIlluProfile_t illu;
	MEMSET( &illu, 0, sizeof( illu ) );

	QDomNode child = element.firstChild();
	while ( !child.isNull() )
	{
		XmlTag tag = XmlTag( child.toElement() );

		if ( (child.toElement().tagName() == CALIB_SENSOR_AWB_ILLUMINATION_NAME_TAG)
				&& (tag.isType( XmlTag::TAG_TYPE_CHAR ))
				&& (tag.Size() > 0) )
		{
			QString value = tag.Value().toUpper();
			strncpy( illu.name, value.toAscii().constData(), sizeof( illu.name ) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_AWB_ILLUMINATION_DOOR_TYPE_TAG)
					&& (tag.isType( XmlTag::TAG_TYPE_CHAR ))
					&& (tag.Size() > 0) )
		{
			QString value = tag.Value().toUpper();
			if ( value == CALIB_SENSOR_AWB_ILLUMINATION_DOOR_TYPE_INDOOR )
			{
				illu.DoorType = CAM_DOOR_TYPE_INDOOR;
			}
			else if ( value == CALIB_SENSOR_AWB_ILLUMINATION_DOOR_TYPE_OUTDOOR )
			{
				illu.DoorType = CAM_DOOR_TYPE_OUTDOOR;
			}
			else
			{
				TRACE(CALIBDB_ERROR, "%s invalid illumination"
						"doortype(%d)\n", __func__, value);
			}
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_AWB_ILLUMINATION_AWB_TYPE_TAG)
					&& (tag.isType( XmlTag::TAG_TYPE_CHAR ))
					&& (tag.Size() > 0) )
		{
			QString value = tag.Value().toUpper();
			if ( value == CALIB_SENSOR_AWB_ILLUMINATION_AWB_TYPE_MANUAL )
			{
				illu.AwbType = CAM_AWB_TYPE_MANUAL;
			}
			else if ( value == CALIB_SENSOR_AWB_ILLUMINATION_AWB_TYPE_AUTO )
			{
				illu.AwbType = CAM_AWB_TYPE_AUTO;
			}
			else
			{
				TRACE(CALIBDB_ERROR, "%s invalid AWB type(%d)\n", __func__, value);
			}
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_AWB_ILLUMINATION_MANUAL_WB_TAG)
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0) )
		{
			int i = ( sizeof(illu.ComponentGain) / sizeof(illu.ComponentGain.fCoeff[0]) );
			int no = ParseFloatArray( tag.Value().toAscii().constData(), illu.ComponentGain.fCoeff, i );
			DCT_ASSERT( (no == tag.Size()) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_AWB_ILLUMINATION_MANUAL_CC_TAG)
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0) )
		{
			int i = ( sizeof(illu.CrossTalkCoeff) / sizeof(illu.CrossTalkCoeff.fCoeff[0]) );
			int no = ParseFloatArray( tag.Value().toAscii().constData(), illu.CrossTalkCoeff.fCoeff, i );
			DCT_ASSERT( (no == tag.Size()) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_AWB_ILLUMINATION_MANUAL_CTO_TAG)
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0) )
		{
			int i = ( sizeof(illu.CrossTalkOffset) / sizeof(illu.CrossTalkOffset.fCoeff[0]) );
			int no = ParseFloatArray( tag.Value().toAscii().constData(), illu.CrossTalkOffset.fCoeff, i );
			DCT_ASSERT( (no == tag.Size()) );
		}
		else if ( child.toElement().tagName() == CALIB_SENSOR_AWB_ILLUMINATION_GMM_TAG )
		{
			QDomNode subchild = child.toElement().firstChild();
			while ( !subchild.isNull() )
			{
				XmlTag tag = XmlTag( subchild.toElement() );
				if ( (subchild.toElement().tagName() == CALIB_SENSOR_AWB_ILLUMINATION_GMM_GAUSSIAN_MVALUE_TAG)
						&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
						&& (tag.Size() > 0) )
				{
					int i = ( sizeof(illu.GaussMeanValue) / sizeof(illu.GaussMeanValue.fCoeff[0]) );
					int no = ParseFloatArray( tag.Value().toAscii().constData(), illu.GaussMeanValue.fCoeff, i );
					DCT_ASSERT( (no == tag.Size()) );
				}
				else if ( (subchild.toElement().tagName() == CALIB_SENSOR_AWB_ILLUMINATION_GMM_INV_COV_MATRIX_TAG)
							&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
							&& (tag.Size() > 0) )
				{
					int i = ( sizeof(illu.CovarianceMatrix) / sizeof(illu.CovarianceMatrix.fCoeff[0]) );
					int no = ParseFloatArray( tag.Value().toAscii().constData(), illu.CovarianceMatrix.fCoeff, i );
					DCT_ASSERT( (no == tag.Size()) );
				}
				else if ( (subchild.toElement().tagName() == CALIB_SENSOR_AWB_ILLUMINATION_GMM_GAUSSIAN_SFACTOR_TAG)
							&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
							&& (tag.Size() > 0) )
				{
					int i = ( sizeof(illu.GaussFactor) / sizeof(illu.GaussFactor.fCoeff[0]) );
					int no = ParseFloatArray( tag.Value().toAscii().constData(), illu.GaussFactor.fCoeff, i );
					DCT_ASSERT( (no == tag.Size()) );
				}
				else if ( (subchild.toElement().tagName() == CALIB_SENSOR_AWB_ILLUMINATION_GMM_TAU_TAG )
							&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
							&& (tag.Size() > 0) )
				{
					int i = ( sizeof(illu.Threshold) / sizeof(illu.Threshold.fCoeff[0]) );
					int no = ParseFloatArray( tag.Value().toAscii().constData(), illu.Threshold.fCoeff, i );
					DCT_ASSERT( (no == tag.Size()) );
				}
				else
				{
					TRACE(CALIBDB_ERROR, "%s parse error in"
							"AWB gaussian mixture modell section (unknown tag: %s)\n",
							__func__, subchild.toElement().tagName());
					return ( false );
				}

				subchild = subchild.nextSibling();
			}
		}
		else if ( child.toElement().tagName() == CALIB_SENSOR_AWB_ILLUMINATION_SAT_CT_TAG )
		{
			float *afGain	= NULL;
			int n_gains 	= 0;
			float *afSat	= NULL;
			int n_sats		= 0;

			QDomNode subchild = child.toElement().firstChild();
			while ( !subchild.isNull() )
			{
				XmlTag tag = XmlTag( subchild.toElement() );

				if ( (subchild.toElement().tagName() == CALIB_SENSOR_AWB_ILLUMINATION_SAT_CT_GAIN_TAG)
						&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
						&& (tag.Size() > 0) )
				{
					if ( !afGain )
					{
						n_gains = tag.Size();
						afGain	= (float *)malloc( (n_gains * sizeof( float )) );
						MEMSET( afGain, 0, (n_gains * sizeof( float )) );
					}

					int no = ParseFloatArray( tag.Value().toAscii().constData(), afGain, n_gains );
					DCT_ASSERT( (no == n_gains) );
				}
				else if ( (subchild.toElement().tagName() == CALIB_SENSOR_AWB_ILLUMINATION_SAT_CT_SAT_TAG)
							&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
							&& (tag.Size() > 0) )
				{
					if ( !afSat )
					{
						n_sats = tag.Size();
						afSat = (float *)malloc( (n_sats * sizeof( float )) );
						MEMSET( afSat, 0, (n_sats * sizeof( float )) );
					}

					int no = ParseFloatArray( tag.Value().toAscii().constData(), afSat, n_sats );
					DCT_ASSERT( (no == n_sats) );
				}
				else
				{
					TRACE(CALIBDB_ERROR, "%s parse error in AWB saturation curve section (unknown tag:%s)\n",
							__func__, subchild.toElement().tagName());
					return ( false );
				}

				subchild = subchild.nextSibling();
			}

			DCT_ASSERT( (n_gains == n_sats) );
			illu.SaturationCurve.ArraySize		= n_gains;
			illu.SaturationCurve.pSensorGain	= afGain;
			illu.SaturationCurve.pSaturation	= afSat;
		}
		else if ( child.toElement().tagName() == CALIB_SENSOR_AWB_ILLUMINATION_VIG_CT_TAG )
		{
			float *afGain	= NULL;
			int n_gains 	= 0;
			float *afVig	= NULL;
			int n_vigs		= 0;

			QDomNode subchild = child.toElement().firstChild();
			while ( !subchild.isNull() )
			{
				XmlTag tag = XmlTag( subchild.toElement() );
				if ( (subchild.toElement().tagName() == CALIB_SENSOR_AWB_ILLUMINATION_VIG_CT_GAIN_TAG )
						&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
						&& (tag.Size() > 0) )
				{
					if ( !afGain )
					{
						n_gains = tag.Size();
						afGain	= (float *)malloc( (n_gains * sizeof( float )) );
						MEMSET( afGain, 0, (n_gains * sizeof( float )) );
					}

					int no = ParseFloatArray( tag.Value().toAscii().constData(), afGain, n_gains );
					DCT_ASSERT( (no == n_gains) );
				}
				else if ( (subchild.toElement().tagName() == CALIB_SENSOR_AWB_ILLUMINATION_VIG_CT_VIG_TAG )
							&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
							&& (tag.Size() > 0) )
				{
					if ( !afVig )
					{
						n_vigs = tag.Size();
						afVig = (float *)malloc( (n_vigs * sizeof( float )) );
						MEMSET( afVig, 0, (n_vigs * sizeof( float )) );
					}

					int no = ParseFloatArray( tag.Value().toAscii().constData(), afVig, n_vigs);
					DCT_ASSERT( (no == n_vigs) );
				}
				else
				{
					TRACE(CALIBDB_ERROR, "%s parse error in AWB vignetting curve section (unknown tag:%s)\n",
							__func__, subchild.toElement().tagName());
					return ( false );
				}

				subchild = subchild.nextSibling();
			}

			DCT_ASSERT( (n_gains == n_vigs) );
			illu.VignettingCurve.ArraySize		= n_gains;
			illu.VignettingCurve.pSensorGain	= afGain;
			illu.VignettingCurve.pVignetting	= afVig;
		}
		else if ( child.toElement().tagName() == CALIB_SENSOR_AWB_ILLUMINATION_ALSC_TAG )
		{
			if ( !parseEntryCell( child.toElement(), tag.Size(), &CalibDb::parseEntryAwbIlluminationAlsc, &illu  ) )
			{
				TRACE(CALIBDB_ERROR, "%s parse error in AWB ALSC(%s)\n",
						__func__, child.toElement().tagName());
				return ( false );
			}
		}
		else if ( child.toElement().tagName() == CALIB_SENSOR_AWB_ILLUMINATION_ACC_TAG )
		{
			QDomNode subchild = child.toElement().firstChild();
			while ( !subchild.isNull() )
			{
				XmlTag tag = XmlTag( subchild.toElement() );
				if ( (subchild.toElement().tagName() == CALIB_SENSOR_AWB_ILLUMINATION_ACC_CC_PROFILE_LIST_TAG)
						&& (tag.isType( XmlTag::TAG_TYPE_CHAR ))
						&& (tag.Size() > 0) )
				{
					QString value = tag.Value().toUpper();
					int no = ParseCcProfileArray( value.toAscii().constData(), illu.cc_profiles, CAM_NO_CC_PROFILES );
					DCT_ASSERT( (no <= CAM_NO_CC_PROFILES) );
					illu.cc_no = no;
				}
				else
				{
					TRACE(CALIBDB_ERROR, "%s parse error in AWB ACC (%s)\n",
							__func__, subchild.toElement().tagName());
					return ( false );
				}

				subchild = subchild.nextSibling();
			}
		}
		else
		{
			TRACE(CALIBDB_ERROR, "%s parse error in AWB illumination section (unknown tag:%s)\n",
					__func__, child.toElement().tagName());
			return ( false );
		}

		child = child.nextSibling();
	}

	RESULT result = CamCalibDbAddIllumination( m_CalibDbHandle, &illu );
	DCT_ASSERT( result == RET_SUCCESS );

	/* cleanup */
	free( illu.SaturationCurve.pSensorGain );
	free( illu.SaturationCurve.pSaturation );
	free( illu.VignettingCurve.pSensorGain );
	free( illu.VignettingCurve.pVignetting );

	TRACE(CALIBDB_INFO, "%s (exit)\n", __func__);

	return ( true );
}



/******************************************************************************
 * CalibDb::parseEntryAwbIlluminationAlsc
 *****************************************************************************/
bool CalibDb::parseEntryAwbIlluminationAlsc
(
    const QDomElement   &element,
    void                *param
)
{
	TRACE(CALIBDB_INFO, "%s (enter)\n", __func__);

	if ( !param )
	{
		return ( false );
	}

	CamIlluProfile_t *pIllu = ( CamIlluProfile_t * )param;

	QString lsc_profiles;
	int resIdx = -1;

	QDomNode child = element.firstChild();
	while ( !child.isNull() )
	{
		XmlTag tag = XmlTag( child.toElement() );
		if ( child.toElement().tagName() == CALIB_SENSOR_AWB_ILLUMINATION_ALSC_RES_LSC_PROFILE_LIST_TAG )
		{
			lsc_profiles = tag.Value().toUpper();
		}
		else if ( child.toElement().tagName() == CALIB_SENSOR_AWB_ILLUMINATION_ALSC_RES_TAG )
		{
			QString value = tag.Value();
			RESULT result = CamCalibDbGetResolutionIdxByName( m_CalibDbHandle, value.toAscii().constData(), &resIdx);
			DCT_ASSERT( result == RET_SUCCESS );
		}
		else
		{
			TRACE(CALIBDB_ERROR, "%s unknown ALSC tag:%s\n", __func__, child.toElement().tagName());
			return ( false );
		}

		child = child.nextSibling();
	}

	DCT_ASSERT( resIdx != -1 );

	int no = ParseLscProfileArray( lsc_profiles.toAscii().constData(), pIllu->lsc_profiles[resIdx], CAM_NO_LSC_PROFILES );
	DCT_ASSERT( (no <= CAM_NO_LSC_PROFILES) );
	pIllu->lsc_no[resIdx] = no;

	pIllu->lsc_res_no++;

	TRACE(CALIBDB_INFO, "%s (exit)\n", __func__);

	return ( true );
}



/******************************************************************************
 * CalibDb::parseEntryAwbIlluminationAcc
 *****************************************************************************/
bool CalibDb::parseEntryAwbIlluminationAcc
(
    const QDomElement   &element,
    void                *param
)
{
	(void)param;

	TRACE(CALIBDB_INFO, "%s (enter)\n", __func__);

	QDomNode child = element.firstChild();
	while ( !child.isNull() )
	{
		XmlTag tag = XmlTag( child.toElement() );
		if ( child.toElement().tagName() == CALIB_SENSOR_AWB_ILLUMINATION_ACC_CC_PROFILE_LIST_TAG )
		{
		}
		else
		{
			TRACE(CALIBDB_ERROR, "%s unknown ACC tag:%s\n", __func__, child.toElement().tagName());
			return ( false );
		}

		child = child.nextSibling();
	}

	TRACE(CALIBDB_INFO, "%s (exit)\n", __func__);

	return ( true );
}



/******************************************************************************
 * CalibDb::parseEntryLsc
 *****************************************************************************/
bool CalibDb::parseEntryLsc
(
    const QDomElement   &element,
    void                *param
)
{
	TRACE(CALIBDB_INFO, "%s (enter)\n", __func__);

	CamLscProfile_t lsc_profile;
	MEMSET( &lsc_profile, 0, sizeof( lsc_profile ) );

	QDomNode child = element.firstChild();
	while ( !child.isNull() )
	{
		XmlTag tag = XmlTag( child.toElement() );
		if ( (child.toElement().tagName() == CALIB_SENSOR_LSC_PROFILE_NAME_TAG )
				&& (tag.isType( XmlTag::TAG_TYPE_CHAR ))
				&& (tag.Size() > 0) )
		{
			QString value = tag.Value().toUpper();
			strncpy( lsc_profile.name, value.toAscii().constData(), sizeof( lsc_profile.name ) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_LSC_PROFILE_RESOLUTION_TAG )
					&& (tag.isType( XmlTag::TAG_TYPE_CHAR ))
					&& (tag.Size() > 0) )
		{
			QString value = tag.Value();
			strncpy( lsc_profile.resolution, value.toAscii().constData(), sizeof( lsc_profile.resolution ) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_LSC_PROFILE_ILLUMINATION_TAG )
					&& (tag.isType( XmlTag::TAG_TYPE_CHAR ))
					&& (tag.Size() > 0) )
		{
			QString value = tag.Value().toUpper();
			strncpy( lsc_profile.illumination, value.toAscii().constData(), sizeof( lsc_profile.illumination ) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_LSC_PROFILE_LSC_SECTORS_TAG )
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0) )
		{
			int no = ParseUshortArray( tag.Value().toAscii().constData(), &lsc_profile.LscSectors, 1 );
			DCT_ASSERT( (no == 1) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_LSC_PROFILE_LSC_NO_TAG )
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0) )
		{
			int no = ParseUshortArray( tag.Value().toAscii().constData(), &lsc_profile.LscNo, 1 );
			DCT_ASSERT( (no == 1) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_LSC_PROFILE_LSC_XO_TAG )
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0) )
		{
			int no = ParseUshortArray( tag.Value().toAscii().constData(), &lsc_profile.LscXo, 1 );
			DCT_ASSERT( (no == 1) );
		}
		else if ( child.toElement().tagName() == CALIB_SENSOR_LSC_PROFILE_LSC_YO_TAG )
		{
			int no = ParseUshortArray( tag.Value().toAscii().constData(), &lsc_profile.LscYo, 1 );
			DCT_ASSERT( (no == 1) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_LSC_PROFILE_LSC_SECTOR_SIZE_X_TAG )
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0) )
		{
			int i = ( sizeof(lsc_profile.LscXSizeTbl) / sizeof(lsc_profile.LscXSizeTbl[0]) );
			int no = ParseUshortArray( tag.Value().toAscii().constData(), lsc_profile.LscXSizeTbl, i );
			DCT_ASSERT( (no == tag.Size()) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_LSC_PROFILE_LSC_SECTOR_SIZE_Y_TAG )
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0) )
		{
			int i = ( sizeof(lsc_profile.LscYSizeTbl) / sizeof(lsc_profile.LscYSizeTbl[0]) );
			int no = ParseUshortArray( tag.Value().toAscii().constData(), lsc_profile.LscYSizeTbl, i );
			DCT_ASSERT( (no == tag.Size()) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_LSC_PROFILE_LSC_VIGNETTING_TAG )
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0) )
		{
			int no = ParseFloatArray( tag.Value().toAscii().constData(), (float *)(&lsc_profile.vignetting), 1 );
			DCT_ASSERT( (no == 1) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_LSC_PROFILE_LSC_SAMPLES_RED_TAG )
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0) )
		{
			int i = ( sizeof(lsc_profile.LscMatrix[CAM_4CH_COLOR_COMPONENT_RED])
							/ sizeof(lsc_profile.LscMatrix[CAM_4CH_COLOR_COMPONENT_RED].uCoeff[0]) );
			int no = ParseUshortArray( tag.Value().toAscii().constData(),
							(lsc_profile.LscMatrix[CAM_4CH_COLOR_COMPONENT_RED].uCoeff), i );
			DCT_ASSERT( (no == tag.Size()) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_LSC_PROFILE_LSC_SAMPLES_GREENR_TAG )
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0) )
		{
			int i = ( sizeof(lsc_profile.LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENR])
							/ sizeof(lsc_profile.LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENR].uCoeff[0]) );
			int no = ParseUshortArray( tag.Value().toAscii().constData(),
							lsc_profile.LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENR].uCoeff, i );
			DCT_ASSERT( (no == tag.Size()) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_LSC_PROFILE_LSC_SAMPLES_GREENB_TAG )
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0) )
		{
			int i = ( sizeof(lsc_profile.LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENB])
							/ sizeof(lsc_profile.LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENB].uCoeff[0]) );
			int no = ParseUshortArray( tag.Value().toAscii().constData(),
							(uint16_t *)(lsc_profile.LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENB].uCoeff), i );
			DCT_ASSERT( (no == tag.Size()) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_LSC_PROFILE_LSC_SAMPLES_BLUE_TAG )
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0) )
		{
			int i = ( sizeof(lsc_profile.LscMatrix[CAM_4CH_COLOR_COMPONENT_BLUE])
							/ sizeof(lsc_profile.LscMatrix[CAM_4CH_COLOR_COMPONENT_BLUE].uCoeff[0]) );
			int no = ParseUshortArray( tag.Value().toAscii().constData(),
							(uint16_t *)(lsc_profile.LscMatrix[CAM_4CH_COLOR_COMPONENT_BLUE].uCoeff), i );
			DCT_ASSERT( (no == tag.Size()) );
		}
		else
		{
			TRACE(CALIBDB_ERROR, "%s parse error in LSC section (unknown tag:%s)\n",
					__func__, child.toElement().tagName());
			return ( false );
		}

		child = child.nextSibling();
	}

	RESULT result = CamCalibDbAddLscProfile( m_CalibDbHandle, &lsc_profile );
	DCT_ASSERT( result == RET_SUCCESS );

	TRACE(CALIBDB_INFO, "%s (exit)\n", __func__);

	return ( true );
}



/******************************************************************************
 * CalibDb::parseEntryCc
 *****************************************************************************/
bool CalibDb::parseEntryCc
(
	const QDomElement	&element,
	void				*param
)
{
	(void)param;

	TRACE(CALIBDB_INFO, "%s (enter)\n", __func__);

	CamCcProfile_t cc_profile;
	MEMSET( &cc_profile, 0, sizeof( cc_profile ) );

	QDomNode child = element.firstChild();
	while ( !child.isNull() )
	{
		XmlTag tag = XmlTag( child.toElement() );
		if ( (child.toElement().tagName() == CALIB_SENSOR_CC_PROFILE_NAME_TAG)
				&& (tag.isType( XmlTag::TAG_TYPE_CHAR ))
				&& (tag.Size() > 0) )
		{
			QString value = tag.Value().toUpper();
			strncpy( cc_profile.name, value.toAscii().constData(), sizeof( cc_profile.name ) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_CC_PROFILE_SATURATION_TAG)
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0) )
		{
			int no = ParseFloatArray( tag.Value().toAscii().constData(), &cc_profile.saturation, 1 );
			DCT_ASSERT( (no == 1) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_CC_PROFILE_CC_MATRIX_TAG )
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0) )
		{
			int i = ( sizeof(cc_profile.CrossTalkCoeff) / sizeof(cc_profile.CrossTalkCoeff.fCoeff[0]) );
			int no = ParseFloatArray( tag.Value().toAscii().constData(), cc_profile.CrossTalkCoeff.fCoeff, i );
			DCT_ASSERT( (no == tag.Size()) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_CC_PROFILE_CC_OFFSETS_TAG )
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0) )
		{
			int i = ( sizeof(cc_profile.CrossTalkOffset) / sizeof(cc_profile.CrossTalkOffset.fCoeff[0]) );
			int no = ParseFloatArray( tag.Value().toAscii().constData(), cc_profile.CrossTalkOffset.fCoeff, i );
			DCT_ASSERT( (no == tag.Size()) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_CC_PROFILE_WB_TAG )
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0) )
		{
			int i = ( sizeof(cc_profile.ComponentGain) / sizeof(cc_profile.ComponentGain.fCoeff[0]) );
			int no = ParseFloatArray( tag.Value().toAscii().constData(), cc_profile.ComponentGain.fCoeff, i );
			DCT_ASSERT( (no == tag.Size()) );
		}
		else
		{
			TRACE(CALIBDB_ERROR, "%s parse error in CC section (unknown tag: %s)\n",
					__func__, child.toElement().tagName());
			return ( false );
		}

		child = child.nextSibling();
	}

	RESULT result = CamCalibDbAddCcProfile( m_CalibDbHandle, &cc_profile );
	DCT_ASSERT( result == RET_SUCCESS );

	TRACE(CALIBDB_INFO, "%s (exit)\n", __func__);

	return ( true );
}



/******************************************************************************
 * CalibDb::parseEntryBls
 *****************************************************************************/
bool CalibDb::parseEntryBls
(
    const QDomElement   &element,
    void                *param
)
{
	(void)param;

	TRACE(CALIBDB_INFO, "%s (enter)\n", __func__);

	CamBlsProfile_t bls_profile;
	MEMSET( &bls_profile, 0, sizeof( bls_profile ) );

	QDomNode child = element.firstChild();
	while ( !child.isNull() )
	{
		XmlTag tag = XmlTag( child.toElement() );
		if ( (child.toElement().tagName() == CALIB_SENSOR_BLS_NAME_TAG)
				&& (tag.isType( XmlTag::TAG_TYPE_CHAR ))
				&& (tag.Size() > 0) )
		{
			QString value = tag.Value().toUpper();
			strncpy( bls_profile.name, value.toAscii().constData(), sizeof( bls_profile.name ) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_BLS_RESOLUTION_TAG )
				&& (tag.isType( XmlTag::TAG_TYPE_CHAR ))
				&& (tag.Size() > 0) )
		{
			QString value = tag.Value();
			strncpy( bls_profile.resolution, value.toAscii().constData(), sizeof( bls_profile.resolution ) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_BLS_DATA_TAG)
				&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
				&& (tag.Size() > 0) )
		{
			int i = ( sizeof(bls_profile.level) / sizeof(bls_profile.level.uCoeff[0]) );
			int no = ParseUshortArray( tag.Value().toAscii().constData(), bls_profile.level.uCoeff, i );
			DCT_ASSERT( (no == tag.Size()) );
		}
		else
		{
			TRACE(CALIBDB_ERROR, "%s parse error in BLS section (unknown tag:%s)\n",
					__func__, child.toElement().tagName());
			return ( false );
		}

		child = child.nextSibling();
	}

	RESULT result = CamCalibDbAddBlsProfile( m_CalibDbHandle, &bls_profile );
	DCT_ASSERT( result == RET_SUCCESS );

	TRACE(CALIBDB_INFO, "%s (exit)\n", __func__);

	return ( true );
}



/******************************************************************************
 * CalibDb::parseEntryCac
 *****************************************************************************/
bool CalibDb::parseEntryCac
(
    const QDomElement   &element,
    void                *param
)
{
	(void)param;

	TRACE(CALIBDB_INFO, "%s (enter)\n", __func__);

	CamCacProfile_t cac_profile;
	MEMSET( &cac_profile, 0, sizeof( cac_profile ) );

	QDomNode child = element.firstChild();
	while ( !child.isNull() )
	{
		XmlTag tag = XmlTag( child.toElement() );
		if ( (child.toElement().tagName() == CALIB_SENSOR_CAC_NAME_TAG )
				&& (tag.isType( XmlTag::TAG_TYPE_CHAR ))
				&& (tag.Size() > 0) )
		{
			QString value = tag.Value().toUpper();
			strncpy( cac_profile.name, value.toAscii().constData(), sizeof( cac_profile.name ) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_CAC_RESOLUTION_TAG )
				&& (tag.isType( XmlTag::TAG_TYPE_CHAR ))
				&& (tag.Size() > 0) )
		{
			QString value = tag.Value();
			strncpy( cac_profile.resolution, value.toAscii().constData(), sizeof( cac_profile.resolution ) );
		}
		else if ( (child.toElement().tagName() == CALIB_SESNOR_CAC_X_NORMSHIFT_TAG )
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0) )
		{
			int no = ParseByteArray( tag.Value().toAscii().constData(), &cac_profile.x_ns, 1 );
			DCT_ASSERT( (no == 1) );
		}
		else if ( (child.toElement().tagName() == CALIB_SESNOR_CAC_X_NORMFACTOR_TAG)
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0) )
		{
			int no = ParseByteArray( tag.Value().toAscii().constData(), &cac_profile.x_nf, 1 );
			DCT_ASSERT( (no == 1) );
		}
		else if ( (child.toElement().tagName() == CALIB_SESNOR_CAC_Y_NORMSHIFT_TAG )
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0) )
		{
			int no = ParseByteArray( tag.Value().toAscii().constData(), &cac_profile.y_ns, 1 );
			DCT_ASSERT( (no == 1) );
		}
		else if ( (child.toElement().tagName() == CALIB_SESNOR_CAC_Y_NORMFACTOR_TAG)
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0) )
		{
			int no = ParseByteArray( tag.Value().toAscii().constData(), &cac_profile.y_nf, 1 );
			DCT_ASSERT( (no == 1) );
		}
		else if ( (child.toElement().tagName() == CALIB_SESNOR_CAC_X_OFFSET_TAG )
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0) )
		{
			int no = ParseShortArray( tag.Value().toAscii().constData(), &cac_profile.hCenterOffset, 1 );
			DCT_ASSERT( (no == 1) );
		}
		else if ( (child.toElement().tagName() == CALIB_SESNOR_CAC_Y_OFFSET_TAG )
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0) )
		{
			int no = ParseShortArray( tag.Value().toAscii().constData(), &cac_profile.vCenterOffset, 1 );
			DCT_ASSERT( (no == 1) );
		}
		else if ( (child.toElement().tagName() == CALIB_SESNOR_CAC_RED_PARAMETERS_TAG )
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0) )
		{
			int i = ( sizeof(cac_profile.Red) / sizeof(cac_profile.Red.fCoeff[0]) );
			int no = ParseFloatArray( tag.Value().toAscii().constData(), cac_profile.Red.fCoeff, i );
			DCT_ASSERT( (no == tag.Size()) );
		}
		else if ( (child.toElement().tagName() == CALIB_SESNOR_CAC_BLUE_PARAMETERS_TAG )
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0) )
		{
			int i = ( sizeof(cac_profile.Blue) / sizeof(cac_profile.Blue.fCoeff[0]) );
			int no = ParseFloatArray( tag.Value().toAscii().constData(), cac_profile.Blue.fCoeff, i );
			DCT_ASSERT( (no == tag.Size()) );
		}
		else
		{
			TRACE(CALIBDB_ERROR, "%s parse error in CAC section (unknown tag:%s)\n",
					__func__, child.toElement().tagName());
			return ( false );
		}

		child = child.nextSibling();
	}

	RESULT result = CamCalibDbAddCacProfile( m_CalibDbHandle, &cac_profile );
	DCT_ASSERT( result == RET_SUCCESS );

	TRACE(CALIBDB_INFO, "%s (exit)\n", __func__);

	return ( true );
}



/******************************************************************************
 * CalibDb::parseEntryDpf
 *****************************************************************************/
bool CalibDb::parseEntryDpf
(
    const QDomElement   &element,
    void                *param
)
{
	(void)param;

	TRACE(CALIBDB_INFO, "%s (enter)\n", __func__);

	CamDpfProfile_t dpf_profile;
	MEMSET( &dpf_profile, 0, sizeof( dpf_profile ) );

	QDomNode child = element.firstChild();
	while ( !child.isNull() )
	{
		XmlTag tag = XmlTag( child.toElement() );

		if ( (child.toElement().tagName() == CALIB_SENSOR_DPF_NAME_TAG)
				&& (tag.isType( XmlTag::TAG_TYPE_CHAR ))
				&& (tag.Size() > 0) )
		{
			QString value = tag.Value().toUpper();
			strncpy( dpf_profile.name, value.toAscii().constData(), sizeof( dpf_profile.name ) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_DPF_RESOLUTION_TAG)
					&& (tag.isType( XmlTag::TAG_TYPE_CHAR ))
					&& (tag.Size() > 0) )
		{
			QString value = tag.Value();
			strncpy( dpf_profile.resolution, value.toAscii().constData(), sizeof( dpf_profile.resolution ) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_DPF_NLL_SEGMENTATION_TAG)
					&& (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
					&& (tag.Size() > 0) )
		{
			int no = ParseUshortArray( tag.Value().toAscii().constData(), &dpf_profile.nll_segmentation, 1 );
			DCT_ASSERT( (no == 1) );
		}
		else if ( child.toElement().tagName() == CALIB_SENSOR_DPF_NLL_COEFF_TAG )
		{
			int i = ( sizeof(dpf_profile.nll_coeff) / sizeof(dpf_profile.nll_coeff.uCoeff[0]) );
			int no = ParseUshortArray( tag.Value().toAscii().constData(), dpf_profile.nll_coeff.uCoeff, i );
			DCT_ASSERT( (no == tag.Size()) );
		}
		else if ( child.toElement().tagName() == CALIB_SENSOR_DPF_SIGMA_GREEN_TAG )
		{
			int no = ParseUshortArray( tag.Value().toAscii().constData(), &dpf_profile.SigmaGreen, 1 );
			DCT_ASSERT( (no == 1) );
		}
		else if ( child.toElement().tagName() == CALIB_SENSOR_DPF_SIGMA_RED_BLUE_TAG )
		{
			int no = ParseUshortArray( tag.Value().toAscii().constData(), &dpf_profile.SigmaRedBlue, 1 );
			DCT_ASSERT( (no == 1) );
		}
		else if ( child.toElement().tagName() == CALIB_SENSOR_DPF_GRADIENT_TAG )
		{
			int no = ParseFloatArray( tag.Value().toAscii().constData(), &dpf_profile.fGradient, 1 );
			DCT_ASSERT( (no == 1) );
		}
		else if ( child.toElement().tagName() == CALIB_SENSOR_DPF_OFFSET_TAG )
		{
			int no = ParseFloatArray( tag.Value().toAscii().constData(), &dpf_profile.fOffset, 1 );
			DCT_ASSERT( (no == 1) );
		}
		else if ( child.toElement().tagName() == CALIB_SENSOR_DPF_NLGAINS_TAG )
		{
			int i = ( sizeof(dpf_profile.NfGains) / sizeof(dpf_profile.NfGains.fCoeff[0]) );
			int no = ParseFloatArray( tag.Value().toAscii().constData(), dpf_profile.NfGains.fCoeff, i );
			DCT_ASSERT( (no == tag.Size()) );
		}
		else
		{
			TRACE(CALIBDB_ERROR, "%s parse error in DPF section (unkonwn tag:%s)\n",
					__func__, child.toElement().tagName());
			return ( false );
		}

		child = child.nextSibling();
	}

	RESULT result = CamCalibDbAddDpfProfile( m_CalibDbHandle, &dpf_profile );
	DCT_ASSERT( result == RET_SUCCESS );

	TRACE(CALIBDB_INFO, "%s (exit)\n", __func__);

	return ( true );
}



/******************************************************************************
 * CalibDb::parseEntryDpcc
 *****************************************************************************/
bool CalibDb::parseEntryDpcc
(
    const QDomElement   &element,
    void                *param
)
{
	(void)param;

	TRACE(CALIBDB_INFO, "%s (enter)\n", __func__);

	CamDpccProfile_t dpcc_profile;
	MEMSET( &dpcc_profile, 0, sizeof( dpcc_profile ) );

	QDomNode child = element.firstChild();
	while ( !child.isNull() )
	{
		XmlTag tag = XmlTag( child.toElement() );

		if ( (child.toElement().tagName() == CALIB_SENSOR_DPCC_NAME_TAG)
				&& (tag.isType( XmlTag::TAG_TYPE_CHAR ))
				&& (tag.Size() > 0) )
		{
			QString value = tag.Value().toUpper();
			strncpy( dpcc_profile.name, value.toAscii().constData(), sizeof( dpcc_profile.name ) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_DPCC_RESOLUTION_TAG)
					&& (tag.isType( XmlTag::TAG_TYPE_CHAR ))
					&& (tag.Size() > 0) )
		{
			QString value = tag.Value();
			strncpy( dpcc_profile.resolution, value.toAscii().constData(), sizeof( dpcc_profile.resolution ) );
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_DPCC_REGISTER_TAG)
					&& (tag.isType( XmlTag::TAG_TYPE_CELL ))
					&& (tag.Size() > 0) )
		{
			if ( !parseEntryCell( child.toElement(), tag.Size(), &CalibDb::parseEntryDpccRegisters, &dpcc_profile ) )
			{
				TRACE(CALIBDB_ERROR, "%s parse error in DPF section (unknown tag:%s)\n",
						__func__, child.toElement().tagName());
				return ( false );
			}
		}
		else
		{
			TRACE(CALIBDB_ERROR, "%s parse error in APCC section (unknown tag:%s)\n",
					__func__, child.toElement().tagName());
			return ( false );
		}

		child = child.nextSibling();
	}

	RESULT result = CamCalibDbAddDpccProfile( m_CalibDbHandle, &dpcc_profile );
	DCT_ASSERT( result == RET_SUCCESS );

	TRACE(CALIBDB_INFO, "%s (exit)\n", __func__);

	return ( true );
}



/******************************************************************************
 * CalibDb::parseEntryDpccRegisters
 *****************************************************************************/
bool CalibDb::parseEntryDpccRegisters
(
    const QDomElement   &element,
    void                *param
)
{
	(void)param;

	TRACE(CALIBDB_INFO, "%s (enter)\n", __func__);

	CamDpccProfile_t *pDpcc_profile = (CamDpccProfile_t *)param;

	QString 	reg_name;
	uint32_t	reg_value = 0U;

	QDomNode child = element.firstChild();
	while ( !child.isNull() )
	{
		XmlTag tag = XmlTag( child.toElement() );

		if ( (child.toElement().tagName() == CALIB_SENSOR_DPCC_REGISTER_NAME_TAG)
				&& (tag.isType( XmlTag::TAG_TYPE_CHAR ))
				&& (tag.Size() > 0) )
		{
			reg_name = tag.Value().toUpper();
		}
		else if ( (child.toElement().tagName() == CALIB_SENSOR_DPCC_REGISTER_VALUE_TAG)
					&& (tag.isType( XmlTag::TAG_TYPE_CHAR ))
					&& (tag.Size() > 0) )
		{
			bool ok;

			reg_value = tag.ValueToUInt( &ok );
			if ( !ok )
			{
				TRACE(CALIBDB_ERROR, "%s parse error: invalid DPCC register value %s/%d\n",
						__func__, child.toElement().tagName(), tag.Value());
				return ( false );
			}
		}
		else
		{
			TRACE(CALIBDB_ERROR, "%s parse error in DPCC register section (unknown tag:%s)\n",
					__func__, child.toElement().tagName());
			return ( false );
		}

		child = child.nextSibling();
	}

	if ( reg_name == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_MODE )
	{
		pDpcc_profile->isp_dpcc_mode = reg_value;
	}
	else if ( reg_name == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_OUTPUT_MODE )
	{
		pDpcc_profile->isp_dpcc_output_mode = reg_value;
	}
	else if ( reg_name == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_SET_USE )
	{
		pDpcc_profile->isp_dpcc_set_use = reg_value;
	}
	else if ( reg_name == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_METHODS_SET_1 )
	{
		pDpcc_profile->isp_dpcc_methods_set_1 = reg_value;
	}
	else if ( reg_name == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_METHODS_SET_2 )
	{
		pDpcc_profile->isp_dpcc_methods_set_2 = reg_value;
	}
	else if ( reg_name == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_METHODS_SET_3 )
	{
		pDpcc_profile->isp_dpcc_methods_set_3 = reg_value;
	}
	else if ( reg_name == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_LINE_THRESH_1 )
	{
		pDpcc_profile->isp_dpcc_line_thresh_1 = reg_value;
	}
	else if ( reg_name == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_LINE_MAD_FAC_1 )
	{
		pDpcc_profile->isp_dpcc_line_mad_fac_1 = reg_value;
	}
	else if ( reg_name == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_PG_FAC_1 )
	{
		pDpcc_profile->isp_dpcc_pg_fac_1 = reg_value;
	}
	else if ( reg_name == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_RND_THRESH_1 )
	{
		pDpcc_profile->isp_dpcc_rnd_thresh_1 = reg_value;
	}
	else if ( reg_name == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_RG_FAC_1 )
	{
		pDpcc_profile->isp_dpcc_rg_fac_1 = reg_value;
	}
	else if ( reg_name == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_LINE_THRESH_2 )
	{
		pDpcc_profile->isp_dpcc_line_thresh_2 = reg_value;
	}
	else if ( reg_name == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_LINE_MAD_FAC_2 )
	{
		pDpcc_profile->isp_dpcc_line_mad_fac_2 = reg_value;
	}
	else if ( reg_name == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_PG_FAC_2 )
	{
		pDpcc_profile->isp_dpcc_pg_fac_2 = reg_value;
	}
	else if ( reg_name == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_RND_THRESH_2 )
	{
		pDpcc_profile->isp_dpcc_rnd_thresh_2 = reg_value;
	}
	else if ( reg_name == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_RG_FAC_2 )
	{
		pDpcc_profile->isp_dpcc_rg_fac_2 = reg_value;
	}
	else if ( reg_name == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_LINE_THRESH_3 )
	{
		pDpcc_profile->isp_dpcc_line_thresh_3 = reg_value;
	}
	else if ( reg_name == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_LINE_MAD_FAC_3 )
	{
		pDpcc_profile->isp_dpcc_line_mad_fac_3 = reg_value;
	}
	else if ( reg_name == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_PG_FAC_3 )
	{
		pDpcc_profile->isp_dpcc_pg_fac_3 = reg_value;
	}
	else if ( reg_name == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_RND_THRESH_3 )
	{
		pDpcc_profile->isp_dpcc_rnd_thresh_3 = reg_value;
	}
	else if ( reg_name == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_RG_FAC_3 )
	{
		pDpcc_profile->isp_dpcc_rg_fac_3 = reg_value;
	}
	else if ( reg_name == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_RO_LIMITS )
	{
		pDpcc_profile->isp_dpcc_ro_limits = reg_value;
	}
	else if ( reg_name == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_RND_OFFS )
	{
		pDpcc_profile->isp_dpcc_rnd_offs = reg_value;
	}
	else
	{
		TRACE(CALIBDB_ERROR, "%s unknown DPCC register (%s)\n",
				__func__, reg_name);
	}

	TRACE(CALIBDB_INFO, "%s (exit)\n", __func__);

	return ( true );
}



/******************************************************************************
 * CalibDb::parseEntrySystem
 *****************************************************************************/
bool CalibDb::parseEntrySystem
(
    const QDomElement   &element,
    void                *param
)
{
	(void)param;

	TRACE(CALIBDB_INFO, "%s (enter)\n", __func__);

	CamCalibSystemData_t system_data;
	MEMSET( &system_data, 0, sizeof( CamCalibSystemData_t ) );

	QDomNode child = element.firstChild();
	while ( !child.isNull() )
	{
		XmlTag tag = XmlTag( child.toElement() );
		QString value = tag.Value();

		if (child.toElement().tagName() == CALIB_SYSTEM_AFPS_TAG)
		{
			QDomNode firstChild = child.toElement().firstChild();
			if ( !firstChild.isNull() )
			{
				XmlTag firstTag = XmlTag( firstChild.toElement() );
				if ( (firstChild.toElement().tagName() == CALIB_SYSTEM_AFPS_DEFAULT_TAG)
							  && (firstTag.isType( XmlTag::TAG_TYPE_CHAR ))
							  && (firstTag.Size() > 0) )
				{
					QString value = tag.Value();
					system_data.AfpsDefault = ( value.contains( "on" ) ) ? BOOL_TRUE : BOOL_FALSE;
				}

			}
		}
		else
		{
			TRACE(CALIBDB_ERROR, "%s parse error in system section (unknown tag:%s)\n",
					__func__, child.toElement().tagName());
			return ( false );
		}

		child = child.nextSibling();
	}

	RESULT result = CamCalibDbSetSystemData( m_CalibDbHandle, &system_data );
	DCT_ASSERT( result == RET_SUCCESS );

	TRACE(CALIBDB_INFO, "%s (exit)\n", __func__);

	return ( true );
}


