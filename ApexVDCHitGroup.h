#ifndef ApexVDCHitGroup_H
#define ApexVDCHitGroup_H
////////////////////////////////////////////////////////////////////////////////////////
//
//  struct: ApexVDCHitGroup 
//  A 'group' is a collection of hits which are close enough to one another to be 
//  considered as a candidate for a cluster.   
//
//  Note: right now its just an alias for a std::vector<ApexVDCHit>, but if it needs
//  more complicated functionality of its own later, I can add it. 
//
//  - Seth H 28 Sep 25 
////////////////////////////////////////////////////////////////////////////////////////

#include "ApexVDCWire.h"
#include "ApexVDCHit.h"
#include <vector> 

using ApexVDCHitGroup = std::vector<ApexVDCHit>; 

#endif 