#include "https_downloader.h"

#include <unistd.h>

#include <arpa/inet.h>
#include <openssl/err.h>

#include <cstring>
#include <iostream>

using namespace std;

HttpsDownloader::~HttpsDownloader()
{
}

HttpsDownloader::HttpsDownloader(const struct DownloadSource& download_source,
                                 std::vector<int>& socket_descriptors,
                                 std::unique_ptr<Writer> writer,
                                 ChunksCollection& chunks_collection)
  : HttpDownloader(download_source, socket_descriptors, move(writer),
                   chunks_collection)
{
  ssl_init_sockets();
}

HttpsDownloader::HttpsDownloader(const struct DownloadSource& download_source,
                                 const std::vector<int>& socket_descriptors)
  : HttpDownloader(download_source, socket_descriptors)
{
  ssl_init_sockets();
}

SSL* HttpsDownloader::get_ssl(BIO* bio)
{
  SSL* ssl = nullptr;
  BIO_get_ssl(bio, &ssl);
  if (ssl == nullptr)
    cerr << "Error occured when getting ssl." << endl;
  return ssl;
}

void HttpsDownloader::ssl_init_sockets()
{
  for (size_t index = 0; index < socket_descriptors.size(); ++index) {
    SSL_CTX* ctx;
    ctx = SSL_CTX_new(TLS_client_method());
    if (SSL_CTX_set_default_verify_paths(ctx) != 1) 
      cerr << "Error setting up trust store" << endl;

    string host_name = download_source.host_name;
    uint16_t port = download_source.port; 
    string destination = host_name + ":" + to_string(port);
    BIO* bio = BIO_new_connect(destination.c_str());
    if (BIO_do_connect(bio) <= 0)
      cerr << "Error in BIO_do_connect" << endl;

    int sock_desc;
    BIO_get_fd(bio, &sock_desc);
    BIO* new_bio = BIO_new_ssl(ctx, 1);
    BIO_push(new_bio, bio);

    SSL_set_tlsext_host_name(get_ssl(new_bio), host_name.c_str());
    SSL_set1_host(get_ssl(new_bio), host_name.c_str());
    if (BIO_do_handshake(new_bio) <= 0)
      cerr << "Error in BIO_do_handshake" << endl;

    verify_the_certificate(get_ssl(new_bio));

    connections[index].bio = new_bio;
    connections[index].sock_desc = sock_desc;
    connections[index].ssl = get_ssl(new_bio);
  }
}

void HttpsDownloader::verify_the_certificate(SSL *ssl)
{
  int err = SSL_get_verify_result(ssl);
  if (err != X509_V_OK) {
    const char *message = X509_verify_cert_error_string(err);
    cerr << "Certificate verification error: " << message <<
      "(" << err << ")" << endl;
  }
  X509 *cert = SSL_get_peer_certificate(ssl);
  if (cert == nullptr)
    cerr << "No certificate was presented by the server." << endl;
}  

bool HttpsDownloader::receive_data(Connection& connection, char* buffer,
                                   size_t& received_len, size_t buffer_capacity)
{
  bool retval = false;

  int len = SSL_read(connection.ssl, buffer, buffer_capacity);
  if (len < 0)
    cerr << "error SSL_read" << endl;
  else if (len > 0) 
    retval = true;
  else 
    cerr << "empty BIO_read" << endl;

  received_len = len;

  return retval;
}

bool HttpsDownloader::send_data(Connection& connection, const char* buffer,
                                size_t len)
{
  BIO_write(connection.bio, buffer, len);
  BIO_flush(connection.bio);
  return true;
}
