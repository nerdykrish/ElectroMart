// MainWindow.cpp : builds the window and connects buttons to the store classes
#include "MainWindow.h"
#include <QtWidgets>
#include <fstream>

const string INVENTORY_FILE = "inventory.txt";
const string BILLS_FILE = "bills.txt";

static QString rs(double v) {
    return "Rs. " + QString::number(v, 'f', 2);
}

MainWindow::MainWindow() : inventory(INVENTORY_FILE) {
    setWindowTitle("ElectroMart : Electronic Components Store");
    resize(1100, 680);

    try {
        inventory.load();
    } catch (exception &e) {
        showError("File Error", e);
    }
    Bill::setNextNumber(1001 + Bill::countBills(BILLS_FILE));

    // coloured header at the top
    QLabel *title = new QLabel("ElectroMart");
    title->setStyleSheet("font-size: 26px; font-weight: bold; color: white;");
    QLabel *subtitle = new QLabel("Electronic Components Store Management System");
    subtitle->setStyleSheet("font-size: 13px; color: #dbe6ff;");
    summaryLabel = new QLabel;
    summaryLabel->setStyleSheet("font-size: 13px; color: white;");

    QVBoxLayout *titleBox = new QVBoxLayout;
    titleBox->addWidget(title);
    titleBox->addWidget(subtitle);
    QHBoxLayout *headerLayout = new QHBoxLayout;
    headerLayout->addLayout(titleBox);
    headerLayout->addStretch();
    headerLayout->addWidget(summaryLabel);
    QFrame *header = new QFrame;
    header->setObjectName("header");
    header->setLayout(headerLayout);
    header->setStyleSheet("QLabel { background: transparent; }"
                          "QFrame#header { background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
                          " stop:0 #1f3c88, stop:1 #3a7bd5); border-radius: 8px; }");

    QTabWidget *tabs = new QTabWidget;
    tabs->addTab(makeInventoryTab(), "Inventory");
    tabs->addTab(makeBillingTab(), "Billing");
    tabs->addTab(makeReportsTab(), "Reports");
    connect(tabs, &QTabWidget::currentChanged, this, [this]() { refreshReports(); });

    QWidget *central = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(central);
    layout->addWidget(header);
    layout->addWidget(tabs);
    setCentralWidget(central);

    setStyleSheet("QPushButton { background: #3a7bd5; color: white; border: none; padding: 7px 14px;"
                  " border-radius: 5px; font-weight: bold; }"
                  "QPushButton:hover { background: #1f3c88; }"
                  "QPushButton#green { background: #2e9d5b; }"
                  "QPushButton#red { background: #d64545; }"
                  "QGroupBox { font-weight: bold; border: 1px solid #c8d1e0; border-radius: 6px;"
                  " margin-top: 10px; padding-top: 8px; }"
                  "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 4px; }"
                  "QHeaderView::section { background: #e8eefb; color: #1f3c88; font-weight: bold; padding: 4px; border: none; }");

    refreshAll();
    statusBar()->showMessage(QString::number(inventory.all().size()) + " items loaded");
}

MainWindow::~MainWindow() {
    clearBill();        // give back stock of an unfinished bill
    try {
        inventory.save();
    } catch (...) {
    }
}

