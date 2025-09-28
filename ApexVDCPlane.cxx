#include "ApexVDCPlane.h"
#include <Database.h> 
#include <THaDetMap.h> 
#include <vector>
#include <stdexcept> 
#include <sstream> 

using namespace std; 

//______________________________________________________________________________________________________
ApexVDCPlane::ApexVDCPlane(const char* name, const char* description, THaDetectorBase* parent)
    : THaSubDetector(name, description, parent), 
    fWires{},
    fHits{}
{
    //noop 
}

//______________________________________________________________________________________________________
ApexVDCPlane::~ApexVDCPlane() 
{
    RemoveVariables();     
}

//______________________________________________________________________________________________________
int ApexVDCPlane::ReadDataBase(const TDatime& date)
{
    // Load VDCPlane parameters from database

    const char* const here = "ReadDatabase";

    FILE* file = OpenFile(date);
    if( !file ) return kFileError;

    // Read fCenter and fSize
    int err; 
    if((err = ReadGeometry(file, date)) != 0) {
        fclose(file);
        return err;
    }

    // Read configuration parameters
    vector<int> detmap, bad_wirelist;
    // timing offset for each wire
    vector<double> tdc_offsets;
    TString ttd_conv = "AnalyticTTDConv";

    // The anatomy of a 'DBRequest' struct is as follows (taken from Database/VarDef.h):
    //
    //    struct DBRequest {
    //        const char*      name;     // Key name
    //        void*            var;      // Pointer to data (default to Double*)
    //        VarType          type;     // (opt) data type (see VarType.h, default Double_t)
    //        UInt_t           nelem;    // (opt) number of array elements (0/1 = 1 or auto)
    //        Bool_t           optional; // (opt) If true, missing key is ok
    //        Int_t            search;   // (opt) Search for key along name tree
    //        const char*      descript; // (opt) Key description (if 0, same as name)
    //     }; 
    //
    DBRequest request[] = {
        { "detmap",         &detmap,           kIntV,    0, false},
        { "nwires",         &fNumWires,        kInt,     0, false, -1 },
        { "wire.start",     &fFirstWirePos,    kDouble,  0, false, -1 },
        { "wire.spacing",   &fWireSpacing,     kDouble,  0, false, -1 },
        { "wire.angle",     &fWireAngle,       kDouble,  0, false, -1 },
        { "wire.badlist",   &bad_wirelist,     kIntV,    0, true,  -1 },
        { "tdc.min",        &fTDC_rawtime_min, kInt,     0, true,  -1 },
        { "tdc.max",        &fTDC_rawtime_max, kInt,     0, true,  -1 },
        { "tdc.res",        &fTDC_resolution,  kDouble,  0, false, -1 },
        { "tdc.offsets",    &tdc_offsets,      kFloatV,  0, false, -1 },
        { "group.minhits",  &fGroup_hits_min,  kInt,     0, true,  -1 },
        { "group.minspan",  &fGroup_span_min,  kInt,     0, true,  -1 },
        { "group.maxspan",  &fGroup_span_max,  kInt,     0, true,  -1 },
        { "group.maxgap",   &fGroup_max_gap,   kInt,     0, true,  -1 },
        { "description",    &fTitle,           kTString, 0, true },
        { nullptr }
    };

    if((err = LoadDB(file, date, request, fPrefix)) != 0) {
        fclose(file);
        return err;
    }
  
    if( FillDetMap(detmap, THaDetMap::kFillLogicalChannel, here) <= 0 ) return kInitError; // Error already printed by FillDetMap

    // All our frontend modules are common stop TDCs
    UInt_t nmodules = fDetMap->GetSize();
    for( UInt_t i = 0; i < nmodules; i++ ) {
        THaDetMap::Module* d = fDetMap->GetModule(i);
        d->MakeTDC();
        d->SetTDCMode(false);
    }

    // Derived geometry quantities
    fWireAngle *= TMath::DegToRad();

    //check if the number of wires we were told that we have actually matches the nubmer of DB file entries for TDC offsets
    if (tdc_offsets.size() != (size_t)fNumWires) {
        ostringstream oss; 
        oss << "in <" << here << ">: number of tdc offsets parsed from DB file " 
            "(" << tdc_offsets.size() << ") does not match number of wires given (" << fNumWires << ")"; 
        throw logic_error(oss.str()); 
    }

    // Initialize wires
    // start with the position of first wire
    double wire_pos = fFirstWirePos; 

    for (int i=0; i<fNumWires; i++) {
        ApexVDCWire wire(i, wire_pos, tdc_offsets[i], fTDC_resolution); 
        fWires.push_back(wire);
        wire_pos += fWireSpacing; 
    }

    return kOK;
}

//______________________________________________________________________________________________________
//______________________________________________________________________________________________________
//______________________________________________________________________________________________________
//______________________________________________________________________________________________________
//______________________________________________________________________________________________________
//______________________________________________________________________________________________________

ClassImp(ApexVDCPlane)