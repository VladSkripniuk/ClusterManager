#include "Connection.h"

Connection::Connection(int fdConnectionArg)
: fdConnection(fdConnectionArg)
{
    pthread_mutex_init(&connectionMutex, NULL);
    last_time = time(NULL);
    status = OK;
    pthread_create(&tid, NULL, refresh, (void*)this);
}

Connection::Connection(const char *IP) {
    struct sockaddr_in addr;

    fdConnection = socket(AF_INET, SOCK_STREAM, 0);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(LISTENER_PORT);
    inet_pton(AF_INET, IP, &addr.sin_addr);

    if (connect(fdConnection, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    	perror("connect");
    	exit(1);
    }
    pthread_mutex_init(&connectionMutex, NULL);
    last_time = time(NULL);
    status = OK;
    pthread_create(&tid, NULL, refresh, (void*)this);
}

Connection::~Connection() {
    close(fdConnection);
    pthread_mutex_destroy(&connectionMutex);
    pthread_cancel(tid);
}

int Connection::ReadSingleMessageFromSocketToBuf() {
    pollfd struct_for_poll;
    struct_for_poll.fd = fdConnection;
    struct_for_poll.events = POLLIN;
    poll(&struct_for_poll, 1, 0);

    if ((struct_for_poll.revents & POLLIN) == 0) {
    	return -1;
    }

	int32_t total_delivered;
    int32_t current_delivered;

    total_delivered = 0;
    int32_t size_of_message;
    while (total_delivered < sizeof(int32_t)) {
        current_delivered = recv(fdConnection, (&size_of_message) + total_delivered,
        	sizeof(int32_t) - total_delivered, 0);
        if ((current_delivered == 0) && (total_delivered == 0)) {
            return -1;
        }
        total_delivered += current_delivered;
    }
    
    size_of_message = ntohl(size_of_message);

    total_delivered = 0;

    while (total_delivered < size_of_message) {
    	current_delivered = recv(fdConnection, buf + total_delivered,
    		size_of_message - total_delivered, 0);
    	total_delivered += current_delivered;
    }

    buf[size_of_message] = '\0';
    return 0;
}

int Connection::DeserializeStructuresFromBufToCorrQueue() {
	tinyxml2::XMLDocument xmlMessage;
    xmlMessage.Parse(buf);
    tinyxml2::XMLElement *pRoot = xmlMessage.FirstChildElement("root");
    tinyxml2::XMLElement *pElement = pRoot->FirstChildElement("message");
    while (pElement != NULL) {
        const char *type;
        type = pElement->Attribute("type");
        if (strcmp(type, "task") == 0) {
        	std::shared_ptr<Task> pTask(new Task);
        	pTask->filename = std::string(pElement->FirstChildElement("name")->GetText());
        	if (pElement->FirstChildElement("arguments")->GetText() != NULL) {
                pTask->args = std::string(pElement->FirstChildElement("arguments")->GetText());
            }
            else {
                pTask->args = std::string("");
            }
        	pElement->FirstChildElement("taskID")->QueryIntText(&pTask->taskID);
        	tasks.push(pTask);
        }
        else if (strcmp(type, "report") == 0) {
            std::shared_ptr<Report> pReport(new Report);
            pReport->report = std::string(pElement->FirstChildElement("report")->GetText());
            pElement->FirstChildElement("taskID")->QueryIntText(&pReport->taskID);
            reports.push(pReport);
        }
        else if (strcmp(type, "info") == 0) {
            std::shared_ptr<Info> pInfo(new Info);
            pInfo->hostname = std::string(pElement->FirstChildElement("hostname")->GetText());
            pInfo->IP = std::string(pElement->FirstChildElement("IP")->GetText());
            info.push(pInfo);
        }
        else if (strcmp(type, "check") == 0) {
            std::shared_ptr<Check> pCheck(new Check);
            pCheck->t = time(NULL);
            checks.push(pCheck);

        }
        pElement = pElement->NextSiblingElement("message");
    }
    return 0;
}

int Connection::DumpAllFromSocketToCorrespondingQueue() {
	while (ReadSingleMessageFromSocketToBuf() == 0) {
        DeserializeStructuresFromBufToCorrQueue();
	}
}

int Connection::GetTask(std::shared_ptr<Task> &pTask) {
    pthread_mutex_lock(&connectionMutex);
    if (tasks.empty()) {
        pthread_mutex_unlock(&connectionMutex);
    	return -1;
    }
    pTask = tasks.front();
    tasks.pop();
    pthread_mutex_unlock(&connectionMutex);
    return 0;
}

int Connection::GetReport(std::shared_ptr<Report> &pReport) {
    pthread_mutex_lock(&connectionMutex);
    if (reports.empty()) {
    	pthread_mutex_unlock(&connectionMutex);
        return -1;
    }
    pReport = reports.front();
    reports.pop();
    pthread_mutex_unlock(&connectionMutex);
    return 0;
}

int Connection::GetInfo(std::shared_ptr<Info> &pInfo) {
    pthread_mutex_lock(&connectionMutex);
    if (info.empty()) {
    	pthread_mutex_unlock(&connectionMutex);
        return -1;
    }
    pInfo = info.front();
    info.pop();
    pthread_mutex_unlock(&connectionMutex);
    return 0;
}

int Connection::SendTask(std::shared_ptr<Task> pTask) {
    pthread_mutex_lock(&connectionMutex);
    SerializeTaskToBuf(pTask);
    WriteSingleMessageFromBufToSocket();
    pthread_mutex_unlock(&connectionMutex);
    return 0;
}

int Connection::SendReport(std::shared_ptr<Report> pReport) {
    pthread_mutex_lock(&connectionMutex);
    SerializeReportToBuf(pReport);
    WriteSingleMessageFromBufToSocket();
    pthread_mutex_unlock(&connectionMutex);
    return 0;
}

int Connection::SendInfo(std::shared_ptr<Info> pInfo) {
    pthread_mutex_lock(&connectionMutex);
    SerializeInfoToBuf(pInfo);
    WriteSingleMessageFromBufToSocket();
    pthread_mutex_unlock(&connectionMutex);
    return 0;
}

int Connection::SendCheck() {
    SerializeCheckToBuf();
    WriteSingleMessageFromBufToSocket();
    return 0;
}

int Connection::SerializeTaskToBuf(std::shared_ptr<Task> pTask) {
    tinyxml2::XMLDocument xmlMessage;
    tinyxml2::XMLElement *pRoot = xmlMessage.NewElement("root");
    tinyxml2::XMLElement *pMessage = xmlMessage.NewElement("message");
    pMessage->SetAttribute("type", "task");
    tinyxml2::XMLElement *pName = xmlMessage.NewElement("name");
    tinyxml2::XMLElement *pArgs = xmlMessage.NewElement("arguments");
    tinyxml2::XMLElement *pReportID = xmlMessage.NewElement("taskID");
    pName->SetText(pTask->filename.c_str());
    pArgs->SetText(pTask->args.c_str());
    pReportID->SetText(pTask->taskID);
    pMessage->InsertFirstChild(pReportID);
    pMessage->InsertFirstChild(pArgs);
    pMessage->InsertFirstChild(pName);
    pRoot->InsertFirstChild(pMessage);
    xmlMessage.InsertFirstChild(pRoot);
    tinyxml2::XMLPrinter printer;
    xmlMessage.Print(&printer);
    strcpy(buf + sizeof(int32_t), printer.CStr());
    *((int32_t*)buf) = htonl(strlen(printer.CStr()));
    return 0;
}

int Connection::SerializeCheckToBuf() {
    tinyxml2::XMLDocument xmlMessage;
    tinyxml2::XMLElement *pRoot = xmlMessage.NewElement("root");
    tinyxml2::XMLElement *pMessage = xmlMessage.NewElement("message");
    pMessage->SetAttribute("type", "check");
    pRoot->InsertFirstChild(pMessage);
    xmlMessage.InsertFirstChild(pRoot);
    tinyxml2::XMLPrinter printer;
    xmlMessage.Print(&printer);
    strcpy(buf + sizeof(int32_t), printer.CStr());
    *((int32_t*)buf) = htonl(strlen(printer.CStr()));
    return 0;
}


int Connection::SerializeReportToBuf(std::shared_ptr<Report> pReport) {
    tinyxml2::XMLDocument xmlMessage;
    tinyxml2::XMLElement *pRoot = xmlMessage.NewElement("root");
    tinyxml2::XMLElement *pMessage = xmlMessage.NewElement("message");
    pMessage->SetAttribute("type", "report");
    tinyxml2::XMLElement *pText = xmlMessage.NewElement("report");
    tinyxml2::XMLElement *pReportID = xmlMessage.NewElement("taskID");
    pText->SetText(pReport->report.c_str());
    pReportID->SetText(pReport->taskID);
    pMessage->InsertFirstChild(pReportID);
    pMessage->InsertFirstChild(pText);
    pRoot->InsertFirstChild(pMessage);
    xmlMessage.InsertFirstChild(pRoot);
    tinyxml2::XMLPrinter printer;
    xmlMessage.Print(&printer);
    strcpy(buf, printer.CStr());
    strcpy(buf + sizeof(int32_t), printer.CStr());
    *((int32_t*)buf) = htonl(strlen(printer.CStr()));
    return 0;
}

int Connection::SerializeInfoToBuf(std::shared_ptr<Info> pInfo) {
	tinyxml2::XMLDocument xmlMessage;
	tinyxml2::XMLElement *pRoot = xmlMessage.NewElement("root");
    tinyxml2::XMLElement *pMessage = xmlMessage.NewElement("message");
    pMessage->SetAttribute("type", "info");
    tinyxml2::XMLElement *pIP = xmlMessage.NewElement("IP");
    pIP->SetText(pInfo->IP.c_str());
    tinyxml2::XMLElement *pHostname = xmlMessage.NewElement("hostname");
    pHostname->SetText(pInfo->hostname.c_str());
    pMessage->InsertFirstChild(pIP);
    pMessage->InsertFirstChild(pHostname);
    pRoot->InsertFirstChild(pMessage);
    xmlMessage.InsertFirstChild(pRoot);
    tinyxml2::XMLPrinter printer;
    xmlMessage.Print(&printer);
    strcpy(buf, printer.CStr());
    strcpy(buf + sizeof(int32_t), printer.CStr());
    *((int32_t*)buf) = htonl(strlen(printer.CStr()));
	return 0;
}

int Connection::WriteSingleMessageFromBufToSocket() {
    int32_t size_of_message_plus_prefix;
    int32_t size_of_message;
    size_of_message = strlen(buf + sizeof(int32_t));

    *((uint32_t*)buf) = htonl(size_of_message);

    int32_t total_sent;
    int32_t current_sent;
    
    total_sent = 0;
    size_of_message_plus_prefix = size_of_message + sizeof(int32_t);
    


    while (total_sent < size_of_message_plus_prefix) {
    	sigset_t sigset;
        sigemptyset(&sigset);
        sigaddset(&sigset, SIGPIPE);
        sigprocmask(SIG_BLOCK, &sigset, NULL);
        current_sent = send(fdConnection, buf + total_sent, size_of_message_plus_prefix - total_sent, 0);
        if (errno == EPIPE) {
            return -1;
        }
    	total_sent += current_sent;
    }

    return 0;
}



void *Connection::refresh(void *arg) {
    Connection *This = (Connection*)arg;
    while (true) {
        pthread_mutex_lock(&This->connectionMutex);
        This->SendCheck();
        This->DumpAllFromSocketToCorrespondingQueue();
        time_t curr_time = time(NULL);
        while (!This->checks.empty()) {
            if (This->last_time < This->checks.front()->t) {
                This->last_time = This->checks.front()->t;
            }
            This->checks.pop();
        }
        if (curr_time - This->last_time > BIG_DELAY) {
            This->status = FAILED;
        }
        else {
            This->status = OK;
        }
        pthread_mutex_unlock(&This->connectionMutex);
        sleep(SMALL_DELAY);
        sched_yield();
    }
}

int Connection::CheckStatus() {
    pthread_mutex_lock(&connectionMutex);
    if (status == OK) {
        pthread_mutex_unlock(&connectionMutex);
        return 0;
    }
    else {
        pthread_mutex_unlock(&connectionMutex);
        return -1;
    }
}