// ---------------- Inventory tab ----------------
QWidget *MainWindow::makeInventoryTab() {
    searchBox = new QLineEdit;
    searchBox->setPlaceholderText("Search by name or category...");
    connect(searchBox, &QLineEdit::textChanged, this, [this]() { refreshStockTable(); });

    stockTable = new QTableWidget(0, 7);
    stockTable->setHorizontalHeaderLabels({"ID", "Category", "Name", "Specs", "Price", "Stock", "GST"});
    stockTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    stockTable->verticalHeader()->hide();
    stockTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    stockTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    stockTable->setAlternatingRowColors(true);

    QVBoxLayout *left = new QVBoxLayout;
    left->addWidget(searchBox);
    left->addWidget(stockTable);

    // form to add a new item
    typeBox = new QComboBox;
    typeBox->addItems({"Resistor", "Capacitor", "IC", "Microcontroller"});
    idBox = new QSpinBox;
    idBox->setRange(1, 9999);
    idBox->setValue(103);
    nameBox = new QLineEdit;
    priceBox = new QDoubleSpinBox;
    priceBox->setRange(0.1, 100000);
    priceBox->setPrefix("Rs. ");
    stockBox = new QSpinBox;
    stockBox->setRange(0, 100000);
    value1Label = new QLabel;
    value2Label = new QLabel;
    value1Box = new QDoubleSpinBox;
    value1Box->setRange(0, 1e9);
    value1Box->setDecimals(2);
    value2Box = new QDoubleSpinBox;
    value2Box->setRange(0, 1e6);
    connect(typeBox, &QComboBox::currentIndexChanged, this, [this](int t) { changeType(t); });

    QFormLayout *form = new QFormLayout;
    form->addRow("Type :", typeBox);
    form->addRow("Item ID :", idBox);
    form->addRow("Name :", nameBox);
    form->addRow("Price :", priceBox);
    form->addRow("Stock :", stockBox);
    form->addRow(value1Label, value1Box);
    form->addRow(value2Label, value2Box);
    QPushButton *addBtn = new QPushButton("Add Item");
    addBtn->setObjectName("green");
    connect(addBtn, &QPushButton::clicked, this, [this]() { addNewItem(); });
    form->addRow(addBtn);
    QGroupBox *addGroup = new QGroupBox("Add New Item");
    addGroup->setLayout(form);
    changeType(0);

    // restock
    restockItemBox = new QComboBox;
    restockQtyBox = new QSpinBox;
    restockQtyBox->setRange(1, 10000);
    restockQtyBox->setValue(10);
    QPushButton *restockBtn = new QPushButton("Restock");
    connect(restockBtn, &QPushButton::clicked, this, [this]() { restockItem(); });
    QFormLayout *rform = new QFormLayout;
    rform->addRow("Item :", restockItemBox);
    rform->addRow("Quantity :", restockQtyBox);
    rform->addRow(restockBtn);
    QGroupBox *restockGroup = new QGroupBox("Restock Item");
    restockGroup->setLayout(rform);

    QVBoxLayout *right = new QVBoxLayout;
    right->addWidget(addGroup);
    right->addWidget(restockGroup);
    right->addStretch();
    QWidget *rightPanel = new QWidget;
    rightPanel->setLayout(right);
    rightPanel->setFixedWidth(320);

    QWidget *tab = new QWidget;
    QHBoxLayout *layout = new QHBoxLayout(tab);
    layout->addLayout(left);
    layout->addWidget(rightPanel);
    return tab;
}

// labels of the extra fields depend on the type of item
void MainWindow::changeType(int type) {
    const char *first[] = {"Resistance (ohm) :", "Capacitance (uF) :", "Pins :", "Pins :"};
    const char *second[] = {"Power (W) :", "Voltage (V) :", "", "Clock (MHz) :"};
    value1Label->setText(first[type]);
    value2Label->setText(second[type]);
    value1Box->setDecimals(type >= 2 ? 0 : 2);     // pins are whole numbers
    value2Box->setDecimals(type == 3 ? 0 : 2);
    value2Box->setVisible(type != 2);
    value2Label->setVisible(type != 2);
}

