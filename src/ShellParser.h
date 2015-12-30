#if !defined( __SHELL_PARSER__H__)
#define __SHELL_PARSER__H__

#include "MsgStruct.h"

#include <memory>
#include <iostream>
#include <string>

class ShellParser {
public:
	int GetTask(std::shared_ptr<Task>);
private:
	int GetName(std::shared_ptr<Task>);
	int GetArgs(std::shared_ptr<Task>);
	int GetNodeID(std::shared_ptr<Task>);
};

#endif