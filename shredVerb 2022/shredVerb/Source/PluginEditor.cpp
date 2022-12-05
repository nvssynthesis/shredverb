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
ShredVerbAudioProcessorEditor::ShredVerbAudioProcessorEditor (ShredVerbAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
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
    
    decayLabel.setText("Decay", juce::dontSendNotification);
    addAndMakeVisible(&decayLabel);
    
    addAndMakeVisible(&sizeSlider);
    sizeSlider.addListener(this);
    sizeSlider.setNumDecimalPlacesToDisplay(6);
    //sizeSlider.setRange(0.f, 1.f);
    sizeSlider.setValue(0.f/*processor.fbParam*/);
    sizeSlider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    sizeSlider.setSkewFactor(0.002, false);
    sizeSliderAttachment = new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "size", sizeSlider);
    sizeLabel.setText("Size", juce::dontSendNotification);
    addAndMakeVisible(&sizeLabel);

    addAndMakeVisible(&dryWetSlider);
    dryWetSlider.addListener(this);
    dryWetSlider.setNumDecimalPlacesToDisplay(6);
    //sizeSlider.setRange(0.f, 1.f);
    dryWetSlider.setValue(0.f/*processor.fbParam*/);
    dryWetSlider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    dryWetSlider.setSkewFactor(0.002, false);
    dryWetSliderAttachment = new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "dryWet", dryWetSlider);
    dryWelLabel.setText("Dry/Wet", juce::dontSendNotification);
    addAndMakeVisible(&dryWelLabel);
    
