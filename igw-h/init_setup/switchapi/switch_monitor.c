/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

#include <stdint.h>
#include <pthread.h>
#include "utils.h"
#include "switch_port.h"
#include "switch_device_int.h"
#include "switch_monitor.h"
#include "switch_config.h"

static pthread_t monitor_thread;
static monitor_devport_array_t g_monitor_devports;

#if 0
static uint8_t get_pipe_from_dev_port_65x(uint32_t dev_port) {
	uint8_t pipe = 0;

	if (dev_port >= 0 && dev_port <= 60) {
		pipe = 0;
	} else if (dev_port >= 128 && dev_port <= 188) {
		pipe = 1;
	} else if (dev_port >= 256 && dev_port <= 316) {
		pipe = 2;
	} else if (dev_port >= 384 && dev_port <= 444) {
		pipe = 3;
	} 
	
	return pipe;
}

static uint8_t get_pipe_from_dev_port_32x(uint32_t dev_port) {
	uint8_t pipe = 0;

	if (dev_port >= 0 && dev_port <= 60) {
		pipe = 0;
	} else if (dev_port >= 128 && dev_port <= 188) {
		pipe = 1;
	} 
	
	return pipe;
}

static uint8_t get_pipe_from_dev_port(uint32_t dev_port) {
	if (switch_cfg.hardware_model == Wedge_100BF_65X) {
		return get_pipe_from_dev_port_65x(dev_port);
	} 
	return get_pipe_from_dev_port_32x(dev_port);
}
#endif

static void monitor_devports_init(){
	int i;	
    switch_hostif_t *hostif;
	
	g_monitor_devports.devport_num = 0;
	for (i = 0; i < g_hostif_info_array.hostif_num; i++) {
		hostif = &g_hostif_info_array.hostifs[i].hostif;
		g_monitor_devports.devports[i].dev_port = hostif->dev_port;		
		g_monitor_devports.devports[i].fp_port = hostif->fake_fp_port;
		g_monitor_devports.devports[i].pipe = DEV_PORT_TO_PIPE(hostif->dev_port);
		g_monitor_devports.devports[i].crcError = 0;		
		g_monitor_devports.devports[i].FCSError = 0;
		g_monitor_devports.devports[i].ig_tm_count = 0;		
		g_monitor_devports.devports[i].eg_tm_count = 0;
		g_monitor_devports.devport_num++;
	}	
}

static void port_syncd_monitor(){
	int i;	
    monitor_devport_t *mdp;
	uint64_t value;
	bf_status_t bf_status;
	
	for (i = 0; i < g_monitor_devports.devport_num; i++) {
		mdp = &g_monitor_devports.devports[i];
		bf_status = bf_pal_port_this_stat_get(DEVICE_ID, mdp->dev_port,
			bf_mac_stat_CRCErrorStomped, &value);
		if (bf_status != BF_SUCCESS) {
			continue;
		}

		if ((value > 0) && (value != mdp->crcError)) {
			mdp->crcError = value;
			SETUP_LOG("IGW_ERROR: fp_port: %d CRCError: %d", mdp->fp_port, value);
		}

		bf_status = bf_pal_port_this_stat_get(DEVICE_ID, mdp->dev_port,
			bf_mac_stat_FramesReceivedwithFCSError, &value);
		if (bf_status != BF_SUCCESS) {
			continue;
		}

		if ((value > 0) && (value != mdp->FCSError)) {
			mdp->FCSError = value;
			SETUP_LOG("IGW_ERROR: fp_port: %d FCSError: %d", mdp->fp_port, value);
		}
	}	
}

static void tm_port_drop_monitor() {
	int i;	
    monitor_devport_t *mdp;
	uint64_t ig_count = 0;	
	uint64_t eg_count = 0;
	bf_status_t bf_status;
	
	for (i = 0; i < g_monitor_devports.devport_num; i++) {
		mdp = &g_monitor_devports.devports[i];
		/** Get per port drop count.
 		* On Ingress, if packet is dropped when usage crosses PPG or
 		* or Port drop limit, this counter gets incremented.
 		* On Egress, queue tail drop are also accounted against port.
		* @param ig_count	   Per port Packet drops from Ingress TM perspective.
		* @param eg_count	   Per port Packet drops from Egress TM perspective.
 		*/
		bf_status = bf_tm_port_drop_get(DEVICE_ID, mdp->pipe,mdp->dev_port,
			&ig_count, &eg_count);
		if (bf_status != BF_SUCCESS) {
			continue;
		}

		if ((eg_count > 0) && (eg_count != mdp->eg_tm_count)) {
			mdp->eg_tm_count = eg_count;
			SETUP_LOG("IGW_ERROR: TM port drop fp_port: %d eg_count: %d", 
			mdp->fp_port, eg_count);
		}

		if ((ig_count > 0) && (ig_count != mdp->ig_tm_count)) {
			mdp->ig_tm_count = ig_count;
			SETUP_LOG("IGW_ERROR: TM port drop fp_port: %d ig_count: %d", 
			mdp->fp_port, ig_count);
		}
	}
}

static void *switch_monitor_main(void *args) {
	int time;

	monitor_devports_init();
	while (1) {		
		port_syncd_monitor();
		tm_port_drop_monitor();
		
		time = MONITOR_CYCLE;
		do {
			time = sleep(time);
		} while(time > 0);
	}
	
  	return NULL;
}

void switch_monitor_init() {
  	if (pthread_create(&monitor_thread, NULL, switch_monitor_main, NULL) !=0)
		SETUP_PANIC("switch_monitor_main thread create fail!\n");
	return ;
}

