/*
Copyright (c) 2010 Nick Gerakines <nick at gerakines dot net>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/queue.h>
#include <stdlib.h>
#include <err.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h> 
#include <stdlib.h> 
#include <stdio.h>

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

void send_command(int sd, char *command);

int main(int argc, char **argv) {
	char *ipaddress = "127.0.0.1";
	int port = 8003;

	int c;
	while (1) {
		static struct option long_options[] = {
			{"ip",      required_argument, 0, 'i'},
			{"port",    required_argument, 0, 'p'},
			{0, 0, 0, 0}
		};
		int option_index = 0;
		c = getopt_long(argc, argv, "i:p:", long_options, &option_index);
		if (c == -1) { break; }
		switch (c) {
			case 0:
				if (long_options[option_index].flag != 0) { break; }
				printf ("option %s", long_options[option_index].name);
				if (optarg) { printf(" with arg %s", optarg); }
				printf("\n");
				break;
			case 'i':
				ipaddress = optarg;
				break;
			case 'p':
				port = atoi(optarg);
				break;
			case '?':
				/* getopt_long already printed an error message. */
				break;
			default:
				abort();
		}
	}

	int action = -1;

	if (argc - optind == 0) {
		printf("Command not provided.\n");
		printf("usage: client [--ip=] [--port=] <command> [... command arguments]\n");
		exit(1);
	}

	if (strcmp(argv[optind], "set") == 0) {
		if (argc - optind != 4) {
			printf("The 'set' command requires 3 command parameters.\n");
			printf("usage: client [--ip=] [--port=] set <namespace> <key> <value>\n");
			exit(1);
		}
		action = 1;
	}

	if (strcmp(argv[optind], "last") == 0) {
		if (argc - optind != 3) {
			printf("The 'last' command requires 2 command parameters.\n");
			printf("usage: client [--ip=] [--port=] last <namespace> <key>\n");
			exit(1);
		}
		action = 2;
	}

	if (action == 0) {
		printf("Invalid command given, should be 'set'.\n");
		printf("usage: client [--ip=] [--port=] <command> [... command arguments]\n");
		exit(1);
	}

	struct hostent *hp;
	struct sockaddr_in pin;
	int sd;

	if ((hp = gethostbyname(ipaddress)) == 0) {
		perror("gethostbyname");
		exit(1);
	}

	memset(&pin, 0, sizeof(pin));
	pin.sin_family = AF_INET;
	pin.sin_addr.s_addr = ((struct in_addr *)(hp->h_addr))->s_addr;
	pin.sin_port = htons(port);

	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	if (connect(sd,(struct sockaddr *)  &pin, sizeof(pin)) == -1) {
		perror("connect");
		exit(1);
	}

	char msg[32];

	switch (action) {
		case 1:
			sprintf(msg, "SET %s %s %s\r\n", argv[optind + 1], argv[optind + 2], argv[optind + 3]);
			send_command(sd, msg);
			break;
		case 2:
			sprintf(msg, "LAST %s %s %s\r\n", argv[optind + 1], argv[optind + 2]);
			send_command(sd, msg);
			break;
		default:
			break;
	}

	close(sd);

	return 0;
}

void send_command(int sd, char *command) {
	if (send(sd, command, strlen(command), 0) == -1) {
		perror("send");
		exit(1);
	}
	char buf[1024];
	int numbytes;
	if((numbytes = recv(sd, buf, 1024-1, 0)) == -1) {
		perror("recv()");
		exit(1);
	}

	char *resp = NULL;
	int buf_len;

	switch (buf[0]) {
		case '-':
			break;
		case '+':
		case ':':
			buf_len = strlen(buf) - 3;
			if (buf_len >= 2) {
				resp = malloc(1 + buf_len);
				memcpy(resp, buf+1, buf_len);
				printf("%s\n", resp);
				free(resp);
			} else {
				printf("protocol error: %s\n", buf);
			}
			break;
		default:
			printf("%s\n", buf);
			break;
	}
}
