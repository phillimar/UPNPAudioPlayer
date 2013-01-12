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
#include <stdlib.h>
#include <gmodule.h>
#include <stdio.h>
#include <signal.h>

#include "connection_manager.h"
#include "av_transport.h"
#include "rendering_control.h"
#include "stream_player.h"

#define AUDIOPLAYER_DATA "AUDIOPLAYER_DATA"

static GMainLoop *_main_loop;

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
	GUPnPContext *context;
	GUPnPRootDevice *dev;
	GUPnPServiceInfo *avtransport_service;
	GUPnPServiceInfo *connection_manager_service;
	GUPnPServiceInfo *rendering_control_service;
	char *data_path;
	
	printf("AudioPlayer - startup\n");
	
	/* Initialize required subsystems */
	g_type_init();
	
	/* Create the GUPnP context with default host and port */
	context = gupnp_context_new(NULL, NULL, 0, NULL);
	
	gupnp_context_host_path(context, "MediaRenderer.xml", "/data/MediaRenderer.xml");
	gupnp_context_host_path(context, "AVTransport.xml", "/data/AVTransport.xml");
	gupnp_context_host_path(context, "ConnectionManager.xml", "/data/ConnectionManager.xml");
	gupnp_context_host_path(context, "RenderingControl.xml", "/data/RenderingControl.xml");
	
	
	/* Create the root device object - get path to the data from environment or use default if that's note set */
	data_path = getenv( AUDIOPLAYER_DATA );
	if ( data_path )
	{
		printf("Using data path %s\n", data_path);
		dev = gupnp_root_device_new(context, "MediaRenderer.xml", data_path);
	}
	else
	{
		printf("Using default data path (../data)\n");
		dev = gupnp_root_device_new(context, "MediaRenderer.xml", "../data");
	}
	/* Activate the root device, so that it announces itself */
	gupnp_root_device_set_available(dev, TRUE);
	
	/* get hold of the services */
	avtransport_service = gupnp_device_info_get_service(GUPNP_DEVICE_INFO (dev), "urn:schemas-upnp-org:service:AVTransport:1");
	connection_manager_service = gupnp_device_info_get_service(GUPNP_DEVICE_INFO (dev), "urn:schemas-upnp-org:service:ConnectionManager:1");
	rendering_control_service = gupnp_device_info_get_service(GUPNP_DEVICE_INFO (dev), "urn:schemas-upnp-org:service:RenderingControl:1");
	
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
  	printf("Clean up...\n");
  	stream_player_shutdown();
  	
  	g_main_loop_unref(_main_loop);
  	g_object_unref(avtransport_service);
  	g_object_unref(connection_manager_service);
  	g_object_unref(rendering_control_service);
  	g_object_unref(dev);
  	g_object_unref(context);
}

