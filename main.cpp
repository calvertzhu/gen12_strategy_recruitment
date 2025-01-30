/* Starting point of a race simulation */

#include <stdlib.h>
#include <stdio.h>

#include <string>
#include <utility>
#include <vector>
#include <filesystem>
#include <cstdlib>
#include <memory>
#include <unordered_set>

#include "Sim.hpp"
#include "Car.hpp"
#include "Utils.hpp"

int main(int argc, char* argv[]) {
  RUNTIME_EXCEPTION(argc == 3, "Need base route location and dni csv location. Example ./sim.exe baseroute.csv dni.csv");

  // Control stops - DO NOT TOUCH
  // For each idx in control_stops, the car must stop for 30 minutes at route.get_route_points()[idx]
  // This Must be reflected in your Sim class implementation
  std::unordered_set<size_t> control_stops = {2962,5559,9462,11421,14439,16990,20832,23202,25987};

  // Load base route csv
  Route route{std::string(argv[1])};

  // Load forecast irradiance csv
  ForecastLut forecast_lut{std::string(argv[2])};

  // Create your model of the car
  std::shared_ptr<Car> Car;

  // First coordinate in baseroute.csv
  const Coord starting_coord = route.get_route_points()[0];

  // Start time of the first race day
  const Time starting_time = Time("2023-10-22 10:00:00");

  // Create your simulator object and set route parameters
  Simulator simulator(Car, starting_coord, starting_time);
  simulator.set_control_stops(control_stops);
  simulator.set_forecast_lut(forecast_lut);
  simulator.set_route(route);

  // Loop through viable speeds from 1 to 100
  for (int i=0; i<100; i++) {
    if (simulator.run_sim(static_cast<double>(i))) {
      std::cout << "Speed " << i << " is viable" << std::endl;
    } else {
      std::cout << "Speed " << i << " is not viable" << std::endl;
    }
  }
  return 0;
}
