/* Logging library, try to log as HTML */

//Errors
#define ELOGMAXRCHD	-1
#define ELOGNOPEN	-2
#define EINVALLOG	-3

//Log record types
#define LOG_INFO	1
#define LOG_WARN	2
#define LOG_CRIT	3

//Functions
int log_open(char *path);
int log_write_string(int log, const char *string, int loglevel);
int log_close(int log);
