#ifndef sonar_bvt_sdk_TYPES_HPP
#define sonar_bvt_sdk_TYPES_HPP

//#include "bvt_sdk.h"
#include <iostream>
/* If you need to define types specific to your oroGen components, define them
 * here. Required headers must be included explicitly
 *
 * However, it is common that you will only import types from your library, in
 * which case you do not need this file
 */

namespace sonar_bvt_sdk 
{
    namespace image_resolution
    {
        enum ImageResolution
        {
            Off = 0,
            Low = 1,
            Medium = 2,
            High = 3,
            Auto = 4
        };
    }

    namespace fluid_type
    {
        enum FluidType
        {
            Saltwater = 0,
            Freshwater = 1,
            Other = 2
        };
    };
}

#endif

