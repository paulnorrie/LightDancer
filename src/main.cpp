#include <cstdio>

extern "C" {
#include "pico/stdlib.h"
}
#include "leds/leds.h"

#include "leds/ws2811pio/ws2811pio.h"

void loop() {
    
    //PixelValue frames[2][NUM_LEDS];  // 5m strip at 760 LEDs/m  
    //Frame frame(760 * 5);

    // 1. Set up driver
    //WS2811Pio leds(frame.num_leds, 800'000, 2); // 5m strip at 760 LEDs/m on GPIO2 setup with DMA // might be nice to reference frame?
    // 2. Draw a frame
    //for (size_t i = 0; i < frame.num_leds; ++i) { // might be nice to have iterators?
    //    frame.data[i].r = 255;
        //frames[0][i].r = 255;
        //frames[0][i].g = 0;
        //frames[0][i].b = 0;
    //};
    //2(a). Or, use an effect:
    // leds.draw_frame(frame); // might be nice to just know (have iterators?)

    // 3. Send the frame, this may be non-blocking, e.g. if DMA is used by the driver or may return
    // only when done.  Alternative is to use this in a FreeRTOS task/thread.
    // leds.send(frame);

    // 4. Build new frame

    // 5. Send the frame, this may block until ready to send (e.g. the prior frame is done)
}



int main() {
    
    // Pico init
    stdio_init_all();
    
    // LED init
    uint8_t gpio_pin = 2;
    uint bps = 400'000;
    WS2811Pio leds(bps, gpio_pin);
    printf("LightDancer is up.\n");
    while (1) {
        leds.test(760 * 5);
    }

    return 0;
}


