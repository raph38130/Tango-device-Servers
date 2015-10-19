//TODO try 2 external webcam1 & webcam2

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <signal.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <linux/videodev2.h>
#include <jpeglib.h>

#define CLEAR(x) memset (&(x), 0, sizeof (x))
 

// global settings
 //unsigned int width = 324;//640;
 //unsigned int height = 240;//480;
 unsigned char jpegQuality = 70;
 char* jpegFilename = NULL;
 int camnum=0;
 //extern char *deviceName;


/**
  Convert from YUV422 format to RGB888. Formulae are described on http://en.wikipedia.org/wiki/YUV

  \param width width of image
  \param height height of image
  \param src source
  \param dst destination
*/
//extern unsigned char* timg;
 void YUV422toRGB888(int width, int height, unsigned char *src, unsigned char *dst)
{
  int line, column;
  unsigned char *py, *pu, *pv;
  unsigned char *tmp = dst;

  /* In this format each four bytes is two pixels. Each four bytes is two Y's, a Cb and a Cr. 
     Each Y goes to one of the pixels, and the Cb and Cr belong to both pixels. */
  py = src;
  pu = src + 1;
  pv = src + 3;

  #define CLIP(x) ( (x)>=0xFF ? 0xFF : ( (x) <= 0x00 ? 0x00 : (x) ) )

  for (line = 0; line < height; ++line) {
    for (column = 0; column < width; ++column) {
	
      *tmp++ = CLIP((double)*py + 1.402*((double)*pv-128.0));
//tmp++;tmp++;
      //*tmp++ = CLIP((double)*py - 0.344*((double)*pu-128.0) - 0.714*((double)*pv-128.0));      
      //*tmp++ = CLIP((double)*py + 1.772*((double)*pu-128.0));

      // increase py every time
      py += 2;
      // increase pu,pv every second time
      if ((column & 1)==1) {
        pu += 4;
        pv += 4;
      }
    }
  }
}

/**
  Print error message and terminate programm with EXIT_FAILURE return code.
  \param s error message to print
*/
 void errno_exit(const char* s)
{
  fprintf(stderr, "%s error %d, %s\n", s, errno, strerror (errno));
  exit(EXIT_FAILURE);
}

/**
  Do ioctl and retry if error was EINTR ("A signal was caught during the ioctl() operation."). Parameters are the same as on ioctl.

  \param fd file descriptor
  \param request request
  \param argp argument
  \returns result from ioctl
*/
 int xioctl(int fd, int request, void* argp)
{
  int r;

  do r = ioctl(fd, request, argp);
  while (-1 == r && EINTR == errno);

  return r;
}



/**
  process image read
*/
 void imageProcess(const void* p, unsigned char *timg, unsigned int & width, unsigned int & height)
{
  unsigned char* src = (unsigned char*)p;
  
  YUV422toRGB888(width,height,src,timg);
  
}

/**
  read single frame
*/

int frameRead(int &fd,UsbCam_ns::buffer* &buffers,unsigned int &n_buffers, unsigned char*timg, unsigned int & width, unsigned int & height)
{
  struct v4l2_buffer buf;

  unsigned int i;

      CLEAR (buf);

      buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf.memory = V4L2_MEMORY_USERPTR;

      if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf)) {
        switch (errno) {
          case EAGAIN:
            return 0;

          case EIO:
            // Could ignore EIO, see spec.

            // fall through
          default:
            errno_exit("VIDIOC_DQBUF");
                        
        }
      }

      for (i = 0; i < n_buffers; ++i)
        if (buf.m.userptr == (unsigned long) buffers[i].start 
	 && buf.length == buffers[i].length)
          break;

      assert (i < n_buffers);

      imageProcess((void *) buf.m.userptr,timg,width,height);

      if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
        errno_exit("VIDIOC_QBUF");

  return 1;
}

