/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

// TODO: add pre-gain, pre-delay, decouple metaparams
// independent adjustment of delay times

#pragma once

#include <JuceHeader.h>
#include "include/nvs_delayFilters.h"
#include "include/nvs_filters.h"
#include "params.h"
#include "PresetListBox.h"

#include "PresetPanel.h"

#include <string>
#include <array>
#define D_IJ 4

//==============================================================================
/**
*/
class ShredVerbAudioProcessor  :  public foleys::MagicProcessor,
                                    private juce::AudioProcessorValueTreeState::Listener
//public juce::AudioProcessor
{
public:
    //==============================================================================
    ShredVerbAudioProcessor ();
//#if DEF_EDITOR
//    ~ShredVerbAudioProcessor() override; //
//#endif
    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const juce::AudioProcessor::BusesLayout& layouts) const override;
   #endif
    void parameterChanged (const juce::String& param, float value) override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

#if DEF_EDITOR
    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;//
    bool hasEditor() const override;//

    //==============================================================================
    const juce::String getName() const override;//

    bool acceptsMidi() const override;//
    bool producesMidi() const override;//
    bool isMidiEffect() const override;//
#endif
    double getTailLengthSeconds() const override;
#if DEF_EDITOR
    //==============================================================================
    int getNumPrograms() override;//
    int getCurrentProgram() override;//
    void setCurrentProgram (int index) override;//
    const juce::String getProgramName (int index) override;//
    void changeProgramName (int index, const juce::String& newName) override;//

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;//
#endif
    void setStateInformation (const void* data, int sizeInBytes) override;//
    //==============================================================================
	
	void initialiseBuilder(foleys::MagicGUIBuilder& builder) override;
	void savePresetInternal();
	void loadPresetInternal(int index);

	// explore code of rev2~ and rev3~
    
    float G[D_IJ][D_IJ] = {
        {0.f,  1.f,  1.f,  0.f},
        {-1.f, 0.f,  0.f, -1.f},
        {1.f,  0.f,  0.f, -1.f},
        {0.f,  1.f, -1.f,  0.f}
    };
	std::array<float, D_IJ> X;
	std::array<float, D_IJ> Y;
	
    std::array<nvs::delays::Delay<32768, float>, 2> preDelays;
	struct diffusedDelay {
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
	
    std::array<diffusedDelay, D_IJ> D;
    float D_times_ranged[D_IJ];

    std::array<nvs::filters::tvap<float>, D_IJ> tvap;
	std::array<nvs::filters::svf_lin_naive<float>, D_IJ> fm_bp;
    std::array<nvs::filters::onePole<float>, D_IJ> hp6dB;
    
    std::array<nvs::filters::butterworth2p<double>, D_IJ> butters;
    
//    param_stuff *getParamStuff(){
//        return &ps;
//    }
    
private:
	static constexpr float timeScaling {2.78f};    // multiplier for the [0..1) delay times, PRE-size parameter
    float minDelTimeMS, maxDelTimeMS, maxPreDelTimeMS;

	juce::ValueTree  presetNode;

    juce::AudioProcessorValueTreeState paramVT;
//    param_stuff ps;

    void addReverbParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout);
    void addDelayParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout);
    void addDistorionParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout);
    void addAllpassParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout);
    
    void addModulationParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout);

    void addOutputParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout);
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState::ParameterLayout createParams();
    
    PresetListBox*  presetList   = nullptr;
	Gui::PresetPanel* presetPanel = nullptr;

    std::atomic<float>* driveParam = nullptr;
    std::atomic<float>* predelayParam = nullptr;

    std::atomic<float>* decayParam  = nullptr;
    std::atomic<float>* sizeParam = nullptr;
    std::atomic<float>* lopParam = nullptr;
    std::atomic<float>* hipParam = nullptr;
    std::atomic<float>* dryWetParam = nullptr;

    std::atomic<float>* allpassFrequencyPiParam0 = nullptr;
    std::atomic<float>* allpassFrequencyPiParam1 = nullptr;
    std::atomic<float>* allpassFrequencyPiParam2 = nullptr;
    std::atomic<float>* allpassFrequencyPiParam3 = nullptr;
    
    std::atomic<float>* allpassBandwidthParam0 = nullptr;
    std::atomic<float>* allpassBandwidthParam1 = nullptr;
    std::atomic<float>* allpassBandwidthParam2 = nullptr;
    std::atomic<float>* allpassBandwidthParam3 = nullptr;
    
    std::atomic<float>* dist1inerParam = nullptr;//metaparams for direct to L-R tvap outputs, controls f_pi_mod
    std::atomic<float>* dist1outrParam = nullptr;//metaparams for indirect to L-R tvap outputs, controls f_pi_mod
    std::atomic<float>* dist2inerParam = nullptr;//metaparams for direct to L-R tvap outputs, controls f_b_mod
    std::atomic<float>* dist2outrParam = nullptr;//metaparams for indirect to L-R tvap outputs, controls f_b_mod
    
    std::atomic<float>* time0Param = nullptr;
    std::atomic<float>* time1Param = nullptr;
    std::atomic<float>* time2Param = nullptr;
    std::atomic<float>* time3Param = nullptr;
	
	std::array<std::atomic<float>*, D_IJ> apdGparams {
		nullptr, nullptr, nullptr, nullptr
	};


    std::atomic<float>* outputGainParam = nullptr;
    
    std::atomic<float>* interpParam = nullptr;
    std::atomic<float>* randomizeParam = nullptr;
    
    juce::Random rando;
	void randomizeParams();
	template<size_t N>
	void randomizeParams(std::array<param_stuff::params_e, N> params);
	void randomizeDelays();
	void randomizeQualia();
	void randomizeCharacter();
	void randomizeAllpass();

//    foleys::MagicProcessorState magicState ;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ShredVerbAudioProcessor)
};
