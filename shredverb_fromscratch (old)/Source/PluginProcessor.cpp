/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ShredverbAudioProcessor::ShredverbAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
        paramVT(*this, nullptr, juce::Identifier ("APVTSshredverb"),createParams())
#endif
{
    // pointers are copied to items declared in object so they don't go out of scope
    fbParam   = paramVT.getRawParameterValue ("feedbackGain");
    sizeParam = paramVT.getRawParameterValue ("size");
    lopParam = paramVT.getRawParameterValue ("lop");
    dryWetParam = paramVT.getRawParameterValue ("dryWet");

    allpassParam0 = paramVT.getRawParameterValue ("tvap_0");
    allpassParam1 = paramVT.getRawParameterValue ("tvap_1");
    allpassParam2 = paramVT.getRawParameterValue ("tvap_2");
    allpassParam3 = paramVT.getRawParameterValue ("tvap_3");
    
    dist1inerParam = paramVT.getRawParameterValue ("dist1iner");
    dist1outrParam = paramVT.getRawParameterValue ("dist1outr");
    dist2inerParam = paramVT.getRawParameterValue ("dist2iner");
    dist2outrParam = paramVT.getRawParameterValue ("dist2outr");
    
    interpParam = paramVT.getRawParameterValue ("interp");
    
    randomizeParam = paramVT.getRawParameterValue ("randomizzze");
    //ALLPASS_PARAM
    
    // need array of delay pointers with parameterized constructors, dynamically allocated
    auto nOut = getTotalNumOutputChannels();
    auto nIn = getTotalNumInputChannels();
    
    
     X = new float[D_IJ];
     Y = new float[D_IJ];
     
     D = new nvs_delays::delay[D_IJ];
     apd = new nvs_delays::allpass_delay[D_IJ];
    
     int n;
     for (n = 0; n < D_IJ; n++) {
         X[n] = 0.f;
         Y[n] = 0.f;
         //G[n] = new float[D_IJ];
             // may want much shorter delay lines
         D_times[n] = 600.f * rando.nextFloat();
         D[n] = *new nvs_delays::delay(32768, 44100.f);
         D[n].setDelayTimeMS(D_times[n]);
         D[n].setInterpolation(nvs_delays::delay::interp::floor);
         apd[n].setDelayTimeMS(D_times[n]);
         apd[n].setInterpolation(nvs_delays::delay::interp::floor);
     }

    tvap = new nvs_filters::tvap<float>[D_IJ + nIn];   //
    //tvap = new nvs_filters::tvap[nIn * 4];  // run in series
    
    //    predel = new nvs_delays::delay[nIn];
    //
    lp6dB = new nvs_filters::onePole<float>[2 * (nIn + nOut)]; //input and every matrix out
}

ShredverbAudioProcessor::~ShredverbAudioProcessor()
{
    //delete[] apd;
    delete[] X;
    delete[] Y;
    delete[] D;
    delete[] tvap;
}

//==============================================================================
const juce::String ShredverbAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ShredverbAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ShredverbAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ShredverbAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ShredverbAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ShredverbAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ShredverbAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ShredverbAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ShredverbAudioProcessor::getProgramName (int index)
{
    return {};
}

void ShredverbAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void ShredverbAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    
     // Use this method as the place to do any pre-playback
     // initialisation that you need..
    auto nIn = getTotalNumInputChannels();
    auto nOut = getTotalNumOutputChannels();

    for (int n = 0; n < D_IJ; n++)
    {
        tvap[n].clear();
        tvap[n].setSampleRate(sampleRate);
        tvap[n].updateCutoff(1000.f * float(n + 1), 1.f);
        tvap[n].updateResonance(100.f * float(n + 1), 1.f);
    }
    //feed forward pair
    tvap[4].clear();
    tvap[4].setSampleRate(sampleRate);
    tvap[4].updateCutoff(4000.f, 1.f);
    tvap[4].updateResonance(2000.f, 1.f);
    tvap[5].clear();
    tvap[5].setSampleRate(sampleRate);
    tvap[5].updateCutoff(4000.f, 1.f);
    tvap[5].updateResonance(2000.f, 1.f);
    
    for (int n = 0; n < (nIn + nOut); n++)
    {
    //        apd[n].setSampleRate((float)sampleRate);
    //        apd[n].update_g(0.5f, 1);
    //        apd[n].setDelayTimeMS(11.1f + float(n)*2.f);
            lp6dB[n].setSampleRate((float)sampleRate);
            lp6dB[n].updateCutoff(12000.f, 1.f);
    }
    for (int i = 0; i < D_IJ; i++)  {
        D[i].setSampleRate((float)sampleRate);
        apd[i].setSampleRate((float)sampleRate);
    }