/** 
  mainloop: read frames and process them
*/
void mainLoop(char *deviceName, int &fd,UsbCam_ns::buffer* &buffers, unsigned int &n_buffers,unsigned char*timg, unsigned int & width, unsigned int & height)
{
  unsigned int count;
  
    for (;;) {
      fd_set fds;
       //struct timespec tv;
     struct timeval tv;
      int r;
     //	sigset_t mask;

      FD_ZERO(&fds);
      FD_SET(fd, &fds);
//printf("mainloop %d %s %d %x %x\n",fd, deviceName,n_buffers,buffers,fds);

      tv.tv_sec = 1;
      tv.tv_usec = 0;
//printf("before select %d %s %d %x %x",fd, deviceName,n_buffers,buffers,fds);
//printf(" thread = %d \n",(int)syscall(SYS_gettid));
       //r = pselect(fd + 1, &fds, NULL, NULL, &tv,&mask);
      r = select(fd + 1, &fds, NULL, NULL, &tv);
//r=1;
//printf("after select %d %s %d %x %x\n",fd, deviceName,n_buffers,buffers,fds);

      if (-1 == r) {
        if (EINTR == errno)
          continue;
//printf("select error1 %d %s %d %x %x\n",fd, deviceName,n_buffers,buffers,fds);
        errno_exit("select");
      }
      if (0 == r) {
        fprintf (stderr, "select timeout\n");
//printf("select error2 %d %s %d %x %x\n",fd, deviceName,n_buffers,buffers,fds);
        exit(EXIT_FAILURE);
      }
      if (frameRead(fd,buffers,n_buffers,timg,width,height))      break;
    }
}

/**
  stop capturing
*/
void captureStop(int fd)
{
  enum v4l2_buf_type type;
      type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      if (-1 == xioctl(fd, VIDIOC_STREAMOFF, &type))
        errno_exit("VIDIOC_STREAMOFF");
}

/**
  start capturing
*/
void captureStart(int &fd,UsbCam_ns::buffer* &buffers,unsigned int &n_buffers)
{
  unsigned int i;
  enum v4l2_buf_type type;

      for (i = 0; i < n_buffers; ++i) {
        struct v4l2_buffer buf;

        CLEAR (buf);
        buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory      = V4L2_MEMORY_USERPTR;
        buf.index       = i;
        buf.m.userptr   = (unsigned long) buffers[i].start;
        buf.length      = buffers[i].length;
        if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
          errno_exit("VIDIOC_QBUF");
      }
      type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))
         errno_exit("VIDIOC_STREAMON");
}

 void deviceUninit(UsbCam_ns::buffer* buffers,unsigned int n_buffers)
{
  unsigned int i;

      for (i = 0; i < n_buffers; ++i)
        free (buffers[i].start);
  free(buffers);
}


void userptrInit(unsigned int &buffer_size,int &fd, char*deviceName,UsbCam_ns::buffer* &buffers,unsigned int & n_buffers)
{
  struct v4l2_requestbuffers req;
  unsigned int page_size;

  page_size = getpagesize ();
  buffer_size = (buffer_size + page_size - 1) & ~(page_size - 1);

  CLEAR(req);

  req.count               = 4;
  req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory              = V4L2_MEMORY_USERPTR;

  if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) {
    if (EINVAL == errno) {
      fprintf(stderr, "%s does not support user pointer i/o\n", deviceName);
      exit(EXIT_FAILURE);
    } else {
      errno_exit("VIDIOC_REQBUFS");
    }
  } 

  buffers = (UsbCam_ns::buffer*)calloc(4, sizeof (*buffers));
  if (!buffers) {
    fprintf(stderr, "Out of memory\n");
    exit(EXIT_FAILURE);
  }

  for (n_buffers = 0; n_buffers < 4; ++n_buffers) {
    buffers[n_buffers].length = buffer_size;
    buffers[n_buffers].start = memalign (/* boundary */ page_size, buffer_size);

    if (!buffers[n_buffers].start) {
      fprintf(stderr, "Out of memory\n");
      exit(EXIT_FAILURE);
    }
  }
//printf("calloc %x %d\n",buffers,n_buffers);
}


