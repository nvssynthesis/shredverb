/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"

#include "PresetPanel.h"
#include "Service/PresetManager.h"

#if DEF_EDITOR
#include "PluginEditor.h"
#else
#endif

/**
 TODO:
	-whatever happened to the 4 delays being allpass delays, with the little 'g' knobs in the middle?
	-replace one pole highpass with butterworth highpass
	-improve effect of altering various inner/outer distortions
	-perhaps add SVF bandpass filter as INPUT stage to the TVAP modulation inputs
		**-overcomplicate UI? should its f_c and q be tied to that of TVAP?
**	-tvaps should be controlled by XYpad instead of 2 knobs?
	-maybe double each of the TVAPs in series to amplify their phasial effect
	-drive should only affect the scaling of the control signals. this would also resolve the unintuitive makeup gain.
 
	-improve quality (dezippering, equal power) of output section
	-add linking option for the 2 sides of outer distortion
 */

#define PROTECT_OUTPUT 1
//==============================================================================
ShredVerbAudioProcessor::ShredVerbAudioProcessor()	:
	foleys::MagicProcessor (juce::AudioProcessor::BusesProperties()
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                       ),
	paramVT(*this, nullptr, juce::Identifier ("APVTSshredverb"), createParameterLayout())
{
    FOLEYS_SET_SOURCE_PATH (__FILE__);

	// set GUI
    // this is how i was loading default, but docs actually say to do this as return... in createEditor
    magicState.setGuiValueTree (BinaryData::_19_9_25_xml, BinaryData::_19_9_25_xmlSize);
	
	initializeParameterPointers();
	
	for (auto &pd : preDelays){
		pd.setInterpolation(nvs::delays::interp_e::floor);
		pd.setDelayTimeMS(param::paramDefaults.at(param::params_e::predelay));
	}
	
	D_times_ranged[0] = param::paramDefaults.at(param::params_e::time0);
	D_times_ranged[1] = param::paramDefaults.at(param::params_e::time1);
	D_times_ranged[2] = param::paramDefaults.at(param::params_e::time2);
	D_times_ranged[3] = param::paramDefaults.at(param::params_e::time3);
	
	X.fill(0.f);
	Y.fill(0.f);
	
    for (int n = 0; n < D_IJ; n++) {
        D[n].setDelayTimeMS(D_times_ranged[n] * timeScaling);
        D[n].setInterpolation(nvs::delays::interp_e::floor);
	}
	for (auto &bp : fm_bp){
		bp.setMode(nvs::filters::mode_e::BP);
	}
	for (auto &hp : hp6dB){
		hp.setMode(nvs::filters::mode_e::HP);
	}
}
void ShredVerbAudioProcessor::initializeParameterPointers()
{
	// Use initializer list for cleaner setup
	std::vector<param::params_e> allParams = {
		param::params_e::drive,
		param::params_e::predelay,
		param::params_e::decay,
		param::params_e::size,
		param::params_e::lowpass,
		param::params_e::highpass,
		param::params_e::drywet,
		param::params_e::wet_gain,
		param::params_e::interp_type,
		param::params_e::randomize,
		param::params_e::dist1_inner,
		param::params_e::dist1_outer,
		param::params_e::dist2_inner,
		param::params_e::dist2_outer
	};
	
	allParams.insert(allParams.end(), param::TVAP_F_PI_PARAMS.begin(), param::TVAP_F_PI_PARAMS.end());
	allParams.insert(allParams.end(), param::TVAP_F_B_PARAMS.begin(), param::TVAP_F_B_PARAMS.end());
	allParams.insert(allParams.end(), param::TIME_PARAMS.begin(), param::TIME_PARAMS.end());
	allParams.insert(allParams.end(), param::DELAY_GAIN_PARAMS.begin(), param::DELAY_GAIN_PARAMS.end());
	allParams.insert(allParams.end(), param::DISTORTION_PARAMS.begin(), param::DISTORTION_PARAMS.end());
	
	for (auto paramId : allParams) {
		paramPtrs[paramId] = paramVT.getRawParameterValue(param::paramIDs.at(paramId));
	}
}
float ShredVerbAudioProcessor::getParam(param::params_e paramId) const
{
	auto it = paramPtrs.find(paramId);
	jassert(it != paramPtrs.end());
	return it->second->load();
}
template<size_t N>
std::array<float, N> ShredVerbAudioProcessor::getParamArray(const std::array<param::params_e, N>& paramIds) const
{
	std::array<float, N> result;
	std::ranges::transform(paramIds, result.begin(), [this](auto paramId) { return getParam(paramId); });
	return result;
}
template<size_t N>
void ShredVerbAudioProcessor::randomizeParams(std::array<param::params_e, N> params){
	for (auto p : params){
		float val = rando.nextFloat();
		
		auto param = paramVT.getParameter(param::paramIDs.at(p));
		
		param->beginChangeGesture();
		param->setValueNotifyingHost(juce::jlimit(0.f, 1.f, val));
		param->endChangeGesture();
	}
}

