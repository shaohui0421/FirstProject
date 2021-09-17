#
# Module Makefile variables definition.
# Modified area: add module files here to do compiling.
#

#
# Module app name
# APP output file name, that will installed into system bin dir.
#
APP_TARGET = IDV_Client

# Module extra FLAGS.
EXTRA_CFLAGS := -Wl,-fsanitize=address `pkg-config --cflags gtk+-2.0 gthread-2.0 glib-2.0 libxml-2.0`
EXTRA_LDFLAGS := -lstdc++

# options only for cpp compiling.
override ADD_CPPFLAGS += -std=c++11

# Defined for submodules, to be compiled as IDV Client modules, ignoring IDV unused codes.
EXTRA_CFLAGS += -DIDV_CLIENT

# options for debug or unit test.
ifeq ("$(CONFIG_DEBUG)", "y")
EXTRA_CFLAGS += -DUNIT_TEST
endif

# Module include path, separate with while space.
# e.g.
# $(APP_TOPDIR)/include
#
ADD_INCLUDE = $(APP_TOPDIR)/include $(APP_TOPDIR)/ui/include

# specify link libs.
ADD_LIB = -lrc_base -lpthread `pkg-config gtk+-2.0 gthread-2.0 --libs glib-2.0 libxml-2.0` -lm -lvirt -levent -lnet -lwpa_client -lsysabslayer_linux

#
# extra intstall target.
#
.PHONY: install_extra
install_extra:
	@echo "install extra files..."
	mkdir -p $(install_confdir)
	mkdir -p $(install_scriptdir)
	mkdir -p $(install_resdir)
	mkdir -p $(install_layerdir)
	mkdir -p $(install_preupgrade_scriptdir)
	mkdir -p $(install_postupgrade_scriptdir)
	mkdir -p $(install_checkupgrade_scriptdir)
	mkdir -p $(install_rccdaemon_scriptdir)
	mkdir -p $(install_autostart_logindir)
	mkdir -p $(install_iptablesruledir)
	cp -f $(APP_TOPDIR)/scripts/*.ini $(install_confdir)
	cp -f $(APP_TOPDIR)/scripts/*.sh $(install_scriptdir)
	cp -f $(APP_TOPDIR)/scripts/*.bash $(install_scriptdir)
	cp -rf $(APP_TOPDIR)/scripts/pre_upgrade/*.sh $(install_preupgrade_scriptdir)
	cp -rf $(APP_TOPDIR)/scripts/post_upgrade/*.sh $(install_postupgrade_scriptdir)
	cp -rf $(APP_TOPDIR)/scripts/check_upgrade/*.sh $(install_checkupgrade_scriptdir)
	cp -rf $(APP_TOPDIR)/scripts/rcc_daemon/* $(install_rccdaemon_scriptdir)
	cp -f $(APP_TOPDIR)/scripts/grub_3.0 $(install_scriptdir)
	cp -rf $(APP_TOPDIR)/ui/icon $(install_resdir)
	cp -f  $(APP_TOPDIR)/layer/layer_templete.tgz $(install_layerdir)
	cp -f  $(APP_TOPDIR)/layer/print.tgz $(install_layerdir)
	cp -f  $(APP_TOPDIR)/scripts/syscof/RCC_Client_os_upgrade_rule.ini $(install_sysconf)
	cp -f  $(APP_TOPDIR)/scripts/0020F_FixAbslayerIDV.sh $(install_autostart_logindir)
	cp -f  $(APP_TOPDIR)/scripts/0021F_FixRjsyscore.sh $(install_autostart_logindir)
	cp -f  $(APP_TOPDIR)/scripts/RCO_IDV_CLIENT_IPTABLES_RULE.yaml $(install_iptablesruledir)
.PHONY: uninstall_extra
uninstall_extra:
	@echo "uninstall extra files..."
	rm -rf $(install_extradir)
	rm -rf $(install_iptablesruledir)

# rootobjs: specify the files of current dir to be compiled.
# e.g.
# root-objs := hello.o
#
SRC := $(wildcard *.cpp)
root-objs := $(SRC:%.cpp=%.o)

# subdir to be compiled.
# e.g.
# rootdir-y := cli
#
rootdir-y := main web vm ui

# rootobj-y: the lingking targets of subdirs.
# if subdir is xxx, then we add: $(_PDIR)/xxx/_sub_xxx.o
# e.g.
# rootobj-y := $(_PDIR)/cli/_sub_cli.o
#
rootobj-y := $(_PDIR)/main/_sub_main.o
rootobj-y += $(_PDIR)/web/_sub_web.o
rootobj-y += $(_PDIR)/vm/_sub_vm.o
rootobj-y += $(_PDIR)/ui/_sub_ui.o
