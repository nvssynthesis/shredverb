# shredverb

ShredVerb is a reverberation device that shreds audio. Really, this algorithm is at its 
core a reverb–but it acts like a stereo distortion. This effect is accomplished not by
adding nonlinear elements such as clippers or waveshapers, but by frequency-modulating
the allpass filters inherent to the reverb. 

Dependencies:
JUCE (mine is v7.0.2)
nvs_libraries, my own DSP template library. You should clone the submodule to use this.