void MainWindow::addNewItem() {
    try {
        string name = nameBox->text().trimmed().toStdString();
        if (name.empty())
            throw invalid_argument("Please enter the name of the item");
        int id = idBox->value(), stock = stockBox->value();
        double price = priceBox->value(), a = value1Box->value(), b = value2Box->value();
        if (a <= 0)
            throw invalid_argument("Please enter " + value1Label->text().chopped(2).toStdString());

        switch (typeBox->currentIndex()) {
        case 0: inventory.addItem(new Resistor(id, name, price, stock, a, b)); break;
        case 1: inventory.addItem(new Capacitor(id, name, price, stock, a, b)); break;
        case 2: inventory.addItem(new IC(id, name, price, stock, (int)a)); break;
        default: inventory.addItem(new Microcontroller(id, name, price, stock, (int)a, (int)b));
        }
        inventory.save();
        nameBox->clear();
        idBox->setValue(id + 1);
        refreshAll();
        statusBar()->showMessage("Added " + QString::fromStdString(name));
    } catch (exception &e) {
        showError("Cannot Add Item", e);
    }
}

void MainWindow::restockItem() {
    try {
        Item *it = inventory.find(restockItemBox->currentData().toInt());
        if (it == nullptr)
            throw invalid_argument("Please select an item");
        *it += restockQtyBox->value();      // overloaded +=
        inventory.save();
        refreshAll();
        statusBar()->showMessage("New stock of " + QString::fromStdString(it->getName()) + " = " +
                                 QString::number(it->getStock()));
    } catch (exception &e) {
        showError("Restock Error", e);
    }
}

// ---------------- Billing tab ----------------
QWidget *MainWindow::makeBillingTab() {
    customerBox = new QLineEdit;
    customerBox->setPlaceholderText("Customer name");
    billItemBox = new QComboBox;
    billQtyBox = new QSpinBox;
    billQtyBox->setRange(1, 10000);
    QPushButton *addBtn = new QPushButton("Add to Bill");
    connect(addBtn, &QPushButton::clicked, this, [this]() { addToBill(); });

    QFormLayout *form = new QFormLayout;
    form->addRow("Customer :", customerBox);
    form->addRow("Item :", billItemBox);
    form->addRow("Quantity :", billQtyBox);
    form->addRow(addBtn);
    QGroupBox *itemGroup = new QGroupBox("New Bill");
    itemGroup->setLayout(form);

    cartTable = new QTableWidget(0, 5);
    cartTable->setHorizontalHeaderLabels({"Item", "Qty", "Rate", "GST", "Amount"});
    cartTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    cartTable->verticalHeader()->hide();
    cartTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    subLabel = new QLabel;
    gstLabel = new QLabel;
    totalLabel = new QLabel;
    totalLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #1f3c88;");
    QPushButton *genBtn = new QPushButton("Generate Bill");
    genBtn->setObjectName("green");
    connect(genBtn, &QPushButton::clicked, this, [this]() { generateBill(); });
    QPushButton *clearBtn = new QPushButton("Clear");
    clearBtn->setObjectName("red");
    connect(clearBtn, &QPushButton::clicked, this, [this]() {
        clearBill();
        refreshAll();
    });

    QHBoxLayout *totals = new QHBoxLayout;
    totals->addWidget(subLabel);
    totals->addWidget(gstLabel);
    totals->addStretch();
    totals->addWidget(totalLabel);
    QHBoxLayout *buttons = new QHBoxLayout;
    buttons->addStretch();
    buttons->addWidget(clearBtn);
    buttons->addWidget(genBtn);

    QVBoxLayout *left = new QVBoxLayout;
    left->addWidget(itemGroup);
    left->addWidget(cartTable);
    left->addLayout(totals);
    left->addLayout(buttons);

    invoiceView = new QPlainTextEdit;
    invoiceView->setReadOnly(true);
    invoiceView->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    invoiceView->setPlaceholderText("Invoice will appear here after Generate Bill");
    QVBoxLayout *right = new QVBoxLayout;
    right->addWidget(new QLabel("<b>Invoice Preview</b>"));
    right->addWidget(invoiceView);

    QWidget *tab = new QWidget;
    QHBoxLayout *layout = new QHBoxLayout(tab);
    layout->addLayout(left, 3);
    layout->addLayout(right, 2);
    return tab;
}

