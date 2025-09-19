/*
  ==============================================================================

    PresetPanel.cpp
    Created: 18 Sep 2025 10:43:10pm
    Author:  Nicholas Solem

  ==============================================================================
*/

#include "PresetPanel.h"

namespace nvs::gui
{

PresetPanel::PresetPanel(service::PresetManager *presetManager)
:	_presetManager(presetManager)
{
	configureButton(saveButton, "Save");
	configureButton(deleteButton, "Delete");
	configureButton(previousPresetButton, "<");
	configureButton(nextPresetButton, ">");
	
	presetListBox.setTextWhenNothingSelected("No Preset Selected");
	presetListBox.setMouseCursor(juce::MouseCursor::PointingHandCursor);
	addAndMakeVisible(presetListBox);
	presetListBox.addListener(this);
	
	loadPresetList();
}
PresetPanel::~PresetPanel() {
	saveButton.removeListener(this);
	deleteButton.removeListener(this);
	previousPresetButton.removeListener(this);
	nextPresetButton.removeListener(this);
	presetListBox.removeListener(this);
}

void PresetPanel::loadPresetList() {
	presetListBox.clear(juce::dontSendNotification);
	
	const auto allPresets = _presetManager->getAllPresets();
	const auto currentPreset = _presetManager->getCurrentPreset();
	presetListBox.addItemList(allPresets, 1);
	presetListBox.setSelectedItemIndex(allPresets.indexOf(currentPreset), juce::dontSendNotification);
}

void PresetPanel::comboBoxChanged(juce::ComboBox *cb) {
	if (cb == &presetListBox){
		_presetManager->loadPreset(presetListBox.getItemText(presetListBox.getSelectedItemIndex()));
	}
}

void PresetPanel::configureButton(juce::Button &b, juce::String const& buttonText){
	b.setButtonText(buttonText);
	b.setMouseCursor(juce::MouseCursor::PointingHandCursor);
	addAndMakeVisible(b);
	b.addListener(this);
}
void PresetPanel::paint(juce::Graphics &g) {
	using namespace juce;
	using Point = Point<float>;
	auto const p0 = Point(getX(), getY());
	auto const p1 = p0 + Point(getWidth(), getHeight());
	g.setGradientFill(ColourGradient(Colours::lightgrey.withMultipliedAlpha(0.1f), p0,
									 Colours::grey.withMultipliedAlpha(0.3f), p1, false));
	g.fillRect(getLocalBounds());
}
void PresetPanel::resized() {
	using namespace juce;
	auto area = getLocalBounds().toFloat();
	
	// set up flex container
	FlexBox fb;
	fb.flexDirection = FlexBox::Direction::row;
	fb.alignItems    = FlexBox::AlignItems::stretch;
	fb.alignContent  = FlexBox::AlignContent::spaceAround;
	
	float margin = 4.0f;
	// add your controls with the same relative widths and 1px margin
	fb.items = {
		FlexItem (saveButton)               .withFlex (0.1f).withMargin (margin),
		FlexItem (previousPresetButton)     .withFlex (0.05f).withMargin (margin),
		FlexItem (presetListBox)            .withFlex (0.6f).withMargin (margin),
		FlexItem (nextPresetButton)         .withFlex (0.05f).withMargin (margin),
		FlexItem (deleteButton)             .withFlex (0.1f).withMargin (margin)
	};
	
	// perform the layout in our component's bounds
	fb.performLayout (area);
}
void PresetPanel::buttonClicked(juce::Button *b) {
	if (b == &saveButton) {
		fileChooser = std::make_unique<juce::FileChooser>(
			"Enter preset name to save...",
			service::PresetManager::defaultDirectory,
			"*." + service::PresetManager::extension
		);
		fileChooser->launchAsync(juce::FileBrowserComponent::saveMode, [&](const juce::FileChooser &fc)
		{
			const auto resultFile = fc.getResult();
			_presetManager->savePreset(resultFile.getFileNameWithoutExtension());
			loadPresetList();
		});
	}
	if (b == &previousPresetButton){
		const auto idx = _presetManager->loadPreviousPreset();
		presetListBox.setSelectedItemIndex(idx, juce::dontSendNotification);
	}
	if (b == &nextPresetButton){
		const auto idx = _presetManager->loadNextPreset();
		presetListBox.setSelectedItemIndex(idx, juce::dontSendNotification);
	}
	if (b == &deleteButton){
		[[maybe_unused]]
		bool const button1Clicked = juce::AlertWindow::showOkCancelBox(
			juce::AlertWindow::WarningIcon,
			"Delete Preset",
			"Are you sure you want to delete the preset '" +
			_presetManager->getCurrentPreset() + "'?\n\nThis action cannot be undone.",
			"Delete",
			"Cancel",
			this,
			juce::ModalCallbackFunction::create([this](int result) {
				if (result == 1) { // User clicked "Delete"
					_presetManager->deletePreset(_presetManager->getCurrentPreset());
					loadPresetList();
				}
				// If result == 0, user clicked "Cancel" - do nothing
			})
		);
	}
}

}	// namespace nvs::gui
