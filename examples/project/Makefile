# name of your application
APPLICATION = project

LINKFLAGS += -lc -lrdimon -u _printf_float

# If no BOARD is found in the environment, use this default:
BOARD ?= nucleo-f401

FEATURES_REQUIRED += periph_i2c
FEATURES_REQUIRED += periph_uart
FEATURES_REQUIRED += periph_adc
FEATURES_REQUIRED += periph_gpio

USEMODULE += shell
USEMODULE += xtimer

EXTERNAL_MODULE_DIRS += $(CURDIR)/thirdparty/jsonlib
USEMODULE += jsonlib
INCLUDES += -I$(CURDIR)/thirdparty/jsonlib

EXTERNAL_MODULE_DIRS += $(CURDIR)/thirdparty/mqttlib
USEMODULE += libmqttlib
INCLUDES += -I$(CURDIR)/thirdparty/mqttlib

EXTERNAL_MODULE_DIRS += $(CURDIR)/src
USEMODULE += src
INCLUDES += -I$(CURDIR)/include

USEPKG += jsmn
INCLUDE += $(RIOTPKG)/jsmn/...

USEPKG += minmea
INCLUDE += $(RIOTPKG)/minmea/...

# This has to be the absolute path to the RIOT base directory:
RIOTBASE = $(CURDIR)/../../

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

#export INCLUDES += -Iapplication_include

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include
