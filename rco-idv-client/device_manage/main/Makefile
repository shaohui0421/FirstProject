#
# Module subdir Makefile
# O_TARGET: _sub_xxx.o
# compiling target, which must be the combination of "_sub_" & dirname, with ".o" end.
#
# obj-y: object files which to be compiled.
# subdir-y: subdirs to be compiled.
# include $(top_srcdir)/Rules.mak: compiling rules.
#

O_TARGET := _sub_main.o

SRC_CPP := $(wildcard *.cpp)
SRC_C := $(wildcard *.c)
obj-y := $(SRC_CPP:%.cpp=%.o) 
obj-y += $(SRC_C:%.c=%.o)
SRC := $(SRC_CPP) $(SRC_C)

include $(top_srcdir)/Rules.mak
