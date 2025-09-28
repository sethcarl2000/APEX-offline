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
#include "ApexVDCHit.h"
#include "ApexVDCPlane.h"

class THaEvData; 

class ApexVDCPlane : public THaSubDetector {

private: 
    std::vector<ApexVDCWire> fWires; 
    std::vector<ApexVDCHit> fHits; 

public: 
    explicit ApexVDCPlane(  const char* name="", 
                            const char* description="", 
                            THaDetectorBase* parent=nullptr); 
    
    ~ApexVDCPlane(); 
    
    int Decode(const THaEvData& data) { return 0; }; //decode raw data
    
    //wires
    int GetNWires() const { return fWires.size(); }
    std::vector<ApexVDCWire> GetWires() const { return fWires; }
    
    //hits 
    int GetNHits() const { return fHits.size(); }
    std::vector<ApexVDCHit> GetHits() const { return fHits; }

    ClassDef(ApexVDCPlane,0); 
}; 

#endif 