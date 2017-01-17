/* Logging library, try to log as HTML */

#include <libLog.h>
#include <time.h>
#include <sys/timeb.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define MAXLOGNUM 5

typedef struct _logfile {
  FILE *log;
  int firstly;
} logfile;

static logfile logs[MAXLOGNUM] = {{0, 0}};
static char header[] = "<html><head><title>Log</title></head><body><table>\n";
static char footer[] = "</table></body></html>";

static char info[] = "<font color='green'><i>%s</i></font>\n";
static char warning[] = "<font color='yellow'>%s</font>\n";
static char critical[] = "<font color='red'><b>%s</b></font>\n";

static char line_begin[] = "<tr><td>\n";
static char line_sep[] = "</td><td>\n";
static char line_end[] = "</td></tr>\n";

static int log_write_time(int log)
{
  struct timeb currtime_b;
  char currtime_s[256];
  struct tm *currtime_tm;
  
  if((log < 0) || (log >= MAXLOGNUM)) return EINVALLOG;
  
  ftime(&currtime_b);
  currtime_tm = localtime(&currtime_b.time);
  sprintf(currtime_s, "<font color='blue'><u>[%02d.%02d.%04d %02d:%02d:%02d.%04d]:</u></font>\n", currtime_tm->tm_mday,
	  currtime_tm->tm_mon+1, currtime_tm->tm_year, currtime_tm->tm_hour, currtime_tm->tm_min,
	  currtime_tm->tm_sec, currtime_b.millitm);
	  
  fwrite(currtime_s, strlen(currtime_s), 1, logs[log].log);
  fflush(logs[log].log);
}

int log_write_string(int log, const char *string, int loglevel)
{
  char tmpstr[1024];
  
  if((log < 0) || (log >= MAXLOGNUM)) return EINVALLOG;
  
  fwrite(line_begin, strlen(line_begin), 1, logs[log].log);
  log_write_time(log);
  fwrite(line_sep, strlen(line_sep), 1, logs[log].log);
  switch (loglevel)
  {
    case LOG_INFO:
      sprintf(tmpstr, info, string);
      break;
    case LOG_WARN:
      sprintf(tmpstr, warning, string);
      break;
    case LOG_CRIT:
      sprintf(tmpstr, critical, string);
      break;
  }
  
  fwrite(tmpstr, strlen(tmpstr), 1, logs[log].log);
  fwrite(line_end, strlen(line_end), 1, logs[log].log);
  fflush(logs[log].log);
}

int log_open(char *path)
{
  int i;
  
  //printf("LOG_OPEN STARTED\n");
  
  for(i = 0; (i < MAXLOGNUM) && (logs[i].log != 0); i++); 
  if (i == MAXLOGNUM) return ELOGMAXRCHD;
  
  logs[i].log = fopen(path, "r");
  
  if(logs[i].log != 0)
  {
    fclose(logs[i].log);
    //printf("log exists, not need to create\n");
    logs[i].firstly = 0;
  } else {
    //printf("log not exists, need to create\n");
    logs[i].firstly = 1;
  }
  
  logs[i].log = fopen(path, "a");
  if (logs[i].log == 0) return ELOGNOPEN;
  
  if(logs[i].firstly) 
  {
    fwrite(header, strlen(header), 1, logs[i].log);
  } else {
    /*	TODO
    int pos = 0;
    lseek(logs[i].log, 0, SEEK_END);
    pos = ftell(logs[i].log);
    printf("pos is %d\n", pos);
    pos = pos - strlen(footer);
    printf("pos is %d\n", pos);
    rewind(logs[i].log);
    lseek(logs[i].log, pos, SEEK_SET);*/
  }
  
  log_write_string(i, "Log started", LOG_INFO);
  
  return i;
}

int log_close(int log)
{
  if((log < 0) || (log >= MAXLOGNUM)) return EINVALLOG;
  
  log_write_string(log, "Log finished", LOG_INFO);
  
  //if(logs[log].firstly) fwrite(footer, strlen(footer), 1, logs[log].log);
  fclose(logs[log].log);
  
  logs[log].log = 0;
}

