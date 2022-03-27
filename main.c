/*
 * libmad - MPEG audio decoder library
 * Copyright (C) 2000-2004 Underbit Technologies, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id: minimad.c,v 1.4 2004/01/23 09:41:32 rob Exp $
 */

# include <stdio.h>
# include <unistd.h>
# include <sys/stat.h>
# include <sys/mman.h>

# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <stdlib.h>

# include "mad.h"


#ifdef ENABLE_DEBUG
    #define DEBUG(fmt, args...)     printf(fmt, ##args)
#else
    #define DEBUG(fmt, args...)
#endif

/*
 * This is a private message structure. A generic pointer to this structure
 * is passed to each of the callback functions. Put here any data you need
 * to access from within the callbacks.
 */

struct buffer {
  unsigned char const *inMp3Data;
  unsigned long inMp3DataLen;
  unsigned char *outPcmData;
  unsigned long outPcmDataLen;
};

/*
 * This is perhaps the simplest example use of the MAD high-level API.
 * Standard input is mapped into memory via mmap(), then the high-level API
 * is invoked with three callbacks: input, output, and error. The output
 * callback converts MAD's high-resolution PCM samples to 16 bits, then
 * writes them to standard output in little-endian, stereo-interleaved
 * format.
 */

static int mp3_decode_body(struct buffer *);

int main(int argc, char *argv[])
{
  struct stat stat;
  int fdMp3 = -1;
  void *vfdMp3 = NULL; /* mmap */
  FILE *fpPcm = NULL;
  char *inMp3FileName = NULL;
  char *outPcmFileName = NULL;
  struct buffer bufferInfo= {};

  if (argc != 3)
  {
	printf("Usage: \n"
           "    %s <in MP3 file> <out PCM file>\n"
           "Examples: \n"
           "    %s audio/test1_44100_stereo.mp3 out1_44100_16bit_stereo.pcm\n"
           "    %s audio/test2_22050_stereo.mp3 out2_22050_16bit_stereo.pcm\n"
           "    %s audio/test3_22050_mono.mp3   out3_22050_16bit_mono.pcm\n"
           "    %s audio/test4_8000_mono.mp3    out4_8000_16bit_mono.pcm\n",
		   argv[0], argv[0], argv[0], argv[0], argv[0]);

    return -1;
  }
  else
  {
    inMp3FileName = argv[1];
    outPcmFileName = argv[2];
  }

  /* open MP3 file and map to memory */
  fdMp3 = open(inMp3FileName, O_RDONLY);
  if (fdMp3 < 0)
  {
    perror("open input MP3 file failed");
    goto exit;
  }

  if (fstat(fdMp3/*STDIN_FILENO*/, &stat) == -1 ||
      stat.st_size == 0)
    goto exit;

  printf("decode input MP3 size: %lu\n", stat.st_size);

  vfdMp3 = mmap(0, stat.st_size, PROT_READ, MAP_SHARED, fdMp3/*STDIN_FILENO*/, 0);
  if (vfdMp3 == MAP_FAILED)
  {
    printf("map MP3 file to memory failed!\n");
    goto exit;
  }

  /* fix up our struct, and we will use to decode mp3 data! */
  bufferInfo.inMp3Data = vfdMp3;
  bufferInfo.inMp3DataLen = stat.st_size;
  bufferInfo.outPcmData = malloc(stat.st_size * 15); /* decode out buf size */
  if (!bufferInfo.outPcmData)
  {
    printf("alloc memory to decode output PCM failed!\n");
    goto exit;
  }
  bufferInfo.outPcmDataLen = 0; /* init to 0 */

  /* decode MP3 data with our struct !!!!! */
  mp3_decode_body(&bufferInfo);

  /* save the decoded out PCM data */
  fpPcm = fopen(outPcmFileName, "wb");
  if (!fpPcm)
  {
    printf("open output PCM file failed!\n");
    goto exit;
  }
  else
  {
    printf("decode output total PCM size: %lu\n", bufferInfo.outPcmDataLen);
    fwrite(bufferInfo.outPcmData, 1, bufferInfo.outPcmDataLen, fpPcm);
    fflush(fpPcm);
    fclose(fpPcm);
  }

exit:

  if (munmap(vfdMp3, stat.st_size) == -1)
    return -1;

  if (fdMp3)
    close(fdMp3);

  if (bufferInfo.outPcmData)
    free(bufferInfo.outPcmData);

  return 0;
}

/*
 * This is the input callback. The purpose of this callback is to (re)fill
 * the stream buffer which is to be decoded. In this example, an entire file
 * has been mapped into memory, so we just call mad_stream_buffer() with the
 * address and length of the mapping. When this callback is called a second
 * time, we are finished decoding.
 */

