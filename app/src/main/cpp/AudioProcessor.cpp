#include "AudioProcessor.h"
#include <math.h>
#include "../../../libs/kiss_fft/kiss_fft.h"
#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds

double * lfo;

static void record_callback(SLAndroidSimpleBufferQueueItf bq, void *context)
{
    memcpy(circular_buffer + write_head, buffer_in, frame_size);
    write_head = (write_head + frame_width) % circular_size;

    (*recorder_buf_q)->Enqueue(recorder_buf_q, buffer_in, bufSize);

    if (write_head == frame_width * 2 && !start_playback) {
        start_playback = true;
        (*player_buf_q)->Enqueue(player_buf_q, buffer_out, bufSize);
    }
}


static void player_callback(SLAndroidSimpleBufferQueueItf bq, void *context)
{
    while((write_head - read_head <= frame_width) && (write_head - read_head >= 0)) {
        LOG("%d %d", write_head, read_head);
        std::this_thread::sleep_for (std::chrono::milliseconds(10));
    }

    for (int i = 0; i < frame_width; i++) {
        int sample_idx = read_head + i;
        buffer_out[i] = circular_buffer[sample_idx % circular_size];
        buffer_out[i] += circular_buffer[(circular_size + sample_idx - 1500) % circular_size] * lfo[sample_idx % circular_size];
        buffer_out[i] += circular_buffer[(circular_size + sample_idx - 1000) % circular_size] * lfo[sample_idx % circular_size];
        buffer_out[i] += circular_buffer[(circular_size + sample_idx - 500) % circular_size] * lfo[sample_idx % circular_size];
        buffer_out[i] *= 0.25;
    }
  //  memcpy(buffer_out, circular_buffer + read_head, frame_size);
    read_head = (read_head + frame_width) % circular_size;

    (*player_buf_q)->Enqueue(player_buf_q, buffer_out, bufSize);
}



