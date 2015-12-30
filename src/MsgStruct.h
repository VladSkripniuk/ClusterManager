#if !defined( __MSGSTRUCT__H__ )
#define __MSGSTRUCT__H__

#include <string>
#include <sys/types.h>
#include <time.h>

typedef int ID;

struct Task {
    std::string filename;
    std::string args;
    ID taskID;
    pid_t pid;
    ID executor;
};

struct Report {
    ID taskID;
    std::string report;
};

struct Info {
    std::string hostname;
    std::string IP;
};

struct Check {
    time_t t;
};

#endif