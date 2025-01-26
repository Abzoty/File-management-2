#ifndef FILE_MANAGEMENT_2_RECORDFILE_H
#define FILE_MANAGEMENT_2_RECORDFILE_H

#ifndef RECORD_FILE_H
#define RECORD_FILE_H

#include <iostream>
#include <fstream>

// Example of a simple recordFile template to manage records in a file.
template <typename BTNode>
class recordFile {
public:
    recordFile(const char* filename);
    ~recordFile();

    // Open the file for reading/writing
    bool open(int mode);

    // Close the file
    void close();

    // Store a BTNode record in the file
    bool store(const BTNode& node);

    // Fetch a BTNode record from the file
    bool fetch(int recAddr, BTNode& node);

private:
    std::fstream file;  // File stream for reading/writing
    const char* filename; // File name
};

template <typename BTNode>
recordFile<BTNode>::recordFile(const char* filename) : filename(filename) {}

template <typename BTNode>
recordFile<BTNode>::~recordFile() {
    if (file.is_open()) {
        close();
    }
}

template <typename BTNode>
bool recordFile<BTNode>::open(int mode) {
    if (mode == 1) {
        file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
    } else {
        file.open(filename, std::ios::out | std::ios::binary);
    }
    return file.is_open();
}

template <typename BTNode>
void recordFile<BTNode>::close() {
    if (file.is_open()) {
        file.close();
    }
}

template <typename BTNode>
bool recordFile<BTNode>::store(const BTNode& node) {
    if (!file.is_open()) return false;
    file.write(reinterpret_cast<const char*>(&node), sizeof(BTNode)); // Write node to file
    return file.good();
}

template <typename BTNode>
bool recordFile<BTNode>::fetch(int recAddr, BTNode& node) {
    if (!file.is_open()) return false;
    file.seekg(recAddr * sizeof(BTNode), std::ios::beg); // Seek to the address
    file.read(reinterpret_cast<char*>(&node), sizeof(BTNode)); // Read node from file
    return file.good();
}

#endif // RECORD_FILE_H


#endif //FILE_MANAGEMENT_2_RECORDFILE_H
