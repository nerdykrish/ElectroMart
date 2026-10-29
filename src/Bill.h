// Bill.h : one customer bill (friend class of Item)
#ifndef BILL_H
#define BILL_H

#include "Item.h"
#include <vector>

class Bill {
public:
    struct Line {
        string name;
        int qty;
        double rate, gst;
        double amount() const { return qty * rate; }
    };

private:
    vector<Line> lines;
    string customer;
    int number;
    static int nextNumber;      // shared by all bills

public:
    Bill(string c);

    void addLine(Item &it, int qty);    // throws StockException
    bool empty() const { return lines.empty(); }
    const vector<Line> &getLines() const { return lines; }
    int getNumber() const { return number; }

    double subTotal() const;
    double gstTotal() const;
    double grandTotal() const { return subTotal() + gstTotal(); }

    string text() const;                        // printable invoice
    void saveToFile(const string &file) const;  // append to bills file

    static void setNextNumber(int n) { nextNumber = n; }
    static int countBills(const string &file);  // bills already saved
};

#endif
