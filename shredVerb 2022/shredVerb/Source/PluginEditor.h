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

    /*
    juce::Slider driveSlider;
    juce::AudioProcessorValueTreeState::SliderAttachment *driveSliderAttachment;
    
    juce::Slider predelaySlider;
    juce::AudioProcessorValueTreeState::SliderAttachment *predelaySliderAttachment;
    
    juce::Slider decaySlider;
    juce::AudioProcessorValueTreeState::SliderAttachment *decaySliderAttachment;
    
    juce::Slider sizeSlider;
    juce::AudioProcessorValueTreeState::SliderAttachment *sizeSliderAttachment;
    
//    juce::ComboBox interpComboBox;
//    juce::AudioProcessorValueTreeState::ComboBoxAttachment *interpComboBoxAttachment;

    juce::Slider lopSlider;
    juce::AudioProcessorValueTreeState::SliderAttachment *lopSliderAttachment;
    
    juce::Slider dryWetSlider;
    juce::AudioProcessorValueTreeState::SliderAttachment *dryWetSliderAttachment;
    
    juce::Slider allpass_fPiSlider0, allpass_fPiSlider1, allpass_fPiSlider2, allpass_fPiSlider3;
    juce::AudioProcessorValueTreeState::SliderAttachment *allpass_fPiSliderAttachment0, *allpass_fPiSliderAttachment1, *allpass_fPiSliderAttachment2, *allpass_fPiSliderAttachment3;
    
   juce::Slider allpass_fB_Slider0, allpass_fB_Slider1, allpass_fB_Slider2, allpass_fB_Slider3;
   juce::AudioProcessorValueTreeState::SliderAttachment *allpass_fB_SliderAttachment0, *allpass_fB_SliderAttachment1, *allpass_fB_SliderAttachment2, *allpass_fB_SliderAttachment3;

    juce::Slider dist1inerSlider;
    juce::Slider dist1outrSlider;
    juce::AudioProcessorValueTreeState::SliderAttachment *dist1inerSliderAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment *dist1outrSliderAttachment;
    juce::Slider dist2inerSlider;
    juce::Slider dist2outrSlider;
    juce::AudioProcessorValueTreeState::SliderAttachment *dist2inerSliderAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment *dist2outrSliderAttachment;
*/
    
    juce::TextButton randomizeButton { "Randomize" };
//    juce::AudioProcessorValueTreeState::ButtonAttachment *randomizeButtonAttachment;
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    ShredVerbAudioProcessor& processor;
    juce::AudioProcessorValueTreeState &valueTreeState;
    
    
//    juce::Label decayLabel, sizeLabel, interpTypeLabel, lopLabel, dryWelLabel, allpassLabel, dist1inerLabel, dist1outrLabel, dist2inerLabel, dist2outrLabel;
    juce::Font labelFont;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ShredVerbAudioProcessorEditor)
};
