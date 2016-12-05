//
// Created by Asif Amirguliyev on 12/3/16.
//

#ifndef TARFX_EFFECTS_H
#define TARFX_EFFECTS_H

#include "common.h"

enum effect_t {
    BUFFER,
    DELAY,
    CHORUS
};


typedef struct Effect {
    effect_t type;
    void * effect_obj;
} Effect;



Effect * initializeBuffer(short * circular_buffer, unsigned int buffer_size);
Effect * initializeDelay(Effect * src, float delay_seconds, unsigned int buffer_size);
Effect * initializeChorus(Effect * src, float delay_seconds, float freq, unsigned int buffer_size);


short read(Effect * effect, int idx);
void destroyEffect(Effect * effect);




#endif //TARFX_EFFECTS_H
