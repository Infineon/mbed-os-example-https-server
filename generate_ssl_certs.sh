#!/bin/sh

################################
# Become a Certificate Authority
################################

HTTPS_SERVER_IP_ADDRESS=$1
MY_DOMAIN_NAME=mysecurehttpserver.local

echo "HTTPS Server IP Address: $HTTPS_SERVER_IP_ADDRESS"
_OS=$(uname -s)
echo "Environment: $_OS"

if [[ "$_OS" == "MINGW"* ]]; then
    OPENSSL_SUBJECT_INFO="//C=IN\ST=Karnataka\L=Bengaluru\O=CY\OU=Engineering\CN=$MY_DOMAIN_NAME"
else
    OPENSSL_SUBJECT_INFO="/C=IN/ST=Karnataka/L=Bengaluru/O=CY/OU=Engineering/CN=$MY_DOMAIN_NAME"
fi

# Generate a private root key
openssl genrsa -out rootCA.key 2048

# Self-sign a certificate.
# When the application is running mDNS, make sure to set the
# "Common Name" field to match your server name defined in the application.
openssl req -x509 -new -nodes -key rootCA.key -sha256 \
-days 3650 -out rootCA.crt -subj $OPENSSL_SUBJECT_INFO

########################
# Create CA-signed certs
########################

# Generate a private key
openssl genrsa -out $MY_DOMAIN_NAME.key 2048

# Create the Certificate Signing Request (CSR).
# Make sure to set the "Common Name" field with MY_DOMAIN_NAME.
openssl req -new -key $MY_DOMAIN_NAME.key -out $MY_DOMAIN_NAME.csr \
-subj $OPENSSL_SUBJECT_INFO

# Create a config file for the extensions
>$MY_DOMAIN_NAME.ext cat <<-EOF
authorityKeyIdentifier=keyid,issuer
basicConstraints=CA:FALSE
keyUsage = digitalSignature, nonRepudiation, keyEncipherment, dataEncipherment
subjectAltName = @alt_names
[alt_names]
DNS.1 = $MY_DOMAIN_NAME
IP.1 = $HTTPS_SERVER_IP_ADDRESS
EOF

# Create the signed certificate
openssl x509 -req -in $MY_DOMAIN_NAME.csr -CA rootCA.crt -CAkey rootCA.key \
-CAcreateserial -out $MY_DOMAIN_NAME.crt -days 3650 -sha256 -extfile $MY_DOMAIN_NAME.ext

################################
# Generate Client Certificate
################################
MY_CLIENT=mysecurehttpclient

# Generating RSA Private Key for Client Certificate
openssl genrsa -out $MY_CLIENT.key 2048

# Generating Certificate Signing Request for Client Certificate
openssl req -new -key $MY_CLIENT.key -out $MY_CLIENT.csr \
-subj $OPENSSL_SUBJECT_INFO

# Generating Certificate for Client Certificate
openssl x509 -req -in $MY_CLIENT.csr -CA rootCA.crt -CAkey rootCA.key -CAcreateserial \
-out $MY_CLIENT.crt -days 3650 -sha256 -extfile $MY_DOMAIN_NAME.ext

# Bundle the client certificate and key.
# Export password is set to empty.
openssl pkcs12 -export -out $MY_CLIENT.pfx -inkey $MY_CLIENT.key -in $MY_CLIENT.crt \
-passout pass:

# We have now successfully generated the server and client certificates.
# Configure your server with the generated certificate, key, and rootCA.
# Configure your client by importing the generated PKCS12 file that
# bundles the client certificate and key.
#
# Remove the intermediate files.
rm rootCA.srl $MY_DOMAIN_NAME.csr $MY_DOMAIN_NAME.ext $MY_CLIENT.csr

echo "Done"
