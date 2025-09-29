#include "ApexVDCPlane.h"
#include <Database.h> 
#include <THaDetMap.h> 
#include <vector>
#include <stdexcept> 
#include <sstream> 
#include <utility> 

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
        { "tdc.offsets",    &tdc_offsets,      kDoubleV, 0, false, -1 },
        { "group.minhits",  &fGroup_hits_min,  kInt,     0, true,  -1 },
        { "group.minspan",  &fGroup_span_min,  kInt,     0, true,  -1 },
        { "group.maxspan",  &fGroup_span_max,  kInt,     0, true,  -1 },
        { "group.maxgap",   &fGroup_max_gap,   kInt,     0, true,  -1 },
        { "description",    &fTitle,           kTString, 0, true },
        { nullptr }
    };

    //try to read the DB variables from our request above. 
    if((err = LoadDB(file, date, request, fPrefix)) != 0) {
        fclose(file);
        return err;
    }

    //try to read the geometry from the DB file
    if ((err = ReadGeometry(file, date)) != 0) {
        fclose(file); 
        return err; 
    }

    //try to fill the detmap
    if( FillDetMap(detmap, THaDetMap::kFillLogicalChannel, here) <= 0 ) 
        return kInitError; // Error already printed by FillDetMap

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
int ApexVDCPlane::ReadGeometry(FILE* file, const TDatime& date)
{
    //read geometry of the VDC from the DB file 
    const char* const here = "ReadGeometry";

    vector<double> position; 
    DBRequest request[] = {
        { "position", &position, kDoubleV, 0, false, 0, "detector center position in DET coord sys.[m])" },
        { "length",   &fLength,  kDouble,  0, false, 0, "detector length (x-direction) [m])" },
        { "width",    &fWidth,   kDouble,  0, false, 0, "detector width (y-direction) [m])"},
        { nullptr }
    };
    int err;
    if((err = LoadDB(file, date, request)) != 0) return kInitError;

    if (position.size() != 3) {
        ostringstream oss;
        oss << "in <" << here << ">: number of DB entries under 'position' "
               "("<< position.size()<<") is incorrect, must be 3 (x/y/z).";

        throw logic_error(oss.str());
        return kInitError; 
    }

    //set the center of the detector
    fCenter.SetXYZ( position[0], position[1], position[2] );

    return kOK; 
}

//______________________________________________________________________________________________________
int ApexVDCPlane::DefineVariables(EMode mode)
{
    // Here is the anatomy of an 'RVarDef' struct, taken from VarDef.h: 
    // 
    // struct RVarDef {
    //    const char*      name;     // Variable name
    //    const char*      desc;     // Variable description
    //    const char*      def;      // Definition of data (data member or method name)
    // };
    //
    RVarDef vars[] = {
        {"nhits",           "Number of (raw) hits in this plane for this event",    "GetNHits()"},
        {"hit.rawtime",     "Array of raw times for each hit",                      "GetHits_rawtime()"},
        {"hit.time_gbl",    "offset-corrected 'real time' [s], but NOT relative to the event's selected S2 hit.", "GetHits_time()"},
        {"hit.wire",        "VDC wire ID of this hit",                              "GetHits_wire()"},
        {"hit.pos",         "position of this hit's wire [m]. given in UV-coords, rel. to central wire in plane.", "GetHits_pos()"},
        {"ngroups",         "Number of hit-groups formed in this plane",            "GetNGroups()"},
        {"group.nhits",     "Number of hits in this group",                         "GetGroups_nhits()"},
        {"group.start",     "Hit index of first hit in group",                      "GetGroups_start()"},
        {"group.end",       "Hit index of last hit in group",                       "GetGroups_end()"},
        {"group.span",      "number of wires between first and last hit + 1 (see above def.)",  "GetGroups_span()"},
        {nullptr}
    }; 

    return DefineVarsFromList(vars, mode); 
}

