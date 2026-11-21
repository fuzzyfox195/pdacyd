#pragma once

namespace PDACYD {
void pro_logger_set_sd_enabled(bool enabled);
void pro_log_info(const char *tag, const char *format, ...);
void pro_log_warn(const char *tag, const char *format, ...);
void pro_log_error(const char *tag, const char *format, ...);
}
