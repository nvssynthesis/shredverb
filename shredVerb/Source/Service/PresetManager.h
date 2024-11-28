/*
  ==============================================================================

    PresetManager.h
    Created: 28 Oct 2023 1:39:01pm
    Author:  Nicholas Solem

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace Service
{

class PresetManager
{
public:
	static const juce::File defaultDirectory;
	static const juce::String extension;
	
	PresetManager(juce::AudioProcessorValueTreeState&);
	
	void savePreset(const juce::String& presetName);
	void deletePreset(const juce::String& presetName);
	void loadPreset(const juce::String& presetName);
	void loadNextPreset();
	void loadPreviousPreset();
	juce::StringArray getAllPresets() const;
	juce::String getCurrentPreset() const;
	
	
private:
	juce::AudioProcessorValueTreeState& apvts;
	juce::String currentPreset;
};

}
