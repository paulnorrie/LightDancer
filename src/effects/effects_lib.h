/**
 * A bunch of LED effects.  Because their methods are templated, they are included in a header file.
 */
#ifndef EVENTS_LIB_H
#define EVENTS_LIB_H

#include "../draw.h" // Frame, DrawInfo
#include "etl/variant.h"

/***************************************************************************************************
 * @brief Base Class of all Effects
 **************************************************************************************************/
template <typename Derived>
class EffectBase {
    public:
    template <typename FreqT, unsigned int FreqN>
    void draw_frame(Frame& frame, DrawInfo<FreqT, FreqN>& info) {
        static_cast<Derived*>(this)->draw_frame(frame, info);
    }
};


/***************************************************************************************************
 * @brief Laser effect.
 * A bar of red light moves across the LED strip every 1 second.
 **************************************************************************************************/
class LaserEffect : public EffectBase<LaserEffect>{
    
    private:
    unsigned int position = 0; // TODO: are we using size_t somewhere else? needs to be consistent
    unsigned int laser_length;

    public:
    template <typename FreqT, unsigned int FreqN>
    void draw_frame(Frame& frame, DrawInfo<FreqT, FreqN>& info){
        if (laser_length == 0) {
            laser_length = frame.num_leds / 10;
        }
        std::fill(frame.data.begin(), frame.data.end(), BLACK);
        position += (info.elapsed_time_ms / 100) * laser_length; // move laser by 1/10 of strip every 100ms (TODO: floating point)
         if (position >= frame.num_leds) {
            position = 0;
        }
    };
};


/***************************************************************************************************
 * @brief Blink Effect
 **************************************************************************************************/
class BlinkEffect : public EffectBase<BlinkEffect> {
    
    private:
    bool is_on = false;

    public:
     template <typename FreqT, unsigned int FreqN>
    void draw_frame([[maybe_unused]]Frame& frame, [[maybe_unused]]DrawInfo<FreqT, FreqN>& info){
        std::fill(frame.data.begin(), frame.data.end(), is_on ? LIME : BLACK);
        is_on = !is_on;
    };
};


/***************************************************************************************************
 * @brief Beat Blink
 **************************************************************************************************/
class BeatBlinkEffect : public EffectBase<BlinkEffect> {
    public:
    template <typename FreqT, unsigned int FreqN>
    void draw_frame([[maybe_unused]]Frame& frame, [[maybe_unused]]DrawInfo<FreqT, FreqN>& info){
        
    };
};



#endif  // EVENTS_LIB_H
