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

int findInsertAtPosition(const Node &node, int recordID)
{
    int pos = 0;

    if (node.nodeType == -1)
    {
        return pos;
    }

    while (pos < MAX_RECORDS - 2 && node.records[pos] != -1 && node.records[pos] <= recordID)
    {
        pos += 2;
    }
    return pos;
}

int findInsertParentPosition(const Node &node, int recordID)
{
    int pos = 0;

    while (pos < MAX_RECORDS - 2 && node.records[pos] != -1 && node.records[pos] <= recordID)
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
    if (i > 9)
        i = -1;
    head.records[0] = i;
    writeNode("index.bin", 0, head);
}

int getFreeNodeIndex()
{
    Node head = readNode("index.bin", 0);
    return head.records[0];
}

int InsertParent(const char *filename, int nodeIndex, int recordID, int reference)
{
    Node rootNode = readNode(filename, nodeIndex);
    int pos = 0;
    while (pos < MAX_RECORDS - 2 && rootNode.records[pos] != -1)
    {
        if (rootNode.records[pos] == recordID)
        {
            rootNode.records[pos + 1] = reference;
            writeNode(filename, nodeIndex, rootNode);
            return nodeIndex;
        }
        pos += 2;
    }
    if (nodeIndex == 1 && rootNode.records[MAX_RECORDS - 2] != -1) // Split the root node
    {
        vector<pair<int, int>> keys;
        for (int i = 0; i < MAX_RECORDS - 2; i += 2)
        {
            keys.push_back({rootNode.records[i], rootNode.records[i + 1]});
        }
        keys.push_back({recordID, reference});
        sort(keys.begin(), keys.end());

        int medianIndex = keys.size() / 2;

        Node newNodeLeft, newNodeRight;
        newNodeLeft.nodeType = 0;
        newNodeRight.nodeType = 0;

        for (int i = 0; i < medianIndex; i++) // 0 1 2 3 4 5 6 7 8 9 10 11
        {
            newNodeLeft.records[i * 2] = keys[i].first;
            newNodeLeft.records[i * 2 + 1] = keys[i].second;
        }

        for (int i = medianIndex; i < keys.size(); i++)
        {
            newNodeRight.records[(i - medianIndex) * 2] = keys[i].first;
            newNodeRight.records[(i - medianIndex) * 2 + 1] = keys[i].second;
        }

        int i = 0;
        while (i < MAX_RECORDS)
        {
            rootNode.records[i] = -1;
            i++;
        }

        int freeNodeIndex = getFreeNodeIndex();
        rootNode.records[0] = keys[medianIndex - 1].first;
        rootNode.records[1] = freeNodeIndex;
        rootNode.records[2] = keys[keys.size() - 1].first;
        rootNode.records[3] = freeNodeIndex + 1;
        rootNode.nodeType = 1;
        newNodeLeft.nodeType = 1;
        newNodeRight.nodeType = 1;
        writeNode(filename, nodeIndex, rootNode);
        writeNode(filename, freeNodeIndex, newNodeLeft);
        updateHead();
        writeNode(filename, freeNodeIndex + 1, newNodeRight);
        updateHead();
        return freeNodeIndex + 1;
    }
    pos = findInsertParentPosition(rootNode, recordID);

    for (int i = MAX_RECORDS - 2; i > pos; i -= 2)
    {
        rootNode.records[i] = rootNode.records[i - 2];
        rootNode.records[i + 1] = rootNode.records[i - 1];
    }

    rootNode.records[pos] = recordID;
    rootNode.records[pos + 1] = reference;

    writeNode(filename, nodeIndex, rootNode);

    return nodeIndex;
}

