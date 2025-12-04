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
    private:
    
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
    unsigned int laser_length = 0;
    uint32_t cum_elapsed_time_us = 0;

    public:
    template <typename FreqT, unsigned int FreqN>
    void draw_frame(Frame& frame, DrawInfo<FreqT, FreqN>& info){
        if (laser_length == 0) {
            laser_length = frame.num_leds / 10;
        }
        
        // move laser by 1/10 of strip every 50ms
        // it is possible draw_frame is called faster than the laser moves by 1 LED position
        // (i.e. info.elapsed_time_us / 50ms < 1 / laser_length) in which case the laser cannot
        // increment it's position ever so we cumulate the elapsed time into cum_elapsed_time
        cum_elapsed_time_us += info.elapsed_time_us;     // in case we are called faster than 100ms / laser_length in which case laser won't move 
        position = (cum_elapsed_time_us / 50'000.0f) * laser_length; 
         if (position >= frame.num_leds) {
            position = 0;
            cum_elapsed_time_us = 0;
        }
        
        // draw laser
        etl::span<RGBValue>::iterator it = frame.data.begin();
        std::fill(it, frame.data.end(), BLACK);
        for (unsigned int i = position; i <= position + laser_length; i++) {
            frame.data[i] = RED; 
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
