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

/*
 TODO: FIGURE OUT WHY THE FUCK THE SLIDER PARAMS (SKEW FACTOR, NUM DECIMALS TO DISPLAY) ARENT UPDATING
 
 
 */

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
   
    
//    addAndMakeVisible(&interpComboBox);
//    interpComboBox.addListener(this); juce::AudioProcessorValueTreeState::ComboBoxAttachment(valueTreeState, "interp", interpComboBox);
////    const std::string interpTypes[3] = {"Cubic", "Linear", "Floor"};
////    juce::String str;
////    const char *text = "Cubic";
////    str = new juce::String(text);
////    interpComboBox.addItemList ({"Cubic", "Linear", "Floor"}, 0);
//    interpTypeLabel.setText("Interpolation Type", juce::dontSendNotification);
//    addAndMakeVisible(&interpTypeLabel);
    
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
    //delete interpComboBoxAttachment;
//    delete randomizeButtonAttachment;
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
//        tps->paramLabels[i].setBounds(<#int x#>, <#int y#>, <#int width#>, <#int height#>)
//        tps->paramLabels[i].setCentrePosition(unit[i % NPARAMS_PER_ROW] + label_X_ofst, label_Y);
//        tps->paramLabels[i].set
        

        std::cout << "row_Y " << row_Y << std::endl;
        std::cout << "label_Y " << label_Y << std::endl;
    }
}
#endif

