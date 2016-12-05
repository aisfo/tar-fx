//
// Created by Asif Amirguliyev on 12/3/16.
//

#include "effects.h"


typedef struct {
    int duration_samples;
    unsigned int buffer_size;
    Effect * src;
} Delay;

typedef struct {
    Effect * delay;
    float * lfo;
    unsigned int buffer_size;
} Chorus;


Effect * initializeBuffer(short * circular_buffer, unsigned int buffer_size) {
    Effect * newEffect = (Effect *) malloc(sizeof(Effect));
    newEffect->type = BUFFER;
    newEffect->effect_obj = circular_buffer;

    return newEffect;
}

short readBuffer(Effect * src, int idx) {
    short * circular_buffer = (short *) src->effect_obj;
    return circular_buffer[idx];
}



Effect * initializeDelay(Effect * src, float delay_seconds, unsigned int buffer_size) {
    Delay * newDelay = (Delay *) malloc(sizeof(Delay));

    newDelay->buffer_size = buffer_size;
    newDelay->duration_samples = int(delay_seconds * SAMPLE_RATE);
    newDelay->src = src;

    Effect * newEffect = (Effect *) malloc(sizeof(Effect));
    newEffect->type = DELAY;
    newEffect->effect_obj = newDelay;

    return newEffect;
}

short readDelay(Effect * effect, int idx) {
    Delay * delay_obj = (Delay *) effect->effect_obj;
    Effect * src = delay_obj->src;
    return read(src, (delay_obj->buffer_size + idx - delay_obj->duration_samples) % delay_obj->buffer_size);
}


Effect * initializeChorus(Effect * src, float delay_seconds, float freq, unsigned int buffer_size) {
    Effect * delay = initializeDelay(src, delay_seconds, buffer_size);

    float * lfo = (float *) calloc(buffer_size, sizeof(float));
    int buffer_duration = buffer_size / SAMPLE_RATE;
    freq = freq * buffer_duration;
    float step = 2 * M_PI * freq / (buffer_size);
    for (int i = 0; i < buffer_size; i++) {
        lfo[i] = sin(-M_PI * freq + step * i);
    }

    Chorus * newChorus = (Chorus *) malloc(sizeof(Chorus));
    newChorus->delay = delay;
    newChorus->lfo = lfo;

    Effect * newEffect = (Effect *) malloc(sizeof(Effect));
    newEffect->type = CHORUS;
    newEffect->effect_obj = newChorus;

    return newEffect;
}

short readChorus(Effect * effect, int idx) {
    Chorus * chorus_obj = (Chorus *) effect->effect_obj;
    short delaySample = read(chorus_obj->delay, idx);
    return delaySample * chorus_obj->lfo[idx];
}



short read(Effect * effect, int idx) {
    switch(effect->type) {
        case BUFFER:
            return readBuffer(effect, idx);
        case DELAY:
            return readDelay(effect, idx);
        case CHORUS:
            return readChorus(effect, idx);
    }
}


void destroyEffect(Effect * effect) {
    switch(effect->type) {

        case BUFFER:
            break;

        case DELAY: {
            Delay *delay_obj = (Delay *) effect->effect_obj;
            free(delay_obj);
            break;
        }

        case CHORUS: {
            Chorus *chorus_obj = (Chorus *) effect->effect_obj;
            destroyEffect(chorus_obj->delay);
            free(chorus_obj->lfo);
            free(chorus_obj);
            break;
        }

    }

    free(effect);
}