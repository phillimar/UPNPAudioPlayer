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


#include "rendering_control.h"
#include "RenderingControl_wrapper.c"
#include "stream_player.h"

#include <stdio.h>

/* state variables */
#define LASTCHANGE_LENGTH 4096
static char LastChange[LASTCHANGE_LENGTH] = "<Event xmlns = \"urn:schemas-upnp-org:metadata-1-0/AVT/\"><InstanceID val=\"0\"><Volume val=\"100\"/></InstanceID></Event>";
static char PresetNameList[] = "";
#define MAX_VOLUME 100
static unsigned short Volume = 100;
#define MAX_VOLUMEDB 100
static short VolumeDB = 0;


static gchar *LastChange_callback(GUPnPService *service, gpointer userdata)
{
	return LastChange;
}

static gchar *PresetNameList_callback(GUPnPService *service, gpointer userdata)
{
	return PresetNameList;
}

static guint Volume_callback(GUPnPService *service, gpointer userdata)
{
	return Volume;
}

static gint VolumeDB_callback(GUPnPService *service, gpointer userdata)
{
	return VolumeDB;
}

/* last change eventing */
static void rc_last_change_notify_int(GUPnPService *service, const char* variable, int i)
{
	snprintf(LastChange,
		 LASTCHANGE_LENGTH,
		 "<Event xmlns = \"urn:schemas-upnp-org:metadata-1-0/AVT/\"><InstanceID val=\"0\"><%s val=\"%d\"/></InstanceID></Event>",
		 variable,
		 i);
		 
	rc_last_change_variable_notify(service, LastChange);
}

/* actions */

static void rc_list_presets(GUPnPService *service, GUPnPServiceAction *action, gpointer user_data)
{
	rc_list_presets_action_set(action, PresetNameList);
        gupnp_service_action_return(action);
}

static void rc_select_preset(GUPnPService *service, GUPnPServiceAction *action, gpointer user_data)
{
	guint instance_id;
	gchar* preset_name;
	
	rc_select_preset_action_get(action, &instance_id, &preset_name);

	/* nothing to do */
	
	gupnp_service_action_return(action);
}

static void rc_get_volume(GUPnPService *service, GUPnPServiceAction *action, gpointer user_data)
{
	rc_get_volume_action_set(action, Volume);
        gupnp_service_action_return(action);
}

static void rc_set_volume(GUPnPService *service, GUPnPServiceAction *action, gpointer user_data)
{
	guint instance_id;
	gchar* channel;
	guint desired_volume;
	
	rc_set_volume_action_get(action, &instance_id, &channel, &desired_volume);
        
        desired_volume = (desired_volume<0) ? 0 : desired_volume;
        desired_volume = (desired_volume>MAX_VOLUME) ? MAX_VOLUME : desired_volume;

	Volume = desired_volume;
	rc_last_change_notify_int(service, "Volume", Volume);
	
	sp_set_volume( (float)desired_volume / 100.0f );
	
	gupnp_service_action_return(action);
}

/* 
static void rc_get_volume_db(GUPnPService *service, GUPnPServiceAction *action, gpointer user_data)
{
	rc_get_volume_db_action_set(action, VolumeDB);
        gupnp_service_action_return(action);
}

static void rc_set_volume_db(GUPnPService *service, GUPnPServiceAction *action, gpointer user_data)
{
	guint instance_id;
	gchar* channel;
	guint desired_volume;
	
	rc_set_volume_action_get(action, &instance_id, &channel, &desired_volume);
        
        desired_volume = (desired_volume<0) ? 0 : desired_volume;
        desired_volume = (desired_volume>MAX_VOLUMEDB) ? MAX_VOLUME : desired_volume;

	VolumeDB = desired_volume;
	
	 TODO : update the last change variable 
}
*/

void rendering_control_init(GUPnPService *service)
{
	printf("Rendering Control - init\n");
	
	rc_last_change_query_connect(service, LastChange_callback, NULL);
	rc_preset_name_list_query_connect(service, PresetNameList_callback, NULL);
	rc_volume_query_connect(service, Volume_callback, NULL);
	rc_volume_db_query_connect(service, VolumeDB_callback, NULL);
	
	rc_list_presets_action_connect(service, G_CALLBACK(rc_list_presets), NULL);
	rc_select_preset_action_connect(service, G_CALLBACK(rc_select_preset), NULL);
	rc_get_volume_action_connect(service, G_CALLBACK(rc_get_volume), NULL);
	rc_set_volume_action_connect(service, G_CALLBACK(rc_set_volume), NULL);
	/* rc_get_volume_db_action_connect(service, G_CALLBACK(rc_get_volume_db), NULL);
	rc_set_volume_db_action_connect(service, G_CALLBACK(rc_set_volume_db), NULL); */
	
	
}

