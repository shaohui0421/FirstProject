#
# IDV client Top Makefile variables definitions.
# Modified area: add new modules here.
#

# Client header files.
RCLIB_MPATH := include/rc_base
ADD_CFLAGS += $(patsubst %,-I$(top_srcdir)/%,$(RCLIB_MPATH))

# Modules common extra FLAGS.
EXTRA_CFLAGS :=
EXTRA_LDFLAGS :=

# Extra Libs to be installed into RCD system.
extra_alibs :=
extra_solibs :=

# All the target applications to be compiled.
target_app := client terminal_config base_cp display_hotplug

# All the target libraries to be compiled.
target_lib := common

#
# for GCOV output PRELOAD
# Usage:
#   LD_PRELOAD=$(install_libdir)/lib$(GCOV_OUTLIB).so cmdline
# add signal handler to lead cmd calling __gcov_flush() after killed or crash.
#
ifeq ($(CONFIG_GCOV),y)
target_lib += gcov_out
endif

# add factory test app.
# target_app += factory_test

# target sample:
# target_app += sample
# target_lib += sample/libhello