/**
  initialize device
*/
void deviceInit(int &fd,char*deviceName,UsbCam_ns::buffer* & buffers,unsigned int & n_buffers,unsigned int&width,unsigned int&height)
{
  struct v4l2_capability cap;
  struct v4l2_cropcap cropcap;
  struct v4l2_crop crop;
  struct v4l2_format fmt;
  unsigned int min;

  if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap)) {
    if (EINVAL == errno) {
      fprintf(stderr, "%s is no V4L2 device\n",deviceName);
      exit(EXIT_FAILURE);
    } else {
      errno_exit("VIDIOC_QUERYCAP");
    }
  }

  if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
    fprintf(stderr, "%s is no video capture device\n",deviceName);
    exit(EXIT_FAILURE);
    }
  if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
        fprintf(stderr, "%s does not support streaming i/o\n",deviceName);
        exit(EXIT_FAILURE);
  	}


  /* Select video input, video standard and tune here. */
  CLEAR(cropcap);

  cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  if (0 == xioctl(fd, VIDIOC_CROPCAP, &cropcap)) {
    crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    crop.c = cropcap.defrect; /* reset to default */

    if (-1 == xioctl(fd, VIDIOC_S_CROP, &crop)) {
      switch (errno) {
        case EINVAL:
          /* Cropping not supported. */
          break;
        default:
          /* Errors ignored. */
          break;
      }
    }
  } else {        
    /* Errors ignored. */
  }

  CLEAR (fmt);

  // v4l2_format
  fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmt.fmt.pix.width       = width; 
  fmt.fmt.pix.height      = height;
  fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
  fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

  if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt))
    errno_exit("VIDIOC_S_FMT");

  /* Note VIDIOC_S_FMT may change width and height. */
  if (width != fmt.fmt.pix.width) {
    width = fmt.fmt.pix.width;
    fprintf(stderr,"Image width set to %i by device %s.\n",width,deviceName);
  }
  if (height != fmt.fmt.pix.height) {
    height = fmt.fmt.pix.height;
    fprintf(stderr,"Image height set to %i by device %s.\n",height,deviceName);
  }

  /* Buggy driver paranoia. */
  min = fmt.fmt.pix.width * 2;
  if (fmt.fmt.pix.bytesperline < min)
    fmt.fmt.pix.bytesperline = min;
  min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
  if (fmt.fmt.pix.sizeimage < min)
    fmt.fmt.pix.sizeimage = min;

  userptrInit(fmt.fmt.pix.sizeimage,fd,deviceName,buffers,n_buffers);
}

/**
  close device
*/
 void deviceClose(int &fd)
{
  if (-1 == close (fd))
    errno_exit("close");

  fd = -1;
}

/**
  open device
*/
 void deviceOpen(char*deviceName, int &fd, unsigned int & width, unsigned int & height)
{
struct stat st;


if (camnum==0)
	{strcpy(deviceName,"/dev/video0");
	width=176,height=144;
	printf("deviceopen %s pid %d %d\n",deviceName,getpid(), (int)syscall(SYS_gettid));}
else if (camnum==1)
	{strcpy(deviceName,"/dev/video1");
	width=162,height=120;
	//width=324,height=240;
	printf("deviceopen %s pid %d %d\n",deviceName,getpid(),(int)syscall(SYS_gettid));}
else
	{strcpy(deviceName,"/dev/video2");
	width=162,height=120;
	printf("deviceopen %s pid %d %d\n",deviceName,getpid(),(int)syscall(SYS_gettid));}

camnum++;
  // stat file

  if (-1 == stat(deviceName, &st)) {
    fprintf(stderr, "Cannot identify '%s': %d, %s\n", deviceName, errno, strerror (errno));
    exit(EXIT_FAILURE);
  }

  // check if its device
  if (!S_ISCHR (st.st_mode)) {
    fprintf(stderr, "%s is no device\n", deviceName);
    exit(EXIT_FAILURE);
  }

  // open device
  fd = open(deviceName, O_RDWR /* required */ | O_NONBLOCK, 0);

  // check if opening was successfull
  if (-1 == fd) {
    fprintf(stderr, "Cannot open '%s': %d, %s\n", deviceName, errno, strerror (errno));
    exit(EXIT_FAILURE);
  }
}


