ultra-remote (arduino-esp32 variant)
====================================

work in progress... stay tuned

preferring ESP-IDF? have a look at this: [fast ESP32 WiFi remote control (ESP-IDF variant)](https://github.com/sp4rkie/ultra-remote-idf)

![alt text](https://github.com/sp4rkie/ultra-remote-idf/raw/main/images/shot0010.png "Title")
![alt text](https://github.com/sp4rkie/ultra-remote-idf/raw/main/images/shot0015.png "Title")

what is it
----------

- a fast ESP32 WiFi remote control with buttons
- various sizes are supported (currently 1 - 16 buttons)

project main objective
----------------------

- implement the fastest way possible to remote control a linux machine (including smartphones with [Termux](https://termux.dev/en/) and [Tasker](https://tasker.joaoapps.com/)) with an [ESP32](https://en.wikipedia.org/wiki/ESP32) device over WiFi 

project status as of current git (see protocol output)
------------------------------------------------------

- *220ms* average time span between keypress (wakeup) to status (received from remote machine)
- any pull requests to improve the current results are very welcome

basic functionality
-------------------

- wake up after key press
- connect to WiFi
- send an ascii command to a remote machine
- receive status from the remote machine
- go to deep sleep again

features
--------

- very fast (see project status)
- very small (see [profile picture](https://avatars.githubusercontent.com/u/3232165?v=4))
- easy to build (documentation will follow)
- battery operated (CR123 A 3V)
- OTAable (press key before inserting the battery)

runtime environment
-------------------

- access point hardware in use: Raspberry Pi 4 Model B Rev 1.4
- access point software in use: debian version 11.6/ hostapd version 2:2.9.0-21
- remote target software to control: ucspi-tcp version 1:0.88-6

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

debug protocol output as of current git
---------------------------------------

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
        <TP01: 45>
        Boot number: 6
        SSID: NOT EXPLICIT, REUSING NVRAM
        <TP02: 91>
        ############################################################
        cmd: zp ^
        .......................
        stat: #[XX]#[0]#[0]#[xxx]#[0]#[0]
        <TP03: 212>                        # <-- STATUS reveived - 212ms overall elapsed time between keypress (wakeup) to status (received from remote machine)
        key RELEASED
        Entering deep sleep

