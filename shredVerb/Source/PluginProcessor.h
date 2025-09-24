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
#include "params.h"
#include "Shredverb.h"

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
	nvs::Shredverb shredverb;
	juce::ValueTree  presetNode;

    juce::AudioProcessorValueTreeState paramVT;
	std::unordered_map<param::params_e, std::atomic<float>*> paramPtrs;
	void initializeParameterPointers();
	float getParamValue(param::params_e paramId) const;
	float getParam(param::params_e paramId) const;
	template<size_t N>
	std::array<float, N> getParamArray(const std::array<param::params_e, N>& paramIds) const;

    juce::Random rando;
	void randomizeParams();
	template<size_t N>
	void randomizeParams(std::array<param::params_e, N> params);
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
