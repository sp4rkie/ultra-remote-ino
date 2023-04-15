ultra-remote (arduino-esp32 variant)
====================================

work in progress...

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

example debug output
--------------------


        Entering deep sleep
        ets Jun  8 2016 00:22:57

        rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
        configsip: 0, SPIWP:0xee
        clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
        mode:DIO, clock div:2
        load:0x3fff0030,len:1184
        load:0x40078000,len:13104
        load:0x40080400,len:3036
        entry 0x400805e4
        +
        <TP01: 64>
        Boot number: 7
        SSID: NOT EXPLICIT, REUSING NVRAM
        <TP02: 131>
        ############################################
        cmd: @beep= f:1000 c:1 t:.05 p:.25 g:-20 ^roja ^
        ...........
        stat: #[XX]#[0]#[0]#[xxx]#[0]#[0]
        <TP03: 221>                                             # <-- 221ms elapsed since keypress (wakeup)
        RSSI: -70
        key RELEASED
        cmd: @beep= f:1040 c:1 t:.05 p:.25 g:-20 ^roja ^
        ...
        stat: #[XX]#[0]#[0]#[xxx]#[0]#[0]
        Entering deep sleep

