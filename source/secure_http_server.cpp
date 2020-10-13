/******************************************************************************
* File Name: secure_http_server.cpp
*
* Description: This file contains the necessary functions to start the HTTPS
* server and processes GET, POST, and PUT request from the HTTPS client.
*
*******************************************************************************
* (c) 2020, Cypress Semiconductor Corporation. All rights reserved.
*******************************************************************************
* This software, including source code, documentation and related materials
* ("Software"), is owned by Cypress Semiconductor Corporation or one of its
* subsidiaries ("Cypress") and is protected by and subject to worldwide patent
* protection (United States and foreign), United States copyright laws and
* international treaty provisions. Therefore, you may use this Software only
* as provided in the license agreement accompanying the software package from
* which you obtained this Software ("EULA").
*
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software source
* code solely for use in connection with Cypress's integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer of such
* system or application assumes all risk of such use and in doing so agrees to
* indemnify Cypress against all liability.
*******************************************************************************/

#include "mbed.h"
#include "secure_http_server.h"
#include "secure_keys.h"
#include "HTTP_server.hpp"

/*******************************************************************************
* Global Variables
********************************************************************************/
/* Default network interface. In this application,
 * it is Wi-Fi as defined in the mbed_app.json file.
 */
NetworkInterface *network;

/* Network interface object which encapsulates the underlying
 * network interface.
 */
cy_network_interface_t nw_interface;

/* Holds the security configuration such as server certificate,
 * server key, and rootCA.
 */
cy_https_server_security_info_t security_config;

/* Holds the response handler for HTTPS GET and POST request from the client. */
cy_resource_dynamic_data_t https_get_post_resource;

/* Holds the response data to a static message. */
cy_resource_dynamic_data_t https_put_resource;

/* Secure HTTP server instance. */
HTTPServer *server;

/* HTTPS new page resource name */
char resource_name[NEW_RESOURCE_NAME_LENGTH + 1] = {0};

/* Holds the current LED status. */
const char *led_status = LED_STATUS_OFF;

/* Queues the HTTPS PUT request to register new page resource in the server. */
Queue<char, sizeof(resource_name)> register_resource_queue_handle;

/* On-board User LED to toggle upon HTTPS POST request
 * from the client.
 */
DigitalOut user_led(LED1, CYBSP_LED_STATE_OFF);

/* Global variable to track number of resources registered. */
static uint32_t number_of_resources_registered = 0;

/* Holds all the URL resources along with its data. */
struct https_url_database_t
{
    char *resource_name;
    char *value;
    uint32_t length;
};

/* Initializes the URL database with first entry reserved for HTTPS root URL.
 * The default value of MAX_NUMBER_OF_HTTP_SERVER_RESOURCES is 10 as defined
 * by the HTTPS server middleware. This can be overriden by setting its value
 * in application `mbed_app.json` file. Refer the README.md for more details.
 */
https_url_database_t url_resources_db[MAX_NUMBER_OF_HTTP_SERVER_RESOURCES] =
{
    /* First entry in the URL database is reserved for HTTPS Root URL. */
    { (char *)"/", NULL, 0 }
};

/******************************************************************************
* Function Prototypes
*******************************************************************************/
static cy_rslt_t configure_https_server(void);

/*******************************************************************************
 * Function Name: dynamic_resource_handler
 *******************************************************************************
 * Summary:
 *  Handles HTTPS GET, POST, and PUT requests from the client.
 *  HTTPS GET sends the current LED status as a response to the client.
 *  HTTPS POST toggles the LED and then sends its current state as a response
 *  to the client.
 *  HTTPS PUT sends an error message as a response to the client if the resource
 *  registration is unsuccessful.
 *
 * Parameters:
 *  url_path - Pointer to the HTTPS URL path.
 *  url_parameters - Pointer to the HTTPS URL query string.
 *  stream - Pointer to the HTTPS response stream.
 *  arg - Pointer to the argument passed during HTTPS resource registration.
 *  https_message_body - Pointer to the HTTPS data from the client.
 *
 * Return:
 *  int32_t - Returns HTTPS_REQUEST_HANDLE_SUCCESS if the request from the client
 *  was handled successfully. Otherwise, it returns HTTPS_REQUEST_HANDLE_ERROR.
 *
 *******************************************************************************/
