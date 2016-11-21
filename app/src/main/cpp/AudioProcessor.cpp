#include "AudioProcessor.h"



static void record_callback(SLAndroidSimpleBufferQueueItf bq, void *context)
{
    if (bufferIndex == 1) {
        bufferIndex = 2;
        (*player_buf_q)->Enqueue(player_buf_q, buffer_1, bufSize);
        (*recorder_buf_q)->Enqueue(recorder_buf_q, buffer_2, bufSize);
    } else {
        bufferIndex = 1;
        (*player_buf_q)->Enqueue(player_buf_q, buffer_2, bufSize);
        (*recorder_buf_q)->Enqueue(recorder_buf_q, buffer_1, bufSize);
    }
}

static void player_callback(SLAndroidSimpleBufferQueueItf bq, void *context)
{
}


extern "C" {


void Java_com_amirgu_tarfx_AudioProcessor_initialize(JNIEnv *env, jobject)
{
    SLresult result;

    bufSize = FRAMES_PER_BUF * NUM_CHANNELS * SL_PCMSAMPLEFORMAT_FIXED_16 / BITS_PER_BYTE;

    SLDataFormat_PCM format;
    format.formatType = SL_DATAFORMAT_PCM;
    format.numChannels = NUM_CHANNELS;
    format.samplesPerSec = static_cast<SLmilliHertz>(SAMPLE_RATE) * 1000;
    format.bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16;
    format.containerSize = BITS_PER_BYTE * sizeof(short);
    format.channelMask = SL_SPEAKER_FRONT_CENTER;
    format.endianness = SL_BYTEORDER_LITTLEENDIAN;

    buffer_1 = (short *) calloc(FRAMES_PER_BUF * NUM_CHANNELS, sizeof(short));
    buffer_2 = (short *) calloc(FRAMES_PER_BUF * NUM_CHANNELS, sizeof(short));


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

    result = (*player)->SetPlayState(player, SL_PLAYSTATE_PLAYING);
    ASSERT(result);

    result = (*recorder)->SetRecordState(recorder, SL_RECORDSTATE_RECORDING);
    ASSERT(result);

    bufferIndex = 1;
    (*recorder_buf_q)->Enqueue(recorder_buf_q, buffer_1, bufSize);

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

    free(buffer_1);
    free(buffer_2);
}


}



