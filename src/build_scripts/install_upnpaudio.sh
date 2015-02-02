#!/bin/sh

dataInstallDir=($DESTDIR)/etc/upnpaudio
defaultInstallDir=($DESTDIR)/etc/default
initInstallDir=($DESTDIR)/etc/init.d
binInstallDir=($DESTDIR)/usr/sbin
manInstallDir=($DESTDIR)/usr/share/man/man1

replaceXMLTag()
{
	local tagName="$1"
	local newValue="$2"
	local fileName="$3"
	
	sed "s#<${tagName}>.*#<${tagName}>${newValue}</${tagName}>#" ${fileName} > ${fileName}.tmp
	mv ${fileName}.tmp ${fileName}
}

echo
echo "*** Installing upnpaudiod ***"

# Have we got root ?
if [ "$(id -u)" != "0" ]; then
   echo "Must be running as root to install upnpaudiod" 1>&2
   exit 1
fi

if [ -e ./friendlyName ]; then
	friendlyName=`cat ./friendlyName`
	echo "Using Friendly Name : ${friendlyName}"
else
	friendlyName=UPNPAudioPlayer
	echo "Using default Friendly Name (create a file containing a different name at src/friendlyName to override)"
fi

# copy config data
rm -fR ${dataInstallDir}
mkdir ${dataInstallDir}
cp -R ../data/*.xml ${dataInstallDir}

# set the friendly name
replaceXMLTag "friendlyName" "${friendlyName}" "${dataInstallDir}/MediaRenderer.xml"

# set a UUID
newUUID=`uuidgen`
replaceXMLTag "UDN" "uuid:${newUUID}" "${dataInstallDir}/MediaRenderer.xml"

# copy executable
cp ./upnpaudiod ${binInstallDir}
# copy start/ stop script
cp ../scripts/upnpaudio ${initInstallDir}
# copy default
cp ../scripts/default/upnpaudio ${defaultInstallDir}
# copy and zip man pages
cp ../scripts/man1/upnpaudiod.1 ${manInstallDir}
gzip -f ${manInstallDir}/upnpaudiod.1

# set autostart
update-rc.d upnpaudio defaults