int32_t dynamic_resource_handler(const char *url_path,
                                 const char *url_parameters,
                                 cy_http_response_stream_t *stream,
                                 void *arg,
                                 cy_http_message_body_t *https_message_body)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    int32_t status = HTTPS_REQUEST_HANDLE_SUCCESS;
    char https_response[MAX_HTTP_RESPONSE_LENGTH] = {0};
    char *register_new_resource = NULL;

    switch (https_message_body->request_type)
    {
        case CY_HTTP_REQUEST_GET:
            APP_INFO(("Received HTTPS GET request.\n"));

            /* Update the current LED status. This status will be sent
             * in response to the POST request.
             */
            if (CYBSP_LED_STATE_ON == user_led)
            {
                led_status = LED_STATUS_ON;
            }
            else
            {
                led_status = LED_STATUS_OFF;
            }

            sprintf(https_response, HTTPS_STARTUP_WEBPAGE, led_status);

            /* Send the HTTPS response. */
            result = server->http_response_stream_write(stream, https_response, sizeof(https_response));

            if (CY_RSLT_SUCCESS != result)
            {
                ERR_INFO(("Failed to send the HTTPS GET response.\n"));
            }
            break;

        case CY_HTTP_REQUEST_POST:
            APP_INFO(("Received HTTPS POST request.\n"));

            /* Send the current LED status before toggling in response to the POST request.
             * The user can then send a GET request to get the latest LED status
             * on the webpage.
             */
            sprintf(https_response, HTTPS_STARTUP_WEBPAGE, led_status);

            /* Toggle the user LED. */
            user_led = !user_led;

            /* Send the HTTPS response. */
            result = server->http_response_stream_write(stream, https_response, sizeof(https_response));

            if (CY_RSLT_SUCCESS != result)
            {
                ERR_INFO(("Failed to send the HTTPS POST response.\n"));
            }
            break;

        case CY_HTTP_REQUEST_PUT:
            register_new_resource = (char *)&resource_name[0];
            if (https_message_body->data_length > sizeof(resource_name))
            {
                /* Report the error response to the client. */
                ERR_INFO(("Resource name length exceeded the limit. Maximum: %d, Received: %d",
                                    sizeof(resource_name), (https_message_body->data_length)));
                sprintf(https_response, HTTPS_RESOURCE_PUT_ERROR, sizeof(resource_name));

                /* Send the HTTPS error response. */
                result = server->http_response_stream_write(stream, https_response, sizeof(https_response));

                if (CY_RSLT_SUCCESS != result)
                {
                    ERR_INFO(("Failed to send the HTTPS PUT error response.\n"));
                }
            }
            else
            {
                memcpy(register_new_resource, (char *)https_message_body->data, https_message_body->data_length);

                /* Received HTTPS PUT request. Put the message into queue
                 * to register a new resource with the HTTPS server.
                 */
                if (false == register_resource_queue_handle.try_put_for(Kernel::wait_for_u32_forever,
                                                                              register_new_resource))
                {
                    ERR_INFO(("Failed to queue the HTTPS PUT request.\n"));
                }
            }
            break;

        default:
            ERR_INFO(("Received invalid HTTP request method. Supported HTTP methods are GET, POST, and PUT.\n"));
            break;
    }

    if (CY_RSLT_SUCCESS != result)
    {
        status = HTTPS_REQUEST_HANDLE_ERROR;
    }

    return status;
}

