/*
  ==============================================================================

    Shredverb.cpp
    Created: 24 Sep 2025 2:45:38pm
    Author:  Nicholas Solem

  ==============================================================================
*/

#include "Shredverb.h"
#include "params.h"

namespace nvs {

Shredverb::Shredverb()
{
	for (auto &pd : preDelays){
		pd.setInterpolation(nvs::delays::interp_e::floor);
		pd.setDelayTimeMS(param::paramDefaults.at(param::params_e::predelay));
	}
	
	D_times_ranged[0] = param::paramDefaults.at(param::params_e::time0);
	D_times_ranged[1] = param::paramDefaults.at(param::params_e::time1);
	D_times_ranged[2] = param::paramDefaults.at(param::params_e::time2);
	D_times_ranged[3] = param::paramDefaults.at(param::params_e::time3);
	
	X.fill(0.f);
	Y.fill(0.f);
	
	for (int n = 0; n < D_IJ; n++) {
		D[n].setDelayTimeMS(D_times_ranged[n] * timeScaling);
		D[n].setInterpolation(nvs::delays::interp_e::floor);
	}
	for (auto &bp : fm_bp){
		bp.setMode(nvs::filters::mode_e::BP);
	}
	for (auto &hp : hp6dB){
		hp.setMode(nvs::filters::mode_e::HP);
	}
}

void Shredverb::prepareToPlay (double sampleRate, int samplesPerBlock, int numInputChannels)
{
	maxPreDelTimeMS = float((preDelays[0].getDelaySize() - 10) * 1000) / (float)sampleRate; // no more than buffer length - 10

	maxDelTimeMS = float((D[0].getDelaySize() - 10) * 1000) / ((float)sampleRate * D[0].getLargestRatio()); // no more than buffer length - 10
	minDelTimeMS = float(3 * 1000) / (float)sampleRate; // no less than 3 samples
	
	//feed forward pair
	for (auto &pd : preDelays){
		pd.clear();
		pd.setSampleRate(sampleRate);
#pragma message("set predelay block size!")
	}
	for (int n = 0; n < D_IJ; n++) {
		tvap[n].clear();
		tvap[n].setSampleRate(sampleRate);
		tvap[n].setBlockSize(samplesPerBlock);
	}
	for (auto &bp : fm_bp){
		bp.clear();
		bp.setSampleRate(sampleRate);
		bp.setBlockSize(samplesPerBlock);
	}
	for (auto &d : D)  {
		d.clear();
		d.setSampleRate((float)sampleRate);
#pragma message("set delay block size!")
	}
	for (auto& hp : hp6dB)
	{
		hp.clear();
		hp.setSampleRate((float)sampleRate);
		hp.setBlockSize(samplesPerBlock);
	}
	for (auto &b : butters){
		b.clear();
		b.setSampleRate(sampleRate);
		b.setBlockSize(samplesPerBlock);
	}
}
void Shredverb::processBlock (juce::AudioBuffer<float>& buff, BlockwiseParams const &params)
{
	for (int i = 0; i < D_IJ; i++) {
		D_times_ranged[i] = params.times[i];
	}

	Array4 current_Dtime;
	for (int i = 0; i < D_IJ; i++) {
		current_Dtime[i] = D_times_ranged[i];
		current_Dtime[i] *= params.size;
	}

	for (int i = 0; i < D_IJ; ++i){
		float bw = params.allpassBandwidths[i];
		tvap[i].setCutoffTarget(params.allpassFreqs[i]);
		tvap[i].setResonanceTarget(bw);

		fm_bp[i].setCutoffTarget(params.allpassFreqs[i]);

		bw = nvs::memoryless::clamp_low(bw, 0.2f);
		float reso = params.allpassFreqs[i] / bw;
		fm_bp[i].setResonanceTarget(reso);
	}

	for (auto &filt : butters){
		filt.setCutoffTarget(params.lowpass);
	}
	for (auto &filt : hp6dB)  {
		filt.setCutoffTarget(params.highpass);
	}
	auto const [dryAmt, wetAmt] = [wet = params.wetMix]() {
		return std::make_pair(std::sqrt(1.0f - wet), std::sqrt(wet));
	}();
	
	int const numSamps = buff.getNumSamples();
	[[deprecated]]	// any function using this param during processBlock should be deprecated
	double const oneOverBlockSize = 1.0 / (double)numSamps;
	
	for (int samp = 0; samp < numSamps; samp++)
	{
		for (auto &pd : preDelays){
			pd.updateDelayTimeMS(params.predelay, (float)oneOverBlockSize);
		}
		for (int i = 0; i < D_IJ; i++) {
			D[i].updateDelayTimeMS(nvs::memoryless::clamp
								 (current_Dtime[i] * timeScaling, minDelTimeMS, maxDelTimeMS),
							 (float)oneOverBlockSize);
			D[i].update_g(params.delayGains[i] * params.decay, (float)oneOverBlockSize);
		}
		for (auto &filt : tvap){
			filt.update_f_pi();
			filt.update_f_b();
		}
		for (auto &filt : fm_bp){
			filt.updateCutoff();
			filt.updateResonance();
		}
		for (auto &f : hp6dB){
			f.updateCutoff();
		}
		for (auto &f : butters){
			f.updateCutoff();
		}
		//=============================================================================
		// now the actual signal path
		std::array<float const* const, 2> inBuff {
			buff.getReadPointer(0),
			// read from channel 0 twice for mono
			numInputChannels > 1 ?
				buff.getReadPointer(1) :
				buff.getReadPointer(0)
		};
		std::array<float* const, 2> outBuff {
			buff.getWritePointer(0),
			buff.getWritePointer(1)
		};
		// preserve these until end for dry/wet mixing
		std::array<float, 2> inSamps {
			*(inBuff[0] + samp),
			*(inBuff[1] + samp)
		};

		{	// scope for preDelSamp
			std::array<float, 2> preDelSamp;

			for (auto i = 0; i < preDelays.size(); ++i){
				preDelSamp[i]  = preDelays[i].tick_cubic(inSamps[i]);
			}

			X[0] = 0.f;
			X[1] = preDelSamp[0];// * inDrive;
			X[2] = preDelSamp[1];// * inDrive;
			X[3] = 0.f;
		}

		Array4 tmp {0.f, 0.f, 0.f, 0.f};

		static constexpr std::array<Array4, 4> G {{
			{0.f,  1.f,  1.f,  0.f},
			{-1.f, 0.f,  0.f, -1.f},
			{1.f,  0.f,  0.f, -1.f},
			{0.f,  1.f, -1.f,  0.f}
		}};
		
		auto const g = params.decay * 0.707106781186548;
		for (int i = 0; i < D_IJ; i++) {
			for (int j = 0; j < D_IJ; j++) {
				tmp[i] += G[i][j] * Y[j] * (g);
			}
			tmp[i] += X[i];
			for (int j = 0; j < D_IJ; ++j){
				tmp[j] = butters[j](tmp[j]);
			}
			for (int j = 0; j < D_IJ; ++j){
				tmp[j] = hp6dB[j](tmp[j]);
			}
		}
		#if CLASSIC_WAY
		tmp[0] = tvap[0].filter_fbmod(tmp[0], inner_f_pi[1], inner_f_b[0]); /*L	INTERNAL*/
		tmp[1] = tvap[1].filter_fbmod(tmp[1], outer_f_pi[0], outer_f_b[1]); /*L DIRECT*/
		tmp[2] = tvap[2].filter_fbmod(tmp[2], outer_f_pi[1], outer_f_b[0]); /*R DIRECT*/
		tmp[3] = tvap[3].filter_fbmod(tmp[3], inner_f_pi[0], inner_f_b[1]); /*R INTERNAL*/
		#else
		Array4 intrnlWcModSig = {
			nvs::memoryless::unboundSat2(fm_bp[0](tmp[0]) * 1000000000.f) * 100.f * params.drive,
			nvs::memoryless::unboundSat2(fm_bp[1](tmp[1]) * 1000000000.f) * 100.f * params.drive,
			nvs::memoryless::unboundSat2(fm_bp[2](tmp[2]) * 1000000000.f) * 100.f * params.drive,
			nvs::memoryless::unboundSat2(fm_bp[3](tmp[3]) * 1000000000.f) * 100.f * params.drive
		};
		#pragma message("need to smooth these cutoff/reso if doing it this way")
		tmp[0] = tvap[0](tmp[0], params.allpassFreqs[0] + params.distortionParams[0]*intrnlWcModSig[0], params.allpassBandwidths[0]); /*L INTERNAL*/
		tmp[1] = tvap[1](tmp[1], params.allpassFreqs[1] + params.distortionParams[1]*intrnlWcModSig[1], params.allpassBandwidths[1]); /*L DIRECT*/
		tmp[2] = tvap[2](tmp[2], params.allpassFreqs[2] + params.distortionParams[2]*intrnlWcModSig[2], params.allpassBandwidths[2]); /*R DIRECT*/
		tmp[3] = tvap[3](tmp[3], params.allpassFreqs[3] + params.distortionParams[3]*intrnlWcModSig[3], params.allpassBandwidths[3]); /*R INTERNAL*/
		#endif
		Y[0] = D[0](tmp[0]);
		Y[1] = D[1](tmp[1]);
		Y[2] = D[2](tmp[2]);
		Y[3] = D[3](tmp[3]);

		std::array<float, 2> wet {
			Y[1] * params.wetGain,// * outDrive,
			Y[2] * params.wetGain // * outDrive
		};

		std::array<float, 2> finalOut {
			dryAmt * inSamps[0] + wetAmt * wet[0],
			dryAmt * inSamps[1] + wetAmt * wet[1]
		};

		for (int i = 0; i < 2; ++i){
			*(outBuff[i] + samp) = finalOut[i] * 0.999;
		}
	} // samplewise
}


}	// namespace nvs
