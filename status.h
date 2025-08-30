#ifndef STATUS_H
#define STATUS_H
enum Status {
    FREE = -1,
    FIX_OUT = 0,
    FIX_IN = 1

};


inline std::ostream& operator<<(std::ostream& os, Status e) {
    switch (e) {
    case FIX_IN:  return os << "FIX_IN";
    case FIX_OUT: return os << "FIX_OUT";
    case FREE: return os << "FREE";
    default:      return os << "UNKNOWN";
    }
}
#endif 


