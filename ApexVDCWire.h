#ifndef ApexVDCWire_H
#define ApexVDCWire_H
////////////////////////////////////////////////////////////////////////////////////////
//
//  class: ApexVDCWire 
//  A very basic object representing a single VDC wire. 
//  Contains: 
//   - a pointer to the partiuclar VDC wire to which it belongs, 
//   - raw TDC time 
//   - corrected, "real" VDC time. 
//
//  These will be owned & accessed by the wires themselves. 
//
//  - Seth H 28 Sep 25 
////////////////////////////////////////////////////////////////////////////////////////

#include <vector> 
#include <limits> 

class ApexVDCWire {
private: 
    //wire position & number 
    double fWirePos;
    unsigned int fWireNum; 
    
    //timing offset & resolution 
    double fTimingOffset; 
    double fTimingResolution; 
    
public: 
    ApexVDCWire(unsigned int wire_num=0, 
                double wire_pos      =std::numeric_limits<double>::quiet_NaN(), 
                double timing_offset =std::numeric_limits<double>::quiet_NaN(), 
                double timing_resolution=0.5e-9)
        : fWirePos{wire_pos}, 
        fWireNum{wire_num}, 
        fTimingOffset{timing_offset}, 
        fTimingResolution{timing_resolution} {};

    ~ApexVDCWire() = default; 

    double GetPos() const { return fWirePos; }
    double GetTimingOffset() const { return fTimingOffset; }
    int    GetNum() const { return fWireNum; }
};

#endif 