#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iomanip>
using namespace std;

const int MAX_RECORDS = 11; // Total slots including node type and next free node
const int MAX_KEYS = 5;     // m in the document

struct Node
{
    int nodeType;                 // 0 for leaf, 1 for non-leaf
    int records[MAX_RECORDS - 1]; // Record/Reference pairs and child node indices

    Node() : nodeType(-1)
    {
        fill(begin(records), end(records), -1);
    }
};

void CreateIndexFileFile(const char *filename, int numberOfRecords, int m)
{
    ofstream file(filename, ios::binary);

    Node firstNode;
    firstNode.records[0] = 1;
    file.write(reinterpret_cast<const char *>(&firstNode), sizeof(Node));

    for (int i = 1; i < numberOfRecords; ++i)
    {
        Node emptyNode;
        emptyNode.records[0] = (i < numberOfRecords - 1) ? (i + 1) : -1;
        file.write(reinterpret_cast<const char *>(&emptyNode), sizeof(Node));
    }

    file.close();
}

Node readNode(const char *filename, int nodeIndex)
{
    ifstream file(filename, ios::in | ios::out | ios::binary);
    Node node;
    file.seekg(nodeIndex * sizeof(Node));
    file.read(reinterpret_cast<char *>(&node), sizeof(Node));
    file.close();
    return node;
}

void writeNode(const char *filename, int nodeIndex, const Node &node)
{
    fstream file(filename, ios::in | ios::out | ios::binary);
    file.seekp(nodeIndex * sizeof(Node));
    file.write(reinterpret_cast<const char *>(&node), sizeof(Node));
    file.close();
}

int findInsertPosition(const Node &node, int recordID)
{
    int pos = 0;

    if (node.nodeType == -1)
    {
        return pos;
    }

    while (pos < MAX_RECORDS - 2 && node.records[pos] != -1 && node.records[pos] < recordID)
    {
        pos += 2;
    }
    return pos;
}

/* void splitChild(Node &x, Node &y)
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
} */

void updateHead()
{
    int i = 1;
    while (true)
    {
        Node head = readNode("index.bin", i);
        if (head.nodeType == -1)
            break;

        i++;
    }

    Node head = readNode("index.bin", 0);
    head.records[0] = i;
    writeNode("index.bin", 0, head);
}

int Insert(const char *filename, int nodeIndex, int recordID, int reference)
{
    Node rootNode = readNode(filename, nodeIndex);

    if (rootNode.nodeType == 0 || rootNode.nodeType == -1)
    {
        // Node is a leaf, find the right position to insert
        int pos = findInsertPosition(rootNode, recordID);

        if (rootNode.nodeType != -1)
        {
            for (int i = MAX_RECORDS - 2; i > pos; i -= 2)
            {
                rootNode.records[i] = rootNode.records[i - 2];
                rootNode.records[i + 1] = rootNode.records[i - 1];
            }
        }

        rootNode.records[pos] = recordID;
        rootNode.records[pos + 1] = reference;
        int type = rootNode.nodeType;
        rootNode.nodeType = 0;
        writeNode(filename, nodeIndex, rootNode);

        if (type == -1)
        {
            updateHead();
        }

        return nodeIndex;
    }
    else if (rootNode.nodeType == 1)
    {
        int childIndex = -1, i = 0;
        // Node is non-leaf, find the first item bigger than recordID
        while (rootNode.records[i] != -1)
        {
            if (rootNode.records[i] > recordID)
            {
                childIndex = rootNode.records[i + 1]; // Return the reference (index + 1)
            }
            i += 2;
        }

        if (childIndex == -1)
        {
            // No child found, insert the record in the last child
            childIndex = rootNode.records[i - 1];
        }

        return Insert(filename, childIndex, recordID, reference);
    }

    return 1;
}

int InsertNewRecordAtIndex(const char *filename, int recordID, int reference)
{
    return Insert(filename, 1, recordID, reference);
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
            for (int j = i; j < MAX_RECORDS -2; j += 2)
            {
                rootNode.records[j] = rootNode.records[j + 2];
                rootNode.records[j + 1] = rootNode.records[j + 3];
            }
            rootNode.records[MAX_RECORDS - 2] = -1;
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
    ifstream file(filename, ios::binary);
    int nodeCount = 0;
    Node node;

    while (file.read(reinterpret_cast<char *>(&node), sizeof(Node)))
    {
        cout << "Node " << setw(2) << nodeCount << ": ";
        cout << "Type: " << setw(2) << node.nodeType << " | ";
        for (int i = 0; i < MAX_RECORDS - 1; ++i)
        {
            cout << setw(2) << node.records[i] << " ";
        }
        cout << endl;
        nodeCount++;
    }

    file.close();
}

int main()
{
    const char *filename = "index.bin";
    CreateIndexFileFile(filename, 10, 5);
    cout << "Index File Content After Creation:" << endl;
    DisplayIndexFileContent(filename);

    InsertNewRecordAtIndex(filename, 3, 12);
    InsertNewRecordAtIndex(filename, 7, 24);
    InsertNewRecordAtIndex(filename, 10, 48);
    InsertNewRecordAtIndex(filename, 24, 60);
    InsertNewRecordAtIndex(filename, 14, 72);

    cout << "Index File Content After Insertions:" << endl;
    DisplayIndexFileContent(filename);

    InsertNewRecordAtIndex(filename, 19, 84);

    cout << "Index File Content After Split:" << endl;
    DisplayIndexFileContent(filename);

    DeleteRecordFromIndex(filename, 10);

    cout << "Index File Content After Deletion:" << endl;
    DisplayIndexFileContent(filename);

    int ref = SearchARecord(filename, 7);
    if (ref != -1)
        cout << "Record 7 found with reference: " << ref << endl;
    else
        cout << "Record 7 not found." << endl;
    
    DeleteRecordFromIndex(filename, 7);

    cout << "Index File Content After Deletion:" << endl;
    DisplayIndexFileContent(filename);

    int ref2 = SearchARecord(filename, 7);
    if (ref2 != -1)
        cout << "Record 7 found with reference: " << ref2 << endl;
    else
        cout << "Record 7 not found." << endl;

    return 0;
}
