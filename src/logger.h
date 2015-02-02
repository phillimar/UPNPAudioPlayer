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


/* Logging Public Interace */


#ifndef __LOGGER_H_
#define __LOGGER_H_

#ifdef __cplusplus
extern "C" {
#endif


#include <stdlib.h>

#define FATAL(...) { _printLog(Fatal, __FILE__, __LINE__, __VA_ARGS__); exit(EXIT_FAILURE); }
#define WARN(...) { _printLog(Warn, __FILE__, __LINE__, __VA_ARGS__); }
#define LOG(...) { _printLog(Default, __FILE__, __LINE__, __VA_ARGS__); }
#define DETAIL(...) { _printLog(Detail, __FILE__, __LINE__, __VA_ARGS__); }


enum LogSeverity {
	Fatal,
	Warn,
	Default,
	Detail
};

enum LogLocation {
	LocationDefault,
	LocationSyslog
};


void setLogLevel(enum LogSeverity s);
void setLogLocation(enum LogLocation l);
void logClose();
void _printLog(enum LogSeverity s, const char* file, unsigned int line, const char* fmt, ...);



#ifdef __cplusplus
}
#endif

#endif /* __LOGGER_H_ */

