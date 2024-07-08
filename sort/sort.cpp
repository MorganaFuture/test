#include <algorithm>
#include <iostream>
#include <vector>
#include <fstream>
#include <utility>

#include <math.h>

namespace  {

void convertBinaryFileToTextFile(const std::string& aBinFileName, const std::string& aTxtFileName) {
    std::ifstream readBinFile(aBinFileName, std::ios::binary);

    if (!readBinFile.is_open()) {
        std::cerr << "Unable to open binary file" << std::endl;
        return;
    }

    std::ofstream writeSortedFile(aTxtFileName);

    double doubleValue = NAN;
    while (readBinFile.read(reinterpret_cast<char*>(&doubleValue), sizeof(doubleValue))) {
        writeSortedFile << doubleValue << '\n';
    }

    readBinFile.close();
    writeSortedFile.close();

    remove(aBinFileName.c_str());
}

}

class ExternalMergeSort {
public:

ExternalMergeSort(std::string aUnsortedName, std::string aSortedName) : 
    iUnsortedFileName(std::move(aUnsortedName)),
    iSortedFileName(std::move(aSortedName)),
    iNumOfChunk(0) {}

void Sort() {
    CreateAndSplitChunks();
    MergeAndCleanupSortedChunks();
}

private:

static auto MergeSort(const std::string &file1, const std::string &file2, const std::string &result) -> bool {
    std::ifstream sorted_1(file1);
    std::ifstream sorted_2(file2);
    
    if(!sorted_1.good() || !sorted_2.good()) {    
        return false;
    }
    
    std::ofstream output_file(result);
    double d_value1 = 0;
    double d_value2 = 0;

    sorted_1.read(reinterpret_cast<char*>(&d_value1), sizeof(d_value1));
    sorted_2.read(reinterpret_cast<char*>(&d_value2), sizeof(d_value2));

    while (sorted_1.good() && sorted_2.good()) {
        if(d_value1 <= d_value2) {
            output_file.write(reinterpret_cast<char*>(&d_value1), sizeof(d_value1));
            sorted_1.read(reinterpret_cast<char*>(&d_value1), sizeof(d_value1));
        } else {
            output_file.write(reinterpret_cast<char*>(&d_value2), sizeof(d_value2));
            sorted_2.read(reinterpret_cast<char*>(&d_value2), sizeof(d_value2));
        }
    }

    // Write the remaining values from sorted_1 (if any)
    while (sorted_1.good()) {
        output_file.write(reinterpret_cast<char*>(&d_value1), sizeof(d_value1));
        sorted_1.read(reinterpret_cast<char*>(&d_value1), sizeof(d_value1));
    }

    // Write the remaining values from sorted_2 (if any)
    while (sorted_2.good()) {
        output_file.write(reinterpret_cast<char*>(&d_value2), sizeof(d_value2));
        sorted_2.read(reinterpret_cast<char*>(&d_value2), sizeof(d_value2));
    }

    output_file.close();
    sorted_1.close();
    sorted_2.close();

    remove(file1.c_str());
    remove(file2.c_str());

    return true;
}

void CreateAndSplitChunks() {
    std::ifstream unsortedFile(iUnsortedFileName);
    std::vector<double> arr;

    if (!unsortedFile.is_open()) {
        std::cerr << "Unable to open unsorted file" << std::endl;
        return;
    }

    while(unsortedFile.good()) {
        std::string line;

        for(size_t i = 0; i < CHUNK_SIZE; i++) {
            std::getline(unsortedFile, line);

            if(!line.empty()) {
                arr.push_back(std::stod(line));
            }

            if(!unsortedFile.good()) { 
                break;
            }
        }
        
        std::sort(arr.begin(), arr.end());
        std::string tmpFile = "sorted_" + std::to_string(iNumOfChunk) +".bin";
        std::ofstream sorted(tmpFile);
        
        for ( auto &i : arr ) {
            sorted.write(reinterpret_cast<char*>(&i), sizeof(i));
        }

        sorted.close();
        iNumOfChunk++;
        arr.clear();
    }

    unsortedFile.close();
}

void MergeAndCleanupSortedChunks() {
    size_t chunkIndex = 0;
    for( ; chunkIndex < iNumOfChunk; chunkIndex += 2, iNumOfChunk++) {
        if(!MergeSort("sorted_" + std::to_string(chunkIndex) + ".bin", 
                        "sorted_" + std::to_string(chunkIndex + 1 ) + ".bin", 
                        "sorted_" + std::to_string(iNumOfChunk) + ".bin")) {
            break;
        }
    }
        
    std::string sortedBinFile = "sorted_" + std::to_string(chunkIndex) +".bin";
    convertBinaryFileToTextFile(sortedBinFile, iSortedFileName);
    remove(sortedBinFile.c_str());
}

private:
    std::string iUnsortedFileName;
    std::string iSortedFileName;
    size_t iNumOfChunk;
    constexpr static size_t CHUNK_SIZE = 100000000/sizeof(double); // 100MB
};

int main(int argc, char **argv) {
    
    if(argc < 3) {
        std::cout << "Please use this format: " << argv[0] << " unsorted_file sorted_file" << std::endl;
        return 1;
    }

    ExternalMergeSort externalMegrge(argv[1], argv[2]);
    externalMegrge.Sort();
    
    return 0;
}


