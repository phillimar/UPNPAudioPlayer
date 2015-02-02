/*
This file is part of UPNPAudioPlayer
Copyright (C) Mark Phillips 2012

    UPNPAudioPlayer is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    UPNPAudioPlayer is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with UPNPAudioPlayer. If not, see <http://www.gnu.org/licenses/>.
*/

#include <libgupnp/gupnp.h>
#include <libgssdp/gssdp.h>
#include <stdlib.h>
#include <gmodule.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include "connection_manager.h"
#include "av_transport.h"
#include "rendering_control.h"
#include "stream_player.h"
#include "logger.h"

static GMainLoop *_main_loop;

struct audio_player_options {
	int advertise_period;
	int daemon_mode;
	int verbose_logging;
	char* path;
	char* interface;
};

static void print_usage_and_quit(int exit_status)
{
	printf("UPNPAudioPlayer :\n");
	printf("  -d                      : daemon mode\n");
	printf("  -i <interface name>     : start on the given interface\n");
	/* printf("  -a <N bigger than zero> : set the UPNP advertise period in seconds (unsupported)\n"); */
	printf("  -p <path name>          : specify the path to the config xml\n");
	printf("  -v                      : verbose logging\n");
	printf("  -h                      : print this message and quit\n");
	exit(exit_status);
}

static void process_command_line(int argc, char** argv, struct audio_player_options *options)
{
	int opt;
	
	if (!options) return;
	
	/* set default options */
	options->advertise_period = -1;
	options->daemon_mode = 0;
	options->path = 0;
	options->interface = 0;
	options->verbose_logging = 0;
	
	while ((opt = getopt(argc, argv, "i:dp:vh")) != -1)  /* removed a: */
	{
		switch (opt)
		{
		case 'i':
			options->interface = optarg;
			break;
		case 'd':
			options->daemon_mode = 1;
			break;
		case 'a':
			if ( atoi(optarg) > 0 )
				options->advertise_period = atoi(optarg);
			else
				print_usage_and_quit(EXIT_FAILURE);
			break;
		case 'p':
			options->path = optarg;
			break;
		case 'v':
			options->verbose_logging = 1;
			break;
		case 'h':
			print_usage_and_quit(EXIT_SUCCESS);
			break;
		case '?':
			print_usage_and_quit(EXIT_FAILURE);
			break;
		default:
			print_usage_and_quit(EXIT_FAILURE);
		}
		
	}
}

static void term(int signum)
{
	g_main_loop_quit( _main_loop );
}

static gboolean monitor_callback( gpointer user_data )
{	
	avt_notify_position( sp_get_position_ns() );
	avt_notify_duration( sp_get_duration_ns() );

	return TRUE;
}
	

int main (int argc, char **argv)
{
	struct audio_player_options options;
	GUPnPContext *context;
	GUPnPRootDevice *dev;
	GUPnPServiceInfo *avtransport_service;
	GUPnPServiceInfo *connection_manager_service;
	GUPnPServiceInfo *rendering_control_service;
	GSSDPResourceGroup *resource_group;
	char *data_path;
	GError* error=0;
	int context_retries = 0;
	
	process_command_line(argc, argv, &options);
	
	/* Initialise Logging */
	if (options.verbose_logging)
		setLogLevel(Detail);
	else
		setLogLevel(Default);
		
	setLogLocation(LocationDefault);
	
	if ( options.daemon_mode )
	{
		if ( daemon(0, 0) != 0 )
		{
			printf("AudioPlayer - could not start daemon\n");
			exit(EXIT_FAILURE);
		}
		setLogLocation(LocationSyslog);
	}
	
	LOG("AudioPlayer - startup\n");
	DETAIL("Verbose Logging\n");
	
	/* Initialize required subsystems */
	#if !GLIB_CHECK_VERSION(2,35,0)
  	g_type_init ();
	#endif
		
	/* Create the GUPnP context with default host and port */
	if (options.interface) LOG("using interface %s\n", options.interface);
	if ( ! options.daemon_mode )
	{
		if ( !(context = gupnp_context_new(NULL, options.interface, 0, &error)))
		{
			WARN("Can't create context. Error was : %s\n", error->message);
			exit(EXIT_FAILURE);
		}
	}
	else /* deamon mode */
	{
		while ( !(context = gupnp_context_new(NULL, options.interface, 0, &error)) )
		{
			context_retries++;
			WARN("Can't create context at attempt %d. Retrying in %d minute(s). Error was : %s\n",
				context_retries, context_retries<10?1:10, error->message);
			error=0;
			sleep(context_retries<10?60:600);
		}
		if ( context_retries > 0 ) LOG("Created Context\n");
	}
	
	gupnp_context_host_path(context, "MediaRenderer.xml", "/data/MediaRenderer.xml");
	gupnp_context_host_path(context, "AVTransport.xml", "/data/AVTransport.xml");
	gupnp_context_host_path(context, "ConnectionManager.xml", "/data/ConnectionManager.xml");
	gupnp_context_host_path(context, "RenderingControl.xml", "/data/RenderingControl.xml");
	
	
	/* Create the root device object */
	if ( options.path )
	{
		LOG("Using data path %s\n", options.path);
		dev = gupnp_root_device_new(context, "MediaRenderer.xml", options.path);
	}
	else
	{
		LOG("Using default data path (../data)\n");
		dev = gupnp_root_device_new(context, "MediaRenderer.xml", "../data");
	}
	
	/* set the max age for UPNP advertisements if it was specified (otherwise we're using the GUPnP default) */
	if (options.advertise_period > 0)
	{
		/* - can't find a way to do this on the GUPnP version in Raspbian so removing for now
		LOG("using max age = %d\n", options.advertise_period);
		resource_group = gupnp_root_device_get_ssdp_resource_group(dev);
		gssdp_resource_group_set_max_age(resource_group, options.advertise_period);
		*/
	}
	
	/* Activate the root device, so that it announces itself */
	gupnp_root_device_set_available(dev, TRUE);
	
	/* get hold of the services */
	avtransport_service = gupnp_device_info_get_service(
									GUPNP_DEVICE_INFO (dev),
									"urn:schemas-upnp-org:service:AVTransport:1");
	connection_manager_service = gupnp_device_info_get_service(
									GUPNP_DEVICE_INFO (dev),
									"urn:schemas-upnp-org:service:ConnectionManager:1");
	rendering_control_service = gupnp_device_info_get_service(
									GUPNP_DEVICE_INFO (dev),
									"urn:schemas-upnp-org:service:RenderingControl:1");
	
	/* initialise services */
	av_transport_init( GUPNP_SERVICE(avtransport_service) );
	connection_manager_init( GUPNP_SERVICE(connection_manager_service) );
	rendering_control_init( GUPNP_SERVICE(rendering_control_service) ); 
	
	/* monitoring function */
	g_timeout_add_seconds(1, monitor_callback, NULL);
	
	/* set up the player */
	stream_player_init();
	
	/* catch signals so we can shutdown cleanly */
	signal(SIGTERM, term);
	signal(SIGINT, term);
	
	/* main loop */
  	_main_loop = g_main_loop_new(NULL, FALSE);
  	g_main_loop_run(_main_loop);

  	/* Cleanup */
  	LOG("AudioPlayer - shutdown\n");
  	stream_player_shutdown();
  	
  	g_main_loop_unref(_main_loop);
  	g_object_unref(avtransport_service);
  	g_object_unref(connection_manager_service);
  	g_object_unref(rendering_control_service);
  	g_object_unref(dev);
  	g_object_unref(context);
  	
  	logClose();
  	
  	return EXIT_SUCCESS;
}

