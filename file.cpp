#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iomanip>

const int MAX_RECORDS = 12; // Total slots including node type and next free node
const int MAX_KEYS = 5;     // m in the document

struct Node
{
    int nodeType;                 // 0 for leaf, 1 for non-leaf
    int nextFreeNode;             // Index of next free node
    int records[MAX_RECORDS - 2]; // Record/Reference pairs and child node indices

    Node() : nodeType(0), nextFreeNode(-1)
    {
        std::fill(std::begin(records), std::end(records), -1);
    }
};

void CreateIndexFileFile(const char *filename, int numberOfRecords, int m)
{
    std::ofstream file(filename, std::ios::binary);

    Node firstNode;
    firstNode.nextFreeNode = 1;
    file.write(reinterpret_cast<const char *>(&firstNode), sizeof(Node));

    for (int i = 1; i < numberOfRecords; ++i)
    {
        Node emptyNode;
        emptyNode.nextFreeNode = (i < numberOfRecords - 1) ? (i + 1) : -1;
        file.write(reinterpret_cast<const char *>(&emptyNode), sizeof(Node));
    }

    file.close();
}

Node readNode(const char *filename, int nodeIndex)
{
    std::ifstream file(filename, std::ios::binary);
    Node node;
    file.seekg(nodeIndex * sizeof(Node));
    file.read(reinterpret_cast<char *>(&node), sizeof(Node));
    file.close();
    return node;
}

void writeNode(const char *filename, int nodeIndex, const Node &node)
{
    std::fstream file(filename, std::ios::in | std::ios::out | std::ios::binary);
    file.seekp(nodeIndex * sizeof(Node));
    file.write(reinterpret_cast<const char *>(&node), sizeof(Node));
    file.close();
}

int findInsertPosition(const Node &node, int recordID)
{
    int pos = 0;
    while (pos < MAX_RECORDS - 2 && node.records[pos] != -1 && node.records[pos] < recordID)
    {
        pos += 2;
    }
    return pos;
}

void splitChild(Node &x, Node &y)
{
    int medianIndex = (MAX_RECORDS - 2) / 2;

    // Move the median key to the new node
    y.records[0] = x.records[medianIndex];
    y.records[1] = x.records[medianIndex + 1];

    // Move the right half of the keys to the new node
    for (int i = medianIndex + 2; i < MAX_RECORDS - 2; i += 2)
    {
        y.records[i - medianIndex] = x.records[i];
        y.records[i - medianIndex + 1] = x.records[i + 1];
    }

    // Update the left half of the keys in the original node
    for (int i = medianIndex; i < MAX_RECORDS - 2; i += 2)
    {
        x.records[i] = -1;
        x.records[i + 1] = -1;
    }

    // Update the nextFreeNode of the original node
    int newNodeIndex = x.nextFreeNode;
    x.nextFreeNode = newNodeIndex;
}

int InsertNewRecordAtIndex(const char *filename, int recordID, int reference)
{
    Node rootNode = readNode(filename, 1);
    if (rootNode.records[MAX_RECORDS - 3] != -1)
    {
        // Node is full, perform a split
        Node newNode;
        newNode.nodeType = 1; // Non-leaf node
        newNode.nextFreeNode = -1;

        // Split the node
        splitChild(rootNode, newNode);

        // Write the updated nodes to the file
        int newNodeIndex = rootNode.nextFreeNode;
        writeNode(filename, 1, rootNode);
        writeNode(filename, newNodeIndex, newNode);

        // Recursively insert the record into the new node
        return InsertNewRecordAtIndex(filename, recordID, reference);
    }

    // Node is not full, insert the record as usual
    int pos = findInsertPosition(rootNode, recordID);

    for (int i = MAX_RECORDS - 3; i > pos; i -= 2)
    {
        rootNode.records[i] = rootNode.records[i - 2];
        rootNode.records[i + 1] = rootNode.records[i - 1];
    }

    rootNode.records[pos] = recordID;
    rootNode.records[pos + 1] = reference;
    writeNode(filename, 1, rootNode);

    return 1;
}

void DeleteRecordFromIndex(const char *filename, int recordID)
{
    Node rootNode = readNode(filename, 1);
    for (int i = 0; i < MAX_RECORDS - 2; i += 2)
    {
        if (rootNode.records[i] == recordID)
        {
            rootNode.records[i] = -1;
            rootNode.records[i + 1] = -1;
            for (int j = i; j < MAX_RECORDS - 4; j += 2)
            {
                rootNode.records[j] = rootNode.records[j + 2];
                rootNode.records[j + 1] = rootNode.records[j + 3];
            }
            rootNode.records[MAX_RECORDS - 4] = -1;
            rootNode.records[MAX_RECORDS - 3] = -1;
            writeNode(filename, 1, rootNode);
            return;
        }
    }
}

int SearchARecord(const char *filename, int recordID)
{
    Node rootNode = readNode(filename, 1);
    for (int i = 0; i < MAX_RECORDS - 2; i += 2)
    {
        if (rootNode.records[i] == recordID)
        {
            return rootNode.records[i + 1]; // Return reference if found
        }
    }
    return -1; // Not found
}

void DisplayIndexFileContent(const char *filename)
{
    std::ifstream file(filename, std::ios::binary);
    int nodeCount = 0;
    Node node;

    while (file.read(reinterpret_cast<char *>(&node), sizeof(Node)))
    {
        std::cout << "Node " << std::setw(2) << nodeCount << ": ";
        std::cout << "Type: " << std::setw(2) << node.nodeType << ", Next Free: " << std::setw(2) << node.nextFreeNode << " | ";
        for (int i = 0; i < MAX_RECORDS - 2; ++i)
        {
            std::cout << std::setw(2) << node.records[i] << " ";
        }
        std::cout << std::endl;
        nodeCount++;
    }

    file.close();
}

int main()
{
    const char *filename = "index.bin";
    CreateIndexFileFile(filename, 10, 5);

    InsertNewRecordAtIndex(filename, 3, 12);
    InsertNewRecordAtIndex(filename, 7, 24);
    InsertNewRecordAtIndex(filename, 10, 48);
    InsertNewRecordAtIndex(filename, 24, 60);
    InsertNewRecordAtIndex(filename, 14, 72);

    std::cout << "Index File Content After Insertions:" << std::endl;
    DisplayIndexFileContent(filename);

    InsertNewRecordAtIndex(filename, 19, 84);

    std::cout << "Index File Content After Split:" << std::endl;
    DisplayIndexFileContent(filename);

    // DeleteRecordFromIndex(filename, 10);

    // std::cout << "Index File Content After Deletion:" << std::endl;
    // DisplayIndexFileContent(filename);

    // int ref = SearchARecord(filename, 7);
    // if (ref != -1)
    //     std::cout << "Record 7 found with reference: " << ref << std::endl;
    // else
    //     std::cout << "Record 7 not found." << std::endl;

    return 0;
}
