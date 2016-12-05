
#include "AudioProcessor.h"


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


short * circular_buffer;
short * buffer_in;
short * buffer_out;
float * lfo;


int frame_width = FRAMES_PER_BUF * NUM_CHANNELS;
int frame_size = frame_width * sizeof(short);
long circular_size = frame_width * (SAMPLE_RATE/frame_width) * CIRCULAR_DURATION;
int bufSize = frame_width * SL_PCMSAMPLEFORMAT_FIXED_16 / BITS_PER_BYTE;


Effect * effect_buffer;
Effect * effect_delay;
Effect * effect_chorus;




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
        buffer_out[i] = read(effect_buffer, sample_idx) * 0.6;
        //buffer_out[i] += read(effect_delay, sample_idx) * 0.1;
        buffer_out[i] += read(effect_chorus, sample_idx) * 0.3;
        buffer_out[i] *= 2;
    }
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



    // EFFECTS

    effect_buffer = initializeBuffer(circular_buffer, circular_size);
    effect_delay = initializeDelay(effect_buffer, 0.02, circular_size);
    effect_chorus = initializeChorus(effect_buffer, 0.04, 4, circular_size);

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

    destroyEffect(effect_delay);
    destroyEffect(effect_buffer);
    destroyEffect(effect_chorus);
}


jshortArray Java_com_amirgu_tarfx_AudioProcessor_readBuffer(JNIEnv *env, jobject)
{
    jshortArray result;
    result = env->NewShortArray(frame_width);
    env->SetShortArrayRegion(result, 0, frame_width, buffer_out);
    return result;
}


}



