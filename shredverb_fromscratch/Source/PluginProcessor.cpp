/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Shredverb_fromscratchAudioProcessor::Shredverb_fromscratchAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
        paramVT(*this, nullptr, juce::Identifier ("APVTSshredverb"),
            {
                std::make_unique<juce::AudioParameterFloat> ("feedbackGain",            // parameterID
                                                             "Feedback Gain",  // parameter name
                                                             0.f,              // minimum value
                                                             0.7071067f,  // maximum value
                                                             0.1f),             // default value
//                std::make_unique<juce::AudioParameterFloat> ("lfodepth",      // parameterID
//                                                             "Depth",     // parameter name
//                                                             0.0f,              // minimum value
//                                                             1.0f,              // maximum value
//                                                             0.5f),             // default value
                std::make_unique<juce::AudioParameterFloat> ("size",      // parameterID
                                                             "Spatial Size",     // parameter name
                                                             0.0f,              // minimum value
                                                             1.0f,              // maximum value
                                                             0.2f),             // default value
                std::make_unique<juce::AudioParameterFloat> ("lop",      // parameterID
                                                             "LP Freq",     // parameter name
                                                             200.0f,              // minimum value
                                                             20000.0f,              // maximum value
                                                             12000.f),             // default value
                std::make_unique<juce::AudioParameterFloat> ("dryWet",      // parameterID
                                                             "Dry/Wet",     // parameter name
                                                             0,              // minimum value
                                                             1,              // maximum value
                                                             0),             // default value
                std::make_unique<juce::AudioParameterFloat> ("tvap_0",      // parameterID
                                                             "TVAP 0",     // parameter name
                                                             20,              // minimum value
                                                             20000,              // maximum value
                                                             200),             // default value
                std::make_unique<juce::AudioParameterFloat> ("tvap_1",      // parameterID
                                                             "TVAP 1",     // parameter name
                                                             20,              // minimum value
                                                             20000,              // maximum value
                                                             2000),             // default value
                std::make_unique<juce::AudioParameterFloat> ("tvap_2",      // parameterID
                                                             "TVAP 2",     // parameter name
                                                             20,              // minimum value
                                                             20000,              // maximum value
                                                             500),             // default value
                std::make_unique<juce::AudioParameterFloat> ("tvap_3",      // parameterID
                                                             "TVAP 3",     // parameter name
                                                             20,              // minimum value
                                                             20000,              // maximum value
                                                             9000),             // default value
                std::make_unique<juce::AudioParameterFloat> ("dist1iner",      // parameterID
                                                             "Distortion 1 Inner",     // parameter name
                                                             0.f,              // minimum value
                                                             1.f,              // maximum value
                                                             0.f),             // default value
                std::make_unique<juce::AudioParameterFloat> ("dist1outr",      // parameterID
                                                             "Distortion 1 Outer",     // parameter name
                                                             0.f,              // minimum value
                                                             1.f,              // maximum value
                                                             0.f),             // default value
                std::make_unique<juce::AudioParameterFloat> ("dist2iner",      // parameterID
                                                             "Distortion 2 Inner",     // parameter name
                                                             0.f,              // minimum value
                                                             1.f,              // maximum value
                                                             0.f),             // default value
                std::make_unique<juce::AudioParameterFloat> ("dist2outr",      // parameterID
                                                             "Distortion 2 Outer",     // parameter name
                                                             0.f,              // minimum value
                                                             1.f,              // maximum value
                                                             0.f),             // default value
                
                std::make_unique<juce::AudioParameterFloat> ("interp",      // parameterID
                                                             "Interpolation Type",     // parameter name
                                                             0.f,              // minimum value
                                                             1.f,              // maximum value
                                                             0.f),             // default value
                std::make_unique<juce::AudioParameterFloat> ("randomizzze",      // parameterID
                                                             "Randomize",     // parameter name
                                                             0,              // minimum value
                                                             1,              // maximum value
                                                             0),             // default value
                
            })
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
     
     D = new nkvdu_delays::delay[D_IJ];
     apd = new nkvdu_delays::allpass_delay[D_IJ];
    
     int n;
     for (n = 0; n < D_IJ; n++) {
         X[n] = 0.f;
         Y[n] = 0.f;
         //G[n] = new float[D_IJ];
             // may want much shorter delay lines
         D_times[n] = 600.f * rando.nextFloat();
         D[n] = *new nkvdu_delays::delay(32768, 44100.f);
         D[n].setDelayTimeMS(D_times[n]);
         D[n].setInterpolation(nkvdu_delays::delay::interp::floor);
         apd[n].setDelayTimeMS(D_times[n]);
         apd[n].setInterpolation(nkvdu_delays::delay::interp::floor);
     }

    tvap = new nkvdu_filters::tvap[D_IJ + nIn];   //
    //tvap = new nkvdu_filters::tvap[nIn * 4];  // run in series
    
    //    predel = new nkvdu_delays::delay[nIn];
    //
    lp6dB = new nkvdu_filters::onePole[2 * (nIn + nOut)]; //input and every matrix out
}

Shredverb_fromscratchAudioProcessor::~Shredverb_fromscratchAudioProcessor()
{
    //delete[] apd;
    delete[] X;
    delete[] Y;
    delete[] D;
    delete[] tvap;
}

//==============================================================================
const juce::String Shredverb_fromscratchAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Shredverb_fromscratchAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Shredverb_fromscratchAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Shredverb_fromscratchAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Shredverb_fromscratchAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Shredverb_fromscratchAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Shredverb_fromscratchAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Shredverb_fromscratchAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String Shredverb_fromscratchAudioProcessor::getProgramName (int index)
{
    return {};
}

void Shredverb_fromscratchAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void Shredverb_fromscratchAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
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

void Shredverb_fromscratchAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Shredverb_fromscratchAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void Shredverb_fromscratchAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
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
    
    nkvdu_memoryless::metaparamA(float(*dist1Iner), inner_f_pi);
    nkvdu_memoryless::metaparamA(float(*dist1Outr), outer_f_pi);
    nkvdu_memoryless::metaparamA(float(*dist2Iner), inner_f_b);
    nkvdu_memoryless::metaparamA(float(*dist2Outr), outer_f_b);
//    nkvdu_memoryless::metaparamA(float(*dist2Iner), f_pi_m);
//    nkvdu_memoryless::metaparamA(float(*dist2Outr), metap[1]);

//    nkvdu_memoryless::metaparamA(metap[0][0], m[0]);
//    nkvdu_memoryless::metaparamA(metap[0][1], m[2]);
//    nkvdu_memoryless::metaparamA(metap[1][0], m[1]);
//    nkvdu_memoryless::metaparamA(metap[1][1], m[3]);

    
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

        *(outBuffL + samp) = finalOutL;
        *(outBuffR + samp) = finalOutR;
    }
}

//==============================================================================
bool Shredverb_fromscratchAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* Shredverb_fromscratchAudioProcessor::createEditor()
{
    return new Shredverb_fromscratchAudioProcessorEditor (*this, paramVT);
}

//==============================================================================
void Shredverb_fromscratchAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void Shredverb_fromscratchAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Shredverb_fromscratchAudioProcessor();
}