void ShredVerbAudioProcessor::randomizeDelays(){
	std::array<param::params_e, 8> dParams = {
		param::params_e::time0,
		param::params_e::time1,
		param::params_e::time2,
		param::params_e::time3,
		param::params_e::g0,
		param::params_e::g1,
		param::params_e::g2,
		param::params_e::g3
	};
	randomizeParams(dParams);
}
void ShredVerbAudioProcessor::randomizeQualia(){
	std::array<param::params_e, 5> dParams = {
		param::params_e::predelay,
		param::params_e::size,
		param::params_e::highpass,
		param::params_e::lowpass,
		param::params_e::decay
	};
	randomizeParams(dParams);
}
void ShredVerbAudioProcessor::randomizeCharacter(){
	std::array<param::params_e, 5> dParams = {
		param::params_e::drive,
		param::params_e::dist1_inner,
		param::params_e::dist1_outer,
		param::params_e::dist2_inner,
		param::params_e::dist2_outer
	};
	randomizeParams(dParams);
}
void ShredVerbAudioProcessor::randomizeAllpass(){
	std::array<param::params_e, 8> dParams = {
		param::params_e::tvap0_f_b,
		param::params_e::tvap1_f_b,
		param::params_e::tvap2_f_b,
		param::params_e::tvap3_f_b,
		param::params_e::tvap0_f_pi,
		param::params_e::tvap1_f_pi,
		param::params_e::tvap2_f_pi,
		param::params_e::tvap3_f_pi
	};
	randomizeParams(dParams);
}
void ShredVerbAudioProcessor::randomizeParams(){
	randomizeDelays();
	randomizeQualia();
	randomizeCharacter();
	randomizeAllpass();
}

