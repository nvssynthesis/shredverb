/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

/** TODO:
 -change static size of each delay within diffusedDelay so that it doesnt waste space
 
 */
// independent adjustment of delay times

#pragma once

#include <JuceHeader.h>
#include "nvs_libraries/include/nvs_delayFilters.h"
#include "nvs_libraries/include/nvs_filters.h"
#include "params.h"

static constexpr int D_IJ {4};

//==============================================================================
class ShredVerbAudioProcessor  :  public foleys::MagicProcessor,
                                    private juce::AudioProcessorValueTreeState::Listener
{
public:
    //==============================================================================
    ShredVerbAudioProcessor ();
    //==============================================================================
#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const juce::AudioProcessor::BusesLayout& layouts) const override;
#endif
	//==============================================================================
	void prepareToPlay (double sampleRate, int samplesPerBlock) override;
	void releaseResources() override;
	//==============================================================================
	using Array4 = std::array<float, D_IJ>;
	
    void parameterChanged (const juce::String& param, float value) override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

#if DEF_EDITOR
    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;//

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
	//==============================================================================
	int getNumPrograms() override;//
	int getCurrentProgram() override;//
	void setCurrentProgram (int index) override;//
	const juce::String getProgramName (int index) override;//
	void changeProgramName (int index, const juce::String& newName) override;
#endif
    double getTailLengthSeconds() const override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;//
    void setStateInformation (const void* data, int sizeInBytes) override;//
    //==============================================================================
	
	void initialiseBuilder(foleys::MagicGUIBuilder& builder) override;

private:
	// should be reverb internals:
	static constexpr std::array<Array4, 4> G = {{
		{0.f,  1.f,  1.f,  0.f},
		{-1.f, 0.f,  0.f, -1.f},
		{1.f,  0.f,  0.f, -1.f},
		{0.f,  1.f, -1.f,  0.f}
	}};
	Array4 X;
	Array4 Y;
	
	std::array<nvs::delays::Delay<32768, float>, 2> preDelays;
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
	
	std::array<DiffusedDelay, D_IJ> D;
	float D_times_ranged[D_IJ];

	std::array<nvs::filters::tvap<float>, D_IJ> tvap;
	std::array<nvs::filters::svf_lin_naive<float>, D_IJ> fm_bp;
	std::array<nvs::filters::onePole<float>, D_IJ> hp6dB;
	
	std::array<nvs::filters::butterworth2p<double>, D_IJ> butters;
	static constexpr float timeScaling {2.78f};    // multiplier for the [0..1) delay times, PRE-size parameter
    float minDelTimeMS, maxDelTimeMS, maxPreDelTimeMS;
//==================================================================================
	juce::ValueTree  presetNode;

    juce::AudioProcessorValueTreeState paramVT;
	std::unordered_map<param_stuff::params_e, std::atomic<float>*> paramPtrs;

	void initializeParameterPointers();
	float getParamValue(param_stuff::params_e paramId) const;
	
	float getParam(param_stuff::params_e paramId) const;
	template<size_t N>
	std::array<float, N> getParamArray(const std::array<param_stuff::params_e, N>& paramIds) const;

		// Static arrays for grouped parameter IDs (cleaner than individual variables)
	 static constexpr std::array<param_stuff::params_e, 4> TVAP_F_PI_PARAMS = {
		 param_stuff::params_e::tvap0_f_pi,
		 param_stuff::params_e::tvap1_f_pi,
		 param_stuff::params_e::tvap2_f_pi,
		 param_stuff::params_e::tvap3_f_pi
	 };
	 
	 static constexpr std::array<param_stuff::params_e, 4> TVAP_F_B_PARAMS = {
		 param_stuff::params_e::tvap0_f_b,
		 param_stuff::params_e::tvap1_f_b,
		 param_stuff::params_e::tvap2_f_b,
		 param_stuff::params_e::tvap3_f_b
	 };
	 
	 static constexpr std::array<param_stuff::params_e, 4> TIME_PARAMS = {
		 param_stuff::params_e::time0,
		 param_stuff::params_e::time1,
		 param_stuff::params_e::time2,
		 param_stuff::params_e::time3
	 };
	 
	 static constexpr std::array<param_stuff::params_e, 4> DELAY_GAIN_PARAMS = {
		 param_stuff::params_e::g0,
		 param_stuff::params_e::g1,
		 param_stuff::params_e::g2,
		 param_stuff::params_e::g3
	 };
	 
	 static constexpr std::array<param_stuff::params_e, 4> DISTORTION_PARAMS = {
		 param_stuff::params_e::dist1_inner,
		 param_stuff::params_e::dist1_outer,
		 param_stuff::params_e::dist2_inner,
		 param_stuff::params_e::dist2_outer
	 };
	
    juce::Random rando;
	void randomizeParams();
	template<size_t N>
	void randomizeParams(std::array<param_stuff::params_e, N> params);
	void randomizeDelays();
	void randomizeQualia();
	void randomizeCharacter();
	void randomizeAllpass();
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ShredVerbAudioProcessor)
};


void addReverbParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout);
void addDelayParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout);
void addDistorionParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout);
void addAllpassParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout);

void addModulationParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout);

void addOutputParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout);
juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
