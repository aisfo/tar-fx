//
// Created by Asif Amirguliyev on 12/3/16.
//

#ifndef TARFX_COMMON_H
#define TARFX_COMMON_H


#include <string>
#include <math.h>
#include <cassert>
#include <android/log.h>
#include "../../../libs/kiss_fft/kiss_fft.h"

#define NUM_CHANNELS 1
#define SAMPLE_RATE 48000
#define FRAMES_PER_BUF 128
#define BITS_PER_BYTE 8
#define CIRCULAR_DURATION 10 //seconds

#define  LOG_TAG    "tar-fx"
#define  LOG(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

#endif //TARFX_COMMON_H
