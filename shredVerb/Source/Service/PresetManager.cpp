/*
  ==============================================================================

    PresetManager.cpp
    Created: 28 Oct 2023 1:39:01pm
    Author:  Nicholas Solem

  ==============================================================================
*/

#include "PresetManager.h"

namespace Service
{

const juce::File PresetManager::defaultDirectory
{ juce::File::getSpecialLocation(juce::File::SpecialLocationType::commonDocumentsDirectory).getChildFile(ProjectInfo::companyName).getChildFile(ProjectInfo::projectName)
};

const juce::String PresetManager::extension("preset");

PresetManager::PresetManager(juce::AudioProcessorValueTreeState& valueTree)
:	apvts(valueTree)
{
	// create default Preset Directory if it doesn't exist
	if (!defaultDirectory.exists()){
		auto const result = defaultDirectory.createDirectory();
		if (result.failed()){
			DBG("Could not create preset directory: " + result.getErrorMessage());
			jassertfalse;
		}
	}
}

void PresetManager::savePreset(const juce::String& presetName) {
	// validation
	if (presetName.isEmpty()){
		return;
	}
	const auto xml = apvts.copyState().createXml();
	const auto presetFile = defaultDirectory.getChildFile(presetName + "." + extension);
	if (!xml->writeTo(presetFile)){
		DBG("Could not create preset file: " + presetFile.getFullPathName());
		jassertfalse;
	}
	currentPreset = presetName;
}
void PresetManager::deletePreset(const juce::String& presetName) {
	// validation
	if (presetName.isEmpty()){
		return;
	}
	const auto presetFile = defaultDirectory.getChildFile(presetName + "."  + extension);
	if (!presetFile.existsAsFile()){
		DBG("Preset file " + presetFile.getFullPathName() + " does not exist");
		jassertfalse;
		return;
	}
	if (!presetFile.deleteFile()){
		DBG("Preset file " + presetFile.getFullPathName() + " could not be deleted");
		jassertfalse;
		return;
	}
	currentPreset = "";
	
}
void PresetManager::loadPreset(const juce::String& presetName) {
	if (presetName.isEmpty()){
		return;
	}
	const auto presetFile = defaultDirectory.getChildFile(presetName + "." + extension);
	if (!presetFile.existsAsFile()){
		DBG("Preset file " + presetFile.getFullPathName() + " does not exist");
		jassertfalse;
		return;
	}
	// preset file (XML) -> (ValueTree)
	juce::XmlDocument xmlDocument {presetFile};
	const auto valueTreeToLoad = juce::ValueTree::fromXml(*xmlDocument.getDocumentElement());
	apvts.replaceState(valueTreeToLoad);
	currentPreset = presetName;
}
void PresetManager::loadNextPreset() {
	const auto allPresets = getAllPresets();
	if (allPresets.isEmpty()){
		return;
	}
	// find index we are on
	const auto currentIndex = allPresets.indexOf(currentPreset);
	const auto nextIndex = currentIndex + 1 > (allPresets.size() - 1) ? 0 : currentIndex + 1;
	loadPreset(allPresets.getReference(nextIndex));
}
void PresetManager::loadPreviousPreset() {
	const auto allPresets = getAllPresets();
	if (allPresets.isEmpty()){
		return;
	}
	// find index we are on
	const auto currentIndex = allPresets.indexOf(currentPreset);
	const auto previousIndex = currentIndex - 1 < 0 ? (allPresets.size() - 1) : currentIndex - 1;
	loadPreset(allPresets.getReference(previousIndex));
}
juce::StringArray PresetManager::getAllPresets() const {
	juce::StringArray presets;
	const auto fileArray = defaultDirectory.findChildFiles(juce::File::TypesOfFileToFind::findFiles, false, "*." + extension);
	for (const auto& file : fileArray){
		presets.add(file.getFileNameWithoutExtension());
	}
	return presets;
}
juce::String PresetManager::getCurrentPreset() const {
	return currentPreset;
}

}
