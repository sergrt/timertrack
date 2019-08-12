#pragma once
#include <QtSql>
#include "Category.h"

class Record;

class SqlLayer {
public:
    SqlLayer();

    std::vector<Category> readCategories() const;
    void deleteCategory(int id) const;
    void archiveCategory(int id) const;
    void unarchiveCategory(int id) const;
    void addCategory(const Category& c) const;
    bool isCategoryUsed(int id) const;
    bool isCategoryArchived(int id) const;
    int addRecord(const Record& r);
    std::vector<Record> readRecords() const;
    void finishRecord(int id) const;
    void interruptRecord(int id) const;
    bool checkIdIsLastNotArchived(int id) const;
    void updateCategory(int id, const QString& name, const QColor& color) const;
private:
    QSqlDatabase database_ = QSqlDatabase::addDatabase("QSQLITE");
};

