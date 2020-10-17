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
 * @file        xmltags.cpp
 *
 *****************************************************************************/
#include <cstdio>
#include "calibtags.h"
#include "xmltags.h"


/******************************************************************************
 * class XmlTag
 *****************************************************************************/

/******************************************************************************
 * XmlTag::XmlTag
 *****************************************************************************/
XmlTag::XmlTag( const QDomElement& e )
    : m_Element( e )
{
}



/******************************************************************************
 * XmlTag::XmlTag
 *****************************************************************************/
int XmlTag::Size()
{
    QDomAttr attr = m_Element.attributeNode( CALIB_ATTRIBUTE_SIZE );
    QString string = attr.value();

    int col;
    int row;

    int res = sscanf( (const char *)string.toAscii().constData(),
                        CALIB_ATTRIBUTE_SIZE_FORMAT, &col, &row );
    if ( res != CALIB_ATTRIBUTE_SIZE_NO_ELEMENTS )
    {
        return ( 0 );
    }

    return ( (col * row) );
}



/******************************************************************************
 * XmlTag::Value
 *****************************************************************************/
QString XmlTag::Value()
{
    return ( m_Element.text().trimmed() );
}


/******************************************************************************
 * XmlTag::ValueTonUInt
 *****************************************************************************/
unsigned int XmlTag::ValueToUInt( bool *ok )
{
    QString value = Value();

    int v = value.toUInt( ok, 16 );
    if ( !(*ok) )
    {
        v = value.toUInt( ok );
    }

    return ( v );
}



/******************************************************************************
 * XmlTag::Type
 *****************************************************************************/
XmlTag::TagType_e XmlTag::Type()
{
    QDomAttr attr = m_Element.attributeNode( CALIB_ATTRIBUTE_TYPE );

    if ( attr.value() == CALIB_ATTRIBUTE_TYPE_CHAR )
    {
        return ( TAG_TYPE_CHAR ); 
    }
    else if ( attr.value() == CALIB_ATTRIBUTE_TYPE_DOUBLE )
    {
        return ( TAG_TYPE_DOUBLE );
    }
    else if ( attr.value() == CALIB_ATTRIBUTE_TYPE_STRUCT )
    {
        return ( TAG_TYPE_STRUCT );
    }
    else if ( attr.value() == CALIB_ATTRIBUTE_TYPE_CELL )
    {
        return ( TAG_TYPE_CELL );
    }
    else 
    {
        return ( TAG_TYPE_INVALID );
    }
    
    return ( TAG_TYPE_INVALID );
}



/******************************************************************************
 * XmlTag::isType
 *****************************************************************************/
bool XmlTag::isType
( 
    const XmlTag::TagType_e type
)
{
    QDomAttr attr = m_Element.attributeNode( CALIB_ATTRIBUTE_TYPE );

    if ( attr.value() == CALIB_ATTRIBUTE_TYPE_CHAR )
    {
        return ( (bool)(TAG_TYPE_CHAR == type) ); 
    }
    else if ( attr.value() == CALIB_ATTRIBUTE_TYPE_DOUBLE )
    {
        return ( (bool)(TAG_TYPE_DOUBLE == type) );
    }
    else if ( attr.value() == CALIB_ATTRIBUTE_TYPE_STRUCT )
    {
        return ( (bool)(TAG_TYPE_STRUCT == type) );
    }
    else if ( attr.value() == CALIB_ATTRIBUTE_TYPE_CELL )
    {
        return ( (bool)(TAG_TYPE_CELL == type) );
    }
    else 
    {
        return ( (bool)(TAG_TYPE_INVALID == type) );
    }
    
    return ( false );
}



/******************************************************************************
 * class XmlCellTag
 *****************************************************************************/

/******************************************************************************
 * XmlCellTag::XmlCellTag
 *****************************************************************************/
XmlCellTag::XmlCellTag( const QDomElement& e )
    : XmlTag( e )
{
}



/******************************************************************************
 * XmlCellTag::XmlTag
 *****************************************************************************/
int XmlCellTag::Index()
{
    int value = 0;

    QDomAttr attr = m_Element.attributeNode( CALIB_ATTRIBUTE_INDEX );
    if ( !attr.isNull() )
    {
        value = attr.value().toInt( );
    }

    return ( value );
}


