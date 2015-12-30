#include "ClusterManager.h"
#include "ClusterShell.h"

ClusterManager::ClusterManager() {
    shell = std::shared_ptr<ClusterShell>(new ClusterShell(this));

}

ClusterManager::~ClusterManager() {
}

int ClusterManager::Run() {
    shell->Run();

	while (true) {
        std::shared_ptr<Connection> connection = listener.AcceptConnection();
        if (connection != NULL) {
            std::shared_ptr<ConnectionManager> pConnectionManager;
            ID connectionID;
            AllocateNewExecutorID(&connectionID);
            pConnectionManager = std::shared_ptr<ConnectionManager>(new ConnectionManager (connection, connectionID, this));
            connections[connectionID] = pConnectionManager;
            pConnectionManager->Run();
        }
        sched_yield();
	}
    return 0;
}


int ClusterManager::AllocateNewExecutorID(ID *newID) {
    for (size_t i = 0; i < connections.size(); ++i) {
        if (connections[i] == NULL) {
        	*newID = i;
        	return 0;
        }
    }
    if (connections.capacity() == connections.size()) {
        connections.reserve(2 * connections.capacity());
    }
    connections.resize(connections.size() + 1);
    *newID = connections.size() - 1;
    return 0;
}

int ClusterManager::AllocateNewTaskID(ID *newID) {
    static ID counter = 0;
    *newID = counter++;
    return 0;
}

int ClusterManager::AddTask(std::shared_ptr<Task> pTask) {
    std::shared_ptr<ConnectionManager> pConnectionManager;
    if (pTask->executor < 0) {
        if (GetExecutorForTask(pConnectionManager) != 0) {
            return -1;
        }
    }
    else {
        if (connections[pTask->executor] == NULL) {
            return -1;
        }
        pConnectionManager = connections[pTask->executor];
    }
    pConnectionManager->AddTask(pTask);
    return 0;
}

int ClusterManager::GetExecutorForTask(std::shared_ptr<ConnectionManager> &pConnectionManager) {
    int min_number_of_tasks = -1;
    int result = -1;
    for (size_t i = 0; i < connections.size(); ++i) {
        if (connections[i] != NULL) {
            if ((min_number_of_tasks == -1) || (min_number_of_tasks > connections[i]->NumberOfTasks())) {
                min_number_of_tasks = connections[i]->NumberOfTasks();
                result = i;
            }
        }
    }
    if (result != -1) {
        pConnectionManager = connections[result];
        return 0;
    }
    else {
        return -1;
    }
}

int ClusterManager::SendToShellReport(std::shared_ptr<Report> pReport) {
    shell->AddReport(pReport);
    return 0;
}

int ClusterManager::GetInfo(std::string &arg) {
    std::stringstream buf;
    std::string connectionInfo;
    int number_of_nodes = 0;
    buf << "----------------\n";    
    for (size_t i = 0; i < connections.size(); ++i) {
        if (connections[i] != NULL) {
            ++number_of_nodes;
            connections[i]->GetInfo(connectionInfo);
            buf << connectionInfo << "****************" << std::endl;
        }
    }
    buf << "total number of nodes: " << number_of_nodes << std::endl;
    buf << "----------------\n\0";
    std::getline(buf, arg, '\0');
    return 0;
}

int ClusterManager::DeleteConnectionAndRearrangeTasks(ID connectionID, std::vector<std::shared_ptr<Task>> tasks) {
    connections[connectionID] = NULL;
    for (size_t i = 0; i < tasks.size(); ++i) {
        AddTask(tasks[i]);
    }
    return 0;
}