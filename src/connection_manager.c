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

#include "connection_manager.h"
#include "ConnectionManager_wrapper.c"

#include <stdio.h>


#define STR_LEN 1024

/* state variables */
static char SourceProtocolInfo[] = "";
static char SinkProtocolInfo[] = "http-get:*:audio/mpeg:*,http-get:*:audio/x-ac3:*,http-get:*:audio/x-m4a:*,http-get:*:audio/m4a:*";
static char CurrentConnectionIDs[] = "0";


static gchar *SourceProtocolInfo_callback(GUPnPService *service, gpointer userdata)
{
	return SourceProtocolInfo;
}

static gchar *SinkProtocolInfo_callback(GUPnPService *service, gpointer userdata)
{
	return SinkProtocolInfo;
}

static gchar *CurrentConnectionIDs_callback(GUPnPService *service, gpointer userdata)
{
	return CurrentConnectionIDs;
}



/* actions */

static void cm_get_protocol_info(GUPnPService *service, GUPnPServiceAction *action, gpointer user_data)
{
	cm_get_protocol_info_action_set(action,SourceProtocolInfo, SinkProtocolInfo);
        gupnp_service_action_return(action);
}

static void cm_get_current_connection_info(GUPnPService *service, GUPnPServiceAction *action, gpointer user_data)
{
	cm_get_current_connection_info_action_set(action, 0, 0, "", "", -1, "Input", "OK");
	gupnp_service_action_return (action);
}

static void cm_get_current_connection_i_ds(GUPnPService *service, GUPnPServiceAction *action, gpointer user_data)
{
	cm_get_current_connection_i_ds_action_set(action, CurrentConnectionIDs);
	gupnp_service_action_return (action);
}


void connection_manager_init(GUPnPService *service)
{
	printf("Connection Manager - init\n");
	
	cm_source_protocol_info_query_connect(service, SourceProtocolInfo_callback, NULL);
	cm_sink_protocol_info_query_connect(service, SinkProtocolInfo_callback, NULL);
	cm_current_connection_i_ds_query_connect(service, CurrentConnectionIDs_callback, NULL);
	
	cm_get_protocol_info_action_connect(service, G_CALLBACK(cm_get_protocol_info), NULL);
	cm_get_current_connection_info_action_connect(service, G_CALLBACK(cm_get_current_connection_info), NULL);
	cm_get_current_connection_i_ds_action_connect(service, G_CALLBACK(cm_get_current_connection_i_ds), NULL);
}

