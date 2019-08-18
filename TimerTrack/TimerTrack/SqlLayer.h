#pragma once
#include <QtSql>
#include "Category.h"

struct Record;

//extern const QString restingCategoryRole;
//extern const QString defaultCategoryRole;

using SecondsSinceEpoch = qint64;

class SqlLayer {
public:
    SqlLayer();

    std::vector<Category> readCategories() const;
    Category getCategory(int id) const;
    void deleteCategory(int id) const;
    void archiveCategory(int id) const;
    void unarchiveCategory(int id) const;
    int addCategory(const Category& c) const;
    bool isCategoryUsed(int id) const;
    bool isCategoryArchived(int id) const;
    int addRecord(const Record& r) const;
    std::vector<Record> readRecords() const;
    void finishRecord(int id) const;
    void interruptRecord(int id) const;
    void updateCategory(int id, const QString& name, const QColor& color) const;
    int restingCategoryId() const;
    bool isCategoryPersistent(int id) const;

    std::vector<std::pair<SecondsSinceEpoch, int>> getCompletedRecords(const std::vector<int>& categories,
                                                                       const QDateTime& from,
                                                                       const QDateTime& till) const;
    int getRecordsStats(const std::vector<int>& categories,
                        const QDateTime& from,
                        const QDateTime& till,
                        int recordStatus,
                        bool inMinutes) const;

    void createDatabase() const;
private:
    QSqlDatabase database_ = QSqlDatabase::addDatabase("QSQLITE");
};

