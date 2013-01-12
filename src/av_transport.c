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

#include "av_transport.h"
#include "AVTransport_wrapper.c"
#include "stream_player.h"

#include <stdio.h>
#include <string.h>


/* state variables */
#define MAX_URI 4096
#define MAX_TIME 32
static char TransportState[32] = "STOPPED";
static char TransportStatus[32] = "OK";
static char PlaybackStorageMedium[] = "NETWORK";
static char RecordStorageMedium[] = "NOT_IMPLEMENTED";
static char PossiblePlaybackStorageMedia[] = "NETWORK";
static char PossibleRecordStorageMedia[] = "NOT_IMPLEMENTED";
static char CurrentPlayMode[] = "NORMAL";
static char TransportPlaySpeed[] = "1";
static char RecordMediumWriteStatus[] = "NOT_IMPLEMENTED";
static char CurrentRecordQualityMode[] = "NOT_IMPLEMENTED";
static char PossibleRecordQualityModes[] = "NOT_IMPLEMENTED";
static unsigned int NumberOfTracks = 0;
static unsigned int CurrentTrack = 0;
static char CurrentTrackDuration[MAX_TIME] = "00:00:00.00";
static char CurrentMediaDuration[MAX_TIME] = "00:00:00.00";
static char CurrentTrackMetaData[] = "";
static char CurrentTrackURI[MAX_URI] = "";
static char AVTransportURI[MAX_URI] = "";
static char AVTransportURIMetaData[MAX_URI] = "";
static char NextAVTransportURI[MAX_URI] = "";
static char NextAVTransportURIMetaData[] = "NOT_IMPLEMENTED";
static char RelativeTimePosition[MAX_TIME] = "00:00:00.00";
static char AbsoluteTimePosition[MAX_TIME] = "00:00:00.00";
static int RelativeCounterPosition = 0;
static int AbsoluteCounterPosition = 0;
#define LASTCHANGE_LENGTH 4096
static char LastChange[LASTCHANGE_LENGTH] = "<Event xmlns = \"urn:schemas-upnp-org:metadata-1-0/AVT/\"><InstanceID val=\"0\"></InstanceID></Event>";


static GUPnPService *_service = NULL;

static void avt_last_change_notify_string(GUPnPService *service, const char* variable, const char* s);
static void avt_last_change_notify_int(GUPnPService *service, const char* variable, int i);


/* helpers */
static void ns_to_upnp_time( gint64 ns, char* upnp_time)
{
	double position_seconds = (double)ns / (double)1e9;
	
	unsigned int hours;
	unsigned int minutes;
	unsigned int seconds;
	unsigned int seconds100;
	
	hours =  (unsigned int)( position_seconds/3600 );
	position_seconds -= hours*3600;
	minutes = (unsigned int)( position_seconds/60 );
	position_seconds -= minutes*60;
	seconds = (unsigned int)( position_seconds );
	position_seconds -= seconds;
	seconds100 = (unsigned int) (position_seconds * 100 );
	
	snprintf( upnp_time, MAX_TIME, "%02d:%02d:%02d.%02d", hours, minutes, seconds, seconds100 );
}


static gint64 upnp_time_to_ns( const char* upnp_time )
{
	int hours, minutes, seconds, decimal;
	
	sscanf( upnp_time, "%d:%d:%d.%d", &hours, &minutes, &seconds, &decimal );
		
	return 1e9 * (hours*3600 + minutes*60 + seconds);   /* eeh - only used for seek, who cares about the decimal */
}

enum transport_state {
	Stopped,
	Playing,
	Paused
	};

static void set_transport_state( enum transport_state state)
{
	switch (state) {
	case Stopped:
		strcpy(TransportState, "STOPPED");
		break;
	case Playing:
		strcpy(TransportState, "PLAYING");
		break;
	case Paused:
		strcpy(TransportState, "PAUSED_PLAYBACK");
		break;
	default:
		printf("set_transport_state - unknown state\n");
	}
}

