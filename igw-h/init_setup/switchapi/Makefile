VPATH += $(RTE_SRCDIR)/switchapi

SRCS-y += switch_port.c switch_packet.c switch_device.c switch_hostif.c switch_config.c switch_packet_table.c switch_monitor.c 
SRCS-y += switch_igw_ip_type_table.c switch_eip_in_jd_vxlan_table.c

CFLAGS += -I$(RTE_SRCDIR)/switchapi/include/ -I$(SDE_INSTALL)/include 
LDFLAGS += -L$(SDE_INSTALL)/lib -lbfsys -lbfutils -ldriver -lavago
LDFLAGS += -ljansson -lnl-3 -lnl-route-3
ifeq ($(SDE_VERSION), 9.3.1)
	LDFLAGS += -L$(SDE_INSTALL)/lib -lbf_switchd_lib -lpython3.4m -ltofinopdfixed_thrift
endif
