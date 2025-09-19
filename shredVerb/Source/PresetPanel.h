/*
  ==============================================================================

    PresetPanel.h
    Created: 2 Oct 2023 10:19:59am
    Author:  Nicholas Solem

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "Service/PresetManager.h"

namespace nvs::gui {

class PresetPanel
:	public juce::Component
,	juce::Button::Listener
,	juce::ComboBox::Listener
{
public:
	PresetPanel();
	~PresetPanel();
	void assignPresetManagerAndInit(service::PresetManager *);
	void loadPresetList();
	
	void comboBoxChanged(juce::ComboBox *cb) override;
	
	void configureButton(juce::Button &b, juce::String const& buttonText);
	void paint(juce::Graphics &g) override;
	void resized() override;
	
private:
	void buttonClicked(juce::Button *b) override;
	service::PresetManager *_presetManager;
	
	juce::TextButton saveButton, deleteButton, previousPresetButton, nextPresetButton;
	juce::ComboBox presetListBox;
	
	std::unique_ptr<juce::FileChooser> fileChooser;
};

}
