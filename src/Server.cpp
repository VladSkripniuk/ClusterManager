#include "ClusterManager.h"

#include <getopt.h>

#include <string>

int main(int argc, char *argv[]) {
	std::string logdir;
    logdir = std::string("/var/log/cluster");
    std::string cfgdir;
    cfgdir = std::string("/etc/cluster");

	int c;
	struct option long_options[] = {
        {"logdir", required_argument, 0, 'l'},
        {"cfgdir", required_argument, 0, 'c'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    int option_index = 0;
	while ((c = getopt_long(argc, argv, "l:c:h", long_options, &option_index)) != -1) {
		switch (c) {
			case 'l':
                logdir = std::string(optarg);
			    break;

			case 'c':
                cfgdir = std::string(optarg);
			    break;

			case 'h':
                printf("Usage: %s [options]\n", argv[0]);
                printf("-l, --logdir=DIR use specified folder for storing logs\n");
                printf("-c, --cfgdir=DIR seek configuration files in specified folder\n");
                printf("-h, --help show this help\n");
                
                printf("commands:\n");
                printf("run - add task\n");
                printf("info - print information");
                return 0;
			    break;

			case '?':
			    break;

			default:
			    abort();
		}
	}

	ClusterManager clusterManager;
	clusterManager.Run();
	return 0;
}