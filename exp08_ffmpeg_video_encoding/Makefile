CC = g++
CFLAGS = -Wall -Wextra
LIBS = -lavcodec -lavutil -lavformat -lavfilter -lswscale

SOURCES = ffmpeg_encode.cpp videoenc_ffmpeg.cpp
EXECUTABLE = ffmpeg_encode

all: $(EXECUTABLE)

$(EXECUTABLE): $(SOURCES)
	$(CC) $(CFLAGS) -o $(EXECUTABLE) $(SOURCES) $(LIBS)

clean:
	rm -f $(EXECUTABLE)