//    addAndMakeVisible(&interpComboBox);
//    interpComboBox.addListener(this); juce::AudioProcessorValueTreeState::ComboBoxAttachment(valueTreeState, "interp", interpComboBox);
////    const std::string interpTypes[3] = {"Cubic", "Linear", "Floor"};
////    juce::String str;
////    const char *text = "Cubic";
////    str = new juce::String(text);
////    interpComboBox.addItemList ({"Cubic", "Linear", "Floor"}, 0);
//    interpTypeLabel.setText("Interpolation Type", juce::dontSendNotification);
//    addAndMakeVisible(&interpTypeLabel);
    
    addAndMakeVisible(&lopSlider);
    lopSlider.addListener(this);
    //lopSlider.setRange(120.f, 20000.f);
    lopSlider.setValue(12000.f/*processor.fbParam*/);
    lopSlider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    lopSlider.setSkewFactor (0.05, false);
    lopSliderAttachment = new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "lop", lopSlider);
    
    lopLabel.setText("Lowpass", juce::dontSendNotification);
    addAndMakeVisible(&lopLabel);
    
    double apSkew = 0.01;
    
    addAndMakeVisible(&allpassSlider0);
    allpassSlider0.addListener(this);
    allpassSlider0.setRange(20, 20000);
    allpassSlider0.setSkewFactor(apSkew);
    allpassSlider0.setValue(200);
    allpassSlider0.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    allpassSliderAttachment0 = new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "tvap_0", allpassSlider0);
    
    addAndMakeVisible(&allpassSlider1);
    allpassSlider1.addListener(this);
    allpassSlider1.setRange(20, 20000);
    allpassSlider1.setSkewFactor(apSkew);
    allpassSlider1.setValue(2000);
    allpassSlider1.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    allpassSliderAttachment1 = new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "tvap_1", allpassSlider1);
    
    addAndMakeVisible(&allpassSlider2);
    allpassSlider2.addListener(this);
    allpassSlider2.setRange(20, 20000);
    allpassSlider2.setSkewFactor(apSkew);
    allpassSlider2.setValue(5000);
    allpassSlider2.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    allpassSliderAttachment2 = new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "tvap_2", allpassSlider2);
    
    addAndMakeVisible(&allpassSlider3);
    allpassSlider3.addListener(this);
    allpassSlider3.setRange(20, 20000);
    allpassSlider3.setSkewFactor(apSkew);
    allpassSlider3.setValue(9000);
    allpassSlider3.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    allpassSliderAttachment3 = new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "tvap_3", allpassSlider3);
    
    allpassLabel.setText("Allpasses", juce::dontSendNotification);
    addAndMakeVisible(&allpassLabel);

    double distSkew = 0.05;

    addAndMakeVisible(&dist1inerSlider);
    dist1inerSlider.addListener(this);
    //dist1Slider[n].setRange(0.f, 1.f);
    dist1inerSlider.setValue(0.f/*processor.fbParam*/);
    dist1inerSlider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    dist1inerSlider.setSkewFactor (distSkew, false);
    dist1inerSliderAttachment = (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "dist1iner", dist1inerSlider));
    dist1inerLabel.setText("Inner Distortion 1", juce::dontSendNotification);
    addAndMakeVisible(&dist1inerLabel);

    addAndMakeVisible(&dist1outrSlider);
    dist1outrSlider.addListener(this);
    dist1outrSlider.setValue(0.f/*processor.fbParam*/);
    dist1outrSlider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    dist1outrSlider.setSkewFactor (distSkew, false);
    dist1outrSlider.setRange(0, 1);
    dist1outrSlider.setNumDecimalPlacesToDisplay(3);
    dist1outrSliderAttachment = (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "dist1outr", dist1outrSlider));
    dist1outrLabel.setText("Outer Distortion 1", juce::dontSendNotification);
    addAndMakeVisible(&dist1outrLabel);

    addAndMakeVisible(&dist2inerSlider);
    dist2inerSlider.addListener(this);
    //dist1Slider[n].setRange(0.f, 1.f);
    dist2inerSlider.setValue(0.f/*processor.fbParam*/);
    dist2inerSlider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    dist2inerSlider.setSkewFactor (distSkew);
    dist2inerSlider.setRange(0, 1);
    dist2inerSlider.setNumDecimalPlacesToDisplay(3);
    dist2inerSliderAttachment = (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "dist2iner", dist2inerSlider));
    dist2inerLabel.setText("Inner Distortion 2", juce::dontSendNotification);
    addAndMakeVisible(&dist2inerLabel);

    addAndMakeVisible(&dist2outrSlider);
    dist2outrSlider.addListener(this);
    dist2outrSlider.setValue(0.f/*processor.fbParam*/);
    dist2outrSlider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    dist2outrSlider.setSkewFactor (distSkew, false);
    dist2outrSlider.setRange(0, 1);
    dist2outrSlider.setNumDecimalPlacesToDisplay(3);
    dist2outrSliderAttachment = (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "dist2outr", dist1outrSlider));
    dist2outrLabel.setText("Outer Distortion 2", juce::dontSendNotification);
    addAndMakeVisible(&dist2outrLabel);
    
    addAndMakeVisible(&randomizeButton);
    randomizeButton.addListener(this);
    randomizeButtonAttachment = (new juce::AudioProcessorValueTreeState::ButtonAttachment (valueTreeState, "randomizzze", randomizeButton));
    
    

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (WIDTH, HEIGHT);
}

ShredVerbAudioProcessorEditor::~ShredVerbAudioProcessorEditor()
{
    delete decaySliderAttachment;
    delete sizeSliderAttachment;
    delete lopSliderAttachment;
    delete dryWetSliderAttachment;

    delete allpassSliderAttachment0;
    delete allpassSliderAttachment1;
    delete allpassSliderAttachment2;
    delete allpassSliderAttachment3;

    
    delete dist1inerSliderAttachment;
    delete dist1outrSliderAttachment;
    delete dist2inerSliderAttachment;
    delete dist2outrSliderAttachment;
    
    //delete interpComboBoxAttachment;
    delete randomizeButtonAttachment;
}

//==============================================================================
void ShredVerbAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    //g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.fillAll (juce::Colours::hotpink);

    g.setColour (juce::Colours::black);
    g.setFont (15.0f);
    //g.drawFittedText ("peh", getLocalBounds(), juce::Justification::centred, 1);
}

