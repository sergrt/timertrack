#include "stdafx.h"
#include "SqlLayer.h"


SqlLayer::SqlLayer() {
    database_.setDatabaseName("statistics.sqlite");
    if (!database_.open())
        throw std::runtime_error("Error open database");
}

std::vector<Category> SqlLayer::readCategories() const {
    static const QString query = "SELECT * FROM Categories ORDER BY id";
    auto result = database_.exec(query);

    if (result.lastError().isValid()) {
        throw std::runtime_error("Error obtaining categories");
    }
    std::vector<Category> res;
    while (result.next()) {
        Category c;
        c.id_ = result.value("id").toInt();
        c.name_ = result.value("name").toString();
        c.color_ = QColor(result.value("color").toString());
        res.push_back(std::move(c));
    }
    
    return res;
}

void SqlLayer::deleteCategory(int id) const {
    const QString query = QString("DELETE FROM Categories WHERE id = %1").arg(id);
    const auto result = database_.exec(query);
    if (result.lastError().isValid())
        throw std::runtime_error("Error deleting category");
}
void SqlLayer::addCategory(const Category& c) const {
    const QString query = QString(R"(INSERT INTO Categories(Name, Color) VALUES ("%1", "%2"))").arg(c.name_).arg(c.color_.name());
    const auto result = database_.exec(query);
    if (result.lastError().isValid())
        throw std::runtime_error("Error adding category");
}
