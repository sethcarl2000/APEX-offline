#include "ApexVDCPlane.h"

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
