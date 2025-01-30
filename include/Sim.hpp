/* 
Class to run the a full scale simulation on a WSC type route
*/

#pragma once

#include <stdbool.h>
#include <string>
#include <memory>
#include <vector>

#include "CustomTime.hpp"
#include "Car.hpp"
#include "Luts.hpp"

class Simulator {
 private:
  // Lookup tables
  Route route;
  ForecastLut forecast_lut;

  // Control stops
  std::unordered_set<size_t> control_stops;

  // NO TOUCH ANYTHING IN SIMULATION PARAMETERS
  /* ---------------------- Simulation parameters ------------------------- */
  // Step size in seconds when charging
  const int CHARGING_STEP_SIZE = 30;
  // Stop time at a control stop in seconds
  const int control_stop_charge_time = 1800;
  // For each timestamp, we track the UTC offset. For the location of the race (Australia),
  // it is 9.5 hours ahead of UTC
  // Day one start time in 24 hour local time
  const Time day_one_start_time = Time("2023-10-22 10:00:00", -9.5);
  // Day one end time in 24 hour local time
  const Time day_one_end_time = Time("2023-10-22 18:00:00", -9.5);
  // Start time from day 2 onwards in 24 hour local time
  const Time day_start_time = Time("09:00:00");
  // End time from day 2 onwards in 24 hour local time
  const Time day_end_time = Time("17:00:00");
  // End time of the entire race in 24 hour local time
  const Time race_end_time = Time("2023-10-28 17:00:00");
  // NO TOUCH
  /* ---------------------- Simulation parameters ------------------------- */

  // Starting coordinate of the car
  Coord starting_coord;
  // Starting time of the simulation
  Time curr_time;

  /* Energy model of the car to simulate on */
  std::shared_ptr<Car> car;

 public:
  /** Construct all simulator objects this way
   * @param model Energy model for your car
   * @param starting_coord The starting coordinate of the car
   * @param current_time Current starting time of the simulation
  */
  Simulator(std::shared_ptr<Car> model, const Coord starting_coord, const Time starting_time);

  // Setters
  inline void set_control_stops(std::unordered_set<size_t> stops) { control_stops = stops; }
  inline void set_route(Route new_route) { route = new_route; }
  inline void set_forecast_lut(ForecastLut new_forecast_lut) { forecast_lut = new_forecast_lut; }

  /** @brief Run a full simulation with a car object and a series of route points
  *
  * @param speed: The speed in kph
  * 
  * @return True if this is a posible
  */
  bool run_sim(const double speed);
};