//     for (int n = 0; n < nOut; n++)
//     {
//     //        apd[n].clear();
//     //        apd[n].setSampleRate((float)sampleRate);
//     //        apd[n].setDelayTimeMS(100.f);
//     }
     
}

void ShredverbAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ShredverbAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void ShredverbAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto nIn  = getTotalNumInputChannels();
    auto nOut = getTotalNumOutputChannels();
    int numSamps = buffer.getNumSamples();
    double _oneOverBlockSize = 1 / (double)numSamps;
    for (auto i = nIn; i < nOut; ++i)
        buffer.clear (i, 0, numSamps);
    
    auto* inBuffL = buffer.getReadPointer(0);
    auto* inBuffR = inBuffL;
    if (getTotalNumInputChannels() > 1)
        inBuffR = buffer.getReadPointer(1);
    
    auto* outBuffL = buffer.getWritePointer(0);
    auto* outBuffR = outBuffL;
    if (getTotalNumOutputChannels() > 1)
        outBuffR = buffer.getWritePointer(1);
    
    //float _g = *fbParam;
    auto _g = paramVT.getRawParameterValue("feedbackGain");
    float _size = *sizeParam;
    _size *= _size;
    float _lop = *lopParam;
    float _dryWet = *dryWetParam;
    float _ap[4];
    _ap[0] = *allpassParam0;
    _ap[1] = *allpassParam1;
    _ap[2] = *allpassParam2;
    _ap[3] = *allpassParam3;

    float inner_f_pi[2], outer_f_pi[2];
    float inner_f_b[2], outer_f_b[2];
    

    //float m[4][2];   // # of tvaps × # of outputs per lowest metaparam layer cell

    // ACTUALLY RESHAPE FROM 2 METAPARAMS INTO 4 PARAMS:
    // dist1iner goes to
    auto dist1Iner = paramVT.getRawParameterValue("dist1iner");
    auto dist1Outr = paramVT.getRawParameterValue("dist1outr");
    auto dist2Iner = paramVT.getRawParameterValue("dist2iner");
    auto dist2Outr = paramVT.getRawParameterValue("dist2outr");
    
    nvs_memoryless::metaparamA(float(*dist1Iner), inner_f_pi);
    nvs_memoryless::metaparamA(float(*dist1Outr), outer_f_pi);
    nvs_memoryless::metaparamA(float(*dist2Iner), inner_f_b);
    nvs_memoryless::metaparamA(float(*dist2Outr), outer_f_b);
//    nvs_memoryless::metaparamA(float(*dist2Iner), f_pi_m);
//    nvs_memoryless::metaparamA(float(*dist2Outr), metap[1]);

