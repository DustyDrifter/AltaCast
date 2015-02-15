#ifndef __TRANSLOG_H__
#define __TRANSLOG_H__

#ifdef __cplusplus
extern "C" {
#endif
void LogMessage(int type, char *source, int line, char *fmt, ...);
void setErrorType(int type);
void setLogFile(char *file);
#ifdef __cplusplus
}
#endif

#define LM_FORCE 0
#define LM_ERROR 1
#define LM_INFO 2
#define LM_DEBUG 3
#define LOG_FORCE LM_FORCE, __FILE__, __LINE__
#define LOG_ERROR LM_ERROR, __FILE__, __LINE__
#define LOG_INFO LM_INFO, __FILE__, __LINE__
#define LOG_DEBUG LM_DEBUG, __FILE__, __LINE__
#define Log	LogMessage

#endif
