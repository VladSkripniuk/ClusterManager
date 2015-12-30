#include "ConnectionManager.h"

ConnectionManager::ConnectionManager(std::shared_ptr<Connection> connectionArg, ID connectionIDArg, ClusterManager *pClusterManagerArg)
: connection(connectionArg)
, connectionID(connectionIDArg)
, pClusterManager(pClusterManagerArg)
{
    pthread_mutex_init(&tasksMutex, NULL);
}

ConnectionManager::~ConnectionManager() {
    pthread_mutex_destroy(&tasksMutex);
}

int ConnectionManager::Run() {
    pthread_create(&tid, NULL, running_in_another_thread, (void*)this);
    return 0;
}

void* ConnectionManager::running_in_another_thread(void* pArg) {
    ConnectionManager *This;
    This = (ConnectionManager*)pArg;
    This->RegisterNode();

    while (true) {
        pthread_mutex_lock(&This->tasksMutex);
        if (This->CheckConnection() != 0) {
            pthread_mutex_unlock(&This->tasksMutex);
            This->CloseConnection();
            return NULL;
        }
        while (!This->waiting_tasks.empty()) {
            std::shared_ptr<Task> pTask = This->waiting_tasks.front();
            This->waiting_tasks.pop();
            This->connection->SendTask(pTask);
            This->running_tasks.push(pTask);
        }
    	pthread_mutex_unlock(&This->tasksMutex);
        
        std::shared_ptr<Report> report;
        while (This->connection->GetReport(report) == 0) {
            if (report->report.compare("finished") == 0) {
                This->DeleteTask(report->taskID);
            }     
            This->pClusterManager->SendToShellReport(report);
        }
        sched_yield();
    }
}

int ConnectionManager::RegisterNode() {
    std::shared_ptr<Info> pInfo;
    while (true) {
        if (connection->GetInfo(pInfo) == 0) {
            break;
        }
    }
    hostname = pInfo->hostname;
    IP = pInfo->IP;
    return 0;
}

int ConnectionManager::AddTask(std::shared_ptr<Task> task) {
    pthread_mutex_lock(&tasksMutex);
    waiting_tasks.push(task);
    pthread_mutex_unlock(&tasksMutex);
    return 0;
}

int ConnectionManager::GetInfo(std::string &name_and_tasks) {
    std::stringstream buf;
    buf << "ID: " << connectionID << std::endl;
    buf << "hostname: " << hostname << std::endl;
    buf << "IP: " << IP << std::endl;
    pthread_mutex_lock(&tasksMutex);
    buf << "waiting_tasks:\n";
    int counter = 0;
    if (!waiting_tasks.empty()) {
        std::shared_ptr<Task> first;
        first = waiting_tasks.front();
        while (true) {
            std::shared_ptr<Task> current = waiting_tasks.front();
            waiting_tasks.pop();
            buf << current->taskID << ' ' << current->filename << std::endl;
            ++counter;
            waiting_tasks.push(current);
            if (waiting_tasks.front() == first) {
                break;
            }
        }
    }
    buf << "total waiting: " << counter << std::endl;
    buf << "running tasks:\n";
    counter = 0;
    if (!running_tasks.empty()) {
        std::shared_ptr<Task> first;
        first = running_tasks.front();
        while (true) {
            std::shared_ptr<Task> current = running_tasks.front();
            running_tasks.pop();
            buf << current->taskID << ' ' << current->filename << std::endl;
            ++counter;
            running_tasks.push(current);
            if (running_tasks.front() == first) {
                break;
            }
        }
    }
    buf << "total running: " << counter << std::endl;
    buf << '\0';
    pthread_mutex_unlock(&tasksMutex);
    std::getline(buf, name_and_tasks, '\0');
    return 0;
}

int ConnectionManager::DeleteTask(int taskID) {
    pthread_mutex_lock(&tasksMutex);
    if (!running_tasks.empty()) {
        std::shared_ptr<Task> first;
        first = running_tasks.front();
        while (true) {
            std::shared_ptr<Task> current;
            current = running_tasks.front();
            running_tasks.pop();
            if (current->taskID == taskID) {
                break;
            }
            running_tasks.push(current);
            if (running_tasks.front() == first) {
                break;
            }
        }
    }

    pthread_mutex_unlock(&tasksMutex);
    return 0;
}

int ConnectionManager::CheckConnection() {
    return connection->CheckStatus();
}

int ConnectionManager::CloseConnection() {
    std::vector<std::shared_ptr<Task>> tasks;
    while (!running_tasks.empty()) {
        std::shared_ptr<Task> pTask;
        pTask = running_tasks.front();
        running_tasks.pop();
        tasks.push_back(pTask);
    }
    while (!waiting_tasks.empty()) {
        std::shared_ptr<Task> pTask;
        pTask = waiting_tasks.front();
        waiting_tasks.pop();
        tasks.push_back(pTask);
    }
    pClusterManager->DeleteConnectionAndRearrangeTasks(this->connectionID, tasks);
    return 0;
}

int ConnectionManager::NumberOfTasks() {
    int result;
    pthread_mutex_lock(&tasksMutex);
    result = waiting_tasks.size() + running_tasks.size();
    pthread_mutex_unlock(&tasksMutex);
    return result;
}