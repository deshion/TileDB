/**
 * @file   logger.h
 *
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2017-2020 TileDB, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * @section DESCRIPTION
 *
 * This file defines class Logger.
 */

#pragma once

#ifndef TILEDB_LOGGER_H
#define TILEDB_LOGGER_H

#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>
#include <iostream>

#include "tiledb/sm/misc/status.h"

namespace tiledb {
namespace sm {

//extern std::mutex logger_sink_create_mutex_;
/** Definition of class Logger. */
class Logger {
  /** Verbosity level. */
  enum class Level : char {
    VERBOSE,
    ERROR,
  };

 public:
  /* ********************************* */
  /*     CONSTRUCTORS & DESTRUCTORS    */
  /* ********************************* */

  /** Constructor. */
  explicit Logger(const std::string& prefix = "");
  explicit Logger(const char* prefix);

  /** Destructor. */
  ~Logger();

  /* ********************************* */
  /*                API                */
  /* ********************************* */

  /**
   * Log a debug statement with no message formatting.
   *
   * @param msg The string to log.
   */
  void debug(const char* msg) {
//    std::lock_guard<std::mutex> lock(logger_sink_create_mutex_);
//    logger_->debug(msg);
      std::cerr << prefix_ << " " << msg << std::endl;
  }

  /**
   * A formatted debug statment.
   *
   * @param fmt A fmtlib format string, see http://fmtlib.net/latest/ for
   *     details.
   * @param arg positional argument to format.
   * @param args optional additional positional arguments to format.
   * @return void
   */
  template <typename Arg1, typename... Args>
  void debug(const char* fmt, const Arg1& arg1, const Args&... args) {
//    std::lock_guard<std::mutex> lock(logger_sink_create_mutex_);
//    logger_->debug(fmt, arg1, args...);
//    std::cerr << prefix_ << " " << msg << std::endl;
    std::string s = prefix_ + " " + fmt + "\n";
    fprintf(std::cerr, s.c_str(), arg1, args...);
  }

  /**
   * Log an error with no message formatting.
   *
   * @param msg The string to log
   * */
  void error(const char* msg) {
//    std::lock_guard<std::mutex> lock(logger_sink_create_mutex_);
//    logger_->error(msg);
    std::cerr << prefix_ << " " << msg << std::endl;
  }

  /** A formatted error statement.
   *
   * @param fmt A fmtlib format string, see http://fmtlib.net/latest/ for
   * details.
   * @param arg1 positional argument to format.
   * @param args optional additional positional arguments to format.
   */
  template <typename Arg1, typename... Args>
  void error(const char* fmt, const Arg1& arg1, const Args&... args) {
//    std::lock_guard<std::mutex> lock(logger_sink_create_mutex_);
//    sprintf(fmt, arg1, args...);
//    std::cerr << prefix_ << " " << msg << std::endl;
    std::string s = prefix_ + " " + fmt + "\n";
    fprintf(std::cerr, s.c_str(), arg1, args...);
  }

  /**
   * Set the logger level.
   *
   * @param lvl Logger::Level VERBOSE logs debug statements, ERROR only logs
   *    Status Error's.
   */
  void set_level(Logger::Level lvl) {
//    std::lock_guard<std::mutex> lock(logger_sink_create_mutex_);
//    switch (lvl) {
//      case Logger::Level::VERBOSE:
//        logger_->set_level(spdlog::level::debug);
//        break;
//      case Logger::Level::ERROR:
//        logger_->set_level(spdlog::level::err);
//        break;
//    }
    lvl_ = lvl;
  }

  /**
   * Returns whether the logger should log a message given the currently set
   * log level.
   *
   * @param lvl The Logger::Level to test
   * @return bool true, if the logger will log the given Logger::Level, false
   *     otherwise.
   */
  bool should_log(Logger::Level lvl) {
    return lvl >= lvl_;
//    std::lock_guard<std::mutex> lock(logger_sink_create_mutex_);
//    switch (lvl) {
//      case Logger::Level::VERBOSE:
//        return logger_->should_log(spdlog::level::debug);
//      case Logger::Level::ERROR:
//        return logger_->should_log(spdlog::level::err);
//    }
  }

 private:
  /* ********************************* */
  /*         PRIVATE ATTRIBUTES        */
  /* ********************************* */

  /** The logger object. */
//  std::shared_ptr<spdlog::logger> logger_;
  Logger::Level lvl_;

  std::string prefix_;
};

/* ********************************* */
/*              GLOBAL               */
/* ********************************* */

/** Global logger function. */
Logger& global_logger();

#ifdef TILEDB_VERBOSE
/** Logs an error. */
inline void LOG_ERROR(const std::string& msg) {
//  std::lock_guard<std::mutex> lock(logger_sink_create_mutex_);
  global_logger().error(msg.c_str());
}

/** Logs a status. */
inline Status LOG_STATUS(const Status& st) {
//  std::lock_guard<std::mutex> lock(logger_sink_create_mutex_);
  global_logger().error(st.to_string().c_str());
  return st;
}
#else
/** Logs an error. */
inline void LOG_ERROR(const std::string& msg) {
//  std::lock_guard<std::mutex> lock(logger_sink_create_mutex_);
  (void)msg;
  return;
}

/** Logs a status. */
inline Status LOG_STATUS(const Status& st) {
//  std::lock_guard<std::mutex> lock(logger_sink_create_mutex_);
  return st;
}
#endif

/** Logs an error and exits with a non-zero status. */
inline void LOG_FATAL(const std::string& msg) {
//  std::lock_guard<std::mutex> lock(logger_sink_create_mutex_);
  global_logger().error(msg.c_str());
  exit(1);
}

}  // namespace sm
}  // namespace tiledb

#endif  // TILEDB_LOGGER_H
