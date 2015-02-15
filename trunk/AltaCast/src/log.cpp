#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include "log.h"

static	int	globalErrorType = LM_INFO;
static	char	gLogFile[1024] = "transcoder.log";

static	FILE	*filep = 0;

void setErrorType(int type) {
	globalErrorType = type;
}

void setLogFile(char *file) {
	if (file) {
		strcpy(gLogFile, file);
	}
	if (filep) {
		fclose(filep);
		filep = 0;
	}
}
void LogMessage(int type, char *source, int line, char *fmt, ...) {
	va_list parms;
	char	errortype[25] = "";
	int	addNewline = 1;
	struct tm *tp;
	time_t t;
	int parseableOutput = 0;
	char    timeStamp[255];

	memset(timeStamp, '\000', sizeof(timeStamp));

	time(&t);
	tp = localtime(&t);
	strftime(timeStamp, sizeof(timeStamp), "%m/%d/%y %T", tp);

	switch (type) {
		case LM_ERROR:
			strcpy(errortype, "Error");
			break;
		case LM_INFO:
			strcpy(errortype, "Info");
			break;
		case LM_DEBUG:
			strcpy(errortype, "Debug");
			break;
		default:
			strcpy(errortype, "Unknown");
			break;
	}

	if (fmt[strlen(fmt)-1] == '\n') {
		addNewline = 0;
	}


	if (type <= globalErrorType) {
		va_start(parms, fmt);

		if (filep == 0) {
			filep = fopen(gLogFile, "a");
		}
		
		if (!filep) {
			fprintf(stdout, "Cannot open logfile: %s(%s:%d): ", errortype, source, line);
			vfprintf(stdout, fmt, parms);
			va_end(parms);
			if (addNewline) {
				fprintf(stdout, "\n");
			}
		}
		else {
			fprintf(filep,  "%s %s(%s:%d): ", timeStamp, errortype, source, line);
			vfprintf(filep, fmt, parms);
			va_end(parms);
			if (addNewline) {
				fprintf(filep, "\n");
			}
			fflush(filep);
			//fclose(filep);
		}

	}

}

