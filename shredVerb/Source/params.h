//
//  params.h
//  shredVerb
//
//  Created by Nicholas Solem on 12/5/22.
//  Copyright © 2022 nvssynthesis. All rights reserved.
//

#pragma once
#include <array>
#include <map>

#define USING_JUCE 1

#if USING_JUCE
#include <JuceHeader.h>
#endif

namespace param_stuff{
    enum class params_e{
        predelay,
        size,
        highpass,
        lowpass,
        decay,
        
        time0,
        time1,
        time2,
        time3,
		
		g0,
		g1,
		g2,
		g3,

        drive,
        dist1_outer,
        dist1_inner,
        dist2_inner,
        dist2_outer,
        
        tvap0_f_pi,
        tvap0_f_b,
        tvap1_f_pi,
        tvap1_f_b,
        tvap2_f_pi,
        tvap2_f_b,
        tvap3_f_pi,
        tvap3_f_b,

        drywet,
        output_gain,
        
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
        {params_e::highpass, "hip"},
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
        {params_e::time0, "delay time 0"},
        {params_e::time1, "delay time 1"},
        {params_e::time2, "delay time 2"},
        {params_e::time3, "delay time 3"},
		{params_e::g0,		"g0"},
		{params_e::g1,		"g1"},
		{params_e::g2,		"g2"},
		{params_e::g3,		"g3"}
    };
    inline static const std::map<params_e, std::string> paramNames =
    {
        {params_e::drive, "Drive"},
        {params_e::predelay, "Predelay"},
        {params_e::decay, "Decay"},
        {params_e::size, "Spatial Size"},
        {params_e::lowpass, "Lowpass Cutoff"},
        {params_e::highpass, "Highpass Cutoff"},
        {params_e::drywet, "Dry/Wet"},
        {params_e::tvap0_f_pi, "Base Frequency 0"},
        {params_e::tvap0_f_b, "Bandwidth 0"},
        {params_e::tvap1_f_pi, "Base Frequency 1"},
        {params_e::tvap1_f_b, "Bandwidth 1"},
        {params_e::tvap2_f_pi, "Base Frequency 2"},
        {params_e::tvap2_f_b, "Bandwidth 2"},
        {params_e::tvap3_f_pi, "Base Frequency 3"},
        {params_e::tvap3_f_b, "Bandwidth 3"},
        {params_e::dist1_inner, "Inner Distortion 1"},
        {params_e::dist1_outer, "Outer Distortion 1"},
        {params_e::dist2_inner, "Inner Distortion 2"},
        {params_e::dist2_outer, "Outer Distortion 2"},
        {params_e::interp_type, "Interpolation Type"},
        {params_e::randomize, "Randomize"},
        {params_e::output_gain, "Output Gain"},
        {params_e::time0, "Delay 0"},
        {params_e::time1, "Delay 1"},
        {params_e::time2, "Delay 2"},
        {params_e::time3, "Delay 3"},
		{params_e::g0,		"g0"},
		{params_e::g1,		"g1"},
		{params_e::g2,		"g2"},
		{params_e::g3,		"g3"}
    };
	inline static const std::array<float, 3> f_pi_rng {1.f, 22000.f, 1000.f};