/*******************************************************************************
 * Function Name: https_put_resource_handler
 *******************************************************************************
 * Summary:
 *  Handles HTTPS GET request for the newly created resource by the client
 *  through HTTPS PUT request. It returns the value registered with the requested
 *  resource.
 *
 * Parameters:
 *  url_path - Pointer to the HTTPS URL path.
 *  url_parameters - Pointer to the HTTPS URL query string.
 *  stream - Pointer to the HTTPS response stream.
 *  arg - Pointer to the argument passed during HTTPS resource registration.
 *  https_message_body - Pointer to the HTTPS data from the client.
 *
 * Return:
 *  int32_t - Returns HTTPS_REQUEST_HANDLE_SUCCESS if the request from the client
 *  was handled successfully. Otherwise, it returns HTTPS_REQUEST_HANDLE_ERROR.
 *
 *******************************************************************************/
int32_t https_put_resource_handler(const char *url_path,
                                   const char *url_parameters,
                                   cy_http_response_stream_t *stream,
                                   void *arg,
                                   cy_http_message_body_t *https_message_body)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    int32_t status = HTTPS_REQUEST_HANDLE_SUCCESS;
    uint32_t index = 0;

    if (CY_HTTP_REQUEST_GET == https_message_body->request_type)
    {
        APP_INFO(("Received HTTPS GET request.\n"));

        while (index < MAX_NUMBER_OF_HTTP_SERVER_RESOURCES)
        {
            if (strcmp(url_resources_db[index].resource_name, (char *)arg) == 0)
            {
                result = server->http_response_stream_write(stream,
                                                            url_resources_db[index].value,
                                                            url_resources_db[index].length);
                break;
            }
            else
            {
                /* Get next entry. */
                index++;
            }
        }
    }

    if (CY_RSLT_SUCCESS != result)
    {
        ERR_INFO(("Failed to send the response message.\n"));
        status = HTTPS_REQUEST_HANDLE_ERROR;
    }

    return status;
}

/*******************************************************************************
 * Function Name: register_https_resource
 *******************************************************************************
 * Summary:
 *  Registers/Updates the new resource with the HTTPS server when HTTPS PUT
 *  request is received from the client.
 *
 * Parameters:
 *  register_resource_name: Pointer to the resource name to be registered 
 *   with the HTTPS server.
 *
 * Return:
 *  None
 *
 *******************************************************************************/
