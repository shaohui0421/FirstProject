#
# target modules Makefile
#
include $(top_srcdir)/env.mak

#
# ALL target
#
.PHONY: all
all: build

# target TOPDIR
TARGET_TOPDIR := $(shell pwd)

# ================ Modified Area START ================ #

include $(TARGET_TOPDIR)/target.mak

override CFLAGS += $(EXTRA_CFLAGS)
override LDFLAGS += $(EXTRA_LDFLAGS)
export CFLAGS LDFLAGS

# ================= Modified Area END ================= #

.PHONY: build
build: $(patsubst %, _dir_%, $(target_modules))
$(patsubst %, _dir_%, $(target_modules)):
	@mkdir -p $(_PDIR)
	$(MAKE) CFLAGS="$(CFLAGS)" -C $(TARGET_TOPDIR)/$(patsubst _dir_%,%,$@) _PDIR="$(_PDIR)/$(patsubst _dir_%,%,$@)"

.PHONY: clean
clean: $(patsubst %, _c_%, $(target_modules))
$(patsubst %, _c_%, $(target_modules)):
	@mkdir -p $(_PDIR)
	$(MAKE) CFLAGS="$(CFLAGS)" -C $(TARGET_TOPDIR)/$(patsubst _c_%,%,$@) _PDIR="$(_PDIR)/$(patsubst _c_%,%,$@)" clean

.PHONY: install
install: $(patsubst %, _inst_%, $(target_modules))
$(patsubst %, _inst_%, $(target_modules)):
	@mkdir -p $(_PDIR)
	$(MAKE) CFLAGS="$(CFLAGS)" -C $(TARGET_TOPDIR)/$(patsubst _inst_%,%,$@) _PDIR="$(_PDIR)/$(patsubst _inst_%,%,$@)" install

.PHONY: uninstall
uninstall: $(patsubst %, _uninst_%, $(target_modules))
$(patsubst %, _uninst_%, $(target_modules)):
	@mkdir -p $(_PDIR)
	$(MAKE) CFLAGS="$(CFLAGS)" -C $(TARGET_TOPDIR)/$(patsubst _uninst_%,%,$@) _PDIR="$(_PDIR)/$(patsubst _uninst_%,%,$@)" uninstall

.PHONY: gcov_prepare
gcov_prepare: $(patsubst %, _gcov_%, $(target_modules))
$(patsubst %, _gcov_%, $(target_modules)):
	@mkdir -p $(_PDIR)
	$(MAKE) -C $(TARGET_TOPDIR)/$(patsubst _gcov_%,%,$@) _PDIR="$(_PDIR)/$(patsubst _gcov_%,%,$@)" gcov_prepare
