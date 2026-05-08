/*
  ==============================================================================

    Shredverb.h
    Created: 24 Sep 2025 2:45:38pm
    Author:  Nicholas Solem

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "nvs_libraries/include/nvs_delayFilters.h"
#include "nvs_libraries/include/nvs_filters.h"

namespace nvs {
static constexpr int D_IJ {4};

class Shredverb
{
public:
	Shredverb();
	
	using Array4 = std::array<float, D_IJ>;
	struct BlockwiseParams
	{
		float predelay, size, decay, lowpass, highpass, drive, wetMix, wetGain;
		Array4 times, delayGains,
			allpassFreqs, allpassBandwidths,
			distortionParams;
	};
	
	void prepareToPlay (double sampleRate, int samplesPerBlock, int numInputChannels);
	void processBlock (juce::AudioBuffer<float>& buff, BlockwiseParams const &params);

private:
	int numInputChannels;
	static constexpr int numOutputChannels {2};
	
	struct DiffusedDelay {
		void setSampleRate(float sampleRate){
			for (auto &d : delays){
				d.setSampleRate(sampleRate);
			}
		}
		void clear(){
			for (auto &d : delays){
				d.clear();
			}
		}
		void setDelayTimeMS(double t){
			for (int i = 0; i < n_delays; ++i){
				delays[i].setDelayTimeMS(t * ratios[i]);
			}
		}
		[[deprecated]]
		void updateDelayTimeMS(float target, float oneOverBlockSize){
			for (int i = 0; i < n_delays; ++i){
				delays[i].updateDelayTimeMS(target * ratios[i], oneOverBlockSize);
			}
		}
		void setInterpolation(nvs::delays::interp_e interp){
			for (auto &d : delays){
				d.setInterpolation(interp);
			}
		}
		void update_g(float g_target, float oneOverBlockSize){
			for (auto &d : delays){
				d.update_g(g_target, oneOverBlockSize);
			}
		}
		unsigned int getDelaySize() const {
			return delays[0].getDelaySize();
		}
		float operator()(float inp){
			float val = delays[3].filter(delays[2].filter(delays[1].filter(delays[0].filter(inp))));
			return val;
		}
		float getLargestRatio() const {
			return ratios.back();
		}
	private:
		static constexpr size_t n_delays {4};
		std::array<nvs::delays::AllpassDelay<8192, float>, n_delays> delays;
		std::array<float, n_delays> ratios {
			5.f, 13.f, 23.f, 53.f
		};
	};
	
	std::array<nvs::delays::Delay<32768, float>, 2> preDelays;

	std::array<DiffusedDelay, D_IJ> D;
	float D_times_ranged[D_IJ];
	
	std::array<nvs::filters::tvap<float>, D_IJ> tvap;
	std::array<nvs::filters::svf_lin_naive<float>, D_IJ> fm_bp;
	
	std::array<nvs::filters::butterworth2p<double>, D_IJ> LPbutters;
	std::array<nvs::filters::butterworth2p<double>, D_IJ> HPbutters;
	static constexpr float timeScaling {2.78f};    // multiplier for the [0..1) delay times, PRE-size parameter
	float minDelTimeMS, maxDelTimeMS, maxPreDelTimeMS;
	
	Array4 Y;
	//==================================================================================
};

}	// namespace nvs