void MainWindow::addToBill() {
    try {
        if (customerBox->text().trimmed().isEmpty())
            throw invalid_argument("Please enter the customer name first");
        Item *it = inventory.find(billItemBox->currentData().toInt());
        if (it == nullptr)
            throw invalid_argument("Please select an item");
        if (bill == nullptr)
            bill = new Bill(customerBox->text().trimmed().toStdString());
        int qty = billQtyBox->value();
        bill->addLine(*it, qty);    // overloaded -= inside, may throw StockException
        billItems.push_back({it->getId(), qty});
        customerBox->setEnabled(false);
        refreshAll();
        statusBar()->showMessage("Added " + QString::number(qty) + " x " + QString::fromStdString(it->getName()));
    } catch (StockException &e) {
        showError("Stock Error", e);
    } catch (exception &e) {
        showError("Billing Error", e);
    }
}

void MainWindow::generateBill() {
    try {
        if (bill == nullptr || bill->empty())
            throw invalid_argument("Add at least one item to the bill");
        bill->saveToFile(BILLS_FILE);
        inventory.save();
        invoiceView->setPlainText(QString::fromStdString(bill->text()));
        QMessageBox::information(this, "Bill Saved", "Bill No. " + QString::number(bill->getNumber()) +
                                 " saved in bills.txt\nGrand Total : " + rs(bill->grandTotal()));
        delete bill;
        bill = nullptr;
        billItems.clear();
        customerBox->clear();
        customerBox->setEnabled(true);
        refreshAll();
    } catch (exception &e) {
        showError("Billing Error", e);
    }
}

// cancels the current bill and puts the stock back
void MainWindow::clearBill() {
    if (bill == nullptr)
        return;
    for (auto &p : billItems) {
        Item *it = inventory.find(p.first);
        if (it != nullptr)
            *it += p.second;
    }
    Bill::setNextNumber(bill->getNumber());
    delete bill;
    bill = nullptr;
    billItems.clear();
    customerBox->setEnabled(true);
}

// ---------------- Reports tab ----------------
QWidget *MainWindow::makeReportsTab() {
    limitBox = new QSpinBox;
    limitBox->setRange(1, 1000);
    limitBox->setValue(30);
    connect(limitBox, &QSpinBox::valueChanged, this, [this]() { refreshReports(); });
    lowStockList = new QListWidget;
    statsLabel = new QLabel;
    statsLabel->setStyleSheet("font-size: 14px;");

    QFormLayout *lform = new QFormLayout;
    lform->addRow("Show stock below :", limitBox);
    QVBoxLayout *lowBox = new QVBoxLayout;
    lowBox->addLayout(lform);
    lowBox->addWidget(lowStockList);
    QGroupBox *lowGroup = new QGroupBox("Low Stock Report");
    lowGroup->setLayout(lowBox);

    QVBoxLayout *statsBox = new QVBoxLayout;
    statsBox->addWidget(statsLabel);
    QGroupBox *statsGroup = new QGroupBox("Store Summary");
    statsGroup->setLayout(statsBox);

    QVBoxLayout *left = new QVBoxLayout;
    left->addWidget(statsGroup);
    left->addWidget(lowGroup);

    historyView = new QPlainTextEdit;
    historyView->setReadOnly(true);
    historyView->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    QVBoxLayout *right = new QVBoxLayout;
    right->addWidget(new QLabel("<b>Bill History (bills.txt)</b>"));
    right->addWidget(historyView);

    QWidget *tab = new QWidget;
    QHBoxLayout *layout = new QHBoxLayout(tab);
    layout->addLayout(left, 2);
    layout->addLayout(right, 3);
    return tab;
}

// ---------------- refresh helpers ----------------
void MainWindow::refreshAll() {
    refreshStockTable();
    refreshItemLists();
    refreshCart();
    refreshReports();
    summaryLabel->setText("Items : " + QString::number(inventory.all().size()) + "     Stock value : " +
                          rs(inventory.stockValue()) + "     Bills : " +
                          QString::number(Bill::countBills(BILLS_FILE)));
}

