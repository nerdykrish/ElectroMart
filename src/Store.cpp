// Store.cpp : code of Inventory and Bill classes (file handling is done here)
#include "Inventory.h"
#include "Bill.h"
#include <fstream>
#include <iomanip>
#include <ctime>

int Item::itemCount = 0;
int Bill::nextNumber = 1001;

// ---------------- Inventory ----------------
Inventory::Inventory(string file) : fileName(file) {}

Inventory::~Inventory() {
    for (Item *it : items)
        delete it;
}

void Inventory::addItem(Item *it) {
    if (find(it->getId()) != nullptr) {
        delete it;
        throw invalid_argument("An item with this ID already exists");
    }
    items.push_back(it);
}

void Inventory::addItem(const string &line) {
    vector<string> f;
    string part;
    stringstream ss(line);
    while (getline(ss, part, '|'))
        f.push_back(part);
    if (f.size() < 6)
        throw runtime_error("Bad record in file : " + line);

    int id = stoi(f[1]);
    double price = stod(f[3]);
    int stock = stoi(f[4]);
    if (f[0] == "R")
        addItem(new Resistor(id, f[2], price, stock, stod(f[5]), stod(f[6])));
    else if (f[0] == "C")
        addItem(new Capacitor(id, f[2], price, stock, stod(f[5]), stod(f[6])));
    else if (f[0] == "I")
        addItem(new IC(id, f[2], price, stock, stoi(f[5])));
    else if (f[0] == "M")
        addItem(new Microcontroller(id, f[2], price, stock, stoi(f[5]), stoi(f[6])));
    else
        throw runtime_error("Unknown item type in file");
}

Item *Inventory::find(int id) {
    for (Item *it : items)
        if (it->getId() == id)
            return it;
    return nullptr;
}

void Inventory::load() {
    ifstream fin(fileName);
    if (!fin) {     // first run : start with some default items
        addItem(new Resistor(101, "Carbon Resistor", 1.50, 200, 1000, 0.25));
        addItem(new Resistor(102, "Power Resistor", 12.00, 40, 10, 5));
        addItem(new Capacitor(201, "Electrolytic Cap", 4.00, 150, 100, 25));
        addItem(new Capacitor(202, "Ceramic Cap", 1.00, 300, 0.1, 50));
        addItem(new IC(301, "LM741 Op-Amp", 18.00, 60, 8));
        addItem(new IC(302, "NE555 Timer", 15.00, 8, 8));
        addItem(new Microcontroller(401, "ATmega328P", 245.00, 25, 28, 16));
        return;
    }
    string line;
    while (getline(fin, line))
        if (!line.empty())
            addItem(line);
}

void Inventory::save() const {
    ofstream fout(fileName);
    if (!fout)
        throw runtime_error("Could not open " + fileName + " for writing");
    for (Item *it : items)
        fout << it->toFileLine() << endl;
}

double Inventory::stockValue() const {
    double total = 0;
    for (Item *it : items)
        total += it->getPrice() * it->getStock();
    return total;
}

// ---------------- Bill ----------------
Bill::Bill(string c) : customer(c) {
    number = nextNumber++;
}

void Bill::addLine(Item &it, int qty) {
    it -= qty;      // throws StockException if stock is not enough
    for (Line &l : lines)
        if (l.name == it.name) {        // same item again : increase quantity
            l.qty += qty;
            return;
        }
    lines.push_back({it.name, qty, it.price, it.gstRate()});   // private data of Item
}

double Bill::subTotal() const {
    double t = 0;
    for (const Line &l : lines)
        t += l.amount();
    return t;
}

double Bill::gstTotal() const {
    double t = 0;
    for (const Line &l : lines)
        t += l.amount() * l.gst;
    return t;
}

string Bill::text() const {
    time_t now = time(nullptr);
    char date[40];
    strftime(date, sizeof(date), "%d-%m-%Y %H:%M", localtime(&now));

    ostringstream out;
    out << fixed << setprecision(2);
    out << "=============== ELECTROMART INVOICE ===============\n";
    out << "Bill No : " << number << "    Date : " << date << "\n";
    out << "Customer : " << customer << "\n";
    out << "---------------------------------------------------\n";
    out << left << setw(20) << "Item" << right << setw(5) << "Qty" << setw(9) << "Rate"
        << setw(6) << "GST" << setw(11) << "Amount" << "\n";
    for (const Line &l : lines)
        out << left << setw(20) << l.name << right << setw(5) << l.qty << setw(9) << l.rate
            << setw(5) << (int)(l.gst * 100) << "%" << setw(11) << l.amount() << "\n";
    out << "---------------------------------------------------\n";
    out << right << setw(40) << "Sub Total : " << setw(11) << subTotal() << "\n";
    out << setw(40) << "GST : " << setw(11) << gstTotal() << "\n";
    out << setw(40) << "Grand Total : " << setw(11) << grandTotal() << "\n";
    out << "===================================================\n";
    return out.str();
}

void Bill::saveToFile(const string &file) const {
    ofstream fout(file, ios::app);
    if (!fout)
        throw runtime_error("Could not open " + file);
    fout << text() << "\n";
}

int Bill::countBills(const string &file) {
    ifstream fin(file);
    string line;
    int n = 0;
    while (getline(fin, line))
        if (line.rfind("Bill No", 0) == 0)
            n++;
    return n;
}
