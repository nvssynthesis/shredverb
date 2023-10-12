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
	class PresetPanel	:	public Component, Button::Listener
	{
	public:
		PresetPanel() {
			saveButton.setButtonText("Save");
			saveButton.setMouseCursor(MouseCursor::PointingHandCursor);
			addAndMakeVisible(saveButton);
			saveButton.addListener(this);
		}
		
		void buttonClicked(Button *) override {
			std::cout << "preset panel's save button clicked\n";
		}
		
	private:
		TextButton saveButton, deleteButton, previousPresetButton, nextPresetButton;
		ComboBox presetList;
		
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetPanel)
	};
}
