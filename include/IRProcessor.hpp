
// include/IRProcessor.hpp
#pragma once
#include "JuceHeader.hpp"
#include <vector>
#include <string>

class IRProcessor
{
public:
    struct IRHeader
    {
        char signature[4] = {'I', 'R', 'P', '\0'};
        uint32_t version = 1;
        uint32_t numChannels;
        uint32_t numSamples;
        double sampleRate;
    };

    static bool processAudioFile(const std::string& inputFile,
                               const std::string& outputFile,
                               const std::string& password);

    static bool convertToIR(juce::AudioBuffer<float>& buffer,
                           double sampleRate,
                           std::vector<uint8_t>& output);
};