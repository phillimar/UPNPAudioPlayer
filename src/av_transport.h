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

#ifndef _AV_TRANSPORT_H_
#define _AV_TRANEPORT_H_

#include <libgupnp/gupnp.h>



void av_transport_init(GUPnPService *service);

void avt_notify_position( gint64 position_ns );
void avt_notify_duration( gint64 duration_ns );

void avt_notify_error(const char* error);
void avt_notify_endofstream();


#endif

