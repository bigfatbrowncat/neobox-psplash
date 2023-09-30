/*
 *  pslash - a lightweight framebuffer splashscreen for embedded devices. 
 *
 *  Copyright (c) 2006 Matthew Allum <mallum@o-hand.com>
 *
 *  Parts of this file ( fifo handling ) based on 'usplash' copyright 
 *  Matthew Garret.
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include "psplash.h"
#include "psplash-config.h"
#include "psplash-colors.h"

#include "neobox-320x480-img.h"
#include "neobox-320x480-20pc-img.h"
#include "neobox-320x480-40pc-img.h"
#include "neobox-320x480-60pc-img.h"
#include "neobox-320x480-80pc-img.h"
#include "neobox-320x480-gray-img.h"
#include "psplash-bar-img.h"

#ifdef HAVE_SYSTEMD
#include <systemd/sd-daemon.h>
#endif

#include FONT_HEADER

#define SPLIT_LINE_POS(fb)                                  \
	(  (fb)->height                                     \
	 - ((  PSPLASH_IMG_SPLIT_DENOMINATOR                \
	     - PSPLASH_IMG_SPLIT_NUMERATOR)                 \
	    * (fb)->height / PSPLASH_IMG_SPLIT_DENOMINATOR) \
	)

void
psplash_exit (int UNUSED(signum))
{
  DBG("mark");

  psplash_console_reset ();
}

void
psplash_draw_msg (PSplashFB *fb, const char *msg)
{
  int w, h;

  psplash_fb_text_size (&w, &h, &FONT_DEF, msg);

  DBG("displaying '%s' %ix%i\n", msg, w, h);

  /* Clear */

  psplash_fb_draw_rect (fb, 
			0, 
			SPLIT_LINE_POS(fb) - h, 
			fb->width,
			h,
			PSPLASH_BACKGROUND_COLOR);

  psplash_fb_draw_text (fb,
			(fb->width-w)/2, 
			SPLIT_LINE_POS(fb) - h,
			PSPLASH_TEXT_COLOR,
			&FONT_DEF,
			msg);
}

#ifdef PSPLASH_SHOW_PROGRESS_BAR
void
psplash_draw_progress (PSplashFB *fb, int value)
{
  int x, y, width, height, barwidth;

  /* 4 pix border */
  x      = ((fb->width  - BAR_IMG_WIDTH)/2) + 4 ;
  y      = SPLIT_LINE_POS(fb) + 4;
  width  = BAR_IMG_WIDTH - 8;
  height = BAR_IMG_HEIGHT - 8;


  if (value > 0)
    {


      barwidth = (CLAMP(value,0,100) * width) / 100;
      psplash_fb_draw_rect (fb, x + barwidth, y,
    			width - barwidth, height,
			PSPLASH_BAR_BACKGROUND_COLOR);
      psplash_fb_draw_rect (fb, x, y, barwidth,
			    height, PSPLASH_BAR_COLOR);
    }
  else
    {
      barwidth = (CLAMP(-value,0,100) * width) / 100;
      psplash_fb_draw_rect (fb, x, y,
    			width - barwidth, height,
			PSPLASH_BAR_BACKGROUND_COLOR);
      psplash_fb_draw_rect (fb, x + width - barwidth,
			    y, barwidth, height,
			    PSPLASH_BAR_COLOR);
    }

  DBG("value: %i, width: %i, barwidth :%i\n", value,
		width, barwidth);
}
#endif /* PSPLASH_SHOW_PROGRESS_BAR */

// The frame contents. Progress and message
char* message_start = PSPLASH_STARTUP_MSG;
char* message;
int progress = 0;

struct frame {
    int img_stride;
    int img_width;
    int img_height;
    int img_bpp;
    uint8* img_data;
};

