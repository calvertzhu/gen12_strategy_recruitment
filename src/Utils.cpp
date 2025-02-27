#include "Utils.hpp"
#include <cmath>

bool isDouble(std::string str) {
	if (str[0] == '-' && str.size() >= 2) {
		return isdigit(str[1]);
	}
	return isdigit(str[0]);
}

double get_distance(Coord src_coord, Coord dst_coord) {
  constexpr double R = 6371e3;

  double phi_1 = src_coord.lat * PI/180;
  double phi_2 = dst_coord.lat * PI/180;
  double delPhi = (dst_coord.lat-src_coord.lat) * PI/180;
  double delLambda = (dst_coord.lon-src_coord.lon) * PI/180;

  double a =  (sin(delPhi/2) * sin(delPhi/2)) + (cos(phi_1) * cos(phi_2) * sin(delLambda/2) * sin(delLambda/2));
  double c = 2 * atan2(sqrt(a), sqrt(1-a));

  // Haversine distance in m
  double dist_km = (R * c);

  // calculate altitude difference
  double alt_1 = src_coord.alt;
  double alt_2 = dst_coord.alt;

  double alt_difference = abs(alt_1-alt_2);

  // Calculate true distance with haversine
  double true_distance = sqrt(dist_km * dist_km + alt_difference * alt_difference);

  return true_distance;
}

/* Gets distance between two lat/lon coordinates */
double get_forecast_coord_distance(ForecastCoord src_coord, ForecastCoord dst_coord) {
  constexpr double R = 6371e3;

  double phi_1 = src_coord.lat * PI/180;
  double phi_2 = dst_coord.lat * PI/180;
  double delPhi = (dst_coord.lat-src_coord.lat) * PI/180;
  double delLambda = (dst_coord.lon-src_coord.lon) * PI/180;

  double a =  (sin(delPhi/2) * sin(delPhi/2) ) + (cos(phi_1) * cos(phi_2) * sin(delLambda/2) * sin(delLambda/2));
  double c = 2 * atan2(sqrt(a), sqrt(1-a));

  // Haversine distance in km
  double dist_km = (R * c)/1000;

  double alt_difference = 0.0;

  // Calculate true distance with haversine
  double true_distance = sqrt(dist_km * dist_km + alt_difference * alt_difference) * 1000.0;

  return true_distance;
}

double julian_day(time_t utc_time_point) {
  // Extract UTC Time
  struct tm tm;
  LOCALTIME_SAFE(&utc_time_point, &tm);
  double year = tm.tm_year + 1900;
  double month = tm.tm_mon + 1;
  double day = tm.tm_mday;
  double hour = tm.tm_hour;
  double min = tm.tm_min;
  double sec = tm.tm_sec;

  if (month <= 2) {
    year -= 1;
    month += 12;
  }

  double jd = floor(365.25*(year + 4716.0)) + floor(30.6001*(month + 1.0)) + 2.0 -
              floor(year / 100.0) + floor(floor(year / 100.0) / 4.0) + day - 1524.5 +
              (hour + min / 60 + sec / 3600) / 24;

  return jd;
}


void get_az_el(time_t utc_time_point, double Lat, double Lon, double Alt, double* Az, double* El) {
  double jd = julian_day(utc_time_point);

  double d = jd - 2451543.5;

  // Keplerian Elements for the Sun(geocentric)
  double w = 282.9404 + 4.70935e-5*d;  // (longitude of perihelion degrees)
  // a = 1.000000; % (mean distance, a.u.)
  double e = 0.016709 - 1.151e-9*d;  // (eccentricity)
  double M = fmod(356.0470 + 0.9856002585*d, 360.0);  // (mean anomaly degrees)

  double L = w + M;  // (Sun's mean longitude degrees)

  double oblecl = 23.4393 - 3.563e-7*d;  // (Sun's obliquity of the ecliptic)

  // auxiliary angle
  double  E = M + (180 / PI)*e*sin(M*(PI / 180))*(1 + e*cos(M*(PI / 180)));

  // rectangular coordinates in the plane of the ecliptic(x axis toward perhilion)
  double x = cos(E*(PI / 180)) - e;
  double y = sin(E*(PI / 180))*sqrt(1 - pow(e, 2));

  // find the distance and true anomaly
  double r = sqrt(pow(x, 2) + pow(y, 2));
  double v = atan2(y, x)*(180 / PI);

  // find the longitude of the sun
  double lon = v + w;

  // compute the ecliptic rectangular coordinates
  double xeclip = r*cos(lon*(PI / 180));
  double yeclip = r*sin(lon*(PI / 180));
  double zeclip = 0.0;
  // rotate these coordinates to equitorial rectangular coordinates
  double xequat = xeclip;

  double yequat = yeclip*cos(oblecl*(PI / 180)) + zeclip * sin(oblecl*(PI / 180));

  double zequat = yeclip*sin(23.4406*(PI / 180)) + zeclip * cos(oblecl*(PI / 180));
  // convert equatorial rectangular coordinates to RA and Decl:
  r = sqrt(pow(xequat, 2) + pow(yequat, 2) + pow(zequat, 2)) - (Alt / 149598000);  // roll up the altitude correction
  double RA = atan2(yequat, xequat)*(180 / PI);

  double delta = asin(zequat / r)*(180 / PI);

  // Following the RA DEC to Az Alt conversion sequence explained here :
  // http ://www.stargazing.net/kepler/altaz.html
  //  Find the J2000 value
  //  J2000 = jd - 2451545.0;
  // hourvec = datevec(UTC);
  // UTH = hourvec(:, 4) + hourvec(:, 5) / 60 + hourvec(:, 6) / 3600;

  // Get UTC representation of time / C++ Specific
  tm tm_obj;
  tm *ptm = &tm_obj;
  GMTIME_SAFE(&utc_time_point, ptm);
  double UTH = static_cast<double>(ptm->tm_hour) + static_cast<double>(ptm->tm_min) / 60 +
               static_cast<double>(ptm->tm_sec / 3600);

  // Calculate local siderial time
  double GMST0 = fmod(L + 180, 360.0) / 15;

  double SIDTIME = GMST0 + UTH + Lon / 15;

  // Replace RA with hour angle HA
  double HA = (SIDTIME*15 - RA);

  // convert to rectangular coordinate system
  x = cos(HA*(PI / 180))*cos(delta*(PI / 180));

  y = sin(HA*(PI / 180))*cos(delta*(PI / 180));
  double z = sin(delta*(PI / 180));

  // rotate this along an axis going east - west.
  double xhor = x*cos((90 - Lat)*(PI / 180)) - z*sin((90 - Lat)*(PI / 180));

  double yhor = y;
  double zhor = x*sin((90 - Lat)*(PI / 180)) + z*cos((90 - Lat)*(PI / 180));

  // Find the h and AZ
  *Az = atan2(yhor, xhor)*(180 / PI) + 180;
  *El = asin(zhor)*(180 / PI);
}
