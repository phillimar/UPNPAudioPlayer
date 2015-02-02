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



#include "logger.h"
#include <stdarg.h>
#include <stdio.h>
#include <syslog.h>
#include <time.h>



static enum LogSeverity _severity = Default;
static enum LogLocation _logLocation = LocationDefault;

static int _logInit = 0;


void setLogLevel(enum LogSeverity s)
{
	_severity = s;
}

void setLogLocation(enum LogLocation l)
{
	_logLocation = l;
	
	if (l == LocationSyslog && !_logInit)
	{
		openlog("upnpaudioplayer", 0, LOG_DAEMON);
		_logInit=1;	
	}
}

void logClose()
{
	if ( _logInit )
	{
		closelog();
	}
}

int mapPriority(enum LogSeverity s)
{
	switch (s)
	{
		case Fatal:
			return LOG_CRIT;
			break;
		case Warn:
			return LOG_WARNING;
			break;
		case Default:
			return LOG_NOTICE;
			break;
		case Detail:
			return LOG_DEBUG;
			break;
	}
}

void _printLog(enum LogSeverity s, const char* file, unsigned int line, const char* fmt, ...)
{
	if ( s<=_severity )
	{
		if (_logLocation == LocationDefault )
		{
			time_t t = time(NULL);
			struct tm tm = *localtime(&t);
			printf("%02d-%02d-%02d %02d:%02d:%02d : ",
				tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
				
			if ( s == Fatal )
				printf("*** FATAL *** at %s line %d   : ", file, line);
			else
				printf("%s line %d   : ", file, line);
	
			va_list args;
			va_start(args, fmt);
			vprintf(fmt, args);
			va_end(args);
		}
		else
		{
			va_list args;
			va_start(args, fmt);
			vsyslog(mapPriority(s), fmt, args);
			va_end(args);
		}
	}
}

