#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <errno.h>
#include <unistd.h>
#include <jpeglib.h>
#include <jerror.h>


#define FBPATH "/dev/fb0"

typedef struct {
	int fb_fd;
	struct fb_var_screeninfo vinfo;
	struct fb_fix_screeninfo finfo;
	long fb_size;
	unsigned char *fb_p;
} FB;

typedef struct {
	unsigned long lc_x;
	unsigned long lc_y;
	unsigned long lenth;
	unsigned long wide;
} LC;


typedef union {
	unsigned long cr_rgbo;
	struct {
		unsigned char cr_r;
		unsigned char cr_g;
		unsigned char cr_b;
		unsigned char cr_o;
	} rgbo;
} CR;

typedef struct {
	unsigned char *buffer;
	long x;
	long y;
} JPEG;

int open_fb(FB *arg)
{
       //int open(const char *pathname, int flags);
	if((arg->fb_fd = open(FBPATH, O_RDWR)) < 0)
	{
		fprintf(stderr, "open fb failed: %s\n", strerror(errno));
		exit(1);
	}
	
	return 0;
}

int ioctl_fb(FB *arg)
{
       //int ioctl(int d, int request, ...);
	if(ioctl(arg->fb_fd, FBIOGET_FSCREENINFO, &arg->finfo) < 0)
	{
		fprintf(stderr, " Reading fixed information failed :%s\n", strerror(errno));
		exit(2);
	}

	if(ioctl(arg->fb_fd, FBIOGET_VSCREENINFO, &arg->vinfo) < 0)
	{
		fprintf(stderr, " Reading variable information failed :%s\n", strerror(errno));
		exit(3);
	}

	return 0;
}

int mmap_fb(FB *arg)
{
	arg->fb_size = arg->vinfo.xres * arg->vinfo.yres *arg->vinfo.bits_per_pixel / 8;

       //void *mmap(void *addr, size_t length, int prot, int flags,int fd, off_t offset);

	if((arg->fb_p = (unsigned char *)mmap(NULL, arg->fb_size, PROT_READ | PROT_WRITE, MAP_SHARED, arg->fb_fd, 0)) == (void *)-1)
	{
		fprintf(stderr, "map devices FBPATH failed :%s\n", strerror(errno));
		exit(4);
	}

	return 0;
}

int munmap_fb(FB *arg)
{
      // int munmap(void *addr, size_t length);
	munmap(arg->fb_p, arg->fb_size);
	close(arg->fb_fd);

	return 0;
}

int disp_fb(FB *arg, LC *lc, CR *cr)
{
	int x, y;
       
        unsigned char *p = arg->fb_p;

        for(y = 0; y < lc->lenth; y++)
        {   
        	for(x = 0; x < lc->wide; x++)
       		{   
         		*(p + arg->vinfo.bits_per_pixel/8*x + arg->vinfo.xres*arg->vinfo.bits_per_pixel/8*y + \
			lc->lc_y*arg->vinfo.xres*arg->vinfo.bits_per_pixel/8 + lc->lc_x*arg->vinfo.bits_per_pixel/8 + 0)= cr->rgbo.cr_b;
         		*(p + arg->vinfo.bits_per_pixel/8*x + arg->vinfo.xres*arg->vinfo.bits_per_pixel/8*y + \
			lc->lc_y*arg->vinfo.xres*arg->vinfo.bits_per_pixel/8 + lc->lc_x*arg->vinfo.bits_per_pixel/8 + 1)= cr->rgbo.cr_g;
         		*(p + arg->vinfo.bits_per_pixel/8*x + arg->vinfo.xres*arg->vinfo.bits_per_pixel/8*y + \
			lc->lc_y*arg->vinfo.xres*arg->vinfo.bits_per_pixel/8 + lc->lc_x*arg->vinfo.bits_per_pixel/8 + 2)= cr->rgbo.cr_r;
         		*(p + arg->vinfo.bits_per_pixel/8*x + arg->vinfo.xres*arg->vinfo.bits_per_pixel/8*y + \
			lc->lc_y*arg->vinfo.xres*arg->vinfo.bits_per_pixel/8 + lc->lc_x*arg->vinfo.bits_per_pixel/8 + 3)= cr->rgbo.cr_o;
      		}
        }  

	return 0;
}