struct frame frames[6] = {
        {
                NEOBOX_GRAY_IMG_ROWSTRIDE,
                NEOBOX_GRAY_IMG_WIDTH,
                NEOBOX_GRAY_IMG_HEIGHT,
                NEOBOX_GRAY_IMG_BYTES_PER_PIXEL,
                NEOBOX_GRAY_IMG_RLE_PIXEL_DATA
        },
        {
                NEOBOX_20PC_IMG_ROWSTRIDE,
                NEOBOX_20PC_IMG_WIDTH,
                NEOBOX_20PC_IMG_HEIGHT,
                NEOBOX_20PC_IMG_BYTES_PER_PIXEL,
                NEOBOX_20PC_IMG_RLE_PIXEL_DATA
        },
        {
                NEOBOX_40PC_IMG_ROWSTRIDE,
                NEOBOX_40PC_IMG_WIDTH,
                NEOBOX_40PC_IMG_HEIGHT,
                NEOBOX_40PC_IMG_BYTES_PER_PIXEL,
                NEOBOX_40PC_IMG_RLE_PIXEL_DATA
        },
        {
                NEOBOX_60PC_IMG_ROWSTRIDE,
                NEOBOX_60PC_IMG_WIDTH,
                NEOBOX_60PC_IMG_HEIGHT,
                NEOBOX_60PC_IMG_BYTES_PER_PIXEL,
                NEOBOX_60PC_IMG_RLE_PIXEL_DATA
        },
        {
                NEOBOX_80PC_IMG_ROWSTRIDE,
                NEOBOX_80PC_IMG_WIDTH,
                NEOBOX_80PC_IMG_HEIGHT,
                NEOBOX_80PC_IMG_BYTES_PER_PIXEL,
                NEOBOX_80PC_IMG_RLE_PIXEL_DATA
        },
        {
                NEOBOX_IMG_ROWSTRIDE,
                NEOBOX_IMG_WIDTH,
                NEOBOX_IMG_HEIGHT,
                NEOBOX_IMG_BYTES_PER_PIXEL,
                NEOBOX_IMG_RLE_PIXEL_DATA
        }
};

void draw_frame(PSplashFB *fb) {

    int frame_index = progress / 16;
    if (frame_index < 0) frame_index = 0;
    if (frame_index > 5) frame_index = 5;

    /* Clear the background with #ecece1 */
    //psplash_fb_draw_rect (fb, 0, 0, fb->width, fb->height,
    //                      PSPLASH_BACKGROUND_COLOR);

    /* Draw the Neobox logo  */
    psplash_fb_draw_image (fb,
                           (fb->width  - frames[frame_index].img_width)/2,
#if PSPLASH_IMG_FULLSCREEN
                           (fb->height - frames[frame_index].img_height)/2,
#else
            (fb->height * PSPLASH_IMG_SPLIT_NUMERATOR
              / PSPLASH_IMG_SPLIT_DENOMINATOR - frames[frame_index].img_height)/2,
#endif
                           frames[frame_index].img_width,
                           frames[frame_index]. img_height,
                           frames[frame_index].img_bpp,
                           frames[frame_index].img_stride,
                           frames[frame_index].img_data);

#ifdef PSPLASH_SHOW_PROGRESS_BAR
    /* Draw progress bar border */
  psplash_fb_draw_image (fb,
			 (fb->width  - BAR_IMG_WIDTH)/2,
			 SPLIT_LINE_POS(fb),
			 BAR_IMG_WIDTH,
			 BAR_IMG_HEIGHT,
			 BAR_IMG_BYTES_PER_PIXEL,
			 BAR_IMG_ROWSTRIDE,
			 BAR_IMG_RLE_PIXEL_DATA);
#endif

#ifdef PSPLASH_SHOW_PROGRESS_BAR
    psplash_draw_progress (fb, progress);
#endif

    psplash_draw_msg (fb, message);

//    /* Scene set so let's flip the buffers. */
//    /* The first time we also synchronize the buffers so we can build on an
//     * existing scene. After the first scene is set in both buffers, only the
//     * text and progress bar change which overwrite the specific areas with every
//     * update.
//     */
    psplash_fb_flip(fb, 1);

}


static int 
parse_command (PSplashFB *fb, char *string)
{
  char *command;

  DBG("got cmd %s", string);
	
  if (strcmp(string,"QUIT") == 0)
    return 1;

  command = strtok(string," ");

  if (!strcmp(command,"MSG"))
    {
      char *arg = strtok(NULL, "\0");

      if (arg) {
          if (message != message_start) { free(message); }
          message = strdup(arg);
      }
    }
  else  if (!strcmp(command,"PROGRESS"))
    {
      char *arg = strtok(NULL, "\0");
        if (arg) {
            progress = atoi(arg);
        }
    }
  else if (!strcmp(command,"QUIT"))
    {
      return 1;
    }

    draw_frame(fb);


  return 0;
}