extern "C" {


void Java_com_amirgu_tarfx_AudioProcessor_initialize(JNIEnv *env, jobject obj)
{
    SLresult result;

    SLDataFormat_PCM format;
    format.formatType = SL_DATAFORMAT_PCM;
    format.numChannels = NUM_CHANNELS;
    format.samplesPerSec = static_cast<SLmilliHertz>(SAMPLE_RATE) * 1000;
    format.bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16;
    format.containerSize = BITS_PER_BYTE * sizeof(short);
    format.channelMask = SL_SPEAKER_FRONT_CENTER;
    format.endianness = SL_BYTEORDER_LITTLEENDIAN;

    buffer_in = (short *) calloc(frame_width, sizeof(short));
    buffer_out = (short *) calloc(frame_width, sizeof(short));
    circular_buffer = (short *) calloc(circular_size, sizeof(short));

    LOG("%d %lu %lu", circular_buffer, circular_size, frame_size);


    lfo = (double *) calloc(circular_size, sizeof(double));
    int freq = 1 * CIRCULAR_DURATION;
    double step = 2 * M_PI * freq / (circular_size);
    for (int i = 0; i < circular_size; i++) {
        lfo[i] = sin(-M_PI * freq + step * i);
    }

    // CREATE ENGINE

    result = slCreateEngine(&engine_obj, 0, NULL, 0, NULL, NULL);
    ASSERT(result);
    result = (*engine_obj)->Realize(engine_obj, SL_BOOLEAN_FALSE);
    ASSERT(result);
    result = (*engine_obj)->GetInterface(engine_obj, SL_IID_ENGINE, &engine);
    ASSERT(result);


    // SET UP PLAYER

    const SLInterfaceID output_ids[] = { SL_IID_VOLUME };
    const SLboolean output_req[] = { SL_BOOLEAN_FALSE };

    result = (*engine)->CreateOutputMix(engine, &output_mix_obj, 0, output_ids, output_req);
    ASSERT(result);
    result = (*output_mix_obj)->Realize(output_mix_obj, SL_BOOLEAN_FALSE);
    ASSERT(result);

    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = { SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2 };
    SLDataSource audioSrc = { &loc_bufq, &format };

    SLDataLocator_OutputMix loc_outmix = { SL_DATALOCATOR_OUTPUTMIX, output_mix_obj };
    SLDataSink audioSnk = { &loc_outmix, NULL };

    const SLInterfaceID player_ids[] = { SL_IID_ANDROIDSIMPLEBUFFERQUEUE };
    const SLboolean player_req[] = { SL_BOOLEAN_TRUE };

    result = (*engine)->CreateAudioPlayer(engine, &player_obj, &audioSrc, &audioSnk, 1, player_ids, player_req);
    ASSERT(result);
    result = (*player_obj)->Realize(player_obj, SL_BOOLEAN_FALSE);
    ASSERT(result);
    result = (*player_obj)->GetInterface(player_obj, SL_IID_PLAY, &player);
    ASSERT(result);

    result = (*player_obj)->GetInterface(player_obj, SL_IID_ANDROIDSIMPLEBUFFERQUEUE, &player_buf_q);
    ASSERT(result);
    result = (*player_buf_q)->RegisterCallback(player_buf_q, player_callback, nullptr);
    ASSERT(result);


    // SET UP RECORDER

    SLDataLocator_IODevice loc_dev = { SL_DATALOCATOR_IODEVICE, SL_IODEVICE_AUDIOINPUT, SL_DEFAULTDEVICEID_AUDIOINPUT, NULL };
    SLDataSource audioSrc2 = { &loc_dev, NULL };

    SLDataLocator_AndroidSimpleBufferQueue loc_bq = { SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2 };
    SLDataSink audioSnk2 = { &loc_bq, &format };

    const SLInterfaceID recorder_ids[] = { SL_IID_ANDROIDSIMPLEBUFFERQUEUE };
    const SLboolean recorder_req[] = { SL_BOOLEAN_TRUE };

    result = (*engine)->CreateAudioRecorder(engine, &recorderObject, &audioSrc2, &audioSnk2, 1, recorder_ids, recorder_req);
    ASSERT(result);
    result = (*recorderObject)->Realize(recorderObject, SL_BOOLEAN_FALSE);
    ASSERT(result);
    result = (*recorderObject)->GetInterface(recorderObject, SL_IID_RECORD, &recorder);
    ASSERT(result);

    result = (*recorderObject)->GetInterface(recorderObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE, &recorder_buf_q);
    ASSERT(result);
    result = (*recorder_buf_q)->RegisterCallback(recorder_buf_q, record_callback, nullptr);
    ASSERT(result);

    LOG("init success");
}


void Java_com_amirgu_tarfx_AudioProcessor_start(JNIEnv *env, jobject)
{
    SLresult result;

    ASSERT_NOT_NULL(player);
    ASSERT_NOT_NULL(recorder);

    start_playback = false;
    write_head = 0;
    read_head = 0;

    result = (*player)->SetPlayState(player, SL_PLAYSTATE_PLAYING);
    ASSERT(result);

    result = (*recorder)->SetRecordState(recorder, SL_RECORDSTATE_RECORDING);
    ASSERT(result);

    (*recorder_buf_q)->Enqueue(recorder_buf_q, buffer_in, bufSize);
    (*player_buf_q)->Enqueue(recorder_buf_q, buffer_out, bufSize);

    LOG("start success");
}


void Java_com_amirgu_tarfx_AudioProcessor_stop(JNIEnv *env, jobject)
{
    SLresult result;

    ASSERT_NOT_NULL(player);
    ASSERT_NOT_NULL(recorder);

    result = (*player)->SetPlayState(player, SL_PLAYSTATE_STOPPED);
    ASSERT(result);

    result = (*recorder)->SetRecordState(recorder, SL_RECORDSTATE_STOPPED);
    ASSERT(result);

    LOG("stop success");
}


void Java_com_amirgu_tarfx_AudioProcessor_destroy(JNIEnv *env, jobject)
{
    if (engine_obj != NULL) {
        (*engine_obj)->Destroy(engine_obj);
        engine_obj = NULL;
        engine = NULL;
    }
    if (output_mix_obj != NULL) {
        (*output_mix_obj)->Destroy(output_mix_obj);
        output_mix_obj = NULL;
    }
    if (player_obj != NULL) {
        (*player_obj)->Destroy(player_obj);
        player_obj = NULL;
        player = NULL;
    }
    if (recorderObject != NULL) {
        (*recorderObject)->Destroy(recorderObject);
        recorderObject = NULL;
        recorder = NULL;
    }

    free(buffer_in);
    free(buffer_out);
    free(circular_buffer);
    free(lfo);
}


jshortArray Java_com_amirgu_tarfx_AudioProcessor_readBuffer(JNIEnv *env, jobject)
{
    jshortArray result;
    result = env->NewShortArray(frame_width);
    env->SetShortArrayRegion(result, 0, frame_width, buffer_out);
    return result;
}


}