void register_https_resource(char *register_resource_name)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    char *url_resource_data = NULL;
    uint32_t index = 0;

    APP_INFO(("New resource to create: %s\n", register_resource_name));

    /* Configure dynamic resource handler. */
    https_put_resource.resource_handler = https_put_resource_handler;
    https_put_resource.arg = NULL;

    /* Split the URL resource name and data from the HTTPS PUT request. */
    strtok_r(register_resource_name, "=", &url_resource_data);
    APP_INFO(("New URL: %s, Response text: %s\n", register_resource_name, url_resource_data));

    /* Register the resource if it does not exist in the URL database or
     * update the resource data if the requested resource already
     * exists in the URL database.
     */
    while (index < MAX_NUMBER_OF_HTTP_SERVER_RESOURCES)
    {
        if ((NULL != url_resources_db[index].resource_name) &&
             strcmp(url_resources_db[index].resource_name, register_resource_name) == 0)
        {
            APP_INFO(("Updating the existing resource: %s\n\n", register_resource_name));
            url_resources_db[index].length = strlen(url_resource_data);
            url_resources_db[index].value = (char *)realloc(url_resources_db[index].value, (url_resources_db[index].length + 1));
            memset(url_resources_db[index].value, 0, (url_resources_db[index].length + 1));
            memcpy(url_resources_db[index].value, url_resource_data, url_resources_db[index].length);
            https_put_resource.arg = url_resources_db[index].resource_name;
            break;
        }
        else if (NULL == url_resources_db[index].resource_name)
        {
            APP_INFO(("Registering the new resource: %s\n\n", register_resource_name));
            url_resources_db[index].value = (char *)calloc(strlen(url_resource_data), sizeof(char));
            url_resources_db[index].resource_name = (char *)calloc(strlen(register_resource_name), sizeof(char));

            if ((NULL == url_resources_db[index].resource_name) ||
                (NULL == url_resources_db[index].value))
            {
                ERR_INFO(("Failed to allocate memory for URL resources.\n"));
                MBED_ASSERT(0);
            }

            /* Add the resource name, data, and data length into URL database. */
            url_resources_db[index].length = strlen(url_resource_data);
            memcpy(url_resources_db[index].resource_name, register_resource_name,
                                                 strlen(register_resource_name));
            memcpy(url_resources_db[index].value, url_resource_data,
                                                 url_resources_db[index].length);
            https_put_resource.arg = url_resources_db[index].resource_name;

            /* Register the new/updated resource with HTTPS server. */
            result = server->register_resource((uint8_t*)url_resources_db[index].resource_name,
                                               (uint8_t*)"text/html",
                                               CY_DYNAMIC_URL_CONTENT,
                                               &https_put_resource);
            PRINT_AND_ASSERT(result, "Failed to register a new resource.\n");

            /* Update the resource count. */
            number_of_resources_registered++;
            break;
        }
        else
        {
            /* Get next resource. */
            index++;
        }
    }

    if (index >= MAX_NUMBER_OF_HTTP_SERVER_RESOURCES)
    {
        ERR_INFO(("Requested resource not registered/updated. Reached Maximum "
                  "allowed number of resource registration: %d\n", MAX_NUMBER_OF_HTTP_SERVER_RESOURCES));
    }

    /* Clear the request buffer. */
    memset(resource_name, 0, sizeof(resource_name));
}

/*******************************************************************************
 * Function Name: configure_https_server
 *******************************************************************************
 * Summary:
 *  Configures the security parameters such as server certificate, private key,
 *  and the root CA certificate to start the HTTP server in secure mode. By default,
 *  it registers a dynamic URL handler to handle the HTTPS GET, POST, and PUT
 *  requests.
 *
 * Parameters:
 *  void
 *
 * Return:
 *  cy_rslt_t: Returns CY_RSLT_SUCCESS if the secure HTTP server is configured
 *  successfully, otherwise, it returns CY_RSLT_TYPE_ERROR.
 *
 *******************************************************************************/
static cy_rslt_t configure_https_server(void)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

    /*
     * Setup the necessary security configuration required for establishing
     * secure HTTP connection with the secure HTTP client. This example uses
     * a self-signed certificate and is not trusted by the browser. Browsers
     * trust only the certificates signed by a CA (certificate authority).
     * To make the browser trust the connection with this HTTP server, user
     * needs to load the server certificate into the browser.
     */
    security_config.private_key                = (uint8_t *)keySERVER_PRIVATE_KEY_PEM;
    security_config.key_length                 = strlen(keySERVER_PRIVATE_KEY_PEM);
    security_config.certificate                = (uint8_t *)keySERVER_CERTIFICATE_PEM;
    security_config.certificate_length         = strlen(keySERVER_CERTIFICATE_PEM);
    security_config.root_ca_certificate        = (uint8_t *)keyCLIENT_ROOTCA_PEM;
    security_config.root_ca_certificate_length = strlen(keyCLIENT_ROOTCA_PEM);

    /* Configure dynamic resource handler. */
    https_get_post_resource.resource_handler = dynamic_resource_handler;
    https_get_post_resource.arg = NULL;

    if (NULL == server)
    {
        /* Allocate HTTPS server object and initialize with the security configuration. */
        server = new HTTPServer(&nw_interface, HTTPS_PORT, MAX_SOCKETS, &security_config);
    }

    /* Register HTML resource */
    result = server->register_resource((uint8_t*)"/",
                                       (uint8_t*)"text/html",
                                       CY_DYNAMIC_URL_CONTENT,
                                       &https_get_post_resource);
    /* Update the resource count. */
    number_of_resources_registered++;

    return result;
}

