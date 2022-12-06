//
//  params.h
//  shredVerb
//
//  Created by Nicholas Solem on 12/5/22.
//  Copyright © 2022 nvssynthesis. All rights reserved.
//

//#ifndef params_h
//#define params_h
#pragma once
#include <array>
#include <map>


struct param_stuff{
    enum class params_e{
        drive,
        predelay,
        size,
        lowpass,
        
        tvap0_f_pi,
        tvap0_f_b,
        tvap1_f_pi,
        tvap1_f_b,
        
        tvap2_f_pi,
        tvap2_f_b,
        tvap3_f_pi,
        tvap3_f_b,
        
        dist1_inner,
        dist1_outer,
        dist2_inner,
        dist2_outer,
        
        decay,
        output_gain,
        drywet,
        
        count,
        
        interp_type,
        randomize
    };

    inline static const std::map<params_e, std::string> paramIDs =
    {
        {params_e::drive, "drive"},
        {params_e::predelay, "predelay"},
        {params_e::decay, "decay"},
        {params_e::size, "size"},
        {params_e::lowpass, "lop"},
        {params_e::drywet, "dryWet"},
        {params_e::tvap0_f_pi, "tvap0 freq"},
        {params_e::tvap0_f_b, "tvap0 bandwidth"},
        {params_e::tvap1_f_pi, "tvap1 freq"},
        {params_e::tvap1_f_b, "tvap1 bandwidth"},
        {params_e::tvap2_f_pi, "tvap2 freq"},
        {params_e::tvap2_f_b, "tvap2 bandwidth"},
        {params_e::tvap3_f_pi, "tvap3 freq"},
        {params_e::tvap3_f_b, "tvap3 bandwidth"},
        {params_e::dist1_inner, "inner distortion 1"},
        {params_e::dist1_outer, "outer distortion 1"},
        {params_e::dist2_inner, "inner distortion 2"},
        {params_e::dist2_outer, "outer distortion 2"},
        {params_e::interp_type, "interpolation type"},
        {params_e::randomize, "randomizzze"},
        {params_e::output_gain, "output gain"},

    };
    inline static const std::map<params_e, std::string> paramNames =
    {
        {params_e::drive, "Drive"},
        {params_e::predelay, "Predelay"},
        {params_e::decay, "Decay"},
        {params_e::size, "Spatial Size"},
        {params_e::lowpass, "Lowpass Cutoff"},
        {params_e::drywet, "Dry/Wet"},
        {params_e::tvap0_f_pi, "Allpass Base Frequency 0"},
        {params_e::tvap0_f_b, "Allpass Bandwidth 0"},
        {params_e::tvap1_f_pi, "Allpass Base Frequency 1"},
        {params_e::tvap1_f_b, "Allpass Bandwidth 1"},
        {params_e::tvap2_f_pi, "Allpass Base Frequency 2"},
        {params_e::tvap2_f_b, "Allpass Bandwidth 2"},
        {params_e::tvap3_f_pi, "Allpass Base Frequency 3"},
        {params_e::tvap3_f_b, "Allpass Bandwidth 3"},
        {params_e::dist1_inner, "Inner Distortion 1"},
        {params_e::dist1_outer, "Outer Distortion 1"},
        {params_e::dist2_inner, "Inner Distortion 2"},
        {params_e::dist2_outer, "Outer Distortion 2"},
        {params_e::interp_type, "Interpolation Type"},
        {params_e::randomize, "Randomize"},
        {params_e::output_gain, "Output Gain"}
    };
    inline static const float f_pi_min = 5.f;
    inline static const float f_pi_max = 22000.f;
    inline static const float f_b_min = 5.f;
    inline static const float f_b_max = 10000.f;
    
