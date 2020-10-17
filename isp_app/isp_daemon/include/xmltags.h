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
 * @file xmltags.h
 *
 *****************************************************************************/
 #ifndef __XMLTAGS_H__
 #define __XMLTAGS_H__

#include <QDomDocument>

/******************************************************************************
 * class XmlTag
 *****************************************************************************/
class XmlTag
{
public:
    enum TagType_e
    {
        TAG_TYPE_INVALID    = 0,
        TAG_TYPE_CHAR,
        TAG_TYPE_DOUBLE,
        TAG_TYPE_STRUCT,
        TAG_TYPE_CELL,
        TAG_TYPE_MAX
    };

    XmlTag( const QDomElement& e );
    
    int Size();
    QString Value();
    unsigned int ValueToUInt( bool *ok );

    TagType_e Type();
    bool isType( const TagType_e type );

protected:
    QDomElement m_Element;
};


/******************************************************************************
 * class CellTag
 *****************************************************************************/
class XmlCellTag: public XmlTag
{
public:
    XmlCellTag( const QDomElement& e );

    int Index();
};

#endif /* __XMLTAGS_H__ */

