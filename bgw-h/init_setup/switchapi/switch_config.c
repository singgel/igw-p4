/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <jansson.h>
#include "utils.h"
#include "switch_config.h"
#include "switch_device_int.h"

const char *hostif_json_file = "/etc/hostif.json";
switch_config_t switch_cfg;

static int get_field_of_string(json_t *section, const char *fieldName, const char **value, int required)
{
	json_t *field;

	field = json_object_get(section, fieldName);
	if (!field) {
		if (required) {
			printf("[msg: field not set, filedname: %s]\n", fieldName);
			return -1;
		} else {
			return 1;
		}
	} else {
		if (!json_is_string(field)) {
			printf("[msg: field type must be string, filename: %s]\n", fieldName);
			return -1;
		}
		*value = json_string_value(field);
		return 0;
	}
}

static int get_field_of_int(json_t *section, const char *fieldName, int *value, int required)
{
	json_t *field;

	field = json_object_get(section, fieldName);
	if (!field) {
		if (required) {
			printf("[msg: field not set, filedname: %s]\n", fieldName);
			return -1;
		} else {
			return 1;
		}
	} else {
		if (!json_is_integer(field)) {
			printf("[msg: field type must be integer, fieldname: %s]\n", fieldName);
			return -1;
		}
		*value = json_integer_value(field);
		return 0;
	}
}

static int switch_config_set_hostif(switch_cfg_hostif_t *hostif, json_t *section)
{
	int res;
	const char *string_val = NULL;

	res = get_field_of_string(section, "intf_name", &string_val, 1);
	if (res == 0) {
		strncpy(hostif->intf_name, string_val, CONST_MAX_PORT_NAME_LEN);
		//printf("[intf_name: %s]\n", hostif->intf_name);
	} else {
		printf("[msg: intf_name required and must be string]\n");
		return -1;
	}

	res = get_field_of_string(section, "ip", &string_val, 1);
	if (res == 0) {
		strncpy(hostif->ip_addr, string_val, CONST_IPV4_ADDR_LEN);
		//printf("[hostif->ip_addr: %s]\n", hostif->ip_addr);
	} else {
		printf("[msg: ip_addr required and must be string]\n");
		return -1;
	}
	
	res = get_field_of_int(section, "net_mask_len", (int *)&hostif->net_mask_len, 1);
	if (res != 0) {
		printf("[msg: fp_port required and must be int]\n");
		return -1;
	}

	res = get_field_of_string(section, "gw_ip", &string_val, 1);
	if (res == 0) {
		strncpy(hostif->gw_ip_addr, string_val, CONST_IPV4_ADDR_LEN);
	} else {
		printf("[msg: ip_addr required and must be string]\n");
		return -1;
	}
		
	res = get_field_of_string(section, "mac", &string_val, 1);
	if (res != 0) {
		printf("[msg: mac required and must be string]\n");
		return -1;
	} else {
		strncpy(hostif->mac_str, string_val, CONST_MAC_ADDR_LEN);
		//printf("[hostif->mac_str: %s]\n", hostif->mac_str);
	}	

	assert(ether_aton(hostif->mac_str, hostif->mac) == 0);

	res = get_field_of_int(section, "fp_port", (int *)&hostif->fp_port, 1);
	if (res != 0) {
		printf("[msg: fp_port required and must be int]\n");
		return -1;
	}

	return 0;
}

static int switch_config_set_hostifs(switch_config_t *cfg, json_t *json_root)
{
	json_t *section = NULL, *j_device = NULL;
	int res, size = 0, i = 0;

	section = json_object_get(json_root, "hostifs");
	if (section == NULL) {
		printf("[msg: section hostifs not exist]\n");
		return -1;
	}

	if (!json_is_array(section)) {
		printf("[msg: section hostifs type must be array]\n");
		return -1;
	}

	size = json_array_size(section);
	if (size == 0 || size > HOSTIFS_SIZE) {
		printf("[msg: section hostifs size must be more than one and < 128]\n");
		return -1;
	}
	
	cfg->hostif_num = size;
	for (i = 0; i < size; ++i) {
		j_device = json_array_get(section, i);
		res = switch_config_set_hostif(&cfg->hostifs[i], j_device);
		if (res != 0) {
			return -1;
		}
	}
	return 0;
}

