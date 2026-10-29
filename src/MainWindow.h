// MainWindow.h : Qt GUI of ElectroMart
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Inventory.h"
#include "Bill.h"

class QTableWidget;
class QLineEdit;
class QComboBox;
class QSpinBox;
class QDoubleSpinBox;
class QLabel;
class QPlainTextEdit;
class QListWidget;

class MainWindow : public QMainWindow {
    Inventory inventory;
    Bill *bill = nullptr;                   // bill being made right now
    vector<pair<int, int>> billItems;       // (item id, qty) to undo on clear

    // inventory tab
    QTableWidget *stockTable;
    QLineEdit *searchBox;
    QComboBox *typeBox;
    QSpinBox *idBox, *stockBox;
    QLineEdit *nameBox;
    QDoubleSpinBox *priceBox, *value1Box, *value2Box;
    QLabel *value1Label, *value2Label, *summaryLabel;
    QComboBox *restockItemBox;
    QSpinBox *restockQtyBox;

    // billing tab
    QLineEdit *customerBox;
    QComboBox *billItemBox;
    QSpinBox *billQtyBox;
    QTableWidget *cartTable;
    QLabel *subLabel, *gstLabel, *totalLabel;
    QPlainTextEdit *invoiceView;

    // reports tab
    QSpinBox *limitBox;
    QListWidget *lowStockList;
    QLabel *statsLabel;
    QPlainTextEdit *historyView;

    QWidget *makeInventoryTab();
    QWidget *makeBillingTab();
    QWidget *makeReportsTab();

    void refreshAll();
    void refreshStockTable();
    void refreshItemLists();
    void refreshCart();
    void refreshReports();
    void changeType(int type);

    void addNewItem();
    void restockItem();
    void addToBill();
    void generateBill();
    void clearBill();
    void showError(const QString &title, const exception &e);

public:
    MainWindow();
    ~MainWindow();
};

#endif
