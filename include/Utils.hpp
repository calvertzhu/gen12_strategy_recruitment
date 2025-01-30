#include <stdexcept>
#include <sstream>
#include <string>
#include <ctime>
#include <iostream>

#pragma once

// Definition for an exception type. Equivalent to an assertion
// Usage: RUNTIME_EXCEPTION(condition, String to print if condition is false)
#define RUNTIME_EXCEPTION(cond, msg, ...)                           \
    do {                                                            \
        if (!(cond)) {                                              \
            std::ostringstream oss;                                 \
            oss << "(ERROR Encountered) => " << __func__            \
                << ", file " << __FILE__ << ", line " << __LINE__   \
                << ". Message: " << msg;                            \
            std::cout << oss.str() << std::endl;                    \
            exit(1);                                                \
        }                                                           \
    } while (false)

// Scientific constants
#define PI (3.14159265358979323846264338327950288)
#define DEGREES_IN_PI (180)
#define HOURS_TO_SECONDS (3.6e3)
#define MINUTES_TO_SECONDS 60.0
#define HOURS_TO_MINUTES 60.0
#define JOULES_TO_KWH (3.6e6)
#define MPS_TO_KPH (3.6)
#define GRAVITY_ACCELERATION (9.81)
#define KM_TO_M (1000.0)
inline double hours2secs(double hours) { return hours * HOURS_TO_SECONDS; }

#ifdef _WIN32
  #define GMTIME_SAFE(time_t_ptr, tm_ptr) gmtime_s(tm_ptr, time_t_ptr)
  #define LOCALTIME_SAFE(time_t_ptr, tm_ptr) localtime_s(tm_ptr, time_t_ptr)
  #define ASCTIME_SAFE(char_ptr, tm_ptr) asctime_s(char_ptr, 26, tm_ptr)
  #define TIMEGM(tm_ptr) _mkgmtime(tm_ptr)
#else
  #define GMTIME_SAFE(time_t_ptr, tm_ptr) gmtime_r(time_t_ptr, tm_ptr)
  #define LOCALTIME_SAFE(time_t_ptr, tm_ptr) localtime_r(time_t_ptr, tm_ptr)
  #define ASCTIME_SAFE(tm_ptr, char_ptr) asctime_r(tm_ptr, char_ptr)
  #define TIMEGM(tm_ptr) timegm(tm_ptr)
#endif

/* A forecast coordinate is represented only by latitude and longitude. These are used by dni.csv */
struct ForecastCoord {
  double lat;
  double lon;

  ForecastCoord(double latitude, double longitude) : lat(latitude), lon(longitude) {}
  ForecastCoord() : lat(0), lon(0) {}
};

/* Standard latitude/latitude/altitude coordinate */
struct Coord {
  double lat;
  double lon;
  double alt;

  Coord(double latitude, double longitude, double altitude) : lat(latitude), lon(longitude), alt(altitude) {}
  Coord() : lat(0), lon(0), alt(0) {}

  /* Conversion operator from type ForecastCoord to Coord by clearing the altitude value */
  explicit Coord(const struct ForecastCoord& fc) : lat(fc.lat), lon(fc.lon), alt(0.0) {}
};

/* Determine if a string can be represented by a double */
bool isDouble(std::string str);

/** @brief Calculate haversine distance between two lat/lon/alt points
 * point 1 -> point 2
 *
 * @param src_coord: point 1
 * @param dst_coord: point 2
 */
double get_distance(Coord src_coord, Coord dst_coord);

/** @brief Calculate haversine distance between two lat/lon points
 * point 1 -> point 2
 *
 * @param src_coord: point 1
 * @param dst_coord: point 2
 */
double get_forecast_coord_distance(ForecastCoord src_coord, ForecastCoord dst_coord);

/** @brief Get julian day from a utc time
 * 
 * Note: time_t is the number of seconds since epoch beginning
 * 
 * @param utc_time_point: UTC time relative to epoch
 */
double julian_day(time_t utc_time_point);

/*
Copyright(c) 2010, Darin Koblick
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met :

*Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Programed by Darin C.Koblick 2 / 17 / 2009
//
//              Darin C.Koblick 4 / 16 / 2013 Vectorized for Speed
//                                         Allow for MATLAB Datevec input in
//                                         addition to a UTC string.
//                                         Cleaned up comments and code to
//                                         avoid warnings in MATLAB editor.
//
//      Kevin Godden 9/1/2020      Ported from Matlab to C++, tried to change as little as possible.
//                                         this is a non-vectorised port.
//
//--------------------------------------------------------------------------
*/
void get_az_el(time_t utc_time_point, double Lat, double Lon, double Alt, double* Az, double* El);

// Define your other utility functions and/or types here
