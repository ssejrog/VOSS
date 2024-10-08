#include "VOSS/localizer/AbstractLocalizer.hpp"
#include "VOSS/constants.hpp"
#include "VOSS/utils/angle.hpp"
#include <cmath>

namespace voss::localizer {

// Creating the localiozer object with a starting pose of x = 0, y = 0, and
// heading = 0
AbstractLocalizer::AbstractLocalizer() {
    this->pose = AtomicPose{0.0, 0.0, 0.0};
}

// Starting the localization task
// Once started it don't stop until program is stopped or data abort
// Uses mutex to keep values protected
void AbstractLocalizer::begin_localization() {
    pros::Task localization_task([this]() {
        this->calibrate();
        while (true) {
            std::unique_lock<pros::Mutex> lock(this->mtx);
            this->update();
            lock.unlock();

            pros::delay(constants::SENSOR_UPDATE_DELAY);
        }
    });
}
// These last few functions allows the user to set and get values of the robot's
// pose while keeping the values safe with mutex
void AbstractLocalizer::set_pose(Pose pose) {
    std::unique_lock<pros::Mutex> lock(this->mtx);
    if (pose.theta.has_value()) {
        this->pose =
            AtomicPose{pose.x, pose.y, voss::to_radians(pose.theta.value())};
    } else {
        double h = this->pose.theta;
        this->pose = AtomicPose{pose.x, pose.y, h};
    }
}

void AbstractLocalizer::set_pose(double x, double y, double theta) {
    this->set_pose({x, y, theta});
}

Pose AbstractLocalizer::get_pose() {
    std::unique_lock<pros::Mutex> lock(this->mtx);
    Pose ret = {this->pose.x.load(), this->pose.y.load(),
                this->pose.theta.load()};
    return ret;
}

double AbstractLocalizer::get_x() {
    std::unique_lock<pros::Mutex> lock(this->mtx);
    double ret = this->pose.x;
    return ret;
}

double AbstractLocalizer::get_y() {
    std::unique_lock<pros::Mutex> lock(this->mtx);
    double ret = this->pose.y;
    return ret;
}

double AbstractLocalizer::get_orientation_rad() {
    std::unique_lock<pros::Mutex> lock(this->mtx);
    double ret = this->pose.theta;
    return ret;
}

double AbstractLocalizer::get_orientation_deg() {
    std::unique_lock<pros::Mutex> lock(this->mtx);
    double ret = this->pose.theta * 180 * M_1_PI;
    return ret;
}

Point AbstractLocalizer::get_position() {
    std::unique_lock<pros::Mutex> lock(this->mtx);
    Point ret{this->pose.x, this->pose.y};
    return ret;
}

} // namespace voss::localizer