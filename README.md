ultra-remote (arduino-esp32 variant)
====================================

preferring ESP-IDF? have a look at this: [fast ESP32 WiFi remote control (ESP-IDF variant)](https://github.com/sp4rkie/ultra-remote-idf)

what is it
----------

- a fast ESP32 WiFi remote control with buttons
- various sizes are supported (currently 1 - 16 buttons)

main purpose
------------

- remote control of any machine suitable to receive ascii commands (including smartphones)

basic functionality
-------------------

- wake up after key press
- connect to WiFi
- send an ascii command to a remote machine
- receive status of the remote machine
- go to deep sleep again

features
--------

- very fast
- time between key press (aka. wakeup) and status receive averages 220ms
- battery operated (CR123 A 3V)

build environment
-----------------

- [Arduino-ESP32](https://espressif-docs.readthedocs-hosted.com/projects/arduino-esp32/en/latest/index.html#)

how to build
------------

- setup Arduino-ESP32 as documented [here](https://github.com/plerup/makeEspArduino)
- then

        git clone git@github.com:sp4rkie/ultra-remote-ino
        cd ultra-remote-ino
        make esp32-2 run

