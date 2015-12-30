#include "Connection.h"
#include "Listener.h"

Listener::Listener() {
    fdListener = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(LISTENER_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int optval = 1 ; 
 
    setsockopt(fdListener, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)); 
    
    if (bind(fdListener, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(1);
    }

    listen(fdListener, LISTENER_QUEUE_SIZE);
}

Listener::~Listener() {
    close(fdListener);
}

std::shared_ptr<Connection> Listener::AcceptConnection() {
    pollfd struct_for_poll;
    struct_for_poll.fd = fdListener;
    struct_for_poll.events = POLLIN;
    poll(&struct_for_poll, 1, 0);
    if ((struct_for_poll.revents & POLLIN) == 0) {
    	return NULL;
    }
    int fdConnection;
    sockaddr_in clientAddr;
    int clientAddrLen = sizeof(clientAddr);
    fdConnection = accept(fdListener, (struct sockaddr*)&clientAddr, (socklen_t*)&clientAddrLen);
    std::shared_ptr<Connection> pConnection(new Connection (fdConnection));
    return pConnection;
}