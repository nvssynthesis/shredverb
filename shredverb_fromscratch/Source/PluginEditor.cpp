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
    sizeSlider.setSkewFactor (0.25, false);
    sizeSliderAttachment = new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "size", sizeSlider);
    
    addAndMakeVisible(&lopSlider);
    lopSlider.addListener(this);
    //lopSlider.setRange(120.f, 20000.f);
    lopSlider.setValue(12000.f/*processor.fbParam*/);
    lopSlider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    lopSlider.setSkewFactor (0.25, false);
    lopSliderAttachment = new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "lop", lopSlider);
    
    addAndMakeVisible(&allpassSlider);
    allpassSlider.addListener(this);
    //allpassSlider.setRange(-1.f, 1.f);
    allpassSlider.setValue(0.f/*processor.fbParam*/);
    allpassSlider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    allpassSliderAttachment = new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "apd_g", allpassSlider);
    
    for (int n = 0; n < D_IJ; n++)
    {
        addAndMakeVisible(&dist1Slider[n]);
        dist1Slider[n].addListener(this);
        //dist1Slider[n].setRange(0.f, 1.f);
        dist1Slider[n].setValue(0.f/*processor.fbParam*/);
        dist1Slider[n].setSliderStyle(juce::Slider::SliderStyle::Rotary);
        dist1Slider[n].setSkewFactor (0.25, false);
        
        //std::string str = "dist";
        
        //dist1SliderAttachment[0] = (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "dist1", dist1Slider[0]));

        addAndMakeVisible(&dist2Slider[n]);
        dist2Slider[n].addListener(this);
        //dist2Slider[n].setRange(0.f, 1.f);
        dist2Slider[n].setValue(0.f/*processor.fbParam*/);
        dist2Slider[n].setSliderStyle(juce::Slider::SliderStyle::Rotary);
        dist2Slider[n].setSkewFactor (0.25, false);
//        dist2SliderAttachment[n] = new[] juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "dist2", dist2Slider[n]);
    }

    dist1SliderAttachment0 = (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "dist1a", dist1Slider[0]));
    dist1SliderAttachment1 = (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "dist1b", dist1Slider[1]));
    dist1SliderAttachment2 = (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "dist1c", dist1Slider[2]));
    dist1SliderAttachment3 = (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "dist1d", dist1Slider[3]));

    dist2SliderAttachment0 = new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "dist2a", dist2Slider[0]);
    dist2SliderAttachment1 = new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "dist2b", dist2Slider[1]);
    dist2SliderAttachment2 = new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "dist2c", dist2Slider[2]);
    dist2SliderAttachment3 = new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "dist2d", dist2Slider[3]);
    
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
    delete dist1SliderAttachment0;
    delete dist1SliderAttachment1;
    delete dist1SliderAttachment2;
    delete dist1SliderAttachment3;
    delete dist2SliderAttachment0;
    delete dist2SliderAttachment1;
    delete dist2SliderAttachment2;
    delete dist2SliderAttachment3;
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
    lopSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textWidth, textHeight);
    lopSlider.setBounds     (unit[2] + offset, row_Y, knobWidth, knobHeight);
    allpassSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textWidth, textHeight);
    allpassSlider.setBounds (unit[3] + offset, row_Y, knobWidth, knobHeight);
    row_Y = 200;
    
    for (int n = 0; n < D_IJ; n++)  {
        dist1Slider[n].setTextBoxStyle(juce::Slider::TextBoxBelow, false, textWidth, textHeight);
        dist1Slider[n].setBounds   (unit[n] + offset, row_Y + 0, knobWidth, knobHeight);
        dist2Slider[n].setTextBoxStyle(juce::Slider::TextBoxBelow, false, textWidth, textHeight);
        dist2Slider[n].setBounds   (unit[n] + offset, row_Y + 150, knobWidth, knobHeight);
    }
}
