#!/bin/sh

dataInstallDir=($DESTDIR)/etc/upnpaudio
defaultInstallDir=($DESTDIR)/etc/default
initInstallDir=($DESTDIR)/etc/init.d
binInstallDir=($DESTDIR)/usr/sbin
manInstallDir=($DESTDIR)/usr/share/man/man1

echo
echo "*** Uninstalling upnpaudiod ***"

# Have we got root ?
if [ "$(id -u)" != "0" ]; then
   echo "Must be running as root to uninstall upnpaudiod" 1>&2
   exit 1
fi

update-rc.d upnpaudio remove
rm -fR ${dataInstallDir}
rm -fR ${binInstallDir}/upnpaudiod
rm -fR ${initInstallDir}/upnpaudio
rm -fR ${defaultInstallDir}/upnpaudio
rm -fR ${manInstallDir}/upnpaudiod.1.*

