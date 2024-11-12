// include/AudioPackager.hpp
#pragma once
#include "JuceHeader.hpp"
#include <vector>
#include <string>

class AudioPackager
{
public:
    struct PackageHeader
    {
        char signature[8] = {'A', 'P', 'K', 'G', '2', '0', '2', '4'};
        uint32_t version = 1;
        uint32_t numFiles = 0;
        uint32_t flags = 0;
        uint8_t salt[32];
        uint8_t iv[16];
        uint8_t hash[32];
    };

    struct FileEntry
    {
        std::string filename;
        uint64_t offset;
        uint32_t originalSize;
        uint32_t compressedSize;
        uint32_t crc32;
        uint8_t fileType;
    };

    static bool packFiles(const std::vector<std::string>& inputFiles,
                         const std::string& outputFile,
                         const std::string& password);

    static bool unpackFiles(const std::string& packageFile,
                          const std::string& outputDir,
                          const std::string& password);

private:
    static std::vector<uint8_t> compressData(const std::vector<uint8_t>& input);
    static std::vector<uint8_t> decompressData(const std::vector<uint8_t>& input, size_t originalSize);
    static uint32_t calculateCRC32(const std::vector<uint8_t>& data);
};