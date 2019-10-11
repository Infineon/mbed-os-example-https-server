# Introduction
This repository contains the code examples and demos for PSOC6 MCU family of devices bundled with connectivity.

This application connects to a Wi-Fi network as a STA (client), then starts a secure (or non-secure) web server and waits for requests. You can connect to the server using a Web browser client or a Python client or command line based clients such as wget and curl. In addition, this application demonstrates server-sent events (SSE) that can be registered as resource and periodically messages will be sent to the HTTP client.

### **Supported Platforms**
This application and it's features are supported on following Cypress platforms:
* [PSoC6 WiFi-BT Prototyping Kit (CY8CPROTO-062-4343W)](https://www.cypress.com/documentation/development-kitsboards/psoc-6-wi-fi-bt-prototyping-kit-cy8cproto-062-4343w)
* [PSoC6 WiFi-BT Pioneer Kit (CY8CKIT-062-WiFi-BT)](https://www.cypress.com/documentation/development-kitsboards/psoc-6-wifi-bt-pioneer-kit-cy8ckit-062-wifi-bt)
* CY8CKIT-062S2-43012

### Dependencies
This section provides the list of dependency libraries required for this middleware library to work.
* [ARM mbed-os stack version 5.13.4 and above](https://os.mbed.com/mbed-os/releases)
* [Cypress HTTP server Library](https://github.com/cypresssemiconductorco/http-server)
* [Cypress Connectivity Utilities Library](https://github.com/cypresssemiconductorco/connectivity-utilities)

# Instructions to build HTTP server daemon code example

1. Clone the desired code examples repository

2. Change to the application folder

3. Prepare the cloned working directory for mbed
        
        mbed config root .

4. Pull the necessary libraries and its dependencies.
This will pull mbed-os, CY HTTP server library and its internal 3rd party dependencies (if any)

        mbed deploy

5. Configure the SSID and passphrase of the desired network in the accopmpanying mbed_app.json

6. Configure the static/dynamic resources to be hosted by the HTTP server, the number of simultaneous HTTP client connections, secure vs non-secure server etc.

7. build the http_server app (.hex binary)

        mbed compile -t GCC_ARM -m CY8CPROTO_062_4343W  

8. Connect to the kit via serial port. You should observe console logs that indicate connection to network and HTTP server being launched.

