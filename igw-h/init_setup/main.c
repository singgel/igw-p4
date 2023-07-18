/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/socket.h>
#include <semaphore.h>
#include <signal.h>
#include "jd_bfrt.h"
#include "utils.h"
#include "switch_device.h"
#include "switch_packet_table.h"
#include "switch_igw_ip_type_table.h"
#include "switch_eip_in_jd_vxlan_table.h"
#include "switch_eip_out_jd_vxlan_table.h"
#include "switch_eip_dl.h"
#include "switch_meter_drop_table.h"
#include "switch_mirror_table.h"
#include "switch_acl_table_init.h"
#include "switch_ecmp_group2.h"
#include "switch_meter_adjust.h"
#include "switch_symmetric_mode_set.h"

const char *g_init_setup_filename = "/var/run/init_setup.file";

extern int switchlink_init(void);

static int init_setup_check(void)
{
	int ret;
	pid_t pid;
	FILE *system_file = NULL;

	/* open system file */
	system_file = fopen(g_init_setup_filename, "r+");
	if (system_file == NULL)
		goto write_file;

	SETUP_PANIC("ERROR: The system has been initialized by init_setup program, Please make sure the system is not running!\n");
	return 0;
	
write_file:
	/* create new system file */
	system_file = fopen(g_init_setup_filename, "w+");
	if (system_file == NULL) {
		SETUP_PANIC("init_setup_check: create system file failed\n");
	}

	fclose(system_file);
	return 0;
}

static void coverage_handler(int signum) {
  printf("init_setup:received signal %d\n", signum);
  bf_switchd_exit_sighandler(signum);
  exit(-1);
}

static void setup_coverage_sighandler() {
  	struct sigaction new_action;
  	/* setup signal hander */
  	new_action.sa_handler = coverage_handler;
  	sigemptyset(&new_action.sa_mask);
  	new_action.sa_flags = 0;

  	sigaction(SIGKILL, &new_action, NULL);
  	sigaction(SIGTERM, &new_action, NULL);
  	sigaction(SIGQUIT, &new_action, NULL);
  	sigaction(SIGINT, &new_action, NULL);
}

static void bf_switchd_parse_hld_mgrs_list(bf_switchd_context_t *ctx,
                                           char *mgrs_list) {
  int len = strlen(mgrs_list);
  int i = 0;
  char mgr;

  while (i < len) {
    mgr = mgrs_list[i];
    switch (mgr) {
      case 'p':
        ctx->skip_hld.pipe_mgr = true;
        break;
      case 'm':
        ctx->skip_hld.mc_mgr = true;
        break;
      case 'k':
        ctx->skip_hld.pkt_mgr = true;
        break;
      case 'r':
        ctx->skip_hld.port_mgr = true;
        break;
      case 't':
        ctx->skip_hld.traffic_mgr = true;
        break;
      default:
        printf("Unknown skip-hld option %c \n", mgr);
        break;
    }
    i++;
  }
}

