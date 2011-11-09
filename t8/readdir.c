#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>


int main(int argc, char **argv)
{
	DIR *dir;
	struct dirent *read_dir;

       //DIR *opendir(const char *name);
	if(!(dir = opendir(argv[1])))
	{
		fprintf(stderr, "open a dirctory %s faield : %s\n", argv[1], strerror(errno));
		exit(1);
	}	
	fprintf(stderr, "open a dirctory %s successfully\n", argv[1]);
	
	#if 0
	struct dirent {
               ino_t          d_ino;       /* inode number */
               off_t          d_off;       /* offset to the next dirent */
               unsigned short d_reclen;    /* length of this record */
               unsigned char  d_type;      /* type of file; not supported
                                              by all file system types */
               char           d_name[256]; /* filename */
           };

	#endif
      // struct dirent *readdir(DIR *dirp);
	if(!(read_dir = readdir(dir)))
	{
		fprintf(stderr, "read a dirctory %s failed : %s \n", argv[1], strerror(errno));
		closedir(dir);
		exit(1);
	}
	fprintf(stdout, "read a dirctory %s successfully\n", argv[1]);

	fprintf(stdout, "inod = %d\noffset = %d\reclen = %hu\ntype:%c\nnname : %s \n", (int)read_dir->d_ino, (int)read_dir->d_off, read_dir->d_reclen, read_dir->d_type, read_dir->d_name);

	closedir(dir);	

	return 0;
}