/********************************************************************************
 * Function Name: wifi_connect
 ********************************************************************************
 * Summary:
 *  The device associates to the Access Point with given SSID, PASSWORD, and
 *  SECURITY type.
 *
 * Parameters:
 *  void
 *
 * Return:
 *  nsapi_error_t: Returns NSAPI_ERROR_OK if the Wi-Fi connection is successfully
 *  established, a network error status otherwise.
 *
 *******************************************************************************/
static nsapi_error_t wifi_connect(void)
{
    SocketAddress sock_addr;
    nsapi_error_t net_status = NSAPI_ERROR_OK;

    /* Get the Wi-Fi network interface. */
    network = NetworkInterface::get_default_instance();
    if (NULL == network)
    {
        ERR_INFO(("No Wi-Fi interface found.\n"));
        return NSAPI_ERROR_UNSUPPORTED;
    }

    /* Establish Wi-Fi connection to AP. By default, it takes the Wi-Fi SSID,
     * PASSWORD, and SECURITY configuration from mbed_app.json file.
     */
    net_status = network->connect();
    if (NSAPI_ERROR_OK != net_status)
    {
        ERR_INFO(("Failed to connect to AP: %s\n", MBED_CONF_NSAPI_DEFAULT_WIFI_SSID));
        return net_status;
    }

    APP_INFO(("Successfully connected to AP: %s\n", MBED_CONF_NSAPI_DEFAULT_WIFI_SSID));
    network->get_ip_address(&sock_addr);
    APP_INFO(("IP Addr\t: %s\n", sock_addr.get_ip_address()));

    nw_interface.object = (void*) network;
    nw_interface.type   = CY_NW_INF_TYPE_WIFI;

    return net_status;
}

/*******************************************************************************
 * Function Name: https_server_thread
 *******************************************************************************
 * Summary:
 *  Starts the HTTP server in secure mode. This example application is using a
 *  self-signed certificate which means there is no third-party certificate issuing
 *  authority involved in the authentication of the server. It is the user's
 *  responsibility to supply the necessary security configurations such as server's
 *  certificate, private key of the server, and RootCA of the server to start the
 *  HTTP server in secure mode.
 *
 * Parameters:
 *  void
 *
 * Return:
 *  None.
 *
 *******************************************************************************/
void https_server_thread(void)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    char *register_new_resource_name = NULL;
    SocketAddress sock_addr;

    /* Ensure the kit has connected to the Wi-Fi Access Point. */
    if (NSAPI_ERROR_OK != wifi_connect())
    {
        ERR_INFO(("Wi-Fi connection to AP failed.\n"));
        MBED_ASSERT(0);
    }

    /* Configure the HTTPS server with all the security parameters and
     * register a default dynamic URL handler.
     */
    result = configure_https_server();
    PRINT_AND_ASSERT(result, "Failed to configure the HTTPS server.\n");

    /* Start the HTTPS server */
    result = server->start();
    PRINT_AND_ASSERT(result, "Failed to start the HTTPS server.\n");

    /* Get IP address */
    network->get_ip_address(&sock_addr);
    APP_INFO(("HTTPS server has successfully started. The server is running at URL https://%s:%d\n\n",
              sock_addr.get_ip_address(), HTTPS_PORT));

    /* Waits for a HTTPS PUT request from the client to register a new HTTPS page resource.*/
    while(true)
    {
       if (true == register_resource_queue_handle.try_get_for(Kernel::wait_for_u32_forever,
                                                              &register_new_resource_name))
       {
           APP_INFO(("New resource name register request: %s\n", register_new_resource_name));
           register_https_resource(register_new_resource_name);
       }
    }
}


/* [] END OF FILE */

