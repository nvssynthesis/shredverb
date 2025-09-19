/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
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

	presetManager = std::make_unique<nvs::service::PresetManager>(paramVT);
	// set GUI
    // this is how i was loading default, but docs actually say to do this as return... in createEditor
    magicState.setGuiValueTree (BinaryData::_19_9_25_xml, BinaryData::_19_9_25_xmlSize);
	
    // pointers are copied to items declared in object so they don't go out of scope
    driveParam    = paramVT.getRawParameterValue (param_stuff::paramIDs.at(param_stuff::params_e::drive));
    predelayParam = paramVT.getRawParameterValue (param_stuff::paramIDs.at(param_stuff::params_e::predelay));

    decayParam   = paramVT.getRawParameterValue (param_stuff::paramIDs.at(param_stuff::params_e::decay));
    sizeParam = paramVT.getRawParameterValue (param_stuff::paramIDs.at(param_stuff::params_e::size));
    lopParam = paramVT.getRawParameterValue (param_stuff::paramIDs.at(param_stuff::params_e::lowpass));
    hipParam = paramVT.getRawParameterValue (param_stuff::paramIDs.at(param_stuff::params_e::highpass));
    dryWetParam = paramVT.getRawParameterValue (param_stuff::paramIDs.at(param_stuff::params_e::drywet));

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

	apdGparams[0] = paramVT.getRawParameterValue (param_stuff::paramIDs.at(param_stuff::params_e::g0));
	apdGparams[1] = paramVT.getRawParameterValue (param_stuff::paramIDs.at(param_stuff::params_e::g1));
	apdGparams[2] = paramVT.getRawParameterValue (param_stuff::paramIDs.at(param_stuff::params_e::g2));
	apdGparams[3] = paramVT.getRawParameterValue (param_stuff::paramIDs.at(param_stuff::params_e::g3));
	
    outputGainParam = paramVT.getRawParameterValue (param_stuff::paramIDs.at(param_stuff::params_e::output_gain));
    interpParam = paramVT.getRawParameterValue (param_stuff::paramIDs.at(param_stuff::params_e::interp_type));
    randomizeParam = paramVT.getRawParameterValue (param_stuff::paramIDs.at(param_stuff::params_e::randomize));
		
	for (auto &pd : preDelays){
		pd.setInterpolation(nvs::delays::interp_e::floor);
		pd.setDelayTimeMS(param_stuff::paramDefaults.at(param_stuff::params_e::predelay));
	}
	
	D_times_ranged[0] = param_stuff::paramDefaults.at(param_stuff::params_e::time0);
	D_times_ranged[1] = param_stuff::paramDefaults.at(param_stuff::params_e::time1);
	D_times_ranged[2] = param_stuff::paramDefaults.at(param_stuff::params_e::time2);
	D_times_ranged[3] = param_stuff::paramDefaults.at(param_stuff::params_e::time3);
	
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
template<size_t N>
void ShredVerbAudioProcessor::randomizeParams(std::array<param_stuff::params_e, N> params){
	for (auto p : params){
		float val = rando.nextFloat();
		
		auto param = paramVT.getParameter(param_stuff::paramIDs.at(p));
		
		param->beginChangeGesture();
		param->setValueNotifyingHost(juce::jlimit(0.f, 1.f, val));
		param->endChangeGesture();
	}
}

