/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

#ifndef _SWITCH_MONITOR_INIT_H__
#define _SWITCH_MONITOR_INIT_H__

#include <stdint.h>
#include "switch_hostif.h"

#define MONITOR_CYCLE  (20)

typedef struct monitor_devport_s {
	uint32_t fp_port;		
	uint8_t  pipe;	
	uint32_t dev_port;
	uint64_t crcError;	
	uint64_t FCSError;
	uint64_t ig_tm_count;	
	uint64_t eg_tm_count;
} monitor_devport_t;

typedef struct monitor_devport_array_s {
	int devport_num;
	monitor_devport_t devports[HOSTIF_ARRAY_SIZE];
} monitor_devport_array_t;

#define DOWN 0
#define UP 1
#define MAX_DOWN_COUNT 15

#endif

