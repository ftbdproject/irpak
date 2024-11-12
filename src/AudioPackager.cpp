// src/AudioPackager.cpp
#include "AudioPackager.hpp"
#include "Encryption.hpp"
#include <zlib.h>

std::vector<uint8_t> AudioPackager::compressData(const std::vector<uint8_t>& input) {
    std::vector<uint8_t> output;
    output.resize(compressBound(input.size()));

    z_stream stream = {};
    deflateInit(&stream, Z_BEST_COMPRESSION);

    stream.next_in = const_cast<Bytef*>(input.data());
    stream.avail_in = static_cast<uInt>(input.size());
    stream.next_out = output.data();
    stream.avail_out = static_cast<uInt>(output.size());

    deflate(&stream, Z_FINISH);
    deflateEnd(&stream);

    output.resize(stream.total_out);
    return output;
}

std::vector<uint8_t> AudioPackager::decompressData(const std::vector<uint8_t>& input, 
                                                  size_t originalSize) {
    std::vector<uint8_t> output(originalSize);
    
    z_stream stream = {};
    inflateInit(&stream);

    stream.next_in = const_cast<Bytef*>(input.data());
    stream.avail_in = static_cast<uInt>(input.size());
    stream.next_out = output.data();
    stream.avail_out = static_cast<uInt>(output.size());

    inflate(&stream, Z_FINISH);
    inflateEnd(&stream);

    return output;
}

uint32_t AudioPackager::calculateCRC32(const std::vector<uint8_t>& data) {
    uint32_t crc = 0xFFFFFFFF;
    
    for (uint8_t byte : data) {
        crc ^= byte;
        for (int i = 0; i < 8; ++i) {
            crc = (crc >> 1) ^ (0xEDB88320 & -(crc & 1));
        }
    }
    
    return ~crc;
}

bool AudioPackager::packFiles(const std::vector<std::string>& inputFiles,
                            const std::string& outputFile,
                            const std::string& password) {
    try {
        PackageHeader header;
        std::vector<FileEntry> entries;
        std::vector<std::vector<uint8_t>> fileData;

        Encryption::generateSaltAndIV(header.salt, header.iv);

        uint64_t currentOffset = sizeof(PackageHeader);
        
        for (const auto& inputPath : inputFiles) {
            juce::File file(inputPath);
            if (!file.existsAsFile())
                continue;

            juce::MemoryBlock rawData;
            juce::FileInputStream input(file);
            
            if (!input.openedOk())
                continue;
                
            input.readIntoMemoryBlock(rawData);

            FileEntry entry;
            entry.filename = file.getFileName().toStdString();
            entry.originalSize = static_cast<uint32_t>(rawData.getSize());
            entry.offset = currentOffset;

            juce::String ext = file.getFileExtension().toLowerCase();
            if (ext == ".wav") entry.fileType = 0;
            else if (ext == ".aif" || ext == ".aiff") entry.fileType = 1;
            else if (ext == ".irp") entry.fileType = 2;
            else continue;

            std::vector<uint8_t> data(static_cast<const uint8_t*>(rawData.getData()),
                                    static_cast<const uint8_t*>(rawData.getData()) + rawData.getSize());

            auto compressed = compressData(data);
            auto encrypted = Encryption::encrypt(compressed, password, header.salt, header.iv);

            entry.compressedSize = static_cast<uint32_t>(encrypted.size());
            entry.crc32 = calculateCRC32(data);

            currentOffset += sizeof(uint32_t) + entry.filename.length() +
                           sizeof(FileEntry) + encrypted.size();

            entries.push_back(entry);
            fileData.push_back(std::move(encrypted));
        }

        header.numFiles = static_cast<uint32_t>(entries.size());

        juce::File outFile(outputFile);
        juce::FileOutputStream output(outFile);
        
        if (!output.openedOk())
            return false;

        output.write(&header, sizeof(PackageHeader));

        for (size_t i = 0; i < entries.size(); ++i) {
            const auto& entry = entries[i];
            const auto& data = fileData[i];

            uint32_t nameLength = static_cast<uint32_t>(entry.filename.length());
            output.writeInt(nameLength);
            output.write(entry.filename.c_str(), nameLength);

            output.writeInt64(entry.offset);
            output.writeInt(entry.originalSize);
            output.writeInt(entry.compressedSize);
            output.writeInt(entry.crc32);
            output.writeByte(entry.fileType);

            output.write(data.data(), data.size());
        }

        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error packing files: " << e.what() << std::endl;
        return false;
    }
}

bool AudioPackager::unpackFiles(const std::string& packageFile,
                              const std::string& outputDir,
                              const std::string& password) {
    try {
        juce::File inFile(packageFile);
        juce::FileInputStream input(inFile);
        
        if (!input.openedOk())
            return false;

        PackageHeader header;
        input.read(&header, sizeof(PackageHeader));

        if (memcmp(header.signature, "APKG2024", 8) != 0)
            return false;

        juce::File outDir(outputDir);
        outDir.createDirectory();

        for (uint32_t i = 0; i < header.numFiles; ++i) {
            uint32_t nameLength;
            input.read(&nameLength, sizeof(uint32_t));

            std::vector<char> nameBuffer(nameLength + 1, '\0');
            input.read(nameBuffer.data(), nameLength);
            
            FileEntry entry;
            entry.filename = std::string(nameBuffer.data());
            entry.offset = input.readInt64();
            entry.originalSize = input.readInt();
            entry.compressedSize = input.readInt();
            entry.crc32 = input.readInt();
            entry.fileType = input.readByte();

            std::vector<uint8_t> encryptedData(entry.compressedSize);
            input.read(encryptedData.data(), entry.compressedSize);

            auto decrypted = Encryption::decrypt(encryptedData, password, 
                                               header.salt, header.iv);
            auto decompressed = decompressData(decrypted, entry.originalSize);

            if (calculateCRC32(decompressed) != entry.crc32)
                return false;

            juce::File outFile = outDir.getChildFile(entry.filename);
            juce::FileOutputStream output(outFile);
            
            if (!output.openedOk())
                return false;

            output.write(decompressed.data(), decompressed.size());
        }

        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error unpacking files: " << e.what() << std::endl;
        return false;
    }
}