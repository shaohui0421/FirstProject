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
LIB_NAME = $(GCOV_OUTLIB)
LIB_VERSION = 1.0

# Module extra FLAGS.
EXTRA_CFLAGS :=
EXTRA_LDFLAGS :=

# options only for cpp compiling.
# override ADD_CPPFLAGS += -std=c++11

# Module include path, separate with while space.
# e.g.
# $(LIBTOPDIR)/include
ADD_INCLUDE =

# specify link libs.
ADD_LIB =

#
# extra intstall target.
#
.PHONY: install_extra
install_extra:
	@echo "install extra files..."

.PHONY: uninstall_extra
uninstall_extra:
	@echo "uninstall extra files..."

# LIB header files or dirs, which will be linked to common header dir.
# e.g.
# LIB_HEADERS := include/hello.h
# LIB_SUBHEADERS := $(wildcard subdir/*.h)
LIB_HEADERS :=
LIB_SUBHEADERS :=

# rootobjs: specify the files of current dir to be compiled.
# e.g.
# root-objs := common.o
#
SRC := $(wildcard *.c)
root-objs := $(SRC:%.c=%.o)

# subdir to be compiled.
# e.g.
# rootdir-y := cli
#
rootdir-y :=

# rootobj-y: the lingking targets of subdirs.
# if subdir is xxx, then we add: $(_PDIR)/xxx/_sub_xxx.o
# e.g.
# rootobj-y := $(_PDIR)/cli/_sub_cli.o
#
rootobj-y :=
