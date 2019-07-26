/*
 * Copyright 2019 Cypress Semiconductor Corporation
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/** @file
 *
 * Reference code example for HTTP server
 */

#include "mbed.h"
#include "HTTP_server.h"

#ifdef __cplusplus
extern "C" {
#endif

#define APP_INFO( x )     printf x

#define STATIC_RESOURCE
#define DYNAMIC_RESOURCE

/* Enable below flag for HTTP server */
#define HTTP_SERVER

/* Enable below flag for HTTPS server and comment out the above flag */
//#define HTTPS_SERVER

/* Note : Currently, the BUFFER_LENGTH dictates the amount of memory needed by the application to store received chunk(s) of data.
 * The data receive callback will be iteratively invoked per MTU. Depending on the size of data, the BUFFER_LENGTH can be
 * suitably increased.
 * For memory constrained devices (such as CY8CKIT_062_WIFI_BT), it is recommended to not increase the buffer length beyond MTU size,
 * rather to transfer each received chunk in memory to flash, prior to receiving the next chunk.
 */
#define BUFFER_LENGTH     1500

#define HTTP_PORT         80
#define HTTPS_PORT        443
/* Note: On memory constrained devices (such as CY8CKIT_062_WIFI_BT), there could be limitation in supporting more than one secure connection.
 * Each secure TLS connection, needs 33KB of heap
 */
#define MAX_SOCKETS       2

/* Certificates provided in this file are for test purposes only.
 * Create valid certificates for production */
#ifdef HTTPS_SERVER
#ifdef ECDHE_ECDSA
/* Server certificate, Private key & RootCA certificate */
const char SSL_SERVERCERT_PEM[] =  "-----BEGIN CERTIFICATE-----\n" \
        "MIICHzCCAaWgAwIBAgIBCTAKBggqhkjOPQQDAjA+MQswCQYDVQQGEwJOTDERMA8G\n" \
        "A1UEChMIUG9sYXJTU0wxHDAaBgNVBAMTE1BvbGFyc3NsIFRlc3QgRUMgQ0EwHhcN\n" \
        "MTMwOTI0MTU1MjA0WhcNMjMwOTIyMTU1MjA0WjA0MQswCQYDVQQGEwJOTDERMA8G\n" \
        "A1UEChMIUG9sYXJTU0wxEjAQBgNVBAMTCWxvY2FsaG9zdDBZMBMGByqGSM49AgEG\n" \
        "CCqGSM49AwEHA0IABDfMVtl2CR5acj7HWS3/IG7ufPkGkXTQrRS192giWWKSTuUA\n" \
        "2CMR/+ov0jRdXRa9iojCa3cNVc2KKg76Aci07f+jgZ0wgZowCQYDVR0TBAIwADAd\n" \
        "BgNVHQ4EFgQUUGGlj9QH2deCAQzlZX+MY0anE74wbgYDVR0jBGcwZYAUnW0gJEkB\n" \
        "PyvLeLUZvH4kydv7NnyhQqRAMD4xCzAJBgNVBAYTAk5MMREwDwYDVQQKEwhQb2xh\n" \
        "clNTTDEcMBoGA1UEAxMTUG9sYXJzc2wgVGVzdCBFQyBDQYIJAMFD4n5iQ8zoMAoG\n" \
        "CCqGSM49BAMCA2gAMGUCMQCaLFzXptui5WQN8LlO3ddh1hMxx6tzgLvT03MTVK2S\n" \
        "C12r0Lz3ri/moSEpNZWqPjkCMCE2f53GXcYLqyfyJR078c/xNSUU5+Xxl7VZ414V\n" \
        "fGa5kHvHARBPc8YAIVIqDvHH1Q==\n" \
        "-----END CERTIFICATE-----\n";

const char SSL_SERVERKEY_PEM[] = "-----BEGIN EC PRIVATE KEY-----\n" \
        "MHcCAQEEIPEqEyB2AnCoPL/9U/YDHvdqXYbIogTywwyp6/UfDw6noAoGCCqGSM49\n" \
        "AwEHoUQDQgAEN8xW2XYJHlpyPsdZLf8gbu58+QaRdNCtFLX3aCJZYpJO5QDYIxH/\n" \
        "6i/SNF1dFr2KiMJrdw1VzYoqDvoByLTt/w==\n" \
        "-----END EC PRIVATE KEY-----\n";

const char SSL_ROOTCA_PEM[] = "-----BEGIN CERTIFICATE-----\n"             \
        "MIICUjCCAdegAwIBAgIJAMFD4n5iQ8zoMAoGCCqGSM49BAMCMD4xCzAJBgNVBAYT\n"  \
        "Ak5MMREwDwYDVQQKEwhQb2xhclNTTDEcMBoGA1UEAxMTUG9sYXJzc2wgVGVzdCBF\n"  \
        "QyBDQTAeFw0xMzA5MjQxNTQ5NDhaFw0yMzA5MjIxNTQ5NDhaMD4xCzAJBgNVBAYT\n"  \
        "Ak5MMREwDwYDVQQKEwhQb2xhclNTTDEcMBoGA1UEAxMTUG9sYXJzc2wgVGVzdCBF\n"  \
        "QyBDQTB2MBAGByqGSM49AgEGBSuBBAAiA2IABMPaKzRBN1gvh1b+/Im6KUNLTuBu\n"  \
        "ww5XUzM5WNRStJGVOQsj318XJGJI/BqVKc4sLYfCiFKAr9ZqqyHduNMcbli4yuiy\n"  \
        "aY7zQa0pw7RfdadHb9UZKVVpmlM7ILRmFmAzHqOBoDCBnTAdBgNVHQ4EFgQUnW0g\n"  \
        "JEkBPyvLeLUZvH4kydv7NnwwbgYDVR0jBGcwZYAUnW0gJEkBPyvLeLUZvH4kydv7\n"  \
        "NnyhQqRAMD4xCzAJBgNVBAYTAk5MMREwDwYDVQQKEwhQb2xhclNTTDEcMBoGA1UE\n"  \
        "AxMTUG9sYXJzc2wgVGVzdCBFQyBDQYIJAMFD4n5iQ8zoMAwGA1UdEwQFMAMBAf8w\n"  \
        "CgYIKoZIzj0EAwIDaQAwZgIxAMO0YnNWKJUAfXgSJtJxexn4ipg+kv4znuR50v56\n"  \
        "t4d0PCu412mUC6Nnd7izvtE2MgIxAP1nnJQjZ8BWukszFQDG48wxCCyci9qpdSMv\n"  \
        "uCjn8pwUOkABXK8Mss90fzCfCEOtIA==\n"                                  \
        "-----END CERTIFICATE-----\n";
#endif

const char SSL_SERVERCERT_PEM[] =  "-----BEGIN CERTIFICATE-----\n"               \
        "MIIDNzCCAh+gAwIBAgIBAjANBgkqhkiG9w0BAQUFADA7MQswCQYDVQQGEwJOTDER\n"     \
        "MA8GA1UEChMIUG9sYXJTU0wxGTAXBgNVBAMTEFBvbGFyU1NMIFRlc3QgQ0EwHhcN\n"     \
        "MTEwMjEyMTQ0NDA2WhcNMjEwMjEyMTQ0NDA2WjA0MQswCQYDVQQGEwJOTDERMA8G\n"     \
        "A1UEChMIUG9sYXJTU0wxEjAQBgNVBAMTCWxvY2FsaG9zdDCCASIwDQYJKoZIhvcN\n"     \
        "AQEBBQADggEPADCCAQoCggEBAMFNo93nzR3RBNdJcriZrA545Do8Ss86ExbQWuTN\n"     \
        "owCIp+4ea5anUrSQ7y1yej4kmvy2NKwk9XfgJmSMnLAofaHa6ozmyRyWvP7BBFKz\n"     \
        "NtSj+uGxdtiQwWG0ZlI2oiZTqqt0Xgd9GYLbKtgfoNkNHC1JZvdbJXNG6AuKT2kM\n"     \
        "tQCQ4dqCEGZ9rlQri2V5kaHiYcPNQEkI7mgM8YuG0ka/0LiqEQMef1aoGh5EGA8P\n"     \
        "hYvai0Re4hjGYi/HZo36Xdh98yeJKQHFkA4/J/EwyEoO79bex8cna8cFPXrEAjya\n"     \
        "HT4P6DSYW8tzS1KW2BGiLICIaTla0w+w3lkvEcf36hIBMJcCAwEAAaNNMEswCQYD\n"     \
        "VR0TBAIwADAdBgNVHQ4EFgQUpQXoZLjc32APUBJNYKhkr02LQ5MwHwYDVR0jBBgw\n"     \
        "FoAUtFrkpbPe0lL2udWmlQ/rPrzH/f8wDQYJKoZIhvcNAQEFBQADggEBAJxnXClY\n"     \
        "oHkbp70cqBrsGXLybA74czbO5RdLEgFs7rHVS9r+c293luS/KdliLScZqAzYVylw\n"     \
        "UfRWvKMoWhHYKp3dEIS4xTXk6/5zXxhv9Rw8SGc8qn6vITHk1S1mPevtekgasY5Y\n"     \
        "iWQuM3h4YVlRH3HHEMAD1TnAexfXHHDFQGe+Bd1iAbz1/sH9H8l4StwX6egvTK3M\n"     \
        "wXRwkKkvjKaEDA9ATbZx0mI8LGsxSuCqe9r9dyjmttd47J1p1Rulz3CLzaRcVIuS\n"     \
        "RRQfaD8neM9c1S/iJ/amTVqJxA1KOdOS5780WhPfSArA+g4qAmSjelc3p4wWpha8\n"     \
        "zhuYwjVuX6JHG0c=\n"                                                     \
        "-----END CERTIFICATE-----\n";

const char SSL_SERVERKEY_PEM[] = "-----BEGIN RSA PRIVATE KEY-----\n"          \
        "MIIEpAIBAAKCAQEAwU2j3efNHdEE10lyuJmsDnjkOjxKzzoTFtBa5M2jAIin7h5r\n"  \
        "lqdStJDvLXJ6PiSa/LY0rCT1d+AmZIycsCh9odrqjObJHJa8/sEEUrM21KP64bF2\n"  \
        "2JDBYbRmUjaiJlOqq3ReB30Zgtsq2B+g2Q0cLUlm91slc0boC4pPaQy1AJDh2oIQ\n"  \
        "Zn2uVCuLZXmRoeJhw81ASQjuaAzxi4bSRr/QuKoRAx5/VqgaHkQYDw+Fi9qLRF7i\n"  \
        "GMZiL8dmjfpd2H3zJ4kpAcWQDj8n8TDISg7v1t7HxydrxwU9esQCPJodPg/oNJhb\n"  \
        "y3NLUpbYEaIsgIhpOVrTD7DeWS8Rx/fqEgEwlwIDAQABAoIBAQCXR0S8EIHFGORZ\n"  \
        "++AtOg6eENxD+xVs0f1IeGz57Tjo3QnXX7VBZNdj+p1ECvhCE/G7XnkgU5hLZX+G\n"  \
        "Z0jkz/tqJOI0vRSdLBbipHnWouyBQ4e/A1yIJdlBtqXxJ1KE/ituHRbNc4j4kL8Z\n"  \
        "/r6pvwnTI0PSx2Eqs048YdS92LT6qAv4flbNDxMn2uY7s4ycS4Q8w1JXnCeaAnYm\n"  \
        "WYI5wxO+bvRELR2Mcz5DmVnL8jRyml6l6582bSv5oufReFIbyPZbQWlXgYnpu6He\n"  \
        "GTc7E1zKYQGG/9+DQUl/1vQuCPqQwny0tQoX2w5tdYpdMdVm+zkLtbajzdTviJJa\n"  \
        "TWzL6lt5AoGBAN86+SVeJDcmQJcv4Eq6UhtRr4QGMiQMz0Sod6ettYxYzMgxtw28\n"  \
        "CIrgpozCc+UaZJLo7UxvC6an85r1b2nKPCLQFaggJ0H4Q0J/sZOhBIXaoBzWxveK\n"  \
        "nupceKdVxGsFi8CDy86DBfiyFivfBj+47BbaQzPBj7C4rK7UlLjab2rDAoGBAN2u\n"  \
        "AM2gchoFiu4v1HFL8D7lweEpi6ZnMJjnEu/dEgGQJFjwdpLnPbsj4c75odQ4Gz8g\n"  \
        "sw9lao9VVzbusoRE/JGI4aTdO0pATXyG7eG1Qu+5Yc1YGXcCrliA2xM9xx+d7f+s\n"  \
        "mPzN+WIEg5GJDYZDjAzHG5BNvi/FfM1C9dOtjv2dAoGAF0t5KmwbjWHBhcVqO4Ic\n"  \
        "BVvN3BIlc1ue2YRXEDlxY5b0r8N4XceMgKmW18OHApZxfl8uPDauWZLXOgl4uepv\n"  \
        "whZC3EuWrSyyICNhLY21Ah7hbIEBPF3L3ZsOwC+UErL+dXWLdB56Jgy3gZaBeW7b\n"  \
        "vDrEnocJbqCm7IukhXHOBK8CgYEAwqdHB0hqyNSzIOGY7v9abzB6pUdA3BZiQvEs\n"  \
        "3LjHVd4HPJ2x0N8CgrBIWOE0q8+0hSMmeE96WW/7jD3fPWwCR5zlXknxBQsfv0gP\n"  \
        "3BC5PR0Qdypz+d+9zfMf625kyit4T/hzwhDveZUzHnk1Cf+IG7Q+TOEnLnWAWBED\n"  \
        "ISOWmrUCgYAFEmRxgwAc/u+D6t0syCwAYh6POtscq9Y0i9GyWk89NzgC4NdwwbBH\n"  \
        "4AgahOxIxXx2gxJnq3yfkJfIjwf0s2DyP0kY2y6Ua1OeomPeY9mrIS4tCuDQ6LrE\n"  \
        "TB6l9VGoxJL4fyHnZb8L5gGvnB1bbD8cL6YPaDiOhcRseC9vBiEuVg==\n"          \
        "-----END RSA PRIVATE KEY-----\n";

const char SSL_ROOTCA_PEM[] = "-----BEGIN CERTIFICATE-----\r\n"                 \
        "MIIDhzCCAm+gAwIBAgIBADANBgkqhkiG9w0BAQUFADA7MQswCQYDVQQGEwJOTDER\r\n"  \
        "MA8GA1UEChMIUG9sYXJTU0wxGTAXBgNVBAMTEFBvbGFyU1NMIFRlc3QgQ0EwHhcN\r\n"  \
        "MTEwMjEyMTQ0NDAwWhcNMjEwMjEyMTQ0NDAwWjA7MQswCQYDVQQGEwJOTDERMA8G\r\n"  \
        "A1UEChMIUG9sYXJTU0wxGTAXBgNVBAMTEFBvbGFyU1NMIFRlc3QgQ0EwggEiMA0G\r\n"  \
        "CSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDA3zf8F7vglp0/ht6WMn1EpRagzSHx\r\n"  \
        "mdTs6st8GFgIlKXsm8WL3xoemTiZhx57wI053zhdcHgH057Zk+i5clHFzqMwUqny\r\n"  \
        "50BwFMtEonILwuVA+T7lpg6z+exKY8C4KQB0nFc7qKUEkHHxvYPZP9al4jwqj+8n\r\n"  \
        "YMPGn8u67GB9t+aEMr5P+1gmIgNb1LTV+/Xjli5wwOQuvfwu7uJBVcA0Ln0kcmnL\r\n"  \
        "R7EUQIN9Z/SG9jGr8XmksrUuEvmEF/Bibyc+E1ixVA0hmnM3oTDPb5Lc9un8rNsu\r\n"  \
        "KNF+AksjoBXyOGVkCeoMbo4bF6BxyLObyavpw/LPh5aPgAIynplYb6LVAgMBAAGj\r\n"  \
        "gZUwgZIwDAYDVR0TBAUwAwEB/zAdBgNVHQ4EFgQUtFrkpbPe0lL2udWmlQ/rPrzH\r\n"  \
        "/f8wYwYDVR0jBFwwWoAUtFrkpbPe0lL2udWmlQ/rPrzH/f+hP6Q9MDsxCzAJBgNV\r\n"  \
        "BAYTAk5MMREwDwYDVQQKEwhQb2xhclNTTDEZMBcGA1UEAxMQUG9sYXJTU0wgVGVz\r\n"  \
        "dCBDQYIBADANBgkqhkiG9w0BAQUFAAOCAQEAuP1U2ABUkIslsCfdlc2i94QHHYeJ\r\n"  \
        "SsR4EdgHtdciUI5I62J6Mom+Y0dT/7a+8S6MVMCZP6C5NyNyXw1GWY/YR82XTJ8H\r\n"  \
        "DBJiCTok5DbZ6SzaONBzdWHXwWwmi5vg1dxn7YxrM9d0IjxM27WNKs4sDQhZBQkF\r\n"  \
        "pjmfs2cb4oPl4Y9T9meTx/lvdkRYEug61Jfn6cA+qHpyPYdTH+UshITnmp5/Ztkf\r\n"  \
        "m/UTSLBNFNHesiTZeH31NcxYGdHSme9Nc/gfidRa0FLOCfWxRlFqAI47zG9jAQCZ\r\n"  \
        "7Z2mCGDNMhjQc+BYcdnl0lPXjdDK6V0qCg1dVewhUBcW5gZKzV7e9+DpVA==\r\n"      \
        "-----END CERTIFICATE-----\r\n";
#endif

/* HTML page sent in response to client */
#define CAPTIVE_PORTAL_REDIRECT_PAGE \
"<html><head>" \
"Hello, This is a reference HTTP server application" \
"</head></html>"


NetworkInterface*      network;
HTTPServer*            server;
cy_network_interface_t nw_interface;

char buffer[BUFFER_LENGTH];
int length = 0;

static int32_t process_handler( const char* url_path, const char* url_parameters, cy_http_response_stream_t* stream, void* arg, cy_http_message_body_t* http_message_body )
{
    memset(buffer, 0, BUFFER_LENGTH);

    memcpy( buffer, http_message_body->data, http_message_body->data_length);
    APP_INFO(("total_message_data_remaining : %ld \n", http_message_body->data_remaining));
    APP_INFO(("Received message_data_length : %d \n", http_message_body->data_length));

    length += http_message_body->data_length;

    APP_INFO(("%.*s \n", http_message_body->data_length, buffer));

    if (http_message_body->data_remaining == 0)
    {
        APP_INFO(("\r\n Received all the data of %d length \r\n", length));
        APP_INFO((" \n"));

        server->http_response_stream_write(stream, CAPTIVE_PORTAL_REDIRECT_PAGE, sizeof(CAPTIVE_PORTAL_REDIRECT_PAGE));
        length = 0;
        memset(buffer, 0, BUFFER_LENGTH);
    }

    return 0;
}

int main(void)
{

    APP_INFO(("Connecting to the network using Wifi...\r\n"));
    network = NetworkInterface::get_default_instance();

    nsapi_error_t net_status = -1;
    for (int tries = 0; tries < 3; tries++)
    {
        net_status = network->connect();
        if (net_status == NSAPI_ERROR_OK)
        {
            break;
        }
        else
        {
            APP_INFO(("Unable to connect to network. Retrying...\r\n"));
        }
    }

    if (net_status != NSAPI_ERROR_OK)
    {
        APP_INFO(("ERROR: Connecting to the network failed (%d)!\r\n", net_status));
        return -1;
    }

    APP_INFO(("Connected to the network successfully. IP address: %s\n", network->get_ip_address()));

    nw_interface.object = (void*) network;
    nw_interface.type   = CY_NW_INF_TYPE_WIFI;

#ifdef HTTP_SERVER
    /* Create instance of HTTP server */
    server = new HTTPServer(&nw_interface, HTTP_PORT, MAX_SOCKETS);
#endif

#ifdef HTTPS_SERVER
    cy_https_server_security_info_t security_cred;

    security_cred.certificate                = (uint8_t*) SSL_SERVERCERT_PEM;
    security_cred.certificate_length         = strlen(SSL_SERVERCERT_PEM);
    security_cred.private_key                = (uint8_t*) SSL_SERVERKEY_PEM;
    security_cred.key_length                 = strlen(SSL_SERVERKEY_PEM);
    security_cred.root_ca_certificate        = NULL;
    security_cred.root_ca_certificate_length = 0;

    server = new HTTPServer(&nw_interface, HTTPS_PORT, MAX_SOCKETS, &security_cred);
#endif

#ifdef STATIC_RESOURCE
    /* Register a static resource with HTTP server */
    cy_resource_static_data_t static_resource;
    static_resource.data = CAPTIVE_PORTAL_REDIRECT_PAGE;
    static_resource.length = sizeof(CAPTIVE_PORTAL_REDIRECT_PAGE);
    server->register_resource((uint8_t*) "/get", (uint8_t*)"text/html", CY_STATIC_URL_CONTENT, &static_resource);
#endif

#ifdef DYNAMIC_RESOURCE
    /* Register a dynamic resource with HTTP server */
    cy_resource_dynamic_data_t dynamic_resource;
    dynamic_resource.resource_handler = process_handler;
    dynamic_resource.arg = NULL;

    server->register_resource((uint8_t*) "/post", (uint8_t*)"text/html", CY_DYNAMIC_URL_CONTENT, &dynamic_resource);
#endif

    /* Start HTTP server */
    server->start();

    APP_INFO(("HTTP server started \n"));

    /* This is daemon so HTTP server is always running. if you want to stop HTTP server, then call stop() API. */
}

#ifdef __cplusplus
} /* extern "C" */
#endif
