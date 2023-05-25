/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>

#include "bf_process_protocl_packet_table.h"

const bf_rt_table_hdl *protoclPacketTable = NULL;

static bf_rt_table_key_hdl *bfKey;
static bf_rt_table_data_hdl *bfData;

// Key field ids
static bf_rt_id_t priority_field_id = 0;
static bf_rt_id_t ipv4_is_valid_field_id = 0;
static bf_rt_id_t vxlan_is_valid_field_id = 0;
static bf_rt_id_t ethernet_type_field_id = 0;
static bf_rt_id_t ipv4_dstaddr_field_id = 0;
static bf_rt_id_t ingress_port_field_id = 0;
static bf_rt_id_t ipv6_is_valid_field_id = 0;
static bf_rt_id_t ipv6_dstaddr_field_id = 0;

// Action Ids
static bf_rt_id_t receive_from_lldp_action_id = 0;
static bf_rt_id_t receive_from_cpu_action_id = 0;
static bf_rt_id_t copy_to_cpu_action_id = 0;

// Data field Ids for copy_to_cpu_nos action
static bf_rt_id_t copy_to_cpu_action_port_field_id = 0;

void protocol_packet_table_setup() 
{
	bf_status_t bf_status;
	jd_bf_rt_t *jd_bf_p = &jd_bfrt;
	
 	bf_status = bf_rt_table_from_name_get(jd_bf_p->bfrtInfo, 
		"P02_Ingress.process_local_packet.process_protocol_packet", 
		&protoclPacketTable);
  	assert(bf_status == BF_SUCCESS);
	
  	/******************************************************
  	// Get action Ids
	/*****************************************************/
	bf_status = bf_rt_action_name_to_id(protoclPacketTable, 
		"P02_Ingress.process_local_packet.receive_from_cpu_lldp", 
		&receive_from_lldp_action_id);
  	assert(bf_status == BF_SUCCESS);
	
	bf_status = bf_rt_action_name_to_id(protoclPacketTable, 
		"P02_Ingress.process_local_packet.receive_from_cpu_nos", 
		&receive_from_cpu_action_id);
  	assert(bf_status == BF_SUCCESS);	

	bf_status = bf_rt_action_name_to_id(protoclPacketTable, 
		"P02_Ingress.process_local_packet.copy_to_cpu_nos", 
		&copy_to_cpu_action_id);
  	assert(bf_status == BF_SUCCESS);

	/******************************************************
  	// Get field-ids for key field
	/*****************************************************/
	bf_status = bf_rt_key_field_id_get(protoclPacketTable, 
		"$MATCH_PRIORITY", 
		&priority_field_id);
  	assert(bf_status == BF_SUCCESS);
	
	bf_status = bf_rt_key_field_id_get(protoclPacketTable, 
		"hdr.ipv4.$valid", 
		&ipv4_is_valid_field_id);
  	assert(bf_status == BF_SUCCESS);
	
	bf_status = bf_rt_key_field_id_get(protoclPacketTable, 
		"hdr.ipv6.$valid", 
		&ipv6_is_valid_field_id);
  	assert(bf_status == BF_SUCCESS);	
	
	bf_status = bf_rt_key_field_id_get(protoclPacketTable, 
		"hdr.vxlan.$valid", 
		&vxlan_is_valid_field_id);
  	assert(bf_status == BF_SUCCESS);
	
	bf_status = bf_rt_key_field_id_get(protoclPacketTable, 
		"hdr.ethernet.etherType", 
		&ethernet_type_field_id);
  	assert(bf_status == BF_SUCCESS);

	bf_status = bf_rt_key_field_id_get(protoclPacketTable, 
		"hdr.ipv4.dstAddr", 
		&ipv4_dstaddr_field_id);
  	assert(bf_status == BF_SUCCESS);
	
	bf_status = bf_rt_key_field_id_get(protoclPacketTable, 
		"hdr.ipv6.dstAddr", 
		&ipv6_dstaddr_field_id);
  	assert(bf_status == BF_SUCCESS);
	
	bf_status = bf_rt_key_field_id_get(protoclPacketTable, 
		"ig_intr_md.ingress_port", 
		&ingress_port_field_id);
  	assert(bf_status == BF_SUCCESS);

	/******************************************************
   	* DATA FIELD ID GET FOR "copy_to_cpu_nos" ACTION
   	*******************************************************/

  	bf_status = bf_rt_data_field_id_with_action_get(
      protoclPacketTable,
      "egress_port",
      copy_to_cpu_action_id,
      &copy_to_cpu_action_port_field_id);
  	assert(bf_status == BF_SUCCESS);
	
	// Allocate key and data once, and use reset across different uses
  	bf_status = bf_rt_table_key_allocate(protoclPacketTable, &bfKey);
  	assert(bf_status == BF_SUCCESS);
  	bf_status = bf_rt_table_data_allocate(protoclPacketTable, &bfData);
  	assert(bf_status == BF_SUCCESS);
}

