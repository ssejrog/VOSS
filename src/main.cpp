#include "main.h"
#include "VOSS/api.hpp"
#include "VOSS/controller/BoomerangControllerBuilder.hpp"
#include "VOSS/controller/PIDControllerBuilder.hpp"
#include "VOSS/controller/SwingControllerBuilder.hpp"
#include "VOSS/localizer/ADILocalizerBuilder.hpp"
#include "VOSS/utils/flags.hpp"

#define LEFT_MOTORS                                                            \
    { -4, -1, -21, 8, 13 }
#define RIGHT_MOTORS                                                           \
    { 10, 3, 9, -7, -15 }

auto odom = voss::localizer::IMELocalizerBuilder::new_builder()
                .with_track_width(11)
                .with_left_right_tpi(18.43)
                .with_imu(16)
                .with_right_motors(LEFT_MOTORS)
                .with_left_motors(RIGHT_MOTORS)
                .build();

auto pid = voss::controller::PIDControllerBuilder::new_builder(odom)
               .with_linear_constants(20, 0.02, 169)
               .with_angular_constants(250, 0.05, 2435)
               .with_min_error(5)
               .with_min_vel_for_thru(100)
               .build();

auto boomerang = voss::controller::BoomerangControllerBuilder::new_builder(odom)
                     .with_linear_constants(20, 0.02, 169)
                     .with_angular_constants(250, 0.05, 2435)
                     .with_exit_error(1.0)
                     .with_lead_pct(0.5)
                     .with_angular_exit_error(1.0)
                     .with_min_error(5)
                     .with_min_vel_for_thru(70)
                     .with_settle_time(200)
                     .build();

auto swing = voss::controller::SwingControllerBuilder::new_builder(odom)
                 .with_angular_constants(250, 0.05, 2435)
                 .build();

auto arc = voss::controller::ArcPIDControllerBuilder(odom)
               .with_track_width(14)
               .with_linear_constants(20, 0.02, 169)
               .with_min_error(5)
               .build();

pros::Controller master(pros::E_CONTROLLER_MASTER);
auto ec = voss::controller::ExitConditions::new_conditions()
              .add_settle(400, 0.5, 400)
              .add_tolerance(1.0, 2.0)
              .add_timeout(22500)
              .add_thru_smoothness(4)
              .build() -> exit_if([]() {
                  return master.get_digital(pros::E_CONTROLLER_DIGITAL_UP);
              });

auto chassis = voss::chassis::DiffChassis(LEFT_MOTORS, RIGHT_MOTORS, pid, ec,
                                          pros::E_MOTOR_BRAKE_COAST);

pros::IMU imu(16);

/**
 * Runs initialization code. This occurs as soon as the program is started.
 *
 * All other competition modes are blocked by initialize; it is recommended
 * to keep execution time for this mode under a few seconds.
 */
void initialize() {
    pros::lcd::initialize();
    odom->begin_localization();
}

/**
 * Runs while the robot is in the disabled state of Field Management System or
 * the VEX Competition Switch, following either autonomous or opcontrol. When
 * the robot is enabled, this task will exit.
 */
void disabled() {
}

/**
 * Runs after initialize(), and before autonomous when connected to the Field
 * Management System or the VEX Competition Switch. This is intended for
 * competition-specific initialization routines, such as an autonomous selector
 * on the LCD.
 *
 * This task will exit when the robot is enabled and autonomous or opcontrol
 * starts.
 */
void competition_initialize() {
}

/**
 * Runs the user autonomous code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the autonomous
 * mode. Alternatively, this function may be called in initialize or opcontrol
 * for non-competition testing purposes.
 *
 * If the robot is disabled or communications is lost, the autonomous task
 * will be stopped. Re-enabling the robot will restart the task, not re-start it
 * from where it left off.
 */
void autonomous() {
    // auto odom = voss::localizer::ADILocalizerBuilder::new_builder().build();
    // auto pid =
    // voss::controller::BoomerangControllerBuilder::new_builder(odom)
    //                .with_lead_pct(60)
    //                .build();

    // // auto pid2 =
    // // voss::controller::BoomerangControllerBuilder::new_builder(odom)
    // //                 .with_lead_pct(65)
    // //                 .build();

    // auto pid2 = voss::controller::ControllerCopy(pid).modify_lead_pct(65);
}

/**
 * Runs the operator control code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the operator
 * control mode.
 *
 * If no competition control is connected, this function will run immediately
 * following initialize().
 *
 * If the robot is disabled or communications is lost, the
 * operator control task will be stopped. Re-enabling the robot will restart the
 * task, not resume it from where it left off.
 */
void opcontrol() {

    while (true) {
        voss::Pose p = odom->get_pose();

        chassis.arcade(master.get_analog(ANALOG_LEFT_Y),
                       master.get_analog(ANALOG_RIGHT_X));

        if (master.get_digital_new_press(DIGITAL_Y)) {
            odom->set_pose({0.0, 0.0, 270});
            chassis.move({24, 24, 45}, boomerang, 100,
                         voss::Flags::THRU | voss::Flags::REVERSE);
            printf("1.\n");
            master.rumble("--");
            chassis.turn(90, 100, voss::Flags::THRU);
            printf("2.\n");
            master.rumble("--");
            chassis.move({-10, 60, 180}, boomerang, 100, voss::Flags::THRU);
            printf("3.\n");
            master.rumble("--");
            chassis.turn(270, swing, 100,
                         voss::Flags::REVERSE | voss::Flags::THRU);
            printf("4.\n");
            master.rumble("--");
            chassis.move({10, 30}, 100, voss::Flags::THRU);
            printf("5.\n");
            master.rumble("--");
            chassis.turn(0);
        }

        pros::lcd::clear_line(1);
        pros::lcd::clear_line(2);
        pros::lcd::clear_line(3);
        pros::lcd::print(1, "%lf", p.x);
        pros::lcd::print(2, "%lf", p.y);
        pros::lcd::print(3, "%lf", odom->get_orientation_deg());
        pros::lcd::print(4, "%s", (odom == nullptr) ? "true" : "false");
        pros::delay(10);
    }
}