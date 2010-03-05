/*
Copyright (c) 2010 Nick Gerakines <nick at gerakines dot net>
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

#include <bert/encoder.h>
#include <bert/magic.h>
#include <bert/errno.h>

typedef char byte_t;

void send_command(int sd, char *command);

int main(int argc, char **argv) {
	char *ipaddress = "127.0.0.1";
	int port = 8002;

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

	if (strcmp(argv[optind], "update") == 0) {
		if (argc - optind != 3) {
			printf("The 'update' command requires 2 command parameters.\n");
			printf("usage: client [--ip=] [--port=] update <item id> <priority value>\n");
			exit(1);
		}
		action = 1;
	}

	if (strcmp(argv[optind], "next") == 0) {
		if (argc - optind != 1) {
			printf("The 'next' command requires 0 command parameters.\n");
			printf("usage: client [--ip=] [--port=] next\n");
			exit(1);
		}
		action = 2;
	}

	if (strcmp(argv[optind], "peek") == 0) {
		if (argc - optind != 1) {
			printf("The 'peek' command requires 0 command parameters.\n");
			printf("usage: client [--ip=] [--port=] peek\n");
			exit(1);
		}
		action = 3;
	}

	if (strcmp(argv[optind], "info") == 0) {
		if (argc - optind != 1) {
			printf("The 'info' command requires 0 command parameters.\n");
			printf("usage: client [--ip=] [--port=] info\n");
			exit(1);
		}
		action = 4;
	}

	if (strcmp(argv[optind], "score") == 0) {
		if (argc - optind != 2) {
			printf("The 'score' command requires 1 command parameter.\n");
			printf("usage: client [--ip=] [--port=] score <item id>\n");
			exit(1);
		}
		action = 5;
	}

	if (action == 0) {
		printf("Invalid command given, should be either update, next, peek or info.\n");
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

	int item_id = 0;
	int priority = 0;
	char msg[32];

	switch (action) {
		case 1:
			item_id = atoi(argv[optind + 1]);
			priority = atoi(argv[optind + 2]);
			sprintf(msg, "UPDATE %d %d\r\n", item_id, priority);
			send_command(sd, msg);
			break;
		case 2:
			send_command(sd, "NEXT\r\n");
			break;
		case 3:
			send_command(sd, "PEEK\r\n");
			break;
		case 4:
			send_command(sd, "INFO\r\n");
			break;
		case 5:
			item_id = atoi(argv[optind + 1]);
			sprintf(msg, "SCORE %d\r\n", item_id);
			send_command(sd, msg);
			break;
		default:
			break;
	}

	close(sd);

	return 0;
}

#define EXPECTED_LENGTH 	2
#define OUTPUT_SIZE			(1 + 1 + 1 + ((1 + 1) * EXPECTED_LENGTH))

void send_command(int sd, char *command) {
	unsigned char output[OUTPUT_SIZE];
	bert_encoder_t *encoder;

	printf("creating encoder\n");
	if (!(encoder = bert_encoder_create()))
	{
		printf("malloc failed\n");
	}
	printf("preparing encode buffer\n");
	bert_encoder_buffer(encoder, output, OUTPUT_SIZE);

	bert_data_t *data;
	int result;
	printf("about to create bert data.\n");
	
	if (!(data = bert_data_create_tuple(EXPECTED_LENGTH)))
	{
		printf("malloc failed\n");
	}

	if (!(data->tuple->elements[0] = bert_data_create_int(1)))
	{
		printf("malloc failed\n");
	}

	if (!(data->tuple->elements[1] = bert_data_create_int(2)))
	{
		printf("malloc failed\n");
	}

	if ((result = bert_encoder_push(encoder,data)) != BERT_SUCCESS)
	{
		printf("%s\n", bert_strerror(result));
		// test_fail(bert_strerror(result));
	}
	
	printf("output data is '%s'\n", output);

	if (send(sd, output, strlen(output), 0) == -1) {
		perror("send");
		exit(1);
	}
	
	bert_data_destroy(data);
	bert_encoder_destroy(encoder);
	
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
				printf("protocol error \n");
			}
			break;
		default:
			printf("%s\n", buf);
			break;
	}
}
