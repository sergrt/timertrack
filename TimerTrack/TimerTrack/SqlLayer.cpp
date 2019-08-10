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
        c.id_ = result.value("Id").toInt();
        c.name_ = result.value("Name").toString();
        c.color_ = QColor(result.value("Color").toString());
        c.archived_ = result.value("Archived").toBool();
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

void SqlLayer::archiveCategory(int id) const {
    const QString query = QString(R"(UPDATE Categories SET Archived = "TRUE" WHERE id = %1)").arg(id);
    const auto result = database_.exec(query);
    if (result.lastError().isValid())
        throw std::runtime_error("Error archiving category");
}

bool SqlLayer::isCategoryUsed(int id) const {
    // Check if any record in the log uses category indicated by supplied 'id'.
    return true;
}

void SqlLayer::addCategory(const Category& c) const {
    const QString query = QString(R"(INSERT INTO Categories(Name, Color, Archived) VALUES ("%1", "%2", "FALSE"))").arg(c.name_).arg(c.color_.name());
    const auto result = database_.exec(query);
    if (result.lastError().isValid())
        throw std::runtime_error("Error adding category");
}
