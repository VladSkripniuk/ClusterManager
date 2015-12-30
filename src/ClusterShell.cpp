#include "ClusterShell.h"

ClusterShell::ClusterShell(ClusterManager *pClusterManagerArg)
: pClusterManager(pClusterManagerArg)
{
    pthread_mutex_init(&reports_mutex, NULL);
}

ClusterShell::~ClusterShell() {
    pthread_mutex_destroy(&reports_mutex);
}

int ClusterShell::Run() {
	pthread_create(&tid, NULL, running_in_another_thread, (void*)this);
	return 0;
}

void *ClusterShell::running_in_another_thread(void *pArg) {
	ClusterShell *This;
	This = (ClusterShell*)pArg;

	while (true) {
		This->ReadUserCommand();
		pthread_mutex_lock(&This->reports_mutex);
        while (!This->reports.empty()) {
            std::cout << This->reports.front()->taskID << ": " << This->reports.front()->report << std::endl;
            This->reports.pop();
        }
		pthread_mutex_unlock(&This->reports_mutex);
		sched_yield();
	}

}

int ClusterShell::ReadUserCommand() {
	std::string command;
	ShellParser parser;
	std::cout << "enter your command: ";
    std::getline(std::cin, command, '\n');
    if (command.length() > 0) {
		if (command.compare("run") == 0) {
			std::shared_ptr<Task> pTask(new Task);
			parser.GetTask(pTask);
			pClusterManager->AllocateNewTaskID(&pTask->taskID);
	    	if (pClusterManager->AddTask(pTask) != 0) {
	    		std::cout << "unable to add task: there are no executors\n";
	    	}
	    	else {
	    		std::cout << "task added: " << pTask->taskID << std::endl;
	    	}
	    }
	    else if (command.compare("info") == 0) {
            std::string info;
            pClusterManager->GetInfo(info);
            std::cout << info;
	    }
	    else if (command.compare("quit") == 0) {
            exit(0);
	    }
	    else {
	    	std::cout << "unknown command\n";
	    	return 0;
	  	}
    }
	return 0;
}


int ClusterShell::AddReport(std::shared_ptr<Report> pReport) {
    pthread_mutex_lock(&reports_mutex);
    reports.push(pReport);
    pthread_mutex_unlock(&reports_mutex);
    return 0;
}