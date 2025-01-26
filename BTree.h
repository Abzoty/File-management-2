#ifndef FILE_MANAGEMENT_2_BTREE_H
#define FILE_MANAGEMENT_2_BTREE_H

// Forward declaration of BTree class template
template<class keyType>
class BTree;

#include <iostream>  // Assuming you need this for I/O (adjust as necessary)
#include "SimpleIndex.h"  // Assuming you're inheriting from this
#include "recordFile.h"
using namespace std;

template<class keyType>
class BTreeNode : public SimpleIndex<keyType> {
public:
    BTreeNode(int maxKeys, int unique = 1);

    int insert(const keyType key, int recAddr);

    int remove(const keyType key, int recAddr = -1);

    int split(BTreeNode<keyType> *newNode);


protected:
    int maxBKeys;

    //===================================
    int numKeys;  // Assuming this variable is used to store the number of keys
    keyType *keys;  // Assuming keys array
    int *RecAddrs;  // Assuming RecAddrs array
    //===================================

    int Init();

    friend class BTree<keyType>;

    void print(ostream &stream) const;
};

template<class keyType>
BTreeNode<keyType>::BTreeNode(int maxKeys, int unique) : SimpleIndex<keyType>(maxKeys + 1, unique) {
    Init();
}

template<class keyType>
int BTreeNode<keyType>::split(BTreeNode<keyType> *newNode) {
    if(numKeys < maxBKeys) return 0;
    int midpt = (numKeys + 1) / 2;
    int numNewKeys = numKeys - midpt;
    if(numNewKeys > newNode->maxBKeys)
    for (int i = midpt; i < numKeys; i++) {
        newNode->keys[i - midpt] = keys[i];
        newNode->RecAddrs[i - midpt] = RecAddrs[i];
    }
    newNode->numKeys = numNewKeys;
    numKeys = midpt;
    return 1;
}

template<class keyType>
int BTreeNode<keyType>::insert(const keyType key, int recAddr) {
    int result = SimpleIndex<keyType>::insert(key, recAddr);
    if (!result) return 0;
    if (numKeys > maxBKeys) return -1; //node overflow
    return 1;
}

template<class keyType>
int BTreeNode<keyType>::remove(const keyType key, int recAddr) {
    int result;
    result = SimpleIndex<keyType>::remove(key, recAddr);
    if (!result)return 0;
    return 1;
}

template<class keyType>
void BTreeNode<keyType>::print(ostream &stream) const {
    SimpleIndex<keyType>::print(cout);
}

//=================================================================================

template<class keyType>
class BTree {
public:
    BTree(int order, int keySize = sizeof(keyType), int unique = 1);

    int open(char *name, int mode);

    int create(char *name, int mode);

    int close();

    int insert(const keyType key, const int recAddr);

    int remove(const keyType key, const int recAddr = -1);

    int search(const keyType key, const int recAddr = -1);

protected:
    typedef BTreeNode<keyType> BTNode;

    BTNode *findLeaf(const keyType key);

    BTNode *fetch(const int recAddr);

    int store(BTNode *);

    BTNode root;
    int height;
    int order;
    BTNode **Nodes;
    recordFile<BTNode> BtreeFile;
};

template<class keyType>
int BTree<keyType>::search(const keyType key, const int recAddr) {
    BTreeNode<keyType> *leafNode;
    leafNode = findLeaf(key);
    return leafNode->search(key, recAddr);
}

template<class keyType>
BTreeNode<keyType> *BTree<keyType>::findLeaf(const keyType key) {
    int recAddr, level;
    for (level = 1; level < height; level++) {
        recAddr = Nodes[level - 1]->search(key, -1);
        Nodes[level] = fetch(recAddr);
    }
    return Nodes[height - 1];  // Assuming we return the final leaf node
}

#endif //FILE_MANAGEMENT_2_BTREE_H
