/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "Task.hpp"

using namespace sonar_bvt_sdk;

Task::Task(std::string const& name)
    : TaskBase(name)
{
}

Task::Task(std::string const& name, RTT::ExecutionEngine* engine)
    : TaskBase(name, engine)
{
}

Task::~Task()
{
}



/// The following lines are template definitions for the various state machine
// hooks defined by Orocos::RTT. See Task.hpp for more detailed
// documentation about them.

bool Task::configureHook()
{
// - todo: use the method IsConnected();
// - use TryGetTemperature(float *temperature) to get temperature;
// - verify what is NavData, it seems to be a file that stores the navigation log, 
//wich might be useful for offline data treatment. see PutNavData(const NavData &nav_data);
// - See what is "orientation", it is gonna be implemented in the bvt_monitor, but maybe it's 
//necessary to create a input port that will receive data from the rotation motor and set the
//bvt orientation using PutOrientation and retrieve with GetOrientation;
// - see what is EventMark class;
    if (! TaskBase::configureHook())
        return false;

    char DataFile[] = "/home/diego/Programas/rock/drivers/sonar_bvt_sdk/data/swimmer.son";

    int ret;
    // Create a new BVTSonar Object
    BVTSonar son = BVTSonar_Create();
    if( son == NULL )
    {
        printf("BVTSonar_Create: failed\n");
        return 1;
    }

    // Open the sonar
    ret = BVTSonar_Open(son, "FILE", DataFile);
    if( ret != 0 )
    {
        printf("BVTSonar_Open: ret=%d\n", ret);
        return 1;
    }

    // Make sure we have the right number of heads
    int heads = -1;
    BVTSonar_GetHeadCount(son, &heads);
    printf("BVTSonar_GetHeadCount: %d\n", heads);


    // Get the first head
    BVTHead head = NULL;
    ret = BVTSonar_GetHead(son, 0, &head);
    if( ret != 0 )
    {
        printf("BVTSonar_GetHead: ret=%d\n", ret);
        return 1;
    }

    // Check the ping count
    int pings = -1;
    BVTHead_GetPingCount(head, &pings);
    printf("BVTHead_GetPingCount: %d\n", pings);

    // Check the min and max range in this file
    float min_range, max_range;
    BVTHead_GetMinimumRange(head, &min_range);
    BVTHead_GetMaximumRange(head, &max_range);
    printf("BVTHead_GetMinimumRange: %0.2f\n", min_range );
    printf("BVTHead_GetMaximumRange: %0.2f\n", max_range );

    BVTImageGenerator ig = BVTImageGenerator_Create();
    BVTImageGenerator_SetHead(ig, head);

    // Now, get a ping!
    BVTPing ping = NULL;
    ret = BVTHead_GetPing(head, 0, &ping);
    if( ret != 0 )
    {
        printf("BVTHead_GetPing: ret=%d\n", ret);
        return 1;
    }

    // Generate an image from the ping
    BVTMagImage img;
    ret = BVTImageGenerator_GetImageXY(ig, ping, &img);
    if( ret != 0 )
    {
        printf("BVTImageGenerator_GetImageXY: ret=%d\n", ret);
        return 1;
    }

    printf("\n");

    /////////////////////////////////////////////////////////

    // Check the image height and width out
    int height;
    BVTMagImage_GetHeight(img, &height);
    printf("BVTMagImage_GetHeight: %d\n", height);
    int width;
    BVTMagImage_GetWidth(img, &width);
    printf("BVTMagImage_GetWidth: %d\n", width);

    // Save it to a PGM (PortableGreyMap)
    ret = BVTMagImage_SavePGM(img, "img.pgm");
    if( ret != 0 )
    {
        printf("BVTMagImage_SavePGM: ret=%d\n", ret);
        return 1;
    }

    /////////////////////////////////////////////////////////

    // Build a color mapper
    BVTColorMapper mapper;
    mapper = BVTColorMapper_Create();
    if( mapper == NULL )
    {
        printf("BVTColorMapper_Create: failed\n");
        return 1;
    }

    // Load the bone colormap
    ret = BVTColorMapper_Load(mapper, "bone.cmap");
    if( ret != 0 )
    {
        printf("BVTColorMapper_Load: ret=%d\n", ret);
        return 1;
    }


    // Perform the colormapping
    BVTColorImage cimg;
    ret = BVTColorMapper_MapImage(mapper, img, &cimg);
    if( ret != 0 )
    {
        printf("BVTColorMapper_MapImage: ret=%d\n", ret);
        return 1;
    }
    printf("\n");

    /////////////////////////////////////////////////////////
    // Check the image height and width out
    BVTColorImage_GetHeight(cimg, &height);
    printf("BVTColorImage_GetHeight: %d\n", height);
    BVTColorImage_GetWidth(cimg, &width);
    printf("BVTColorImage_GetWidth: %d\n", width);


    // Save it to a PPM (PortablePixMap)
    ret = BVTColorImage_SavePPM(cimg, "cimg.ppm");
    if( ret != 0 )
    {
        printf("BVTColorImage_SavePPM: ret=%d\n", ret);
        return 1;
    }

    // Clean up
    BVTColorImage_Destroy(cimg);
    BVTMagImage_Destroy(img);
    BVTColorMapper_Destroy(mapper);
    BVTPing_Destroy(ping);
    BVTSonar_Destroy(son);
    return true;
}
bool Task::startHook()
{
    if (! TaskBase::startHook())
        return false;
    return true;
}
void Task::updateHook()
{
    TaskBase::updateHook();
}
void Task::errorHook()
{
    TaskBase::errorHook();
}
void Task::stopHook()
{
    TaskBase::stopHook();
}
void Task::cleanupHook()
{
    TaskBase::cleanupHook();
}