//    nvs_memoryless::metaparamA(metap[0][0], m[0]);
//    nvs_memoryless::metaparamA(metap[0][1], m[2]);
//    nvs_memoryless::metaparamA(metap[1][0], m[1]);
//    nvs_memoryless::metaparamA(metap[1][1], m[3]);

    
//_ap_dist1[1] = *dist1outrParam;
//    float _ap_dist2[4];
//    _ap_dist2[0] = *dist2inerParam * -1;
//    _ap_dist2[1] = *dist2outrParam * -1;
    //=============================================================================
    float current_Dtime[D_IJ];
    for (int i = 0; i < D_IJ; i++) {
        current_Dtime[i] = D_times[i];
        current_Dtime[i] *= _size;
        current_Dtime[i] += 0.1f;
        // add LFO mod here
        
//        D[i].updateDelayTimeMS(current_Dtime, _oneOverBlockSize);
//        apd[i].updateDelayTimeMS(current_Dtime, _oneOverBlockSize);
        //apd[i].update_g(_ap_g, (float)_oneOverBlockSize);
    }
    for (int n = 0; n < (nIn + nOut); n++)  {
        lp6dB[n].updateCutoff(_lop, _oneOverBlockSize);
    }
    
    for (int samp = 0; samp < numSamps; samp++)
    {
        for (int i = 0; i < D_IJ; i++) {
            D[i].updateDelayTimeMS(current_Dtime[i], (float)_oneOverBlockSize);
            apd[i].updateDelayTimeMS(current_Dtime[i], (float)_oneOverBlockSize);
            
            tvap[i].update_f_pi(_ap[i], (float)_oneOverBlockSize);
        }
        
        
        float leftSamp = *(inBuffL + samp);
        float rightSamp = *(inBuffR + samp);
        
        X[0] = 0.f;
        X[1] = leftSamp;//tvap[4].filter_fbmod(leftSamp, 0.f, 0.f);
        X[2] = rightSamp;//tvap[5].filter_fbmod(rightSamp, 0.f, 0.f);
        X[3] = 0.f;
        

        
        float tmp[4] = {0.f, 0.f, 0.f, 0.f};
        
//        Y[0] = tvap[0].filter_fbmod(Y[0], _ap_dist1, _ap_dist2);
//        Y[1] = tvap[1].filter_fbmod(Y[1], _ap_dist1, _ap_dist2);
//        Y[2] = tvap[2].filter_fbmod(Y[2], _ap_dist1, _ap_dist2);
//        Y[3] = tvap[3].filter_fbmod(Y[3], _ap_dist1, _ap_dist2);
        
//        Y[0] = lp6dB->tpt_lp(Y[0], _lop);
//        Y[1] = lp6dB->tpt_lp(Y[1], _lop);
//        Y[2] = lp6dB->tpt_lp(Y[2], _lop);
//        Y[3] = lp6dB->tpt_lp(Y[3], _lop);
        for (int i = 0; i < D_IJ; i++)
        {
            int j;
            for (j = 0; j < D_IJ; j++)
            {
                tmp[i] += G[i][j] * Y[j] * (*_g);
            }
            tmp[i] += X[i];
            tmp[0] = lp6dB[0].tpt_lp(tmp[0], _lop);
            tmp[1] = lp6dB[1].tpt_lp(tmp[1], _lop);
            tmp[2] = lp6dB[2].tpt_lp(tmp[2], _lop);
            tmp[3] = lp6dB[3].tpt_lp(tmp[3], _lop);
            tmp[0] = lp6dB[4].tpt_lp(tmp[0], _lop);
            tmp[1] = lp6dB[5].tpt_lp(tmp[1], _lop);
            tmp[2] = lp6dB[6].tpt_lp(tmp[2], _lop);
            tmp[3] = lp6dB[7].tpt_lp(tmp[3], _lop);
        }
/* L?*/tmp[0] = tvap[0].filter_fbmod(tmp[0], inner_f_pi[0], inner_f_b[0]);
/*LEFT for sure*/tmp[1] = tvap[1].filter_fbmod(tmp[1], outer_f_pi[0], outer_f_b[0]);
/*RGHT for sure*/tmp[2] = tvap[2].filter_fbmod(tmp[2], outer_f_pi[1], outer_f_b[1]);
/* R?*/tmp[3] = tvap[3].filter_fbmod(tmp[3], inner_f_pi[1], inner_f_b[1]);
        
        // can just have 1 tmp line and 1 Y line in for loop
        
//        Y[0] = apd[0][0].filter(tmp[0]) + apd[0][1].filter(tmp[1])
//        + apd[0][2].filter(tmp[2]) + apd[0][3].filter(tmp[3]);
//        Y[1] = apd[1][0].filter(tmp[0]) + apd[1][1].filter(tmp[1])
//        + apd[1][2].filter(tmp[2]) + apd[1][3].filter(tmp[3]);
//        Y[2] = apd[2][0].filter(tmp[0]) + apd[2][1].filter(tmp[1])
//        + apd[2][2].filter(tmp[2]) + apd[2][3].filter(tmp[3]);
//        Y[3] = apd[3][0].filter(tmp[0]) + apd[3][1].filter(tmp[1])
//        + apd[3][2].filter(tmp[2]) + apd[3][3].filter(tmp[3]);
        
        Y[0] = D[0].tick_cubic(tmp[0]);
        Y[1] = D[1].tick_cubic(tmp[1]);
        Y[2] = D[2].tick_cubic(tmp[2]);
        Y[3] = D[3].tick_cubic(tmp[3]);
        
        // D should just be 4 delay lines!
//        for (int i = 0; i < D_IJ; i++)  {
//            for (int j = 1; j < D_IJ; j++)  {
//                Y[i] += D[i][j].tick_linear(tmp[j]);
//            }
//        }
// ideally this should work if function pointer works
//        Y[0] = D[0][0].tick(tmp[0]) + D[0][1].tick(tmp[1])
//        + D[0][2].tick(tmp[2]) + D[0][3].tick(tmp[3]);
//        Y[1] = D[1][0].tick(tmp[0]) + D[1][1].tick(tmp[1])
//        + D[1][2].tick(tmp[2]) + D[1][3].tick(tmp[3]);
//        Y[2] = D[2][0].tick(tmp[0]) + D[2][1].tick(tmp[1])
//        + D[2][2].tick(tmp[2]) + D[2][3].tick(tmp[3]);
//        Y[3] = D[3][0].tick(tmp[0]) + D[3][1].tick(tmp[1])
//        + D[3][2].tick(tmp[2]) + D[3][3].tick(tmp[3]);

        float finalOutL = Y[1] * (1 - _dryWet) + leftSamp * _dryWet;
        float finalOutR = Y[2] * (1 - _dryWet) + rightSamp * _dryWet;

        *(outBuffL + samp) = finalOutL * 0.125f;
        *(outBuffR + samp) = finalOutR;
    }
}

