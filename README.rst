ESP32 Current Clamp
===================

Overview
--------

Software to monitor a current clamp with an ESP32-S3.

Purpose
-------

Use an ESP32-S3 to monitor the earth leakage current of appliances.

Build
-----

This project can be built with the `ESP-IDF build system
<https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/build-system.html>`_.

Configure::

    idf.py set-target esp32s3
    idf.py menuconfig

Under "Component config" you'll find "Current Clamp" where you can configure
the WiFi network.

Build::

    idf.py build

Flash::

    idf.py flash

Usage
-----

TODO
