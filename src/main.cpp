// src/main.cpp
#include "AudioPackager.hpp"
#include "IRProcessor.hpp"
#include <iostream>

void printUsage() {
    std::cout << "Usage:\n"
              << "Pack:   IRPack -p <output.apkg> <password> <input files...>\n"
              << "Unpack: IRPack -u <input.apkg> <output_dir> <password>\n"
              << "Convert to IR: IRPack -c <input_audio> <output.irp> <password>\n";
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        printUsage();
        return 1;
    }

    std::string command = argv[1];
    
    try {
        if (command == "-p") {
            if (argc < 5) {
                printUsage();
                return 1;
            }

            std::string outputFile = argv[2];
            std::string password = argv[3];
            std::vector<std::string> inputFiles;
            
            for (int i = 4; i < argc; ++i) {
                inputFiles.push_back(argv[i]);
            }

            if (!AudioPackager::packFiles(inputFiles, outputFile, password)) {
                std::cerr << "Failed to pack files" << std::endl;
                return 1;
            }
            
            std::cout << "Successfully packed files to: " << outputFile << std::endl;
        }
        else if (command == "-u") {
            if (argc != 5) {
                printUsage();
                return 1;
            }

            std::string inputFile = argv[2];
            std::string outputDir = argv[3];
            std::string password = argv[4];

            if (!AudioPackager::unpackFiles(inputFile, outputDir, password)) {
                std::cerr << "Failed to unpack files" << std::endl;
                return 1;
            }
            
            std::cout << "Successfully unpacked files to: " << outputDir << std::endl;
        }
        else if (command == "-c") {
            if (argc != 5) {
                printUsage();
                return 1;
            }

            std::string inputFile = argv[2];
            std::string outputFile = argv[3];
            std::string password = argv[4];

            if (!IRProcessor::processAudioFile(inputFile, outputFile, password)) {
                std::cerr << "Failed to convert audio file" << std::endl;
                return 1;
            }
            
            std::cout << "Successfully converted to IR: " << outputFile << std::endl;
        }
        else {
            printUsage();
            return 1;
        }

        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}