/* Parse cmd-line options of bf_switchd */
static void bf_switchd_parse_options(bf_switchd_context_t *ctx,
                                     int argc,
                                     char **argv) {
  char *skip_hld_mgrs_list = NULL;
  while (1) {
    int option_index = 0;
    /* Options without short equivalents */
    enum long_opts {
      OPT_START = 256,
      OPT_INSTALLDIR,
      OPT_CONFFILE,
      OPT_TCPPORTBASE,
      OPT_SKIP_P4,
      OPT_SKIP_HLD,
      OPT_SKIP_PORT_ADD,
      OPT_STS_PORT,
      OPT_KERNEL_PKT,
      OPT_BACKGROUND,
      OPT_UCLI,
      OPT_BFS_LOCAL,
      OPT_INIT_MODE,
      OPT_NO_PI,
      OPT_P4RT_SERVER,
      OPT_SHELL_NO_WAIT,
    };
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"install-dir", required_argument, 0, OPT_INSTALLDIR},
        {"conf-file", required_argument, 0, OPT_CONFFILE},
        {"tcp-port-base", required_argument, 0, OPT_TCPPORTBASE},
        {"skip-p4", no_argument, 0, OPT_SKIP_P4},
        {"skip-hld", required_argument, 0, OPT_SKIP_HLD},
        {"skip-port-add", no_argument, 0, OPT_SKIP_PORT_ADD},
        {"status-port", required_argument, 0, OPT_STS_PORT},
        {"kernel-pkt", no_argument, 0, OPT_KERNEL_PKT},
        {"background", no_argument, 0, OPT_BACKGROUND},
        {"ucli", no_argument, 0, OPT_UCLI},
        {"bfs-local-only", no_argument, 0, OPT_BFS_LOCAL},
        {"init-mode", required_argument, 0, OPT_INIT_MODE},
        {"p4rt-server", required_argument, 0, OPT_P4RT_SERVER},
        {"shell-no-wait", no_argument, 0, OPT_SHELL_NO_WAIT},
        {0, 0, 0, 0}};
    int c = getopt_long(argc, argv, "h", long_options, &option_index);
    if (c == -1) {
      break;
    }
    switch (c) {
      case OPT_INSTALLDIR:
        ctx->install_dir = strdup(optarg);
        printf("Install Dir: %s (%p)\n",
               ctx->install_dir,
               (void *)ctx->install_dir);
        break;
      case OPT_CONFFILE:
        ctx->conf_file = strdup(optarg);
        break;
      case OPT_TCPPORTBASE:
        ctx->tcp_port_base = atoi(optarg);
        break;
      case OPT_SKIP_P4:
        ctx->skip_p4 = true;
        break;
      case OPT_SKIP_HLD:
        skip_hld_mgrs_list = strdup(optarg);
        printf("Skip-hld-mgrs list is %s \n", skip_hld_mgrs_list);
        bf_switchd_parse_hld_mgrs_list(ctx, skip_hld_mgrs_list);
        free(skip_hld_mgrs_list);
        break;
      case OPT_SKIP_PORT_ADD:
        ctx->skip_port_add = true;
        break;
      case OPT_STS_PORT:
        ctx->dev_sts_thread = true;
        ctx->dev_sts_port = atoi(optarg);
        break;
      case OPT_KERNEL_PKT:
        ctx->kernel_pkt = true;
        break;
      case OPT_BACKGROUND:
        ctx->running_in_background = true;
        break;
      case OPT_UCLI:
        ctx->shell_set_ucli = true;
        break;
      case OPT_BFS_LOCAL:
        ctx->bfshell_local_only = true;
        break;
      case OPT_INIT_MODE:
        if (!strncmp(optarg, "cold", 4)) {
          ctx->init_mode = BF_DEV_INIT_COLD;
        } else if (!strncmp(optarg, "fastreconfig", 4)) {
          ctx->init_mode = BF_DEV_WARM_INIT_FAST_RECFG;
        } else if (!strncmp(optarg, "hitless", 4)) {
          ctx->init_mode = BF_DEV_WARM_INIT_HITLESS;
        } else {
          printf(
              "Unknown init mode, expected one of: \"cold\", \"fastreconfig\", "
              "\"hitless\"\nDefaulting to \"cold\"");
          ctx->init_mode = BF_DEV_INIT_COLD;
        }
        break;
      case OPT_P4RT_SERVER:
        ctx->p4rt_server = strdup(optarg);
        break;
      case OPT_SHELL_NO_WAIT:
        ctx->shell_before_dev_add = true;
        break;
      case 'h':
      case '?':
        printf("init_setup \n");
        printf("Usage: init_setup --conf-file <file> [OPTIONS]...\n");
        printf("\n");
        printf(" --install-dir=directory that has installed build artifacts\n");
        printf(" --conf-file=configuration file for bf_switchd\n");
        printf(" --tcp-port-base=TCP port base to be used for DMA sim\n");
        printf(" --skip-p4 Skip loading P4 program\n");
        printf(" --skip-hld Skip high level drivers\n");
        printf(
            "   p:pipe_mgr, m:mc_mgr, k:pkt_mgr, r:port_mgr, t:traffic_mgr\n");
        printf(" --skip-port-add Skip adding ports\n");
        printf(" --background Disable interactive features so bf_switchd\n");
        printf("              can run in the background\n");
        printf(" --init-mode Specify cold boot or warm init mode\n");
        printf(
            " cold:Cold boot device, fastreconfig:Apply fast reconfig to "
            "device\n");
        printf(" --p4rt-server=<addr:port> Run the P4Runtime gRPC server\n");
        printf(" --shell-no-wait Start the shell before devices are added\n");
        printf(" -h,--help Display this help message and exit\n");
        exit(c == 'h' ? 0 : 1);
        break;
    }
  }

  /* Sanity check args */
  if ((ctx->install_dir == NULL) || (ctx->conf_file == NULL)) {
    printf("ERROR: --install-dir and --conf-file must be specified\n");
    exit(0);
  }
}