	inline static const std::array<float, 3> f_b_rng {0.1f, 10000.f, 500.f};

// min, max, knob center position
    inline static const std::map<params_e, std::array<float, 3>> paramRanges =
    {
        {params_e::drive,       {-60.f, 	60.f, 	0.f}},
        {params_e::predelay,    {0.f, 		500.f, 	100.f}},
        {params_e::decay,       {0.f, 		1.f, 	0.75f}},
        {params_e::size,        {0.001f, 	1.f, 	0.5f}},
        {params_e::lowpass,     {200.f, 	22000.f,1000.f}},
        {params_e::highpass,    {1.f, 		300.f, 	40.f}},
        {params_e::drywet,      {0.f, 		100.f, 	70.7f}},
        {params_e::tvap0_f_pi,  f_pi_rng},
        {params_e::tvap0_f_b,   f_b_rng},
        {params_e::tvap1_f_pi,  f_pi_rng},
        {params_e::tvap1_f_b,   f_b_rng},
        {params_e::tvap2_f_pi,  f_pi_rng},
        {params_e::tvap2_f_b,   f_b_rng},
        {params_e::tvap3_f_pi,  f_pi_rng},
        {params_e::tvap3_f_b,   f_b_rng},
        {params_e::dist1_inner, {0.f,		1.f, 	0.02f}},
        {params_e::dist1_outer, {0.f,		1.f, 	0.02f}},
        {params_e::dist2_inner, {0.f,		1.f, 	0.02f}},
        {params_e::dist2_outer, {0.f,		1.f, 	0.02f}},
        {params_e::output_gain, {-69.f, 	24.f,	std::midpoint(-69.f, 24.f)}},
        {params_e::time0,       {0.01f, 	1.f,	0.5f}},
        {params_e::time1,       {0.01f, 	1.f,	0.5f}},
        {params_e::time2,       {0.01f, 	1.f,	0.5f}},
        {params_e::time3,       {0.01f, 	1.f,	0.5f}},
		{params_e::g0,			{-0.999f, 	0.999f,	0.f}},
		{params_e::g1,		  	{-0.999f, 	0.999f,	0.f}},
		{params_e::g2,			{-0.999f, 	0.999f,	0.f}},
		{params_e::g3,			{-0.999f, 	0.999f,	0.f}}
    };
    inline static const std::map<params_e, float> paramDefaults =
    {
        {params_e::drive,       0.f},
        {params_e::predelay,    0.f},
        {params_e::decay,       0.75f},
        {params_e::size,        0.5f},
        {params_e::lowpass,     12000.f},
        {params_e::highpass,    20.f},
        {params_e::tvap0_f_pi,  500.f},
        {params_e::tvap0_f_b,   500.f},
        {params_e::tvap1_f_pi,  500.f},
        {params_e::tvap1_f_b,   500.f},
        {params_e::tvap2_f_pi,  500.f},
        {params_e::tvap2_f_b,   500.f},
        {params_e::tvap3_f_pi,  500.f},
        {params_e::tvap3_f_b,   500.f},
        {params_e::dist1_inner, 0.f},
        {params_e::dist1_outer, 0.f},
        {params_e::dist2_inner, 0.f},
        {params_e::dist2_outer, 0.f},
        {params_e::time0,       0.411f},	// inner 1
        {params_e::time1,       0.5f},		// outer 1
        {params_e::time2,       0.55f},		// outer 2
        {params_e::time3,       0.653f},	// inner 2
		
		{params_e::g0,			0.5f},		// inner 1
		{params_e::g1,		  	0.7f},		// outer 1
		{params_e::g2,			0.3f},		// outer 2
		{params_e::g3,			-0.8f},		// inner 2
		{params_e::drywet,      100.f},
		{params_e::output_gain, -10.f}
    };

    inline static const std::map<params_e, float> paramSkewFactorFromMidpoints =
    {
        {params_e::drive,       0.f},
        {params_e::predelay,    100.f},
        {params_e::decay,       0.75f},
        {params_e::size,        0.5f},
        {params_e::lowpass,     1000.f},
        {params_e::highpass,    1000.f},
        {params_e::drywet,      50.f},
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
        {params_e::output_gain, 0.f},
        {params_e::time0,       0.5f},
        {params_e::time1,       0.5f},
        {params_e::time2,       0.5f},
        {params_e::time3,       0.5f},
		{params_e::g0,			0.f},
		{params_e::g1,		  	0.f},
		{params_e::g2,			0.f},
		{params_e::g3,			0.f}
    };
    inline static const std::map<params_e, int> paramNumDecimalPlacesToDisplay =
    {
        {params_e::drive,       2},
        {params_e::predelay,    2},
        {params_e::decay,       3},
        {params_e::size,        3},
        {params_e::lowpass,     1},
        {params_e::highpass,    1},
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
        {params_e::output_gain, 2},
        {params_e::time0,       2},
        {params_e::time1,       2},
        {params_e::time2,       2},
        {params_e::time3,       2},
		{params_e::g0,			3},
		{params_e::g1,		  	3},
		{params_e::g2,			3},
		{params_e::g3,			3}
    };
	
	
#if USING_JUCE
	namespace bast{	// from https://github.com/Mrugalla/ParametersExample
		using String = juce::String;
		
