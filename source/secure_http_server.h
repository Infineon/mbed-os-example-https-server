/******************************************************************************
 * File Name: secure_http_server.h
 *
 * Description: This file contains configuration parameters for the secure HTTP
 * server along with the HTML page that the server will host.
 *
 ******************************************************************************
 * Copyright (2020), Cypress Semiconductor Corporation.
 ******************************************************************************
 * This software, including source code, documentation and related materials
 * (“Software”), is owned by Cypress Semiconductor Corporation or one of its
 * subsidiaries (“Cypress”) and is protected by and subject to worldwide patent
 * protection (United States and foreign), United States copyright laws and
 * international treaty provisions. Therefore, you may use this Software only
 * as provided in the license agreement accompanying the software package from
 * which you obtained this Software (“EULA”).
 *
 * If no EULA applies, Cypress hereby grants you a personal, nonexclusive,
 * non-transferable license to copy, modify, and compile the Software source
 * code solely for use in connection with Cypress’s integrated circuit products.
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
 * significant property damage, injury or death (“High Risk Product”). By
 * including Cypress’s product in a High Risk Product, the manufacturer of such
 * system or application assumes all risk of such use and in doing so agrees to
 * indemnify Cypress against all liability.
 *****************************************************************************/

/*******************************************************************************
* Include guard
*******************************************************************************/
#ifndef SECURE_HTTP_SERVER_H_
#define SECURE_HTTP_SERVER_H_

#define APP_INFO(x)                              do { printf("Info: "); printf x; } while(0);
#define ERR_INFO(x)                              do { printf("Error: "); printf x; } while(0);

#define PRINT_AND_ASSERT(result, msg, args...)   do                                 \
                                                 {                                  \
                                                     if (CY_RSLT_SUCCESS != result) \
                                                     {                              \
                                                         ERR_INFO((msg, ## args));  \
                                                         MBED_ASSERT(0);            \
                                                     }                              \
                                                 } while(0);

#define LED_STATUS_ON                            "ON"
#define LED_STATUS_OFF                           "OFF"
#define MAX_LED_STATUS_LENGTH                    (3)
#define HTTPS_PORT                               (50007)
#define MAX_SOCKETS                              (2)
#define NEW_RESOURCE_NAME_LENGTH                 (100)
#define HTTPS_RESOURCE_PUT_ERROR                 "Maximum resource name length is %d characters."
#define HTTPS_REQUEST_HANDLE_SUCCESS             (0)
#define HTTPS_REQUEST_HANDLE_ERROR               (-1)
#define HTTPS_SERVER_TASK_STACK_SIZE             (5 * 1024)

#define HTTPS_STARTUP_WEBPAGE \
              "<!DOCTYPE html>" \
              "<html>" \
              "<head>" \
              "<title>HTTPS Server Demo</title>" \
              "</head>" \
              "<body>" \
                "<h1>HTTPS Server Demo</h1>" \
                "<form method=\"get\">" \
                    "<fieldset>" \
                        "<legend>HTTPS GET</legend>" \
                        "<input type=\"submit\" value=\"Get LED status\"/>" \
                        "<input type=\"text\" name=\"led_status\" value=\"%s\" size=\"3\" readonly/></br></br>" \
                    "</fieldset>" \
                    "</br>" \
                "</form>" \
                "<form method=\"post\">" \
                    "<fieldset>" \
                        "<legend>HTTPS POST</legend>" \
                        "<input type=\"submit\" name=\"toggle_led\" value=\"Toggle LED\"/></br></br>" \
                    "</fieldset>" \
                    "</br>" \
                "</form>" \
              "</body>" \
              "</html>"

#define MAX_HTTP_RESPONSE_LENGTH                 (sizeof(HTTPS_STARTUP_WEBPAGE) + sizeof(MAX_LED_STATUS_LENGTH) + 1)

/*******************************************************************************
* Function Prototypes
********************************************************************************/
void https_server_thread(void);

#endif /* SECURE_HTTP_SERVER_H_ */


/* [] END OF FILE */
