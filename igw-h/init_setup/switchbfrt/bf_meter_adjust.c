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

#include "bf_meter_adjust.h"

const bf_rt_table_hdl *eipInMeterTable = NULL;

const bf_rt_table_attributes_hdl *eipInMeterAttributes;

void eip_in_meter_adjust_setup() 
{
	bf_status_t bf_status;
	jd_bf_rt_t *jd_bf_p = &jd_bfrt;
	
 	bf_status = bf_rt_table_from_name_get(jd_bf_p->bfrtInfo, 
		"P13_Egress.eip_in_meter.bw_ratelimit", 
		&eipInMeterTable);
  	assert(bf_status == BF_SUCCESS);

  	bf_status = bf_rt_table_meter_byte_count_adjust_attributes_allocate(
					eipInMeterTable, &eipInMeterAttributes);
  	assert(bf_status == BF_SUCCESS);

}

void eip_in_meter_byte_count_adjust_set(const int byte_count_adj) {
	bf_status_t bf_status;
	jd_bf_rt_t *jd_bf_p = &jd_bfrt;
	bf_rt_table_attributes_hdl *tmp = (bf_rt_table_attributes_hdl *)eipInMeterAttributes;

	bf_status = bf_rt_attributes_meter_byte_count_adjust_set(
		tmp, byte_count_adj);
  	assert(bf_status == BF_SUCCESS);

	 bf_status = bf_rt_table_attributes_set(
	 			eipInMeterTable, 
				jd_bf_p->session, 
    			&jd_bf_p->dev_tgt, 
	#ifdef BFRT_GENERIC_FLAGS
							   0,
	#endif
				eipInMeterAttributes);
  	assert(bf_status == BF_SUCCESS);
}

const bf_rt_table_hdl *eip6InMeterTable = NULL;

const bf_rt_table_attributes_hdl *eip6InMeterAttributes;

void eip6_in_meter_adjust_setup() 
{
	bf_status_t bf_status;
	jd_bf_rt_t *jd_bf_p = &jd_bfrt;
	
 	bf_status = bf_rt_table_from_name_get(jd_bf_p->bfrtInfo, 
		"P13_Egress.eip_in_meter.ipv6_bw_ratelimit", 
		&eip6InMeterTable);
  	assert(bf_status == BF_SUCCESS);

  	bf_status = bf_rt_table_meter_byte_count_adjust_attributes_allocate(
					eip6InMeterTable, &eip6InMeterAttributes);
  	assert(bf_status == BF_SUCCESS);

}

void eip6_in_meter_byte_count_adjust_set(const int byte_count_adj) {
	bf_status_t bf_status;
	jd_bf_rt_t *jd_bf_p = &jd_bfrt;
	bf_rt_table_attributes_hdl *tmp = (bf_rt_table_attributes_hdl *)eip6InMeterAttributes;

	bf_status = bf_rt_attributes_meter_byte_count_adjust_set(
		tmp, byte_count_adj);
  	assert(bf_status == BF_SUCCESS);

	 bf_status = bf_rt_table_attributes_set(
	 			eip6InMeterTable, 
				jd_bf_p->session, 
    			&jd_bf_p->dev_tgt, 
	#ifdef BFRT_GENERIC_FLAGS
							   0,
	#endif
				eip6InMeterAttributes);
  	assert(bf_status == BF_SUCCESS);
}

const bf_rt_table_hdl *eipInShareMeterTable = NULL;

const bf_rt_table_attributes_hdl *eipInShareMeterAttributes;

void eip_in_share_meter_adjust_setup() 
{
	bf_status_t bf_status;
	jd_bf_rt_t *jd_bf_p = &jd_bfrt;
	
 	bf_status = bf_rt_table_from_name_get(jd_bf_p->bfrtInfo, 
		"P13_Ingress.eip_in_shared_meter.shared_bw_ratelimit", 
		&eipInShareMeterTable);
  	assert(bf_status == BF_SUCCESS);

  	bf_status = bf_rt_table_meter_byte_count_adjust_attributes_allocate(
					eipInShareMeterTable, &eipInShareMeterAttributes);
  	assert(bf_status == BF_SUCCESS);

}

void eip_in_share_meter_byte_count_adjust_set(const int byte_count_adj) {
	bf_status_t bf_status;
	jd_bf_rt_t *jd_bf_p = &jd_bfrt;
	bf_rt_table_attributes_hdl *tmp = (bf_rt_table_attributes_hdl *)eipInShareMeterAttributes;

	bf_status = bf_rt_attributes_meter_byte_count_adjust_set(
		tmp, byte_count_adj);
  	assert(bf_status == BF_SUCCESS);

	 bf_status = bf_rt_table_attributes_set(
	 			eipInShareMeterTable, 
				jd_bf_p->session, 
    			&jd_bf_p->dev_tgt, 
	#ifdef BFRT_GENERIC_FLAGS
							   0,
	#endif
				eipInShareMeterAttributes);
  	assert(bf_status == BF_SUCCESS);
}

