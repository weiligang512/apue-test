#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <pwd.h>




int main(int argc, char **argv)
{
	struct passwd *pwd;

       //struct passwd *getpwnam(const char *name);
       //struct passwd *getpwuid(uid_t uid);
	pwd = getpwnam("bsec");
#if 0
	struct passwd {
               char   *pw_name;       /* username */
               char   *pw_passwd;     /* user password */
               uid_t   pw_uid;        /* user ID */
               gid_t   pw_gid;        /* group ID */
               char   *pw_gecos;      /* real name */
               char   *pw_dir;        /* home directory */
               char   *pw_shell;      /* shell program */
           };
#endif
	puts(pwd->pw_passwd);
	printf("%d\n", pwd->pw_uid);

	return 0;
}
