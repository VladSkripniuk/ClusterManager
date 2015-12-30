#if !defined( __CLUSTER_MANAGER__H__ )
#define __CLUSTER_MANAGER__H__

class ClusterManager;
class ConnectionManager;
class ClusterShell;

#include "Listener.h"
#include "ConnectionManager.h"
#include "ClusterShell.h"

#include <sstream>
#include <vector>
#include <memory>
#include <sched.h>

class ClusterManager {
public:
    ClusterManager();
    ~ClusterManager();
    int Run();
    int AddTask(std::shared_ptr<Task> pTask);
    int SendToShellReport(std::shared_ptr<Report> pReport);
    int AllocateNewTaskID(ID*);
    int GetInfo(std::string &arg);
    int DeleteConnectionAndRearrangeTasks(ID, std::vector<std::shared_ptr<Task>>);
private:
	int GetExecutorForTask(std::shared_ptr<ConnectionManager>&);
	int AllocateNewExecutorID(ID*);
	
	std::vector<std::shared_ptr<ConnectionManager>> connections;
	Listener listener;
	std::shared_ptr<ClusterShell> shell;
};

#endif
