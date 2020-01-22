#include "AConvolver.h"
#include "../AWAVFile.h"

// IR partition block size
constexpr size_t blockSize = 256;
constexpr size_t N = blockSize * 2;

AConvolver::AConvolver() :
	partitions(0),
	inputBuffer(nullptr),
	fftResult(nullptr),
	ifftInput(nullptr),
	outputBuffer(nullptr),
	inputPtr(0),
	fdlPtr(0),
	fftPlan(nullptr),
	ifftPlan(nullptr)
{
	bAcceptsInput = true;
	bCanProcessInPlace = true;
}

AConvolver::AConvolver(std::string impulseResponseFilepath) : AConvolver()
{
	setIR(impulseResponseFilepath);
}

void AConvolver::setIR(const std::vector<float>& newIR)
{
	impulseResponse = newIR;
	size_t blocks = impulseResponse.size() / blockSize;
	if (impulseResponse.size() % blockSize) blocks++;
	impulseResponse.resize(blocks * blockSize); // zero pad last block

	if (bInitialized) loadIR();
}

void AConvolver::setIR(std::string filepath)
{
	AWAVFile wav(filepath);
	if (wav.data.empty() || wav.channels != 1) {
		fprintf(stderr, "Incompatible wav file.\n");
	}
	else {
		setIR(wav.data);
	}
}

void AConvolver::init(float sampleRate)
{
	ADSPBase::init(sampleRate);
	loadIR();
}

void AConvolver::deinit()
{
	ADSPBase::deinit();
	unloadIR();
}

void AConvolver::loadIR()
{
	unloadIR();

	partitions = impulseResponse.size() / blockSize;
	if (!partitions) return;

	impulseResponseFFTs.reserve(partitions);
	freqDelayLine.reserve(partitions);

	float* fftIn = fftwf_alloc_real(N);
	complex* fftOut = reinterpret_cast<complex*>(fftwf_alloc_complex(N));
	if (!assert(fftIn && fftOut)) return;
	fftwf_plan p = fftwf_plan_dft_r2c_1d(N, fftIn, reinterpret_cast<fftwf_complex*>(fftOut), FFTW_ESTIMATE);
	std::fill_n(fftIn + blockSize, blockSize, 0.f); // zero pad input

	for (size_t i = 0; i < partitions; i++) {
		std::copy_n(impulseResponse.data() + blockSize * i, blockSize, fftIn);
		fftwf_execute(p);

		// save IR FFT partition
		complex* arr = reinterpret_cast<complex*>(fftwf_alloc_complex(blockSize + 1));
		if (!assert(arr)) return;
		std::copy_n(&fftOut[0], blockSize + 1, &arr[0]);
		impulseResponseFFTs.push_back(arr);

		// push input FDL buffer
		complex* fdlBuffer = reinterpret_cast<complex*>(fftwf_alloc_complex(blockSize + 1));
		if (!assert(fdlBuffer)) return;
		std::fill_n(&fdlBuffer[0], blockSize + 1, 0.f);
		freqDelayLine.push_back(fdlBuffer);
	}

	fftwf_destroy_plan(p);
	fftwf_free(fftIn);
	fftwf_free(fftOut);

	// Input buffer filled from process() input
	inputBuffer = fftwf_alloc_real(N);
	if (!assert(inputBuffer)) return;

	// Result of the real to complex FFT on the input stream
	fftResult = reinterpret_cast<complex*>(fftwf_alloc_complex(N));
	if (!assert(fftResult)) return;

	// Result and final output of the IFFT
	outputBuffer = fftwf_alloc_real(N);
	if (!assert(outputBuffer)) return;

	// Input to the IFFT
	ifftInput = reinterpret_cast<complex*>(fftwf_alloc_complex(N));
	if (!assert(ifftInput)) return;

	fftPlan = fftwf_plan_dft_r2c_1d(
		N,
		inputBuffer,
		reinterpret_cast<fftwf_complex*>(fftResult),
		FFTW_MEASURE | FFTW_PRESERVE_INPUT);

	ifftPlan = fftwf_plan_dft_c2r_1d(
		N,
		reinterpret_cast<fftwf_complex*>(ifftInput),
		outputBuffer,
		FFTW_MEASURE | FFTW_DESTROY_INPUT);

	std::fill_n(inputBuffer, blockSize, 0.f);
	std::fill_n(outputBuffer + blockSize, blockSize, 0.f);

	inputPtr = blockSize;
	fdlPtr = 0;
}

void AConvolver::unloadIR()
{
	partitions = 0;
	for (complex* c : impulseResponseFFTs) fftwf_free(reinterpret_cast<fftwf_complex*>(c));
	for (complex* c : freqDelayLine) fftwf_free(reinterpret_cast<fftwf_complex*>(c));
	if (inputBuffer) fftwf_free(inputBuffer);
	if (fftResult) fftwf_free(fftResult);
	if (ifftInput) fftwf_free(ifftInput);
	if (outputBuffer) fftwf_free(outputBuffer);
	if (fftPlan) fftwf_destroy_plan(fftPlan);
	if (ifftPlan) fftwf_destroy_plan(ifftPlan);
	impulseResponseFFTs.clear();
	freqDelayLine.clear();
	inputBuffer = nullptr;
	fftResult = nullptr;
	ifftInput = nullptr;
	outputBuffer = nullptr;
	fftPlan = nullptr;
	ifftPlan = nullptr;
}

void AConvolver::process(float* outbuffer, const float* inbuffer, size_t n)
{
	if (!partitions) {
		std::copy_n(inbuffer, n, outbuffer);
		return;
	}

	for (size_t i = 0; i < n; i++) {
		inputBuffer[inputPtr] = inbuffer[i];
		outbuffer[i] = outputBuffer[inputPtr++] / N;

		if (inputPtr == N) {
			inputPtr = blockSize;
			if (fftPlan) {
				fftwf_execute(fftPlan);

				complex* fdlBank = freqDelayLine[fdlPtr++];
				std::copy_n(&fftResult[0], blockSize + 1, &fdlBank[0]);
				if (fdlPtr == freqDelayLine.size()) fdlPtr = 0;

				// pointwise multiply and add all FDL blocks
				std::fill_n(&ifftInput[0], blockSize + 1, 0.f);
				for (size_t p = 0; p < partitions; p++) {
					size_t fdlIdx = (fdlPtr + p) % partitions;
					size_t irIdx = partitions - p - 1;
					for (size_t n = 0; n <= blockSize; n++) { // n < blockSize + 1
						ifftInput[n] += freqDelayLine[fdlIdx][n] * impulseResponseFFTs[irIdx][n];
					}
				}

				// fill outputBuffer
				fftwf_execute(ifftPlan);

				// shift input samples one block to the left
				std::copy_n(inputBuffer + blockSize, blockSize, inputBuffer);
			}
		}
	}
}

bool AConvolver::assert(bool condition)
{
	if (!condition) deinit();
	return condition;
}
