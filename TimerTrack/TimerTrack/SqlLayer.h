#pragma once
#include <QtSql>
#include "Category.h"

class SqlLayer {
public:
    SqlLayer();

    std::vector<Category> readCategories() const;
    void deleteCategory(int id) const;
    void addCategory(const Category& c) const;
private:
    QSqlDatabase database_ = QSqlDatabase::addDatabase("QSQLITE");
};

