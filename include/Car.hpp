#pragma once

#include <memory>

#ifndef CAR_HPP
class Car {
 private:
  /* Define car parameters here according to the doc */
  // Private methods for calculating each loss/gain component
  
    double calc_aero_loss(double velocity);
    double calc_rolling_loss(double velocity);
    double calc_gravity_loss(double velocity, double angle);
    double calc_solar_gain(double irradiance);

 public:
  Car();
  // Energy consumption calculation (W)
  double energy_consumption(double velocity, double angle, double irradiance);
  
  // Define energy loss functions

};

#endif // CAR_HPP
