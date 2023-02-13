RTE_SRCDIR = $(shell pwd)
export RTE_SRCDIR

# binary name
APP = init_setup

SDE_VERSION = 9.7.3
ifeq ($(SDE_VERSION), 9.7.1)
	CFLAGS += -DUSE_SDE_9_7
	CFLAGS += -DBFRT_GENERIC_FLAGS
	SDE_INSTALL = /sde/bf-sde-9.7.1/install
else ifeq ($(SDE_VERSION), 9.7.3)
	CFLAGS += -DUSE_SDE_9_7
	CFLAGS += -DBFRT_GENERIC_FLAGS
	SDE_INSTALL = /root/sde/bf-sde-9.7.3/install
else ifeq ($(SDE_VERSION), 9.3.1)
	SDE_INSTALL = /root/sde/bf-sde-9.3.1/install
endif

export SDE_INSTALL
export SDE_VERSION

.PHONY:default
default:$(APP)

# all source are stored in SRCS-y
SRCS-y := main.c
CPPSRCS-y :=  

SUBDIRS :=  switchapi switchbfrt switchlink
SUBDIR_MAKEFILES := $(foreach f, $(SUBDIRS), $(RTE_SRCDIR)/$(f)/Makefile)
include $(SUBDIR_MAKEFILES)

CFLAGS += -g -O2
CFLAGS += -I$(RTE_SRCDIR)/include
CXXFLAGS += -Wall -g
LDFLAGS += -lpthread

include $(RTE_SRCDIR)/app.mk
