/*
 * lws-minimal-http-server-form-post-file
 *
 * Written in 2010-2019 by Andy Green <andy@warmcat.com>
 *
 * This file is made available under the Creative Commons CC0 1.0
 * Universal Public Domain Dedication.
 *
 * This demonstrates a minimal http server that performs POST with a couple
 * of parameters and a file upload, all in multipart (mime) form mode.
 * It saves the uploaded file in the current directory, dumps the parameters to
 * the console log and redirects to another page.
 */

#include <libwebsockets.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>

#define BASE	1024
#define BLOCK	BASE * BASE

struct pss {
	struct lws_spa *spa; /* lws helper decodes multipart form */
	char filename[128]; /* the filename of the uploaded file */
	unsigned long long file_length; /* the amount of bytes uploaded */
	int fd; /* fd on file being saved */
};

static int interrupted;

unsigned char alphabet[62] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g',
			'h', 'i', 'j', 'k', 'l', 'm', 'n',
			'o', 'p', 'q', 'r', 's', 't', 'u',
			'v', 'w', 'x', 'y', 'z', 'A', 'B',
			'C', 'D', 'E', 'F', 'G', 'H', 'I',
			'J', 'K', 'L', 'M', 'N', 'O', 'P',
			'Q', 'R', 'S', 'T', 'U', 'V', 'W',
			'X', 'Y', 'Z', '0', '1', '2', '3',
			'4', '5', '6', '7', '8', '9'};

void printRandomString(int n, unsigned char *res)
{
	for (int i = 0; i < n; i++)
		res[i + 1] = alphabet[rand() % 62];
}


static int callback_http(struct lws *wsi, enum lws_callback_reasons reason,
		void *user, void *in, size_t len) {
	return 0;
}

static int callback_speedtest(struct lws *wsi, enum lws_callback_reasons reason,
		void *user, void *in, size_t len)
{
	unsigned int i;
	unsigned char *buf_send;

	switch (reason) {
	case LWS_CALLBACK_ESTABLISHED:
		printf("connection established\n");
		buf_send = (unsigned char*) malloc(
			LWS_SEND_BUFFER_PRE_PADDING + BLOCK +
			LWS_SEND_BUFFER_POST_PADDING);

		printRandomString(BLOCK,
			&buf_send[LWS_SEND_BUFFER_PRE_PADDING]);

		printf("sending data \n");
		lws_write(wsi, &buf_send[LWS_SEND_BUFFER_PRE_PADDING], BLOCK,
			LWS_WRITE_TEXT);
		free(buf_send);
		break;
	case LWS_CALLBACK_RECEIVE: {

		unsigned char *buf = (unsigned char*) malloc(
				LWS_SEND_BUFFER_PRE_PADDING + len
						+ LWS_SEND_BUFFER_POST_PADDING);
		for (i = 0; i < len; i++) {
			buf[LWS_SEND_BUFFER_PRE_PADDING + (len - 1) - i] =
					((char *) in)[i];
		}

		printf("received data: %s, replying: %.*s\n", (char *) in,
				(int) len, buf + LWS_SEND_BUFFER_PRE_PADDING);

		lws_write(wsi, &buf[LWS_SEND_BUFFER_PRE_PADDING], len,
				LWS_WRITE_TEXT);

		free(buf);
		break;
	}
	default:
		break;
	}

	return 0;
}

static struct lws_protocols protocols[] = {
	{ "http", callback_http, sizeof(struct pss), 0 },
	{ "speed-test", callback_speedtest, 0},
	{ NULL, NULL, 0, 0 } /* terminator */
};


void sigint_handler(int sig) {
	interrupted = 1;
}

int main(int argc, const char **argv) {
	struct lws_context_creation_info info;
	struct lws_context *context;

	const char *p;
	int n = 0, logs = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE
	/* | LLL_INFO *//* | LLL_PARSER *//* | LLL_HEADER */
	/* | LLL_EXT *//* | LLL_CLIENT *//* | LLL_LATENCY */
	/* | LLL_DEBUG */;
	srand(time(NULL));
	signal(SIGINT, sigint_handler);

	if ((p = lws_cmdline_option(argc, argv, "-d")))
		logs = atoi(p);

	lws_set_log_level(logs, NULL);
	lwsl_user(
			"LWS minimal http server POST file | visit http://localhost:7681\n");

	memset(&info, 0, sizeof info); /* otherwise uninitialized garbage */
	info.port = 7681;
	info.protocols = protocols;
	info.options =
	LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;

	context = lws_create_context(&info);
	if (!context) {
		lwsl_err("lws init failed\n");
		return 1;
	}

	while (n >= 0 && !interrupted)
		n = lws_service(context, 0);

	lws_context_destroy(context);
	return 0;
}