//______________________________________________________________________________________________________
vector<double> ApexVDCPlane::GetHits_rawtime() const noexcept
{
    vector<double> ret; ret.reserve(fHits.size()); 
    for (const auto& hit : fHits) ret.emplace_back(hit.rawtime);
    return ret;  
}
//______________________________________________________________________________________________________
vector<double> ApexVDCPlane::GetHits_time() const noexcept
{
    vector<double> ret; ret.reserve(fHits.size()); 
    for (const auto& hit : fHits) ret.emplace_back(hit.realtime);
    return ret;  
}
//______________________________________________________________________________________________________
vector<int> ApexVDCPlane::GetHits_wire() const noexcept
{
    vector<int> ret; ret.reserve(fHits.size()); 
    for (const auto& hit : fHits) ret.emplace_back(hit.wire->GetNum());
    return ret;  
}
//______________________________________________________________________________________________________
vector<double> ApexVDCPlane::GetHits_pos() const noexcept
{
    vector<double> ret; ret.reserve(fHits.size()); 
    for (const auto& hit : fHits) ret.emplace_back(hit.wire->GetPos());
    return ret;  
}
//______________________________________________________________________________________________________
vector<int> ApexVDCPlane::GetGroups_start() const noexcept
{
    vector<int> ret; ret.reserve(fHits.size()); 
    for (const auto& group : fGroups) ret.emplace_back(group.front().wire->GetNum());
    return ret;  
}
//______________________________________________________________________________________________________
vector<int> ApexVDCPlane::GetGroups_end() const noexcept
{
    vector<int> ret; ret.reserve(fHits.size()); 
    for (const auto& group : fGroups) ret.emplace_back(group.back().wire->GetNum());
    return ret;  
}
//______________________________________________________________________________________________________
vector<int> ApexVDCPlane::GetGroups_span() const noexcept
{
    vector<int> ret; ret.reserve(fHits.size()); 
    for (const auto& group : fGroups) ret.emplace_back(group.front().wire->GetNum() - group.back().wire->GetNum());
    return ret;  
}
//______________________________________________________________________________________________________
int ApexVDCPlane::StoreHit(const DigitizerHitInfo_t& hit_info, UInt_t data)
{
    //according to THaDetMap.h, 'DigitizerHitInfo_t', which is just an 
    // alias for 'THaDetMap::Iterator::HitInfo_t', has the following public data members:
    //
    // ...  
    //  Decoder::Module*     module; // Current frontend module being decoded
    //  Decoder::ChannelType type;   // Measurement type for current channel (ADC/TDC)
    //  Decoder::ChannelType modtype; // Module type (ADC/TDC/MultiFunctionADC etc.)
    //  UInt_t  ev;      // Event number (for error messages)
    //  UInt_t  crate;   // Hardware crate
    //  UInt_t  slot;    // Hardware slot
    //  UInt_t  chan;    // Physical channel in current module
    //  UInt_t  nhit;    // Number of hits in current channel
    //  UInt_t  hit;     // Hit number in current channel
    //  Int_t   lchan;   // Logical channel according to detector map
    // ... 
    // 
    const char* const here = "StoreHit"; 

    int wire_num = hit_info.lchan; 
    //check if this logical channel number is invalid or not
    if (wire_num < 0 || (size_t)wire_num >= fWires.size()) {
        ostringstream oss;
        oss <<  "in <" << here << ">: wire number of hit is invalid (" << wire_num << ") must be"
                " in the range [0," << fWires.size()-1 << "]."; 
        throw logic_error(oss.str()); 
        return -1;  
    }

    //convert the data to a double 
    double rawtime = static_cast<double>(data); 

    //check to see if this hit is outside of our TDC cutoff range 
    if (rawtime > fTDC_rawtime_max || rawtime < fTDC_rawtime_min) return 0; 

    //grab our wire so we can get some info from it 
    const ApexVDCWire* wire = &fWires[wire_num]; 

    //now, compute the real-time
    double realtime = (((double)rawtime) * fTDC_resolution)  +  wire->GetTimingOffset(); 

    //add this hit to the list of all hits
    fHits.push_back({wire, realtime, rawtime}); 

    return 0; 
}

//______________________________________________________________________________________________________
//______________________________________________________________________________________________________
//______________________________________________________________________________________________________
//______________________________________________________________________________________________________
//______________________________________________________________________________________________________
//______________________________________________________________________________________________________
//______________________________________________________________________________________________________
//______________________________________________________________________________________________________
//______________________________________________________________________________________________________
//______________________________________________________________________________________________________
//______________________________________________________________________________________________________
//______________________________________________________________________________________________________

ClassImp(ApexVDCPlane)