/* Parse cmd-line options of bf_switchd */
static void bf_switchd_parse_options_v2(bf_switchd_context_t *ctx,
                                     int argc,
                                     char **argv) {
  char *skip_hld_mgrs_list = NULL;
  while (1) {
    int option_index = 0;
    /* Options without short equivalents */
    enum long_opts {
      OPT_START = 256,
      OPT_INSTALLDIR,
      OPT_CONFFILE,
      OPT_TCPPORTBASE,
      OPT_SKIP_P4,
      OPT_SKIP_HLD,
      OPT_SKIP_PORT_ADD,
      OPT_STS_PORT,
      OPT_KERNEL_PKT,
      OPT_BACKGROUND,
      OPT_UCLI,
      OPT_BFS_LOCAL,
      OPT_INIT_MODE,
      OPT_NO_PI,
      OPT_P4RT_SERVER,
      OPT_SHELL_NO_WAIT,
      OPT_STATUS_SERVER_LOCALHOST_ONLY,
      OPT_DMA_CHANNEL_SERVER_LOCALHOST_ONLY,
      OPT_REG_CHANNEL_SERVER_LOCALHOST_ONLY,
      OPT_FPGA_CHANNEL_SERVER_LOCALHOST_ONLY,
      OPT_BF_RT_SERVER_LOCALHOST_ONLY,
      OPT_SERVER_LISTEN_ON_LOCALHOST_ONLY,
    };
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"install-dir", required_argument, 0, OPT_INSTALLDIR},
        {"conf-file", required_argument, 0, OPT_CONFFILE},
        {"tcp-port-base", required_argument, 0, OPT_TCPPORTBASE},
        {"skip-p4", no_argument, 0, OPT_SKIP_P4},
        {"skip-hld", required_argument, 0, OPT_SKIP_HLD},
        {"skip-port-add", no_argument, 0, OPT_SKIP_PORT_ADD},
        {"status-port", required_argument, 0, OPT_STS_PORT},
        {"kernel-pkt", no_argument, 0, OPT_KERNEL_PKT},
        {"background", no_argument, 0, OPT_BACKGROUND},
        {"ucli", no_argument, 0, OPT_UCLI},
        {"bfs-local-only", no_argument, 0, OPT_BFS_LOCAL},
        {"status-server-local-only",
         no_argument,
         0,
         OPT_STATUS_SERVER_LOCALHOST_ONLY},
        {"dma-channel-server-local-only",
         no_argument,
         0,
         OPT_DMA_CHANNEL_SERVER_LOCALHOST_ONLY},
        {"reg-channel-server-local-only",
         no_argument,
         0,
         OPT_REG_CHANNEL_SERVER_LOCALHOST_ONLY},
        {"fpga-channel-server-local-only",
         no_argument,
         0,
         OPT_FPGA_CHANNEL_SERVER_LOCALHOST_ONLY},
        {"bfrt-grpc-server-local-only",
         no_argument,
         0,
         OPT_BF_RT_SERVER_LOCALHOST_ONLY},
        {"server-listen-local-only",
         no_argument,
         0,
         OPT_SERVER_LISTEN_ON_LOCALHOST_ONLY},
        {"init-mode", required_argument, 0, OPT_INIT_MODE},
        {"p4rt-server", required_argument, 0, OPT_P4RT_SERVER},
        {"shell-no-wait", no_argument, 0, OPT_SHELL_NO_WAIT},
        {0, 0, 0, 0}};
    int c = getopt_long(argc, argv, "h", long_options, &option_index);
    if (c == -1) {
      break;
    }
    switch (c) {
      case OPT_INSTALLDIR:
        ctx->install_dir = strdup(optarg);
        break;
      case OPT_CONFFILE:
        ctx->conf_file = strdup(optarg);
        break;
      case OPT_TCPPORTBASE:
        ctx->tcp_port_base = atoi(optarg);
        break;
      case OPT_SKIP_P4:
        ctx->skip_p4 = true;
        break;
      case OPT_SKIP_HLD:
        skip_hld_mgrs_list = strdup(optarg);
        printf("Skip-hld-mgrs list is %s \n", skip_hld_mgrs_list);
        bf_switchd_parse_hld_mgrs_list(ctx, skip_hld_mgrs_list);
        free(skip_hld_mgrs_list);
        break;
      case OPT_SKIP_PORT_ADD:
        ctx->skip_port_add = true;
        break;
      case OPT_STS_PORT:
        ctx->dev_sts_thread = true;
        ctx->dev_sts_port = atoi(optarg);
        break;
      case OPT_KERNEL_PKT:
        ctx->kernel_pkt = true;
        break;
      case OPT_BACKGROUND:
        ctx->running_in_background = true;
        break;
      case OPT_UCLI:
        ctx->shell_set_ucli = true;
        break;
      case OPT_BFS_LOCAL:
        ctx->bfshell_local_only = true;
        break;
      case OPT_STATUS_SERVER_LOCALHOST_ONLY:
        ctx->status_server_local_only = true;
        break;
      case OPT_DMA_CHANNEL_SERVER_LOCALHOST_ONLY:
        ctx->dma_channel_server_local_only = true;
        break;
      case OPT_REG_CHANNEL_SERVER_LOCALHOST_ONLY:
        ctx->regular_channel_server_local_only = true;
        break;
      case OPT_FPGA_CHANNEL_SERVER_LOCALHOST_ONLY:
        ctx->fpga_channel_server_local_only = true;
        break;
      case OPT_BF_RT_SERVER_LOCALHOST_ONLY:
        ctx->bf_rt_server_local_only = true;
        break;
      case OPT_SERVER_LISTEN_ON_LOCALHOST_ONLY:
        ctx->server_listen_local_only = true;
        break;
      case OPT_INIT_MODE:
        if (!strncmp(optarg, "cold", 4)) {
          ctx->init_mode = BF_DEV_INIT_COLD;
        } else if (!strncmp(optarg, "fastreconfig", 4)) {
          ctx->init_mode = BF_DEV_WARM_INIT_FAST_RECFG;
        } else if (!strncmp(optarg, "hitless", 4)) {
          ctx->init_mode = BF_DEV_WARM_INIT_HITLESS;
        } else {
          printf(
              "Unknown init mode, expected one of: \"cold\", \"fastreconfig\", "
              "\"hitless\"\nDefaulting to \"cold\"");
          ctx->init_mode = BF_DEV_INIT_COLD;
        }
        break;
      case OPT_P4RT_SERVER:
        ctx->p4rt_server = strdup(optarg);
        break;
      case OPT_SHELL_NO_WAIT:
        ctx->shell_before_dev_add = true;
        break;
      case 'h':
      case '?':
        printf("bf_switchd \n");
        printf("Usage: bf_switchd --conf-file <file> [OPTIONS]...\n");
        printf("\n");
        printf(" --install-dir=directory that has installed build artifacts\n");
        printf(" --conf-file=configuration file for bf_switchd\n");
        printf(" --tcp-port-base=TCP port base to be used for DMA sim\n");
        printf(" --skip-p4 Skip loading P4 program\n");
        printf(" --skip-hld Skip high level drivers\n");
        printf(
            "   p:pipe_mgr, m:mc_mgr, k:pkt_mgr, r:port_mgr, t:traffic_mgr\n");
        printf(" --skip-port-add Skip adding ports\n");
        printf(" --background Disable interactive features so bf_switchd\n");
        printf("              can run in the background\n");
        printf(" --init-mode Specify cold boot or warm init mode\n");
        printf(
            " cold:Cold boot device, fastreconfig:Apply fast reconfig to "
            "device\n");
        printf(" --p4rt-server=<addr:port> Run the P4Runtime gRPC server\n");
        printf(" --shell-no-wait Start the shell before devices are added\n");
        printf(" -h,--help Display this help message and exit\n");
        exit(c == 'h' ? 0 : 1);
        break;
    }
  }

  /* Sanity check args */
  if ((ctx->install_dir == NULL) || (ctx->conf_file == NULL)) {
    printf("ERROR: --install-dir and --conf-file must be specified\n");
    exit(0);
  }
}

