#ifndef WS2811_H
#define WS2811_H

extern "C" {
#include "hardware/pio.h"
}
#include "../../draw.h"

#include <cstdint>

/**
 * @brief Driver for WS2811 LED using Raspberry Pi Pico Programmable I/O (PIO).
 * 
 * Raspberry Pi PIO does not run on a CPU core but on a separate state machine, keeping the CPU
 * free for other tasks.  It is available on RP2040 and RP2350 microcontrollers.
 */
class WS2811Pio final {
    

    private:

    PIO pio_ = nullptr;
    uint sm_ = 0;
    uint offset_ = 0;
    

    public:

    WS2811Pio() = delete;
    WS2811Pio(const WS2811Pio&) = delete;
    ~WS2811Pio();

    /**
     * @brief Construct an instance of WS2811.  If an instance cannot be constructed, abort() 
     * is called.
     * 
     * @param [in] num_leds Number of LEDs on LED strip
     * @param [in] bps Transmission frequency in bits per second, typically 800,000 or 400,000.
     * @param [in] pin Number of the GPIO pin for data out.
     */
    WS2811Pio(uint bps, uint8_t pin);

    /**
     * @brief Send a frame to the LED strip to display.  This may block until the frame is completely
     * sent or it may return immediately (e.g. if the driver uses DMA).  If it returns immediately,
     * a subsequent call to `send()` will block until the prior frame is completely sent.
     */
    void send(const Frame& frame);

    /**
     * @brief Test LEDs work with alternating pattern of Red, Green, & Blue colours.
     * 
     * This blocks until data is written to the LED, upon which LEDs should be showing a 
     * steady pattern of alternating Red, Green, & Blue. 
     */
    void test(size_t num_leds);
};

#endif