static
enum mad_flow mp3_decode_input(void *data,
		    struct mad_stream *stream)
{
  struct buffer *buffer = data;

  if (!buffer->inMp3DataLen)
    return MAD_FLOW_STOP;

  DEBUG("[%s: %d] decode input size: %lu\n", __FUNCTION__, __LINE__, buffer->inMp3DataLen);

  mad_stream_buffer(stream, buffer->inMp3Data, buffer->inMp3DataLen);

  buffer->inMp3DataLen = 0;

  return MAD_FLOW_CONTINUE;
}

/*
 * The following utility routine performs simple rounding, clipping, and
 * scaling of MAD's high-resolution samples down to 16 bits. It does not
 * perform any dithering or noise shaping, which would be recommended to
 * obtain any exceptional audio quality. It is therefore not recommended to
 * use this routine if high-quality output is desired.
 */

static inline
signed int scale(mad_fixed_t sample)
{
  /* round */
  sample += (1L << (MAD_F_FRACBITS - 16));

  /* clip */
  if (sample >= MAD_F_ONE)
    sample = MAD_F_ONE - 1;
  else if (sample < -MAD_F_ONE)
    sample = -MAD_F_ONE;

  /* quantize */
  return sample >> (MAD_F_FRACBITS + 1 - 16);
}

/*
 * This is the output callback function. It is called after each frame of
 * MPEG audio data has been completely decoded. The purpose of this callback
 * is to output (or play) the decoded PCM audio.
 */

static
enum mad_flow mp3_decode_output(void *data,
		     struct mad_header const *header,
		     struct mad_pcm *pcm)
{
  struct buffer *buffer = data;
  unsigned int nchannels, nsamples;
  mad_fixed_t const *left_ch, *right_ch;

  /* pcm->samplerate contains the sampling frequency */

  nchannels = pcm->channels;
  nsamples  = pcm->length;
  left_ch   = pcm->samples[0];
  right_ch  = pcm->samples[1];

  DEBUG("[%s: %d] decode ouput size: %d\n", __FUNCTION__, __LINE__, nsamples*2*nchannels/* print as 16bit */);

  while (nsamples--) {
    signed int sample;

    /* output sample(s) in 16-bit signed little-endian PCM */

    sample = scale(*left_ch++);
    #if 0
    putchar((sample >> 0) & 0xff);
    putchar((sample >> 8) & 0xff);
    #else
    buffer->outPcmData[buffer->outPcmDataLen++] = (sample >> 0) & 0xff;
    buffer->outPcmData[buffer->outPcmDataLen++] = (sample >> 8) & 0xff;
    #endif

    if (nchannels == 2) {
      sample = scale(*right_ch++);
      #if 0
      putchar((sample >> 0) & 0xff);
      putchar((sample >> 8) & 0xff);
      #else
      buffer->outPcmData[buffer->outPcmDataLen++] = (sample >> 0) & 0xff;
      buffer->outPcmData[buffer->outPcmDataLen++] = (sample >> 8) & 0xff;
      #endif
    }
  }

  return MAD_FLOW_CONTINUE;
}

/*
 * This is the error callback function. It is called whenever a decoding
 * error occurs. The error is indicated by stream->error; the list of
 * possible MAD_ERROR_* errors can be found in the mad.h (or stream.h)
 * header file.
 */

static
enum mad_flow mp3_decode_error(void *data,
		    struct mad_stream *stream,
		    struct mad_frame *frame)
{
  struct buffer *buffer = data;

  fprintf(stderr, "decoding error 0x%04x (%s) at byte offset %lu\n",
	  stream->error, mad_stream_errorstr(stream),
	  stream->this_frame - buffer->inMp3Data);

  /* return MAD_FLOW_BREAK here to stop decoding (and propagate an error) */

  return MAD_FLOW_CONTINUE;
}

/*
 * This is the function called by main() above to perform all the decoding.
 * It instantiates a decoder object and configures it with the input,
 * output, and error callback functions above. A single call to
 * mad_decoder_run() continues until a callback function returns
 * MAD_FLOW_STOP (to stop decoding) or MAD_FLOW_BREAK (to stop decoding and
 * signal an error).
 */

static
int mp3_decode_body(struct buffer *bufferInfo)
{
  //struct buffer buffer;
  struct mad_decoder decoder;
  int result;

  #if 0
  /* initialize our private message structure */

  buffer.start  = start;
  buffer.length = length;
  #endif

  /* configure input, output, and error functions */

  mad_decoder_init(&decoder, bufferInfo,
		   mp3_decode_input, 0 /* header */, 0 /* filter */, mp3_decode_output,
		   mp3_decode_error, 0 /* message */);

  /* start decoding */

  result = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);

  /* release the decoder */

  mad_decoder_finish(&decoder);

  return result;
}
