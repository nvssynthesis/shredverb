/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/

class Shredverb_fromscratchAudioProcessorEditor  : public juce::AudioProcessorEditor
,                                                    private juce::Button::Listener
,                                                    private juce::Slider::Listener
,                                                    private juce::ComboBox::Listener
{
public:
    Shredverb_fromscratchAudioProcessorEditor (Shredverb_fromscratchAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~Shredverb_fromscratchAudioProcessorEditor() override;

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
    
    juce::Slider allpassSlider0, allpassSlider1, allpassSlider2, allpassSlider3;
    juce::AudioProcessorValueTreeState::SliderAttachment *allpassSliderAttachment0, *allpassSliderAttachment1, *allpassSliderAttachment2, *allpassSliderAttachment3;
    
    
    juce::Slider dist1inerSlider;
    juce::Slider dist1outrSlider;
    juce::AudioProcessorValueTreeState::SliderAttachment *dist1inerSliderAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment *dist1outrSliderAttachment;
    juce::Slider dist2inerSlider;
    juce::Slider dist2outrSlider;
    juce::AudioProcessorValueTreeState::SliderAttachment *dist2inerSliderAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment *dist2outrSliderAttachment;

    
    juce::TextButton randomizeButton { "Randomize" };
    juce::AudioProcessorValueTreeState::ButtonAttachment *randomizeButtonAttachment;
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    Shredverb_fromscratchAudioProcessor& processor;
    juce::AudioProcessorValueTreeState &valueTreeState;
    
    
    juce::Label decayLabel, sizeLabel, interpTypeLabel, lopLabel, dryWelLabel, allpassLabel, dist1inerLabel, dist1outrLabel, dist2inerLabel, dist2outrLabel;
    juce::Font labelFont;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Shredverb_fromscratchAudioProcessorEditor)
};