void protocol_packet_table_teardown() {
  	bf_status_t bf_status;
  
  	// Deallocate key and data
  	bf_status = bf_rt_table_key_deallocate(bfKey);
  	assert(bf_status == BF_SUCCESS);
  	bf_status = bf_rt_table_data_deallocate(bfData);
  	assert(bf_status == BF_SUCCESS);
}

static int protoclPacket_key_setup(const protocolPacketKey *key,
                       bf_rt_table_key_hdl *table_key) {
  	bf_status_t bf_status;
	bf_status = bf_rt_key_field_set_value(table_key, 
			priority_field_id, 
			key->priority);
  	if (bf_status != BF_SUCCESS) {
		return -1;
  	}
	
  	bf_status = bf_rt_key_field_set_value_and_mask(table_key, 
			ipv4_is_valid_field_id, 
			key->ipv4_isvalid,
			key->ipv4_isvalid_mask);
  	if (bf_status != BF_SUCCESS) {
		return -1;
  	}

	bf_status = bf_rt_key_field_set_value_and_mask(table_key, 
			ipv6_is_valid_field_id, 
			key->ipv6_isvalid,
			key->ipv6_isvalid_mask);
  	if (bf_status != BF_SUCCESS) {
		return -1;
  	}
	
  	bf_status = bf_rt_key_field_set_value_and_mask(table_key, 
			vxlan_is_valid_field_id, 
			key->vxlan_isvalid,
			key->vxlan_isvalid_mask);
  	if (bf_status != BF_SUCCESS) {
		return -1;
  	}

	bf_status = bf_rt_key_field_set_value_and_mask(table_key, 
			ethernet_type_field_id, 
			key->ethertype,
			key->ethertype_mask);
    if (bf_status != BF_SUCCESS) {
		return -1;
  	}

	bf_status = bf_rt_key_field_set_value_and_mask(table_key, 
			ipv4_dstaddr_field_id, 
			key->dip,
			key->dip_mask);
  	if (bf_status != BF_SUCCESS) {
		return -1;
  	}
	
	bf_status = bf_rt_key_field_set_value_and_mask_ptr(table_key, 
			ipv6_dstaddr_field_id, 
			key->dip6,
			key->dip6_mask,
			16);
  	if (bf_status != BF_SUCCESS) {
		return -1;
  	}

	bf_status = bf_rt_key_field_set_value_and_mask(table_key, 
			ingress_port_field_id, 
			key->ingress_port,
			key->ingress_port_mask);
  	if (bf_status != BF_SUCCESS) {
		return -1;
  	}

	return 0;
}

static int data_setup_for_copy_to_cpu(const copy_tocpuData *data,
                                  bf_rt_table_data_hdl *table_data) {
	bf_status_t bf_status;

  	bf_status = bf_rt_data_field_set_value(
      	table_data, 
      	copy_to_cpu_action_port_field_id, 
      	data->egress_port);
  	if(bf_status != BF_SUCCESS) {
		return -1;
  	}
  	return 0;
}

