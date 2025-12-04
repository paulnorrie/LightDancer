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
 * 
 * This class uses DMA to transfer data to the PIO state machine, hence `send_frame(Frame&, DrawInfo&)`
 * returns before the frame is sent.
 * 
 * This class is designed to have only one instance, because it registers an IRQ handler for when
 * DMA transfers are complete.  
 * 
 */
class WS2811Pio final {
    

    private:

    static inline WS2811Pio *instance_ = nullptr;   // singleton instance
    PIO pio_ = nullptr;             // PIO in use
    uint sm_ = 0;                   // State Machine in use
    uint offset_ = 0;               // Offset in SM, pio code starts at
    uint dma_chan_;                 // DMA channel for data transfer from RAM to PIO State Machine
    int num_words_to_reset_;        // No o 32-bit words required to send RESET signal

    static void dma_irq_handler_c_wrapper(void); // IRQ handler to be registered with Pico SDK
    void dma_irq_handler();                      // IRQ handler that can use member variables
    void send_reset_signal();                    
    void install_pio_and_run(uint8_t pin, uint bps);
    void setup_dma();


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