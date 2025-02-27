#include "Luts.hpp"
#include "date.h"
#include <fstream>

template <typename T>
BaseLut<T>::BaseLut(const std::filesystem::path path) {
  lut_path = path;
}

ForecastLut::ForecastLut(const std::string path) :
  BaseLut<double>(std::filesystem::path(path)) {
    std::cout << "Csv: " << lut_path.string() << std::endl;
    load_LUT();
}

void ForecastLut::load_LUT() {
  std::fstream file(lut_path);
  RUNTIME_EXCEPTION(file.is_open(), "Forecast file not found " + lut_path.string());
  std::string times_line;
  file >> times_line;
  std::stringstream times_stream(times_line);

  // Remove 'latitude' and 'longitude' from first 2 cols of csv input.
  std::string time;
  std::getline(times_stream, time, ',');
  std::getline(times_stream, time, ',');

  /* Create an array of the time keys */
  while (!times_stream.eof()) {
    std::getline(times_stream, time, ',');
    RUNTIME_EXCEPTION(isDouble(time), "Time " + time + " is not a number in ForecastLUT " + lut_path.string());
    uint64_t temp_time = std::stoull(time);
    int seconds = temp_time % 100;
    temp_time /= 100;
    int minutes = temp_time % 100;
    temp_time /= 100;
    int hours = temp_time % 100;
    temp_time /= 100;
    int days = temp_time % 100;
    temp_time /= 100;
    int month = (temp_time % 100);
    temp_time /= 100;
    int year = temp_time;

    /* Construct YYYY-MM-DD HH:MM:SS string */
    std::string forecast_time = "20" + std::to_string(year) + "-" + std::to_string(month) + "-"
      + std::to_string(days) + " " + std::to_string(hours) + ":" + std::to_string(minutes) + ":"
      + std::to_string(seconds);

    std::istringstream iss(forecast_time);
    date::sys_time<std::chrono::seconds> epoch_time;
    iss >> date::parse("%F %T", epoch_time);
    time_t local_time_t = std::chrono::system_clock::to_time_t(epoch_time);

    forecast_times.push_back(local_time_t);
  }

  int row_counter = 0;
  while (!file.eof()) {
    std::string file_line;
    file >> file_line;
    std::stringstream file_linestream(file_line);
    if (file_linestream.str().empty()) break;

    std::string cell;
    ForecastCoord coord{};
    std::getline(file_linestream, cell, ',');
    RUNTIME_EXCEPTION(isDouble(cell), "Value " + cell + " is not a number in Forecast LUT " + lut_path.string());
    coord.lat = std::stod(cell);

    std::getline(file_linestream, cell, ',');
    RUNTIME_EXCEPTION(isDouble(cell), "Value " + cell + " is not a number in Efficiency LUT " + lut_path.string());
    coord.lon = std::stod(cell);

    forecast_coords.emplace_back(coord);

    std::getline(file_linestream, cell, ',');
    RUNTIME_EXCEPTION(isDouble(cell), "Value " + cell + " is not a number in Efficiency LUT " + lut_path.string());
    double value = std::stod(cell);
    std::vector<double> inner_vector;
    inner_vector.emplace_back(value);
    this->values.push_back(inner_vector);

    int column_counter = 0;
    while (!file_linestream.eof()) {
      std::getline(file_linestream, cell, ',');
      RUNTIME_EXCEPTION(isDouble(cell), "Value " + cell + " is not a number in Efficiency LUT " + lut_path.string());
      this->values[row_counter].push_back(std::stod(cell));
      column_counter++;
    }

    row_counter++;
  }

  this->num_rows = forecast_coords.size();
  this->num_cols = forecast_times.size();

  row_cache = 0;
  column_cache = 0;
}

double ForecastLut::get_value(ForecastCoord coord, time_t time) {
  double row_key;
  double col_key;

  double min_distance = std::numeric_limits<double>::max();
  for (size_t row=0; row < num_rows; row++) {
    ForecastCoord forecast_coord = forecast_coords[row];
    double distance = get_forecast_coord_distance(coord, forecast_coord);
    if (distance < min_distance) {
      min_distance = distance;
      row_key = row;
    }
  }

  double min_time = std::numeric_limits<double>::max();
  for (size_t col=0; col < num_cols; col++) {
    uint64_t forecast_time = forecast_times[col];
    int time_diff = time - forecast_time;
    if (std::abs(static_cast<double>(time_diff)) < min_time) {
      min_time = std::abs(static_cast<double>(time_diff));
      col_key = col;
    }
  }

  RUNTIME_EXCEPTION(row_key >= 0 && row_key < num_rows && col_key >= 0 && col_key < num_cols,
                    "Out of bounds access in Forecast LUT " + lut_path.string());
  return this->values[row_key][col_key];
}

