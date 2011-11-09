/*Function : test utime()
 *Description :
 *Author :
 *History : Shark
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <utime.h>
#include <sys/time.h>
#include <errno.h>
#include <time.h>


int main(int argc, char **argv)
{
	struct utimbuf utime_buff;
	struct timeval timeval_buff[2];
	struct tm tm_time;

	#if 0
	struct utimbuf {
               time_t actime;       /* access time */
               time_t modtime;      /* modification time */
           };
	struct tm {
               int tm_sec;         /* seconds */
               int tm_min;         /* minutes */
               int tm_hour;        /* hours */
               int tm_mday;        /* day of the month */
               int tm_mon;         /* month */
               int tm_year;        /* year */
               int tm_wday;        /* day of the week */
               int tm_yday;        /* day in the year */
               int tm_isdst;       /* daylight saving time */
           };
	#endif
	
	tm_time.tm_year = 1989 - 1900;
	tm_time.tm_mon = 10 -1;
	tm_time.tm_mday = 1;
	tm_time.tm_hour = 11 + 1;
	tm_time.tm_min = 11;
	tm_time.tm_sec = 11;
	
	//time_t mktime(struct tm *tm);
	utime_buff.actime = mktime(&tm_time);
	utime_buff.modtime = mktime(&tm_time);
	
//       int utime(const char *filename, const struct utimbuf *times);
  //     int utimes(const char *filename, const struct timeval times[2]);
	if(utime(argv[1], &utime_buff) < 0)
	{	
		// change file last access and modification times
		fprintf(stderr, "change file %s last access and modification times failed: %s\n", argv[1], strerror(errno));
		exit(1);
	}
	fprintf(stdout, "change file %s last access and modification time successfully\n", argv[1]);
	
	return 0;
}
