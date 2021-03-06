#lufa makefile abomination

MCU          = atmega8u2
ARCH         = AVR8
BOARD        = NONE
F_CPU        = 16000000
F_USB        = $(F_CPU)
OPTIMIZATION = s
TARGET       = c_locc
SRC          = $(TARGET).c dial.c locc.c keypad.c Descriptors.c $(LUFA_SRC_USB) $(LUFA_SRC_USBCLASS)
LUFA_PATH    = lufa/LUFA
CC_FLAGS     = -DUSE_LUFA_CONFIG_HEADER
LD_FLAGS     =

# Default target
all:

# Include LUFA build script makefiles
include $(LUFA_PATH)/Build/lufa_core.mk
include $(LUFA_PATH)/Build/lufa_sources.mk
include $(LUFA_PATH)/Build/lufa_build.mk
include $(LUFA_PATH)/Build/lufa_cppcheck.mk
include $(LUFA_PATH)/Build/lufa_doxygen.mk
include $(LUFA_PATH)/Build/lufa_dfu.mk
include $(LUFA_PATH)/Build/lufa_hid.mk
include $(LUFA_PATH)/Build/lufa_avrdude.mk
include $(LUFA_PATH)/Build/lufa_atprogram.mk

program: all
	dfu-programmer $(MCU) erase
	dfu-programmer $(MCU) flash $(TARGET).hex

flash-mk2: all
	avrdude -c avrispmkII -P usb -p m8u2 -v -U flash:w:$(TARGET).hex

flash-dragon: all
	avrdude -c dragon_isp -P usb -p m8u2 -v -U flash:w:$(TARGET).hex
