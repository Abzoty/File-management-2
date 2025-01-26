#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <map>

using namespace std;


//2 * MAX_KEYS + 1 >>>>>>>>>> Size of each record (NODE)
const int MAX_RECORDS = 11;
const int MAX_KEYS = 5;
const int sizeOfNode = (2 * (2 * MAX_KEYS + 1)) + 2;
const int DELIMITER = 2573; // It's used to signal when reading should stop or when a specific section of the file has ended.

typedef map<short, short> Records;

struct Node {
    short nodeType;                 // 0 for leaf, 1 for non-leaf
    vector<pair<short, short>> records;
};

pair<int, int> findMaxInstance(const Records &recordMap) {
    pair<int, int> maxInstance = {INT_MIN, INT_MIN};
    for (const auto &p: recordMap) {
        if (p.first > maxInstance.first) {
            maxInstance = p;
        }
    }
    return maxInstance;
}

void split(Records map1, short id, short reference, fstream &file, int position, int Parent);

void CreateIndexFileFile(const char *filename, int numberOfRecords, int record);

Records readRecord(short offset, fstream &file);

void writeRecord(Records record, fstream &file);

int InsertNewRecordAtIndex(const char *filename, short recordID, short reference);

void DisplayIndexFileContent(const char *filename);

int main() {
    const char *filename = "index.txt";
    CreateIndexFileFile(filename, 10, 5);
    cout << "Index File Content After Creation:" << endl;
    DisplayIndexFileContent(filename);

    InsertNewRecordAtIndex(filename, 3, 12);
    InsertNewRecordAtIndex(filename, 7, 24);
    InsertNewRecordAtIndex(filename, 10, 48);
    InsertNewRecordAtIndex(filename, 24, 60);
    InsertNewRecordAtIndex(filename, 14, 72);

    cout << "\nIndex File Content After Insertions:" << endl;
    DisplayIndexFileContent(filename);

    InsertNewRecordAtIndex(filename, 19, 84);

    cout << "\nIndex File Content After Split:" << endl;
    DisplayIndexFileContent(filename);

    return 0;
}

void split(Records map1, short id, short reference, fstream &file, int position, int Parent) {
    const short nodeSize = (2 * (2 * MAX_KEYS + 1)) + 2;
    short nodeType = 0;

    Records parentNode;
    file.seekg(Parent + 2, ios::beg);
    parentNode = readRecord(Parent, file);

    map1.insert({id, reference});

    size_t originalSize = map1.size();
    size_t halfSize = originalSize / 2;

    Records child1(map1.begin(), next(map1.begin(), halfSize));
    Records child2(next(map1.begin(), halfSize), map1.end());

    //Max instance in every half
    pair<short, short> maxInstanceChild1 = findMaxInstance(child1);
    pair<short, short> maxInstanceChild2 = findMaxInstance(child2);

    if (0 < parentNode.size() && parentNode.size() < MAX_KEYS && Parent != 0) {
        file.seekg(position - 2, ios::beg);
        short x = file.tellg() / nodeSize;

        parentNode[maxInstanceChild1.first] = x;
        nodeType = 0;
        file.write(reinterpret_cast<char *>(&nodeType), sizeof(nodeType));
        writeRecord(child1, file);

        short value;
        file.read(reinterpret_cast<char *>(&value), sizeof(value));
        while (value != DELIMITER) {
            short x = -1;
            file.seekg(-2, ios::cur);
            file.write(reinterpret_cast<char *>(&x), sizeof(x));
            file.read(reinterpret_cast<char *>(&value), sizeof(value));
        }

        file.seekg(nodeSize, ios::beg);
        file.read(reinterpret_cast<char *>(&nodeType), sizeof(nodeType));

        short p1;
        for (short i = 1; i < 10; ++i) {
            if (nodeType == -1) {
                nodeType = 0;
                file.seekg(-2, ios::cur);
                p1 = i;

                file.write(reinterpret_cast<char *>(&nodeType), sizeof(nodeType));
                writeRecord(child2, file);
                file.flush();
                break;
            } else {
                file.seekg(nodeSize - 2, ios::cur);
                file.read(reinterpret_cast<char *>(&nodeType), sizeof(nodeType));
            }
        }
        parentNode[maxInstanceChild2.first] = p1;
        file.seekg(Parent + 2, ios::beg);
        writeRecord(parentNode, file);
    } else {
        // Put the maximum instances back into the original map
        map1.clear();
        map1.insert(maxInstanceChild1);
        map1.insert(maxInstanceChild2);
        // Assuming position is the position in the file where you want to write
        file.seekg(position - 2, ios::beg);
        file.read(reinterpret_cast<char *>(&nodeType), sizeof(nodeType));

        short p1;
        for (short i = 1; i < 10; ++i) {
            // Your existing code for reading isLeaf goes here
            if (nodeType == -1) {
                nodeType = 0;

                file.seekg(-2, ios::cur);
                short r = file.tellg();
                p1 = i;

                file.write(reinterpret_cast<char *>(&nodeType), sizeof(nodeType));
                writeRecord(child1, file);
                //p2 = i+1;
                file.seekg(r + nodeSize, ios::beg);

                file.write(reinterpret_cast<char *>(&nodeType), sizeof(nodeType));

                writeRecord(child2, file);

                file.flush();

                break;
            } else {

                file.seekg(nodeSize - 2, ios::cur);
                file.read(reinterpret_cast<char *>(&nodeType), sizeof(nodeType));
            }

        }
        file.seekg(position - 2, ios::beg);

        nodeType = 1;

        file.write(reinterpret_cast<char *>(&nodeType), sizeof(nodeType));

        for (const auto &entry: map1) {

            if (entry.first != -1) {
                map1[entry.first] = p1++;
            }

            file.write(reinterpret_cast<const char *>(&entry.first), sizeof(entry.first));
            file.write(reinterpret_cast<const char *>(&entry.second), sizeof(entry.second));
        }
    }
    short value;
    file.read(reinterpret_cast<char *>(&value), sizeof(value));
    while (value != DELIMITER) {

        short x = -1;

        file.seekg(-2, ios::cur);
        file.write(reinterpret_cast<char *>(&x), sizeof(x));
        file.read(reinterpret_cast<char *>(&value), sizeof(value));
    }
}


