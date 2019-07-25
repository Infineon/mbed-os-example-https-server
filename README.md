# Introduction
This repository contains the code examples and demos for PSOC6 MCU family of devices bundled with connectivity.

This app connects to a Wi-Fi network as a STA (client), then starts a secure (or non-secure) web server and waits for requests.

# Instructions to build HTTP server daemon code example

1. prepare the cloned working directory for mbed
        
        mbed config root .

2. pull the necessary libraries and its dependencies.
This will pull mbed-os, CY HTTP server library and its internal 3rd party dependencies (if any)

        mbed deploy

3. build the http_server app (.hex binary)

        mbed compile -t GCC_ARM -m CY8CPROTO_062_4343W  

