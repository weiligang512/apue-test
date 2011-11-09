#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>

int main(int argc, char **argv)
{
	struct passwd *pwd = NULL;	
	uid_t uid;
	
	if (argc != 4)
	{
		fprintf(stdout, "Usage: %s <users> <groups> <pathname>\n", argv[0]);
		exit(1);
	}

	if(atoi(argv[1]) == -1)
	{
		uid = -1;
	}
	else
	{
      // struct passwd *getpwnam(const char *name);
		pwd = getpwnam(argv[1]);
		uid = pwd->pw_uid;
	}

	struct group *gru = NULL;
	gid_t gid;

	if(atoi(argv[2]) == -1)
	{
		gid = -1;
	}
	else
	{
       //struct group *getgrnam(const char *name);
		gru = getgrnam(argv[2]);
		gid = gru->gr_gid;
	}

      // int chown(const char *path, uid_t owner, gid_t group);
	if(chown(argv[3], uid, gid) < 0)
	{
		fprintf(stderr, "change ownership of file %s failed:%s\n", argv[3], strerror(errno));
		exit(1);
	}
	else
	{
		fprintf(stderr, "change ownership of file %s success\n", argv[3]);
	}

	return 0;
}