void ShredVerbAudioProcessorEditor::resized()
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
    offset = int((double)unit[1] * 0.25);

    int label_Y = row_Y + 100;
    int label_X_ofst = offset + 30;
//decayLabel, sizeLabel, interpTypeLabel, lopLabel, allpassLabel, dist1inerLabel, dist1outrLabel;
    decayLabel.setBounds    (unit[0] + label_X_ofst, label_Y, 40, 20);
    decaySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textWidth, textHeight);
    decaySlider.setBounds   (unit[0] + offset, row_Y, knobWidth, knobHeight);// (int x, int y, int width, int height)

    sizeLabel.setBounds     (unit[1] + label_X_ofst + 3, label_Y, 60, 20);
    sizeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textWidth, textHeight);
    sizeSlider.setBounds    (unit[1] + offset, row_Y, knobWidth, knobHeight);
    
    //interpComboBox.setBounds(unit[1] + offset + 115, row_Y + 20, 40, 20);
    
    lopLabel.setBounds      (unit[2] + label_X_ofst - 11, label_Y, 80, 20);
    lopSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textWidth, textHeight);
    lopSlider.setBounds     (unit[2] + offset, row_Y, knobWidth, knobHeight);
    
    dryWelLabel.setBounds      (unit[3] + label_X_ofst - 8, label_Y, 80, 20);
    dryWetSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textWidth, textHeight);
    dryWetSlider.setBounds     (unit[3] + offset, row_Y, knobWidth, knobHeight);
    
//    allpassLabel.setBounds  (unit[3] + label_X_ofst - 7, label_Y, 100, 20);
//    allpassSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textWidth, textHeight);
//    allpassSlider.setBounds (unit[3] + offset, row_Y, knobWidth, knobHeight);
//
    row_Y = 200;
    label_Y = row_Y + 100;
    
    allpassLabel.setBounds(unit[0] + label_X_ofst, label_Y, 100, 20);
    allpassSlider0.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textWidth, textHeight);
    allpassSlider0.setBounds(unit[0] + offset, row_Y + 0, knobWidth, knobHeight);
    
    allpassLabel.setBounds(unit[1] + label_X_ofst, label_Y, 100, 20);
    allpassSlider1.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textWidth, textHeight);
    allpassSlider1.setBounds(unit[1] + offset, row_Y + 0, knobWidth, knobHeight);
    
    allpassLabel.setBounds(unit[2] + label_X_ofst, label_Y, 100, 20);
    allpassSlider2.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textWidth, textHeight);
    allpassSlider2.setBounds(unit[2] + offset, row_Y + 0, knobWidth, knobHeight);
    
    allpassLabel.setBounds(unit[3] + label_X_ofst, label_Y, 100, 20);
    allpassSlider3.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textWidth, textHeight);
    allpassSlider3.setBounds(unit[3] + offset, row_Y + 0, knobWidth, knobHeight);
    
    row_Y = 350;
    label_Y = row_Y + 100;
    
    dist1inerLabel.setBounds    (unit[0] + label_X_ofst - 26, label_Y, 100, 20);
    dist1inerSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textWidth, textHeight);
    dist1inerSlider.setBounds   (unit[0] + offset, row_Y + 0, knobWidth, knobHeight);
    
    dist1outrLabel.setBounds    (unit[1] + label_X_ofst - 31, label_Y, 100, 20);
    dist1outrSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textWidth, textHeight);
    dist1outrSlider.setBounds   (unit[1] + offset, row_Y + 0, knobWidth, knobHeight);
    
    dist2inerLabel.setBounds    (unit[2] + label_X_ofst - 26, label_Y, 100, 20);
    dist2inerSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textWidth, textHeight);
    dist2inerSlider.setBounds   (unit[2] + offset, row_Y + 0, knobWidth, knobHeight);
    
    dist2outrLabel.setBounds    (unit[3] + label_X_ofst - 31, label_Y, 100, 20);
    dist2outrSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textWidth, textHeight);
    dist2outrSlider.setBounds   (unit[3] + offset, row_Y + 0, knobWidth, knobHeight);
    
}
