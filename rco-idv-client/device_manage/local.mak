#
# Module Makefile variables definition.
# Modified area: add module files here to do compiling.
#

#
# LIB name & version.
# e.g.
# LIB_NAME = common
# LIB_VERSION = 1.0
# output LIB file: libcommon.so -> libcommon.so.1.0
#
LIB_NAME = device_manage
LIB_VERSION = 1.0

# Module extra FLAGS.
EXTRA_CFLAGS := 
EXTRA_LDFLAGS :=

# options only for cpp compiling.
override ADD_CPPFLAGS += -std=c++11

# Module include path, separate with while space.
# e.g.
# $(LIBTOPDIR)/include
ADD_INCLUDE = $(LIBTOPDIR)/include

# specify link libs.
ADD_LIB = -lrc_base -lpthread -lpulse

#
# extra intstall target.
#
.PHONY: install_extra
install_extra:
	@echo "install extra files..."
	cp -f $(LIBTOPDIR)/res/pulse-daemon /usr/bin/pulse-daemon
.PHONY: uninstall_extra
uninstall_extra:
	@echo "uninstall extra files..."
	rm -f /usr/bin/pulse-daemon
# LIB header files or dirs, which will be linked to common header dir.
LIB_HEADERS := include/dev_api.h

# rootobjs: specify the files of current dir to be compiled.
# e.g.
# root-objs := common.o
#
SRC := $(wildcard *.cpp)
root-objs := $(SRC:%.cpp=%.o)

# subdir to be compiled.
# e.g.
# rootdir-y := cli
#
rootdir-y := main audio video

# rootobj-y: the lingking targets of subdirs.
# if subdir is xxx, then we add: $(_PDIR)/xxx/_sub_xxx.o
# e.g.
# rootobj-y := $(_PDIR)/cli/_sub_cli.o
#
rootobj-y := $(_PDIR)/main/_sub_main.o
rootobj-y += $(_PDIR)/audio/_sub_audio.o
rootobj-y += $(_PDIR)/video/_sub_video.o
