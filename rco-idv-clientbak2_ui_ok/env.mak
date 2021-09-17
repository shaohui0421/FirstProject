#
# env.mak
# Copyright (C)2017 Ruijie Network Inc. All rights reserved.
# Author: yejx@ruijie.com.cn
#
# IDV Client compiling environment definition.
#

# Tools.
CC      := gcc
CPP     := g++
LD      := ld
AR      := ar
NM      := nm
OBJDUMP := objdump

# GCOV Tools.
LCOV	:= $(top_srcdir)/scripts/lcov-1.13/bin/lcov
GENHTML	:= $(top_srcdir)/scripts/lcov-1.13/bin/genhtml

export CC CPP LD AR NM OBJDUMP

common_headerdir := $(top_srcdir)/include
extra_libdir := $(top_srcdir)/extralib

install_dir := /usr/local
install_sysconf := /etc
install_extradir := $(install_sysconf)/RCC_Client
install_cfg := $(install_sysconf)/UserCfg

install_bindir := $(install_dir)/bin
install_libdir := $(install_dir)/lib

install_bindir_systemdir := $(install_bindir)/system

install_resdir := $(install_extradir)/res
install_confdir := $(install_extradir)
install_layerdir := $(install_extradir)/layer
install_scriptdir := $(install_extradir)/scripts
install_preupgrade_scriptdir := $(install_scriptdir)/pre_upgrade
install_postupgrade_scriptdir := $(install_scriptdir)/post_upgrade
install_checkupgrade_scriptdir := $(install_scriptdir)/check_upgrade
install_rccdaemon_scriptdir := $(install_scriptdir)/rcc_daemon
install_autostart_logindir := $(install_sysconf)/AutoStart.login
install_iptablesruledir := $(install_cfg)/iptablesrule

# Basic compiling options.
BUILD_CFLAGS := -Os -Wall -g
BUILD_LDFLAGS := -Os -g

# GCOV compiling options.
GCOV_FLAGS := -fprofile-arcs -ftest-coverage

GCOV_INFO := rcd.info
GCOV_OUTPUT := rcd_coverage
GCOV_OUTLIB := gcov_out