int InsertNewRecordAtIndex(const char *filename, short recordID, short reference) {
    fstream file(filename, std::ios::binary | std::ios::out | std::ios::in);
    short sizeOfNode = (2 * (2 * MAX_KEYS + 1)) + 2;
    short nodeType;

    file.seekg(sizeOfNode, std::ios::beg);
    file.read(reinterpret_cast<char *>(&nodeType), sizeof(nodeType));

    short position = file.tellg();

    if (nodeType == -1) {
        short leaf = 0;
        file.seekg(sizeOfNode, ios::beg);
        file.write(reinterpret_cast<char *>(&leaf), sizeof(leaf));
        file.write(reinterpret_cast<char *>(&recordID), sizeof(recordID));
        file.write(reinterpret_cast<char *>(&reference), sizeof(reference));
        file.flush();
        file.close();
        return 0;
    }

    map<short, short> node = readRecord(position, file);

    // if the node is a leaf (nodeType == 0)
    if (!nodeType) {
        file.seekg(position, ios::beg);
        if (node.size() < MAX_KEYS) {
            node[recordID] = reference;
            file.seekp(position, ios::beg);
            writeRecord(node, file);
            file.close();
            return 0;
        } else {
            split(node, recordID, reference, file, position, 0);
            return 0;
        }
    }

    if (nodeType) {
        vector<pair<short, short>> memory;
        memory.push_back(make_pair(node.size(), position - 2));

        short start = -2;
        bool isBiggest = false;
        while (nodeType != 0) {

            short place = -2;
            for (const auto &entry: node) {
                if (entry.first > recordID) {
                    place = entry.second;
                    break;
                }
            }
            if (place == -2) {
                isBiggest = true;

                auto lastElementIterator = node.rbegin();

                short lastValue = lastElementIterator->second;

                place = lastValue;
            }
            file.seekg((place * sizeOfNode), ios::beg);
            start = file.tellg();
            file.read(reinterpret_cast<char *>(&nodeType), sizeof(nodeType));
            node = readRecord(place, file);
            memory.push_back(make_pair(node.size(), start));
        }

        memory.pop_back();

        if (node.size() < MAX_KEYS) {
            node[recordID] = reference;
            file.seekp(start + 2, ios::beg);
            writeRecord(node, file);

            while (!memory.empty()) {

                file.seekg((memory.rbegin()->second) + 2, ios::beg);
                short p = file.tellg();
                node = readRecord(p, file);

                auto lastElementIterator = node.rbegin();

                short lastValue = lastElementIterator->first;

                if (isBiggest) {
                    node[recordID] = node[lastValue];
                    node.erase(lastValue);
                    file.seekg((memory.rbegin()->second) + 2, ios::beg);
                    writeRecord(node, file);
                }
                memory.pop_back();
            }
            file.close();
            file.open(filename, ios::binary | ios::out | ios::in);
            short leaf = -2;
            file.seekg(sizeOfNode, ios::beg);
            file.read((char *) &leaf, sizeof(leaf));
            while (true) {
                if (leaf == -1) {
                    short offset = (file.tellg());
                    offset = (offset - 2) / sizeOfNode;
                    file.seekg(2, ios::beg);
                    file.write((char *) &offset, sizeof(offset));
                    break;
                } else {
                    file.seekg(sizeOfNode - 2, ios::cur);
                    file.read((char *) &leaf, sizeof(leaf));
                }
            }
            return 0;
        } else {
            file.seekg(0, ios::beg);
            while (!memory.empty()) {
                if (memory.rbegin()->first < MAX_KEYS) {
                    split(node, recordID, reference, file, start + 2, memory.rbegin()->second);
                } else {
                    file.seekg(memory.rbegin()->second, ios::beg);
                    short p = file.tellg();
                    node = readRecord(p, file);
                }
                memory.pop_back();
            }

        }
    }
    file.close();
    file.open(filename, ios::binary | ios::out | ios::in);
    short leaf = -2;
    file.seekg(sizeOfNode, ios::beg);
    file.read((char *) &leaf, sizeof(leaf));
    while (true) {
        if (leaf == -1) {
            short offset = (file.tellg());
            offset = (offset - 2) / sizeOfNode;
            file.seekg(2, ios::beg);
            file.write((char *) &offset, sizeof(offset));
            break;
        } else {
            file.seekg(sizeOfNode - 2, ios::cur);
            file.read((char *) &leaf, sizeof(leaf));
        }
    }
    return 0;
}