static void bf_switchd_init_sig_set(sigset_t *set) {
  sigemptyset(set);
  sigaddset(set, SIGQUIT);
  sigaddset(set, SIGTERM);
  sigaddset(set, SIGUSR1);
}

static void *bf_switchd_nominated_signal_thread(void *arg) {
  (void)arg;
  sigset_t set;
  siginfo_t info;
  int s, signum;

  bf_switchd_init_sig_set(&set);

  s = pthread_detach(pthread_self());
  if (s != 0) {
    perror("pthread_detach");
    exit(-1);
  }

  for (;;) {
    signum = sigwaitinfo(&set, &info);
    if (signum == -1) {
      if (errno == EINTR) continue;
      perror("sigwait");
      continue;
    }
    switch (signum) {
      case SIGQUIT:
      case SIGTERM:
        bf_switchd_exit_sighandler(signum);
        exit(0);
        break;
      case SIGUSR1:
#ifdef COVERAGE_ENABLED
        extern void __gcov_flush(void);
        /* coverage signal handler to allow flush of coverage data*/
        __gcov_flush(); /* dump coverage data on receiving SIGUSR1 */
#endif
        exit(-1);
        break;
      default:
        printf("bf_switchd:received signal %d\n", signum);
        break;
    }
  }
  pthread_exit(NULL);
}

