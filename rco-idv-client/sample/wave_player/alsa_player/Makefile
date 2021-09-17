#
# Module subdir Makefile
# O_TARGET: _sub_xxx.o
# compiling target, which must be the combination of "_sub_" & dirname, with ".o" end.
#
# obj-y: object files which to be compiled.
# subdir-y: subdirs to be compiled.
# include $(top_srcdir)/Rules.mak: compiling rules.
#

O_TARGET := _sub_alsa_player.o

SRC := $(wildcard *.c)
obj-y := $(SRC:%.c=%.o)

include $(top_srcdir)/Rules.mak