/* public functions */
void avt_notify_position( gint64 position_ns )
{
	char new_position[MAX_TIME];
	ns_to_upnp_time( position_ns, new_position );
	
	if ( _service && strcmp(new_position, RelativeTimePosition) != 0 )
	{
		strcpy(RelativeTimePosition, new_position);
		avt_last_change_notify_string( _service, "RelativeTimePosition", RelativeTimePosition );
	}
}

void avt_notify_duration( gint64 duration_ns )
{	
	char new_duration[MAX_TIME];
	ns_to_upnp_time( duration_ns, new_duration );
	
	if ( _service && strcmp(new_duration, CurrentTrackDuration) != 0 )
	{
		strcpy(CurrentTrackDuration, new_duration);
		avt_last_change_notify_string( _service, "CurrentTrackDuration", CurrentTrackDuration );
	}
}

void avt_notify_error(const char* error)
{
	set_transport_state(Stopped);
	avt_last_change_notify_string(_service, "TransportState", TransportState);
}

void avt_notify_endofstream()
{
	/* for my synology control point setting the stopped state causes it to send the next track.
	   tch - where's that in the UPNP spec ?! */ 
	set_transport_state(Stopped);
	avt_last_change_notify_string(_service, "TransportState", TransportState);
}


/* state variables */
static gchar *TransportState_callback(GUPnPService *service, gpointer userdata)
{
	return TransportState;
}

static gchar *TransportStatus_callback(GUPnPService *service, gpointer userdata)
{
	return TransportStatus;
}

static gchar *PlaybackStorageMedium_callback(GUPnPService *service, gpointer userdata)
{
	return PlaybackStorageMedium;
}

static gchar *RecordStorageMedium_callback(GUPnPService *service, gpointer userdata)
{
	return RecordStorageMedium;
}

static gchar *PossiblePlaybackStorageMedia_callback(GUPnPService *service, gpointer userdata)
{
	return PossiblePlaybackStorageMedia;
}

static gchar *PossibleRecordStorageMedia_callback(GUPnPService *service, gpointer userdata)
{
	return PossibleRecordStorageMedia;
}

static gchar *CurrentPlayMode_callback(GUPnPService *service, gpointer userdata)
{
	return CurrentPlayMode;
}

static gchar *TransportPlaySpeed_callback(GUPnPService *service, gpointer userdata)
{
	return TransportPlaySpeed;
}

static gchar *RecordMediumWriteStatus_callback(GUPnPService *service, gpointer userdata)
{
	return RecordMediumWriteStatus;
}

static gchar *CurrentRecordQualityMode_callback(GUPnPService *service, gpointer userdata)
{
	return CurrentRecordQualityMode;
}

static gchar *PossibleRecordQualityModes_callback(GUPnPService *service, gpointer userdata)
{
	return PossibleRecordQualityModes;
}

static guint NumberOfTracks_callback(GUPnPService *service, gpointer userdata)
{
	return NumberOfTracks;
}

static guint CurrentTrack_callback(GUPnPService *service, gpointer userdata)
{
	return CurrentTrack;
}

static gchar *CurrentTrackDuration_callback(GUPnPService *service, gpointer userdata)
{
	return CurrentTrackDuration;
}

static gchar *CurrentMediaDuration_callback(GUPnPService *service, gpointer userdata)
{
	return CurrentMediaDuration;
}

static gchar *CurrentTrackMetaData_callback(GUPnPService *service, gpointer userdata)
{
	return CurrentTrackMetaData;
}

static gchar *CurrentTrackURI_callback(GUPnPService *service, gpointer userdata)
{
	return CurrentTrackURI;
}

static gchar *AVTransportURI_callback(GUPnPService *service, gpointer userdata)
{
	return AVTransportURI;
}

static gchar *AVTransportURIMetaData_callback(GUPnPService *service, gpointer userdata)
{
	return AVTransportURIMetaData;
}

static gchar *NextAVTransportURI_callback(GUPnPService *service, gpointer userdata)
{
	return NextAVTransportURI;
}

static gchar *NextAVTransportURIMetaData_callback(GUPnPService *service, gpointer userdata)
{
	return NextAVTransportURIMetaData;
}

