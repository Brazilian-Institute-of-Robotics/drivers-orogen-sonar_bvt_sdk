    /* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "Task.hpp"

#include <iostream>
#include <stdexcept>

using namespace sonar_bvt_sdk;

Task::Task(std::string const& name)
    : TaskBase(name), head(NULL) 
{
}

Task::Task(std::string const& name, RTT::ExecutionEngine* engine)
    : TaskBase(name, engine), head(NULL)
{
}

Task::~Task()
{
}



/// The following lines are template definitions for the various state machine
// hooks defined by Orocos::RTT. See Task.hpp for more detailed
// documentation about them.

bool Task::configureHook() {
    if (!TaskBase::configureHook())
        return false;

    try {
        // connect to connect blueview sonar
        son.Open("NET", _ip_address.get().c_str());
        head = son.GetHead(0);

        // set the range
        this->setRange(_range.get());

        // set the analog gain adjustment
        double db = 49 * _gain.get() + 1;
        head.SetGainAdjustment(db);

        // set Clipping Threshold
        head.SetClippingThreshold(_clipping_thresh.get());

        // set TVG Slope
        head.SetTVGSlope(_tvg_slope.get());

        // set the speed of sound
        head.SetSoundSpeed(_sound_speed.get());

        // set Fluid Type
        this->setFluidType(_fluid_type.get());

        // set image processing resolution
        this->setImageResolution(_image_resolution.get());

        first_ping = true;

    } catch (BVTSDK::SdkException &e) {
        // throw it here since BVTSDK::SdkException doesn't inherit from std::runtime_error
        throw std::runtime_error(e.ErrorMessage());
    }

    return true;
}

bool Task::startHook() {
    if (!TaskBase::startHook())
        return false;

    // set head to the image generator
    img_gen.SetHead(head);

    return true;
}

void Task::updateHook() {
    TaskBase::updateHook();

    try {
        // get Ping
        BVTSDK::Ping ping(head.GetPing(-1));

        // get the R-Theta image from the ping
        BVTSDK::MagImage img_rtheta(img_gen.GetImageRTheta(ping));

        // set Sonar parameters
        if (first_ping) {
            this->setSonarParameters(ping, img_rtheta);
            first_ping = false;
        }

	bvt_sonar.bin_count = img_rtheta.GetHeight();

        // get the raw data pointer
        unsigned short *bit_buffer = img_rtheta.GetBits();
        int beams = bvt_sonar.beam_count;
        int bins = bvt_sonar.bin_count;
        bvt_sonar.bins.clear();

        // normalize raw data with its maximum value (2^16)
        for (int i = 0; i < beams; ++i) {
            for (int j = bins; j > 0; --j) {
                float value = (bit_buffer[(j * beams + i)] * 1.0 / 65535);
                bvt_sonar.bins.push_back(value);
            }
        }
        bvt_sonar.time = base::Time::now();

        // display sonar data (Sonar structure)
        _sonar_output.write(bvt_sonar);

        // display sonar data (SonarScan structure)
        base::samples::SonarScan bvt_scan;
        bvt_scan = convertSonarToSonarScan(bvt_sonar);
        _sonar_samples.write(bvt_scan);

        // display sonar temperature (in degree celsius)
        float temperature;
        if (son.TryGetTemperature(&temperature)) {
            _sonar_temperature.write((double) son.GetTemperature());
        }
    } catch (BVTSDK::SdkException &e) {
        throw std::runtime_error(e.ErrorMessage());
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

bool Task::setRange(double value) {
    head.SetStopRange(value);
    return (sonar_bvt_sdk::TaskBase::setRange(value));
}

bool Task::setTvg_slope(double value) {
    // check if the desired tvg is valid
    if (value < 0 || value > 10)
        return false;

    head.SetTVGSlope(value);
    return (sonar_bvt_sdk::TaskBase::setTvg_slope(value));
}

bool Task::setGain(double value) {
    // convert gain percent to dB value
    double db = 49 * value + 1;
    head.SetGainAdjustment(db);
    return (sonar_bvt_sdk::TaskBase::setGain(value));
}

void Task::setFluidType(sonar_bvt_sdk::fluid_type::FluidType type) {
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
            throw std::invalid_argument("Fluid type parameter does not match a known enum value");
    }
}

void Task::setImageResolution(sonar_bvt_sdk::image_resolution::ImageResolution resolution) {
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
            throw std::invalid_argument("Image resolution parameter does not match a known enum value");
    }
}

void Task::setSonarParameters(BVTSDK::Ping &ping, BVTSDK::MagImage &img_rtheta) {
    float max_angle = img_rtheta.GetFOVMaxAngle();
    float min_angle = img_rtheta.GetFOVMinAngle();

    bvt_sonar.speed_of_sound = ping.GetSoundSpeed() * 1.0;
    bvt_sonar.beam_width = base::Angle::fromDeg(max_angle - min_angle);
    bvt_sonar.beam_height = base::Angle::fromDeg(1); // from datasheet of Blueview MB1350-N
    bvt_sonar.beam_count = img_rtheta.GetWidth();

    float angular_resolution = bvt_sonar.beam_width.getDeg() / bvt_sonar.beam_count;
    bvt_sonar.setRegularBeamBearings(base::Angle::fromDeg(min_angle), base::Angle::fromDeg(angular_resolution));

    bvt_sonar.validate();
}

base::samples::SonarScan Task::convertSonarToSonarScan(base::samples::Sonar const &sonar) {
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

    // convert blueview raw data to 8 bits
    sonar_scan.data.clear();
    for (int i = 0; i < sonar.bins.size(); ++i) {
        sonar_scan.data.push_back((uint8_t) (sonar.bins[i] * 255));
    }

    return sonar_scan;
}
