/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#if DEF_EDITOR
#include "PluginEditor.h"
#endif

#define PROTECT_OUTPUT 1
//==============================================================================
ShredVerbAudioProcessor::ShredVerbAudioProcessor()
//#ifndef JucePlugin_PreferredChannelConfigurations
     :  foleys::MagicProcessor (juce::AudioProcessor::BusesProperties()
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                       ),
        paramVT(*this, nullptr, juce::Identifier ("APVTSshredverb"), createParameterLayout())
//,       magicState(*this)
//#endif
{
    FOLEYS_SET_SOURCE_PATH (__FILE__);
    /*auto file = juce::File::getSpecialLocation (juce::File::currentApplicationFile)
        .getChildFile ("Contents")
        .getChildFile ("Resources")
        .getChildFile ("magic.xml");
     */
//    juce::File file("/Users/nicholassolem/Documents/GitHub/shredverb/shredVerb 2022/shredVerb/Builds/MacOSX/build/Debug/0th_iter.1");

//    if (file.existsAsFile())
//        magicState.setGuiValueTree (file);
//    else{
////        paramVT = magicState.createGuiValueTree();
////        magicState.setGuiValueTree (BinaryData::magic_xml, BinaryData::magic_xmlSize);
//    }
    
    // this is how i was loading default, but docs actually say to do this as return... in createEditor
    magicState.setGuiValueTree (BinaryData::DEFAULT_v4_1_xml, BinaryData::DEFAULT_v4_1_xmlSize);

    presetList = magicState.createAndAddObject<PresetListBox>("presets");
    /*presetList->onSelectionChanged = [&](int number)
    {
        loadPresetInternal (number);
    };
    magicState.addTrigger ("save-preset", [this]
    {
        savePresetInternal();
    });
    */
    auto nOut = getTotalNumOutputChannels();
    auto nIn = getTotalNumInputChannels();
    std::cout << "num in: " << nIn << std::endl;
//    magicState.setApplicationSettingsFile (juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
//                                           .getChildFile (ProjectInfo::companyName)
//                                           .getChildFile (ProjectInfo::projectName + juce::String (".settings")));
//
//    magicState.setPlayheadUpdateFrequency (30);


    // pointers are copied to items declared in object so they don't go out of scope
    driveParam    = paramVT.getRawParameterValue (param_stuff::paramIDs.at(param_stuff::params_e::drive));
    predelayParam = paramVT.getRawParameterValue (param_stuff::paramIDs.at(param_stuff::params_e::predelay));

    fbParam   = paramVT.getRawParameterValue (param_stuff::paramIDs.at(param_stuff::params_e::decay));
    sizeParam = paramVT.getRawParameterValue (param_stuff::paramIDs.at(param_stuff::params_e::size));
    lopParam = paramVT.getRawParameterValue (param_stuff::paramIDs.at(param_stuff::params_e::lowpass));
    hipParam = paramVT.getRawParameterValue (param_stuff::paramIDs.at(param_stuff::params_e::highpass));
    juce::String js = param_stuff::paramIDs.at(param_stuff::params_e::drywet);
    dryWetParam = paramVT.getRawParameterValue (js);

    allpassFrequencyPiParam0 = paramVT.getRawParameterValue (param_stuff::paramIDs.at(param_stuff::params_e::tvap0_f_pi));
    allpassFrequencyPiParam1 = paramVT.getRawParameterValue (param_stuff::paramIDs.at(param_stuff::params_e::tvap1_f_pi));
    allpassFrequencyPiParam2 = paramVT.getRawParameterValue (param_stuff::paramIDs.at(param_stuff::params_e::tvap2_f_pi));
    allpassFrequencyPiParam3 = paramVT.getRawParameterValue (param_stuff::paramIDs.at(param_stuff::params_e::tvap3_f_pi));
    
    allpassBandwidthParam0 = paramVT.getRawParameterValue (param_stuff::paramIDs.at(param_stuff::params_e::tvap0_f_b));
    allpassBandwidthParam1 = paramVT.getRawParameterValue (param_stuff::paramIDs.at(param_stuff::params_e::tvap1_f_b));
    allpassBandwidthParam2 = paramVT.getRawParameterValue (param_stuff::paramIDs.at(param_stuff::params_e::tvap2_f_b));
    allpassBandwidthParam3 = paramVT.getRawParameterValue (param_stuff::paramIDs.at(param_stuff::params_e::tvap3_f_b));
    
    dist1inerParam = paramVT.getRawParameterValue (param_stuff::paramIDs.at(param_stuff::params_e::dist1_inner));
    dist1outrParam = paramVT.getRawParameterValue (param_stuff::paramIDs.at(param_stuff::params_e::dist1_outer));
    dist2inerParam = paramVT.getRawParameterValue (param_stuff::paramIDs.at(param_stuff::params_e::dist2_inner));
    dist2outrParam = paramVT.getRawParameterValue (param_stuff::paramIDs.at(param_stuff::params_e::dist2_outer));
    
    time0Param = paramVT.getRawParameterValue (param_stuff::paramIDs.at(param_stuff::params_e::time0));
    time1Param = paramVT.getRawParameterValue (param_stuff::paramIDs.at(param_stuff::params_e::time1));
    time2Param = paramVT.getRawParameterValue (param_stuff::paramIDs.at(param_stuff::params_e::time2));
    time3Param = paramVT.getRawParameterValue (param_stuff::paramIDs.at(param_stuff::params_e::time3));

    outputGainParam = paramVT.getRawParameterValue (param_stuff::paramIDs.at(param_stuff::params_e::output_gain));

    interpParam = paramVT.getRawParameterValue (param_stuff::paramIDs.at(param_stuff::params_e::interp_type));
    
    randomizeParam = paramVT.getRawParameterValue (param_stuff::paramIDs.at(param_stuff::params_e::randomize));
    //ALLPASS_PARAM
    
    // need array of delay pointers with parameterized constructors, dynamically allocated

    X = new float[D_IJ];
    Y = new float[D_IJ];
     
    preD = new nvs_delays::delay[nIn];
    for (int n = 0; n < nIn; n++){
        preD[n] = *new nvs_delays::delay(32768, 44100.f);
    }
    
    D = new nvs_delays::delay[D_IJ];
    apd = new nvs_delays::allpass_delay[D_IJ];

    int n;
    
    D_times_ranged[0] = param_stuff::paramDefaults.at(param_stuff::params_e::time0);
    D_times_ranged[1] = param_stuff::paramDefaults.at(param_stuff::params_e::time1);
    D_times_ranged[2] = param_stuff::paramDefaults.at(param_stuff::params_e::time2);
    D_times_ranged[3] = param_stuff::paramDefaults.at(param_stuff::params_e::time3);

    for (n = 0; n < D_IJ; n++) {
         X[n] = 0.f;
         Y[n] = 0.f;
         //G[n] = new float[D_IJ];
             // may want much shorter delay lines
//        float tmp = rando.nextFloat();
//        int tmp = (accum1 + accum2);
//        accum1 = accum2;
//        accum2 = tmp;
//        std::cout << accum2 <<std::endl;
        D[n] = *new nvs_delays::delay(65536, 44100.f);
        D[n].setDelayTimeMS(D_times_ranged[n] * timeScaling);
        D[n].setInterpolation(nvs_delays::delay::interp::floor);
        apd[n].setDelayTimeMS(D_times_ranged[n] * timeScaling);
        apd[n].setInterpolation(nvs_delays::delay::interp::floor);
     }

	for (auto &hp : hp6dB){
		hp.setMode(nvs::filters::mode_e::HP);
	}
}
ShredVerbAudioProcessor::~ShredVerbAudioProcessor()
{
    delete[] X;
    delete[] Y;
    delete[] D;
    delete[] preD;
}
#if DEF_EDITOR
//==============================================================================
const juce::String ShredVerbAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ShredVerbAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ShredVerbAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ShredVerbAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}
#endif
double ShredVerbAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}
#if DEF_EDITOR
int ShredVerbAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ShredVerbAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ShredVerbAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ShredVerbAudioProcessor::getProgramName (int index)
{
    return {};
}

void ShredVerbAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}
#endif
//==============================================================================
void ShredVerbAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    auto nIn = getTotalNumInputChannels();
    auto nOut = getTotalNumOutputChannels();
    
    maxDelTimeMS = float((D[0].getDelaySize() - 10) * 1000) / (float)sampleRate; // no more than buffer length - 10
    minDelTimeMS = float(3 * 1000) / (float)sampleRate; // no less than 3 samples

    std::cout << "min del time: " << minDelTimeMS << "\nmax del time: " << maxDelTimeMS << std::endl;
    
    for (int n = 0; n < D_IJ; n++)
    {
        tvap[n].clear();
        tvap[n].setSampleRate(sampleRate);
		tvap[n].setBlockSize(samplesPerBlock);
        tvap[n].setCutoff(1000.f * float(n + 1));
        tvap[n].setResonance(100.f * float(n + 1));
    }
    //feed forward pair
    if (nIn > 0){
        preD[0].setSampleRate(sampleRate);
        if (nIn == 2){
            preD[1].setSampleRate(sampleRate);
        }
    }
    else{
        std::cerr << "NUM INPUT CHANS == 0" << std::endl;
    }

    for (int n = 0; n < (nIn + nOut); n++)
    {
		hp6dB[n].setSampleRate((float)sampleRate);
		hp6dB[n].setBlockSize(samplesPerBlock);
		hp6dB[n].setCutoff(80.f);
    }
    for (int n = 0; n < D_IJ; n++){
        butters[n].setSampleRate(sampleRate);
		butters[n].setBlockSize(samplesPerBlock);
        butters[n].setCutoff(18000.0);
    }
    for (int i = 0; i < D_IJ; i++)  {
        D[i].setSampleRate((float)sampleRate);
        apd[i].setSampleRate((float)sampleRate);
    }
}

void ShredVerbAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ShredVerbAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void ShredVerbAudioProcessor::parameterChanged (const juce::String& param, float value){}

void ShredVerbAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto const nIn  = getTotalNumInputChannels();
    auto const nOut = getTotalNumOutputChannels();
    int const numSamps = buffer.getNumSamples();
	
	[[deprecated]]	// any function using this param during processBlock should be deprecated
    double const _oneOverBlockSize = 1 / (double)numSamps;
	for (auto i = nIn; i < nOut; ++i){
		buffer.clear (i, 0, numSamps);
	}
	
    auto const* const inBuffL = buffer.getReadPointer(0);
    auto const* inBuffR = inBuffL;
	if (getTotalNumInputChannels() > 1){
		inBuffR = buffer.getReadPointer(1);
	}
	
    auto* const outBuffL = buffer.getWritePointer(0);
    auto* outBuffR = outBuffL;
    if (getTotalNumOutputChannels() > 1)
        outBuffR = buffer.getWritePointer(1);
    
    float const _drive = *driveParam;
    float const _predel = *predelayParam;
    
    float const _g = *fbParam;
    float _size = *sizeParam;
    _size *= _size;		// breaking const is a sign that the param itself should be shaped
    float const _hip = *hipParam;
    float const _lop = *lopParam;
	
	float  time0 = *time0Param;
	float  time1 = *time1Param;
	float  time2 = *time2Param;
	float  time3 = *time3Param;
    std::array<float const *, 4> const times {
		&time0,
		&time1,
		&time2,
		&time3
	};
	
    float const _dryWet = dryWetParam->load();
    float const _outputGain = outputGainParam->load();

	std::array<float, 4> _ap_f_pi {
		*allpassFrequencyPiParam0,
		*allpassFrequencyPiParam1,
		*allpassFrequencyPiParam2,
		*allpassFrequencyPiParam3
	};
	
	std::array<float, 4> _ap_fb {
		*allpassBandwidthParam0,
		*allpassBandwidthParam1,
		*allpassBandwidthParam2,
		*allpassBandwidthParam3
	};
	
	/* these cannot be std::arrays until i redo  nvs::memoryless::metaparamA */
    float inner_f_pi[2], outer_f_pi[2];
    float inner_f_b[2], outer_f_b[2];

    // ACTUALLY RESHAPE FROM 2 METAPARAMS INTO 4 PARAMS:
    auto dist1Iner = paramVT.getRawParameterValue(param_stuff::paramIDs.at(param_stuff::params_e::dist1_inner));
    auto dist1Outr = paramVT.getRawParameterValue(param_stuff::paramIDs.at(param_stuff::params_e::dist1_outer));
    auto dist2Iner = paramVT.getRawParameterValue(param_stuff::paramIDs.at(param_stuff::params_e::dist2_inner));
    auto dist2Outr = paramVT.getRawParameterValue(param_stuff::paramIDs.at(param_stuff::params_e::dist2_outer));
    
    nvs::memoryless::metaparamA(float(*dist1Iner), inner_f_pi);
    nvs::memoryless::metaparamA(float(*dist1Outr), outer_f_pi);
    nvs::memoryless::metaparamA(float(*dist2Iner), inner_f_b);
    nvs::memoryless::metaparamA(float(*dist2Outr), outer_f_b);
    
    //=============================================================================
    float current_preDtime[nIn];
    for (int i = 0; i < nIn; i++) {
		current_preDtime[i] = nvs::memoryless::clamp(_predel, minDelTimeMS, maxDelTimeMS);
    }
	
	// yes these are not an efficient way to update all this stuff... will fix later
    for (int i = 0; i < D_IJ; i++) {
        D_times_ranged[i] = *(times[i]);
    }
	
    std::array<float, D_IJ> current_Dtime;
    for (int i = 0; i < D_IJ; i++) {
        current_Dtime[i] = D_times_ranged[i];
        current_Dtime[i] *= _size;
    }
	
	for (int i = 0; i < D_IJ; ++i){
		tvap[i].setCutoffTarget(_ap_f_pi[i]);
		tvap[i].setResonanceTarget(_ap_fb[i]);
	}
	
    for (int n = 0; n < (nIn + nOut); n++)  {
        hp6dB[n].setCutoffTarget(_lop);
    }
    
    for (int samp = 0; samp < numSamps; samp++)
    {
        for (int i = 0; i < nIn; i++){
            preD[i].updateDelayTimeMS(current_preDtime[i], (float)_oneOverBlockSize);
        }
        for (int i = 0; i < D_IJ; i++) {
            D[i].updateDelayTimeMS(nvs::memoryless::clamp
				   (current_Dtime[i] * timeScaling, minDelTimeMS, maxDelTimeMS), (float)_oneOverBlockSize);
            apd[i].updateDelayTimeMS(nvs::memoryless::clamp
					(current_Dtime[i] * timeScaling, minDelTimeMS, maxDelTimeMS), (float)_oneOverBlockSize);
            
            tvap[i].update_f_pi();
            tvap[i].update_f_b();
        }
        
        float leftSamp = *(inBuffL + samp);
        float rightSamp = *(inBuffR + samp);
        
        float inDrive = juce::Decibels::decibelsToGain<float>(_drive);
        float outDrive = 1.f / inDrive;
        
        float preDelSamp[nIn];
        
        if (nIn > 0){   // could be optimized to avoid branch
            preDelSamp[0]  = preD[0].tick_cubic(leftSamp);
            if (nIn == 2){
                preDelSamp[1] = preD[1].tick_cubic(rightSamp);
            }
        }
        
        X[0] = 0.f;
        X[1] = preDelSamp[0] * inDrive;
        X[2] = preDelSamp[1] * inDrive;
        X[3] = 0.f;

        float tmp[4] {0.f, 0.f, 0.f, 0.f};
        
//        Y[0] = tvap[0].filter_fbmod(Y[0], _ap_dist1, _ap_dist2);
//        Y[1] = tvap[1].filter_fbmod(Y[1], _ap_dist1, _ap_dist2);
//        Y[2] = tvap[2].filter_fbmod(Y[2], _ap_dist1, _ap_dist2);
//        Y[3] = tvap[3].filter_fbmod(Y[3], _ap_dist1, _ap_dist2);
        
        for (int i = 0; i < D_IJ; i++) {
            int j;
            for (j = 0; j < D_IJ; j++) {
                tmp[i] += G[i][j] * Y[j] * (_g);
            }
            tmp[i] += X[i];
			for (int j = 0; j < D_IJ; ++j){
				tmp[0] = butters[0](tmp[0], _lop);
				tmp[1] = butters[1](tmp[1], _lop);
				tmp[2] = butters[2](tmp[2], _lop);
				tmp[3] = butters[3](tmp[3], _lop);
			}
			for (int j = 0; j < D_IJ; ++j){
				tmp[0] = hp6dB[0](tmp[0], _hip);
				tmp[1] = hp6dB[1](tmp[1], _hip);
				tmp[2] = hp6dB[2](tmp[2], _hip);
				tmp[3] = hp6dB[3](tmp[3], _hip);
			}
        }
/*L	INTERNAL*/ tmp[0] = tvap[0].filter_fbmod(tmp[0], inner_f_pi[0], inner_f_b[0]);
/*L DIRECT*/ tmp[1] = tvap[1].filter_fbmod(tmp[1], outer_f_pi[0], outer_f_b[0]);
/*R DIRECT*/ tmp[2] = tvap[2].filter_fbmod(tmp[2], outer_f_pi[1], outer_f_b[1]);
/*R INTERNAL*/ tmp[3] = tvap[3].filter_fbmod(tmp[3], inner_f_pi[1], inner_f_b[1]);
        
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
//             + D[0][2].tick(tmp[2]) + D[0][3].tick(tmp[3]);
//        Y[1] = D[1][0].tick(tmp[0]) + D[1][1].tick(tmp[1])
//             + D[1][2].tick(tmp[2]) + D[1][3].tick(tmp[3]);
//        Y[2] = D[2][0].tick(tmp[0]) + D[2][1].tick(tmp[1])
//             + D[2][2].tick(tmp[2]) + D[2][3].tick(tmp[3]);
//        Y[3] = D[3][0].tick(tmp[0]) + D[3][1].tick(tmp[1])
//             + D[3][2].tick(tmp[2]) + D[3][3].tick(tmp[3]);

        float outGain = juce::Decibels::decibelsToGain<float>(_outputGain);

        float wetL = Y[1];
        float wetR = Y[2];
        
        wetL *= outGain * outDrive;
        wetR *= outGain * outDrive;

#if PROTECT_OUTPUT
        wetL = nvs::memoryless::clamp1<float>(wetL);
        wetR = nvs::memoryless::clamp1<float>(wetR);
#endif
        float finalOutL = wetL * _dryWet + leftSamp * (1 - _dryWet) ;
        float finalOutR = wetR * _dryWet + rightSamp * (1 - _dryWet) ;

        
        *(outBuffL + samp) = finalOutL * 0.999;
        *(outBuffR + samp) = finalOutR * 0.999;
    }
}
#if DEF_EDITOR
//==============================================================================
bool ShredVerbAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}
juce::AudioProcessorEditor* ShredVerbAudioProcessor::createEditor()
{
    return new foleys::MagicPluginEditor(magicState);
//    return new ShredVerbAudioProcessorEditor (*this, paramVT);
}
//==============================================================================
void ShredVerbAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
        auto state = paramVT.copyState();
        std::unique_ptr<juce::XmlElement> xml (state.createXml());
        copyXmlToBinary (*xml, destData);
}


