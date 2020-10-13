/******************************************************************************
 * File Name: main.cpp
 *
 * Description:
 *   This is the main application file which starts the HTTPS server thread.
 *   The thread establishes Wi-Fi connection to the AP, and starts the HTTP
 *   server in secure mode. All the HTTP security keys are configured in the
 *   file secure_keys.h using openSSL utility.
 *   See README.md to understand how the security keys are generated.
 *
 * Related Document:
 *   README.md
 *
 ******************************************************************************
 * (c) 2020, Cypress Semiconductor Corporation. All rights reserved.
 ******************************************************************************
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
 *****************************************************************************/

#include "mbed.h"
#include "secure_http_server.h"

/* Thread handle to start an HTTPS server. */
Thread t1(osPriorityNormal, HTTPS_SERVER_TASK_STACK_SIZE);

/******************************************************************************
 * Function Name: main()
 ******************************************************************************
 * Summary:
 *   Entry function of this application. This starts a thread which connects to
 *   Wi-Fi AP and starts a secure HTTP server. The Wi-Fi credentials such as
 *   SSID, Password, and security type need to be mentioned in the
 *   mbed_app.json file.
 *
 *****************************************************************************/
int main(void)
{
    /* \x1b[2J\x1b[;H - ANSI ESC sequence to clear screen */
    APP_INFO(("\x1b[2J\x1b[;H"));
    APP_INFO(("===================================\n"));
    APP_INFO(("PSoC 6 MCU: HTTPS Server\n"));
    APP_INFO(("===================================\n\n"));

    /* Starts the HTTPS server in secure mode. */
    t1.start(callback(https_server_thread));

    return 0;
}


/* [] END OF FILE */

