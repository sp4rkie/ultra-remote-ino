MACHINE != uname -n
ifeq ($(MACHINE),roja)
    FLAGS = -DCFG_LOCAL
endif
ifneq ($(findstring esp32-2 ,$(MAKECMDGOALS)),)
    OTA_ADDR = esp32-2
    ultra_remote.ino.cpp_CFLAGS = -DESP32_2 -fpermissive $(FLAGS)
#    FLASH_DEF = 16M
else ifneq ($(findstring esp32-9 ,$(MAKECMDGOALS)),)
    OTA_ADDR = esp32-9
    ultra_remote.ino.cpp_CFLAGS = -DESP32_9 -fpermissive $(FLAGS)
#    FLASH_DEF = 16M
endif
ifeq ($(OTA_ADDR),)
    $(error params wrong or missing: make <machine> <target>)
endif
ifneq ($(findstring ota,$(MAKECMDGOALS)),)
ifeq ($(MACHINE),roja)
    OTA_HPORT != esp32_time_wait
endif
    $(info OTA_ADDR: $(OTA_ADDR))
    $(info OTA_HPORT: $(OTA_HPORT))
endif

CHIP = esp32
BOARD = esp32
UPLOAD_SPEED = 921600
#1
UPLOAD_PORT = /dev/serial/by-id/usb-Silicon_Labs_CP2104_USB_to_UART_Bridge_Controller_0255CF15-if00-port0
#2
#UPLOAD_PORT = /dev/serial/by-id/usb-Silicon_Labs_CP2104_USB_to_UART_Bridge_Controller_02EXVH5I-if00-port0

ESP_ROOT = $(HOME)/esp32
LIBS = $(HOME)/Arduino/libraries
include $(HOME)/makeEspArduino/makeEspArduino.mk
BUILD_OPT_H := $(shell touch $(BUILD_DIR)/build_opt.h)  # work around https://github.com/plerup/makeEspArduino/issues/189

esp32-2: FRC
esp32-9: FRC

# must recompile it always since last flags were unknown
FRC:
	touch ultra_remote.ino

prune: clean FRC
	rm -frv cscope.out /tmp/mkESP