#endif
void ShredVerbAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    /*std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (paramVT.state.getType()))
            paramVT.replaceState (juce::ValueTree::fromXml (*xmlState));*/
}
void ShredVerbAudioProcessor::addReverbParameters (juce::AudioProcessorValueTreeState::ParameterLayout& layout){
    auto predel  = getUniqueParam(param_stuff::params_e::predelay);
    auto size = getUniqueParam(param_stuff::params_e::size);
    auto decay = getUniqueParam(param_stuff::params_e::decay);
    auto lowpass = getUniqueParam(param_stuff::params_e::lowpass);
    auto hipass = getUniqueParam(param_stuff::params_e::highpass);

    predel->range.setSkewForCentre(250.f);
    
    size->range.setSkewForCentre(0.66f);
    
    decay->range.setSkewForCentre(0.55f);
    decay->range.interval = 0.01f;

    lowpass->range.setSkewForCentre(4000.f);
    hipass->range.setSkewForCentre(200.f);

    auto group = std::make_unique<juce::AudioProcessorParameterGroup>("qualia", "QUALIA", "|",
                                                                      std::move (predel),
                                                                      std::move (size),
                                                                      std::move (decay),
                                                                      std::move (hipass),
                                                                      std::move (lowpass));
    layout.add (std::move (group));
}
void ShredVerbAudioProcessor::addDelayParameters (juce::AudioProcessorValueTreeState::ParameterLayout& layout){
    std::unique_ptr<juce::AudioParameterFloat> time0  = getUniqueParam(param_stuff::params_e::time0);
    std::unique_ptr<juce::AudioParameterFloat> time1  = getUniqueParam(param_stuff::params_e::time1);
    std::unique_ptr<juce::AudioParameterFloat> time2  = getUniqueParam(param_stuff::params_e::time2);
    std::unique_ptr<juce::AudioParameterFloat> time3  = getUniqueParam(param_stuff::params_e::time3);
        
    float centrVal = 0.7f;
    float interval = 0.0002f;
    time0->range.setSkewForCentre(centrVal);
    time0->range.interval = interval;
    time1->range.setSkewForCentre(centrVal);
    time1->range.interval = interval;
    time2->range.setSkewForCentre(centrVal);
    time2->range.interval = interval;
    time3->range.setSkewForCentre(centrVal);
    time3->range.interval = interval;

    auto group = std::make_unique<juce::AudioProcessorParameterGroup>("timings", "TIMINGS", "|",
                                                                      std::move (time0),
                                                                      std::move (time1),
                                                                      std::move (time2),
                                                                      std::move (time3));
    layout.add (std::move (group));
}
void ShredVerbAudioProcessor::addAllpassParameters (juce::AudioProcessorValueTreeState::ParameterLayout& layout){
    auto getRange = [&](param_stuff::params_e idx){return juce::NormalisableRange<float>(getMin(idx), getMax(idx),
        [](float start, float end, float normalised)    // convertFrom0To1Func
        {
            return start + (std::pow (2.0f, normalised * 10.0f) - 1.0f) * (end - start) / 1023.0f;
        },
        [](float start, float end, float value)         // convertTo0To1Func
        {
            return (std::log (((value - start) * 1023.0f / (end - start)) + 1.0f) / std::log ( 2.0f)) / 10.0f;
        },
        [](float start, float end, float value)         // snapToLegalValueFunc
        {
            if (value > 3000.0f)
                return juce::jlimit (start, end, 10.0f * juce::roundToInt (value / 10.0f));

            if (value > 1000.0f)
                return juce::jlimit (start, end, 1.0f * juce::roundToInt (value / 1.0f));

            return juce::jlimit (start, end, 0.1f * float (juce::roundToInt (value * 10)));
    });};
    
    auto get_param = [&](param_stuff::params_e idx){
        return std::make_unique<juce::AudioParameterFloat>(
                                juce::ParameterID (getID(idx), versionHint),
                                getName(idx),
                                getRange(idx),
                                getDef(idx),
                                param_stuff::paramNames.at(idx),
                                juce::AudioProcessorParameter::genericParameter,
                                [](float value, int) { return juce::String (value, 2); },
                                [](const juce::String& text) { return text.getFloatValue(); } );
    };

    auto ap0f = get_param(param_stuff::params_e::tvap0_f_pi);
    auto ap0b = get_param(param_stuff::params_e::tvap0_f_b);
    auto ap1f = get_param(param_stuff::params_e::tvap1_f_pi);
    auto ap1b = get_param(param_stuff::params_e::tvap1_f_b);
    auto ap2f = get_param(param_stuff::params_e::tvap2_f_pi);
    auto ap2b = get_param(param_stuff::params_e::tvap2_f_b);
    auto ap3f = get_param(param_stuff::params_e::tvap3_f_pi);
    auto ap3b = get_param(param_stuff::params_e::tvap3_f_b);

    auto group = std::make_unique<juce::AudioProcessorParameterGroup>("allpass", "ALLPASS", "|",
                                                                      std::move (ap0f),
                                                                      std::move (ap0b),
                                                                      std::move (ap1f),
                                                                      std::move (ap1b),
                                                                      std::move (ap2f),
                                                                      std::move (ap2b),
                                                                      std::move (ap3f),
                                                                      std::move (ap3b));
    layout.add (std::move (group));
}
void ShredVerbAudioProcessor::addDistorionParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout){
    float distInterval = 0.0001f;
    auto getUniqueDistParam = [&](param_stuff::params_e idx){
        return std::make_unique<juce::AudioParameterFloat>(juce::ParameterID (getID(idx), versionHint), getName(idx), juce::NormalisableRange<float> (getMin(idx), getMax(idx), distInterval), getDef(idx));
    };
    
    auto drive  = getUniqueDistParam(param_stuff::params_e::drive);
    auto inner1 = getUniqueDistParam(param_stuff::params_e::dist1_inner);
    auto outer1 = getUniqueDistParam(param_stuff::params_e::dist1_outer);
    auto inner2 = getUniqueDistParam(param_stuff::params_e::dist2_inner);
    auto outer2 = getUniqueDistParam(param_stuff::params_e::dist2_outer);
    
    float centrVal = 0.2f;
    inner1->range.setSkewForCentre(centrVal);
    inner2->range.setSkewForCentre(centrVal);
    outer1->range.setSkewForCentre(centrVal);
    outer2->range.setSkewForCentre(centrVal);
    inner1->range.interval = distInterval / 25.f;
    inner2->range.interval = distInterval / 25.f;
    outer1->range.interval = distInterval / 25.f;
    outer2->range.interval = distInterval / 25.f;

    auto group = std::make_unique<juce::AudioProcessorParameterGroup>("shred", "SHRED", "|",
                                                                      std::move (drive),
                                                                      std::move (inner1),
                                                                      std::move (outer1),
                                                                      std::move (inner2),
                                                                      std::move (outer2));
    layout.add (std::move (group));
}
void ShredVerbAudioProcessor::addModulationParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout)
{}
void ShredVerbAudioProcessor::addOutputParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout){
    auto drywet = getUniqueParam(param_stuff::params_e::drywet);
    auto output = getUniqueParam(param_stuff::params_e::output_gain);

    auto group = std::make_unique<juce::AudioProcessorParameterGroup>("output", "OUTPUT", "|",
                                                                      std::move (drywet),
                                                                      std::move (output));
    layout.add (std::move (group));
}
juce::AudioProcessorValueTreeState::ParameterLayout ShredVerbAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    ShredVerbAudioProcessor::addReverbParameters (layout);
    ShredVerbAudioProcessor::addDelayParameters (layout);
    ShredVerbAudioProcessor::addAllpassParameters (layout);
    ShredVerbAudioProcessor::addDistorionParameters (layout);
    ShredVerbAudioProcessor::addOutputParameters (layout);
    return layout;
}
// OLD WAY
juce::AudioProcessorValueTreeState::ParameterLayout ShredVerbAudioProcessor::createParams()
{
    const unsigned int versionHint = 1;
//    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    juce::AudioProcessorValueTreeState::ParameterLayout params;

    for (int i = 0; i < ps.numParams; ++i ){
        param_stuff::params_e ii = (param_stuff::params_e)(i);

        juce::String parameterID_str = ps.paramIDs.at(ii);
        juce::String parameterName = ps.paramNames.at(ii);
        juce::NormalisableRange<float> normRange;
        if ((parameterID_str.compare(ps.paramIDs.at(param_stuff::params_e::tvap0_f_pi)) &&
            parameterID_str.compare(ps.paramIDs.at(param_stuff::params_e::tvap0_f_b)) &&
            parameterID_str.compare(ps.paramIDs.at(param_stuff::params_e::tvap1_f_pi)) &&
            parameterID_str.compare(ps.paramIDs.at(param_stuff::params_e::tvap1_f_b)) &&
            parameterID_str.compare(ps.paramIDs.at(param_stuff::params_e::tvap2_f_pi)) &&
            parameterID_str.compare(ps.paramIDs.at(param_stuff::params_e::tvap2_f_b)) &&
            parameterID_str.compare(ps.paramIDs.at(param_stuff::params_e::tvap3_f_pi)) &&
            parameterID_str.compare(ps.paramIDs.at(param_stuff::params_e::tvap3_f_b))
              ))
        {
            normRange = juce::NormalisableRange<float> (ps.paramRanges.at(ii)[0],
                                                 ps.paramRanges.at(ii)[1],
                                                 0.0001f);
        } else {
            normRange = juce::NormalisableRange<float> {0.23f, 22000.0f,
                [](float start, float end, float normalised)
                {
                    return start + (std::pow (2.0f, normalised * 10.0f) - 1.0f) * (end - start) / 1023.0f;
                },
                [](float start, float end, float value)
                {
                    return (std::log (((value - start) * 1023.0f / (end - start)) + 1.0f) / std::log ( 2.0f)) / 10.0f;
                },
                [](float start, float end, float value)
                {
                    if (value > 3000.0f)
                        return juce::jlimit (start, end, 100.0f * juce::roundToInt (value / 100.0f));

                    if (value > 1000.0f)
                        return juce::jlimit (start, end, 10.0f * juce::roundToInt (value / 10.0f));

                    return juce::jlimit (start, end, float (juce::roundToInt (value)));
                }};
        }
        float defaultVal = ps.paramDefaults.at(ii);
        
//        params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID {parameterID_str, versionHint}, parameterName, normRange, defaultVal));
        params.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID {parameterID_str, versionHint}, parameterName, normRange, defaultVal));
    }
    return params;
//    return { params.begin(), params.end() };
}


//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ShredVerbAudioProcessor();
}
