/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#define WIDTH 800
#define HEIGHT 500
#define NPARAMS 4

//==============================================================================
Shredverb_fromscratchAudioProcessorEditor::Shredverb_fromscratchAudioProcessorEditor (Shredverb_fromscratchAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    //: AudioProcessorEditor (&p), audioProcessor (p)
    : AudioProcessorEditor (&p), processor (p), valueTreeState(vts)
{
    juce::Colour thumbColour = juce::Colours::darkviolet;
    juce::Colour fillColour = juce::Colours::dimgrey;
    
    addAndMakeVisible(&decaySlider);
    decaySlider.addListener(this);
    //decaySlider.setRange(0.f, 1.5f);
    decaySlider.setValue(0.f/*processor.fbParam*/);
    decaySlider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    decaySlider.setColour(juce::Slider::ColourIds::thumbColourId, thumbColour);
    decaySlider.setColour(juce::Slider::ColourIds::rotarySliderFillColourId, fillColour);
    decaySliderAttachment = new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "feedbackGain", decaySlider);
    
    addAndMakeVisible(&sizeSlider);
    sizeSlider.addListener(this);
    //sizeSlider.setRange(0.f, 1.f);
    sizeSlider.setValue(0.f/*processor.fbParam*/);
    sizeSlider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    sizeSlider.setSkewFactor(0.005, false);
    sizeSliderAttachment = new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "size", sizeSlider);
    
    addAndMakeVisible(&interpComboBox);
    interpComboBox.addListener(this);
    interpComboBoxAttachment = new juce::AudioProcessorValueTreeState::ComboBoxAttachment(valueTreeState, "interp", interpComboBox);
//    const std::string interpTypes[3] = {"Cubic", "Linear", "Floor"};
//    juce::String str;
//    const char *text = "Cubic";
//    str = new juce::String(text);
//    interpComboBox.addItemList ({"Cubic", "Linear", "Floor"}, 0);
    
    addAndMakeVisible(&lopSlider);
    lopSlider.addListener(this);
    //lopSlider.setRange(120.f, 20000.f);
    lopSlider.setValue(12000.f/*processor.fbParam*/);
    lopSlider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    lopSlider.setSkewFactor (0.05, false);
    lopSliderAttachment = new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "lop", lopSlider);
    
    addAndMakeVisible(&allpassSlider);
    allpassSlider.addListener(this);
    //allpassSlider.setRange(-1.f, 1.f);
    allpassSlider.setValue(0.f/*processor.fbParam*/);
    allpassSlider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    allpassSliderAttachment = new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "apd_g", allpassSlider);

    addAndMakeVisible(&dist1inerSlider);
    dist1inerSlider.addListener(this);
    //dist1Slider[n].setRange(0.f, 1.f);
    dist1inerSlider.setValue(0.f/*processor.fbParam*/);
    dist1inerSlider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    dist1inerSlider.setSkewFactor (0.25, false);
    
    addAndMakeVisible(&dist1outrSlider);
    dist1outrSlider.addListener(this);
    //dist1Slider[n].setRange(0.f, 1.f);
    dist1outrSlider.setValue(0.f/*processor.fbParam*/);
    dist1outrSlider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    dist1outrSlider.setSkewFactor (0.25, false);

    dist1inerSliderAttachment = (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "dist1iner", dist1inerSlider));
    dist1outrSliderAttachment = (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "dist1outr", dist1outrSlider));

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (WIDTH, HEIGHT);
}

Shredverb_fromscratchAudioProcessorEditor::~Shredverb_fromscratchAudioProcessorEditor()
{
    delete decaySliderAttachment;
    delete sizeSliderAttachment;
    delete lopSliderAttachment;
    delete allpassSliderAttachment;
    delete dist1inerSliderAttachment;
    delete dist1outrSliderAttachment;
    
    delete interpComboBoxAttachment;
}

//==============================================================================
void Shredverb_fromscratchAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    //g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.fillAll (juce::Colours::indigo);

    g.setColour (juce::Colours::black);
    g.setFont (15.0f);
    //g.drawFittedText ("peh", getLocalBounds(), juce::Justification::centred, 1);
}

void Shredverb_fromscratchAudioProcessorEditor::resized()
{
    
    int row_Y = 50;
    int knobWidth = 100;
    int knobHeight = 100;
    int unit[NPARAMS];
    int offset;
    int textWidth, textHeight;
    textWidth = 60;
    textHeight = 30;
    for (int n = 0; n < NPARAMS; n++) {
        unit[n] = (WIDTH/NPARAMS) * n;
    }
    offset = unit[0] * 0.05;

    
    decaySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textWidth, textHeight);
    decaySlider.setBounds   (unit[0] + offset, row_Y, knobWidth, knobHeight);// (int x, int y, int width, int height)

    sizeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textWidth, textHeight);
    sizeSlider.setBounds    (unit[1] + offset, row_Y, knobWidth, knobHeight);
    
    interpComboBox.setBounds(unit[1] + offset + 105, row_Y + 20, 40, 20);
    
    lopSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textWidth, textHeight);
    lopSlider.setBounds     (unit[2] + offset, row_Y, knobWidth, knobHeight);
    allpassSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textWidth, textHeight);
    allpassSlider.setBounds (unit[3] + offset, row_Y, knobWidth, knobHeight);
    
    
    row_Y = 200;
    
    dist1inerSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textWidth, textHeight);
    dist1inerSlider.setBounds   (unit[0] + offset, row_Y + 0, knobWidth, knobHeight);
    dist1outrSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textWidth, textHeight);
    dist1outrSlider.setBounds   (unit[1] + offset, row_Y + 0, knobWidth, knobHeight);
}
