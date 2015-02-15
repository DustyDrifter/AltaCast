AUTOMAKE_OPTIONS = foreign 1.6

lib_LIBRARIES = liboddcast.a
#liboddcast_la_LDFLAGS = -version-info 3:0:0

include_HEADERS = liboddcast.h liboddcast_resample.h liboddcast_socket.h
liboddcast_a_SOURCES = liboddcast.cpp \
						resample.c \
						Socket.cpp

EXTRA_DIST = \
				enc_if.h \
				liboddcast.dsp \
				liboddcast.h \
				liboddcast_resample.h \
				liboddcast_socket.h

LIBS = @LIBS@ @OGG_LIBS@ @VORBIS_LIBS@ @MAD_LIBS@ @LAME_LIBS@ @VORBISENC_LIBS@ @VORBISFILE_LIBS@ @FAAC_LIBS@ @LIBFLAC_LIBS@
CFLAGS = -g @CFLAGS@ @OGG_CFLAGS@ @VORBIS_CFLAGS@ @MAD_CFLAGS@ @LAME_CFLAGS@ @FAAC_CFLAGS@ @LIBFLAC_CFLAGS@
