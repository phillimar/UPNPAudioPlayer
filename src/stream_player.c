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

#include "stream_player.h"
#include "av_transport.h"

#include <gst/gst.h>

#include <stdio.h>


static GstElement *_pipeline = NULL;
static short _seek_inprogress = FALSE;
static gint64 _seek_target;


static gboolean bus_call(GstBus *bus, GstMessage *msg, void *user_data)
{
	switch (GST_MESSAGE_TYPE(msg)) {
	case GST_MESSAGE_EOS: {
		printf("stream player : end of stream\n");
		avt_notify_endofstream();
		gst_element_set_state(GST_ELEMENT(_pipeline), GST_STATE_READY);
		break;
	}
	case GST_MESSAGE_ERROR: {
		GError *err;
		gst_message_parse_error(msg, &err, NULL);
		printf("GStreamer Error : %s\n", err->message);
		avt_notify_error( err->message );
		g_error_free(err);
		break;
		}
	case GST_MESSAGE_BUFFERING: {
		gint percent = 0;
		gst_message_parse_buffering(msg, &percent);
		// printf("Buffering (%u %)\n", percent);
		// if buffering < 100% we want to pause
		// otherwise can play the stream
		break;
		}
	case GST_MESSAGE_ASYNC_DONE: {
		_seek_inprogress= FALSE;
		break;
		}
	default:
		break;
	}
 
	return TRUE;
}

void sp_play()
{
	if (_pipeline) gst_element_set_state(GST_ELEMENT(_pipeline), GST_STATE_PLAYING);
}

void sp_pause()
{
	if (_pipeline) gst_element_set_state(GST_ELEMENT(_pipeline), GST_STATE_PAUSED);
}

void sp_seek_ns(gint64 position_ns)
{
	if (_pipeline)
	{
		gst_element_seek_simple(_pipeline, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT, position_ns);
		_seek_inprogress = TRUE;
		_seek_target = position_ns;
	}
}

void sp_set_volume(float volume) /* 0=mute, 1=max */
{
	if (_pipeline)
		g_object_set(_pipeline, "volume", (double)(volume*volume), NULL);
}


gint64 sp_get_position_ns()
{
	gint64 position = 0;
	
	if (_pipeline)
	{
		/* if we're in the middle of a seek, return the seek target
		   otherwise the values jump around */
		if ( _seek_inprogress )
		{
			position = _seek_target;
		}
		else
		{
			GstFormat format = GST_FORMAT_TIME;
			gst_element_query_position(_pipeline, &format, &position);
		}	
	}
	
	return position;		
}

gint64 sp_get_duration_ns()
{
	gint64 duration = 0;
	
	if (_pipeline)
	{
		GstFormat format = GST_FORMAT_TIME;
		gst_element_query_duration(_pipeline, &format, &duration);
	}
	
	return duration;
}


void sp_set_uri(const char *uri)
{
	if (uri)
	{
		gst_element_set_state(GST_ELEMENT(_pipeline), GST_STATE_READY);
		g_object_set(G_OBJECT(_pipeline), "uri", uri, NULL);
		gst_element_set_state(GST_ELEMENT(_pipeline), GST_STATE_PAUSED);
	}		
}


void stream_player_init()
{
	GstBus *bus;
	
	printf("Stream Player - init\n");

	gst_init(NULL, NULL);
	  
	_pipeline = gst_element_factory_make("playbin2", "player");
  
	bus = gst_pipeline_get_bus(GST_PIPELINE(_pipeline));
	gst_bus_add_watch(bus, bus_call, NULL);
	gst_object_unref(bus);
 
	_seek_inprogress = FALSE;
}

void stream_player_shutdown()
{
	printf("Stream Player - shutdown\n");
	
	gst_element_set_state(GST_ELEMENT(_pipeline), GST_STATE_NULL);
	gst_object_unref(_pipeline);
}

