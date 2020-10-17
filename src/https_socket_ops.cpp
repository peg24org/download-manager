#include <iostream>

#include "https_socket_ops.h"

using namespace std;

bool HttpsSocketOps::connect()
{
    ctx = SSL_CTX_new(TLS_client_method());
    if (SSL_CTX_set_default_verify_paths(ctx) != 1) 
      cerr << "Error setting up trust store" << endl;

    string destination = ip + ":" + to_string(port);
    bio = BIO_new_connect(destination.c_str());
    if (BIO_do_connect(bio) <= 0)
      cerr << "Error in BIO_do_connect" << endl;

    BIO_get_fd(bio, &sock_desc);
    BIO* new_bio = BIO_new_ssl(ctx, 1);
    BIO_push(new_bio, bio);

    SSL_set_tlsext_host_name(get_ssl(new_bio), ip.c_str());
    SSL_set1_host(get_ssl(new_bio), ip.c_str());
    if (BIO_do_handshake(new_bio) <= 0)
      cerr << "Error in BIO_do_handshake" << endl;

    // Verify the certificate.
    int err = SSL_get_verify_result(ssl);
    if (err != X509_V_OK)
      cerr << "Certificate verification error: " <<
              X509_verify_cert_error_string(err) << "(" << err << ")" << endl;
    X509 *cert = SSL_get_peer_certificate(ssl);
    if (cert == nullptr)
      cerr << "No certificate was presented by the server." << endl;

    bio = new_bio;
    //sock_desc = sock_desc;
    ssl = get_ssl(new_bio);

    // TODO: handle errors.
    return true;
}

SSL* HttpsSocketOps::get_ssl(BIO* bio)
{
  SSL* ssl = nullptr;

  BIO_get_ssl(bio, &ssl);
  if (ssl == nullptr)
    cerr << "Error occurred when getting ssl." << endl;

  return ssl;
}

bool HttpsSocketOps::disconnect()
{
  BIO_ssl_shutdown(bio);
  X509_free(cert);
  close(sock_desc);
  SSL_CTX_free(ctx);
  SSL_free(ssl);
  // TODO: handle errors.
  return true;
}