void ForecastLut::initialize_caches(ForecastCoord coord, time_t time) {
  /* Initialize row cache */
  Coord forecast_coord_as_coord = Coord(coord);
  double min_distance = std::numeric_limits<double>::max();
  for (size_t i = 0; i < num_rows; i++) {
    Coord forecast_coord = Coord(forecast_coords[i]);
    double distance = get_distance(forecast_coord, forecast_coord_as_coord);
    if (distance < min_distance) {
      min_distance = distance;
      row_cache = i;
    }
  }

  /* Initialize column cache */
  double min_time = std::numeric_limits<double>::max();
  for (size_t i=0; i < num_cols; i++) {
    time_t forecast_time = forecast_times[i];
    int time_diff = time - forecast_time;
    if (std::abs(static_cast<double>(time_diff)) < min_time) {
      min_time = std::abs(static_cast<double>(time_diff));
      column_cache = i;
    }
  }
}

void ForecastLut::initialize_caches(Coord coord, time_t time) {
  /* Initialize row cache */
  double min_distance = std::numeric_limits<double>::max();
  for (size_t i = 0; i < num_rows; i++) {
    Coord forecast_coord = Coord(forecast_coords[i]);
    double distance = get_distance(forecast_coord, coord);
    if (distance < min_distance) {
      min_distance = distance;
      row_cache = i;
    }
  }

  /* Initialize column cache */
  double min_time = std::numeric_limits<double>::max();
  for (size_t i=0; i < num_cols; i++) {
    time_t forecast_time = forecast_times[i];
    int time_diff = time - forecast_time;
    if (std::abs(static_cast<double>(time_diff)) < min_time) {
      min_time = std::abs(static_cast<double>(time_diff));
      column_cache = i;
    }
  }
}

/* Begins searching from the specified indices */
void ForecastLut::update_index_cache(ForecastCoord coord, time_t time) {
  if (row_cache < num_rows-1) {
    ForecastCoord next_coord = forecast_coords[row_cache+1];
    ForecastCoord current_coord = forecast_coords[row_cache];

    double dist_from_current_coord = get_forecast_coord_distance(coord, current_coord);
    double dist_from_next_coord = get_forecast_coord_distance(coord, next_coord);

    row_cache = dist_from_current_coord <= dist_from_next_coord ? row_cache : row_cache+1;
  }
  if (column_cache < num_cols-1) {
    uint64_t current_time = forecast_times[column_cache];
    uint64_t next_time = forecast_times[column_cache+1];

    uint64_t diff_time_from_current = abs(static_cast<double>(time - current_time));
    uint64_t diff_time_from_next = abs(static_cast<double>(time - next_time));

    column_cache = diff_time_from_current <= diff_time_from_next ? column_cache : column_cache+1;
  }
}

double ForecastLut::get_value_with_cache() {
  return this->values[row_cache][column_cache];
}

Route::Route(const std::string lut_path) {
  const std::filesystem::path route_path(lut_path);
  std::fstream base_route(route_path);
  RUNTIME_EXCEPTION(base_route.is_open(), "Base route file not found " + route_path.string());

  Coord last_coord;

  // Read and parse the file
  while (!base_route.eof()) {
    std::string line;
    base_route >> line;
    std::stringstream linestream(line);

    while (!linestream.eof() && !linestream.str().empty()) {
      std::string cell;
      Coord coord{};

      std::getline(linestream, cell, ',');
      RUNTIME_EXCEPTION(isDouble(cell), "Value " + cell + " in route file " + route_path.string() + " is not a number");
      coord.lat = std::stod(cell);

      std::getline(linestream, cell, ',');
      RUNTIME_EXCEPTION(isDouble(cell), "Value " + cell + " in route file " + route_path.string() + " is not a number");
      coord.lon = std::stod(cell);

      std::getline(linestream, cell, ',');
      RUNTIME_EXCEPTION(isDouble(cell), "Value " + cell + " in route file " + route_path.string() + " is not a number");
      coord.alt = std::stod(cell);

      route_points.emplace_back(coord);
    }
  }
}