//void CreateIndexFileFile(const char *filename, int numberOfRecords, int m) {
//    fstream file(filename, ios::out);
//
//    if (file.is_open()) {
//        numberOfRecords = (numberOfRecords * 2) + 1;
//        short num = -1;
//        for (short i = 0; i < m; i++) {
//            for (int j = 0; j < numberOfRecords; j++) {
//                file.write((char *) &num, sizeof(num));
//            }
//            file << '\n';
//        }
//    }
//}

void CreateIndexFileFile(const char *filename, int n, int record) {
    fstream file(filename, ios::out);
    if (file.is_open()) {
        cout << "file open successfully\n";
        n = (n * 2) + 1;
        short num = -1, orderR = 1;
        for (short i = 0; i < 10; i++) {
            for (int j = 0; j < 11; j++) {
                file.write((char *) &num, sizeof(num));
            }
            file << '\n';
        }
    }
}

Records readRecord(short offset, fstream &file) {
    map<short, short> node;
    int temp = sizeOfNode - 1;
    while (temp) {
        short value;
        file.read(reinterpret_cast<char *>(&value), sizeof(value));
        file.read(reinterpret_cast<char *>(&offset), sizeof(offset));

        if (value == -1 || value == DELIMITER)
            break;

        node[value] = offset;
        temp -= 2;
    }
    return node;
}

void writeRecord(Records record, fstream &file) {
    for (const auto &entry: record) {
        file.write(reinterpret_cast<const char *>(&entry.first), sizeof(entry.first));
        file.write(reinterpret_cast<const char *>(&entry.second), sizeof(entry.second));
    }
    file.flush();
}

void DisplayIndexFileContent(const char *filename) {
    ifstream file(filename, ios::binary);
    short value;
    int count = 0;

    while (file.read(reinterpret_cast<char *>(&value), sizeof(value))) {
        if (value != 2573) {
            cout << setw(4) << value << ' ';
            count++;

            if (count % 11 == 0) {
                cout << '\n';
                count = 0;
            }
        }
    }
    file.close();
}

//void DisplayIndexFileContent(const char *filename) {
//    ifstream file(filename, ios::binary);
//    short value;
//
//    while (file.read(reinterpret_cast<char *>(&value), sizeof(value))) {
//        if (value != DELIMITER) cout << value << ' ';
//        else cout << '\n';
//    }
//    file.close();
//}
