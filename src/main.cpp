extern "C" {
#include "pico/stdio.h"
}

#include "ws2811.h"

int main() {
    
    // Pico init
    stdio_init_all();
    
    // LED init
    uint8_t gpio_pin = 1;
    uint bps = 800'000;
    WS2811 leds(760 * 5, bps, gpio_pin);
    leds.test();

    return 0;
}


