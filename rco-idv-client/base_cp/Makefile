#
# IDV Client modules main Makefile
#
include $(top_srcdir)/env.mak

#
# ALL target
#
all: do-it-all

# Module root dir.
APP_TOPDIR := $(shell pwd)

# ================ Modified Area START ================ #

include $(APP_TOPDIR)/local.mak

override CFLAGS += $(EXTRA_CFLAGS)
override LDFLAGS += $(EXTRA_LDFLAGS)

# ================= Modified Area END ================= #

ifdef QUIET
Q:=@
else
Q:=
endif

#
# target ELF & MAP files' paths.
#
ELF_FILE = $(_PDIR)/$(APP_TARGET)
MAP_FILE = $(_PDIR)/$(APP_TARGET).map.sym

override SUBDIRS := $(rootdir-y)
override ALL_OBJS := $(rootobj-y) $(addprefix $(_PDIR)/,$(root-objs))
override CFLAGS += $(patsubst %,-I%,$(ADD_INCLUDE))

.PHONY: do-it-all
do-it-all: build

build: root_targets
	$Q$(MAKE) ngsa

ngsa: ngsasubdirs
	rm -f $(MAP_FILE) $(ELF_FILE)
	@echo "Linking $(ELF_FILE)..."
	$(CC) $(ALL_OBJS) -o $(ELF_FILE) $(LDFLAGS) $(ADD_LIB)
	$(NM) $(ELF_FILE) | grep -v '\(compiled\)\|\(\.o$$\)\|\( [aUw] \)\|\(\.\.ng$$\)\|\(LASH[RL]DI\)' \
		| sort > $(MAP_FILE)
#	$(OBJDUMP) -D -j .vectors -j .init -j .text $(ELF_FILE) | gzip > $(ELF_FILE).disasm.gz

.PHONY: make-dir
make-dir:
	@mkdir -p $(_PDIR)

root_targets: make-dir
	$Q$(MAKE) root-compile

#
# subdirs: traversal to do compiling.
#
SUBDIRS := $(wildcard $(SUBDIRS))
ngsasubdirs: $(patsubst %, _dir_%, $(SUBDIRS))
$(patsubst %, _dir_%, $(SUBDIRS)):
	$(MAKE) CFLAGS="$(CFLAGS)" -C $(patsubst _dir_%, %, $@) _PDIR="$(_PDIR)/$(patsubst _dir_%,%,$@)"

# depend files.
root-deps := $(patsubst %.o,%.d,$(root-objs))

root-compile: $(addprefix $(_PDIR)/,$(root-deps)) $(addprefix $(_PDIR)/,$(root-objs))

$(addprefix $(_PDIR)/,$(root-deps)):
$(addprefix $(_PDIR)/,$(root-objs)):

# generate depend files.
$(_PDIR)/%.d: %.c
	$Q$(top_srcdir)/scripts/mkcdep $< "$(CFLAGS) $(ADD_CFLAGS) $(CFLAGS_$(patsubst $(_PDIR)/%,%,$@))" > $@

$(_PDIR)/%.d: %.cpp
	$Q$(top_srcdir)/scripts/mkcppdep $< "$(CFLAGS) $(ADD_CFLAGS) $(ADD_CPPFLAGS) $(CFLAGS_$(patsubst $(_PDIR)/%,%,$@))" > $@

# contain .d files.
FILES_DEPEND_EXIST := $(wildcard $(addprefix $(_PDIR)/,$(root-deps)))
ifneq ($(FILES_DEPEND_EXIST),)
include $(FILES_DEPEND_EXIST)
endif

# target compiling rules.
$(_PDIR)/%.o: %.c
ifdef QUIET
	@echo "[*] Compiling `pwd`/$<"
endif
	$Q$(CC) $(CFLAGS) $(ADD_CFLAGS) $(CFLAGS_$(patsubst $(_PDIR)/%,%,$@)) -c $< -o $@
	$Q ( \
	    echo 'ifeq ($(strip $(subst $(comma),:,$(CFLAGS) $(ADD_CFLAGS) $(CFLAGS_$(patsubst $(_PDIR)/%,%,$@)))),$$(strip $$(subst $$(comma),:,$$(CFLAGS) $$(ADD_CFLAGS) $$(CFLAGS_$(patsubst $(_PDIR)/%,%,$@)))))' ; \
	    echo 'FILES_FLAGS_UP_TO_DATE += $@' ; \
	    echo 'endif' \
	) > $(dir $@)/.$(notdir $@).flags

$(_PDIR)/%.o: %.cpp
ifdef QUIET
	@echo "[*] Compiling `pwd`/$<"
endif
	$Q$(CPP) $(CFLAGS) $(ADD_CFLAGS) $(ADD_CPPFLAGS) $(CFLAGS_$(patsubst $(_PDIR)/%,%,$@)) -c $< -o $@
	$Q ( \
	    echo 'ifeq ($(strip $(subst $(comma),:,$(CFLAGS) $(ADD_CFLAGS) $(ADD_CPPFLAGS) $(CFLAGS_$(patsubst $(_PDIR)/%,%,$@)))),$$(strip $$(subst $$(comma),:,$$(CFLAGS) $$(ADD_CFLAGS) $$(ADD_CPPFLAGS) $$(CFLAGS_$(patsubst $(_PDIR)/%,%,$@)))))' ; \
	    echo 'FILES_FLAGS_UP_TO_DATE += $@' ; \
	    echo 'endif' \
	) > $(dir $@)/.$(notdir $@).flags

# rules for gcov.
.PHONY: gcov_prepare
gcov_prepare: $(addprefix $(_PDIR)/,$(SRC)) $(patsubst %, _gcov_%, $(SUBDIRS))
$(patsubst %, _gcov_%, $(SUBDIRS)):
	$(MAKE) -C $(patsubst _gcov_%, %, $@) _PDIR="$(_PDIR)/$(patsubst _gcov_%,%,$@)" gcov_prepare

$(_PDIR)/%.c: %.c
	rm -f $@
	ln -s $(shell pwd)/$< $@

$(_PDIR)/%.cpp: %.cpp
	rm -f $@
	ln -s $(shell pwd)/$< $@

.PHONY: install
install:
	mkdir -p $(install_bindir)
	cp -f $(ELF_FILE) $(install_bindir)
	$Q$(MAKE) install_extra

.PHONY: uninstall
uninstall:
	rm -f $(install_bindir)/$(ELF_FILE)
	$Q$(MAKE) uninstall_extra

.PHONY: clean
clean:
	rm -rf $(_PDIR)
