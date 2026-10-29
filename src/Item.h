// Item.h : all component classes of the store
// Abstract base class Item, hierarchical inheritance (Resistor, Capacitor, IC)
// and multilevel inheritance (Item -> IC -> Microcontroller)
#ifndef ITEM_H
#define ITEM_H

#include <stdexcept>
#include <string>
#include <sstream>
using namespace std;

// user defined exception for stock problems
class StockException : public runtime_error {
public:
    StockException(const string &msg) : runtime_error(msg) {}
};

class Item {
protected:
    int id;
    string name;
    double price;
    int stock;
    static int itemCount;           // number of Item objects in memory

public:
    Item() : id(0), name("NA"), price(0), stock(0) {
        itemCount++;
    }

    Item(int i, string n, double p, int s) : id(i), name(n), price(p), stock(s) {
        itemCount++;
    }

    virtual ~Item() {
        itemCount--;
    }

    // pure virtual functions make Item an abstract class
    virtual string category() const = 0;
    virtual string specs() const = 0;
    virtual string toFileLine() const = 0;

    // different GST for different categories (overridden in derived classes)
    virtual double gstRate() const {
        return 0.18;
    }

    int getId() const { return id; }
    string getName() const { return name; }
    double getPrice() const { return price; }
    int getStock() const { return stock; }

    // operator overloading to change the stock
    Item &operator+=(int qty) {
        if (qty <= 0)
            throw invalid_argument("Quantity must be more than zero");
        stock += qty;
        return *this;
    }

    Item &operator-=(int qty) {
        if (qty <= 0)
            throw invalid_argument("Quantity must be more than zero");
        if (qty > stock)
            throw StockException("Only " + to_string(stock) + " unit(s) of " + name + " are in stock");
        stock -= qty;
        return *this;
    }

    static int getItemCount() {
        return itemCount;
    }

    friend class Bill;      // Bill reads name and price directly
};

class Resistor : public Item {
    double ohms, watts;

public:
    Resistor(int i, string n, double p, int s, double o, double w)
        : Item(i, n, p, s), ohms(o), watts(w) {}

    string category() const { return "Resistor"; }

    string specs() const {
        ostringstream os;
        os << ohms << " ohm, " << watts << " W";
        return os.str();
    }

    double gstRate() const { return 0.12; }

    string toFileLine() const {
        ostringstream os;
        os << "R|" << id << "|" << name << "|" << price << "|" << stock << "|" << ohms << "|" << watts;
        return os.str();
    }
};

class Capacitor : public Item {
    double microFarad, volts;

public:
    Capacitor(int i, string n, double p, int s, double uf, double v)
        : Item(i, n, p, s), microFarad(uf), volts(v) {}

    string category() const { return "Capacitor"; }

    string specs() const {
        ostringstream os;
        os << microFarad << " uF, " << volts << " V";
        return os.str();
    }

    double gstRate() const { return 0.12; }

    string toFileLine() const {
        ostringstream os;
        os << "C|" << id << "|" << name << "|" << price << "|" << stock << "|" << microFarad << "|" << volts;
        return os.str();
    }
};

class IC : public Item {
protected:
    int pins;

public:
    IC(int i, string n, double p, int s, int pn) : Item(i, n, p, s), pins(pn) {}

    string category() const { return "IC"; }

    string specs() const { return to_string(pins) + " pins"; }

    string toFileLine() const {
        ostringstream os;
        os << "I|" << id << "|" << name << "|" << price << "|" << stock << "|" << pins;
        return os.str();
    }
};

class Microcontroller : public IC {
    int clockMHz;

public:
    Microcontroller(int i, string n, double p, int s, int pn, int mhz)
        : IC(i, n, p, s, pn), clockMHz(mhz) {}

    string category() const { return "Microcontroller"; }

    string specs() const { return to_string(pins) + " pins, " + to_string(clockMHz) + " MHz"; }

    string toFileLine() const {
        ostringstream os;
        os << "M|" << id << "|" << name << "|" << price << "|" << stock << "|" << pins << "|" << clockMHz;
        return os.str();
    }
};

#endif