static gchar *RelativeTimePosition_callback(GUPnPService *service, gpointer userdata)
{
	return RelativeTimePosition;
}

static gchar *AbsoluteTimePosition_callback(GUPnPService *service, gpointer userdata)
{
	return AbsoluteTimePosition;
}

static gint RelativeCounterPosition_callback(GUPnPService *service, gpointer userdata)
{
	return RelativeCounterPosition;
}

static gint AbsoluteCounterPosition_callback(GUPnPService *service, gpointer userdata)
{
	return AbsoluteCounterPosition;
}

static gchar *LastChange_callback(GUPnPService *service, gpointer userdata)
{
	return LastChange;
}


/* last change eventing */
static void avt_last_change_notify_int(GUPnPService *service, const char* variable, int i)
{
	snprintf(LastChange,
		 LASTCHANGE_LENGTH,
		 "<Event xmlns = \"urn:schemas-upnp-org:metadata-1-0/AVT/\"><InstanceID val=\"0\"><%s val=\"%d\"/></InstanceID></Event>",
		 variable,
		 i);
		 
	rc_last_change_variable_notify(service, LastChange);
	
	// printf("Event Last Change (int) %s\n", LastChange);
}


static void avt_last_change_notify_string(GUPnPService *service, const char* variable, const char* s)
{
	snprintf(LastChange,
		 LASTCHANGE_LENGTH,
		 "<Event xmlns = \"urn:schemas-upnp-org:metadata-1-0/AVT/\"><InstanceID val=\"0\"><%s val=\"%s\"/></InstanceID></Event>",
		 variable,
		 s);
		 
	rc_last_change_variable_notify(service, LastChange);
	
	// printf("Event Last Change (string) %s\n", LastChange);
}

/* actions */

static void avt_set_av_transport_uri(GUPnPService *service, GUPnPServiceAction *action, gpointer user_data)
{
	guint instance_id;
	gchar* current_uri;
	gchar* current_uri_meta_data;
	unsigned long duration_ns;

	avt_set_av_transport_uri_action_get( action, &instance_id, &current_uri, &current_uri_meta_data );
	
	strncpy(AVTransportURI, current_uri, MAX_URI);
	strncpy(AVTransportURIMetaData, current_uri_meta_data, MAX_URI);
	avt_last_change_notify_string(service, "AVTransportURI", AVTransportURI);
	avt_last_change_notify_string(service, "AVTransportURIMetaData", AVTransportURIMetaData);
	
	sp_set_uri(AVTransportURI);
	
	gupnp_service_action_return(action);
	
	printf("Set AVTransportURI %s\n", AVTransportURI);
	printf("Set AVTransportURI Meta Data%s\n", AVTransportURIMetaData);
	
}

static void avt_set_next_av_transport_uri(GUPnPService *service, GUPnPServiceAction *action, gpointer user_data)
{
	guint instance_id;
	gchar* next_uri;
	gchar* next_uri_meta_data;
	
	avt_set_next_av_transport_uri_action_get( action, &instance_id, &next_uri, &next_uri_meta_data );
	
	strncpy(NextAVTransportURI, next_uri, MAX_URI);
	avt_last_change_notify_string(service, "NextAVTransportURI", NextAVTransportURI);
	
	/* TODO : implement call to stream player */
	
	gupnp_service_action_return(action);
	
	// printf("Set NextAVTransportURI %s\n", NextAVTransportURI);
}

static void avt_get_media_info(GUPnPService *service, GUPnPServiceAction *action, gpointer user_data)
{
	guint instance_id;
	avt_get_media_info_action_get( action, &instance_id );

	avt_get_media_info_action_set (action,
				       NumberOfTracks,
				       CurrentMediaDuration,
				       AVTransportURI,
				       AVTransportURIMetaData,
				       NextAVTransportURI,
				       NextAVTransportURIMetaData,
				       PlaybackStorageMedium,
				       RecordStorageMedium,
				       RecordMediumWriteStatus);
                               
	gupnp_service_action_return(action);
}

