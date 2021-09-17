#
# Module subdir Makefile
# O_TARGET: _sub_xxx.o
# compiling target, which must be the combination of "_sub_" & dirname, with ".o" end.
#
# obj-y: object files which to be compiled.
# subdir-y: subdirs to be compiled.
# include $(top_srcdir)/Rules.mak: compiling rules.
#

O_TARGET := _sub_inifile.o

SRC := $(wildcard *.c)
obj-y := $(SRC:%.c=%.o)

override ADD_CFLAGS += -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include

include $(top_srcdir)/Rules.mak
