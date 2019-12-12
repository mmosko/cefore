/*
 * Copyright (c) 2016, National Institute of Information and Communications
 * Technology (NICT). All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the NICT nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NICT AND CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE NICT OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/*
 * cefputstream.c
 */

#define __CEF_PUTSTREAM_SOURECE__

/****************************************************************************************
 Include Files
 ****************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <limits.h>
#include <sys/time.h>

#include <cefore/cef_define.h>
#include <cefore/cef_frame.h>
#include <cefore/cef_client.h>
#include <cefore/cef_valid.h>
#include <cefore/cef_log.h>

/****************************************************************************************
 Macros
 ****************************************************************************************/
#define	T_USEC				(1000000)		/* 1(sec) = 1000000(usec)	*/

/****************************************************************************************
 Structures Declaration
 ****************************************************************************************/


/****************************************************************************************
 State Variables
 ****************************************************************************************/

static int app_running_f = 0;
CefT_Client_Handle fhdl;
static struct timeval start_t;
static struct timeval end_t;
static uint64_t stat_send_frames = 0;
static uint64_t stat_send_bytes = 0;

static uint64_t stat_jitter_sum = 0;
static uint64_t stat_jitter_sq_sum = 0;
static uint64_t stat_jitter_max = 0;


/****************************************************************************************
 Static Function Declaration
 ****************************************************************************************/

static void
post_process (
	void
);
static void
sigcatch (
	int sig
);
static void
print_usage (
	void
);

/****************************************************************************************
 ****************************************************************************************/
