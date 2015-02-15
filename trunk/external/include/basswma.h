/* BASSWMA 2.0 C/C++ header file, copyright (c) 2002-2003 Ian Luck.
   Requires BASS 2.0 - available from www.un4seen.com

   See the BASSWMA.CHM file for more complete documentation */

#ifndef BASSWMA_H
#define BASSWMA_H

#include "bass.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef BASSWMADEF
#define BASSWMADEF(f) WINAPI f
#endif

typedef DWORD HWMENCODE;		// WMA encoding handle

// Additional error codes returned by BASS_ErrorGetCode
#define BASS_ERROR_WMA_LICENSE	1000	// the file is protected
#define BASS_ERROR_WMA_WM9		1001	// WM9 is required
#define BASS_ERROR_WMA_DENIED	1002	// access denied (user/pass is invalid)
#define BASS_ERROR_WMA_CODEC	1003	// no appropriate codec is installed

// Additional flags for use with BASS_WMA_EncodeOpen/File/Network/Publish
#define BASS_WMA_ENCODE_TAGS	0x10000 // set tags in the WMA encoding
#define BASS_WMA_ENCODE_SCRIPT	0x20000 // set script (mid-stream tags) in the WMA encoding

// Additional flag for use with BASS_WMA_EncodeGetRates
#define BASS_WMA_ENCODE_RATES_VBR	0x10000 // get available VBR quality settings

typedef void (CALLBACK CLIENTCONNECTPROC)(HWMENCODE handle, BOOL connect, char *ip, DWORD user);
/* Client connection notification callback function.
handle : The encoder
connect: TRUE=client is connecting, FALSE=disconnecting
ip     : The client's IP (xxx.xxx.xxx.xxx:port)
user   : The 'user' parameter value given when calling BASS_EncodeSetNotify */

typedef void (CALLBACK WMENCODEPROC)(HWMENCODE handle, DWORD type, void *buffer, DWORD length, DWORD user);
/* Encoder callback function.
handle : The encoder handle
type   : The type of data, one of BASS_WMA_ENCODE_xxx values
buffer : The encoded data
length : Length of the data
user   : The 'user' parameter value given when calling BASS_WMA_EncodeOpen */

// WMENCODEPROC "type" values
#define BASS_WMA_ENCODE_HEAD	0
#define BASS_WMA_ENCODE_DATA	1
#define BASS_WMA_ENCODE_DONE	2

// BASS_CHANNELINFO type
#define BASS_CTYPE_STREAM_WMA	0x10300


HSTREAM BASSWMADEF(BASS_WMA_StreamCreateFile)(BOOL mem, const void *file, DWORD offset, DWORD length, DWORD flags);
void *BASSWMADEF(BASS_WMA_GetIWMReader)(HSTREAM handle);

int *BASSWMADEF(BASS_WMA_EncodeGetRates)(DWORD freq, DWORD flags);
HWMENCODE BASSWMADEF(BASS_WMA_EncodeOpen)(DWORD freq, DWORD flags, int bitrate, WMENCODEPROC *proc, DWORD user);
HWMENCODE BASSWMADEF(BASS_WMA_EncodeOpenFile)(DWORD freq, DWORD flags, int bitrate, const char *file);
HWMENCODE BASSWMADEF(BASS_WMA_EncodeOpenNetwork)(DWORD freq, DWORD flags, int bitrate, DWORD port, DWORD clients);
HWMENCODE BASSWMADEF(BASS_WMA_EncodeOpenPublish)(DWORD freq, DWORD flags, int bitrate, const char *url, const char *user, const char *pass);
DWORD BASSWMADEF(BASS_WMA_EncodeGetPort)(HWMENCODE handle);
BOOL BASSWMADEF(BASS_WMA_EncodeSetNotify)(HWMENCODE handle, CLIENTCONNECTPROC *proc, DWORD user);
DWORD BASSWMADEF(BASS_WMA_EncodeGetClients)(HWMENCODE handle);
BOOL BASSWMADEF(BASS_WMA_EncodeSetTag)(HWMENCODE handle, const char *tag, const char *text);
BOOL BASSWMADEF(BASS_WMA_EncodeWrite)(HWMENCODE handle, const void *buffer, DWORD length);
void BASSWMADEF(BASS_WMA_EncodeClose)(HWMENCODE handle);

#ifdef __cplusplus
}
#endif

#endif
