#include <libLog.h>

int main()
{
	int log = log_open("testlog.html");
	if (log < 0) printf("LOG ERROR\n");
	log_write_string(log, "Hello from log world!!!!!", LOG_INFO);
	log_write_string(log, "Hello from log world!!!!!", LOG_WARN);
	log_write_string(log, "Hello from log world!!!!!", LOG_CRIT);
	log_close(log);


	log = log_open("testlog.html");
	log_write_string(log, "Hello from log world!!!!!", LOG_INFO);
	log_write_string(log, "Hello from log world!!!!!", LOG_WARN);
	log_write_string(log, "Hello from log world!!!!!", LOG_CRIT);
	log_close(log);
}