int main (
	int argc,
	char** argv
) {
	int res;
	unsigned char buff[CefC_Max_Length];
	CefT_Object_TLVs prames;
	int seqnum = 0;
	int opt;
	char uri[1024];

	double interval;
	long interval_us;
	static struct timeval now_t;
	uint64_t next_tus;
	uint64_t now_tus;
	uint64_t now_ms;
	uint64_t jitter;
	char*	work_arg;
	int 	i;
	
	char 	conf_path[PATH_MAX] = {0};
	int 	port_num = CefC_Unset_Port;
	
	/***** flags 		*****/
	int uri_f 		= 0;
	int rate_f 		= 0;
	int blocks_f 	= 0;
	int expiry_f 	= 0;
	int cachet_f 	= 0;
	int dir_path_f 	= 0;
	int port_num_f 	= 0;
	int key_path_f 	= 0;
	
	/***** parameters 	*****/
	uint16_t cache_time 	= 0;
	uint64_t expiry 		= 0;
	int rate 				= 5;
	int block_size 			= 1024;
	
	/*------------------------------------------
		Checks specified options
	--------------------------------------------*/
	uri[0] = 0;
	fprintf (stderr, "[cefputstream] Start\n");
	fprintf (stderr, "[cefputstream] Parsing parameters ... ");
	
	/* Inits logging 		*/
	cef_log_init ("cefputstream");
	
	/* Obtains options 		*/
	for (i = 1 ; i < argc ; i++) {
		
		work_arg = argv[i];
		if (work_arg == NULL || work_arg[0] == 0) {
			break;
		}
		
		if (strcmp (work_arg, "-r") == 0) {
			if (rate_f) {
				fprintf (stderr, "ERROR: [-r] is duplicated.\n");
				print_usage ();
				return (-1);
			}
			if (i + 1 == argc) {
				fprintf (stderr, "ERROR: [-r] has no parameter.\n");
				print_usage ();
				return (-1);
			}
			work_arg = argv[i + 1];
			rate = atoi (work_arg);
			if ((rate < 1) || (rate > 32)) {
				rate = 1;
			}
			rate_f++;
			i++;
		} else if (strcmp (work_arg, "-b") == 0) {
			if (blocks_f) {
				fprintf (stderr, "ERROR: [-b] is duplicated.\n");
				print_usage ();
				return (-1);
			}
			if (i + 1 == argc) {
				fprintf (stderr, "ERROR: [-b] has no parameter.\n");
				print_usage ();
				return (-1);
			}
			work_arg = argv[i + 1];
			block_size = atoi (work_arg);
			
			if (block_size < 60) {
				block_size = 60;
			}
			if (block_size > 1460) {
				block_size = 1460;
			}
			blocks_f++;
			i++;
		} else if (strcmp (work_arg, "-e") == 0) {
			if (expiry_f) {
				fprintf (stderr, "ERROR: [-e] is duplicated.\n");
				print_usage ();
				return (-1);
			}
			if (i + 1 == argc) {
				fprintf (stderr, "ERROR: [-e] has no parameter.\n");
				print_usage ();
				return (-1);
			}
			work_arg = argv[i + 1];
			expiry = atoi (work_arg);
			
			if (expiry > 86400) {
				expiry = 86400;
			}
			expiry_f++;
			i++;
		} else if (strcmp (work_arg, "-t") == 0) {
			if (cachet_f) {
				fprintf (stderr, "ERROR: [-t] is duplicated.\n");
				print_usage ();
				return (-1);
			}
			if (i + 1 == argc) {
				fprintf (stderr, "ERROR: [-t] has no parameter.\n");
				print_usage ();
				return (-1);
			}
			work_arg = argv[i + 1];
			cache_time = atoi (work_arg);
			
			if ((cache_time < 0) || (cache_time > 65535)) {
				cache_time = 10;
			}
			cachet_f++;
			i++;
		} else if (strcmp (work_arg, "-k") == 0) {
			if (key_path_f) {
				fprintf (stderr, "ERROR: [-k] is duplicated.\n");
				print_usage ();
				return (-1);
			}
			key_path_f++;
		} else if (strcmp (work_arg, "-h") == 0) {
			print_usage ();
			exit (1);
		} else if (strcmp (work_arg, "-d") == 0) {
			if (dir_path_f) {
				fprintf (stderr, "ERROR: [-d] is duplicated.\n");
				print_usage ();
				return (-1);
			}
			if (i + 1 == argc) {
				fprintf (stderr, "ERROR: [-d] has no parameter.\n");
				print_usage ();
				return (-1);
			}
			work_arg = argv[i + 1];
			strcpy (conf_path, work_arg);
			dir_path_f++;
			i++;
		} else if (strcmp (work_arg, "-p") == 0) {
			if (port_num_f) {
				fprintf (stderr, "ERROR: [-p] is duplicated.\n");
				print_usage ();
				return (-1);
			}
			if (i + 1 == argc) {
				fprintf (stderr, "ERROR: [-p] has no parameter.\n");
				print_usage ();
				return (-1);
			}
			work_arg = argv[i + 1];
			port_num = atoi (work_arg);
			port_num_f++;
			i++;
		} else {
			
			work_arg = argv[i];
			
			if (work_arg[0] == '-') {
				fprintf (stderr, "ERROR: unknown option is specified.\n");
				print_usage ();
				return (-1);
			}
			
			if (uri_f) {
				fprintf (stderr, "ERROR: uri is duplicated.\n");
				print_usage ();
				return (-1);
			}
			res = strlen (work_arg);
			
			if (res >= 1204) {
				fprintf (stderr, "ERROR: uri is too long.\n");
				print_usage ();
				return (-1);
			}
			strcpy (uri, work_arg);
			uri_f++;
		}
	}
	
	if (uri_f == 0) {
		fprintf (stderr, "ERROR: URI is not specified.\n");
		print_usage ();
		exit (1);
	}
	fprintf (stderr, "OK\n");
#ifdef CefC_Debug
	cef_dbg_init ("cefputstream", conf_path, 1);
#endif // CefC_Debug
	
	/*------------------------------------------
		Creates the name from URI
	--------------------------------------------*/
	memset (&prames, 0, sizeof (CefT_Object_TLVs));
	cef_frame_init ();
	res = cef_client_init (port_num, conf_path);
	if (res < 0) {
		fprintf (stderr, "ERROR: Failed to init the client package.\n");
		exit (1);
	}
	fprintf (stderr, "[cefputstream] Init Cefore Client package ... OK\n");
	fprintf (stderr, "[cefputstream] Conversion from URI into Name ... ");
	res = cef_frame_conversion_uri_to_name (uri, prames.name);
	if (res < 0) {
		fprintf (stderr, "ERROR: Invalid URI is specified.\n");
		exit (1);
	}
	prames.name_len 	= res;
	prames.chnk_num_f 	= 1;
	fprintf (stderr, "OK\n");
	
	/*------------------------------------------
		Checks Validation 
	--------------------------------------------*/
	if (key_path_f) {
		prames.alg.pubkey_len = cef_valid_read_pubkey (conf_path, prames.alg.pubkey);
		
		if (prames.alg.pubkey_len > 0) {
			fprintf (stderr, 
				"[cefputstream] Read the public key ... OK\n");
		} else {
			fprintf (stderr, 
				"[cefputstream] Read the public key ... NG\n");
			exit (1);
		}
	}
	
	/*------------------------------------------
		Sets Expiry Time and RCT
	--------------------------------------------*/
	gettimeofday (&now_t, NULL);
	now_ms = now_t.tv_sec * 1000 + now_t.tv_usec / 1000;
	
	if (cache_time > 0) {
		prames.opt.cachetime_f 	= 1;
		prames.opt.cachetime 	= now_ms + cache_time * 1000;
	} else {
		prames.opt.cachetime_f 	= 1;
		prames.opt.cachetime 	= now_ms;
	}
	
	if (expiry > 0) {
		prames.expiry = now_ms + expiry * 1000;
	} else {
		prames.expiry = 0;
	}
	
	/*------------------------------------------
		Connects to CEFORE
	--------------------------------------------*/
	fprintf (stderr, "[cefputstream] Connect to cefnetd ... ");
	fhdl = cef_client_connect ();
	if (fhdl < 1) {
		fprintf (stderr, "ERROR: cefnetd is not running.\n");
		exit (1);
	}
	app_running_f = 1;
	fprintf (stderr, "OK\n");
	
	/*------------------------------------------
		Calculates the interval
	--------------------------------------------*/
	interval = (double)((double) rate * T_USEC) / (double)(block_size * 8);
	interval_us = (long)((1.0 / interval) * T_USEC);

	/*------------------------------------------
		Main Loop
	--------------------------------------------*/
	fprintf (stderr, "[cefputstream] URI         = %s\n", uri);
	fprintf (stderr, "[cefputstream] Rate        = %d Mbps\n", rate);
	fprintf (stderr, "[cefputstream] Block Size  = %d Bytes\n", block_size);
	fprintf (stderr, "[cefputstream] Cache Time  = %d sec\n", cache_time);
	fprintf (stderr, "[cefputstream] Expiration  = "FMTU64" sec\n", expiry);
	
	memset (buff, 1, CefC_Max_Length);
	gettimeofday (&start_t, NULL);
	next_tus = start_t.tv_sec * T_USEC + start_t.tv_usec;

	if (signal(SIGINT, sigcatch) == SIG_ERR){
		fprintf (stderr, "[cefputstream] ERROR: signal(SIGINT)");
	}
	if (signal(SIGTERM, sigcatch) == SIG_ERR){
		fprintf (stderr, "[cefputstream] ERROR: signal(SIGTERM)");
	}
	if (signal(SIGPIPE, sigcatch) == SIG_ERR){
		fprintf (stderr, "[cefputstream] ERROR: signal(SIGPIPE)");
	}
	fprintf (stderr, "[cefputstream] Start creating Content Objects\n");
	
	while (app_running_f) {
		gettimeofday (&now_t, NULL);
		now_tus = now_t.tv_sec * T_USEC + now_t.tv_usec;

		if (now_tus > next_tus) {
			
			if (stat_send_frames > 0) {
				jitter = (now_t.tv_sec - end_t.tv_sec) * T_USEC
								+ (now_t.tv_usec - end_t.tv_usec);

				stat_jitter_sum    += jitter;
				stat_jitter_sq_sum += jitter * jitter;
				if (jitter > stat_jitter_max) {
					stat_jitter_max = jitter;
				}
			}
			
			res = read (0, buff, block_size);
			
			if (res > 0) {
				memcpy (prames.payload, buff, res);
				prames.payload_len = (uint16_t) res;
				prames.chnk_num = seqnum;
				cef_client_object_input (fhdl, &prames);
				stat_send_frames++;
				stat_send_bytes += res;
				seqnum++;
				
				gettimeofday (&end_t, NULL);
			} else {
				break;
			}
			next_tus = now_tus + interval_us;
		}
	}
	post_process ();
	exit (0);
}

