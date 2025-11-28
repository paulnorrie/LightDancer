#ifndef FFT_FIXED_POINT_H
#define FFT_FIXED_POINT_H

#include <stdint.h>
#include <array>
#include <type_traits>
#include "etl/array.h"

/**
 * The Window Type. 
 * 
 * Choose the window type for FixedPointFFT based on your application's needs:
 * | Window         | Sidelobe Suppression | Main Lobe Width  | Memory   | Computation |
 * |----------------|----------------------|------------------|----------|-------------|
 * | Bartlett       | ~26 dB               | Narrow           | 1 byte   | Fast        |
 * | Hann           | ~31 dB               | Medium           | N*2 bytes| Moderate    |
 * | Blackman-Harris| ~92 dB               | Wider            | N*2 bytes| Moderate+   |
 */ 
enum class WindowType {
    Bartlett,
    Hann,
    BlackmanHarris
};



/**
 * @brief Fixed-point Fast Fourier Transform (FFT).
 * 
 * This handles real-valued, symmetric inputs, such as from audio.  This is suitable for microcontrollers
 * where:
 * - Floating point operations are slow or unavailable.
 * - Heap allocations are prohibited or should be avoided.
 * 
 * Usage example:
 * ```cpp
 * 
 * // Handle 256 samples, the input is int16_t, output magnitudes are uint16_t & use a Hann window
 * FixedPointFFT<256, int16_t, uint16_t, WindowType::Hann> fft;
 * 
 * etl::array<int16_t, 256> samples;     // audio samples
 * etl::array<uint16_t, 129> magnitudes; // 256/2 + 1 = 129
 * 
 * while (get_audio_samples(samples)) {  // hypothetical function to fill samples
 *     fft.magnitudes(samples, magnitudes);
 *     // do something with the magnitudes...
 * }
 * ```
 * 
 * @param N number of samples to calculate the FFT with (must be a power of 2).  e.g. 256, 512, 1024.
 * see @ref magnitudes(etl::array<InputType, N>&, etl::array<OutputType, N/2+1>&) for information
 * on the effects of N on frequency resolution.
 * @param InputType Type of input samples (int16_t or int32_t). Use the one that best matches your
 * input data.
 * @param OutputType Type of output magnitudes (uint16_t or uint32_t).  use uint16_t for lower memory usage,
 * uint32_t if you need higher dynamic range (~192dB vs ~96dB).
 * @param Window Type of windowing function to apply before FFT. Blackman-Harris provides the best
 * sidelobe suppression but is more computationally expensive than the other WindowType variants.
 * 
 * Memory Usage (including input and output buffers):
 * | InputType | WindowType             | Stack Usage      |
 * | OutputType|                        |                  |
 * |-----------|------------------------|------------------|
 * | int16_t   | Bartlett               | ~12N bytes       |
 * | int16_t   | Hann or BlackmanHarris | ~14N bytes       |
 * | int32_t   | Bartlett               | ~16N bytes       |
 * | int32_t   | Hann or BlackmanHarris | ~18N bytes       |
 * NB: InputType and OutputType may be different but this gives you the memory range required.
 * 
 * <code>magnitudes(InputType, OutputType)</code> provides the sound power for N/2+1 frequencies.
 * The range between frequencies being <code>sample_rate / N</code>.
 
 */
template<uint16_t N, typename InputType = int16_t, typename OutputType = uint16_t, WindowType Window = WindowType::Bartlett>
class FixedPointFFT {

    private:
    
    // Compile-time assertions
    static_assert( (N & (N - 1)) == 0, "FFT size must be power of 2");
    static_assert(std::is_same<InputType, int16_t>::value || std::is_same<InputType, int32_t>::value, 
                  "InputType must be int16_t or int32_t");
    static_assert(std::is_same<OutputType, uint16_t>::value || std::is_same<OutputType, uint32_t>::value, 
                  "OutputType must be uint16_t or uint32_t");
    
    // Q15 format: 1 sign bit + 15 fractional bits
    static constexpr int16_t Q15_ONE = 32767;
    
    // Twiddle factors stored as Q15
    int16_t twiddle_real[N/2];
    int16_t twiddle_imag[N/2];
    
    // Window coefficients stored as Q15 (only allocated if not Bartlett)
    int16_t window_coeffs[Window == WindowType::Bartlett ? 1 : N];
    
    // Bit reversal lookup table
    uint16_t bit_reverse[N];
    

