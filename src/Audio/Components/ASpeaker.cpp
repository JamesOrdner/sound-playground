#include "ASpeaker.h"
#include "../ADelayLine.h"
#include "../../Engine/EObject.h"
#include <SDL.h>

// IR partition block size
constexpr size_t blockSize = 64;
constexpr size_t N		   = blockSize * 2;

ASpeaker::ASpeaker() :
	prev(0.f),
	inputBuffer(nullptr),
	inputPtr(blockSize),
	inputFFT(nullptr),
	fdlPtr(0)
	
{
	bAcceptsInput = false;
	bAcceptsOutput = true;

	SDL_AudioSpec s;
	Uint8* data;
	Uint32 n;
	if (!SDL_LoadWAV("res/sound/auratone_8192.wav", &s, &data, &n)) {
		fprintf(stderr, "Could not open wav: %s\n", SDL_GetError());
	}
	else {
		SDL_assert(s.format == AUDIO_S16);

		partitions = s.samples / blockSize;

		float* fftIn = fftwf_alloc_real(N);
		std::fill(fftIn + blockSize, fftIn + N, 0.f); // zero padding

		fftwf_complex* fftOut = fftwf_alloc_complex(N);
		fftwf_plan p = fftwf_plan_dft_r2c_1d(N, fftIn, fftOut, FFTW_ESTIMATE);

		ir.reserve(partitions);
		inputFDL.reserve(partitions);

		Sint16* sData = (Sint16*) data;
		for (size_t i = 0; i < partitions; i++) {
			size_t offset = i * blockSize;
			for (size_t j = 0; j < blockSize; j++) {
				fftIn[j] = static_cast<float>(sData[offset + j]) / SDL_MAX_SINT16;
			}
			
			fftwf_execute(p);

			fftwf_complex* arr = fftwf_alloc_complex(N);
			std::copy_n((float*)fftOut, 2 * N, (float*)arr);
			ir.push_back(arr);

			// Create input FDL buffers
			fftwf_complex* fdlBuffer = fftwf_alloc_complex(N);
			std::fill((float*) fdlBuffer, (float*) (fdlBuffer + N), 0.f);
			inputFDL.push_back(fdlBuffer);
		}

		fftwf_destroy_plan(p);
		fftwf_free(fftIn);
		fftwf_free(fftOut);
		SDL_FreeWAV(data);

		// Input FFT
		inputBuffer = fftwf_alloc_real(N);
		inputFFT = fftwf_alloc_complex(N);
		inputPlan = fftwf_plan_dft_r2c_1d(N, inputBuffer, inputFFT, FFTW_ESTIMATE);
		std::fill(inputBuffer, inputBuffer + N, 0.f);

		// Output IFFT
		ifftInput = fftwf_alloc_complex(N);
		outputBuffer = fftwf_alloc_real(N);
		ifftPlan = fftwf_plan_dft_c2r_1d(N, ifftInput, outputBuffer, FFTW_ESTIMATE);
		std::fill(outputBuffer + blockSize, outputBuffer + N, 0.f);
	}
}

ASpeaker::~ASpeaker()
{
	fftwf_destroy_plan(inputPlan);
	fftwf_destroy_plan(ifftPlan);
	if (inputBuffer) fftwf_free(inputBuffer);
	if (inputFFT) fftwf_free(inputFFT);
	if (ifftInput) fftwf_free(ifftInput);
	if (outputBuffer) fftwf_free(outputBuffer);

	for (fftwf_complex* c : ir) fftwf_free(c);
	for (fftwf_complex* c : inputFDL) fftwf_free(c);
}

size_t ASpeaker::process(size_t n)
{
	size_t p = pushCount();
	if (p < n) n = p;
	size_t size = ir.size();
	for (const auto& output : outputs) {
		//float gain = calcGain(output->dest.lock()->position());
		for (size_t i = 0; i < n; i++) {
			float out = outputBuffer[inputPtr] / N;
			output->buffer.push(out);

			float s = static_cast<float>(rand()) / RAND_MAX * 2.f - 1.f;
			inputBuffer[inputPtr++] = s;
			if (inputPtr == N) {
				// perform fft of input
				fftwf_execute(inputPlan);
				fftwf_complex* arr = inputFDL[fdlPtr++];
				std::copy_n((float*)inputFFT, 2 * N, (float*)arr);
				if (fdlPtr == inputFDL.size()) fdlPtr = 0;

				// perform pointwise multiply and add of all FDL blocks
				std::fill((float*)ifftInput, (float*)(ifftInput + N), 0.f);
				for (size_t partition = 0; partition < partitions; partition++) {
					size_t p = (fdlPtr + partition) % partitions; // start at oldest FDL block
					for (size_t n = 0; n < N; n++) {
						for (size_t k = 0; k < 2; k++) {
							size_t irIdx = partitions - partition - 1; // last to first
							ifftInput[n][k] += inputFDL[p][n][k] * ir[irIdx][n][k];
						}
					}
				}
				fftwf_execute(ifftPlan);

				// shift samples one block to the left and reset inputPtr
				std::copy_n(inputBuffer + blockSize, blockSize, inputBuffer);
				inputPtr = blockSize;
			}

			//float filtered = (out + prev * (1.f - gain)) / (2.f / (1.f + gain)) * (gain * 0.5f + 0.5f);
			//output->buffer.push(filtered);
			//prev = out;
		}
	}
	return n;
}

float ASpeaker::calcGain(const mat::vec3& dest)
{
	using namespace mat;
	vec3 dir = normal(dest - position());
	const vec3& forward = owner.lock()->forward();
	return dot(dir, forward) * 0.5f + 0.5f;
}