#if DEF_EDITOR
//==============================================================================
const juce::String ShredVerbAudioProcessor::getName() const{return JucePlugin_Name;}
bool ShredVerbAudioProcessor::acceptsMidi() const {
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}
bool ShredVerbAudioProcessor::producesMidi() const {
    return false;
}
bool ShredVerbAudioProcessor::isMidiEffect() const {
    return false;
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
    
	maxPreDelTimeMS = float((preDelays[0].getDelaySize() - 10) * 1000) / (float)sampleRate; // no more than buffer length - 10

    maxDelTimeMS = float((D[0].getDelaySize() - 10) * 1000) / ((float)sampleRate * D[0].getLargestRatio()); // no more than buffer length - 10
    minDelTimeMS = float(3 * 1000) / (float)sampleRate; // no less than 3 samples
    
	//feed forward pair
	for (auto &pd : preDelays){
		pd.clear();
		pd.setSampleRate(sampleRate);
#pragma message("set predelay block size!")
	}
	
    for (int n = 0; n < D_IJ; n++) {
        tvap[n].clear();
        tvap[n].setSampleRate(sampleRate);
		tvap[n].setBlockSize(samplesPerBlock);
    }
	for (auto &bp : fm_bp){
		bp.clear();
		bp.setSampleRate(sampleRate);
		bp.setBlockSize(samplesPerBlock);
	}

	for (auto &d : D)  {
		d.clear();
		d.setSampleRate((float)sampleRate);
#pragma message("set delay block size!")
	}
    for (int n = 0; n < (nIn + nOut); n++)
    {
		hp6dB[n].clear();
		hp6dB[n].setSampleRate((float)sampleRate);
		hp6dB[n].setBlockSize(samplesPerBlock);
    }
    for (auto &b : butters){
		b.clear();
        b.setSampleRate(sampleRate);
		b.setBlockSize(samplesPerBlock);
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
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}
#endif

void ShredVerbAudioProcessor::parameterChanged (const juce::String& param, float value){}

void ShredVerbAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto const nIn  = getTotalNumInputChannels();
    auto const nOut = getTotalNumOutputChannels();
    int const numSamps = buffer.getNumSamples();
	
	for (auto i = nIn; i < nOut; ++i){
		buffer.clear (i, 0, numSamps);
	}
	
	[[deprecated]]	// any function using this param during processBlock should be deprecated
    double const oneOverBlockSize = 1.0 / (double)numSamps;
	
	std::array<float const* const, 2> inBuff {
		buffer.getReadPointer(0),
		(getTotalNumInputChannels() > 1) ? buffer.getReadPointer(1) : buffer.getReadPointer(0)
	};

	std::array<float* const, 2> outBuff {
		buffer.getWritePointer(0),
		(getTotalNumOutputChannels() > 1) ? buffer.getWritePointer(1) : buffer.getWritePointer(0)
	};
	auto const times = getParamArray(param::TIME_PARAMS);
	auto const delayGains = getParamArray(param::DELAY_GAIN_PARAMS);
	auto const allpassFreqs = getParamArray(param::TVAP_F_PI_PARAMS);
	auto const allpassBandwidths = getParamArray(param::TVAP_F_B_PARAMS);
	auto const distortionParams = getParamArray(param::DISTORTION_PARAMS);
	
	auto const [dryAmt, wetAmt] = [this]() {
		auto const wet = getParam(param::params_e::drywet) / 100.0f;
		return std::make_pair(std::sqrt(1.0f - wet), std::sqrt(wet));
	}();
	
	float const wetGain = juce::Decibels::decibelsToGain<float>(getParam(param::params_e::wet_gain));
	float const sizeVal = getParam(param::params_e::size);
	float const decay = getParam(param::params_e::decay);
	float const inDrive = juce::Decibels::decibelsToGain<float>(getParam(param::params_e::drive));

    for (int i = 0; i < D_IJ; i++) {
        D_times_ranged[i] = times[i];
    }
	
    Array4 current_Dtime;
    for (int i = 0; i < D_IJ; i++) {
        current_Dtime[i] = D_times_ranged[i];
        current_Dtime[i] *= sizeVal;
    }
	
	for (int i = 0; i < D_IJ; ++i){
		float bw = allpassBandwidths[i]; //_ap_fb[i];
		tvap[i].setCutoffTarget(allpassFreqs[i]); //(_ap_f_pi[i]);
		tvap[i].setResonanceTarget(bw);
		
		fm_bp[i].setCutoffTarget(allpassFreqs[i]);//(_ap_f_pi[i]);
		
		bw = nvs::memoryless::clamp_low(bw, 0.2f);
		float reso = /*_ap_f_pi[i]*/ allpassFreqs[i] / bw;
		fm_bp[i].setResonanceTarget(reso);
	}
	
	for (auto &filt : butters){
		filt.setCutoffTarget(getParam(param::params_e::lowpass));
	}
    for (auto &filt : hp6dB)  {
        filt.setCutoffTarget(getParam(param::params_e::highpass));
    }
	
    for (int samp = 0; samp < numSamps; samp++)
    {
        for (auto &pd : preDelays){
            pd.updateDelayTimeMS(getParam(param::params_e::predelay), (float)oneOverBlockSize);
        }
        for (int i = 0; i < D_IJ; i++) {
            D[i].updateDelayTimeMS(nvs::memoryless::clamp
										(current_Dtime[i] * timeScaling, minDelTimeMS, maxDelTimeMS),
									(float)oneOverBlockSize);
			D[i].update_g(delayGains[i] * decay, (float)oneOverBlockSize);
        }
		for (auto &filt : tvap){
			filt.update_f_pi();
			filt.update_f_b();
		}
		for (auto &filt : fm_bp){
			filt.updateCutoff();
			filt.updateResonance();
		}
		for (auto &f : hp6dB){
			f.updateCutoff();
		}
		for (auto &f : butters){
			f.updateCutoff();
		}
//=============================================================================
		// now the actual signal path
		
		// preserve these until end for dry/wet mixing
		std::array<float, 2> inSamps {
			*(inBuff[0] + samp),
			*(inBuff[1] + samp)
		};
		
        {	// scope for preDelSamp
			std::array<float, 2> preDelSamp;
			
			for (auto i = 0; i < preDelays.size(); ++i){
				preDelSamp[i]  = preDelays[i].tick_cubic(inSamps[i]);
			}
			
			X[0] = 0.f;
			X[1] = preDelSamp[0];// * inDrive;
			X[2] = preDelSamp[1];// * inDrive;
			X[3] = 0.f;
        }

        Array4 tmp {0.f, 0.f, 0.f, 0.f};
        
		/* G:
			{0.f,  1.f,  1.f,  0.f},
			{-1.f, 0.f,  0.f, -1.f},
			{1.f,  0.f,  0.f, -1.f},
			{0.f,  1.f, -1.f,  0.f}
		*/
		auto const g = decay * 0.707106781186548;
        for (int i = 0; i < D_IJ; i++) {
            for (int j = 0; j < D_IJ; j++) {
                tmp[i] += G[i][j] * Y[j] * (g);
            }
            tmp[i] += X[i];
			for (int j = 0; j < D_IJ; ++j){
				tmp[j] = butters[j](tmp[j]);
			}
			for (int j = 0; j < D_IJ; ++j){
				tmp[j] = hp6dB[j](tmp[j]);
			}
        }
#if CLASSIC_WAY
/*L	INTERNAL*/ tmp[0] = tvap[0].filter_fbmod(tmp[0], inner_f_pi[1], inner_f_b[0]);
/*L DIRECT*/ tmp[1] = tvap[1].filter_fbmod(tmp[1], outer_f_pi[0], outer_f_b[1]);
/*R DIRECT*/ tmp[2] = tvap[2].filter_fbmod(tmp[2], outer_f_pi[1], outer_f_b[0]);
/*R INTERNAL*/ tmp[3] = tvap[3].filter_fbmod(tmp[3], inner_f_pi[0], inner_f_b[1]);
#else
		Array4 intrnlWcModSig = {
			nvs::memoryless::unboundSat2(fm_bp[0](tmp[0]) * 1000000000.f) * 100.f * inDrive,
			nvs::memoryless::unboundSat2(fm_bp[1](tmp[1]) * 1000000000.f) * 100.f * inDrive,
			nvs::memoryless::unboundSat2(fm_bp[2](tmp[2]) * 1000000000.f) * 100.f * inDrive,
			nvs::memoryless::unboundSat2(fm_bp[3](tmp[3]) * 1000000000.f) * 100.f * inDrive
		};
#pragma message("need to smooth these cutoff/reso if doing it this way")
		tmp[0] = tvap[0](tmp[0], allpassFreqs[0] + distortionParams[0]*intrnlWcModSig[0], allpassBandwidths[0]); /*L INTERNAL*/
		tmp[1] = tvap[1](tmp[1], allpassFreqs[1] + distortionParams[1]*intrnlWcModSig[1], allpassBandwidths[1]); /*L DIRECT*/
		tmp[2] = tvap[2](tmp[2], allpassFreqs[2] + distortionParams[2]*intrnlWcModSig[2], allpassBandwidths[2]); /*R DIRECT*/
		tmp[3] = tvap[3](tmp[3], allpassFreqs[3] + distortionParams[3]*intrnlWcModSig[3], allpassBandwidths[3]); /*R INTERNAL*/
#endif
        Y[0] = D[0](tmp[0]);
        Y[1] = D[1](tmp[1]);
        Y[2] = D[2](tmp[2]);
        Y[3] = D[3](tmp[3]);

		std::array<float, 2> wet {
			Y[1] * wetGain,// * outDrive,
			Y[2] * wetGain // * outDrive
		};

#if PROTECT_OUTPUT
		for (auto &w : wet){
			w = nvs::memoryless::clamp(w, -2.5f, 2.5f);
		}
#endif
		std::array<float, 2> finalOut {
			dryAmt * inSamps[0] + wetAmt * wet[0],
			dryAmt * inSamps[1] + wetAmt * wet[1]
		};
		
		for (int i = 0; i < 2; ++i){
			*(outBuff[i] + samp) = finalOut[i] * 0.999;
		}
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
}
#endif

//==============================================================================
void ShredVerbAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
	auto state = paramVT.copyState();
	std::unique_ptr<juce::XmlElement> xml (state.createXml());
	copyXmlToBinary (*xml, destData);
}
void ShredVerbAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (paramVT.state.getType()))
            paramVT.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ShredVerbAudioProcessor();
}
//==============================================================================