    /*
    Fixed-point sine approximation (Q15 format)
    Uses Taylor series for small angles
    */
    int16_t sin_q15(int32_t angle_q15) {
        // Normalize angle to -pi to pi range
        while (angle_q15 > Q15_ONE) angle_q15 -= 2 * Q15_ONE;
        while (angle_q15 < -Q15_ONE) angle_q15 += 2 * Q15_ONE;
        
        // For better accuracy over full range, use symmetry
        bool negate = false;
        if (angle_q15 < 0) {
            negate = true;
            angle_q15 = -angle_q15;
        }
        
        if (angle_q15 > (Q15_ONE >> 1)) {
            angle_q15 = Q15_ONE - angle_q15;
        }
        
        // Taylor series: sin(x) ≈ x - x³/6 + x⁵/120
        int32_t x = angle_q15;
        int32_t x2 = (x * x) >> 15;
        int32_t x3 = (x2 * x) >> 15;
        int32_t x5 = (x3 * x2) >> 15;
        
        int32_t result = x - (x3 / 6) + (x5 / 120);
        
        // Clamp to Q15 range
        if (result > Q15_ONE) result = Q15_ONE;
        if (result < -Q15_ONE) result = -Q15_ONE;
        
        return negate ? -result : result;
    }
    

    /*
    Fixed-point cosine (Q15 format)
    */ 
    int16_t cos_q15(int32_t angle_q15) {
        return sin_q15(angle_q15 + (Q15_ONE >> 1)); // cos(x) = sin(x + π/2)
    }
    

    /*
    Saturating multiplication in Q15
    */
    int16_t mul_q15(int16_t a, int16_t b) {
        int32_t result = ((int32_t)a * (int32_t)b) >> 15;
        if (result > Q15_ONE) return Q15_ONE;
        if (result < -32768) return -32768;
        return (int16_t)result;
    }
    

    /*
    Saturating addition
    */
    int16_t add_sat(int16_t a, int16_t b) {
        int32_t result = (int32_t)a + (int32_t)b;
        if (result > Q15_ONE) return Q15_ONE;
        if (result < -32768) return -32768;
        return (int16_t)result;
    }
    

    /*
    Saturating subtraction
    */
    int16_t sub_sat(int16_t a, int16_t b) {
        int32_t result = (int32_t)a - (int32_t)b;
        if (result > Q15_ONE) return Q15_ONE;
        if (result < -32768) return -32768;
        return (int16_t)result;
    }
    

    /* 
    Bit reversal for FFT reordering
    */
    uint16_t reverse_bits(uint16_t x, uint16_t bits) {
        uint16_t result = 0;
        for (uint16_t i = 0; i < bits; i++) {
            result = (result << 1) | (x & 1);
            x >>= 1;
        }
        return result;
    }
    
    /*
    Calculate number of bits needed
    */
    uint16_t log2_n() {
        uint16_t bits = 0;
        uint16_t n = N;
        while (n > 1) {
            n >>= 1;
            bits++;
        }
        return bits;
    }
    

    /*
    Apply a window
    */ 
    void apply_window(int16_t* data) {
        if (Window == WindowType::Bartlett) {
            // Bartlett (triangular) window - computed on-the-fly
            for (uint16_t i = 0; i < N; i++) {
                int16_t window_val;
                if (i < N/2) {
                    // Rising edge: 2*i/N
                    window_val = (int16_t)(((int32_t)i * 2 * Q15_ONE) / N);
                } else {
                    // Falling edge: 2*(N-i)/N
                    window_val = (int16_t)(((int32_t)(N - i) * 2 * Q15_ONE) / N);
                }
                data[i] = mul_q15(data[i], window_val);
            }
        } else {
            // Hann or Blackman-Harris - use pre-computed coefficients
            for (uint16_t i = 0; i < N; i++) {
                data[i] = mul_q15(data[i], window_coeffs[i]);
            }
        }
    }
    

    /*
    Convert input to Q15 format
    */ 
    int16_t input_to_q15(InputType value) {
        if (std::is_same<InputType, int16_t>::value) {
            return static_cast<int16_t>(value);
        } else {
            // int32_t (24-bit) to Q15: shift right by 8
            return static_cast<int16_t>(value >> 8);
        }
    }
    