int disp_line(FB *arg, LC *lc, JPEG *jpeg)
{
	long i;
	unsigned char *p = arg->fb_p;

        	for(i = 0; i < jpeg->x; i++)
       		{  
         		*(p + arg->vinfo.bits_per_pixel/8*i + arg->vinfo.xres*arg->vinfo.bits_per_pixel/8*jpeg->y + \
			lc->lc_y*arg->vinfo.xres*arg->vinfo.bits_per_pixel/8 + lc->lc_x*arg->vinfo.bits_per_pixel/8 + 0)= *(jpeg->buffer + i * 3 + 2);
         		*(p + arg->vinfo.bits_per_pixel/8*i + arg->vinfo.xres*arg->vinfo.bits_per_pixel/8*jpeg->y + \
			lc->lc_y*arg->vinfo.xres*arg->vinfo.bits_per_pixel/8 + lc->lc_x*arg->vinfo.bits_per_pixel/8 + 1)= *(jpeg->buffer + i * 3 + 1);
         		*(p + arg->vinfo.bits_per_pixel/8*i + arg->vinfo.xres*arg->vinfo.bits_per_pixel/8*jpeg->y + \
			lc->lc_y*arg->vinfo.xres*arg->vinfo.bits_per_pixel/8 + lc->lc_x*arg->vinfo.bits_per_pixel/8 + 2)= *(jpeg->buffer + i * 3 + 0);
         		*(p + arg->vinfo.bits_per_pixel/8*i + arg->vinfo.xres*arg->vinfo.bits_per_pixel/8*jpeg->y + \
			lc->lc_y*arg->vinfo.xres*arg->vinfo.bits_per_pixel/8 + lc->lc_x*arg->vinfo.bits_per_pixel/8 + 3)= 0xff;
      		}
	
	return 0;
}

int clean_screen(int data, FB *arg)
{
	int x, y;
       
        unsigned char *p = arg->fb_p;

        for(y = 0; y < arg->vinfo.yres; y++)
        {   
        	for(x = 0; x < arg->vinfo.xres; x++)
       		{   
         		*(int *)(p + 4*x + 1280*4*y)= data;
      		}
        }  

	return 0;
}

int move_pic(FB *arg, LC *lc)
{
	static unsigned char flag = 0x01;

	if(lc->lc_y >= arg->vinfo.yres - lc->lenth || lc->lc_x >= arg->vinfo.xres - lc->wide || !(flag & 0x01))
	{
		flag &= 0x00;
		lc->lc_x--;
		lc->lc_y--;
	}
	if(lc->lc_y <= 0 || lc->lc_x <= 0 || (flag & 0x01))
	{
		flag |= 0x01;
		lc->lc_x++;
		lc->lc_y++;
	}

	return 0;
}
int main(int argc, char **argv)
{
	FB *fb_arg;
	LC fb_lc;
	CR fb_cr;
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	FILE           *infile;
	JPEG jpeg;
#if 0	
	if (argc != 2)
	{
		fprintf(stdout, "Usage: %s <filename>\n", argv[0]);
	}
#endif
	fb_lc.lc_x = 0;
	fb_lc.lc_y = 0;
	fb_lc.lenth = 320;
	fb_lc.wide = 240;

	fb_cr.cr_rgbo = 0xff0fff00;

	//void *malloc(size_t size);
	if((fb_arg = (FB *)malloc(sizeof(FB))) == NULL)
	{
		fprintf(stderr, "malloc failed :%s\n", strerror(errno));
		exit(1);
	}

	open_fb(fb_arg);
	ioctl_fb(fb_arg);
	mmap_fb(fb_arg);
	printf("fb:%dx%d/%dbits\n",fb_arg->vinfo.xres ,fb_arg->vinfo.yres, fb_arg->vinfo.bits_per_pixel);	

	int j = 1;
	while(argv)
	{
	clean_screen(0x00, fb_arg);
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	// open jpeg file
	if ((infile = fopen(argv[j], "rb")) == NULL)
	{
		perror("fopen");
		free(fb_arg);
		exit(1);
	}

	jpeg_stdio_src(&cinfo, infile);
	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);

	if ((cinfo.output_width > fb_arg->vinfo.xres) || (cinfo.output_height > fb_arg->vinfo.yres)) 
	{
		printf("too large JPEG file,cannot display\n");
		munmap_fb(fb_arg);
		fclose(infile);
		exit(1);
	}

	jpeg.buffer = (unsigned char *) malloc(cinfo.output_width * cinfo.output_components);
	
	if (jpeg.buffer == NULL)
	{
		perror("malloc");
		munmap_fb(fb_arg);
		fclose(infile);
		exit(1);

	}

	fb_lc.lc_x = (fb_arg->vinfo.xres - cinfo.output_width) / 2;
	fb_lc.lc_y = (fb_arg->vinfo.yres - cinfo.output_height) / 2;
	jpeg.y = 0;
	jpeg.x = cinfo.output_width;
	while (cinfo.output_scanline < cinfo.output_height)
	{
		jpeg_read_scanlines(&cinfo, &jpeg.buffer, 1);
		disp_line(fb_arg, &fb_lc, &jpeg);
		usleep(5000);
		jpeg.y++;
	}

//	disp_fb(fb_arg, &fb_lc, &fb_cr);

	fclose(infile);
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	j++;
	free(jpeg.buffer);
	sleep(3);
	}
	munmap_fb(fb_arg);
	free(fb_arg);
//	clean_screen(0x00, fb_arg);

	return 0;
}
