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

static pthread_t monitor_thread;
static monitor_devport_array_t g_monitor_devports;

static void monitor_devports_init(){
	int i;	
    switch_hostif_t *hostif;
	
	g_monitor_devports.devport_num = 0;
	for (i = 0; i < g_hostif_info_array.hostif_num; i++) {
		hostif = &g_hostif_info_array.hostifs[i].hostif;
		g_monitor_devports.devports[i].dev_port = hostif->dev_port;		
		g_monitor_devports.devports[i].fp_port = hostif->fake_fp_port;
		g_monitor_devports.devports[i].crcError = 0;		
		g_monitor_devports.devports[i].FCSError = 0;
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
			SETUP_LOG("BGW_ERROR: fp_port: %d CRCError: %d", mdp->fp_port, value);
		}

		bf_status = bf_pal_port_this_stat_get(DEVICE_ID, mdp->dev_port,
			bf_mac_stat_FramesReceivedwithFCSError, &value);
		if (bf_status != BF_SUCCESS) {
			continue;
		}

		if ((value > 0) && (value != mdp->FCSError)) {
			mdp->FCSError = value;
			SETUP_LOG("BGW_ERROR: fp_port: %d FCSError: %d", mdp->fp_port, value);
		}
	}	
}

static void *switch_monitor_main(void *args) {
	int time;

	monitor_devports_init();
	while (1) {		
		port_syncd_monitor();
		
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

