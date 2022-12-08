/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/
// TODO: make all slider attachment pointers unique_ptr. this will allow to save memory while plugin window is closed.


#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/

class ShredVerbAudioProcessorEditor  : public juce::AudioProcessorEditor
,                                                    private juce::Button::Listener
,                                                    private juce::Slider::Listener
,                                                    private juce::ComboBox::Listener
{
public:
    ShredVerbAudioProcessorEditor (ShredVerbAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~ShredVerbAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    //==== override from Button::Listener ==================================================
    void buttonClicked (juce::Button*) override {}
    /** Called when the button's state changes. */
    //void buttonStateChanged (juce::Button*) override {}
    
    //==== override from Slider::Listener ==================================================
    void sliderValueChanged (juce::Slider* slider) override {}
    //==============================================================================
    /** Called when the slider is about to be dragged.
     
     This is called when a drag begins, then it's followed by multiple calls
     to sliderValueChanged(), and then sliderDragEnded() is called after the
     user lets go.
     
     @see sliderDragEnded, Slider::startedDragging
     */
    void sliderDragStarted (juce::Slider*) override {}
    
    /** Called after a drag operation has finished.
     @see sliderDragStarted, Slider::stoppedDragging
     */
    void sliderDragEnded (juce::Slider*) override {}
    //==============================================================================
    void comboBoxChanged (juce::ComboBox* comboBoxThatHasChanged) override {}
    
    juce::TextButton randomizeButton { "Randomize" };
//    juce::AudioProcessorValueTreeState::ButtonAttachment *randomizeButtonAttachment;
    juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> ownedSliderAttachmentPtrs;
    
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    ShredVerbAudioProcessor& processor;
    juce::AudioProcessorValueTreeState &valueTreeState;
    
    
//    juce::Label decayLabel, sizeLabel, interpTypeLabel, lopLabel, dryWelLabel, allpassLabel, dist1inerLabel, dist1outrLabel, dist2inerLabel, dist2outrLabel;
    juce::Font labelFont;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ShredVerbAudioProcessorEditor)
};
