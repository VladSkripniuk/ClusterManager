#if !defined( __CONNECTION__H__ )
#define __CONNECTION__H__

#include "MsgStruct.h"

#include "tinyxml2.h"

#include <queue>
#include <string>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <unistd.h>
#include <memory>
#include <iostream>
#include <fstream>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include <sched.h>

#define OK 0
#define FAILED 1
#define MAX_SIZE_OF_MESSAGE 1048576 // 1 MB
#define LISTENER_PORT 8080
#define BIG_DELAY 5
#define SMALL_DELAY 2

class Connection {
public:
	Connection(int);
	Connection(const char*);
	~Connection();
	
	int GetInfo(std::shared_ptr<Info>&);
	int SendInfo(std::shared_ptr<Info>);
	
	int GetTask(std::shared_ptr<Task>&);
	int SendTask(std::shared_ptr<Task>);
	
	int GetReport(std::shared_ptr<Report>&);
	int SendReport(std::shared_ptr<Report>);

	int CheckStatus();
private:
	static void* refresh(void*);
    
    int ReadSingleMessageFromSocketToBuf();
    int WriteSingleMessageFromBufToSocket();

    int SendCheck();
    int SerializeCheckToBuf();
    int SerializeTaskToBuf(std::shared_ptr<Task>);
    int SerializeReportToBuf(std::shared_ptr<Report>);
    int SerializeInfoToBuf(std::shared_ptr<Info>);
    int DeserializeStructuresFromBufToCorrQueue();
    int DumpAllFromSocketToCorrespondingQueue();

    pthread_mutex_t connectionMutex;
	
	int status;
    time_t last_time;
    pthread_t tid;
	int fdConnection;
    
    std::queue<std::shared_ptr<Report>> reports;
    std::queue<std::shared_ptr<Task>> tasks;
    std::queue<std::shared_ptr<Info>> info;
    std::queue<std::shared_ptr<Check>> checks;

    char buf[MAX_SIZE_OF_MESSAGE];
};

#endif