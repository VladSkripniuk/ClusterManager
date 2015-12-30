#include "ClientManager.h"

#include <getopt.h>
#include <string>
#include <iostream>
#include <fstream>


int main(int argc, char *argv[]) {

    std::string logdir = std::string("/var/log/cluster");
    std::string cfgdir = std::string("./cfg");
    std::string IP = std::string("127.0.0.1");

    struct option long_options[] = {
        {"cfgdir", required_argument, 0, 'c'},
        {"logdir", required_argument, 0, 'l'},
        {"help", no_argument, 0, 'h'},
        {"server", required_argument, 0, 's'},
        {0, 0, 0, 0}
    };
    int c;
    int option_index = 0;

    while ((c = getopt_long(argc, argv, "c:l:hs:", long_options, &option_index)) != -1) {
    	switch (c) {
    		case 'c':
    		    cfgdir = std::string(optarg);
    		    break;

    		case 'l':
    		    logdir = std::string(optarg);
    		    break;

    		case 'h':
    		    printf("Usage: %s [options]\n", argv[0]);
    		    printf("-l, --logdir=DIR use specified folder for storing logs\n");
                printf("-c, --cfgdir=DIR seek configuration files in specified folder\n");
                printf("-h, --help show this help\n");
                printf("-s, --server=IP connect to server with specified IP\n");
			    break;

			case 's':
			    IP = std::string(optarg);
			    break;

			case '?':
			    break;

			default:
			    abort();
    	}
    }

	ClientManager clientManager;
	clientManager.Run(IP.c_str());
	return 0;
}