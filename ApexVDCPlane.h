#ifndef ApexVDCPlane_H
#define ApexVDCPlane_H 
////////////////////////////////////////////////////////////////////////////////////////
//
//  class: ApexVDCPlane 
// 
//  This class represents one of 4 VDC plane (U1, V1, U2, V2).   
//
//  - Seth H 28 Sep 25 
////////////////////////////////////////////////////////////////////////////////////////

#include <THaSubDetector.h> 
#include <THaDetectorBase.h>
#include <TDatime.h>  
#include "ApexVDCHit.h"
#include "ApexVDCPlane.h"

class THaEvData; 

class ApexVDCPlane : public THaSubDetector {

private: 

    //  These are the default parameters for a lot of values. 
    //      If you would like to change them without having to hard-code them here, see how they can 
    //      be manually speficied a DB file in the ApexVDCPlane::ReadDatabase() function. 
    //
    double fTDC_resolution  = 0.5e-9;  //by default, half a nanosecond. 
    double fTDC_rawtime_max = 2200.;  //maximum & minimum raw TDC times for each wire
    double fTDC_rawtime_min = 0.; 

    //information about the geometry of the detector
    int fNumWires = 368; 
    double fFirstWirePos, fWireSpacing, fWireAngle; 

    int fGroup_span_min = 2;      //for hit 'groups', the min and max span
    int fGroup_span_max = 10;     
    int fGroup_hits_min = 2;      //minimum number of hits for one group 
    int fGroup_max_gap  = 3;      //largest allowable 'gap' between hits in a group 

    std::vector<ApexVDCWire> fWires; //list of all wires 
    std::vector<ApexVDCHit>  fHits;  //list of all hits (for a single event)

public: 
    explicit ApexVDCPlane(  const char* name="", 
                            const char* description="", 
                            THaDetectorBase* parent=nullptr); 
    
    ~ApexVDCPlane(); 
    
    //decode raw TDC data into hit-data 
    int Decode(const THaEvData& data) { return 0; }; //decode raw data
    
    //read database, given the input date 
    int ReadDataBase(const TDatime& date);

    //wires
    int GetNWires() const { return fWires.size(); }
    std::vector<ApexVDCWire> GetWires() const { return fWires; }
    
    //hits 
    int GetNHits() const { return fHits.size(); }
    std::vector<ApexVDCHit> GetHits() const { return fHits; }

    ClassDef(ApexVDCPlane,0); 
}; 

#endif 