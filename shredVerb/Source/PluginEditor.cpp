/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/
#if DEF_EDITOR

#include "PluginProcessor.h"
#include "PluginEditor.h"

#define WIDTH 500
#define HEIGHT 300
#define NPARAMS_PER_ROW 4

//==============================================================================
ShredVerbAudioProcessorEditor::ShredVerbAudioProcessorEditor (ShredVerbAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    //: AudioProcessorEditor (&p), audioProcessor (p)
    : AudioProcessorEditor (&p), processor (p), valueTreeState(vts), labelFont(HEIGHT / 25)
{
    param_stuff *tps = processor.getParamStuff();
    
    juce::Colour thumbColour = juce::Colours::darkviolet;
    juce::Colour fillColour = juce::Colours::dimgrey;
    
    labelFont.setTypefaceName("Luminari");

    for (int i = 0; i < param_stuff::numParams; ++i){
        addAndMakeVisible(&(tps->paramSliders[i]));
        tps->paramSliders[i].addListener(this);
        param_stuff::params_e ii = (param_stuff::params_e)(i);
        int numDec = tps->paramNumDecimalPlacesToDisplay.at(ii);
        tps->paramSliders[i].setNumDecimalPlacesToDisplay(numDec);
        tps->paramSliders[i].setRange(param_stuff::paramRanges.at(ii)[0], param_stuff::paramRanges.at(ii)[1]);
        tps->paramSliders[i].setValue(param_stuff::paramDefaults.at(ii));
        tps->paramSliders[i].setSkewFactorFromMidPoint(param_stuff::paramSkewFactorFromMidpoints.at(ii));
        tps->paramSliders[i].setColour(juce::Slider::ColourIds::thumbColourId, thumbColour);
        tps->paramSliders[i].setColour(juce::Slider::ColourIds::rotarySliderFillColourId, fillColour);
        tps->paramSliders[i].setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
        
        tps->paramSliders[i].setName(param_stuff::paramNames.at(ii));
        
        tps->paramSliderAttachmentPtrs[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(valueTreeState, param_stuff::paramIDs.at(ii), tps->paramSliders[i]);
        
        tps->paramLabels[i].setText(param_stuff::paramNames.at(ii), juce::dontSendNotification);
        tps->paramLabels[i].setFont(labelFont);
        addAndMakeVisible(tps->paramLabels[i]);
    }
   
    
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (WIDTH, HEIGHT);
}

ShredVerbAudioProcessorEditor::~ShredVerbAudioProcessorEditor()
{
    param_stuff *tps = processor.getParamStuff();
    for (int i = 0; i < param_stuff::numParams; ++i){
        tps->paramSliders[i].removeListener(this);
    }
}

//==============================================================================
void ShredVerbAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::hotpink);

    g.setColour (juce::Colours::black);
    g.setFont (15.0f);
}

void ShredVerbAudioProcessorEditor::resized()
{
    int row_Y = 10;
    int knobWidth = ((WIDTH / NPARAMS_PER_ROW) * 7) / 11;
    int knobHeight = knobWidth;
    int unit[NPARAMS_PER_ROW];
    int offset;
    int textWidth, textHeight;
    textWidth = (knobWidth * 2) / 3;
    textHeight = knobHeight / 5;
    for (int n = 0; n < NPARAMS_PER_ROW; n++) {
        unit[n] = (WIDTH/NPARAMS_PER_ROW) * n;
    }
    offset = int((double)unit[1] * 0.05);

    int label_Y = row_Y + 100;
    int label_X_ofst = offset + 30;

    param_stuff *tps = processor.getParamStuff();

    const int nRows = (param_stuff::numParams / NPARAMS_PER_ROW);
    
    for (int i = 0; i < param_stuff::numParams; ++i){
        bool isReadOnly = true;
        if ((i > 0) && ((i % NPARAMS_PER_ROW) == 0)){
            int heightwiseUnit = HEIGHT / nRows;
            row_Y += heightwiseUnit;
            label_Y = row_Y + (heightwiseUnit / 15);
        }

        tps->paramSliders[i].setTextBoxStyle(juce::Slider::TextBoxBelow, isReadOnly, textWidth, textHeight);
        tps->paramSliders[i].setBounds(unit[i % NPARAMS_PER_ROW] + offset, row_Y, knobWidth, knobHeight);
        
        tps->paramLabels[i].setBounds(unit[i % NPARAMS_PER_ROW] + label_X_ofst, label_Y, 180, 19);

        std::cout << "row_Y " << row_Y << std::endl;
        std::cout << "label_Y " << label_Y << std::endl;
    }
}
#endif
