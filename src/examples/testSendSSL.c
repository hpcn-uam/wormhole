#include <stdio.h>
#include <common.h>
#include "../async_inline.c"
#include <assert.h>
#include <sys/time.h>
#include <stdlib.h>
#include <strings.h>
#include <malloc.h>

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define NUM_SMALL_MESSAGES 5000000//00
#define NUM_BIG_MESSAGES 500000//0
#define SIZE_BUFFER 1024*4

int main(int argc, char **argv)
{
	void *buffer = calloc(SIZE_BUFFER, 1);

	if (!buffer) {
		fprintf(stderr, "Error reservando memoria\n");
		return 1;
	}


	int sock = tcp_connect_to("127.0.0.1", 5000);

	assert(sock > 0);


	struct timeval start, end;

	//INIT OPEN SSL
	SSL_load_error_strings();
	SSL_library_init();
	OpenSSL_add_all_algorithms();

	//tmp variables;
	SSL_CTX *sslctx;
	SSL *cSSL;

	sslctx = SSL_CTX_new(TLSv1_2_client_method());
	SSL_CTX_set_options(sslctx, SSL_OP_SINGLE_DH_USE);

	if (!SSL_CTX_load_verify_locations(sslctx, "../certs/ca.pem", NULL)) {
		ERR_print_errors_fp(stderr);
		return -1;
	}

	if (!SSL_CTX_use_certificate_file(sslctx, "../certs/worm.pem", SSL_FILETYPE_PEM)) {
		ERR_print_errors_fp(stderr);
		return -1;
	}

	if (!SSL_CTX_use_PrivateKey_file(sslctx, "../certs/prv/worm.key.pem", SSL_FILETYPE_PEM)) {
		ERR_print_errors_fp(stderr);
		return -1;
	}

	/* verify private key */
	if (!SSL_CTX_check_private_key(sslctx)) {
		fprintf(stderr, "Private key does not match the public certificate\n");
		abort();
	}

	cSSL = SSL_new(sslctx);
	SSL_set_fd(cSSL, sock);
	int ssl_err = SSL_connect(cSSL);

	if (ssl_err <= 0) {
		ERR_print_errors_fp(stderr);
		printf("TODO MAL\n");
		fflush(stdout);
		//ssl_err = SSL_connect(cSSL);
		SSL_shutdown(cSSL);
		SSL_free(cSSL);
		return -1;
	}

	fprintf(stderr, "Utilizando cifrado: %s\n", SSL_get_cipher_name(cSSL));
	fprintf(stderr, "Comenzando pruebas de enviar valores pequeños\n");

	uint64_t value = 0;
	gettimeofday(&start, 0);

	for (int i = 0; i < NUM_SMALL_MESSAGES; i++) {
		value = i;
		SSL_write(cSSL, (void *)&value, sizeof(uint64_t));
	}

	gettimeofday(&end, 0);


	fprintf(stderr, "Terminadas pruebas. %f gbps, %f segundos\n",
			((double)NUM_SMALL_MESSAGES * sizeof(uint64_t) * 8 / 1000) / (((double)end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec)),
			(((double)end.tv_sec - start.tv_sec) + ((double)end.tv_usec - start.tv_usec) / 1000000));




	fprintf(stderr, "Utilizando cifrado: %s\n", SSL_get_cipher_name(cSSL));
	fprintf(stderr, "Comenzando pruebas de enviar valores grandes\n");


	gettimeofday(&start, 0);

	for (int i = 0; i < NUM_BIG_MESSAGES; i++) {
		*((int *) buffer) = i;
		SSL_write(cSSL, (void *)buffer, SIZE_BUFFER);
		//tcp_message_send(sock.sockfd, (void *)buffer, SIZE_BUFFER);
	}

	gettimeofday(&end, 0);

	SSL_shutdown(cSSL);
	SSL_free(cSSL);
	close(sock);

	fprintf(stderr, "Terminadas pruebas. %f gbps, %f segundos\n",
			(((double)NUM_BIG_MESSAGES * SIZE_BUFFER * 8) / 1000) / (((double)end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec)),
			(((double)end.tv_sec - start.tv_sec) + ((double)end.tv_usec - start.tv_usec) / 1000000));

}
