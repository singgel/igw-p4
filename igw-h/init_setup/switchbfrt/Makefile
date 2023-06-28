VPATH += $(RTE_SRCDIR)/switchbfrt

SRCS-y += bf_outer_mac_rewrite_table.c jd_bfrt.c 
SRCS-y += bf_process_protocl_packet_table.c bf_igw_ip_type_table.c
SRCS-y += bf_eip_in_jd_vxlan_table.c bf_eip_out_jd_vxlan_table.c 
SRCS-y += bf_eip_in_action_profile.c bf_eip_in_selector.c 
SRCS-y += bf_eip_in_select_redirect_ip.c bf_eip_out_action_profile.c
SRCS-y += bf_eip_out_selector.c bf_eip_out_select_redirect_ip.c
SRCS-y += bf_eip_in_meter_drop.c bf_mirror_cfg.c bf_mirror_drop.c

CFLAGS += -I$(RTE_SRCDIR)/switchbfrt/include/ -I$(SDE_INSTALL)/include 
LDFLAGS += 
