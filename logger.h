#include <iostream>

#ifndef LOGGER_H
#define LOGGER_H

class Logger {
private:
    bool show_prints;
public:
    Logger();
    Logger(const bool sp);

    void log_endl(const std::string& message) const;

    void log(const std::string& message) const;

    void log_tab(const std::string& message) const;

    void set_show_prints(const bool sp);
};

#endif 