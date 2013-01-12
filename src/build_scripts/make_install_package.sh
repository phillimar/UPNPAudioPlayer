#!/bin/sh

replaceXMLTag()
{
	local tagName="$1"
	local newValue="$2"
	local fileName="$3"
	
	sed "s#<${tagName}>.*#<${tagName}>${newValue}</${tagName}>#" ${fileName} > ${fileName}.tmp
	mv ${fileName}.tmp ${fileName}
}

echo
echo "*** Creating Install Package ***"

rm -fR ./UPNPAudioPlayer
rm -rf ./UPNPAudioPlayer.tgz
mkdir ./UPNPAudioPlayer

if [ -e ./friendlyName ]; then
	friendlyName=`cat ./friendlyName`
	echo "Using Friendly Name : ${friendlyName}"
else
	echo "Enter a friendly name for this player"
	read friendlyName
fi

# copy config data
mkdir ./UPNPAudioPlayer/data
cp -R ../data/*.xml ./UPNPAudioPlayer/data

# set the friendly name
replaceXMLTag "friendlyName" "${friendlyName}" "./UPNPAudioPlayer/data/MediaRenderer.xml"

# set a UUID
newUUID=`uuidgen`
replaceXMLTag "UDN" "${newUUID}" "./UPNPAudioPlayer/data/MediaRenderer.xml" "./UPNPAudioPlayer/data/MediaRenderer.xml"

# copy executables
mkdir ./UPNPAudioPlayer/bin
cp ./audio_player ./UPNPAudioPlayer/bin
cp ../scripts/start_audio_player ./UPNPAudioPlayer/bin
cp ../scripts/stop_audio_player ./UPNPAudioPlayer/bin

tar -czf UPNPAudioPlayer.tgz UPNPAudioPlayer
rm -fR ./UPNPAudioPlayer


