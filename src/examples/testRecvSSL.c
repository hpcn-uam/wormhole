#include <stdio.h>
#include <common.h>
#include <assert.h>
#include <sys/time.h>
#include <stdlib.h>
#include "../async_inline.c"



#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define NUM_SMALL_MESSAGES 5000000//00
#define NUM_BIG_MESSAGES 500000//0
#define SIZE_BUFFER 1024*4

int verify_callback(int preverify_ok, X509_STORE_CTX *ctx)
{
	char    buf[256];
	X509   *err_cert;
	int     err, depth;
	//SSL    *ssl;

	err_cert = X509_STORE_CTX_get_current_cert(ctx);
	err = X509_STORE_CTX_get_error(ctx);
	depth = X509_STORE_CTX_get_error_depth(ctx);

	/*
	* Retrieve the pointer to the SSL of the connection currently treated
	* and the application specific data stored into the SSL object.
	*/
	//ssl = X509_STORE_CTX_get_ex_data(ctx, SSL_get_ex_data_X509_STORE_CTX_idx());
	X509_NAME_oneline(X509_get_subject_name(err_cert), buf, 256);

	/*
	       * Catch a too long certificate chain. The depth limit set using
	       * SSL_CTX_set_verify_depth() is by purpose set to "limit+1" so
	       * that whenever the "depth>verify_depth" condition is met, we
	       * have violated the limit and want to log this error condition.
	       * We must do it here, because the CHAIN_TOO_LONG error would not
	       * be found explicitly; only errors introduced by cutting off the
	       * additional certificates would be logged.
	       */
	if (depth > 1) {
		preverify_ok = 0;
		err = X509_V_ERR_CERT_CHAIN_TOO_LONG;
		X509_STORE_CTX_set_error(ctx, err);
	}

	if (!preverify_ok) {
		printf("verify error:num=%d:%s:depth=%d:%s\n", err,
			   X509_verify_cert_error_string(err), depth, buf);

	} else if (1) {
		printf("depth=%d:%s\n", depth, buf);
	}

	/*
	 * At this point, err contains the last verification error. We can use
	 * it for something special
	 */
	if (!preverify_ok && (err == X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT)) {
		X509_NAME_oneline(X509_get_issuer_name(ctx->current_cert), buf, 256);
		printf("issuer= %s\n", buf);
	}

	return preverify_ok;
}

int main(int argc, char **argv)
{
		UNUSED(argc);
		UNUSED(argv);
		
	void *buffer = malloc(SIZE_BUFFER);
	int listen_socket = tcp_listen_on_port(5000);
	assert(listen_socket != -1);

	int sock;

	sock = tcp_accept(listen_socket, NULL);

	struct timeval start, end;

	//INIT OPEN SSL
	SSL_load_error_strings();
	SSL_library_init();

	//tmp variables;
	SSL_CTX *sslctx;
	SSL *cSSL;

	sslctx = SSL_CTX_new(TLSv1_2_server_method());
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
		return -1;
	}

	/* Set the ciphers list */
	/*const char* whc_rc4md5 	= "RC4-MD5";	*/
	const char *whc_rc4sha 	= "RC4-SHA"; 		/*
	const char* whc_des 	= "DES-CBC-SHA";
	const char* whc_3des 	= "DES-CBC3-SHA";
	const char* whc_aes256A	= "AES256-GCM-SHA384";
	const char* whc_aes256B	= "AES256-SHA256";*/

	if (!SSL_CTX_set_cipher_list(sslctx, whc_rc4sha)) {
		fprintf(stderr, "No cipher could be selected\n");
		return -1;
	}

	/* Set to require peer (client) certificate verification */
	SSL_CTX_set_verify(sslctx, SSL_VERIFY_PEER, verify_callback);
	/* Set the verification depth to 1 */
	SSL_CTX_set_verify_depth(sslctx, 1);

	cSSL = SSL_new(sslctx);
	SSL_set_fd(cSSL, sock);
	printf("Recv Connection!\n");
	fflush(stdout);
	int ssl_err = SSL_accept(cSSL);

	if (ssl_err <= 0) {
		ERR_print_errors_fp(stderr);
		printf("TODO MAL\n");
		fflush(stdout);
		//ssl_err = SSL_accept(cSSL);
		SSL_shutdown(cSSL);
		SSL_free(cSSL);
		return -1;
	}

	fprintf(stderr, "Utilizando cifrado: %s\n", SSL_get_cipher_name(cSSL));
	fprintf(stderr, "Comenzando pruebas de recibir valores pequeños\n");

	uint64_t value;
	gettimeofday(&start, 0);

	for (uint32_t i = 0; i < NUM_SMALL_MESSAGES; i++) {
		SSL_read(cSSL, (void *)&value, sizeof(uint64_t));

		if (value != i) {
			fprintf(stderr, "Paquete perdido %d\n", i);
		}
	}

	gettimeofday(&end, 0);

	fprintf(stderr, "Terminadas pruebas. %f gbps\n",
			((double)NUM_SMALL_MESSAGES * sizeof(uint64_t) * 8 / 1000) / (((double)end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec)));


	fprintf(stderr, "Utilizando cifrado: %s\n", SSL_get_cipher_name(cSSL));
	fprintf(stderr, "Comenzando pruebas de recibir valores grandes\n");


	gettimeofday(&start, 0);

	for (int i = 0; i < NUM_BIG_MESSAGES; i++) {
		SSL_read(cSSL, (void *)buffer, SIZE_BUFFER);

		if (i > 499000) {
			//flush_recv(&sock);
		}
	}

	gettimeofday(&end, 0);
	//destroy_asyncSocket(&sock);
	SSL_shutdown(cSSL);
	SSL_free(cSSL);
	close(sock);

	fprintf(stderr, "Terminadas pruebas. %f gbps\n",
			(((double)NUM_BIG_MESSAGES * SIZE_BUFFER * 8) / 1000) / (((double)end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec)));

}
