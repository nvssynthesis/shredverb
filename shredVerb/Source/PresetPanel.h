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

namespace Gui
{
using namespace juce;
class PresetPanel	:		public Component
, 							Button::Listener
, 							ComboBox::Listener
{
public:
	PresetPanel(int i)
	{
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
	void assignPresetManager(Service::PresetManager * const pm){
		presetManager = pm;
		if (presetManager){
			auto const allPresets = presetManager->getAllPresets();
			auto const currentPreset = presetManager->getCurrentPreset();
			
			presetList.addItemList(allPresets, 1);
			presetList.setSelectedItemIndex(allPresets.indexOf(currentPreset), juce::dontSendNotification);
		}
		else {
			DBG("no preset manager!");
			jassertfalse;
			return;
		}
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
	void buttonClicked(Button *button) override {
		if (!presetManager){
			DBG("No preset manager attached");
			jassertfalse;
			return;
		}
		if (button == &saveButton){
			fileChooser = std::make_unique<juce::FileChooser>(
				"Please enter the desired preset name to save",
				Service::PresetManager::defaultDirectory,
				"*." + Service::PresetManager::extension
			);
			fileChooser->launchAsync(juce::FileBrowserComponent::saveMode,
			[&](const juce::FileChooser& chooser){
				auto const resultFile = chooser.getResult();
				presetManager->savePreset(resultFile.getFileNameWithoutExtension());
			});
		}
		if (button == &previousPresetButton){
			presetManager->loadPreviousPreset();
		}
		if (button == &nextPresetButton){
			presetManager->loadNextPreset();
		}
		if (button == &deleteButton){
#pragma message("need a warning for deleting preset here")
			presetManager->deletePreset(presetManager->getCurrentPreset());
		}
	}
	void comboBoxChanged(ComboBox* comboBoxThatChanged) override {
		if (comboBoxThatChanged == &presetList){
			presetManager->loadPreset(
									  presetList.getItemText(presetList.getSelectedItemIndex())
									  );
		}
	}
	
	void configureButton(Button& b, const String& buttonText){
		b.setButtonText(buttonText);
		b.setMouseCursor(MouseCursor::PointingHandCursor);
		addAndMakeVisible(b);
		b.addListener(this);	// could have used onClick
	}
	
	Service::PresetManager *presetManager;
	std::unique_ptr<juce::FileChooser> fileChooser;
	TextButton saveButton, deleteButton, previousPresetButton, nextPresetButton;
	ComboBox presetList;
	
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetPanel)
};

class PresetPanelItem	:	public foleys::GuiItem
{
public:
//	FOLEYS_DECLARE_GUI_FACTORY(PresetPanelItem);
//	FOLEYS_DECLARE_GUI_FACTORY with extra arg:
	static inline std::unique_ptr<foleys::GuiItem> factory (foleys::MagicGUIBuilder& builder, const juce::ValueTree& node, int i)
	{
		return std::make_unique<PresetPanelItem>(builder, node, i);
	}
	
	PresetPanelItem(foleys::MagicGUIBuilder& builder, const juce::ValueTree& node, int i)
	: foleys::GuiItem (builder, node),
	pp(i)
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
//	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetPanelItem)
};

//PresetPanel pp;
}
