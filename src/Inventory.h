// Inventory.h : keeps all items and saves / loads them from a text file
#ifndef INVENTORY_H
#define INVENTORY_H

#include "Item.h"
#include <vector>

class Inventory {
    vector<Item *> items;
    string fileName;

public:
    Inventory(string file);
    ~Inventory();

    // function overloading : add a ready object, or build one from a file line
    void addItem(Item *it);
    void addItem(const string &line);

    Item *find(int id);
    const vector<Item *> &all() const { return items; }

    void load();
    void save() const;
    double stockValue() const;
};

#endif