    /*
    Compute magnitude and convert to output type
    */
    OutputType compute_magnitude(int16_t real_val, int16_t imag_val, uint16_t scale_count) {
        int32_t r = real_val < 0 ? -real_val : real_val;
        int32_t im = imag_val < 0 ? -imag_val : imag_val;
        
        int32_t max_val = r > im ? r : im;
        int32_t min_val = r < im ? r : im;
        
        int32_t mag = max_val + (min_val >> 1);
        
        // Scale back for any scaling that occurred during FFT
        int64_t scaled_mag = (int64_t)mag << scale_count;
        
        if (std::is_same<OutputType, uint16_t>::value) {
            // Clamp to uint16_t range
            if (scaled_mag > 65535) scaled_mag = 65535;
            if (scaled_mag < 0) scaled_mag = 0;
            return static_cast<OutputType>(scaled_mag);
        } else {
            // Clamp to uint32_t range
            if (scaled_mag > 4294967295LL) scaled_mag = 4294967295LL;
            if (scaled_mag < 0) scaled_mag = 0;
            return static_cast<OutputType>(scaled_mag);
        }
    }
    
    /*
    Normalize magnitudes for uint16_t output
    */
    void normalize_magnitudes_uint16(int16_t* real, int16_t* imag, uint16_t scale_count, 
                                     etl::array<uint16_t, N/2+1>& magnitudes) {
        // First pass: find max magnitude
        int32_t max_magnitude = 0;
        for (uint16_t i = 0; i <= N/2; i++) {
            int32_t r = real[i] < 0 ? -real[i] : real[i];
            int32_t im = imag[i] < 0 ? -imag[i] : imag[i];
            
            int32_t max_val = r > im ? r : im;
            int32_t min_val = r < im ? r : im;
            
            int32_t mag = max_val + (min_val >> 1);
            mag = mag << scale_count;
            
            if (mag > max_magnitude) {
                max_magnitude = mag;
            }
        }
        
        // Calculate normalization shift if needed
        uint16_t norm_shift = 0;
        if (max_magnitude > 52428) {  // 80% of 65535
            int32_t temp = max_magnitude;
            while (temp > 52428) {
                temp >>= 1;
                norm_shift++;
            }
        }
        
        // Second pass: compute normalized magnitudes
        for (uint16_t i = 0; i <= N/2; i++) {
            int32_t r = real[i] < 0 ? -real[i] : real[i];
            int32_t im = imag[i] < 0 ? -imag[i] : imag[i];
            
            int32_t max_val = r > im ? r : im;
            int32_t min_val = r < im ? r : im;
            
            int32_t mag = max_val + (min_val >> 1);
            mag = (mag << scale_count) >> norm_shift;
            
            if (mag > 65535) mag = 65535;
            if (mag < 0) mag = 0;
            
            magnitudes[i] = static_cast<uint16_t>(mag);
        }
    }
    

public:

    /**
     * @brief Construct a new Fixed Point FFT object.
     */
    FixedPointFFT() {
        // Calculate twiddle factors
        for (uint16_t k = 0; k < N/2; k++) {
            // W_N^k = e^(-j*2*pi*k/N)
            // angle = -2*pi*k/N in Q15 format
            int32_t angle = (int32_t)(-2 * Q15_ONE * k) / N;
            twiddle_real[k] = cos_q15(angle);
            twiddle_imag[k] = sin_q15(angle);
        }
        
        // Calculate window coefficients based on window type
        if constexpr (Window == WindowType::Hann) {
            // Hann window: w(n) = 0.5 * (1 - cos(2πn/N))
            for (uint16_t n = 0; n < N; n++) {
                int32_t angle = (int32_t)(2 * Q15_ONE * n) / N;
                int16_t cos_val = cos_q15(angle);
                // w(n) = 0.5 - 0.5*cos_val = 16384 - (cos_val >> 1)
                window_coeffs[n] = 16384 - (cos_val >> 1);
            }
        } else if (Window == WindowType::BlackmanHarris) {
            // Blackman-Harris window
            // w(n) = a0 - a1*cos(2πn/N) + a2*cos(4πn/N) - a3*cos(6πn/N)
            // Coefficients: a0 = 0.35875, a1 = 0.48829, a2 = 0.14128, a3 = 0.01168
            const int16_t a0 = 11761;  // 0.35875 * 32767
            const int16_t a1 = 16001;  // 0.48829 * 32767
            const int16_t a2 = 4630;   // 0.14128 * 32767
            const int16_t a3 = 383;    // 0.01168 * 32767
            
            for (uint16_t n = 0; n < N; n++) {
                int32_t angle1 = (int32_t)(2 * Q15_ONE * n) / N;  // 2πn/N
                int32_t angle2 = (int32_t)(4 * Q15_ONE * n) / N;  // 4πn/N
                int32_t angle3 = (int32_t)(6 * Q15_ONE * n) / N;  // 6πn/N
                
                int16_t cos1 = cos_q15(angle1);
                int16_t cos2 = cos_q15(angle2);
                int16_t cos3 = cos_q15(angle3);
                
                int32_t window_val = a0;
                window_val -= mul_q15(a1, cos1);
                window_val += mul_q15(a2, cos2);
                window_val -= mul_q15(a3, cos3);
                
                if (window_val > Q15_ONE) window_val = Q15_ONE;
                if (window_val < 0) window_val = 0;
                
                window_coeffs[n] = (int16_t)window_val;
            }
        }
        // Bartlett window computed on-the-fly, no coefficients to store
        
        // Build bit-reversal lookup table
        uint16_t bits = log2_n();
        for (uint16_t i = 0; i < N; i++) {
            bit_reverse[i] = reverse_bits(i, bits);
        }
    }
    
