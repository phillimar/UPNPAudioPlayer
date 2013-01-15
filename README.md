UPNPAudioPlayer
===============


What is it ?
------------
It's an application that plays audio that is controlled via UPnP ("Universal Plug and Play") - designed to run on a Raspberry PI. There's no GUI (i.e. its "headless"). It's controlled via a UPnP control point, which could run on an Android or iOS device. Media is streamed to the player via a Media Server which must also be UPnP compliant.


What do I need to use it ?
--------------------------
* A UPnP Media Server to stream the content.
* A UPnP control point (e.g. an Android Tablet)
* A Raspberry PI to run it on that has networking + audio configured. I've used the Debian Squeeze + Raspian distributions.


What's a UPnP Media Server and Control Point ?
----------------------------------------------
Within UPnP, a "Media Server" is something that streams content to a "Renderer". A "Renderer" is something that plays the content. Both are controlled via a "Control Point".  
See the docs directory for more info.  

How do I build and use it ?
---------------------------
It requires the GUPNP and GStreamer libraries to be installed. The installer also requires the uuidgen utility. To do this under Debian :  
sudo apt-get update  
sudo apt-get -y install libgstreamer0.10-0 libgstreamer0.10-dev gstreamer0.10-tools gstreamer0.10-doc  
sudo apt-get -y install gstreamer0.10-alsa gstreamer0.10-plugins-base-apps gstreamer0.10-plugins-base-doc gstreamer0.10-plugins-base gstreamer0.10-plugins-good gstreamer0.10-plugins-good-doc gstreamer0.10-plugins-bad gstreamer0.10-plugins-bad-doc gstreamer0.10-plugins-ugly gstreamer0.10-plugins-ugly-doc gst123  
sudo apt-get -y install gupnp-tools libgupnp-1.0-4 libgupnp-1.0-dev libgupnp-doc  
sudo apt-get -y install uuid-runtime  

To make the app  
cd UPNPAudioPlayer/src  
make spotless ; make  

To run it :  
./audio_player  

To make an installable package :  
make spotless  
make install  

This will create a UPNPAudioPlayer.tgz file in the src directory. Copy this file to where ever you want to install then do :  
tar -xvzf UPNPAudioPlayer.tgz
which will unpack it. It'll unpack to a directory named UPNPAudioPlayer. So don't try and unpack to the same place as you put the source...

To start and stop the player use :
UPNPAudioPlayer/bin/start_audio_player
and
UPNPAudioPlayer/bin/stop_audio_player

You'll likely want to add UPNPAudioPlayer/bin to your PATH.


What audio formats does it support ?
------------------------------------
UPNP Audio Player uses GStreamer to play the audio stream. So if your audio format is supported by GStreamer you'll be able to get it to work. If not you won't (unless you want to write a new GStreamer plugin).  
I say "you'll be able to get it to work" because I've been lazy. A UPnP renderer is required to report all the formats it supports to a control point. I've hard coded this. You may need to take a look at the source file "connection_manager.c" and add your format to the "SinkProtocolInfo" variable. There's a comment to guide you...


How do I tell if GStreamer Supports format X
--------------------------------------------
Use the "gst123" app we installed above. For example :  
gst123 file:///home/pi/mytrack.mp3


I don't get any audio output from my PI. What do I do ?
-------------------------------------------------------
For the PI's audio jack try :

sudo modprobe snd-bcm2385

At the time of writing quality isn't great. I'm sure the wonderful PI folk will sort this out over time.

USB :
Have a look at the "my-setup" in the docs directory

HDMI : 
No idea. Look at the PI forums.


I can't build it - what do I do ?
---------------------------------
Check you've installed the dependencies above.  
What does "gupnp-binding-tool --help" give you? make sure /usr/bin is in your path. If it is - have you installed the dependencies above ?   
Do you see a "/usr/include/gupnp-1.0" directory ?  
How about "/usr/include/gstreamer-0.10" ?  
What does gst123 --help do ?   
Can you build anything ?  cc -v ?  


Will it run on other linux machines?
------------------------------------
I've built on Ubuntu 12.10. I'd imagine it will work under other linux flavours but I've not tried.


How do I find it more ?
-----------------------
There's some more information in the documents directory.  



Enjoy !



Mark.

