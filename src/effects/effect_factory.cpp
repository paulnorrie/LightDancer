#include "effect_factory.h"
#include "../draw.h"
#include "etl/variant.h"

EffectFactory::EffectFactory(): ev_(LaserEffect{}){
} 


void EffectFactory::set_effect(const size_t index) {
    switch (index) {
        case LASER: ev_.emplace<LaserEffect>(); break;
        case BLINK: ev_.emplace<BlinkEffect>(); break;
        case BEATBLINK: ev_.emplace<BeatBlinkEffect>(); break;
        
        default: ev_.emplace<LaserEffect>(); break;
    };
}

