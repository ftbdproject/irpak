// src/IRProcessor.cpp
#include "IRProcessor.hpp"
#include <cmath>

bool IRProcessor::processAudioFile(const std::string& inputFile,
                                 const std::string& outputFile,
                                 const std::string& password) {
    try {
        juce::File input(inputFile);
        juce::AudioFormatManager formatManager;
        formatManager.registerBasicFormats();

        std::unique_ptr<juce::AudioFormatReader> reader(
            formatManager.createReaderFor(input));

        if (!reader) {
            return false;
        }

        juce::AudioBuffer<float> buffer(
            reader->numChannels,
            static_cast<int>(reader->lengthInSamples)
        );

        reader->read(&buffer, 0, 
                    static_cast<int>(reader->lengthInSamples), 
                    0, true, true);

        std::vector<uint8_t> irData;
        if (!convertToIR(buffer, reader->sampleRate, irData)) {
            return false;
        }

        IRHeader header;
        header.numChannels = buffer.getNumChannels();
        header.numSamples = buffer.getNumSamples();
        header.sampleRate = reader->sampleRate;

        juce::File outFile(outputFile);
        juce::FileOutputStream output(outFile);
        
        if (!output.openedOk()) {
            return false;
        }

        output.write(&header, sizeof(IRHeader));
        output.write(irData.data(), irData.size());

        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error processing audio file: " << e.what() << std::endl;
        return false;
    }
}

bool IRProcessor::convertToIR(juce::AudioBuffer<float>& buffer,
                            double sampleRate,
                            std::vector<uint8_t>& output) {
    try {
        // Find maximum amplitude for normalization
        float maxAmp = 0.0f;
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
            const float* channelData = buffer.getReadPointer(channel);
            for (int i = 0; i < buffer.getNumSamples(); ++i) {
                maxAmp = std::max(maxAmp, std::abs(channelData[i]));
            }
        }

        // Normalize if necessary
        if (maxAmp > 0.0f && maxAmp != 1.0f) {
            float normalizeGain = 1.0f / maxAmp;
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
                float* channelData = buffer.getWritePointer(channel);
                for (int i = 0; i < buffer.getNumSamples(); ++i) {
                    channelData[i] *= normalizeGain;
                }
            }
        }

        // Calculate buffer size and prepare output
        size_t dataSize = buffer.getNumSamples() * 
                         buffer.getNumChannels() * 
                         sizeof(float);
        output.resize(dataSize);

        // Copy audio data
        memcpy(output.data(), 
               buffer.getReadPointer(0), 
               dataSize);

        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error converting to IR: " << e.what() << std::endl;
        return false;
    }
}
