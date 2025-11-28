/**
 * @file effect_factory.h
 * @brief A way to choose and use an effect chosen by a variable known only at run-time. If you 
 * know this at compile-time, just use the effect classes directly.
 */
#ifndef EFFECTS_FACTORY_H
#define EFFECTS_FACTORY_H

#include "../draw.h"
#include "effects_lib.h"

/** 
 * @brief Variant type containing one LED Effect
 * Must match `EffectId` and `createEffect`
 * 
 * ** WARNING ** Do not change the order of these effects.  They are used in `createEffect` to
 * determine which effect to create.
 */


class EffectFactory {

    public:
    EffectFactory();
    EffectFactory(const EffectFactory&) = delete;

    enum EffectType {
        LASER = 0,
        BLINK = 1,
        BEATBLINK = 2
    };

    /**
     * @brief 
    */
    void set_effect(const size_t index);

    template <typename FreqT, unsigned int FreqN>
    void draw_frame(Frame& frame, DrawInfo<FreqT, FreqN>& info) {
        etl::visit([&](auto& obj) {
            obj.draw_frame(frame, info);
        }, ev_);
    };

    private:
    using EffectVariant = etl::variant<LaserEffect, BlinkEffect, BeatBlinkEffect>;
    EffectVariant ev_;


};


#endif // EFFECTS_FACTORY_H


