#ifndef DRAW_H
#define DRAW_H

#include <cstdint>
#include <algorithm>
#include "etl/span.h"
#include "etl/array.h"

#define MAX_LEDS 3800 /// @todo Put this in global config for stack memory layout - or const?
#define MULTIPLE_OF_FOUR(n) ((n + 3) / 4) * 4
#define MAX_LED_DATA_LEN MULTIPLE_OF_FOUR(MAX_LEDS)

/**
 * @brief Colour structure representing an RGB pixel.
 */
struct RGBValue {

    uint8_t r;  /// Red component
    uint8_t g;  /// Green component
    uint8_t b;  /// Blue component
   
    /**
     * @brief Convert to 32-bit RGB value in little endian format with red the most significant byte
     * and zero as the least significant byte.
     * ie. {MSB, ..., LSB} = {Red, Green, Blue, 0}
     */
    uint32_t as_RGB() {
        return  (static_cast<uint32_t>(r) << 24) |
                (static_cast<uint32_t>(g) << 16) |
                (static_cast<uint32_t>(b) << 8);
    }
};

// from 16 standard (CSS/HTML) colours
#define WHITE RGBValue{255, 255, 255}
#define RED   RGBValue{255, 0, 0}
#define LIME RGBValue{0, 255, 0}
#define BLUE  RGBValue{0, 0, 255}
#define BLACK RGBValue{0, 0, 0}
#define YELLOW RGBValue{0xFF, 0xFF, 0}


/**
 * @brief Memory buffer of LED pixel values.
 * 
 * Memory is allocated on the stack for MAX_LEDS at compile-time, however the actual number of LEDs
 * can be set at runtime in the constructor, capped to MAX_LEDS to prevent stack overrun.
 * Stack requirement is slightly > sizeof(RGBValue) * MAX_LEDS.
 * 
 * Memory can be read, written, and iterated using the 'data` member.
 */
class Frame {
    
    private:
    
    RGBValue inner_data_[MAX_LED_DATA_LEN]; // must be multiple of 4 so we can transfer
                                                      // 32-bit words at a time (not every word will be RGB). 


    public:

    etl::span<RGBValue> data;      /// mutable pixel data (can be less than MAX_LEDS)
    const unsigned int num_leds;   /// number of LEDs (specifically LED drivers) in this frame

    /**
     * @brief Construct a Frame instance.
     * 
     * @param [in] num_of_leds Number of LED _drivers (ICs)_ in this frame. Most of the time this
     * is the same as the number of LEDs but some drivers drive multiple LEDs.
     * It is capped to MAX_LEDS. The value
     * (capped) is available in the 'num_leds' member and this should be used for the actual number
     * of leds going forward.  
     */
    Frame(int num_of_leds) : data(&inner_data_[0], num_of_leds),
                             num_leds(std::min(num_of_leds, MAX_LEDS)){
    };

    

};


/**
 * @brief Information passed to `Effect::draw_frame(Frame&, const DrawInfo&)`
 * 
 * @param FreqT data type of the magnitudes of the Fast Fourier Transform (typically some form of
 *        integer)
 * @param FreqN number of elements in the magnitudes of the FFT
 */
template <typename FreqT, unsigned int FreqN>
struct DrawInfo {

    uint32_t elapsed_time_us; /// Microseconds since `draw_frame` was called last time. The first call may be slightly > 0us
        
    etl::array<FreqT, FreqN>& freq_magnitudes; /// Magnitudes of a FFT spread between 0Hz and the sample rate of the FFT
};


#endif // DRAW_H