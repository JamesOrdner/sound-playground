#include "ASpeaker.h"
#include "../ADelayLine.h"
#include "../../Engine/EObject.h"
#include <SDL.h>

// IR partition block size
constexpr size_t blockSize = 64;
constexpr size_t N		   = blockSize * 2;

ASpeaker::ASpeaker() :
	partitions(0),
	inputBuffer(nullptr),
	inputPtr(blockSize),
	fftOutput(nullptr),
	fftPlan(nullptr),
	fdlPtr(0),
	ifftInput(nullptr),
	outputBuffer(nullptr),
	ifftPlan(nullptr),
	prev(0.f)
{
	bAcceptsInput = false;
	bAcceptsOutput = true;

	// These should be set up even if wav load fails
	inputBuffer = fftwf_alloc_real(N);
	outputBuffer = fftwf_alloc_real(N);
	std::fill_n(inputBuffer, N, 0.f);
	std::fill_n(outputBuffer + blockSize, blockSize, 0.f);

	SDL_AudioSpec s;
	Uint8* data;
	Uint32 n;
	if (!SDL_LoadWAV("res/sound/auratone_8192.wav", &s, &data, &n)) {
		fprintf(stderr, "Could not open wav: %s\n", SDL_GetError());
		return;
	}
	if (s.format != AUDIO_S16) {
		fprintf(stderr, "Invalid wav format! Expected signed 16 bit.\n");
		return;
	}

	partitions = s.samples / blockSize;

	float* fftIn = fftwf_alloc_real(N);
	std::fill(fftIn + blockSize, fftIn + N, 0.f); // zero padding

	fftwf_complex* fftOut = fftwf_alloc_complex(N);
	fftwf_plan p = fftwf_plan_dft_r2c_1d(N, fftIn, fftOut, FFTW_ESTIMATE);

	ir.reserve(partitions);
	freqDelayLine.reserve(partitions);

	Sint16* sData = (Sint16*) data;
	for (size_t i = 0; i < partitions; i++) {
		size_t offset = i * blockSize;
		for (size_t j = 0; j < blockSize; j++) {
			fftIn[j] = static_cast<float>(sData[offset + j]) / SDL_MAX_SINT16;
		}
			
		fftwf_execute(p);

		fftwf_complex* arr = fftwf_alloc_complex(blockSize + 1);
		std::copy_n(&fftOut[0][0], 2 * (blockSize + 1), &arr[0][0]);
		ir.push_back(arr);

		// Create input FDL buffers
		fftwf_complex* fdlBuffer = fftwf_alloc_complex(blockSize + 1);
		std::fill_n(&fdlBuffer[0][0], 2 * (blockSize + 1), 0.f);
		freqDelayLine.push_back(fdlBuffer);
	}

	fftwf_destroy_plan(p);
	fftwf_free(fftIn);
	fftwf_free(fftOut);
	SDL_FreeWAV(data);

	// Input FFT
	fftOutput = fftwf_alloc_complex(N);
	fftPlan = fftwf_plan_dft_r2c_1d(N, inputBuffer, fftOutput, FFTW_ESTIMATE);

	// Output IFFT
	ifftInput = fftwf_alloc_complex(N);
	std::fill(&(ifftInput + blockSize + 1)[0][0], &(ifftInput + N)[0][0], 0.f);
	ifftPlan = fftwf_plan_dft_c2r_1d(N, ifftInput, outputBuffer, FFTW_ESTIMATE);
}

ASpeaker::~ASpeaker()
{
	if (fftPlan) fftwf_destroy_plan(fftPlan);
	if (ifftPlan) fftwf_destroy_plan(ifftPlan);
	if (inputBuffer) fftwf_free(inputBuffer);
	if (fftOutput) fftwf_free(fftOutput);
	if (ifftInput) fftwf_free(ifftInput);
	if (outputBuffer) fftwf_free(outputBuffer);

	for (fftwf_complex* c : ir) fftwf_free(c);
	for (fftwf_complex* c : freqDelayLine) fftwf_free(c);
}

size_t t = 0;

size_t ASpeaker::process(size_t n)
{
	size_t p = pushCount();
	if (p < n) n = p;
	size_t size = ir.size();
	for (const auto& output : outputs) {
		float gain = calcGain(output->dest.lock()->position());
		for (size_t i = 0; i < n; i++) {
			float out = outputBuffer[inputPtr] / N;
			float s = static_cast<float>(rand()) / RAND_MAX * 2.f - 1.f;
			//inputBuffer[inputPtr++] = s;

			//if (inputPtr == N) {
			//	inputPtr = blockSize;
			//	if (fftPlan) {
			//		// perform fft of input
			//		fftwf_execute(fftPlan);
			//		fftwf_complex* arr = freqDelayLine[fdlPtr++];
			//		std::copy_n(&fftOutput[0][0], 2 * (blockSize + 1), &arr[0][0]);
			//		if (fdlPtr == freqDelayLine.size()) fdlPtr = 0;

			//		// perform pointwise multiply and add of all FDL blocks
			//		std::fill_n((float*)ifftInput[0], 2 * (blockSize + 1), 0.f);
			//		for (size_t partition = 0; partition < partitions; partition++) {
			//			size_t p = (fdlPtr + partition) % partitions; // start at oldest FDL block
			//			for (size_t n = 0; n <= blockSize; n++) {
			//				for (size_t k = 0; k < 2; k++) {
			//					size_t irIdx = partitions - partition - 1; // last to first
			//					ifftInput[n][k] += freqDelayLine[p][n][k] * ir[irIdx][n][k];
			//				}
			//			}
			//		}
			//		fftwf_execute(ifftPlan);

			//		// shift samples one block to the left
			//		std::copy_n(inputBuffer + blockSize, blockSize, inputBuffer);
			//	}
			//}

			//float filtered = (out + prev * (1.f - gain)) / (2.f / (1.f + gain)) * (gain * 0.5f + 0.5f);
			//output->buffer.push(filtered);
			//prev = out;
			
			output->buffer.push(sinf(t++ * 2.f * mat::pi * 1000.f / sampleRate));
		}
	}
	return n;
}

float ASpeaker::calcGain(const mat::vec3& dest)
{
	using namespace mat;
	vec3 dir = normal(dest - position());
	return dot(dir, forward()) * 0.5f + 0.5f;
}
