#include "ShellParser.h"

int ShellParser::GetTask(std::shared_ptr<Task> pTask) {
    GetName(pTask);
    GetArgs(pTask);
    GetNodeID(pTask);
    return 0;
}

int ShellParser::GetName(std::shared_ptr<Task> pTask) {
    std::string command;
    std::cout << "enter name of program: ";
    std::getline(std::cin, command, '\n');
    if (command.length() == 0) {
        std::cout << "invalid name\n";
        return 0;
	}
	pTask->filename = command;
	return 0;
}

int ShellParser::GetArgs(std::shared_ptr<Task> pTask) {
    std::string command;
    std::cout << "enter arguments: ";
	std::getline(std::cin, command, '\n');
	pTask->args = command;
	return 0;
}

int ShellParser::GetNodeID(std::shared_ptr<Task> pTask) {
    std::string command;
    std::cout << "enter node ID: ";
	std::getline(std::cin, command, '\n');
	if (command.length() > 0) {
	    pTask->executor = strtol(command.c_str(), NULL, 10);
	    if (errno == ERANGE) {
	    	pTask->executor = -1;
	    }
    }
    else {
    	pTask->executor = -1;
    }
    return 0;
}