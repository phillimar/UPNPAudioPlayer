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


#ifndef _STREAM_PLAYER_H_
#define _STREAM_PLAYER_H_

#include <gst/gst.h>

void stream_player_init();
void stream_player_shutdown();

void sp_set_uri(const char *uri);
void sp_play();
void sp_pause();
void sp_seek_ns(gint64 position_ns);
void sp_set_volume(float volume); /* 0=mute, 1=max */
gint64 sp_get_position_ns();
gint64 sp_get_duration_ns();

#endif