		using ValToStr = std::function<String(float, int)>;
		using StrToVal = std::function<float(const String&)>;
		using APF = juce::AudioParameterFloat;
		using APP = juce::AudioProcessorParameter;
		using APPCategory = APP::Category;
		using APF = juce::AudioParameterFloat;
		using RangeF = juce::NormalisableRange<float>;
	
		enum class Unit
		{
			Db,
			Hz,
			ms,
			Percent,
			unitless,
			NumUnits
		};
		inline String toString(Unit unit)
		{
			switch (unit)
			{
			case Unit::Db: return "dB";
			case Unit::Hz: return "hz";
			case Unit::ms: return "ms";
			case Unit::Percent: return "%";
			default: return "Unknown";
			}
		}
		namespace range
		{
			inline RangeF biased(float start, float end, float bias) noexcept
			{
				// https://www.desmos.com/calculator/ps8q8gftcr
				const auto a = bias * .5f + .5f;
				const auto a2 = 2.f * a;
				const auto aM = 1.f - a;

				const auto r = end - start;
				const auto aR = r * a;
				if (bias != 0.f)
					return
				{
						start, end,
						[a2, aM, aR](float min, float, float x)
						{
							const auto denom = aM - x + a2 * x;
							if (denom == 0.f)
								return min;
							return min + aR * x / denom;
						},
						[a2, aM, aR](float min, float, float x)
						{
							const auto denom = a2 * min + aR - a2 * x - min + x;
							if (denom == 0.f)
								return 0.f;
							auto val = aM * (x - min) / denom;
							return val > 1.f ? 1.f : val;
						},
						[](float min, float max, float x)
						{
							return x < min ? min : x > max ? max : x;
						}
				};
				else return { start, end };
			}

			inline RangeF stepped(float start, float end, float steps = 1.f) noexcept
			{
				return { start, end, steps };
			}

			inline RangeF toggle() noexcept
			{
				return stepped(0.f, 1.f);
			}

			inline RangeF lin(float start, float end) noexcept
			{
				const auto range = end - start;

				return
				{
					start,
					end,
					[range](float min, float, float normalized)
					{
						return min + normalized * range;
					},
					[inv = 1.f / range](float min, float, float denormalized)
					{
						return (denormalized - min) * inv;
					},
					[](float min, float max, float x)
					{
						return juce::jlimit(min, max, x);
					}
				};
			}

			inline RangeF withCentre(float start, float end, float centre) noexcept
			{
				const auto r = end - start;
				const auto v = (centre - start) / r;

				return biased(start, end, 2.f * v - 1.f);
			}
		}
		namespace valToStr
		{
			inline ValToStr db()
			{
				return [](float val, int)
				{
					return String(val, 2) + " dB";
				};
			}
			
			inline ValToStr hz()
			{
				return [](float val, int)
				{
					if (val < 100.f)
						return String(val, 2) + " hz";
					else if (val < 1000.f)
						return String(val, 1) + " hz";
					else if (val < 10000.f)
					{
						auto k = val / 1000.f;
						return String(k, 2) + " khz";
					}
					else
					{
						auto k = val / 1000.f;
						return String(k, 1) + " khz";
					}
					return String();
				};
			}
			inline ValToStr ms()
			{
				return [](float val, int)
				{
					return String(val, 1) + " ms";
				};
			}
			inline ValToStr percent()
			{
				return [](float val, int)
				{
					return String(val, 1) + " %";
				};
			}
			inline ValToStr noop()
			{
				return [](float val, int)
				{
					return String(val, 3);
				};
			}
		}	// namespace valToStr
		
		namespace strToVal
		{
			 StrToVal db()
			{
				return [](const String& str)
				{
					return str.removeCharacters(toString(Unit::Db)).getFloatValue();
				};
			}
			
