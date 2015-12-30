#if !defined( __CLIENT_MANAGER__H__ )
#define __CLIENT_MANAGER__H__

#include "Connection.h"
#include "MsgStruct.h"

#include <string>
#include <sstream>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>
#include <memory>
#include <netdb.h>
#include <sched.h>
#include <stdlib.h>

#define HOSTNAME_MAX 4096

class ClientManager {
public:
    ClientManager();
    ~ClientManager();
    int Run(const char*);
private:
    int RegisterInCluster(const char*);
    int CheckTasksStatus();
    int LaunchTask(std::shared_ptr<Task>);
    int AllocateNewProcessID(ID*);

    std::shared_ptr<Connection> connection;
    std::string hostname;
    std::string IP;
    std::vector<std::shared_ptr<Task>> running_tasks;
};

#endif