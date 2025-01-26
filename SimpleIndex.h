#ifndef FILE_MANAGEMENT_2_SIMPLEINDEX_H
#define FILE_MANAGEMENT_2_SIMPLEINDEX_H


#include <iostream>
#include <vector>
#include <algorithm> // For std::lower_bound

template<class keyType>
class SimpleIndex {
public:
    SimpleIndex(int unique = 1);

    virtual int insert(const keyType &key, int recAddr);

    virtual int search(const keyType &key, int recAddr = -1) const;

    virtual int remove(const keyType &key, int recAddr = -1);

protected:
    std::vector<keyType> keys;        // Vector of keys
    std::vector<int> recAddrs;        // Vector of record addresses
    int unique;                       // Whether the index should enforce unique keys
};

template<class keyType>
SimpleIndex<keyType>::SimpleIndex(int unique) : unique(unique) {}

// Insert key and associated record address
template<class keyType>
int SimpleIndex<keyType>::insert(const keyType &key, int recAddr) {
    // Check if the key already exists
    if (unique) {
        if (std::find(keys.begin(), keys.end(), key) != keys.end()) {
            return 0; // Key already exists and unique constraint is enforced
        }
    }

    // Insert the key and record address at the appropriate position (sorted order)
    auto pos = std::lower_bound(keys.begin(), keys.end(), key);
    int index = pos - keys.begin();
    keys.insert(pos, key);
    recAddrs.insert(recAddrs.begin() + index, recAddr);

    return 1;
}

// Search for the key and return the associated record address (or recAddr if given)
template<class keyType>
int SimpleIndex<keyType>::search(const keyType &key, int recAddr) const {
    auto pos = std::lower_bound(keys.begin(), keys.end(), key);

    if (pos != keys.end() && *pos == key) {
        int index = pos - keys.begin();
        if (recAddr == -1 || recAddrs[index] == recAddr) {
            return recAddrs[index];
        }
    }
    return -1;  // Key not found
}

// Remove the key and its associated record address
template<class keyType>
int SimpleIndex<keyType>::remove(const keyType &key, int recAddr) {
    auto pos = std::lower_bound(keys.begin(), keys.end(), key);

    if (pos != keys.end() && *pos == key) {
        int index = pos - keys.begin();
        if (recAddr == -1 || recAddrs[index] == recAddr) {
            keys.erase(pos);
            recAddrs.erase(recAddrs.begin() + index);
            return 1;
        }
    }
    return 0;  // Key not found
}



#endif //FILE_MANAGEMENT_2_SIMPLEINDEX_H