			inline StrToVal hz()
			{
				return [](const String& str)
				{
					auto s = str.removeCharacters(toString(Unit::Hz));
					if (s.endsWith("k"))
					{
						s = s.dropLastCharacters(1);
						return s.getFloatValue() * 1000.f;
					}
					return s.getFloatValue();
				};
			}
		
			inline StrToVal ms()
			{
				return [](const String& str)
				{
					return str.removeCharacters(toString(Unit::ms)).getFloatValue();
				};
			}
			inline StrToVal percent()
			{
				return [](const String& str)
				{
					return str.removeCharacters(toString(Unit::Percent)).getFloatValue();
				};
			}
			inline StrToVal noop(){
				return [](const String& str){
					return str.getFloatValue();
				};
			}
		}	// namespace strToVal
		using namespace param_stuff;

		inline static const std::map<param_stuff::params_e, Unit> paramStrToUnit
		{
			{params_e::predelay,    Unit::unitless},
			{params_e::size,    	Unit::unitless},
			{params_e::lowpass,     Unit::Hz},
			{params_e::highpass,    Unit::Hz},
			{params_e::decay,    	Unit::unitless},
			
			{params_e::time0,       Unit::unitless},
			{params_e::time1,       Unit::unitless},
			{params_e::time2,       Unit::unitless},
			{params_e::time3,       Unit::unitless},
			
			{params_e::g0,    		Unit::unitless},
			{params_e::g1,    		Unit::unitless},
			{params_e::g2,    		Unit::unitless},
			{params_e::g3,    		Unit::unitless},
			
			{params_e::drive,       Unit::Db},
			{params_e::dist1_outer, Unit::unitless},
			{params_e::dist1_inner, Unit::unitless},
			{params_e::dist2_inner, Unit::unitless},
			{params_e::dist2_outer, Unit::unitless},

			{params_e::tvap0_f_pi,  Unit::Hz},
			{params_e::tvap0_f_b,   Unit::Hz},
			{params_e::tvap1_f_pi,  Unit::Hz},
			{params_e::tvap1_f_b,   Unit::Hz},
			{params_e::tvap2_f_pi,  Unit::Hz},
			{params_e::tvap2_f_b,   Unit::Hz},
			{params_e::tvap3_f_pi,  Unit::Hz},
			{params_e::tvap3_f_b,   Unit::Hz},
			
			{params_e::drywet,		Unit::Percent},
			{params_e::output_gain, Unit::Db}
		};
	
	std::unique_ptr<APF> createParam(params_e param)
		{
			ValToStr valToStrFunc;
			StrToVal strToValFunc;

			const auto name = paramNames.at(param);
			const auto ID = paramIDs.at(param);

			const auto minmaxcentre = param_stuff::paramRanges.at(param);
			juce::NormalisableRange<float> range = range::withCentre(minmaxcentre[0], minmaxcentre[1], minmaxcentre[2]);

			switch (paramStrToUnit.at(param))
			{
				case Unit::Db:
					valToStrFunc = valToStr::db();
					strToValFunc = strToVal::db();
					break;
				case Unit::Hz:
					valToStrFunc = valToStr::hz();
					strToValFunc = strToVal::hz();
					break;
				case Unit::ms:
					valToStrFunc = valToStr::ms();
					strToValFunc = strToVal::ms();
					break;
				case Unit::Percent:
					valToStrFunc = valToStr::percent();
					strToValFunc = strToVal::percent();
					break;
				case Unit::unitless:
					valToStrFunc = valToStr::noop();
					strToValFunc = strToVal::noop();
					break;
			}
		
			return (std::make_unique<APF>
			(
				juce::ParameterID {ID, 1},
				name,
				range,
				paramDefaults.at(param),
				toString(paramStrToUnit.at(param)),
				APPCategory::genericParameter,
				valToStrFunc,
				strToValFunc
			));
		}
	}// namespace bast

#endif
    // to be defined/initialized in PluginEditor constructor
    static const size_t numParams = (size_t)(params_e::count);
//    std::array<juce::Slider, numParams> paramSliders;
//    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, numParams> paramSliderAttachmentPtrs;
//    std::array<juce::Label, numParams> paramLabels;
};

//static const std::map<param_stuff::params_e, std::string> param_stuff::paramIDs =