void ShredVerbAudioProcessor::randomizeDelays(){
	std::array<param_stuff::params_e, 8> dParams = {
		param_stuff::params_e::time0,
		param_stuff::params_e::time1,
		param_stuff::params_e::time2,
		param_stuff::params_e::time3,
		param_stuff::params_e::g0,
		param_stuff::params_e::g1,
		param_stuff::params_e::g2,
		param_stuff::params_e::g3
	};
	randomizeParams(dParams);
}
void ShredVerbAudioProcessor::randomizeQualia(){
	std::array<param_stuff::params_e, 5> dParams = {
		param_stuff::params_e::predelay,
		param_stuff::params_e::size,
		param_stuff::params_e::highpass,
		param_stuff::params_e::lowpass,
		param_stuff::params_e::decay
	};
	randomizeParams(dParams);
}
void ShredVerbAudioProcessor::randomizeCharacter(){
	std::array<param_stuff::params_e, 5> dParams = {
		param_stuff::params_e::drive,
		param_stuff::params_e::dist1_inner,
		param_stuff::params_e::dist1_outer,
		param_stuff::params_e::dist2_inner,
		param_stuff::params_e::dist2_outer
	};
	randomizeParams(dParams);
}
void ShredVerbAudioProcessor::randomizeAllpass(){
	std::array<param_stuff::params_e, 8> dParams = {
		param_stuff::params_e::tvap0_f_b,
		param_stuff::params_e::tvap1_f_b,
		param_stuff::params_e::tvap2_f_b,
		param_stuff::params_e::tvap3_f_b,
		param_stuff::params_e::tvap0_f_pi,
		param_stuff::params_e::tvap1_f_pi,
		param_stuff::params_e::tvap2_f_pi,
		param_stuff::params_e::tvap3_f_pi
	};
	randomizeParams(dParams);
}
void ShredVerbAudioProcessor::randomizeParams(){
	randomizeDelays();
	randomizeQualia();
	randomizeCharacter();
	randomizeAllpass();
}
nvs::service::PresetManager& ShredVerbAudioProcessor::getPresetManager() const {
	return *presetManager;
}

class PresetPanelItem : public foleys::GuiItem
{
public:
	FOLEYS_DECLARE_GUI_FACTORY (PresetPanelItem)

	PresetPanelItem (foleys::MagicGUIBuilder& builder, const juce::ValueTree& node) : foleys::GuiItem (builder, node)
	{
		// Create the colour names to have them configurable
//		setColourTranslation ({
//			{"preset-panel-background", nvs::gui::PresetPanel::backgroundColourId},
//			{"preset-panel-draw", nvs::gui::PresetPanel::drawColourId},
//			{"preset-panel-fill", nvs::gui::PresetPanel::fillColourId} });

		addAndMakeVisible (presetPanel);
	}

	std::vector<foleys::SettableProperty> getSettableProperties() const override
	{
//		std::vector<foleys::SettableProperty> newProperties;
//		newProperties.push_back ({ configNode, "factor", foleys::SettableProperty::Number, 1.0f, {} });
//		return newProperties;
		return {};
	}

	// Override update() to set the values to your custom component
	void update() override
	{
//		auto* presetPanel = getMagicState().getObjectWithType<nvs::gui::PresetPanel>("Preset Panel");
//		presetPanel.assignPresetManagerAndInit(service::PresetManager *)
	}

