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
#include <string>
#define D_IJ 4

//==============================================================================
/**
*/
class ShredVerbAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    ShredVerbAudioProcessor ();
    ~ShredVerbAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    
    // explore code of rev2~ and rev3~
    
    float G[D_IJ][D_IJ] = {
        {0.f, 1.f, 1.f, 0.f},
        {-1.f, 0.f, 0.f, -1.f},
        {1.f, 0.f, 0.f, -1.f},
        {0.f, 1.f, -1.f, 0.f}
    };
    
    nvs_delays::delay *D = NULL;
    nvs_delays::allpass_delay *apd = NULL;
    float D_times[D_IJ];

    float *X = NULL;
    float *Y = NULL;
     
    //float **G = NULL;    // gain matrix
    
    nvs_filters::tvap<float> *tvap = NULL;
    //    nvs_delays::delay *predel = NULL;
    nvs_filters::onePole<float> *lp6dB = NULL;
    
    param_stuff *getParamStuff(){
        return &ps;
    }
    
private:
    juce::AudioProcessorValueTreeState paramVT;
    param_stuff ps;

    juce::AudioProcessorValueTreeState::ParameterLayout createParams();
    
    std::atomic<float>* driveParam = nullptr;
    std::atomic<float>* predelayParam = nullptr;

    std::atomic<float>* fbParam  = nullptr;
    std::atomic<float>* sizeParam = nullptr;
    std::atomic<float>* lopParam = nullptr;
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
    
    std::atomic<float>* outputGainParam = nullptr;
    
    std::atomic<float>* interpParam = nullptr;
    std::atomic<float>* randomizeParam = nullptr;
    juce::Random rando;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ShredVerbAudioProcessor)
};
