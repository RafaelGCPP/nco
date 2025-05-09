#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#define PI ((double)3.141592653589793)

// This is a very simple numeric controlled oscillator (NCO) implementation in C.
// it will generate sine and cosine values at 2400Hz for a sampling rate of 24000Hz.

#define NCO_CHUNK_SIZE 200 // the chunk size must be a multiple of the full wave size. In our case, a full wave is 10 samples.
#define NCO_FREQ 2400      // this should be an integer divisor of the sampling rate to allow for a number of full waves to be generated
#define NCO_SAMPLING_RATE 24000
#define NCO_PHASE_INC (2. * PI * (float)NCO_FREQ / (float)NCO_SAMPLING_RATE)

float sin_t[NCO_CHUNK_SIZE], cos_t[NCO_CHUNK_SIZE];
int nco_phase = 0;

void nco_init(void)
{
    for (int i = 0; i < NCO_CHUNK_SIZE; i++)
    {
        sin_t[i] = sinf(NCO_PHASE_INC * i);
        cos_t[i] = cosf(NCO_PHASE_INC * i);
        printf("sin_t[%d] = %f, cos_t[%d] = %f\n", i, sin_t[i], i, cos_t[i]);
    }
    nco_phase = 0;
}

void float_multiply(float *input1, float *input2, float *output, int size)
{
    for (int i = 0; i < size; i++)
    {
        output[i] = input1[i] * input2[i];
    }
}

void nco_mix(float *input, float *inphase, float *quadrature, int chunk_size)
{
    int start = 0;
    int remaining = NCO_CHUNK_SIZE - nco_phase;

    printf("Received chunk: %d (remains %d phase samples)\n", chunk_size, remaining);

    // if we have more than 200 samples to process, we can process them in chunks of 200
    // and we can use the current phase to start the next chunk
    // We start consuming the current NCO chunk, so the next chunk will start at 0
    while (chunk_size > remaining)
    {
        // arm_mult_f32(&input[start], &sin_t[nco_phase], inphase, remaining);
        // arm_mult_f32(&input[start], &cos_t[nco_phase], quadrature, remaining);

        printf("-- 1: Processing %d samples from %d to %d\n", remaining, start, start + remaining - 1);
        printf("-- 1: starting phase: %d\n", nco_phase);

        float_multiply(&input[start], &sin_t[nco_phase], inphase, remaining);
        float_multiply(&input[start], &cos_t[nco_phase], quadrature, remaining);

        start += remaining;
        chunk_size -= remaining;

        nco_phase = 0;
        remaining = NCO_CHUNK_SIZE;
        printf("-- 1: ending phase: %d\n", 199);
    }

    // now we have less than 200 samples to process and the current phase is 0
    // if we still have data to process, we can process it and advance the phase
    if (chunk_size > 0)
    {
        printf("-- 2: Processing %d samples from %d to %d\n", chunk_size, start, start + chunk_size - 1);
        printf("-- 2: starting phase: %d\n", nco_phase);

        float_multiply(&input[start], &sin_t[nco_phase], inphase, chunk_size);
        float_multiply(&input[start], &cos_t[nco_phase], quadrature, chunk_size);

        nco_phase += chunk_size;
        printf("-- 2: ending phase: %d\n", nco_phase - 1);
    }
    // we should never overrun the phase, but just in case we do, we reset it to 0
    if (nco_phase >= NCO_CHUNK_SIZE)
    {
        nco_phase = 0;
        printf("-- 3: reset phase\n");
    }
}

#define CHUNK 200

float input[8192];
float inphase[8192];
float quadrature[8192];
int main(int argc, char *argv[])
{
    nco_init();

    // fill the input buffer with some data
    for (int i = 0; i < 8192; i++)
    {
        input[i] = (float)1.0;
    }

    for (int i = 0; i < 8192; i += CHUNK)
    {
        int current_chunk_size = (i + CHUNK > 8192) ? (8192 - i) : CHUNK;

        nco_mix(&input[i], &inphase[i], &quadrature[i], current_chunk_size);
    }

    // check if the inphase correspond to a series of sine waves
    for (int i = 0; i < 8192; i += NCO_CHUNK_SIZE)
    {
        for (int j = 0; j < NCO_CHUNK_SIZE; j++)
        {
            if (i + j >= 8192)
                break;

            if (fabsf(inphase[i + j] - sin_t[j]) > 0.01)
            {
                printf("Error in inphase at %d: %f != %f \n", i + j, inphase[i + j], sin_t[j]);
                return -1;
            }

            if (fabsf(quadrature[i + j] - cos_t[j]) > 0.01)
            {
                printf("Error in quadrature at %d: %f != %f \n", i + j, quadrature[i + j], cos_t[j]);
                return -1;
            }
        }
    }
    return 0;
}