static int protocol_packet_entry_find(const protocolPacketKey *key) {
	bf_status_t status = BF_SUCCESS;
	bf_rt_entry_read_flag_e flag = ENTRY_READ_FROM_HW;
	jd_bf_rt_t *jd_bf_p = &jd_bfrt;

  	bf_rt_table_key_reset(protoclPacketTable, &bfKey);
  	bf_rt_table_data_reset(protoclPacketTable, &bfData);

  	if (protoclPacket_key_setup(key, bfKey) < 0)
		return -1;
	
#ifdef BFRT_GENERIC_FLAGS
	uint64_t flags = 0;
  	BF_RT_FLAG_CLEAR(flags, BF_RT_FROM_HW);
	status = bf_rt_table_entry_get(protoclPacketTable, jd_bf_p->session, 
  									&jd_bf_p->dev_tgt, flags, bfKey, bfData);
#else
  	// Entry get from hardware with the flag set to read from hardware
  	status = bf_rt_table_entry_get(protoclPacketTable, jd_bf_p->session, 
  									&jd_bf_p->dev_tgt, bfKey, bfData, flag);
#endif
  	if (status != BF_SUCCESS)
		return -1;
	
  	return 0;
}

static int __entry_add_with_copy_to_cpu_nos(const protocolPacketKey *key,
                                         const copy_tocpuData *data,
                                         const bool add) {
	bf_status_t status;
	jd_bf_rt_t *jd_bf_p = &jd_bfrt;

  	bf_rt_table_key_reset(protoclPacketTable, &bfKey);
  	bf_rt_table_action_data_reset(protoclPacketTable, copy_to_cpu_action_id, &bfData);

  	// Fill in the Key and Data object
  	if (protoclPacket_key_setup(key, bfKey) < 0)
		return -1;
  	if(data_setup_for_copy_to_cpu(data, bfData) < 0)
		return -1;

  	if (add) {
    	status = bf_rt_table_entry_add(protoclPacketTable, 
					jd_bf_p->session, 
    				&jd_bf_p->dev_tgt,
    			#ifdef BFRT_GENERIC_FLAGS
							       0,
				#endif
    				bfKey, 
    				bfData);
  	} else {
    	status = bf_rt_table_entry_mod(protoclPacketTable, 
					jd_bf_p->session, 
    				&jd_bf_p->dev_tgt,
    			#ifdef BFRT_GENERIC_FLAGS
							       0,
				#endif
    				bfKey, 
    				bfData);
  	}
	
  	if (status == BF_SUCCESS) {
  		bf_rt_session_complete_operations(jd_bf_p->session);
		return 0;
  	}
	
	return -1;
}

static int __entry_add_with_receive_from_cpu_nos(const protocolPacketKey *key,
                                         const bool add) {
	bf_status_t status;
	jd_bf_rt_t *jd_bf_p = &jd_bfrt;

  	bf_rt_table_key_reset(protoclPacketTable, &bfKey);
  	bf_rt_table_action_data_reset(protoclPacketTable, receive_from_cpu_action_id, &bfData);

  	// Fill in the Key and Data object
  	if (protoclPacket_key_setup(key, bfKey) < 0)
		return -1;

  	if (add) {
    	status = bf_rt_table_entry_add(protoclPacketTable, 
					jd_bf_p->session, 
    				&jd_bf_p->dev_tgt, 
		#ifdef BFRT_GENERIC_FLAGS
							       0,
		#endif
					bfKey, 
    				bfData);
  	} else {
    	status = bf_rt_table_entry_mod(protoclPacketTable, 
					jd_bf_p->session, 
    				&jd_bf_p->dev_tgt, 
    		#ifdef BFRT_GENERIC_FLAGS
							       0,
			#endif
    				bfKey, 
    				bfData);
  	}
	
  	if (status == BF_SUCCESS) {
  		bf_rt_session_complete_operations(jd_bf_p->session);
		return 0;
  	}
	
	return -1;
}

static int __entry_add_with_receive_from_cpu_lldp(const protocolPacketKey *key,
                                         const bool add) {
	bf_status_t status;
	jd_bf_rt_t *jd_bf_p = &jd_bfrt;

  	bf_rt_table_key_reset(protoclPacketTable, &bfKey);
  	bf_rt_table_action_data_reset(protoclPacketTable, receive_from_lldp_action_id, &bfData);

  	// Fill in the Key and Data object
  	if (protoclPacket_key_setup(key, bfKey) < 0)
		return -1;

  	if (add) {
    	status = bf_rt_table_entry_add(protoclPacketTable, 
					jd_bf_p->session, 
    				&jd_bf_p->dev_tgt,
    			#ifdef BFRT_GENERIC_FLAGS
							       0,
				#endif
    				bfKey, 
    				bfData);
  	} else {
    	status = bf_rt_table_entry_mod(protoclPacketTable, 
					jd_bf_p->session, 
    				&jd_bf_p->dev_tgt,
    			#ifdef BFRT_GENERIC_FLAGS
							       0,
				#endif
    				bfKey, 
    				bfData);
  	}
	
  	if (status == BF_SUCCESS) {
  		bf_rt_session_complete_operations(jd_bf_p->session);
		return 0;
  	}
	
	return -1;
}

