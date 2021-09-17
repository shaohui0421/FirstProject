#
# gcov_out library main Makefile
#
include $(top_srcdir)/env.mak

#
# ALL target
#
all: do-it-all

# LIB root dir.
LIBTOPDIR :=$(shell pwd)

# LIB compiling options.
LIB_CFLAGS := -fPIC -shared

# ================ Modified Area START ================ #

include $(LIBTOPDIR)/local.mak

# for gcov_out, do not use GCOV compiling flags.
override CFLAGS := $(BUILD_CFLAGS) $(LIB_CFLAGS) $(EXTRA_CFLAGS)
override LDFLAGS := $(BUILD_LDFLAGS) $(LIB_CFLAGS) $(EXTRA_LDFLAGS)

# ================= Modified Area END ================= #

ifdef QUIET
Q:=@
else
Q:=
endif

#
# target LIB output path.
#
LIB_FILE = lib$(LIB_NAME).so.$(LIB_VERSION)
LIB_LINK = lib$(LIB_NAME).so
LIB_OUTPUT = $(_PDIR)/$(LIB_FILE)

override SUBDIRS := $(rootdir-y)
override ALL_OBJS := $(rootobj-y) $(addprefix $(_PDIR)/,$(root-objs))
override CFLAGS += $(patsubst %,-I%,$(ADD_INCLUDE))

.PHONY: do-it-all
do-it-all: install-build

.PHONY: install-build
install-build: build
	mkdir -p $(BUILD_LIBDIR)
	cp -f $(LIB_OUTPUT) $(BUILD_LIBDIR)
	rm -f $(BUILD_LIBDIR)/$(LIB_LINK)
	ln -s $(BUILD_LIBDIR)/$(LIB_FILE) $(BUILD_LIBDIR)/$(LIB_LINK)

build: root_targets header
	$Q$(MAKE) ngsa

.PHONY: header
header: $(patsubst %, _h_%, $(LIB_HEADERS)) $(patsubst %, _h_%, $(LIB_SUBHEADERS))

$(patsubst %, _h_%, $(LIB_HEADERS)):
	mkdir -p $(common_headerdir)/$(LIB_NAME)
	rm -f $(common_headerdir)/$(LIB_NAME)/$(shell basename $(patsubst _h_%,%,$@))
	ln -s $(LIBTOPDIR)/$(patsubst _h_%,%,$@) $(common_headerdir)/$(LIB_NAME)/$(shell basename $(patsubst _h_%,%,$@))

$(patsubst %, _h_%, $(LIB_SUBHEADERS)):
	mkdir -p $(common_headerdir)/$(LIB_NAME)/$(shell dirname $(patsubst _h_%,%,$@))
	rm -f $(common_headerdir)/$(LIB_NAME)/$(shell basename $(patsubst _h_%,%,$@))
	ln -s $(LIBTOPDIR)/$(patsubst _h_%,%,$@) $(common_headerdir)/$(LIB_NAME)/$(patsubst _h_%,%,$@)

# generate target lib file.
ngsa: ngsasubdirs
	$(CC) $(LDFLAGS) $(ALL_OBJS) $(ADD_LIB) -o $(LIB_OUTPUT)

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
	mkdir -p $(install_libdir)
	cp -f $(LIB_OUTPUT) $(install_libdir)
	rm -f $(install_libdir)/$(LIB_LINK)
	cd $(install_libdir) && ln -s $(LIB_FILE) $(LIB_LINK)
	$Q$(MAKE) install_extra

.PHONY: uninstall
uninstall:
	rm -f $(install_libdir)/$(LIB_FILE) $(install_libdir)/$(LIB_LINK)
	$Q$(MAKE) uninstall_extra

.PHONY: clean
clean:
	rm -rf $(_PDIR)
	rm -rf $(common_headerdir)/$(LIB_NAME)
	rm -f $(BUILD_LIBDIR)/$(LIB_FILE) $(BUILD_LIBDIR)/$(LIB_LINK)