/*
 old way
 
 addAndMakeVisible(&decaySlider);
 decaySlider.addListener(this);
 decaySlider.setNumDecimalPlacesToDisplay(3);
 //decaySlider.setRange(0.f, 1.5f);
 decaySlider.setValue(0.f );
 decaySlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
 decaySlider.setColour(juce::Slider::ColourIds::thumbColourId, thumbColour);
 decaySlider.setColour(juce::Slider::ColourIds::rotarySliderFillColourId, fillColour);
 decaySliderAttachment = new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "feedbackGain", decaySlider);
 
 decayLabel.setText("Decay", juce::dontSendNotification);
 addAndMakeVisible(&decayLabel);
 

 addAndMakeVisible(&lopSlider);
 lopSlider.addListener(this);
 //lopSlider.setRange(120.f, 20000.f);
 lopSlider.setValue(12000.f);
 lopSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
//    lopSlider.setSkewFactor (0.05, false);
 lopSliderAttachment = new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "lop", lopSlider);
 
 lopLabel.setText("Lowpass", juce::dontSendNotification);
 addAndMakeVisible(&lopLabel);
 
 double apSkew = 0.01;
 
 addAndMakeVisible(&allpassSlider0);
 allpassSlider0.addListener(this);
 allpassSlider0.setRange(20, 20000);
//    allpassSlider0.setSkewFactor(apSkew);
 allpassSlider0.setValue(200);
 allpassSlider0.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
 allpassSliderAttachment0 = new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "tvap_0", allpassSlider0);
 
 addAndMakeVisible(&allpassSlider1);
 allpassSlider1.addListener(this);
 allpassSlider1.setRange(20, 20000);
//    allpassSlider1.setSkewFactor(apSkew);
 allpassSlider1.setValue(2000);
 allpassSlider1.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
 allpassSliderAttachment1 = new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "tvap_1", allpassSlider1);
 
 addAndMakeVisible(&allpassSlider2);
 allpassSlider2.addListener(this);
 allpassSlider2.setRange(20, 20000);
//    allpassSlider2.setSkewFactor(apSkew);
 allpassSlider2.setValue(5000);
 allpassSlider2.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
 allpassSliderAttachment2 = new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "tvap_2", allpassSlider2);
 
 addAndMakeVisible(&allpassSlider3);
 allpassSlider3.addListener(this);
 allpassSlider3.setRange(20, 20000);
//    allpassSlider3.setSkewFactor(apSkew);
 allpassSlider3.setSkewFactorFromMidPoint(200.f);
 allpassSlider3.setValue(9000);
 allpassSlider3.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
 allpassSliderAttachment3 = new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "tvap_3", allpassSlider3);
 
 allpassLabel.setText("Allpasses", juce::dontSendNotification);
 addAndMakeVisible(&allpassLabel);

 double distSkew = 0.05;

 dist1inerSlider.addListener(this);
 //dist1Slider[n].setRange(0.f, 1.f);
 dist1inerSlider.setValue(0.f);
 dist1inerSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
//    dist1inerSlider.setSkewFactor (distSkew, false);
 dist1inerSlider.setSkewFactorFromMidPoint(0.3f);
 dist1inerSlider.setRange(0.0, 1.0);
 dist1inerSlider.setNumDecimalPlacesToDisplay(3);
 dist1inerSliderAttachment = (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "dist1iner", dist1inerSlider));
 dist1inerLabel.setText("Inner Distortion 1", juce::dontSendNotification);
 addAndMakeVisible(&dist1inerSlider);
 addAndMakeVisible(&dist1inerLabel);

 addAndMakeVisible(&dist1outrSlider);
 dist1outrSlider.addListener(this);
 dist1outrSlider.setValue(0.f);
 dist1outrSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
//    dist1outrSlider.setSkewFactor (distSkew, false);
 dist1outrSlider.setSkewFactorFromMidPoint(0.5f);
 dist1outrSlider.setRange(0, 1);
 dist1outrSlider.setNumDecimalPlacesToDisplay(3);
 dist1outrSliderAttachment = (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "dist1outr", dist1outrSlider));
 dist1outrLabel.setText("Outer Distortion 1", juce::dontSendNotification);
 addAndMakeVisible(&dist1outrLabel);

 addAndMakeVisible(&dist2inerSlider);
 dist2inerSlider.addListener(this);
 //dist1Slider[n].setRange(0.f, 1.f);
 dist2inerSlider.setValue(0.f);
 dist2inerSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
//    dist2inerSlider.setSkewFactor (distSkew);
 dist2inerSlider.setSkewFactorFromMidPoint(0.4f);
 dist2inerSlider.setRange(0, 1);
 dist2inerSlider.setNumDecimalPlacesToDisplay(3);
 dist2inerSliderAttachment = (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "dist2iner", dist2inerSlider));
 dist2inerLabel.setText("Inner Distortion 2", juce::dontSendNotification);
 addAndMakeVisible(&dist2inerLabel);

 addAndMakeVisible(&dist2outrSlider);
 dist2outrSlider.addListener(this);
 dist2outrSlider.setValue(0.f);
 dist2outrSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
//    dist2outrSlider.setSkewFactor (distSkew, false);
 dist2outrSlider.setSkewFactorFromMidPoint(0.4f);
 dist2outrSlider.setRange(0, 1);
 dist2outrSlider.setNumDecimalPlacesToDisplay(3);
 dist2outrSliderAttachment = (new juce::AudioProcessorValueTreeState::SliderAttachment (valueTreeState, "dist2outr", dist1outrSlider));
 dist2outrLabel.setText("Outer Distortion 2", juce::dontSendNotification);
 addAndMakeVisible(&dist2outrLabel);
 
 addAndMakeVisible(&randomizeButton);
 randomizeButton.addListener(this);
 randomizeButtonAttachment = (new juce::AudioProcessorValueTreeState::ButtonAttachment (valueTreeState, "randomizzze", randomizeButton));
 
 
 
 //================================================

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
 
 
 //================================================
 
 //decayLabel, sizeLabel, interpTypeLabel, lopLabel, allpassLabel, dist1inerLabel, dist1outrLabel;
 decayLabel.setBounds    (unit[0] + label_X_ofst, label_Y, 40, 20);
 sizeLabel.setBounds     (unit[1] + label_X_ofst + 3, label_Y, 60, 20);
 lopLabel.setBounds      (unit[2] + label_X_ofst - 11, label_Y, 80, 20);
 dryWelLabel.setBounds      (unit[3] + label_X_ofst - 8, label_Y, 80, 20);

 
 decaySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textWidth, textHeight);
 decaySlider.setBounds   (unit[0] + offset, row_Y, knobWidth, knobHeight);// (int x, int y, int width, int height)

 sizeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textWidth, textHeight);
 sizeSlider.setBounds    (unit[1] + offset, row_Y, knobWidth, knobHeight);
 
 //interpComboBox.setBounds(unit[1] + offset + 115, row_Y + 20, 40, 20);
 
 lopSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textWidth, textHeight);
 lopSlider.setBounds     (unit[2] + offset, row_Y, knobWidth, knobHeight);
 
 dryWetSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textWidth, textHeight);
 dryWetSlider.setBounds     (unit[3] + offset, row_Y, knobWidth, knobHeight);
 
 //    allpassLabel.setBounds  (unit[3] + label_X_ofst - 7, label_Y, 100, 20);
 //    allpassSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textWidth, textHeight);
 //    allpassSlider.setBounds (unit[3] + offset, row_Y, knobWidth, knobHeight);
 //

 row_Y = 200;
 label_Y = row_Y + 100;
 
 allpassLabel.setBounds(unit[0] + label_X_ofst, label_Y, 100, 20);
 allpassLabel.setBounds(unit[1] + label_X_ofst, label_Y, 100, 20);
 allpassLabel.setBounds(unit[2] + label_X_ofst, label_Y, 100, 20);
 allpassLabel.setBounds(unit[3] + label_X_ofst, label_Y, 100, 20);

 allpassSlider0.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textWidth, textHeight);
 allpassSlider0.setBounds(unit[0] + offset, row_Y + 0, knobWidth, knobHeight);
 
 allpassSlider1.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textWidth, textHeight);
 allpassSlider1.setBounds(unit[1] + offset, row_Y + 0, knobWidth, knobHeight);
 
 allpassSlider2.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textWidth, textHeight);
 allpassSlider2.setBounds(unit[2] + offset, row_Y + 0, knobWidth, knobHeight);
 
 allpassSlider3.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textWidth, textHeight);
 allpassSlider3.setBounds(unit[3] + offset, row_Y + 0, knobWidth, knobHeight);
 
 row_Y = 350;
 label_Y = row_Y + 100;
 
 dist1inerLabel.setBounds    (unit[0] + label_X_ofst - 26, label_Y, 100, 20);
 dist1outrLabel.setBounds    (unit[1] + label_X_ofst - 31, label_Y, 100, 20);
 dist2inerLabel.setBounds    (unit[2] + label_X_ofst - 26, label_Y, 100, 20);
 dist2outrLabel.setBounds    (unit[3] + label_X_ofst - 31, label_Y, 100, 20);

 
 dist1inerSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textWidth, textHeight);
 dist1inerSlider.setBounds   (unit[0] + offset, row_Y + 0, knobWidth, knobHeight);
 
 dist1outrSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textWidth, textHeight);
 dist1outrSlider.setBounds   (unit[1] + offset, row_Y + 0, knobWidth, knobHeight);
 
 dist2inerSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textWidth, textHeight);
 dist2inerSlider.setBounds   (unit[2] + offset, row_Y + 0, knobWidth, knobHeight);
 
 dist2outrSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textWidth, textHeight);
 dist2outrSlider.setBounds   (unit[3] + offset, row_Y + 0, knobWidth, knobHeight);
 
 
 
 */