void 
psplash_main (PSplashFB *fb, int pipe_fd, int timeout) 
{
  int            err;
  ssize_t        length = 0;
  fd_set         descriptors;
  struct timeval tv;
  char          *end;
  char          *cmd;
  char           command[2048];

  tv.tv_sec = timeout;
  tv.tv_usec = 0;


  FD_ZERO(&descriptors);
  FD_SET(pipe_fd, &descriptors);

  end = command;

  while (1)
  {
      if (timeout != 0) err = select(pipe_fd+1, &descriptors, NULL, NULL, &tv);
      else err = select(pipe_fd+1, &descriptors, NULL, NULL, NULL);

      if (err <= 0) {
          /*
          if (errno == EINTR)
            continue;
          */
          return;
      }


//      if (elapsed_usec > 1000000 /* 1000 ms */ && frame_index == 0 ) {
//          frame_index = 1;
//          draw_frame(fb, frame_index);
//      }

      length += read (pipe_fd, end, sizeof(command) - (end - command));

      if (length == 0) 
	{
	  /* Reopen to see if there's anything more for us */
	  close(pipe_fd);
	  pipe_fd = open(PSPLASH_FIFO,O_RDONLY|O_NONBLOCK);
	  goto out;
	}

      cmd = command;
      do {
	int cmdlen;
        char *cmdend = memchr(cmd, '\n', length);

        /* Replace newlines with string termination */
        if (cmdend)
            *cmdend = '\0';

        cmdlen = strnlen(cmd, length);

        /* Skip string terminations */
	if (!cmdlen && length)
          {
            length--;
            cmd++;
	    continue;
          }

	if (parse_command(fb, cmd))
	  return;

	length -= cmdlen;
	cmd += cmdlen;
      } while (length);

    out:
      end = &command[length];
    
      tv.tv_sec = timeout;
      tv.tv_usec = 0;
      
      FD_ZERO(&descriptors);
      FD_SET(pipe_fd,&descriptors);
    }

  return;
}

int 
main (int argc, char** argv) 
{
  char      *rundir;
  int        pipe_fd, i = 0, angle = 0, fbdev_id = 0, ret = 0;
  PSplashFB *fb;
  bool       disable_console_switch = FALSE;

  signal(SIGHUP, psplash_exit);
  signal(SIGINT, psplash_exit);
  signal(SIGQUIT, psplash_exit);

  while (++i < argc) {
    if (!strcmp(argv[i],"-n") || !strcmp(argv[i],"--no-console-switch"))
      {
        disable_console_switch = TRUE;
        continue;
      }

    if (!strcmp(argv[i],"-a") || !strcmp(argv[i],"--angle"))
      {
        if (++i >= argc) goto fail;
        angle = atoi(argv[i]);
        continue;
      }

    if (!strcmp(argv[i],"-f") || !strcmp(argv[i],"--fbdev"))
      {
        if (++i >= argc) goto fail;
        fbdev_id = atoi(argv[i]);
        continue;
      }

    fail:
      fprintf(stderr, 
              "Usage: %s [-n|--no-console-switch][-a|--angle <0|90|180|270>][-f|--fbdev <0..9>]\n", 
              argv[0]);
      exit(-1);
  }

  rundir = getenv("PSPLASH_FIFO_DIR");

  if (!rundir)
    rundir = "/run";

  chdir(rundir);

  if (mkfifo(PSPLASH_FIFO, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP))
    {
      if (errno!=EEXIST) 
	    {
	      perror("mkfifo");
	      exit(-1);
	    }
    }

  pipe_fd = open (PSPLASH_FIFO,O_RDONLY|O_NONBLOCK);
  
  if (pipe_fd==-1) 
    {
      perror("pipe open");
      exit(-2);
    }

  if (!disable_console_switch)
    psplash_console_switch ();

  if ((fb = psplash_fb_new(angle,fbdev_id)) == NULL)
    {
	  ret = -1;
	  goto fb_fail;
    }

#ifdef HAVE_SYSTEMD
  sd_notify(0, "READY=1");
#endif

    message = message_start;

  /* Draw the initial frame  */
    draw_frame(fb);




  psplash_main (fb, pipe_fd, 0);

  psplash_fb_destroy (fb);

 fb_fail:
  unlink(PSPLASH_FIFO);

  if (!disable_console_switch)
    psplash_console_reset ();

  return ret;
}