	juce::Component* getWrappedComponent() override
	{
		return &presetPanel;
	}

private:
	nvs::gui::PresetPanel presetPanel;

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
//	
	presetPanel = state.createAndAddObject<nvs::gui::PresetPanel>("Preset Panel");
	presetPanel->assignPresetManagerAndInit(presetManager.get());
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
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}
bool ShredVerbAudioProcessor::isMidiEffect() const {
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
    
	maxPreDelTimeMS = float((preDelays[0].getDelaySize() - 10) * 1000) / (float)sampleRate; // no more than buffer length - 10

    maxDelTimeMS = float((D[0].getDelaySize() - 10) * 1000) / ((float)sampleRate * D[0].getLargestRatio()); // no more than buffer length - 10
    minDelTimeMS = float(3 * 1000) / (float)sampleRate; // no less than 3 samples
    
	//feed forward pair
	for (auto &pd : preDelays){
		pd.clear();
		pd.setSampleRate(sampleRate);
//		pd.set
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
	
	for (auto i = nIn; i < nOut; ++i){
		buffer.clear (i, 0, numSamps);
	}
	
	[[deprecated]]	// any function using this param during processBlock should be deprecated
    double const _oneOverBlockSize = 1 / (double)numSamps;
	
	std::array<float const* const, 2> inBuff {
		buffer.getReadPointer(0),
		(getTotalNumInputChannels() > 1) ? buffer.getReadPointer(1) : buffer.getReadPointer(0)
	};

	std::array<float* const, 2> outBuff {
		buffer.getWritePointer(0),
		(getTotalNumOutputChannels() > 1) ? buffer.getWritePointer(1) : buffer.getWritePointer(0)
	};
    
	float const inDrive = juce::Decibels::decibelsToGain<float>(driveParam->load());
//	float const outDrive = 1.f / inDrive;
	
	float const _predel = nvs::memoryless::clamp(
					 static_cast<float>(predelayParam->load()), minDelTimeMS, maxPreDelTimeMS);

    float const _decay = decayParam->load();
	auto const _g = _decay * 0.707106781186548;
    float _size = sizeParam->load();
    _size *= _size;		// breaking const is a sign that the param itself should be shaped
	
    float const _hip = hipParam->load();
    float const _lop = lopParam->load();
	
    std::array<float const, 4> const times {
		time0Param->load(),
		time1Param->load(),
		time2Param->load(),
		time3Param->load()
	};
	
	std::array<float const, 4> const del_gs {
		apdGparams[0]->load(),
		apdGparams[1]->load(),
		apdGparams[2]->load(),
		apdGparams[3]->load()
	};
	
    float const _dryWet = dryWetParam->load() / 100.f;
	float const outGain = juce::Decibels::decibelsToGain<float>(outputGainParam->load());

	std::array<float, 4> _ap_f_pi {
		allpassFrequencyPiParam0->load(),
		allpassFrequencyPiParam1->load(),
		allpassFrequencyPiParam2->load(),
		allpassFrequencyPiParam3->load()
	};
	
	std::array<float, 4> _ap_fb {
		allpassBandwidthParam0->load(),
		allpassBandwidthParam1->load(),
		allpassBandwidthParam2->load(),
		allpassBandwidthParam3->load()
	};
	
	/* these cannot be std::arrays until i redo  nvs::memoryless::metaparamA */
//    float inner_f_pi[2], outer_f_pi[2];
//    float inner_f_b[2], outer_f_b[2];

    // ACTUALLY RESHAPE FROM 2 METAPARAMS INTO 4 PARAMS:
//    auto dist1Iner = paramVT.getRawParameterValue(param_stuff::paramIDs.at(param_stuff::params_e::dist1_inner));
//    auto dist1Outr = paramVT.getRawParameterValue(param_stuff::paramIDs.at(param_stuff::params_e::dist1_outer));
//    auto dist2Iner = paramVT.getRawParameterValue(param_stuff::paramIDs.at(param_stuff::params_e::dist2_inner));
//    auto dist2Outr = paramVT.getRawParameterValue(param_stuff::paramIDs.at(param_stuff::params_e::dist2_outer));
    
//    nvs::memoryless::metaparamA(float(*dist1Iner), inner_f_pi);
//    nvs::memoryless::metaparamA(float(*dist1Outr), outer_f_pi);
//    nvs::memoryless::metaparamA(float(*dist2Iner), inner_f_b);
//    nvs::memoryless::metaparamA(float(*dist2Outr), outer_f_b);
    
	float dist1inner = dist1inerParam->load();
	float dist2inner = dist2inerParam->load();
	float dist1outer = dist1outrParam->load();
	float dist2outer = dist2outrParam->load();
    //=============================================================================
	
    for (int i = 0; i < D_IJ; i++) {
        D_times_ranged[i] = times[i];
    }
	
    std::array<float, D_IJ> current_Dtime;
    for (int i = 0; i < D_IJ; i++) {
        current_Dtime[i] = D_times_ranged[i];
        current_Dtime[i] *= _size;
    }
	
	for (int i = 0; i < D_IJ; ++i){
		float bw = _ap_fb[i];
		tvap[i].setCutoffTarget(_ap_f_pi[i]);
		tvap[i].setResonanceTarget(bw);
		
		fm_bp[i].setCutoffTarget(_ap_f_pi[i]);
		
		bw = nvs::memoryless::clamp_low(bw, 0.2f);
		float reso = _ap_f_pi[i] / bw;
		fm_bp[i].setResonanceTarget(reso);
	}
	
	for (auto &filt : butters){
		filt.setCutoffTarget(_lop);
	}
    for (auto &filt : hp6dB)  {
        filt.setCutoffTarget(_hip);
    }
    
    for (int samp = 0; samp < numSamps; samp++)
    {
        for (auto &pd : preDelays){
            pd.updateDelayTimeMS(_predel, (float)_oneOverBlockSize);
        }
        for (int i = 0; i < D_IJ; i++) {
            D[i].updateDelayTimeMS(nvs::memoryless::clamp
										(current_Dtime[i] * timeScaling, minDelTimeMS, maxDelTimeMS),
									(float)_oneOverBlockSize);
			D[i].update_g(del_gs[i] * _decay, (float)_oneOverBlockSize);
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

        std::array<float, D_IJ> tmp {0.f, 0.f, 0.f, 0.f};
        
		/* G:
			{0.f,  1.f,  1.f,  0.f},
			{-1.f, 0.f,  0.f, -1.f},
			{1.f,  0.f,  0.f, -1.f},
			{0.f,  1.f, -1.f,  0.f}
		*/
        for (int i = 0; i < D_IJ; i++) {
            for (int j = 0; j < D_IJ; j++) {
                tmp[i] += G[i][j] * Y[j] * (_g);
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
		std::array<float, 4> intrnlWcModSig = {
			nvs::memoryless::unboundSat2(fm_bp[0](tmp[0]) * 1000000000.f) * 100.f * inDrive,
			nvs::memoryless::unboundSat2(fm_bp[1](tmp[1]) * 1000000000.f) * 100.f * inDrive,
			nvs::memoryless::unboundSat2(fm_bp[2](tmp[2]) * 1000000000.f) * 100.f * inDrive,
			nvs::memoryless::unboundSat2(fm_bp[3](tmp[3]) * 1000000000.f) * 100.f * inDrive
		};
#pragma message("need to smooth these cutoff/reso if doing it this way")
/*L	INTERNAL*/ tmp[0] = tvap[0](tmp[0], _ap_f_pi[0] + dist1inner*intrnlWcModSig[0], _ap_fb[0]);
/*L DIRECT*/ tmp[1] = tvap[1](tmp[1], _ap_f_pi[1] + dist1outer*intrnlWcModSig[1], _ap_fb[1]);
/*R DIRECT*/ tmp[2] = tvap[2](tmp[2], _ap_f_pi[2] + dist2outer*intrnlWcModSig[2], _ap_fb[2]);
/*R INTERNAL*/ tmp[3] = tvap[3](tmp[3], _ap_f_pi[3] + dist2inner*intrnlWcModSig[3], _ap_fb[3]);
#endif
        Y[0] = D[0](tmp[0]);
        Y[1] = D[1](tmp[1]);
        Y[2] = D[2](tmp[2]);
        Y[3] = D[3](tmp[3]);

		std::array<float, 2> wet {
			Y[1] * outGain,// * outDrive,
			Y[2] * outGain // * outDrive
		};

#if PROTECT_OUTPUT
		for (auto &w : wet){
			w = nvs::memoryless::clamp(w, -2.5f, 2.5f);
		}
#endif
		std::array<float, 2> finalOut {
			wet[0] * _dryWet + inSamps[0] * (1 - _dryWet),
			wet[1] * _dryWet + inSamps[1] * (1 - _dryWet)
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
#if WORKING_ON_PRESETS
	std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (paramVT.state.getType()))
            paramVT.replaceState (juce::ValueTree::fromXml (*xmlState));
#endif
}
void ShredVerbAudioProcessor::addReverbParameters (juce::AudioProcessorValueTreeState::ParameterLayout& layout){
	auto predel  = param_stuff::bast::createParam(param_stuff::params_e::predelay);
    auto size = param_stuff::bast::createParam(param_stuff::params_e::size);
    auto decay = param_stuff::bast::createParam(param_stuff::params_e::decay);
    auto lowpass = param_stuff::bast::createParam(param_stuff::params_e::lowpass);
    auto hipass = param_stuff::bast::createParam(param_stuff::params_e::highpass);

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
    std::unique_ptr<juce::AudioParameterFloat> time0  = param_stuff::bast::createParam(param_stuff::params_e::time0);
    std::unique_ptr<juce::AudioParameterFloat> time1  = param_stuff::bast::createParam(param_stuff::params_e::time1);
    std::unique_ptr<juce::AudioParameterFloat> time2  = param_stuff::bast::createParam(param_stuff::params_e::time2);
    std::unique_ptr<juce::AudioParameterFloat> time3  = param_stuff::bast::createParam(param_stuff::params_e::time3);
        
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
		param_stuff::bast::createParam(param_stuff::params_e::g0),
		param_stuff::bast::createParam(param_stuff::params_e::g1),
		param_stuff::bast::createParam(param_stuff::params_e::g2),
		param_stuff::bast::createParam(param_stuff::params_e::g3)
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
void ShredVerbAudioProcessor::addAllpassParameters (juce::AudioProcessorValueTreeState::ParameterLayout& layout){

    auto ap0f = param_stuff::bast::createParam(param_stuff::params_e::tvap0_f_pi);
    auto ap0b = param_stuff::bast::createParam(param_stuff::params_e::tvap0_f_b);
    auto ap1f = param_stuff::bast::createParam(param_stuff::params_e::tvap1_f_pi);
    auto ap1b = param_stuff::bast::createParam(param_stuff::params_e::tvap1_f_b);
    auto ap2f = param_stuff::bast::createParam(param_stuff::params_e::tvap2_f_pi);
    auto ap2b = param_stuff::bast::createParam(param_stuff::params_e::tvap2_f_b);
    auto ap3f = param_stuff::bast::createParam(param_stuff::params_e::tvap3_f_pi);
    auto ap3b = param_stuff::bast::createParam(param_stuff::params_e::tvap3_f_b);

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
    
    auto drive  = param_stuff::bast::createParam(param_stuff::params_e::drive);
    auto inner1 = param_stuff::bast::createParam(param_stuff::params_e::dist1_inner);
    auto outer1 = param_stuff::bast::createParam(param_stuff::params_e::dist1_outer);
    auto inner2 = param_stuff::bast::createParam(param_stuff::params_e::dist2_inner);
    auto outer2 = param_stuff::bast::createParam(param_stuff::params_e::dist2_outer);
    
    float centrVal = 0.2f;
    inner1->range.setSkewForCentre(centrVal);
    inner2->range.setSkewForCentre(centrVal);
    outer1->range.setSkewForCentre(centrVal);
    outer2->range.setSkewForCentre(centrVal);
//    inner1->range.interval = distInterval / 25.f;
//    inner2->range.interval = distInterval / 25.f;
//    outer1->range.interval = distInterval / 25.f;
//    outer2->range.interval = distInterval / 25.f;

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
    auto drywet = param_stuff::bast::createParam(param_stuff::params_e::drywet);
    auto output = param_stuff::bast::createParam(param_stuff::params_e::output_gain);

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

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ShredVerbAudioProcessor();
}
