#include <memory>
#include <vector>
#include <unordered_set>
#include <limits>
#include <utility>

#include "Sim.hpp"
#include "Utils.hpp"

bool Simulator::run_sim(const double speed) {
  // Write your implementation here
  return false;
}

Simulator::Simulator(std::shared_ptr<Car> model, const Coord starting_coord,
                     const Time starting_time) : car(model), starting_coord(starting_coord),
                                                 curr_time(starting_time) {}
