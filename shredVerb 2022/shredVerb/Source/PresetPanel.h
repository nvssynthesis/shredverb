/*
  ==============================================================================

    PresetPanel.h
    Created: 2 Oct 2023 10:19:59am
    Author:  Nicholas Solem

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

namespace Gui
{
using namespace juce;
class PresetPanel	:		public Component
, 							Button::Listener
, 							ComboBox::Listener
{
public:
	PresetPanel() {
		configureButton(saveButton, "Save");
		configureButton(deleteButton, "Delete");
		configureButton(previousPresetButton, "<");
		configureButton(nextPresetButton, ">");
		
		presetList.setTextWhenNothingSelected("No Preset Selected");
		presetList.setMouseCursor(MouseCursor::PointingHandCursor);
		addAndMakeVisible(presetList);
		presetList.addListener(this);
	}
	~PresetPanel(){
		saveButton.removeListener(this);
		deleteButton.removeListener(this);
		previousPresetButton.removeListener(this);
		nextPresetButton.removeListener(this);
		presetList.removeListener(this);
	}

	void resized() override {
		const auto container = getLocalBounds().reduced(4);
		auto bounds = container;
		
		saveButton.setBounds(bounds.removeFromLeft(container.proportionOfWidth(0.2f)).reduced(4));
		previousPresetButton.setBounds(bounds.removeFromLeft(container.proportionOfWidth(0.1f)).reduced(4));
		presetList.setBounds(bounds.removeFromLeft(container.proportionOfWidth(0.4f)).reduced(4));
		nextPresetButton.setBounds(bounds.removeFromLeft(container.proportionOfWidth(0.1f)).reduced(4));
		deleteButton.setBounds(bounds.removeFromLeft(container.proportionOfWidth(0.2f)).reduced(4));

	}
private:
	void buttonClicked(Button *) override {
		std::cout << "preset panel's save button clicked\n";
	}
	void comboBoxChanged(ComboBox* comboBoxThatChanged) override {}
	
	void configureButton(Button& b, const String& buttonText){
		b.setButtonText(buttonText);
		b.setMouseCursor(MouseCursor::PointingHandCursor);
		addAndMakeVisible(b);
		b.addListener(this);	// could have used onClick
	}
	
	TextButton saveButton, deleteButton, previousPresetButton, nextPresetButton;
	ComboBox presetList;
	
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetPanel)
};

class PresetPanelItem	:	public foleys::GuiItem
{
public:
	FOLEYS_DECLARE_GUI_FACTORY(PresetPanelItem);
	PresetPanelItem(foleys::MagicGUIBuilder& builder, const juce::ValueTree& node)
	: foleys::GuiItem (builder, node)
	{
		addAndMakeVisible(pp);
	}
	juce::Component* getWrappedComponent() override
	{
		return &pp;
	}
	void update() override
	{
		// set the values to your custom component
	}
private:
	PresetPanel pp;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetPanelItem)
};

//PresetPanel pp;
}