static int switch_config_set_hbgw(switch_config_t *cfg, json_t *json_root)
{
	json_t *section = NULL;
	const char *string_val = NULL;
	int res;
	
	section = json_object_get(json_root, "hbgw");
	if (section == NULL) {
		printf("[msg: section hbgw not exist in config file]\n");
		return -1;
	}
	
	if (!json_is_object(section)) {
		printf("[msg: section hbgw type must be object]\n");
		return -1;
	}

	res = get_field_of_string(section, "mgt_ip", &string_val, 1);
	if (res == 0) {
		strncpy(cfg->mgt_ip_addr, string_val, CONST_IPV4_ADDR_LEN);
	} else {
		printf("[msg: mgt_ip_addr required and must be string]\n");
		return -1;
	}
	cfg->mgt_ip = ip_atoi(cfg->mgt_ip_addr);

	res = get_field_of_string(section, "vip", &string_val, 1);
	if (res == 0) {
		strncpy(cfg->vip_addr, string_val, CONST_IPV4_ADDR_LEN);
	} else {
		printf("[msg: vip_addr required and must be string]\n");
		return -1;
	}
	cfg->vip = ip_atoi(cfg->vip_addr);

	res = get_field_of_string(section, "backup_vip", &string_val, 1);
	if (res == 0) {
		strncpy(cfg->backup_vip_addr, string_val, CONST_IPV4_ADDR_LEN);
		cfg->backup_vip = ip_atoi(cfg->backup_vip_addr);
	} else {
		//if not exist or not string
		cfg->backup_vip = 0;
	}

	if (cfg->vip == 0 || cfg->mgt_ip == 0) {
		printf("vip or mgt_ip not exist in config file\n");
		return -1;
	}
	
	res = get_field_of_int(section, "port_speed", (int *)&cfg->port_speed, 1);
	if (res != 0) {
		printf("[msg: port_speed required and must be int]\n");
		return -1;
	}

	if (cfg->port_speed != 40 && cfg->port_speed != 100) {
		printf("port_speed must be 100 or 40\n");
		return -1;
	}

	res = get_field_of_int(section, "Hardware", (int *)&cfg->hardware_model, 1);
	if (res != 0) {
		printf("[msg: Hardware required and must be int]\n");
		return -1;
	}

	if (cfg->hardware_model != Wedge_100BF_65X && 
		cfg->hardware_model != Wedge_100BF_32X) {
		printf("Hardware must be 1 or 2\n");
		return -1;
	}
	
	return 0;
}

int switch_config_init(void){
	json_error_t json_err;
	json_t *json_root = NULL;
	int res;
	
	json_root = json_load_file(hostif_json_file, 0, &json_err);
	if (json_root == NULL) {
		printf("[msg: json load error, text: %s, source: %s, line: %d, column: %d, position: %u]\n",
								json_err.text, json_err.source, json_err.line, json_err.column, json_err.position);
		return -1;
	}

	if (!json_is_object(json_root)) {
		printf( "[msg: config file root must be json object]\n");
 		return -1;
	}

	memset(&switch_cfg, 0, sizeof(switch_config_t));
	res = switch_config_set_hostifs(&switch_cfg, json_root);
	if (res != 0) {
		printf("[msg: switch_config_set_hostifs failed]\n");
 		return -1;
	}

	res = switch_config_set_hbgw(&switch_cfg, json_root);
	if (res != 0) {
		printf("[msg: switch_config_set_hbgw failed]\n");
 		return -1;
	}
	
	return 0;
}

