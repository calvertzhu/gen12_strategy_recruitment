#include <string>
#include <cstdint>
#include <cmath>
#include <ctime>
#include <chrono>
#include <iomanip>
#include <sstream>

#include "CustomTime.hpp"
#include "Utils.hpp"

Time::Time(std::string local_time_point, double utc_adjustment) {
  local_time = local_time_point;
  std::istringstream iss(local_time_point);
  std::string date_str, time_str;
  iss >> date_str >> time_str;
  if (time_str.empty()) {
    /* The timestamp is in HH:MM:SS format, call the other constructor */
    HH_MM_SS_constructor(local_time_point);
    return;
  }

  std::istringstream rss(local_time_point);
  date::sys_time<std::chrono::seconds> epoch_time;
  rss >> date::parse("%F %T", epoch_time);

  // Convert sys_time to time_t
  time_t local_time_t = std::chrono::system_clock::to_time_t(epoch_time);

  t_datetime_local = local_time_t;

  this->utc_adjustment = hours2secs(utc_adjustment);
  t_datetime_utc = t_datetime_local + this->utc_adjustment;


  GMTIME_SAFE(&t_datetime_local, &m_datetime_local);
  GMTIME_SAFE(&t_datetime_utc, &m_datetime_utc);

  m_milliseconds = 0;

  hh_mm_ss_only = false;
  return;
}

Time::Time(std::string local_time_point) {
  HH_MM_SS_constructor(local_time_point);
}

void Time::HH_MM_SS_constructor(const std::string local_time_point) {
  std::istringstream iss(local_time_point);
  int hours, minutes, seconds;

  char delimiter;
  iss >> hours >> delimiter >> minutes >> delimiter >> seconds;

  time_t now = time(nullptr);
  LOCALTIME_SAFE(&now, &m_datetime_local);
  m_datetime_local.tm_hour = hours;
  m_datetime_local.tm_min = minutes;
  m_datetime_local.tm_sec = seconds;
  m_milliseconds = 0;

  hh_mm_ss_only = true;
  return;
}

void Time::copy_hh_mm_ss(const Time& other, bool copy_milliseconds) {
  RUNTIME_EXCEPTION(!this->hh_mm_ss_only, "Cannot copy hh:mm:ss data into existing HH:MM:SS only timestamp");
  this->m_datetime_local.tm_hour = other.m_datetime_local.tm_hour;
  this->m_datetime_local.tm_min = other.m_datetime_local.tm_min;
  this->m_datetime_local.tm_sec = other.m_datetime_local.tm_sec;
  if (copy_milliseconds) {
    this->m_milliseconds = other.m_milliseconds;
  }

  this->t_datetime_local = TIMEGM(&this->m_datetime_local);
  this->t_datetime_utc = this->t_datetime_local + this->utc_adjustment;

  GMTIME_SAFE(&this->t_datetime_utc, &this->m_datetime_utc);
}

/* Compare each member of the tm struct */
bool Time::operator>(const Time& other) const {
  if (!this->hh_mm_ss_only && !other.hh_mm_ss_only) {
    return this->t_datetime_local > other.t_datetime_local;
  }

  if (this->m_datetime_local.tm_hour > other.m_datetime_local.tm_hour) {
    return true;
  } else if (this->m_datetime_local.tm_hour < other.m_datetime_local.tm_hour) {
    return false;
  }

  if (this->m_datetime_local.tm_min > other.m_datetime_local.tm_min) {
    return true;
  } else if (this->m_datetime_local.tm_min < other.m_datetime_local.tm_min) {
    return false;
  }

  if (this->m_datetime_local.tm_sec > other.m_datetime_local.tm_sec) {
    return true;
  } else if (this->m_datetime_local.tm_sec < other.m_datetime_local.tm_sec) {
    return false;
  }

  if (this->m_milliseconds > other.m_milliseconds) {
    return true;
  } else if (this->m_milliseconds < other.m_milliseconds) {
    return false;
  }

  return false;
}

/* Compare each member of the tm struct */
bool Time::operator>=(const Time& other) const {
  if (!this->hh_mm_ss_only && !other.hh_mm_ss_only) {
    return this->t_datetime_local >= other.t_datetime_local;
  }

  if (this->m_datetime_local.tm_hour > other.m_datetime_local.tm_hour) {
    return true;
  } else if (this->m_datetime_local.tm_hour < other.m_datetime_local.tm_hour) {
    return false;
  }

  if (this->m_datetime_local.tm_min > other.m_datetime_local.tm_min) {
    return true;
  } else if (this->m_datetime_local.tm_min < other.m_datetime_local.tm_min) {
    return false;
  }

  if (this->m_datetime_local.tm_sec > other.m_datetime_local.tm_sec) {
    return true;
  } else if (this->m_datetime_local.tm_sec < other.m_datetime_local.tm_sec) {
    return false;
  }

  if (this->m_milliseconds > other.m_milliseconds) {
    return true;
  } else if (this->m_milliseconds < other.m_milliseconds) {
    return false;
  }

  return true;
}


