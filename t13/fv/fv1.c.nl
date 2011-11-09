  1 /*
  2  * $Id: fv.c
  3  * $Desp: draw jpeg to framebuffer
  4  * $Author: rockins
  5  * $Date: Wed Jan  3 20:15:49 CST 2007
  6  */
  7 
  8 #include <stdio.h>
  9 #include <stdlib.h>
 10 #include <fcntl.h>
 11 #include <linux/fb.h>
 12 #include <sys/types.h>
 13 #include <sys/stat.h>
 14 #include <sys/mman.h>
 15 #include <jpeglib.h>
 16 #include <jerror.h>
 17 
 18 #define	FB_DEV	"/dev/fb0"
 19 
 20 /***************** function declaration ******************/
 21 void            usage(char *msg);
 22 unsigned short  RGB888toRGB565(unsigned char red,
 23 							   unsigned char green, unsigned char blue);
 24 int             fb_open(char *fb_device);
 25 int             fb_close(int fd);
 26 int             fb_stat(int fd, int *width, int *height, int *depth);
 27 void           *fb_mmap(int fd, unsigned int screensize);
 28 int             fb_munmap(void *start, size_t length);
 29 int             fb_pixel(void *fbmem, int width, int height,
 30 						 int x, int y, unsigned short color);
 31 
 32 /************ function implementation ********************/
 33 int
 34 main(int argc, char *argv[])
 35 {
 36 	/*
 37 	 * declaration for jpeg decompression
 38 	 */
 39 	struct jpeg_decompress_struct cinfo;
 40 	struct jpeg_error_mgr jerr;
 41 	FILE           *infile;
 42 	unsigned char  *buffer;
 43 
 44 	/*
 45 	 * declaration for framebuffer device
 46 	 */
 47 	int             fbdev;
 48 	char           *fb_device;
 49 	unsigned char  *fbmem;
 50 	unsigned int    screensize;
 51 	unsigned int    fb_width;
 52 	unsigned int    fb_height;
 53 	unsigned int    fb_depth;
 54 	unsigned int    x;
 55 	unsigned int    y;
 56 
 57 	/*
 58 	 * check auguments
 59 	 */
 60 	if (argc != 2) {
 61 		usage("insuffient auguments");
 62 		exit(-1);
 63 	}
 64 
 65 	/*
 66 	 * open framebuffer device
 67 	 */
 68 	if ((fb_device = getenv("FRAMEBUFFER")) == NULL)
 69 		fb_device = FB_DEV;
 70 	fbdev = fb_open(fb_device);
 71 
 72 	/*
 73 	 * get status of framebuffer device
 74 	 */
 75 	fb_stat(fbdev, &fb_width, &fb_height, &fb_depth);
 76 
 77 	/*
 78 	 * map framebuffer device to shared memory
 79 	 */
 80 	screensize = fb_width * fb_height;
 81 	fbmem = fb_mmap(fbdev, screensize);
 82 
 83 	/*
 84 	 * open input jpeg file 
 85 	 */
 86 	if ((infile = fopen(argv[1], "rb")) == NULL) {
 87 		fprintf(stderr, "open %s failed\n", argv[1]);
 88 		exit(-1);
 89 	}
 90 
 91 	/*
 92 	 * init jpeg decompress object error handler
 93 	 */
 94 	cinfo.err = jpeg_std_error(&jerr);
 95 	jpeg_create_decompress(&cinfo);
 96 
 97 	/*
 98 	 * bind jpeg decompress object to infile
 99 	 */
100 	jpeg_stdio_src(&cinfo, infile);
101 
102 
103 	/*
104 	 * read jpeg header
105 	 */
106 	jpeg_read_header(&cinfo, TRUE);
107 
108 	/*
109 	 * decompress process.
110 	 * note: after jpeg_start_decompress() is called
111 	 * the dimension infomation will be known,
112 	 * so allocate memory buffer for scanline immediately
113 	 */
114 	jpeg_start_decompress(&cinfo);
115 	if ((cinfo.output_width > fb_width) ||
116 		(cinfo.output_height > fb_height)) {
117 		printf("too large JPEG file,cannot display\n");
118 		return (-1);
119 	}
120 
121 	buffer = (unsigned char *) malloc(cinfo.output_width *
122 									  cinfo.output_components);
123 	y = 0;
124 	while (cinfo.output_scanline < cinfo.output_height) {
125 		jpeg_read_scanlines(&cinfo, &buffer, 1);
126 		if (fb_depth == 16) {
127 			unsigned short  color;
128 			for (x = 0; x < cinfo.output_width; x++) {
129 				color = RGB888toRGB565(buffer[x * 3], 
130 						buffer[x * 3 + 1], buffer[x * 3 + 2]);
131 				fb_pixel(fbmem, fb_width, fb_height, x, y, color);
132 			}
133 		} else if (fb_depth == 24) {
134 			memcpy((unsigned char *) fbmem + y * fb_width * 3,
135 				   buffer, cinfo.output_width * cinfo.output_components);
136 		}
137 		y++;					// next scanline
138 	}
139 
140 	/*
141 	 * finish decompress, destroy decompress object
142 	 */
143 	jpeg_finish_decompress(&cinfo);
144 	jpeg_destroy_decompress(&cinfo);
145 
146 	/*
147 	 * release memory buffer
148 	 */
149 	free(buffer);
150 
151 	/*
152 	 * close jpeg inputing file
153 	 */
154 	fclose(infile);
155 
156 	/*
157 	 * unmap framebuffer's shared memory
158 	 */
159 	fb_munmap(fbmem, screensize);
160 
161 	/*
162 	 * close framebuffer device
163 	 */
164 	fb_close(fbdev);
165 
166 	return (0);
167 }
168 
169 void
170 usage(char *msg)
171 {
172 	fprintf(stderr, "%s\n", msg);
173 	printf("Usage: fv some-jpeg-file.jpg\n");
174 }
175 
176 /*
177  * convert 24bit RGB888 to 16bit RGB565 color format
178  */
179 unsigned short
180 RGB888toRGB565(unsigned char red, unsigned char green, unsigned char blue)
181 {
182 	unsigned short  B = (blue >> 3) & 0x001F;
183 	unsigned short  G = ((green >> 3) << 5) & 0x07E0;
184 	unsigned short  R = ((red >> 3) << 11) & 0xF800;
185 
186 	return (unsigned short) (R | G | B);
187 }
188 
189 /*
190  * open framebuffer device.
191  * return positive file descriptor if success,
192  * else return -1.
193  */
194 int
195 fb_open(char *fb_device)
196 {
197 	int             fd;
198 
199 	if ((fd = open(fb_device, O_RDWR)) < 0) {
200 		perror(__func__);
201 		return (-1);
202 	}
203 	return (fd);
204 }
205 
206 /*
207  * get framebuffer's width,height,and depth.
208  * return 0 if success, else return -1.
209  */
210 int
211 fb_stat(int fd, int *width, int *height, int *depth)
212 {
213 	struct fb_fix_screeninfo fb_finfo;
214 	struct fb_var_screeninfo fb_vinfo;
215 
216 	if (ioctl(fd, FBIOGET_FSCREENINFO, &fb_finfo)) {
217 		perror(__func__);
218 		return (-1);
219 	}
220 
221 	if (ioctl(fd, FBIOGET_VSCREENINFO, &fb_vinfo)) {
222 		perror(__func__);
223 		return (-1);
224 	}
225 
226 	*width = fb_vinfo.xres;
227 	*height = fb_vinfo.yres;
228 	*depth = fb_vinfo.bits_per_pixel;
229 
230 	return (0);
231 }
232 
233 /*
234  * map shared memory to framebuffer device.
235  * return maped memory if success,
236  * else return -1, as mmap dose.
237  */
238 void *
239 fb_mmap(int fd, unsigned int screensize)
240 {
241 	void *	fbmem;
242 
243 	if ((fbmem = mmap(0, screensize, PROT_READ | PROT_WRITE,
244 					  MAP_SHARED, fd, 0)) == MAP_FAILED) {
245 		perror(__func__);
246 		return (void *) (-1);
247 	}
248 
249 	return (fbmem);
250 }
251 
252 /*
253  * unmap map memory for framebuffer device.
254  */
255 int
256 fb_munmap(void *start, size_t length)
257 {
258 	return (munmap(start, length));
259 }
260 
261 /*
262  * close framebuffer device
263  */
264 int
265 fb_close(int fd)
266 {
267 	return (close(fd));
268 }
269 
270 /*
271  * display a pixel on the framebuffer device.
272  * fbmem is the starting memory of framebuffer,
273  * width and height are dimension of framebuffer,
274  * x and y are the coordinates to display,
275  * color is the pixel's color value.
276  * return 0 if success, otherwise return -1.
277  */
278 int
279 fb_pixel(void *fbmem, int width, int height,
280 		 int x, int y, unsigned short color)
281 {
282 	if ((x > width) || (y > height))
283 		return (-1);
284 
285 	unsigned short *dst = ((unsigned short *) fbmem + y * width + x);
286 
287 	*dst = color;
288 	return (0);
289 }
