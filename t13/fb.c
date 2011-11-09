#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>

//#include "image.h"
//#include "image0.h"
//#include "image1.h"

unsigned int image1[320*240*2]={
	0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,
	0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,
	0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,
	0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,
	0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,
	0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,
	0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,
	0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,
	0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,
	0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,
	0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0,0x07,0xe0
	};

#define FB_NAME "/dev/fb0"
#define TRUE 1
struct FB_MSG
{
        int fb;
        struct fb_var_screeninfo vinfo;
        struct fb_fix_screeninfo finfo;
        long screensize;
        char *fbp;
};

typedef struct FB_MSG FB;


void open_fb(FB *fb_msg)
{
        int fb;
        fb=open(FB_NAME,O_RDWR);
        if (fb<0)
        {
                printf("Error: Can not open framebuffer device \n");
                exit(1);
        }
         fb_msg->fb=fb;
}
int get_fb_msg(FB *fb_msg)
{
        if (ioctl(fb_msg->fb,FBIOGET_FSCREENINFO,&fb_msg->finfo))
        {
                printf("Error: Reading fixed information \n");
                exit(2);
        }
        if (ioctl(fb_msg->fb,FBIOGET_VSCREENINFO,&fb_msg->vinfo))
        {
                printf("Error: Reading variable information \n");
                exit(3);
        }

        return TRUE;
}

int fb_mmap(FB *fb_msg)
{
        fb_msg->screensize=fb_msg->vinfo.xres * fb_msg->vinfo.yres * fb_msg->vinfo.bits_per_pixel/8;
        fb_msg->fbp=mmap(NULL,fb_msg->screensize,PROT_READ|PROT_WRITE,MAP_SHARED,fb_msg->fb,0);
        if (fb_msg->fbp ==(void *)-1)
        {
                printf("Error : failed to map framebuffer device to memory. \n");
               //printf("%d\n",fb_msg->fb);
               // printf("%d\n",fb_msg->screensize);
                exit(4);
        }
        return TRUE;
}

int fb_ummap(FB *fb_msg)
{
        munmap(fb_msg->fbp,fb_msg->screensize);
        close(fb_msg->fb);
}
void draw_picture(const unsigned int *image,FB *fb_msg)
{
        int x,y,count=0;
        long location=0;
        unsigned char *p=fb_msg->fbp;
        for(y=0;y<320;y++)
	{
		//p += (1280*4*y);
        for(x=0;x<240;x++)
       {
         *(p+4*x+1280*4*y+600*1024*4+600*4)= 0xff;
         *(p+4*x+1280*4*y+600*1024*4+600*4+1)= 0x00;
         *(p+4*x+1280*4*y+600*1024*4+600*4+2)= 0x00;
         *(p+4*x+1280*4*y+600*1024*4+600*4+3)= 0xff;
         
       }
	}
      
      /*    for(x=0;x<fb_msg->vinfo.xres;x++)
                for(y=fb_msg->vinfo.yres-1;y>=0;y--)
                {
       
       
                     location = x*(fb_msg->vinfo.bits_per_pixel / 8) + y* fb_msg->finfo.line_length;
                        *(fb_msg->fbp+location)=image[count];
                        *(fb_msg->fbp+location+1)=image[count+1];
                        count=count+2;
       
                }*/
}

void clear_screen(int color,FB *fb_msg)
{
        int x,y;
        long location=0;
        for(x=0;x<fb_msg->vinfo.xres;x++)
                for(y=fb_msg->vinfo.yres-1;y>=0;y--)
                {
                        location = x*(fb_msg->vinfo.bits_per_pixel / 8) + y* fb_msg->finfo.line_length;
                        *(fb_msg->fbp+location)=color;
                        *(fb_msg->fbp+location+1)=color;
                }

}
main()
{
        FB *fb;
        fb=(FB *)malloc(sizeof(FB));
        open_fb(fb);
        get_fb_msg(fb);
        printf("fb:%dx%d/%dbits\n",fb->vinfo.xres ,fb->vinfo.yres, fb->vinfo.bits_per_pixel);
        fb_mmap(fb);
        //clear_screen(0xff,fb);
//      draw_picture(image0,fb);
        draw_picture(image1,fb);
//      draw_picture(image,fb);
        fb_ummap(fb);
        free(fb);
}