static void avt_get_transport_info(GUPnPService *service, GUPnPServiceAction *action, gpointer user_data)
{
	guint instance_id;
	avt_get_transport_info_action_get( action, &instance_id );

	avt_get_transport_info_action_set ( action,
					    TransportState,
					    TransportStatus,
					    TransportPlaySpeed);
					    
	gupnp_service_action_return(action);
}

static void avt_get_position_info(GUPnPService *service, GUPnPServiceAction *action, gpointer user_data)
{
	guint instance_id;
	avt_get_position_info_action_get( action, &instance_id );

	avt_get_position_info_action_set ( action,
					   CurrentTrack,
					   CurrentTrackDuration,
					   CurrentTrackMetaData,
					   CurrentTrackURI,
					   RelativeTimePosition,
					   AbsoluteTimePosition,
					   RelativeCounterPosition,
					   AbsoluteCounterPosition );
	
	gupnp_service_action_return(action);
}

static void avt_get_device_capabilities(GUPnPService *service, GUPnPServiceAction *action, gpointer user_data)
{
	guint instance_id;
	avt_get_device_capabilities_action_get( action, &instance_id );

	avt_get_device_capabilities_action_set(action, PossiblePlaybackStorageMedia, PossibleRecordStorageMedia, PossibleRecordQualityModes);
	gupnp_service_action_return(action);
}

static void avt_get_transport_settings(GUPnPService *service, GUPnPServiceAction *action, gpointer user_data)
{
	guint instance_id;
	avt_get_transport_settings_action_get( action, &instance_id );	

	avt_get_transport_settings_action_set ( action,
						CurrentPlayMode,
						CurrentRecordQualityMode );
	
	gupnp_service_action_return(action);
}

static void avt_play(GUPnPService *service, GUPnPServiceAction *action, gpointer user_data)
{
	set_transport_state(Playing);
	avt_last_change_notify_string(service, "TransportState", TransportState);
	
	sp_play();
	
	gupnp_service_action_return(action);
	
	// printf("Play\n");
}

static void avt_pause(GUPnPService *service, GUPnPServiceAction *action, gpointer user_data)
{
	guint instance_id;
	avt_pause_action_get( action, &instance_id );

	set_transport_state(Paused);
	avt_last_change_notify_string(service, "TransportState", TransportState);
	
	sp_pause();
	
	gupnp_service_action_return(action);
	
	// printf("Pause\n");
}

static void avt_stop(GUPnPService *service, GUPnPServiceAction *action, gpointer user_data)
{
	guint instance_id;
	avt_stop_action_get( action, &instance_id );

	set_transport_state(Stopped);
	avt_last_change_notify_string(service, "TransportState", TransportState);
	
	sp_pause();
	sp_seek_ns(0);
	
	gupnp_service_action_return (action);
	
	// printf("Stop Instance ID %d\n", instance_id);
}

static void avt_seek(GUPnPService *service, GUPnPServiceAction *action, gpointer user_data)
{
	guint instance_id;
	gchar* unit;
	gchar* target;

	avt_seek_action_get (action, &instance_id, &unit, &target);

	if ( strcmp(unit, "REL_TIME") == 0 )
	{
		gint64 seek_pos_ns;
		
		seek_pos_ns = upnp_time_to_ns( target );
		sp_seek_ns( seek_pos_ns );
		
		strcpy(RelativeTimePosition, target);
		avt_last_change_notify_string(service, "RelativeTimePosition", RelativeTimePosition);
		
		// printf("Seek Unit : %s, Target %s\n", unit, target);
	}
	else
	{
		printf("Seek Unit : %s, Target %s - NOT SUPPORTED\n", unit, target);
	}
	
	gupnp_service_action_return(action);
	
	
}

static void avt_next(GUPnPService *service, GUPnPServiceAction *action, gpointer user_data)
{
	guint instance_id;
	avt_next_action_get (action, &instance_id);

	/* TODO - implement me */
	
	gupnp_service_action_return(action);
	
	// printf("Next\n");
}

static void avt_previous(GUPnPService *service, GUPnPServiceAction *action, gpointer user_data)
{
	guint instance_id;
	avt_previous_action_get (action, &instance_id);

	/* TODO - implement me */
	
	gupnp_service_action_return(action);
	
	// printf("Previous\n");
}

