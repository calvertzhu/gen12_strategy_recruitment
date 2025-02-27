#include "Car.hpp"


// Write implemenation for your energy model here

#include <cmath>

// Constants
const double rho = 1.225;                 // Air density (kg/m^3)
const double cda = 0.15;                  // Drag area (m^2)
const double rolling_resistance = 0.0026; // Rolling resistance coefficient
const double car_mass = 283.0;            // Car mass (kg)
const double gravity = 9.81;              // Acceleration due to gravity (m/s^2)
const double passive_loss = 20.0;         // Passive electric loss (W)
const double array_area = 4.0;            // Solar array area (m^2)
const double array_efficiency = 0.252;    // Solar array efficiency
const double battery_efficiency = 0.98;   // Battery efficiency
const double motor_efficiency = 0.8;      // Motor efficiency

Car::Car() {}

// Calculate aerodynamic loss (W)
double Car::calc_aero_loss(double velocity) {
    return 0.5 * rho * cda * pow(velocity, 3);
}

// Calculate rolling resistance loss (W)
double Car::calc_rolling_loss(double velocity) {
    return rolling_resistance * car_mass * gravity * velocity;
}

// Calculate gravitational loss/gain (W)
double Car::calc_gravity_loss(double velocity, double angle) {
    return car_mass * gravity * velocity * sin(angle);
}

// Calculate solar energy gain (W)
double Car::calc_solar_gain(double irradiance) {
    return irradiance * array_area * array_efficiency;
}

// Positive value indicates battery charging, negative indicates discharging
double Car::energy_consumption(double velocity, double angle, double irradiance) {
    double aero = calc_aero_loss(velocity);
    double rolling = calc_rolling_loss(velocity);
    double gravity = calc_gravity_loss(velocity, angle);
    double solar = calc_solar_gain(irradiance);

    double total_losses = (aero + rolling + gravity) / motor_efficiency + passive_loss;

    // Net power = solar gain - total losses, adjusted for battery efficiency
    return solar * battery_efficiency - total_losses;
}
