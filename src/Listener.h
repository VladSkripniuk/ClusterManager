#if !defined( __LISTENER__H__ )
#define __LISTENER__H__

#include "Connection.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>
#include <unistd.h>
#include <memory>

#define LISTENER_PORT		8080
#define LISTENER_QUEUE_SIZE	10

class Listener {
public:
	Listener();
	~Listener();
	std::shared_ptr<Connection> AcceptConnection();
private:
    int fdListener;
};

#endif