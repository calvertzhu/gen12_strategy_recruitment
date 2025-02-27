#include <memory>
#include <vector>
#include <unordered_set>
#include <limits>
#include <utility>

#include "Sim.hpp"
#include "Utils.hpp"
#include "Car.hpp"

Simulator::Simulator(std::shared_ptr<Car> model, const Coord starting_coord,
                     const Time starting_time) : car(model), starting_coord(starting_coord),
                                                 curr_time(starting_time) {}
          
// Write your implementation here

bool Simulator::run_sim(const double speed) {
  RUNTIME_EXCEPTION(car != nullptr, "Car is null");

  curr_time = day_one_start_time;
  const double battery_capacity = 5.2 * 3600 * 1000; // 5.2 kWh in Joules
  double battery_energy = battery_capacity;

  // Load route points.
  const std::vector<Coord>& points = route.get_route_points();
  size_t num_points = points.size();

  Time finish_deadline("2023-10-28 17:00:00", -9.5);

  // Constants
  const double array_area = 4.0;
  const double array_efficiency = 0.252;
  const double battery_efficiency = 0.98;
  const double EPS = 1e-6;

  // Returns true if t is within allowed driving hours.
  auto is_driving_time = [&](const Time &t) -> bool {
    tm tm_local = t.m_datetime_local;
    int hour = tm_local.tm_hour, day = tm_local.tm_mday;
    int month = tm_local.tm_mon + 1, year = tm_local.tm_year + 1900;
    if (year == 2023 && month == 10) {
      if (day == 22)
        return (hour >= 10 && hour < 18);
      else if (day >= 23 && day <= 28)
        return (hour >= 9 && hour < 17);
    }
    return false;
  };

  // Returns seconds until the next driving window.
  auto time_until_driving_start = [&](const Time &t) -> double {
    tm tm_local = t.m_datetime_local;
    double current_seconds = tm_local.tm_hour * 3600 + tm_local.tm_min * 60 + tm_local.tm_sec;
    double start_seconds = 0;
    int day = tm_local.tm_mday, month = tm_local.tm_mon + 1, year = tm_local.tm_year + 1900;
    if (year == 2023 && month == 10)
      start_seconds = (day == 22) ? 10 * 3600 : 9 * 3600;
    if (current_seconds < start_seconds)
      return start_seconds - current_seconds;
    return 24 * 3600 - current_seconds + 9 * 3600;
  };

  // Returns the remaining driving time in the current window.
  auto driving_time_remaining = [&](const Time &t) -> double {
    tm tm_local = t.m_datetime_local;
    double current_seconds = tm_local.tm_hour * 3600 + tm_local.tm_min * 60 + tm_local.tm_sec;
    double end_seconds = 0;
    int day = tm_local.tm_mday, month = tm_local.tm_mon + 1, year = tm_local.tm_year + 1900;
    if (year == 2023 && month == 10)
      end_seconds = (day == 22) ? 18 * 3600 : 17 * 3600;
    return (end_seconds > current_seconds) ? (end_seconds - current_seconds) : 0;
  };

  // Returns current UTC time (for irradiance lookup).
  auto get_epoch = [&]() -> time_t {
    return curr_time.t_datetime_utc;
  };

  // Returns true if the finish deadline is exceeded.
  auto check_deadline = [&]() -> bool {
    return (curr_time > finish_deadline);
  };

  // Process each route segment.
  for (size_t i = 0; i < num_points - 1; i++) {
    if (check_deadline())
      return false;
    const Coord &curr_point = points[i];
    const Coord &next_point = points[i + 1];
    double segment_distance = get_distance(curr_point, next_point);
    double alt_diff = next_point.alt - curr_point.alt;
    double angle = (segment_distance > 0) ? asin(alt_diff / segment_distance) : 0.0;
    double remaining_distance = segment_distance;

    // Drive the segment until finished.
    while (remaining_distance > EPS) {
      if (check_deadline())
        return false;
      if (!is_driving_time(curr_time)) {
        double wait_time = time_until_driving_start(curr_time);
        double irradiance = forecast_lut.get_value({curr_point.lat, curr_point.lon}, get_epoch());
        double stationary_net_power = irradiance * array_area * array_efficiency * battery_efficiency;
        battery_energy += stationary_net_power * wait_time;
        if (battery_energy > battery_capacity)
          battery_energy = battery_capacity;
        curr_time = curr_time + wait_time;
        if (check_deadline())
          return false;
        continue;
      }
      double avail_time = driving_time_remaining(curr_time);
      double travel_time = std::min(avail_time, remaining_distance / speed);
      double irradiance = forecast_lut.get_value({curr_point.lat, curr_point.lon}, get_epoch());
      double net_power = car->energy_consumption(speed, angle, irradiance);
      battery_energy += net_power * travel_time;
      if (battery_energy > battery_capacity)
        battery_energy = battery_capacity;
      curr_time = curr_time + travel_time;
      remaining_distance -= speed * travel_time;
    }
    // If a control stop is scheduled at the next point, pause for 30 minutes with charging.
    if (control_stops.find(i + 1) != control_stops.end()) {
      double stop_duration = 30 * 60;
      double irradiance = forecast_lut.get_value({points[i + 1].lat, points[i + 1].lon}, get_epoch());
      double stationary_net_power = irradiance * array_area * array_efficiency * battery_efficiency;
      battery_energy += stationary_net_power * stop_duration;
      if (battery_energy > battery_capacity)
        battery_energy = battery_capacity;
      curr_time = curr_time + stop_duration;
      if (check_deadline())
        return false;
    }
  }

  // Final output: if simulation finished before the deadline and battery never went negative.
  if (check_deadline() || battery_energy < 0) {
    std::cout << "Did not finish." << std::endl;
    return false;
  } else {
    double finish_time_sec = curr_time - day_one_start_time; // returns seconds.
    std::cout << "Finished in " << finish_time_sec << " seconds." << std::endl;
    return true;
  }
}