void MainWindow::refreshStockTable() {
    QString key = searchBox->text().trimmed();
    stockTable->setRowCount(0);
    for (Item *it : inventory.all()) {      // virtual functions called through Item*
        QString name = QString::fromStdString(it->getName());
        QString cat = QString::fromStdString(it->category());
        if (!key.isEmpty() && !name.contains(key, Qt::CaseInsensitive) && !cat.contains(key, Qt::CaseInsensitive))
            continue;
        int r = stockTable->rowCount();
        stockTable->insertRow(r);
        QStringList cells = {QString::number(it->getId()), cat, name, QString::fromStdString(it->specs()),
                             rs(it->getPrice()), QString::number(it->getStock()),
                             QString::number(it->gstRate() * 100) + " %"};
        for (int c = 0; c < cells.size(); c++) {
            QTableWidgetItem *cell = new QTableWidgetItem(cells[c]);
            if (it->getStock() < 20) {
                cell->setBackground(QColor("#ffe1e1"));     // low stock in light red
                cell->setForeground(QColor("#a31515"));
            }
            stockTable->setItem(r, c, cell);
        }
    }
}

void MainWindow::refreshItemLists() {
    int restockId = restockItemBox->currentData().toInt();
    int billId = billItemBox->currentData().toInt();
    restockItemBox->clear();
    billItemBox->clear();
    for (Item *it : inventory.all()) {
        QString text = QString::number(it->getId()) + " : " + QString::fromStdString(it->getName());
        restockItemBox->addItem(text, it->getId());
        billItemBox->addItem(text + "  (" + QString::number(it->getStock()) + " left)", it->getId());
    }
    if (restockId)
        restockItemBox->setCurrentIndex(restockItemBox->findData(restockId));
    if (billId)
        billItemBox->setCurrentIndex(billItemBox->findData(billId));
}

void MainWindow::refreshCart() {
    cartTable->setRowCount(0);
    double sub = 0, gst = 0, total = 0;
    if (bill != nullptr) {
        for (const Bill::Line &l : bill->getLines()) {
            int r = cartTable->rowCount();
            cartTable->insertRow(r);
            QStringList cells = {QString::fromStdString(l.name), QString::number(l.qty), rs(l.rate),
                                 QString::number(l.gst * 100) + " %", rs(l.amount())};
            for (int c = 0; c < cells.size(); c++)
                cartTable->setItem(r, c, new QTableWidgetItem(cells[c]));
        }
        sub = bill->subTotal();
        gst = bill->gstTotal();
        total = bill->grandTotal();
    }
    subLabel->setText("Sub Total : " + rs(sub));
    gstLabel->setText("   GST : " + rs(gst));
    totalLabel->setText("Grand Total : " + rs(total));
}

void MainWindow::refreshReports() {
    lowStockList->clear();
    for (Item *it : inventory.all())
        if (it->getStock() < limitBox->value())
            lowStockList->addItem(QString::fromStdString(it->getName()) + "  :  " +
                                  QString::number(it->getStock()) + " left");
    if (lowStockList->count() == 0)
        lowStockList->addItem("No item is below this limit");

    ifstream fin(BILLS_FILE);
    string all((istreambuf_iterator<char>(fin)), istreambuf_iterator<char>());
    historyView->setPlainText(all.empty() ? "No bills saved yet" : QString::fromStdString(all));

    statsLabel->setText("Total items : " + QString::number(inventory.all().size()) +
                        "<br>Stock value : " + rs(inventory.stockValue()) +
                        "<br>Bills saved : " + QString::number(Bill::countBills(BILLS_FILE)) +
                        "<br>Item objects in memory : " + QString::number(Item::getItemCount()));
}

void MainWindow::showError(const QString &title, const exception &e) {
    QMessageBox::warning(this, title, QString::fromStdString(e.what()));
    statusBar()->showMessage(title);
}
