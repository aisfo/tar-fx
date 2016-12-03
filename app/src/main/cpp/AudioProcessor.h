//
// Created by Asif Amirguliyev on 11/20/16.
//

#ifndef TARFX_AUDIOPROCESSOR_H
#define TARFX_AUDIOPROCESSOR_H

#include <jni.h>
#include <string>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <android/log.h>
#include "math.h"
#include <cassert>

#define NUM_CHANNELS 1
#define SAMPLE_RATE 48000
#define FRAMES_PER_BUF 128
#define BITS_PER_BYTE 8
#define CIRCULAR_DURATION 10 //seconds


#define  LOG_TAG    "magic"
#define  LOG(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)

#define ASSERT(X) assert(X == SL_RESULT_SUCCESS)
#define ASSERT_NOT_NULL(X) assert(X != NULL)


SLObjectItf engine_obj;
SLEngineItf engine;

SLObjectItf output_mix_obj;

SLObjectItf player_obj;
SLPlayItf player;

SLObjectItf recorderObject;
SLRecordItf recorder;

SLAndroidSimpleBufferQueueItf player_buf_q;
SLAndroidSimpleBufferQueueItf recorder_buf_q;

bool start_playback;
long write_head;
long read_head;

int frame_width = FRAMES_PER_BUF * NUM_CHANNELS;
int frame_size = frame_width * sizeof(short);
long circular_size = frame_width * (SAMPLE_RATE/frame_width) * CIRCULAR_DURATION;
int bufSize = frame_width * SL_PCMSAMPLEFORMAT_FIXED_16 / BITS_PER_BYTE;

short * circular_buffer;
short * buffer_in;
short * buffer_out;



#endif //TARFX_AUDIOPROCESSOR_H
