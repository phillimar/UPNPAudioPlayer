
CC=cc
BINDING_TOOL=gupnp-binding-tool
BINDING_TOOL_FLAGS=-m server

CFLAGS = -c $(shell pkg-config --cflags gupnp-1.0) $(shell pkg-config --cflags gstreamer-0.10)		 
LDFLAGS = $(shell pkg-config --libs gupnp-1.0) $(shell pkg-config --libs gstreamer-0.10)

SOURCES=audio_player.c av_transport.c connection_manager.c rendering_control.c stream_player.c
OBJECTS=$(SOURCES:.c=.o)

XML_DIR=../data
XML_SOURCES=AVTransport.xml ConnectionManager.xml RenderingControl.xml
XML_OBJECTS=$(XML_SOURCES:.xml=_wrapper.c)

EXECUTABLE=audio_player


all: wrappers audio_player

audio_player: $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXECUTABLE) $(LDFLAGS)
	chmod +x $(EXECUTABLE)

.c.o: $(SOURCES)
	$(CC) $(CFLAGS) $< -o $@


%_wrapper.c: $(XML_DIR)/%.xml
	$(BINDING_TOOL) $(BINDING_TOOL_FLAGS) -p $(shell echo $< | awk -f ./build_scripts/make_prefix ) $< > $@

wrappers: $(XML_OBJECTS)

clean:
	rm -rf *.o $(EXECUTABLE)
	rm -fR UPNPAudioPlayer
	rm -rf UPNPAudioPlayer.tgz
	
spotless:
	rm -rf *.o $(EXECUTABLE) *_wrapper.c
	rm -fR UPNPAudioPlayer
	rm -rf UPNPAudioPlayer.tgz

install: all
	sh ./build_scripts/make_install_package.sh

