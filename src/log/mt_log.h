/*
 * mt_log.h
 *
 *  Created on: Apr 2, 2019
 *      Author: jerry
 */

#ifndef SRC_MT_LOG_H_
#define SRC_MT_LOG_H_

#include "../include/spdlog/sinks/rotating_file_sink.h"
#include "../include/spdlog/sinks/stdout_color_sinks.h"
#include "../include/spdlog/spdlog.h"

#include <unistd.h>

using namespace std;

class MtLog {
 public:
  MtLog() { app_logger_ = NULL; }
  ~MtLog() {}

  void Initialize(string log_path) {
    log_file_full_path_name_ = log_path + "/log/";
    if (access(log_file_full_path_name_.c_str(), F_OK) != 0) {
      mkdir(log_file_full_path_name_.c_str(), S_IRWXG | S_IRWXU | S_IRWXO);
    }
    log_file_full_path_name_ += kAppLogFileName;
    this->LoggerRegistry();
  }
  bool IsReady() { return (app_logger_ != NULL); }
  void SetLogLevel(spdlog::level::level_enum log_level) {
    app_logger_->set_level(log_level);
  }
  spdlog::level::level_enum GetLogLevel() { return app_logger_->level(); }

  void OutputToFile() { app_logger_ = async_file_logger_; }
  void OutputToConsole() { app_logger_ = console_logger_; }

  std::shared_ptr<spdlog::logger> AppLogger() { return app_logger_; }

  void ShutDown(void) {
    spdlog::shutdown();  // Release all spdlog resources, and drop all loggers
                         // in the registry
  }

  template <typename... Args>
  void Trace(const char *fmt, const Args &... args) {
    app_logger_->trace(fmt, args...);
  }

  template <typename... Args>
  void Debug(const char *fmt, const Args &... args) {
    app_logger_->debug(fmt, args...);
  }

  template <typename... Args>
  void Info(const char *fmt, const Args &... args) {
    app_logger_->info(fmt, args...);
  }

  template <typename... Args>
  void Warn(const char *fmt, const Args &... args) {
    app_logger_->warn(fmt, args...);
  }

  template <typename... Args>
  void Error(const char *fmt, const Args &... args) {
    app_logger_->error(fmt, args...);
  }

  template <typename... Args>
  void Critical(const char *fmt, const Args &... args) {
    app_logger_->critical(fmt, args...);
  }

 private:
  std::shared_ptr<spdlog::logger> app_logger_;
  std::shared_ptr<spdlog::logger> console_logger_;
  std::shared_ptr<spdlog::logger> async_file_logger_;

  const size_t kMaxFileSizeBytes =
      1024 * 1024 * 8;  // max file size is 8M bytes
  const size_t kMaxNumFiles = 3;
  //	const size_t kQueueSize = std::pow(2, 16);

  const std::string kAppLogFileName = "dm8500_app.log";
  const std::string kLogPatternString = "<%g>,%#";
  std::string log_file_full_path_name_;

  void LoggerRegistry() {
    // Create a stdout logger
    console_logger_ = spdlog::stdout_color_mt("console");

    // Create a file rotating logger with kMaxFileSizeBytes and kMaxNumFiles
    // rotated files.
    async_file_logger_ = spdlog::rotating_logger_mt(
        "app log", log_file_full_path_name_, kMaxFileSizeBytes, kMaxNumFiles);

    app_logger_ = console_logger_;  // set default logger of application

    app_logger_->set_level(spdlog::level::info);  // set default log level
    // app_logger_->set_pattern(kLogPatternString);

    spdlog::flush_every(std::chrono::seconds(5));
  }
};

#endif /* SRC_MT_LOG_H_ */
