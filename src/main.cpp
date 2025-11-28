#include <cstdio>

#include "etl/random.h"

extern "C" {
#include "pico/stdlib.h"
}
#include "draw.h"
#include "effects/effect_factory.h"
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
    // loop
    //      get effect
    //      refresh_rate = // how many times we can send frames (incl reset) every second = bps / (num_leds * bit_depth) = 800000 / (760 * 5 * 24) = 8.77 fps we can send fastest
    //      max_frame_rate = refresh_rate // but we may want slower?
    //      
    //      max_frame_rate = min(refresh_rate, target_frame_rate) // if target_frame_rate > refresh_rate then we are limited by refresh_rate and can't do anything about it
    //         but that doesn't mean we can send every frame at max_frame_rate if building the frame is slower, which means we could skip frames or just slow it down (with beat tracking looks odd)
    //      max_frame_time = 1 / max_frame_rate; //1 fps = 1000ms, 2fps = 500ms, 3fps = 333ms, 8.77fps = 114ms, 17fps = 57ms
    //      
    //      loop
    //         if elapsed_time >= max_frame_time
    //      effect.draw_frame(frame)  // what about draw on beat and pass freqs?
    //      leds.send(frame)
    //         - wait until any DMA is complete (IRQ handler sends RESET and flags done)
    //         - setup DMA to read from frame
    //         - start DMA and return
    //
    //         wait until next frame time
    //      
    //      effects that use beat should get the beat passed to them to adjust their drawing accordingly
    
    etl::random_xorshift rng;
    auto i = rng.range(0, 1);

    Frame frame(760 * 5);
    EffectFactory effect_factory;
    effect_factory.set_effect(i);

    etl::array<unsigned short, 1> fft_mags {1};
    DrawInfo<unsigned short, 1> info {(unsigned short)100, fft_mags};
    effect_factory.draw_frame(frame, info);
    // etl::visit([&frame, &info](auto& obj) {
    //     obj.draw_frame(frame, info);
    // }, effect);
    // leds.send(frame);


    while (1) {
        leds.test(760 * 5);
    }

    return 0;
}