    /**
     * @brief Compute FFT and return magnitudes between 0 and the sample rate / 2.
     * 
     * Each element in <code>magnitudes</code> corresponds to the sound power at a specific frequency.
     * Element <code>n</code> corresponds to frequency <code>n * (sample rate / N)</code>.
     * The magnitudes are linear power values, not in dB, or dbA (human-perceived loudness).
     * 
     * Frequencies that will leak into neighbouring  the least will be multiples of the inputs sample
     * rate divided by N.
     * E.g. for 44,100Hz sample rate and N=512, frequencies that are multiples of 86.132Hz will leak the least.
     * Musical notes lower in the scale have smaller frequency differences while musical notes higher in 
     * the scale have larger frequency differences. E.g. A0 to B0 has a delta of 3.3Hz while A7 to B7 has a delta of 2117Hz.
     * Choose N appropriately to get good resolution in the frequency range of interest.
     * 
     * @param [in] input the samples to compute FFT
     * @param [out] to store resulting magnitudes (size N/2 + 1)
     */ 
    void magnitudes(const etl::array<InputType, N>& input, etl::array<OutputType, N/2+1>& magnitudes) {
        // Working buffers on stack
        int16_t real[N];
        int16_t imag[N];
        
        // Convert input to Q15 and initialize
        for (uint16_t i = 0; i < N; i++) {
            real[i] = input_to_q15(input[i]);
            imag[i] = 0;
        }
        
        apply_window(real);
        
        // Bit-reversal permutation
        for (uint16_t i = 0; i < N; i++) {
            uint16_t j = bit_reverse[i];
            if (i < j) {
                int16_t temp = real[i];
                real[i] = real[j];
                real[j] = temp;
            }
        }
        
        // FFT butterfly computation
        uint16_t scale_count = 0;
        for (uint16_t stage = 1; stage <= log2_n(); stage++) {
            uint16_t m = 1 << stage;
            uint16_t m2 = m >> 1;
            
            // Check if we need to scale this stage to prevent overflow
            bool need_scale = false;
            for (uint16_t i = 0; i < N; i++) {
                if (real[i] > 16384 || real[i] < -16384 || 
                    imag[i] > 16384 || imag[i] < -16384) {
                    need_scale = true;
                    break;
                }
            }
            
            if (need_scale) {
                for (uint16_t i = 0; i < N; i++) {
                    real[i] >>= 1;
                    imag[i] >>= 1;
                }
                scale_count++;
            }
            
            for (uint16_t k = 0; k < N; k += m) {
                for (uint16_t j = 0; j < m2; j++) {
                    uint16_t idx = (j * N) / m;
                    
                    int16_t wr = twiddle_real[idx];
                    int16_t wi = twiddle_imag[idx];
                    
                    uint16_t i1 = k + j;
                    uint16_t i2 = i1 + m2;
                    
                    // Complex multiplication: (real[i2] + j*imag[i2]) * (wr + j*wi)
                    int16_t tr = sub_sat(mul_q15(real[i2], wr), mul_q15(imag[i2], wi));
                    int16_t ti = add_sat(mul_q15(real[i2], wi), mul_q15(imag[i2], wr));
                    
                    // Butterfly without automatic scaling
                    real[i2] = sub_sat(real[i1], tr);
                    imag[i2] = sub_sat(imag[i1], ti);
                    real[i1] = add_sat(real[i1], tr);
                    imag[i1] = add_sat(imag[i1], ti);
                }
            }
        }
        
        // Compute magnitudes based on output type
        if constexpr (std::is_same<OutputType, uint16_t>::value) {
            // Use normalization for uint16_t output
            normalize_magnitudes_uint16(real, imag, scale_count, 
                                       reinterpret_cast<etl::array<uint16_t, N/2+1>&>(magnitudes));
        } else {
            // uint32_t output - no normalization needed
            for (uint16_t i = 0; i <= N/2; i++) {
                magnitudes[i] = compute_magnitude(real[i], imag[i], scale_count);
            }
        }
    }
};

#endif // FFT_FIXED_POINT_H