//==============================================================================
bool ShredverbAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ShredverbAudioProcessor::createEditor()
{
    return new ShredverbAudioProcessorEditor (*this, paramVT);
}

//==============================================================================
void ShredverbAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void ShredverbAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

juce::AudioProcessorValueTreeState::ParameterLayout ShredverbAudioProcessor::createParams()
{
    const unsigned int versionHint = 1;

    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    juce::String parameterID_str = "feedbackGain";
    juce::String parameterName = "Feedback Gain";
    juce::NormalisableRange<float> normRange(0.f,   // min
                                    0.7071067f,     // max
                                             0.01f);  // interval value (?)
    juce::NormalisableRange<float> nr(0,0.7071f, 0.f);
    float defaultVal = 0.1f;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID {parameterID_str, versionHint}, parameterName, nr, defaultVal));
    
    parameterID_str = "size";      // parameterID
    parameterName = "Spatial Size";     // parameter name
    normRange.start = 0.f;
    normRange.end = 1.f;
    defaultVal = 0.2f;
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID {parameterID_str, versionHint}, parameterName, normRange, defaultVal));

    parameterID_str = "lop";      // parameterID
    parameterName = "LP Freq";     // parameter name
    normRange.start = 200.0f;
    normRange.end = 20000.0f;
    defaultVal = 12000.f;
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID {parameterID_str, versionHint}, parameterName, normRange, defaultVal));

    parameterID_str = "dryWet";      // parameterID
    parameterName = "Dry/Wet";     // parameter name
    normRange.start = 0.f;
    normRange.end = 1.f;
    defaultVal = 0.f;
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID {parameterID_str, versionHint}, parameterName, normRange, defaultVal));

    parameterID_str = "tvap_0";      // parameterID
    parameterName = "TVAP 0";     // parameter name
    normRange.start = 20.f;
    normRange.end = 20000.f;
    defaultVal = 200.f;
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID {parameterID_str, versionHint}, parameterName, normRange, defaultVal));

    parameterID_str = "tvap_1";      // parameterID
    parameterName = "TVAP 1";     // parameter name
    normRange.start = 20.f;
    normRange.end = 20000.f;
    defaultVal = 2000.f;
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID {parameterID_str, versionHint}, parameterName, normRange, defaultVal));

    parameterID_str = "tvap_2";      // parameterID
    parameterName = "TVAP 2";     // parameter name
    normRange.start = 20.f;
    normRange.end = 20000.f;
    defaultVal = 500.f;
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID {parameterID_str, versionHint}, parameterName, normRange, defaultVal));

    parameterID_str = "tvap_3";      // parameterID
    parameterName = "TVAP 3";     // parameter name
    normRange.start = 20.f;
    normRange.end = 20000.f;
    defaultVal = 9000.f;
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID {parameterID_str, versionHint}, parameterName, normRange, defaultVal));

    parameterID_str = "dist1iner";      // parameterID
    parameterName = "Distortion 1 Inner";     // parameter name
    normRange.start = 0.f;
    normRange.end = 1.f;
    defaultVal = 0.f;
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID {parameterID_str, versionHint}, parameterName, normRange, defaultVal));

    parameterID_str = "dist1outr";      // parameterID
    parameterName = "Distortion 1 Outer";     // parameter name
    normRange.start = 0.f;
    normRange.end = 1.f;
    defaultVal = 0.f;
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID {parameterID_str, versionHint}, parameterName, normRange, defaultVal));

    parameterID_str = "dist2iner";      // parameterID
    parameterName = "Distortion 2 Inner";     // parameter name
    normRange.start = 0.f;
    normRange.end = 1.f;
    defaultVal = 0.f;
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID {parameterID_str, versionHint}, parameterName, normRange, defaultVal));

    parameterID_str = "dist2outr";      // parameterID
    parameterName = "Distortion 2 Outer";     // parameter name
    normRange.start = 0.f;
    normRange.end = 1.f;
    defaultVal = 0.f;
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID {parameterID_str, versionHint}, parameterName, normRange, defaultVal));

    parameterID_str = "interp";      // parameterID
    parameterName = "Interpolation Type";     // parameter name
    normRange.start = 0.f;
    normRange.end = 1.f;
    defaultVal = 0.f;
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID {parameterID_str, versionHint}, parameterName, normRange, defaultVal));

    parameterID_str = "randomizzze";      // parameterID
    parameterName = "Randomize";     // parameter name
    normRange.start = 0.f;
    normRange.end = 1.f;
    defaultVal = 0.f;
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID {parameterID_str, versionHint}, parameterName, normRange, defaultVal));

    return { params.begin(), params.end() };
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ShredverbAudioProcessor();
}
