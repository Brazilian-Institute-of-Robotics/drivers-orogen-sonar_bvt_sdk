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
    struct BvtMonitor
    {
        /** BVT internal temperature in Celsius **/
        float temperature;
        std::string serial_number;
        std::string firmware_revision;
        int head_numbers;
        std::string model_name;
        std::string sonar_name;
        int orientation;
        //BVTSDK::NetworkSettings;
        bool support_target_tracking;
        bool support_source_level_control;
        bool support_external_hardware_trigger;
        float source_level;
    };
}

#endif

