#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem> 
#include <zstd.h>
#include "sarcReader.h" 

namespace fs = std::filesystem;

int main() {
    std::string inputPath, outputDir;

    std::cout << "--- ZS Extractor ---" << std::endl;
    std::cout << ".zs path: ";
    std::getline(std::cin, inputPath);
    std::cout << "dest folder: ";
    std::getline(std::cin, outputDir);

    std::ifstream file(inputPath, std::ios::binary | std::ios::ate);
    if (!file) return (std::cerr << "Not a valid path!\n", 1);

    std::streamsize srcSize = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<u8> srcBuffer(srcSize);
    file.read((char*)srcBuffer.data(), srcSize);

    unsigned long long const decompSize = ZSTD_getFrameContentSize(srcBuffer.data(), srcSize);
    std::vector<u8> decompBuffer(decompSize);

    size_t const result = ZSTD_decompress(decompBuffer.data(), decompSize, srcBuffer.data(), srcSize);
    if (ZSTD_isError(result)) {
        std::cerr << "Zstd error: " << ZSTD_getErrorName(result) << std::endl;
        return 1;
    }
    std::cout << "succes.\n";

    SARCReader reader;
    if (!reader.open(decompBuffer.data())) {
        std::cerr << "SARC couldnt be opened!\n";
        return 1;
    }

    auto files = reader.listFiles();
    std::cout << "Extract " << files.size() << " files...\n";

    if (!fs::exists(outputDir)) fs::create_directories(outputDir);

    for (const auto& entry : files) {
        fs::path outPath = fs::path(outputDir) / entry.name;
        fs::create_directories(outPath.parent_path());

        std::ofstream out(outPath, std::ios::binary);
        out.write((const char*)entry.data, entry.size);
        std::cout << "saved: " << entry.name << std::endl;
    }

    std::cout << "\nAll Files Extracted.\n";
    return 0;
}