int InsertAt(const char *filename, int nodeIndex, int recordID, int reference, int parentIndex)
{
    Node rootNode = readNode(filename, nodeIndex);

    if (rootNode.nodeType == 0 || rootNode.nodeType == -1)
    {

        if (rootNode.nodeType == 0 && nodeIndex == 1 && rootNode.records[MAX_RECORDS - 2] != -1) // Split the root node
        {
            vector<pair<int, int>> keys;
            for (int i = 0; i < MAX_RECORDS - 2; i += 2)
            {
                keys.push_back({rootNode.records[i], rootNode.records[i + 1]});
            }
            keys.push_back({recordID, reference});
            sort(keys.begin(), keys.end());

            int medianIndex = keys.size() / 2;

            Node newNodeLeft, newNodeRight;
            newNodeLeft.nodeType = 0;
            newNodeRight.nodeType = 0;

            for (int i = 0; i < medianIndex; i++) // 0 1 2 3 4 5 6 7 8 9 10 11
            {
                newNodeLeft.records[i * 2] = keys[i].first;
                newNodeLeft.records[i * 2 + 1] = keys[i].second;
            }

            for (int i = medianIndex; i < keys.size(); i++)
            {
                newNodeRight.records[(i - medianIndex) * 2] = keys[i].first;
                newNodeRight.records[(i - medianIndex) * 2 + 1] = keys[i].second;
            }

            int i = 0;
            while (i < MAX_RECORDS)
            {
                rootNode.records[i] = -1;
                i++;
            }

            rootNode.records[0] = keys[medianIndex - 1].first;
            rootNode.records[1] = nodeIndex + 1;
            rootNode.records[2] = keys[keys.size() - 1].first;
            rootNode.records[3] = nodeIndex + 2;
            rootNode.nodeType = 1;
            writeNode(filename, nodeIndex, rootNode);
            writeNode(filename, getFreeNodeIndex(), newNodeLeft);
            updateHead();
            writeNode(filename, getFreeNodeIndex(), newNodeRight);
            updateHead();
            return nodeIndex + 2;
        }
        else if (rootNode.nodeType == 0 && rootNode.records[MAX_RECORDS - 2] != -1) // Split the leaf node
        {
            Node newNode;
            newNode.nodeType = 0;

            // Gather all the data and sort them
            vector<pair<int, int>> keys;
            for (int i = 0; i < MAX_RECORDS - 2; i += 2)
            {
                if (rootNode.records[i] != -1)
                {
                    keys.push_back({rootNode.records[i], rootNode.records[i + 1]});
                }
            }
            keys.push_back({recordID, reference});
            sort(keys.begin(), keys.end());

            // Determine the median index
            int medianIndex = keys.size() / 2;

            // Place the first half in the current rootNode
            for (int i = 0; i < medianIndex; i++)
            {
                rootNode.records[i * 2] = keys[i].first;
                rootNode.records[i * 2 + 1] = keys[i].second;
            }
            for (int i = medianIndex * 2; i < MAX_RECORDS - 1; i++)
            {
                rootNode.records[i] = -1;
            }

            // Place the second half in the new node
            for (int i = medianIndex; i < keys.size(); i++)
            {
                newNode.records[(i - medianIndex) * 2] = keys[i].first;
                newNode.records[(i - medianIndex) * 2 + 1] = keys[i].second;
            }

            InsertParent(filename, parentIndex, keys[keys.size() - 1].first, getFreeNodeIndex());
            // Update rootNode and write nodes back to the file
            writeNode(filename, nodeIndex, rootNode);
            writeNode(filename, getFreeNodeIndex(), newNode);
            updateHead();
            InsertParent(filename, parentIndex, keys[medianIndex - 1].first, nodeIndex);

            return nodeIndex;
        }
        // Node is a leaf, find the right position to insertAt
        int pos = findInsertAtPosition(rootNode, recordID);

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
        while (rootNode.records[i] != -1 && i < MAX_RECORDS - 2)
        {
            if (rootNode.records[i] > recordID)
            {
                childIndex = rootNode.records[i + 1]; // Return the reference (index + 1)
                break;
            }
            i += 2;
        }

        if (childIndex == -1)
        {
            // No child found, insertAt the record in the last child
            childIndex = rootNode.records[i - 1];
            rootNode.records[i - 2] = recordID;
            writeNode(filename, nodeIndex, rootNode);
        }

        return InsertAt(filename, childIndex, recordID, reference, nodeIndex);
    }

    return 1;
}

