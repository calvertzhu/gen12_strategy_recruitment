/* Classes for two types of lookup tables:

   1. Regular lookup table. Table indexed by a certain row and column index starting from 0.
  
   3. Weather lookup table. The first two columns represent the latitude and longitude respectively, and the
   first row represents a series of timestamps. Index using a lat/lon key along with a timestep key in order to 
   obtain an irradiance or wind value.

 */

#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <filesystem>
#include <vector>

#include "Utils.hpp"

/* Base LUT */
template <typename T>
class BaseLut {
 protected:
  /* Relative path to LUT */
  std::filesystem::path lut_path;

  /* LUT stored as a matrix */
  std::vector<std::vector<T>> values;

  /* Dimensions of the LUT */
  size_t num_rows;
  size_t num_cols;

  virtual void load_LUT() = 0;

 public:
  /* Only stores the relative path to the LUT */
  explicit BaseLut(const std::filesystem::path path);
  BaseLut() {}

  /* We let the derived LUTs implement their own lookup functionality */
};

/* Represents a forecast lookup table of double values */
class ForecastLut : public BaseLut<double>{
 private:
  /* Coordinates used to index the lookup table */
  std::vector<ForecastCoord> forecast_coords;

  /* Timesteps used to index the lookup table as unix epoch times */
  std::vector<time_t> forecast_times;

  void load_LUT() override;

 public:
  /* Load a csv upon construction */
  explicit ForecastLut(const std::string path);

  /* Empty default constructor */
  ForecastLut() {}

  /* Get a certain value with lat/lon and unix time as keys. Uses the closest keys */
  double get_value(ForecastCoord coord, time_t time);

  /* Caches for faster accessing */
  int row_cache;
  int column_cache;

  /* Directly indexes the csv to return a value */
  double get_value_with_cache();

  /* Updates the index_cache struct using new keys */
  void update_index_cache(ForecastCoord coord, time_t time);

  /* Initialize the cache variables */
  void initialize_caches(ForecastCoord coord, time_t time);
  void initialize_caches(Coord coord, time_t time);
};

class Route {
 private:
  /* Points of the route */
  std::vector<Coord> route_points;
 public:
  /* Read a CSV with columns |latitude|longitude|altitude(m)| */
  explicit Route(const std::string route_path);

  /* Empty default constructor */
  Route() {}

  inline std::vector<Coord> get_route_points() const { return route_points; }
};
