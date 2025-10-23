#include <cstddef>
#include <stdint.h>
#include "etl/array.h"
/**
* @brief Compile-time Waveform Generator.
* 
* Generates a sine wave at a specified frequency, sample rate and number of samples.
*
* e.g.
* @code{.cpp}
* WaveGen<44000, 512> generator; // 512 samples at 44kHz sample rate
* etl::array<int16_t, 512> samples = generator.sin(1000); // for 1000Hz sine wave
* @endcode
* 
* @param Fs sampling frequency in samples per second
* @param N number of samples to generate - must be even
* 
*/
template<size_t Fs, size_t N> 
class WaveGen {
    
    public:
    
    static_assert(!(N & 1), "N must be even");
    static constexpr double amplitude_max = 32767;
    
    /**
     * @brief Generate a sine wave at the specified frequency.
     */
    etl::array<int16_t, N> sin(const double freq) {
        
        etl::array<int16_t, N> wave;
        
        // Generate the sine wave
        for (int i = 0; i < N; ++i) {
            double time = static_cast<double>(i) / static_cast<double>(Fs);
            wave[i] = static_cast<int16_t>(amplitude_max * std::sin(2 * M_PI * freq * i / (double)Fs));
        }
        
        return wave;
    }
    
};
