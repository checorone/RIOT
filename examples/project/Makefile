# name of your application
APPLICATION = project

# If no BOARD is found in the environment, use this default:
BOARD ?= nucleo-f401

FEATURES_REQUIRED = periph_uart

USEMODULE += shell
USEMODULE += xtimer

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