void addReverbParameters (juce::AudioProcessorValueTreeState::ParameterLayout& layout){
	auto predel  = param::bast::createParam(param::params_e::predelay);
	auto size = param::bast::createParam(param::params_e::size);
	auto decay = param::bast::createParam(param::params_e::decay);
	auto lowpass = param::bast::createParam(param::params_e::lowpass);
	auto hipass = param::bast::createParam(param::params_e::highpass);

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
void addDelayParameters (juce::AudioProcessorValueTreeState::ParameterLayout& layout){
	std::unique_ptr<juce::AudioParameterFloat> time0  = param::bast::createParam(param::params_e::time0);
	std::unique_ptr<juce::AudioParameterFloat> time1  = param::bast::createParam(param::params_e::time1);
	std::unique_ptr<juce::AudioParameterFloat> time2  = param::bast::createParam(param::params_e::time2);
	std::unique_ptr<juce::AudioParameterFloat> time3  = param::bast::createParam(param::params_e::time3);
		
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
	
	std::array<
		std::unique_ptr<juce::AudioParameterFloat>,
	4> gs {
		param::bast::createParam(param::params_e::g0),
		param::bast::createParam(param::params_e::g1),
		param::bast::createParam(param::params_e::g2),
		param::bast::createParam(param::params_e::g3)
	};
	
	centrVal = 0.f;
	interval = 0.f;
	for (auto & uptr : gs){
		uptr->range.setSkewForCentre(centrVal);
		uptr->range.interval = interval;
	}
	
	group = std::make_unique<juce::AudioProcessorParameterGroup>("diffusions", "DIFFUSIONS", "|",
																	  std::move (gs[0]),
																	  std::move (gs[1]),
																	  std::move (gs[2]),
																	  std::move (gs[3]));
	layout.add(std::move(group));
}
void addAllpassParameters (juce::AudioProcessorValueTreeState::ParameterLayout& layout){

	auto ap0f = param::bast::createParam(param::params_e::tvap0_f_pi);
	auto ap0b = param::bast::createParam(param::params_e::tvap0_f_b);
	auto ap1f = param::bast::createParam(param::params_e::tvap1_f_pi);
	auto ap1b = param::bast::createParam(param::params_e::tvap1_f_b);
	auto ap2f = param::bast::createParam(param::params_e::tvap2_f_pi);
	auto ap2b = param::bast::createParam(param::params_e::tvap2_f_b);
	auto ap3f = param::bast::createParam(param::params_e::tvap3_f_pi);
	auto ap3b = param::bast::createParam(param::params_e::tvap3_f_b);

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
void addDistorionParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout){
	
	auto drive  = param::bast::createParam(param::params_e::drive);
	auto inner1 = param::bast::createParam(param::params_e::dist1_inner);
	auto outer1 = param::bast::createParam(param::params_e::dist1_outer);
	auto inner2 = param::bast::createParam(param::params_e::dist2_inner);
	auto outer2 = param::bast::createParam(param::params_e::dist2_outer);
	
	float centrVal = 0.2f;
	inner1->range.setSkewForCentre(centrVal);
	inner2->range.setSkewForCentre(centrVal);
	outer1->range.setSkewForCentre(centrVal);
	outer2->range.setSkewForCentre(centrVal);
	
	auto group = std::make_unique<juce::AudioProcessorParameterGroup>("shred", "SHRED", "|",
																	  std::move (drive),
																	  std::move (inner1),
																	  std::move (outer1),
																	  std::move (inner2),
																	  std::move (outer2));
	layout.add (std::move (group));
}
void addModulationParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout)
{
#pragma message("we should add some modulation params")
}
void addOutputParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout){
	auto drywet = param::bast::createParam(param::params_e::drywet);
	auto wet_gain = param::bast::createParam(param::params_e::wet_gain);

	auto group = std::make_unique<juce::AudioProcessorParameterGroup>("output", "OUTPUT", "|",
																	  std::move (drywet),
																	  std::move (wet_gain));
	layout.add (std::move (group));
}

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
	juce::AudioProcessorValueTreeState::ParameterLayout layout;
	addReverbParameters (layout);
	addDelayParameters (layout);
	addAllpassParameters (layout);
	addDistorionParameters (layout);
	addOutputParameters (layout);
	return layout;
}