const bf_rt_table_hdl *eipOutMeterTable = NULL;

const bf_rt_table_attributes_hdl *eipOutMeterAttributes;

void eip_out_meter_adjust_setup() 
{
	bf_status_t bf_status;
	jd_bf_rt_t *jd_bf_p = &jd_bfrt;
	
 	bf_status = bf_rt_table_from_name_get(jd_bf_p->bfrtInfo, 
		"P02_Egress.process_gw_egress.eip_out_meter.bw_ratelimit", 
		&eipOutMeterTable);
  	assert(bf_status == BF_SUCCESS);

  	bf_status = bf_rt_table_meter_byte_count_adjust_attributes_allocate(
					eipOutMeterTable, &eipOutMeterAttributes);
  	assert(bf_status == BF_SUCCESS);

}

void eip_out_meter_byte_count_adjust_set(const int byte_count_adj) {
	bf_status_t bf_status;
	jd_bf_rt_t *jd_bf_p = &jd_bfrt;
	bf_rt_table_attributes_hdl *tmp = (bf_rt_table_attributes_hdl *)eipOutMeterAttributes;

	bf_status = bf_rt_attributes_meter_byte_count_adjust_set(
		tmp, byte_count_adj);
  	assert(bf_status == BF_SUCCESS);

	 bf_status = bf_rt_table_attributes_set(
	 			eipOutMeterTable, 
				jd_bf_p->session, 
    			&jd_bf_p->dev_tgt, 
	#ifdef BFRT_GENERIC_FLAGS
							   0,
	#endif
				eipOutMeterAttributes);
  	assert(bf_status == BF_SUCCESS);
}

const bf_rt_table_hdl *eip6OutMeterTable = NULL;

const bf_rt_table_attributes_hdl *eip6OutMeterAttributes;

void eip6_out_meter_adjust_setup() 
{
	bf_status_t bf_status;
	jd_bf_rt_t *jd_bf_p = &jd_bfrt;
	
 	bf_status = bf_rt_table_from_name_get(jd_bf_p->bfrtInfo, 
		"P02_Egress.process_gw_egress.eip_out_meter.ipv6_bw_ratelimit", 
		&eip6OutMeterTable);
  	assert(bf_status == BF_SUCCESS);

  	bf_status = bf_rt_table_meter_byte_count_adjust_attributes_allocate(
					eip6OutMeterTable, &eip6OutMeterAttributes);
  	assert(bf_status == BF_SUCCESS);

}

void eip6_out_meter_byte_count_adjust_set(const int byte_count_adj) {
	bf_status_t bf_status;
	jd_bf_rt_t *jd_bf_p = &jd_bfrt;
	bf_rt_table_attributes_hdl *tmp = (bf_rt_table_attributes_hdl *)eip6OutMeterAttributes;

	bf_status = bf_rt_attributes_meter_byte_count_adjust_set(
		tmp, byte_count_adj);
  	assert(bf_status == BF_SUCCESS);

	 bf_status = bf_rt_table_attributes_set(
	 			eip6OutMeterTable, 
				jd_bf_p->session, 
    			&jd_bf_p->dev_tgt, 
	#ifdef BFRT_GENERIC_FLAGS
							   0,
	#endif
				eip6OutMeterAttributes);
  	assert(bf_status == BF_SUCCESS);
}

const bf_rt_table_hdl *eipOutShareMeterTable = NULL;

const bf_rt_table_attributes_hdl *eipOutShareMeterAttributes;

void eip_out_share_meter_adjust_setup() 
{
	bf_status_t bf_status;
	jd_bf_rt_t *jd_bf_p = &jd_bfrt;
	
 	bf_status = bf_rt_table_from_name_get(jd_bf_p->bfrtInfo, 
		"P02_Egress.process_gw_egress.eip_out_shared_meter.shared_bw_ratelimit", 
		&eipOutShareMeterTable);
  	assert(bf_status == BF_SUCCESS);

  	bf_status = bf_rt_table_meter_byte_count_adjust_attributes_allocate(
					eipOutShareMeterTable, &eipOutShareMeterAttributes);
  	assert(bf_status == BF_SUCCESS);

}

void eip_out_share_meter_byte_count_adjust_set(const int byte_count_adj) {
	bf_status_t bf_status;
	jd_bf_rt_t *jd_bf_p = &jd_bfrt;
	bf_rt_table_attributes_hdl *tmp = (bf_rt_table_attributes_hdl *)eipOutShareMeterAttributes;

	bf_status = bf_rt_attributes_meter_byte_count_adjust_set(
		tmp, byte_count_adj);
  	assert(bf_status == BF_SUCCESS);

	 bf_status = bf_rt_table_attributes_set(
	 			eipOutShareMeterTable, 
				jd_bf_p->session, 
    			&jd_bf_p->dev_tgt, 
	#ifdef BFRT_GENERIC_FLAGS
							   0,
	#endif
				eipOutShareMeterAttributes);
  	assert(bf_status == BF_SUCCESS);
}

