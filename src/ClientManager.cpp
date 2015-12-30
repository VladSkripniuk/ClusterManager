#include "ClientManager.h"

ClientManager::ClientManager() {
    char buf[HOSTNAME_MAX];
    gethostname(buf, HOSTNAME_MAX);
    hostname = buf;
    struct hostent *host_info = 0;
    host_info = gethostbyname(buf);
    char *ip;
    ip = host_info->h_addr_list[0];
    std::stringstream ss;
    ss << unsigned(ip[0]) << '.' << unsigned(ip[1]) << '.' << unsigned(ip[2]) << '.' << unsigned(ip[3]);
    ss >> IP;
}

ClientManager::~ClientManager() {
}


int ClientManager::AllocateNewProcessID(ID *newProcessID) {
    for (size_t i = 0; i < running_tasks.size(); ++i) {
        if (running_tasks[i] == NULL) {
        	*newProcessID = i;
        	return 0;
        }
    }
    if (running_tasks.capacity() == running_tasks.size()) {
        running_tasks.reserve(2 * running_tasks.capacity());
    }
    running_tasks.resize(running_tasks.size() + 1);
    *newProcessID = running_tasks.size() - 1;
    return 0;
}

int ClientManager::LaunchTask(std::shared_ptr<Task> pTask) {
    ID processID;
    AllocateNewProcessID(&processID);
    running_tasks[processID] = pTask;

    pid_t pid = fork();
    if (pid == 0) {
        execl("/bin/sh", "/bin/sh", "-c", (pTask->filename + std::string(" ") + pTask->args).c_str(), (char*)NULL);
    }
    running_tasks[processID]->pid = pid;
    return 0;
}

int ClientManager::CheckTasksStatus() {
	for (size_t i = 0; i < running_tasks.size(); ++i) {
		if (running_tasks[i] == NULL) {
			continue;
		}
		int status;
		if (waitpid(running_tasks[i]->pid, &status, WNOHANG) != 0) {
			if (WIFEXITED(status) && (WEXITSTATUS(status) == 0)) {
				std::shared_ptr<Report> pReport(new Report);
				pReport->taskID = running_tasks[i]->taskID;
				pReport->report = "finished";
				connection->SendReport(pReport);
				running_tasks[i] = NULL;
				continue;
			}
			else if (WIFEXITED(status) && (WEXITSTATUS(status) != 0)) {
				printf("restarted\n");
			    LaunchTask(running_tasks[i]);
			    running_tasks[i] = NULL;
			}
		}
	}
	return 0;
}

int ClientManager::Run(const char *serverIP) {
    RegisterInCluster(serverIP);
    while (true) {
        if (connection->CheckStatus() != 0) {
            exit(0);
        }
        std::shared_ptr<Task> task;
        while (connection->GetTask(task) == 0) {
            LaunchTask(task); 
        }
        CheckTasksStatus();
        sched_yield();
    }
    return 0;
}

int ClientManager::RegisterInCluster(const char *serverIP) {
	connection = std::shared_ptr<Connection>(new Connection (serverIP));
    
    std::shared_ptr<Info> pInfo(new Info);
    pInfo->hostname = hostname;
    pInfo->IP = IP;
    connection->SendInfo(pInfo);
    return 0;
}