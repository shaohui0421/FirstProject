#
# Module Makefile variables definition.
# Modified area: add module files here to do compiling.
#

#
# Module app name
# APP output file name, that will installed into system bin dir.
#
APP_TARGET = wave_player

# Module extra FLAGS.
EXTRA_CFLAGS :=
EXTRA_LDFLAGS :=

# options only for cpp compiling.
# override ADD_CPPFLAGS += -std=c++11

# Module include path, separate with while space.
# e.g.
# $(APP_TOPDIR)/include
#
ADD_INCLUDE = $(APP_TOPDIR)/include

# specify link libs.
ADD_LIB = -lasound -lpthread -lpulse -lpulse-simple

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
SRC := $(wildcard *.c)
root-objs := $(SRC:%.c=%.o)

# subdir to be compiled.
# e.g.
# rootdir-y := cli
#
rootdir-y := alsa_player pa_player

# rootobj-y: the lingking targets of subdirs.
# if subdir is xxx, then we add: $(_PDIR)/xxx/_sub_xxx.o
# e.g.
# rootobj-y := $(_PDIR)/cli/_sub_cli.o
#
rootobj-y := $(_PDIR)/alsa_player/_sub_alsa_player.o
rootobj-y += $(_PDIR)/pa_player/_sub_pa_player.o
