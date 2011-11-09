#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

      // int fstat(int fd, struct stat *buf);
      // int lstat(const char *path, struct stat *buf);




int main(int argc, char **argv)
{
	struct stat st;
	char *file_type_mode[] =
	{
		"NULL",
		"FIFO",
		"charactr device",
		"NULL",
		"directory",
		"NULL",
		"block device",
		"NULL",
		"regular file",
		"NULL",
		"symbolic link",
		"NULL",
		"socket"	
	};
	#if 0
      // int stat(const char *path, struct stat *buf);
	 struct stat {
               dev_t     st_dev;     /* ID of device containing file */
               ino_t     st_ino;     /* inode number */
               mode_t    st_mode;    /* protection */
               nlink_t   st_nlink;   /* number of hard links */
               uid_t     st_uid;     /* user ID of owner */
               gid_t     st_gid;     /* group ID of owner */
               dev_t     st_rdev;    /* device ID (if special file) */
               off_t     st_size;    /* total size, in bytes */
               blksize_t st_blksize; /* blocksize for file system I/O */
               blkcnt_t  st_blocks;  /* number of 512B blocks allocated */
               time_t    st_atime;   /* time of last access */
               time_t    st_mtime;   /* time of last modification */
               time_t    st_ctime;   /* time of last status change */
           };
	   S_IFMT     0170000   bit mask for the file type bit fields
           S_IFSOCK   0140000   socket
           S_IFLNK    0120000   symbolic link
           S_IFREG    0100000   regular file
           S_IFBLK    0060000   block device
           S_IFDIR    0040000   directory
           S_IFCHR    0020000   character device
           S_IFIFO    0010000   FIFO
           S_ISUID    0004000   set UID bit
           S_ISGID    0002000   set-group-ID bit (see below)
           S_ISVTX    0001000   sticky bit (see below)
           S_IRWXU    00700     mask for file owner permissions
           S_IRUSR    00400     owner has read permission
           S_IWUSR    00200     owner has write permission
           S_IXUSR    00100     owner has execute permission
           S_IRWXG    00070     mask for group permissions
           S_IRGRP    00040     group has read permission
           S_IWGRP    00020     group has write permission
           S_IXGRP    00010     group has execute permission
           S_IRWXO    00007     mask for permissions for others (not in group)
           S_IROTH    00004     others have read permission
           S_IWOTH    00002     others have write permission
           S_IXOTH    00001     others have execute permission

	#endif

	if(argc != 2)
	{
		fprintf(stdout, "Usage: %s <pathname>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
//	int open(const char *pathname, int flags);
     //  int open(const char *pathname, int flags, mode_t mode);
	int fd;

	if((fd = open(argv[1], O_RDONLY)) < 0)
	{
		perror("argv[1]");
		exit(EXIT_FAILURE);
	}
	if(fstat(fd, &st))
	{
		perror("argv[1]");
		exit(EXIT_FAILURE);
	}
	fprintf(stdout, "  File: \"%s\"\n", argv[1]);
	fprintf(stdout, "  Size: %ld            Blocks: %ld          IO block: %ld  %s\n",\
		 st.st_size, st.st_blocks, st.st_blksize, file_type_mode[(st.st_mode & S_IFMT) >> 12]);
	fprintf(stdout, "Device: ?/?            Inode: %ld           Links: %ld\n", (long)st.st_ino, (long)st.st_nlink);

	return 0;
}
