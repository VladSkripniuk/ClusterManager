#if !defined( __CLUSTER_SHELL__H__ )
#define __CLUSTER_SHELL__H__

class ClusterManager;
class ConnectionManager;
class ClusterShell;

#include "ClusterManager.h"
#include "ShellParser.h"

#include <pthread.h>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <queue>
#include <sched.h>
#include <stdlib.h>

class ClusterShell {
public:
    ClusterShell(ClusterManager *pClusterManagerArg);
    ~ClusterShell();
    int AddReport(std::shared_ptr<Report> pReport);
    int Run();
private:
	int ReadUserCommand();
    static void* running_in_another_thread(void*);

	pthread_t tid;

    ClusterManager *pClusterManager;
 
    pthread_mutex_t reports_mutex;
    std::queue<std::shared_ptr<Report>> reports;
};

#endif