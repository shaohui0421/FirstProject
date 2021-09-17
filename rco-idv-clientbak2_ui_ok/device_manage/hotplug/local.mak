#
# Module Makefile variables definition.
# Modified area: add module files here to do compiling.
#

#
# Module app name
# APP output file name, that will installed into system bin dir.
#
APP_TARGET = hotplug_daemon

# Module extra FLAGS.
EXTRA_CFLAGS :=
EXTRA_LDFLAGS :=

# options only for cpp compiling.
override ADD_CPPFLAGS += -std=c++11 -Wall -finline-functions

# Defined for submodules, to be compiled as IDV Client modules, ignoring IDV unused codes.
#EXTRA_CFLAGS += -DIDV_CLIENT

# options for debug or unit test.
#ifeq ("$(CONFIG_DEBUG)", "y")
#EXTRA_CFLAGS += -DUNIT_TEST
#endif

# Module include path, separate with while space.
# e.g.
# $(APP_TOPDIR)/include
#
ADD_INCLUDE = $(shell pwd)/../include

# specify link libs.
#ADD_LIB = -lrc_base -lpthread `pkg-config gtk+-2.0 gthread-2.0 --libs glib-2.0 libxml-2.0` -lm -lvirt -levent -lnet -lwpa_client -ldevice_manage

ADD_LIB = -lpthread -ldevice_manage

#
# extra intstall target.
#
.PHONY: install_extra
install_extra:
	@echo "install extra files..."
.PHONY: uninstall_extra
uninstall_extra:
	@echo "uninstall extra files..."

# rootobjs: specify the files of current dir to be compiled.
# e.g.
# root-objs := hello.o
#
VIDEO_PATH=../video
COM_PATH=../main

SRC := $(wildcard *.cpp) \
       $(wildcard $(VIDEO_PATH)/*.cpp) \
       $(wildcard $(COM_PATH)/*.cpp)

root-objs := $(SRC:%.cpp=%.o)

# subdir to be compiled.
# e.g.
# rootdir-y := cli
#
#rootdir-y := ./

# rootobj-y: the lingking targets of subdirs.
# if subdir is xxx, then we add: $(_PDIR)/xxx/_sub_xxx.o
# e.g.
# rootobj-y := $(_PDIR)/cli/_sub_cli.o
#
#rootobj-y := $(_PDIR)/main/_sub_main.o
#rootobj-y += $(_PDIR)/web/_sub_web.o
#rootobj-y += $(_PDIR)/vm/_sub_vm.o
#rootobj-y += $(_PDIR)/ui/_sub_ui.o