bool Time::operator<(const Time& other) const {
  if (!this->hh_mm_ss_only && !other.hh_mm_ss_only) {
    return this->t_datetime_local < other.t_datetime_local;
  }

  if (this->m_datetime_local.tm_hour < other.m_datetime_local.tm_hour) {
    return true;
  } else if (this->m_datetime_local.tm_hour > other.m_datetime_local.tm_hour) {
    return false;
  }

  if (this->m_datetime_local.tm_min < other.m_datetime_local.tm_min) {
    return true;
  } else if (this->m_datetime_local.tm_min > other.m_datetime_local.tm_min) {
    return false;
  }

  if (this->m_datetime_local.tm_sec < other.m_datetime_local.tm_sec) {
    return true;
  } else if (this->m_datetime_local.tm_sec > other.m_datetime_local.tm_sec) {
    return false;
  }
  if (this->m_milliseconds < other.m_milliseconds) {
    return true;
  } else if (this->m_milliseconds > other.m_milliseconds) {
    return false;
  }

  return false;
}

bool Time::operator<=(const Time& other) const {
  if (!this->hh_mm_ss_only && !other.hh_mm_ss_only) {
    return this->t_datetime_local <= other.t_datetime_local;
  }

  if (this->m_datetime_local.tm_hour < other.m_datetime_local.tm_hour) {
    return true;
  } else if (this->m_datetime_local.tm_hour > other.m_datetime_local.tm_hour) {
    return false;
  }

  if (this->m_datetime_local.tm_min < other.m_datetime_local.tm_min) {
    return true;
  } else if (this->m_datetime_local.tm_min > other.m_datetime_local.tm_min) {
    return false;
  }

  if (this->m_datetime_local.tm_sec < other.m_datetime_local.tm_sec) {
    return true;
  } else if (this->m_datetime_local.tm_sec > other.m_datetime_local.tm_sec) {
    return false;
  }
  if (this->m_milliseconds < other.m_milliseconds) {
    return true;
  } else if (this->m_milliseconds > other.m_milliseconds) {
    return false;
  }

  return true;
}

std::string Time::get_local_readable_time() const {
  std::ostringstream oss;

  if (hh_mm_ss_only) {
      oss << std::setw(2) << std::setfill('0') << m_datetime_local.tm_hour << ":"
          << std::setw(2) << std::setfill('0') << m_datetime_local.tm_min << ":"
          << std::setw(2) << std::setfill('0') << m_datetime_local.tm_sec;
      return oss.str();
  }

  oss << std::setw(4) << std::setfill('0') << (m_datetime_local.tm_year + 1900) << "-"
      << std::setw(2) << std::setfill('0') << (m_datetime_local.tm_mon + 1) << "-"
      << std::setw(2) << std::setfill('0') << m_datetime_local.tm_mday << " "
      << std::setw(2) << std::setfill('0') << m_datetime_local.tm_hour << ":"
      << std::setw(2) << std::setfill('0') << m_datetime_local.tm_min << ":"
      << std::setw(2) << std::setfill('0') << m_datetime_local.tm_sec << "."
      << std::setw(3) << std::setfill('0') << m_milliseconds;

  return oss.str();
}

void Time::update_time_seconds(const double seconds) {
  int64_t total_milliseconds = static_cast<int64_t>(seconds * 1000) + m_milliseconds;

  t_datetime_local += total_milliseconds / 1000;
  t_datetime_utc += total_milliseconds / 1000;

  GMTIME_SAFE(&t_datetime_local, &m_datetime_local);
  GMTIME_SAFE(&t_datetime_utc, &m_datetime_utc);
  m_milliseconds = total_milliseconds % 1000;
}

Time Time::operator+(const double seconds) const {
  Time new_time(*this);
  new_time.update_time_seconds(seconds);
  return new_time;
}

double Time::operator-(const Time& other) const {
  int64_t milliseconds1;
  int64_t milliseconds2;
  if (this->hh_mm_ss_only || other.hh_mm_ss_only) {
      milliseconds1 = this->m_datetime_local.tm_hour * 3600000 +
                      this->m_datetime_local.tm_min * 60000 +
                      this->m_datetime_local.tm_sec * 1000 +
                      this->m_milliseconds;
      milliseconds2 = other.m_datetime_local.tm_hour * 3600000 +
                      other.m_datetime_local.tm_min * 60000 +
                      other.m_datetime_local.tm_sec * 1000 +
                      other.m_milliseconds;
  } else {
      milliseconds1 = this->t_datetime_local * 1000 + this->m_milliseconds;
      milliseconds2 = other.t_datetime_local * 1000 + other.m_milliseconds;
  }

  return static_cast<double>((milliseconds1 - milliseconds2) / 1000.0);
}

std::string Time::get_utc_readable_time() const {
  std::ostringstream oss;

  if (hh_mm_ss_only) {
      oss << std::setw(2) << std::setfill('0') << m_datetime_utc.tm_hour << ":"
          << std::setw(2) << std::setfill('0') << m_datetime_utc.tm_min << ":"
          << std::setw(2) << std::setfill('0') << m_datetime_utc.tm_sec;
      return oss.str();
  }

  oss << std::setw(4) << std::setfill('0') << (m_datetime_utc.tm_year + 1900) << "-"
      << std::setw(2) << std::setfill('0') << (m_datetime_utc.tm_mon + 1) << "-"
      << std::setw(2) << std::setfill('0') << m_datetime_utc.tm_mday << " "
      << std::setw(2) << std::setfill('0') << m_datetime_utc.tm_hour << ":"
      << std::setw(2) << std::setfill('0') << m_datetime_utc.tm_min << ":"
      << std::setw(2) << std::setfill('0') << m_datetime_utc.tm_sec << "."
      << std::setw(3) << std::setfill('0') << m_milliseconds;

  return oss.str();
}