/* Use dedicated signal thread to handle async signal
 * All other threads created by main() will inherit a
 * copy of above signal mask, which blocks those signals.
 *
 * There are synchronous and asynchronous signals.
 *
 * Synchronous signals like SIGSEGV, SIGILL, SIGBUS, SIGFPE, etc
 * are only delivered to the thread that caused it, so if we want to
 * handle those signals we need 1) not mask those signal  and 2) establish a
 * process-wide signal handler with sigaction().
 *
 * Asynchronous signals can be handled by a separate thread, which
 * calls sigwait() and blocks until a signal arrives.
 * To handle those signals we block them in all threads in bf_switchd process,
 * including the thread calling sigwait().
 * This will only work if an asynchronous signal is sent to the whole process,
 * if one thread calls pthread_kill() to send a signal to another thread, or
 * calls raise() which only sends signal to the thread who raises it,
 * that signal will be treated as synchronous.
 */
static void setup_async_signal_handling_thread() {
  sigset_t set;
  bf_switchd_init_sig_set(&set);
  /* add more asynchronous signal if needed
   * exclude: SIGIO  - used in bf_switchd.c
   * exclude: SIGINT - GDB does not catch signal with sigwait
   * */

  /* block the signals in this process by default*/
  int s = pthread_sigmask(SIG_BLOCK, &set, NULL);
  if (s != 0) {
    perror("pthread_sigmask");
    exit(1);
  }
  pthread_t thread;
  s = pthread_create(&thread, NULL, &bf_switchd_nominated_signal_thread, NULL);
  if (s != 0) {
    perror("pthread_create");
    exit(1);
  }
  s = pthread_setname_np(thread, "bf_signal");
  if (s != 0) {
    perror("pthread_setname_np");
    exit(1);
  }
}