static void
print_usage (
	void
) {
	
	fprintf (stderr, "\nUsage: \n");
	fprintf (stderr, "  cefputstream uri [-r rate] [-b block_size] [-e expiry] "
					 "[-t cache_time]\n\n");
	
}

static void
post_process (
	void
) {
	uint64_t diff_t;
	uint64_t send_bits;
	uint64_t jitter_ave;
	
	if (stat_send_frames) {
		diff_t = ((end_t.tv_sec - start_t.tv_sec) * T_USEC
							+ (end_t.tv_usec - start_t.tv_usec)) / T_USEC;
	} else {
		diff_t = 0;
	}
	usleep (T_USEC);
	fprintf (stderr, "[cefputstream] Unconnect to cefnetd ... ");
	cef_client_close (fhdl);
	fprintf (stderr, "OK\n");
	
	fprintf (stderr, "[cefputstream] Stop\n");
	fprintf (stderr, "[cefputstream] Tx Frames = "FMTU64"\n", stat_send_frames);
	fprintf (stderr, "[cefputstream] Tx Bytes  = "FMTU64"\n", stat_send_bytes);
	fprintf (stderr, "[cefputstream] Duration  = "FMTU64"\n", diff_t);
	if (diff_t > 0) {
		send_bits = stat_send_bytes * 8;
		fprintf (stderr, "[cefputstream] Thorghput = "FMTU64"\n", send_bits / diff_t);
	}
	if (stat_send_frames > 0) {
		jitter_ave = stat_jitter_sum / stat_send_frames;

		fprintf (stderr, "[cefputstream] Jitter (Ave) = "FMTU64" us\n", jitter_ave);
		fprintf (stderr, "[cefputstream] Jitter (Max) = "FMTU64" us\n", stat_jitter_max);
		fprintf (stderr, "[cefputstream] Jitter (Var) = "FMTU64" us\n"
			, (stat_jitter_sq_sum / stat_send_frames) - (jitter_ave * jitter_ave));
	}
	exit (0);
}

static void
sigcatch (
	int sig
) {
	fprintf (stderr, "[cefputstream] Catch the signal\n");
	switch ( sig ){
	case SIGINT:
	case SIGTERM:
	case SIGPIPE:
		app_running_f = 0;
		break;
	}
}
