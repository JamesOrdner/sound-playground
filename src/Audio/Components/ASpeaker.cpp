#include "ASpeaker.h"
#include "../../Engine/EObject.h"
#include "../DSP/ADelayLine.h"
#include "../AWAVFile.h"

// IR partition block size
constexpr size_t blockSize = 256;
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
	if (!inputBuffer || !outputBuffer) return;
	std::fill_n(inputBuffer, N, 0.f);
	std::fill_n(outputBuffer + blockSize, blockSize, 0.f);

	AWAVFile wav("res/sound/auratone_8192.wav");
	if (wav.data.size() == 0 || wav.channels != 1) {
		fprintf(stderr, "Incompatible wav file.\n");
		return;
	}

	partitions = wav.data.size() / blockSize;

	float* fftIn = fftwf_alloc_real(N);
	fftwf_complex* fftOut = fftwf_alloc_complex(N);
	fftwf_plan p = fftwf_plan_dft_r2c_1d(N, fftIn, fftOut, FFTW_ESTIMATE);
	std::fill(fftIn + blockSize, fftIn + N, 0.f); // zero pad input

	ir.reserve(partitions);
	freqDelayLine.reserve(partitions);

	for (size_t i = 0; i < partitions; i++) {
		std::copy_n(&wav.data[0] + blockSize * i, blockSize, fftIn);
		fftwf_execute(p);

		// push IR FFT
		fftwf_complex* arr = fftwf_alloc_complex(blockSize + 1);
		std::copy_n(&fftOut[0][0], (blockSize + 1) * 2, &arr[0][0]);
		ir.push_back(arr);

		// push input FDL buffer
		fftwf_complex* fdlBuffer = fftwf_alloc_complex(blockSize + 1);
		std::fill_n(&fdlBuffer[0][0], (blockSize + 1) * 2, 0.f);
		freqDelayLine.push_back(fdlBuffer);
	}

	fftwf_destroy_plan(p);
	fftwf_free(fftIn);
	fftwf_free(fftOut);

	// complex output buffer of input FFT
	fftOutput = fftwf_alloc_complex(N);
	fftPlan = fftwf_plan_dft_r2c_1d(N, inputBuffer, fftOutput, FFTW_ESTIMATE);

	// complex input buffer to output FFT
	ifftInput = fftwf_alloc_complex(N);
	ifftPlan = fftwf_plan_dft_c2r_1d(N, ifftInput, outputBuffer, FFTW_ESTIMATE);
	std::fill_n(&ifftInput[0][0] + (blockSize + 1) * 2, (blockSize - 1) * 2, 0.f);
}

ASpeaker::~ASpeaker()
{
	if (ifftPlan) fftwf_destroy_plan(ifftPlan);
	if (ifftInput) fftwf_free(ifftInput);
	if (fftPlan) fftwf_destroy_plan(fftPlan);
	if (fftOutput) fftwf_free(fftOutput);
	if (outputBuffer) fftwf_free(outputBuffer);
	if (inputBuffer) fftwf_free(inputBuffer);

	for (fftwf_complex* c : ir) fftwf_free(c);
	for (fftwf_complex* c : freqDelayLine) fftwf_free(c);
}

size_t t = 0;

size_t ASpeaker::process(size_t n)
{
	for (size_t i = 0; i < n; i++) {
		for (const auto& output : outputs) if (!output->pushable()) return i;

		float gen = sinf(t++ * 2.f * mat::pi * 500.f / sampleRate);
		/// float gen = static_cast<float>(rand()) / RAND_MAX * 2.f - 1.f;
		inputBuffer[inputPtr] = gen;
		float out = outputBuffer[inputPtr++] / N;

		if (inputPtr == N) {
			inputPtr = blockSize;
			if (fftPlan) {
				fftwf_execute(fftPlan);

				fftwf_complex* fdlBank = freqDelayLine[fdlPtr++];
				std::copy_n(&fftOutput[0][0], (blockSize + 1) * 2, &fdlBank[0][0]);
				if (fdlPtr == freqDelayLine.size()) fdlPtr = 0;

				// pointwise multiply and add all FDL blocks
				std::fill_n(&ifftInput[0][0], (blockSize + 1) * 2, 0.f);
				for (size_t p = 0; p < partitions; p++) {
					size_t fdlIdx = (fdlPtr + p) % partitions;
					size_t irIdx = partitions - p - 1;
					for (size_t n = 0; n <= blockSize; n++) { // n < blockSize + 1
						float a = freqDelayLine[fdlIdx][n][0];
						float b = freqDelayLine[fdlIdx][n][1];
						float c = ir[irIdx][n][0];
						float d = ir[irIdx][n][1];
						float real = a * c - b * d;
						float imag = (a + b) * (c + d) - a * c - b * d;
						ifftInput[n][0] += real;
						ifftInput[n][1] += imag;
					}
				}

				// fill outputBuffer
				fftwf_execute(ifftPlan);

				// shift samples one block to the left
				std::copy_n(inputBuffer + blockSize, blockSize, inputBuffer);
			}
		}

		for (const auto& output : outputs) output->push(&out);
	}
	
	return n;
}

float ASpeaker::calcGain(const mat::vec3& dest)
{
	using namespace mat;
	vec3 dir = normal(dest - position());
	return dot(dir, forward()) * 0.5f + 0.5f;
}