void av_transport_init(GUPnPService *service)
{
	printf("AV Transport - init\n");
	
	_service = service;
	
	/* query state variable callbacks */
	avt_transport_state_query_connect(service, TransportState_callback, NULL);
	avt_transport_status_query_connect(service, TransportStatus_callback, NULL);
	avt_playback_storage_medium_query_connect(service, PlaybackStorageMedium_callback, NULL);
	avt_record_storage_medium_query_connect(service, RecordStorageMedium_callback, NULL);
	avt_possible_playback_storage_media_query_connect(service, PossiblePlaybackStorageMedia_callback, NULL);
	avt_possible_record_storage_media_query_connect(service, PossibleRecordStorageMedia_callback, NULL);
	avt_current_play_mode_query_connect(service, CurrentPlayMode_callback, NULL);
	avt_transport_play_speed_query_connect(service, TransportPlaySpeed_callback, NULL);
	avt_record_medium_write_status_query_connect(service, RecordMediumWriteStatus_callback, NULL);
	avt_current_record_quality_mode_query_connect(service, CurrentRecordQualityMode_callback, NULL);
	avt_possible_record_quality_modes_query_connect(service, PossibleRecordQualityModes_callback, NULL);
	avt_number_of_tracks_query_connect(service, NumberOfTracks_callback, NULL);
	avt_current_track_query_connect(service, CurrentTrack_callback, NULL);
	avt_current_track_duration_query_connect(service, CurrentTrackDuration_callback, NULL);
	avt_current_media_duration_query_connect(service, CurrentMediaDuration_callback, NULL);
	avt_current_track_meta_data_query_connect(service, CurrentTrackMetaData_callback, NULL);
	avt_current_track_uri_query_connect(service, CurrentTrackURI_callback, NULL);
	avt_av_transport_uri_query_connect(service, AVTransportURI_callback, NULL);
	avt_av_transport_uri_meta_data_query_connect(service, AVTransportURIMetaData_callback, NULL);
	avt_next_av_transport_uri_query_connect(service, NextAVTransportURI_callback, NULL);
	avt_next_av_transport_uri_meta_data_query_connect(service, NextAVTransportURIMetaData_callback, NULL);
	avt_relative_time_position_query_connect(service, RelativeTimePosition_callback, NULL);
	avt_absolute_time_position_query_connect(service, AbsoluteTimePosition_callback, NULL);
	avt_relative_counter_position_query_connect(service, RelativeCounterPosition_callback, NULL);
	avt_absolute_counter_position_query_connect(service, AbsoluteCounterPosition_callback, NULL);
	avt_last_change_query_connect(service, LastChange_callback, NULL);
	
	/* action callbacks */
	avt_set_av_transport_uri_action_connect(service, G_CALLBACK(avt_set_av_transport_uri), NULL);
	avt_set_next_av_transport_uri_action_connect(service, G_CALLBACK(avt_set_next_av_transport_uri), NULL);
	avt_get_media_info_action_connect(service, G_CALLBACK(avt_get_media_info), NULL);
	avt_get_transport_info_action_connect(service, G_CALLBACK(avt_get_transport_info), NULL);
	avt_get_position_info_action_connect(service, G_CALLBACK(avt_get_position_info), NULL);
	avt_get_device_capabilities_action_connect(service, G_CALLBACK(avt_get_device_capabilities), NULL);
	avt_get_transport_settings_action_connect(service, G_CALLBACK(avt_get_transport_settings), NULL);
	avt_play_action_connect(service, G_CALLBACK(avt_play), NULL);
	avt_pause_action_connect(service, G_CALLBACK(avt_pause), NULL);
	avt_stop_action_connect(service, G_CALLBACK(avt_stop), NULL);
	avt_seek_action_connect(service, G_CALLBACK(avt_seek), NULL);
	avt_next_action_connect(service, G_CALLBACK(avt_next), NULL);
	avt_previous_action_connect(service, G_CALLBACK(avt_previous), NULL);
}

