// include/JuceHeader.hpp
#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_core/juce_core.h>

#define JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP 0
#define JUCE_STANDALONE_APPLICATION 1

namespace AudioUtils
{
    inline float dbToGain(float db) { return std::pow(10.0f, db * 0.05f); }
    inline float gainToDb(float gain) { return 20.0f * std::log10(gain); }
}