    inline static const std::map<params_e, std::array<float, 2>> paramRanges =
    {
        {params_e::drive,       {-24.f, 24.f}},
        {params_e::predelay,    {0.f, 1000.f}},
        {params_e::decay,       {0.f, 0.7071067}},
        {params_e::size,        {0.f, 1.f}},
        {params_e::lowpass,     {20.f, 22000.f}},
        {params_e::drywet,      {0.f, 1.f}},
        {params_e::tvap0_f_pi,  {f_pi_min, f_pi_max}},
        {params_e::tvap0_f_b,   {f_b_min, f_b_max}},
        {params_e::tvap1_f_pi,  {f_pi_min, f_pi_max}},
        {params_e::tvap1_f_b,   {f_b_min, f_b_max}},
        {params_e::tvap2_f_pi,  {f_pi_min, f_pi_max}},
        {params_e::tvap2_f_b,   {f_b_min, f_b_max}},
        {params_e::tvap3_f_pi,  {f_pi_min, f_pi_max}},
        {params_e::tvap3_f_b,   {f_b_min, f_b_max}},
        {params_e::dist1_inner, {0.f,1.f}},
        {params_e::dist1_outer, {0.f,1.f}},
        {params_e::dist2_inner, {0.f,1.f}},
        {params_e::dist2_outer, {0.f,1.f}},
        {params_e::interp_type, {0.f,1.f}},
        {params_e::randomize,   {0.f,1.f}},
        {params_e::output_gain, {-69.f, 24.f}}
    };
    inline static const std::map<params_e, float> paramDefaults =
    {
        {params_e::drive,       0.f},
        {params_e::predelay,    0.f},
        {params_e::decay,       0.1f},
        {params_e::size,        0.5f},
        {params_e::lowpass,     18000.f},
        {params_e::drywet,      1.f},
        {params_e::tvap0_f_pi,  200.f},
        {params_e::tvap0_f_b,   500.f},
        {params_e::tvap1_f_pi,  500.f},
        {params_e::tvap1_f_b,   1000.f},
        {params_e::tvap2_f_pi,  900.f},
        {params_e::tvap2_f_b,   10.f},
        {params_e::tvap3_f_pi,  5000.f},
        {params_e::tvap3_f_b,   200.f},
        {params_e::dist1_inner, 0.f},
        {params_e::dist1_outer, 0.f},
        {params_e::dist2_inner, 0.f},
        {params_e::dist2_outer, 0.f},
        {params_e::interp_type, 0.f},
        {params_e::randomize,   0.f},
        {params_e::output_gain, 0.f}
    };

    inline static const std::map<params_e, float> paramSkewFactorFromMidpoints =
    {
        {params_e::drive,       0.f},
        {params_e::predelay,    100.f},
        {params_e::decay,       0.5f},
        {params_e::size,        0.5f},
        {params_e::lowpass,     1000.f},
        {params_e::drywet,      0.5f},
        {params_e::tvap0_f_pi,  1000.f},
        {params_e::tvap0_f_b,   1000.f},
        {params_e::tvap1_f_pi,  1000.f},
        {params_e::tvap1_f_b,   1000.f},
        {params_e::tvap2_f_pi,  1000.f},
        {params_e::tvap2_f_b,   1000.f},
        {params_e::tvap3_f_pi,  1000.f},
        {params_e::tvap3_f_b,   1000.f},
        {params_e::dist1_inner, 0.3f},
        {params_e::dist1_outer, 0.3f},
        {params_e::dist2_inner, 0.3f},
        {params_e::dist2_outer, 0.3f},
        {params_e::interp_type, 0.3f},
        {params_e::randomize,   0.5f},
        {params_e::output_gain, 0.f}
        
    };
    inline static const std::map<params_e, int> paramNumDecimalPlacesToDisplay =
    {
        {params_e::drive,       2},
        {params_e::predelay,    2},
        {params_e::decay,       3},
        {params_e::size,        3},
        {params_e::lowpass,     1},
        {params_e::drywet,      3},
        {params_e::tvap0_f_pi,  1},
        {params_e::tvap0_f_b,   1},
        {params_e::tvap1_f_pi,  1},
        {params_e::tvap1_f_b,   1},
        {params_e::tvap2_f_pi,  1},
        {params_e::tvap2_f_b,   1},
        {params_e::tvap3_f_pi,  1},
        {params_e::tvap3_f_b,   1},
        {params_e::dist1_inner, 3},
        {params_e::dist1_outer, 3},
        {params_e::dist2_inner, 3},
        {params_e::dist2_outer, 3},
        {params_e::interp_type, 3},
        {params_e::randomize,   2},
        {params_e::output_gain, 2}
    };
    // to be defined/initialized in PluginEditor constructor
    static const size_t numParams = (size_t)(params_e::count);
    std::array<juce::Slider, numParams> paramSliders;
    std::array<juce::AudioProcessorValueTreeState::SliderAttachment*, numParams> paramSliderAttachmentPtrs;
    std::array<juce::Label, numParams> paramLabels;
};

//static const std::map<param_stuff::params_e, std::string> param_stuff::paramIDs =


//#endif /* params_h */
