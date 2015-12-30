#if !defined( __CONNECTION_MANAGER__H__ )
#define __CONNECTION_MANAGER__H__

class ClusterManager;
class ConnectionManager;
class ClusterShell;

#include "Connection.h"
#include "MsgStruct.h"
#include "ClusterManager.h"

#include <queue>
#include <pthread.h>
#include <string.h>
#include <memory>
#include <sched.h>

class ConnectionManager {
public:
    ConnectionManager(std::shared_ptr<Connection>, ID, ClusterManager*);
    ~ConnectionManager();
    int Run();
    int AddTask(std::shared_ptr<Task>);
    int GetInfo(std::string &name_and_tasks);
    int NumberOfTasks();
private:
    int CheckConnection();
    int CloseConnection();
    int DeleteTask(int taskID);
	static void* running_in_another_thread(void*);
	int RegisterNode();

    pthread_mutex_t tasksMutex;
    std::queue<std::shared_ptr<Task>> waiting_tasks;
    std::queue<std::shared_ptr<Task>> running_tasks;
    std::shared_ptr<Connection> connection;
    ID connectionID;
    ClusterManager *pClusterManager;
    std::string hostname;
    std::string IP;
    pthread_t tid;
};

#endif