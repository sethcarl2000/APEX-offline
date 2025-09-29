#ifndef ApexVDCHit_H
#define ApexVDCHit_H
////////////////////////////////////////////////////////////////////////////////////////
//
//  class: ApexVDCWire 
//  A very basic container for APEX vdc hit information.  
//
//  - Seth H 28 Sep 25 
////////////////////////////////////////////////////////////////////////////////////////

#include "ApexVDCWire.h"

struct ApexVDCHit {
    const ApexVDCWire* wire; 
    double realtime, rawtime;  

    //needed for std::sort()
    bool operator<(const ApexVDCHit& rhs) const {
        //sort by ascending wire num
        if (wire->GetNum() < rhs.wire->GetNum()) return true; 
        //sort by ascending realtime (so descending rawtime)
        if (wire->GetNum() == rhs.wire->GetNum() && rawtime > rhs.rawtime) return true;  
        
        return false; 
    }  
};

#endif 