static int bf_switchd_main(bf_switchd_context_t *switchd_main_ctx
	,int argc, char **argv) {
	int ret = 0;
	int agent_idx = 0;
	
#if defined (USE_SDE_9_7)
	setup_async_signal_handling_thread();
	bf_switchd_parse_options_v2(switchd_main_ctx, argc, argv);
#else
  	setup_coverage_sighandler();
    /* Parse bf_switchd arguments */
  	bf_switchd_parse_options(switchd_main_ctx, argc, argv);
#endif

    switchd_main_ctx->kernel_pkt = true;
	ret = bf_switchd_lib_init(switchd_main_ctx);
	return ret;
}

static void syslog_open(void) {
	openlog("init_setup", 0, 6);
}

static void init_setup_daemonize(void) {
	pid_t pid;
	if ((pid = fork()) < 0) {
		SETUP_PANIC("fork error:%s\n", strerror(errno));		
	} else if (pid != 0) {
		/* parent process exit */
		exit(0);
	}
	if (setsid() < 0) {
		SETUP_PANIC("setsid error:%s\n", strerror(errno));
	}
	if (0 != fork())
		exit(0);
	/* align the tty output */
	printf("\n");
}

#define P4_NAME "igw_switch"

extern void switch_monitor_init(void);
extern sem_t netlink_sem;
int main(int argc, char **argv) {	
	int agent_idx = 0;
	bf_switchd_context_t *switchd_main_ctx = NULL;

	init_setup_check();
	init_setup_daemonize();
	syslog_open();

  	/* Allocate memory to hold switchd configuration and state */
  	if ((switchd_main_ctx = malloc(sizeof(bf_switchd_context_t))) == NULL) {
    	printf("ERROR: Failed to allocate memory for switchd context\n");
    	return -1;
  	}
  	memset(switchd_main_ctx, 0, sizeof(bf_switchd_context_t));	
	bf_switchd_main(switchd_main_ctx, argc, argv);

	switch_device_create();
	bf_rt_setup(P4_NAME);
	mirror_table_init();
	igw_ip_type_table_init();
	eip_in_jd_vxlan_table_init();
	eip_out_jd_vxlan_table_init();
	eip_ecmp_dl_table_init();
	meter_drop_table_init();
	acl_table_init();
	ecmp_group02_init();
	meter_adjust_init();
	symmetric_mode_set_init();
	
	/*switchlink_init must execute before 
		process_protocol_packet_table_init*/
	switchlink_init(); 
	sem_wait(&netlink_sem);
	sleep(1);
	process_protocol_packet_table_init();
	switch_monitor_init();

  	pthread_join(switchd_main_ctx->tmr_t_id, NULL);
  	pthread_join(switchd_main_ctx->dma_t_id, NULL);
  	pthread_join(switchd_main_ctx->int_t_id, NULL);
  	pthread_join(switchd_main_ctx->pkt_t_id, NULL);
  	pthread_join(switchd_main_ctx->port_fsm_t_id, NULL);
  	pthread_join(switchd_main_ctx->drusim_t_id, NULL);
  	pthread_join(switchd_main_ctx->accton_diag_t_id, NULL);
  	for (agent_idx = 0; agent_idx < BF_SWITCHD_MAX_AGENTS; agent_idx++) {
    	if (switchd_main_ctx->agent_t_id[agent_idx] != 0) {
      	pthread_join(switchd_main_ctx->agent_t_id[agent_idx], NULL);
    	}
  	}
	
  	if (switchd_main_ctx) {	
	#if defined (USE_SDE_9_7)
	    free(switchd_main_ctx->p4rt_server);
    	free(switchd_main_ctx->install_dir);
    	free(switchd_main_ctx->conf_file);
	#endif
		free(switchd_main_ctx);	
	}
	
	return 0;
}