//=====================================================GUI STUFF=====================================================

class PresetPanelItem : public foleys::GuiItem
{
public:
	FOLEYS_DECLARE_GUI_FACTORY(PresetPanelItem)
	
	PresetPanelItem (foleys::MagicGUIBuilder& builder, const juce::ValueTree& node) : foleys::GuiItem (builder, node)
	{}

	std::vector<foleys::SettableProperty> getSettableProperties() const override
	{
		return {};
	}

	void update() override
	{
		_presetPanel.reset();
		auto *manager = magicBuilder.getMagicState().getObjectWithType<nvs::service::PresetManager>("PresetManager");
		if (manager != nullptr){
			_presetPanel = std::make_unique<nvs::gui::PresetPanel>(manager);
			addAndMakeVisible(*_presetPanel);
		}
	}

	juce::Component* getWrappedComponent() override
	{
		return _presetPanel.get();
	}

private:
	std::unique_ptr<nvs::gui::PresetPanel> _presetPanel;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetPanelItem)
};

void ShredVerbAudioProcessor::initialiseBuilder(foleys::MagicGUIBuilder& builder) {
	builder.registerJUCEFactories();
	builder.registerJUCELookAndFeels();
	
	builder.registerFactory("PresetPanel", *PresetPanelItem::factory);
	
	foleys::MagicGUIState& state = builder.getMagicState();
	
	state.addTrigger("randomize", [this]{
		randomizeParams();
	});
	state.addTrigger("randomize delays", [this]{
		randomizeDelays();
	});
	state.addTrigger("randomize qualia", [this]{
		randomizeQualia();
	});
	state.addTrigger("randomize shred", [this]{
		randomizeCharacter();
	});
	state.addTrigger("randomize allpass", [this]{
		randomizeAllpass();
	});
	
	state.createAndAddObject<nvs::service::PresetManager>("PresetManager", paramVT);
}
