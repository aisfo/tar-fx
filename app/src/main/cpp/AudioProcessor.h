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
#include <cassert>

#define NUM_CHANNELS 1
#define SAMPLE_RATE 48000
#define FRAMES_PER_BUF 240
#define BITS_PER_BYTE 8


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

int bufSize;
int bufferIndex;
short *buffer_1;
short *buffer_2;



#endif //TARFX_AUDIOPROCESSOR_H
