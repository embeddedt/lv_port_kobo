SHELL:=/bin/bash
#
# Makefile
#
CC= $(CROSS_COMPILE)gcc
CXX= $(CROSS_COMPILE)g++
LVGL_DIR ?= ${shell pwd}
CFLAGS ?= -Wall -Wshadow -Wundef -Wmaybe-uninitialized -O3 -ggdb -I$(LVGL_DIR) -I$(LVGL_DIR)/lvgl -I$(LVGL_DIR)/lvgl/src -DLV_CONF_INCLUDE_SIMPLE
CXXFLAGS ?=
LDFLAGS ?= -lpthread
BIN = demo

export CC
export CXX


#Collect the files to compile
MAINSRC = ./main.c ./kobo_helpers.c ./lv_font_roboto_30.c

include $(LVGL_DIR)/lvgl/lvgl.mk
include $(LVGL_DIR)/lv_drivers/lv_drivers.mk
include $(LVGL_DIR)/lv_examples/lv_examples.mk

OBJEXT ?= .o

AOBJS = $(ASRCS:.S=$(OBJEXT))
COBJS = $(CSRCS:.c=$(OBJEXT))

MAINOBJ = $(MAINSRC:.c=$(OBJEXT))

SRCS = $(ASRCS) $(CSRCS) $(MAINSRC)
OBJS = $(AOBJS) $(COBJS)

## MAINOBJ -> OBJFILES

.PHONY: all default clean

all: default

%.o: %.c
	@echo "CC $<"
	@$(CC)  $(CFLAGS) -c $< -o $@

%.o: %.cpp
	@echo "CXX $<"
	@$(CXX)  $(CXXFLAGS) $(CFLAGS) -c $< -o $@

default: $(AOBJS) $(COBJS) $(MAINOBJ)
	$(CXX) -o $(BIN) $(MAINOBJ) $(AOBJS) $(COBJS) $(LDFLAGS)
clean:
	rm -f $(BIN) $(AOBJS) $(COBJS) $(MAINOBJ)
