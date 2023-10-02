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
    ~ShredVerbAudioProcessor() override; //
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
    
    // explore code of rev2~ and rev3~
    
    float G[D_IJ][D_IJ] = {
        {0.f, 1.f, 1.f, 0.f},
        {-1.f, 0.f, 0.f, -1.f},
        {1.f, 0.f, 0.f, -1.f},
        {0.f, 1.f, -1.f, 0.f}
    };
    
    nvs_delays::delay *preD = NULL;
    nvs_delays::delay *D = NULL;
    nvs_delays::allpass_delay *apd = NULL;
    float D_times_ranged[D_IJ];

    float *X = NULL;
    float *Y = NULL;
     
    //float **G = NULL;    // gain matrix
    
    nvs::filters::tvap<float> *tvap = NULL;
    //    nvs_delays::delay *predel = NULL;
    nvs::filters::onePole<float> *lp6dB = nullptr;
    
    nvs::filters::butterworth2p<double> *butter = NULL;
    
    param_stuff *getParamStuff(){
        return &ps;
    }
    
private:
    const float timeScaling = 500.f;    // multiplier for the [0..1) delay times, PRE-size parameter
    float minDelTimeMS, maxDelTimeMS;
    
    static const unsigned int versionHint = 1;
    static constexpr float intervalVal = 0.01f;
    
    static std::string getID(param_stuff::params_e idx){    return param_stuff::paramIDs.at(idx);       }
    static std::string getName(param_stuff::params_e idx){  return param_stuff::paramNames.at(idx);     }
    static float getMin(param_stuff::params_e idx){         return param_stuff::paramRanges.at(idx)[0]; }
    static float getMax(param_stuff::params_e idx){         return param_stuff::paramRanges.at(idx)[1]; }
    static float getDef(param_stuff::params_e idx){         return param_stuff::paramDefaults.at(idx);  }
    
    static std::unique_ptr<juce::AudioParameterFloat> getUniqueParam(param_stuff::params_e idx){
        return std::make_unique<juce::AudioParameterFloat>(juce::ParameterID (getID(idx), versionHint), getName(idx), juce::NormalisableRange<float> (getMin(idx), getMax(idx), intervalVal), getDef(idx));
    }

    juce::AudioProcessorValueTreeState paramVT;
    param_stuff ps;

    void addReverbParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout);
    void addDelayParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout);
    void addDistorionParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout);
    void addAllpassParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout);
    
    void addModulationParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout);

    void addOutputParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout);
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState::ParameterLayout createParams();
    
    PresetListBox*  presetList   = nullptr;

    std::atomic<float>* driveParam = nullptr;
    std::atomic<float>* predelayParam = nullptr;

    std::atomic<float>* fbParam  = nullptr;
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

    std::atomic<float>* outputGainParam = nullptr;
    
    std::atomic<float>* interpParam = nullptr;
    std::atomic<float>* randomizeParam = nullptr;
    
    juce::Random rando;
    
//    foleys::MagicProcessorState magicState ;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ShredVerbAudioProcessor)
};
