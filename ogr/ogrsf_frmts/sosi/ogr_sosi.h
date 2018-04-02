/******************************************************************************
 * $Id: ogr_sosi.h 34420 2016-06-24 21:06:03Z rouault $
 *
 * Project:  SOSI Translator
 * Purpose:  Implements OGRSOSIDriver.
 * Author:   Thomas Hirsch, <thomas.hirsch statkart no>
 *
 ******************************************************************************
 * Copyright (c) 2010, Thomas Hirsch
 * Copyright (c) 2010, Even Rouault <even dot rouault at mines-paris dot org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 ****************************************************************************/

#ifndef OGR_SOSI_H_INCLUDED
#define OGR_SOSI_H_INCLUDED

#include "ogrsf_frmts.h"
#include "fyba.h"
#include <map>

// Note: WRITE_SUPPORT not defined, since this is only partially implemented

/* interpolation of arcs(BUEP) creates # points for a full circle */
#define ARC_INTERPOLATION_FULL_CIRCLE 36.0

typedef std::map<CPLString, CPLString> S2S;
typedef std::map<CPLString, unsigned int> S2I;

void RegisterOGRSOSI();

class ORGSOSILayer;      /* defined below */
class OGRSOSIDataSource; /* defined below */

/************************************************************************
 *                           OGRSOSILayer                               *
 * OGRSOSILayer reports all the OGR Features generated by the data      *
 * source, in an orderly fashion.                                       *
 ************************************************************************/

class OGRSOSILayer : public OGRLayer {
    int                 nNextFID;

    OGRSOSIDataSource  *poParent;   /* used to call methods from data source */
    LC_FILADM          *poFileadm;  /* ResetReading needs to refer to the file struct */
    OGRFeatureDefn     *poFeatureDefn;  /* the common definition of all features returned by this layer  */
    S2I                *poHeaderDefn;

    LC_SNR_ADM          oSnradm;
    LC_BGR              oNextSerial;  /* used by FYBA to iterate through features */
    LC_BGR             *poNextSerial;

public:
    OGRSOSILayer( OGRSOSIDataSource *poPar, OGRFeatureDefn *poFeatDefn, LC_FILADM *poFil, S2I *poHeadDefn);
    ~OGRSOSILayer();

    void                ResetReading();
    OGRFeature *        GetNextFeature();
    OGRFeatureDefn *    GetLayerDefn();
#ifdef WRITE_SUPPORT
    OGRErr              CreateField(OGRFieldDefn *poField, int bApproxOK=TRUE);
    OGRErr              ICreateFeature(OGRFeature *poFeature);
#endif
    int                 TestCapability( const char * );
};

/************************************************************************
 *                           OGRSOSIDataSource                          *
 * OGRSOSIDataSource reads a SOSI file, prebuilds the features, and     *
 * creates one OGRSOSILayer per geometry type                           *
 ************************************************************************/
class OGRSOSIDataSource : public OGRDataSource {
    char                *pszName;
    OGRSOSILayer        **papoLayers;
    int                 nLayers;

#define MODE_READING 0
#define MODE_WRITING 1
    int                 nMode;

    void                buildOGRPoint(long nSerial);
    void                buildOGRLineString(int nNumCoo, long nSerial);
    void                buildOGRMultiPoint(int nNumCoo, long nSerial);
    void                buildOGRLineStringFromArc(long nSerial);

public:

    OGRSpatialReference *poSRS;
    const char          *pszEncoding;
    unsigned int        nNumFeatures;
    OGRGeometry         **papoBuiltGeometries;  /* OGRSOSIDataSource prebuilds some features upon opening, te be used
                                                * by the more complex geometries later. */
    //FYBA specific
    LC_BASEADM          *poBaseadm;
    LC_FILADM           *poFileadm;

    S2I                 *poPolyHeaders;   /* Contain the header definitions of the four feature layers */
    S2I                 *poPointHeaders;
    S2I                 *poCurveHeaders;
    S2I                 *poTextHeaders;

    OGRSOSIDataSource();
    ~OGRSOSIDataSource();

    int                 Open  ( const char * pszFilename, int bUpdate);
#ifdef WRITE_SUPPORT
    int                 Create( const char * pszFilename );
#endif
    const char          *GetName() {
        return pszName;
    }
    int                 GetLayerCount() {
        return nLayers;
    }
    OGRLayer            *GetLayer( int );
#ifdef WRITE_SUPPORT
    OGRLayer            *ICreateLayer( const char *pszName, OGRSpatialReference  *poSpatialRef=NULL, OGRwkbGeometryType eGType=wkbUnknown, char **papszOptions=NULL);
#endif
    int                 TestCapability( const char * );
};


/************************************************************************
 *                           OGRSOSIDataTypes                           *
 * OGRSOSIDataTypes provides the correct data types for some of the     *
 * most common SOSI elements.                                           *
 ************************************************************************/

class OGRSOSISimpleDataType {
    const char          *pszName; 
    OGRFieldType        nType;

public:
    OGRSOSISimpleDataType ();
    OGRSOSISimpleDataType (const char *pszName, OGRFieldType nType);
    ~OGRSOSISimpleDataType();

    void setType (const char *pszName, OGRFieldType nType);
    const char          *GetName() {
        return pszName;
    };
    OGRFieldType        GetType() {
        return nType;
    };

};

class OGRSOSIDataType {
    OGRSOSISimpleDataType* poElements;
    int                    nElementCount;
public:
    OGRSOSIDataType (int nSize);
    ~OGRSOSIDataType();

    void setElement(int nIndex, const char *name, OGRFieldType type);
    OGRSOSISimpleDataType* getElements() {
        return poElements;
    };
    int getElementCount() {
        return nElementCount;
    };
};

typedef std::map<CPLString, OGRSOSIDataType> C2F;

void SOSIInitTypes();
OGRSOSIDataType* SOSIGetType(CPLString name);
int  SOSITypeToInt(const char* value);
double  SOSITypeToReal(const char* value);
void SOSITypeToDate(const char* value, int* date);
void SOSITypeToDateTime(const char* value, int* date);

#endif /* OGR_SOSI_H_INCLUDED */
