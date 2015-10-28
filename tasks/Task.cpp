    /* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "Task.hpp"

using namespace sonar_bvt_sdk;

Task::Task(std::string const& name)
    : TaskBase(name), head(BVTSDK::Head(NULL)), head_file(BVTSDK::Head(NULL)) 
{
}

Task::Task(std::string const& name, RTT::ExecutionEngine* engine)
    : TaskBase(name, engine), head(BVTSDK::Head(NULL)), head_file(BVTSDK::Head(NULL))
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
    if (! TaskBase::configureHook())
        return false;

    try
    {
        // Open the sonar
        std::string ip_address = _ip_address.get();
//        son.Open("NET", _ip_address.get().c_str());

        son.Open("FILE", "/home/romulo/flat_fish/dev/drivers/sonar_bvt_sdk/data/swimmer.son");
        head = son.GetHead(0);

        //set the range
        this->setRange(_range.get());
        //set the analog gain adjustment (in dB)
        head.SetGainAdjustment(_gain.get());

        //set Clipping Threshold
        head.SetClippingThreshold(_clipping_thresh.get());

        //set TVG Slope
        head.SetTVGSlope(_tvg_slope.get());

        //set the speed of sound
        head.SetSoundSpeed(_sound_speed.get());

        //set Fluid Type
        this->setFluidType(_fluid_type.get());

        //set image processing resolution
        this->setImageResolution(_image_resolution.get());

        /////// Create a file to save the pings    /////
        if (_save_file.get())
        {
            file.CreateFile(_file_name.get().c_str(), son, "");
            // Request the first head for the file
            head_file = file.GetHead(0);
        }

        first_ping = true;
    }
    catch (BVTSDK::SdkException &e)
    {
        std::cout << e.ErrorName() << std::endl;
        throw std::runtime_error(e.ErrorMessage());
    }

    return true;
    
}
bool Task::startHook()
{
    if (! TaskBase::startHook())
    {
        return false;
    }
    return true;
}
void Task::updateHook()
{
    TaskBase::updateHook();

    for (int i = 0; i < head.GetPingCount(); ++i) {

        try {
            //Get Ping
//            BVTSDK::Ping ping(head.GetPing(-1));
            BVTSDK::Ping ping(head.GetPing(i));

            // Save ping in the file
            if (_save_file.get()) {
                head_file.PutPing(ping);
            }

            // Generate an image from the ping
            BVTSDK::ImageGenerator img_gen;
            img_gen.SetHead(head);

            // Get the R-Theta image from the ping
            BVTSDK::MagImage img_rtheta(img_gen.GetImageRTheta(ping));

            // Set SonarScan parameters
            if (first_ping) {
                this->setSonarParameters(ping, img_rtheta);
                first_ping = false;
            }

            // Get the raw data pointer
            unsigned short *bit_buffer = img_rtheta.GetBits();
            bvt_sonar.bins.clear();

            bvt_sonar.bin_count = img_rtheta.GetHeight();
            int beams = bvt_sonar.beam_count;
            int bins = bvt_sonar.bin_count;
            bvt_sonar.bins.clear();
            for (int i = 0; i < beams; ++i) {
                for (int j = bins; j > 0; --j) {
                    // normalize raw data with its maximum value (2^13)
                    float value = (bit_buffer[(j * beams + i)] * 1.0 / 8191);
                    bvt_sonar.bins.push_back(value);
                }
            }

            bvt_sonar.time = base::Time::now();
            _sonar_output.write(bvt_sonar);

            base::samples::SonarScan bvt_scan;
            bvt_scan = convertSonarToSonarScan(bvt_sonar);
            _sonar_scan.write(bvt_scan);

            float temperature;
            if (son.TryGetTemperature(&temperature)) {
                _sonar_temperature.write((double) son.GetTemperature());
            }
        } catch (BVTSDK::SdkException &e) {
            std::cout << e.ErrorName() << std::endl;
            throw std::runtime_error(e.ErrorMessage());
        }
    }
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

bool Task::setRange(double value)
{
    head.SetStopRange(value);
    //check if the desired range was really set.
    if (value != head.GetStopRange()) {
        std::cout << "Required range is larger than allowed, the maximum value " << head.GetStopRange() << " was set." << std::endl;
    }
    return (sonar_bvt_sdk::TaskBase::setRange(head.GetStopRange()));
}

bool Task::setGain(double value)
{
    head.SetGainAdjustment(value);
    return (sonar_bvt_sdk::TaskBase::setGain(value));
}

void Task::setFluidType(sonar_bvt_sdk::fluid_type::FluidType type)
{
    switch (type) {
        case sonar_bvt_sdk::fluid_type::Saltwater:
            head.SetFluidType(BVTSDK::FluidType::Saltwater);
            break;
        case sonar_bvt_sdk::fluid_type::Freshwater:
            head.SetFluidType(BVTSDK::FluidType::Freshwater);
            break;
        case sonar_bvt_sdk::fluid_type::Other:
            head.SetFluidType(BVTSDK::FluidType::Other);
            break;
        default:
            throw std::runtime_error("The fluid type could not be set");
    }
}

void Task::setImageResolution(sonar_bvt_sdk::image_resolution::ImageResolution resolution)
{
    switch (resolution) {
        case sonar_bvt_sdk::image_resolution::Off:
            head.SetImageRes(BVTSDK::ImageRes::Off);
            break;
        case sonar_bvt_sdk::image_resolution::Low:
            head.SetImageRes(BVTSDK::ImageRes::Low);
            break;
        case sonar_bvt_sdk::image_resolution::Medium:
            head.SetImageRes(BVTSDK::ImageRes::Medium);
            break;
        case sonar_bvt_sdk::image_resolution::High:
            head.SetImageRes(BVTSDK::ImageRes::High);
            break;
        case sonar_bvt_sdk::image_resolution::Auto:
            head.SetImageRes(BVTSDK::ImageRes::Auto);
            break;
        default:
            throw std::runtime_error("The image resolution could not be set");
    }
}

void Task::setSonarParameters(BVTSDK::Ping &ping, BVTSDK::MagImage &img_rtheta)
{
    float max_angle = img_rtheta.GetFOVMaxAngle();
    float min_angle = img_rtheta.GetFOVMinAngle();

    bvt_sonar.speed_of_sound = ping.GetSoundSpeed() * 1.0;
    bvt_sonar.beam_width = base::Angle::fromDeg(max_angle - min_angle);
    bvt_sonar.beam_height = base::Angle::fromDeg(1); //from datasheet of Blueview MB1350-N
    bvt_sonar.beam_count = img_rtheta.GetWidth();

    float angular_resolution = bvt_sonar.beam_width.getDeg() / bvt_sonar.beam_count;
    bvt_sonar.setRegularBeamBearings(base::Angle::fromDeg(min_angle), base::Angle::fromDeg(angular_resolution));

    bvt_sonar.validate();
}

base::samples::SonarScan Task::convertSonarToSonarScan(base::samples::Sonar const &sonar)
{
    base::samples::SonarScan sonar_scan;

    sonar_scan.time = sonar.time;
    sonar_scan.speed_of_sound = sonar.speed_of_sound;
    sonar_scan.start_bearing = sonar.bearings[0];
    sonar_scan.angular_resolution = sonar.bearings[1] - sonar.bearings[0];
    sonar_scan.beamwidth_horizontal = sonar.beam_width;
    sonar_scan.beamwidth_vertical = sonar.beam_height;
    sonar_scan.number_of_beams = sonar.beam_count;
    sonar_scan.number_of_bins = sonar.bin_count;
    sonar_scan.memory_layout_column = true;
    sonar_scan.polar_coordinates = true;

    sonar_scan.data.clear();
    for (int i = 0; i < sonar.bins.size(); ++i) {
        // convert normalized data to 8 bits format
        sonar_scan.data.push_back((uint8_t) (sonar.bins[i] * 255));
    }

    return sonar_scan;
}