int InsertAtNewRecordAtIndex(const char *filename, int recordID, int reference)
{
    return InsertAt(filename, 1, recordID, reference, 0);
}

void DeleteRecordFromIndex(const char *filename, int recordID)
{
    for (int i = 1; i <= 9; i++)
    {
        Node node = readNode(filename, i);
        for (int j = 0; j < MAX_RECORDS - 2; j += 2)
        {
            if (node.records[j] == recordID)
            {
                if (node.nodeType == 0)
                {
                    int k;
                    for (k = j; k < MAX_RECORDS - 4; k += 2)
                    {
                        node.records[k] = node.records[k + 2];
                        node.records[k + 1] = node.records[k + 3];
                    }
                    node.records[k] = -1;
                    node.records[k + 1] = -1;
                    writeNode(filename, i, node);
                    return;
                }

                break;
            }
        }
    }
}

int SearchARecord(const char *filename, int recordID)
{
    int i = 1;
    while (true)
    {
        Node node = readNode(filename, i);
        for (int j = 0; j < MAX_RECORDS - 2; j += 2)
        {
            if (node.records[j] >= recordID)
            {
                if (node.nodeType == 0)
                {
                    return node.records[j + 1];
                }
                else
                {
                    i = node.records[j + 1];
                    break;
                }
            }
        }
    }
    return -1;
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
            cout << setw(3) << node.records[i] << " ";
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

    InsertAtNewRecordAtIndex(filename, 3, 12);
    InsertAtNewRecordAtIndex(filename, 7, 24);
    InsertAtNewRecordAtIndex(filename, 10, 48);
    InsertAtNewRecordAtIndex(filename, 24, 60);
    InsertAtNewRecordAtIndex(filename, 14, 72);

    cout << "Index File Content After Insertions:" << endl;
    DisplayIndexFileContent(filename);

    InsertAtNewRecordAtIndex(filename, 19, 84);

    cout << "Index File Content After Split leaf:" << endl;
    DisplayIndexFileContent(filename);

    InsertAtNewRecordAtIndex(filename, 30, 96);
    InsertAtNewRecordAtIndex(filename, 15, 108);
    InsertAtNewRecordAtIndex(filename, 1, 120);
    InsertAtNewRecordAtIndex(filename, 5, 132);

    cout << "Index File Content After Update root:" << endl;
    DisplayIndexFileContent(filename);

    InsertAtNewRecordAtIndex(filename, 2, 144);

    cout << "Index File Content After Split leaf:" << endl;
    DisplayIndexFileContent(filename);

    InsertAtNewRecordAtIndex(filename, 8, 156);
    InsertAtNewRecordAtIndex(filename, 9, 168);
    InsertAtNewRecordAtIndex(filename, 6, 180);
    InsertAtNewRecordAtIndex(filename, 11, 192);
    InsertAtNewRecordAtIndex(filename, 12, 204);
    InsertAtNewRecordAtIndex(filename, 17, 216);
    InsertAtNewRecordAtIndex(filename, 18, 228);

    cout << "Index File Content After Split:" << endl;
    DisplayIndexFileContent(filename);

    InsertAtNewRecordAtIndex(filename, 32, 240);

    cout << "Index File Content After Split Root:" << endl;
    DisplayIndexFileContent(filename);

    int ref = SearchARecord(filename, 32);
    if (ref != -1)
        cout << "Record 32 found with reference: " << ref << endl;
    else
        cout << "Record 32 not found." << endl;

    return 0;
}