int entry_add_with_copy_to_cpu_nos(const protocolPacketKey *key, 
	const copy_tocpuData *data) {
	bool flag = true;
	if (protocol_packet_entry_find(key) == 0) {
		flag = false;
	} 
	return __entry_add_with_copy_to_cpu_nos(key, data, flag);
}

int entry_add_with_receive_from_cpu_nos(const protocolPacketKey *key) {
	bool flag = true;
	if (protocol_packet_entry_find(key) == 0) {
		flag = false;
	} 
	return __entry_add_with_receive_from_cpu_nos(key, flag);
}

int entry_add_with_receive_from_cpu_lldp(const protocolPacketKey *key) {
	bool flag = true;
	if (protocol_packet_entry_find(key) == 0) {
		flag = false;
	} 
	return __entry_add_with_receive_from_cpu_lldp(key, flag);
}

static void protocol_packet_entry_add_test(){
	protocolPacketKey key;
	copy_tocpuData data;
	struct in_addr addr;
	protocol_packet_table_setup();

	key.priority = 0;
	key.ipv4_isvalid = 0;
	key.ipv4_isvalid_mask = 0x1;
	key.ethertype = 0x88CC;
	key.ethertype_mask = 0xFFFF;
	inet_aton("0.0.0.0",&addr);	
	key.dip = addr.s_addr;
	key.dip_mask = 0;
	key.ingress_port = 320;
	key.ingress_port_mask = 0x1FF;
	assert(entry_add_with_receive_from_cpu_lldp(&key) == 0);

	key.priority = 10;
	key.ipv4_isvalid = 0;
	key.ipv4_isvalid_mask = 0;
	key.ethertype = 0;
	key.ethertype_mask = 0;
	inet_aton("0.0.0.0",&addr);	
	key.dip = addr.s_addr;
	key.dip_mask = 0;
	key.ingress_port = 320;
	key.ingress_port_mask = 0x1FF;
	assert(entry_add_with_receive_from_cpu_nos(&key) == 0);

	key.priority = 20;
	key.ipv4_isvalid = 0;
	key.ipv4_isvalid_mask = 0x1;
	key.ethertype = 0x0806;
	key.ethertype_mask = 0xFFFF;
	key.dip = 0;
	key.dip_mask = 0;
	key.ingress_port = 0;
	key.ingress_port_mask = 0;
	data.egress_port = 320;
	assert(entry_add_with_copy_to_cpu_nos(&key, &data) == 0);

	key.priority = 20;
	key.ipv4_isvalid = 1;
	key.ipv4_isvalid_mask = 0x1;
	key.ethertype = 0x0800;
	key.ethertype_mask = 0xFFFF;
	inet_aton("10.226.137.225",&addr);	
	key.dip = addr.s_addr;
	key.dip_mask = 0xFFFFFFFF;
	key.ingress_port = 123;
	key.ingress_port_mask = 0x1FF;
	data.egress_port = 320;
	
	assert(entry_add_with_copy_to_cpu_nos(&key, &data) == 0);
	
	key.priority = 20;
	key.ipv4_isvalid = 1;
	key.ipv4_isvalid_mask = 0x1;
	key.ethertype = 0x0800;
	key.ethertype_mask = 0xFFFF;
	inet_aton("10.226.137.225",&addr);	
	key.dip = addr.s_addr;
	key.dip_mask = 0xFFFFFFFF;
	key.ingress_port = 123;
	key.ingress_port_mask = 0x1FF;
	data.egress_port = 64;
	
	assert(entry_add_with_copy_to_cpu_nos(&key, &data) == 0);

}
