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
#include <TVector3.h> 
#include "ApexVDCHit.h"
#include "ApexVDCWire.h"
#include "ApexVDCHitGroup.h"

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
    TVector3 fCenter;       //center of the detector, displaced from the origin of the DET coordinate system [m]. 
    double fLength, fWidth; //length (x-dir) and width (y-dir) of the detector [m]
    int fNumWires = 368; 
    //position of the first wire from the center of the detector [m], along with wire spacing [m]. 
    double fFirstWirePos, fWireSpacing, fWireAngle; 

    int fGroup_span_min = 2;      //for hit 'groups', the min and max span
    int fGroup_span_max = 10;     
    int fGroup_hits_min = 2;      //minimum number of hits for one group 
    int fGroup_max_gap  = 3;      //largest allowable 'gap' between hits in a group 

    std::vector<ApexVDCWire>        fWires; //list of all wires 
    std::vector<ApexVDCHit>         fHits;  //list of all hits (for a single event)
    std::vector<ApexVDCHitGroup>    fGroups; //list of all hit 'groups'  

public: 
    explicit ApexVDCPlane(  const char* name="", 
                            const char* description="", 
                            THaDetectorBase* parent=nullptr); 
    
    ~ApexVDCPlane(); 
    
    //read database, given the input date 
    int ReadDataBase(const TDatime& date);

    //read geometry from the database
    int ReadGeometry(FILE* file, const TDatime& date); 

    //define variables which this detector can provide
    int DefineVariables(EMode mode = kDefine); 

    //these are a list of functions that will give us the variables we want. 
    // see the implementation of 'DefineVariables' above to see how they are defined. 
    std::vector<double> GetHits_rawtime()   const noexcept;
    std::vector<double> GetHits_time()      const noexcept; 
    std::vector<int>    GetHits_wire()      const noexcept;  
    std::vector<double> GetHits_pos()       const noexcept;  
    std::vector<int>    GetGroups_start()   const noexcept; 
    std::vector<int>    GetGroups_end()     const noexcept; 
    std::vector<int>    GetGroups_span()    const noexcept; 

    //decode raw TDC data into hit-data 
    int Decode(const THaEvData& data) { return 0; }; //decode raw data
    

    //wires
    int GetNWires() const { return fWires.size(); }
    std::vector<ApexVDCWire> GetWires() const noexcept { return fWires; }
    
    //hits 
    int GetNHits() const { return fHits.size(); }
    std::vector<ApexVDCHit> GetHits() const noexcept { return fHits; }

    //groups
    int GetNGroups() const { return fGroups.size(); }
    std::vector<ApexVDCHitGroup> GetGroups() const noexcept { return fGroups; }

    ClassDef(ApexVDCPlane,0); 
}; 

#endif 