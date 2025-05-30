#include "logger.h"

Logger::Logger() : show_prints(false) {}
Logger::Logger(const bool sp) : show_prints(sp) {}

Logger::~Logger(){}

void Logger::operator=(const Logger& l) {
    show_prints = l.show_prints;
}

void Logger::log_endl(const std::string& message) const {
    if (show_prints) {
        std::cout << message << std::endl;
    }
}
void Logger::log(const std::string& message) const {
    if (show_prints) {
        std::cout << message;
    }
}

void Logger::log_tab(const std::string& message) const {
    log(message);
    log("\t");
}
    
void Logger::set_show_prints(const bool sp) {
    show_prints = sp;
}
