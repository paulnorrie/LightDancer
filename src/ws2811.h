#ifndef WS2811_H
#define WS2811_H

extern "C" {
#include "hardware/pio.h"
}


class WS2811 {
    

    private:

    PIO pio_ = nullptr;
    uint sm_ = 0;
    uint num_leds_ = 0;
    uint offset_ = 0;

    public:

    WS2811() = delete;
    WS2811(const WS2811&) = delete;
    ~WS2811();

    /**
     * @brief Construct an instance of WS2811.  If an instance cannot be constructed, abort() 
     * is called.
     * 
     * @param [in] num_leds Number of LEDs on LED strip
     * @param [in] bps Transmission frequency in bits per second, typically 800,000 or 400,000.
     * @param [in] pin Number of the GPIO pin for data out.
     */
    WS2811(uint num_leds, uint bps, uint8_t pin);

    /**
     * @brief Test LEDs work with alternating pattern of Red, Green, & Blue colours.
     * 
     * This blocks until data is written to the LED, upon which LEDs should be showing a 
     * steady pattern of alternating Red, Green, & Blue. 
     */
    void test();
};

#endif