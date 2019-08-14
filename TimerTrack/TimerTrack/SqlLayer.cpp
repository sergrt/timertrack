#include "stdafx.h"
#include "SqlLayer.h"
#include "Record.h"

const auto databaseName = QString("timertrack.sqlite");

extern const QString restingCategoryRole = "Resting";
extern const QString defaultCategoryRole = "Default";
extern const int pomodoroStatusCompleted = 1;
extern const int pomodoroStatusInterrupted = 2;

SqlLayer::SqlLayer() {
    bool needCreateStructure = !QFile::exists(databaseName);
    database_.setDatabaseName(databaseName);
    if (!database_.open())
        throw std::runtime_error("Error open database");

    if (needCreateStructure)
        createDatabase();
}

void SqlLayer::createDatabase() {
    static const auto createQuery = QString(R"(
DROP TABLE IF EXISTS Categories;
CREATE TABLE Categories (Id INTEGER PRIMARY KEY ASC AUTOINCREMENT NOT NULL, Name varchar NOT NULL, Color VARCHAR NOT NULL, Archived BOOLEAN NOT NULL, Role VARCHAR);
INSERT INTO Categories (Id, Name, Color, Archived, Role) VALUES (0, 'Resting', '#4c4c4c', 'False', 'Resting');
INSERT INTO Categories (Id, Name, Color, Archived, Role) VALUES (1, 'General', '#ff915a', 'False', 'Default');

DROP TABLE IF EXISTS Records;
CREATE TABLE Records (Id INTEGER PRIMARY KEY NOT NULL, Category INTEGER NOT NULL REFERENCES Categories (Id), Status INTEGER NOT NULL REFERENCES RecordStatuses (Id), StartTime DATETIME NOT NULL, PlannedTime INTEGER NOT NULL, PassedTime INTEGER NOT NULL);

DROP TABLE IF EXISTS RecordStatuses;
CREATE TABLE RecordStatuses (Id INTEGER PRIMARY KEY NOT NULL, Name VARCHAR NOT NULL);
INSERT INTO RecordStatuses (Id, Name) VALUES (0, 'Started');
INSERT INTO RecordStatuses (Id, Name) VALUES (1, 'Finished');
INSERT INTO RecordStatuses (Id, Name) VALUES (2, 'Interrupted');
    )");

    const auto statements = createQuery.split("\n");
    for (auto statement : statements) {
        statement = statement.trimmed();
        if (statement.isEmpty())
            continue;
        const auto result = database_.exec(statement);
        if (result.lastError().isValid()) {
            qDebug(("Error creating database: " + result.lastError().text()).toStdString().c_str());
            throw std::runtime_error(("Error creating database: " + result.lastError().text()).toStdString());
        }
    }
}


std::vector<Category> SqlLayer::readCategories() const {
    static const auto query = QString("SELECT * FROM Categories ORDER BY id");
    auto result = database_.exec(query);

    if (result.lastError().isValid())
        throw std::runtime_error("Error obtaining categories");

    std::vector<Category> res;
    while (result.next()) {
        Category c;
        c.id_ = result.value("Id").toInt();
        c.name_ = result.value("Name").toString();
        c.color_ = QColor(result.value("Color").toString());
        c.archived_ = result.value("Archived").toBool();
        c.role_ = Category::roleFromStr(result.value("Role").toString());
        res.push_back(std::move(c));
    }
    
    return res;
}

void SqlLayer::deleteCategory(int id) const {
    const auto query = QString("DELETE FROM Categories WHERE id = %1").arg(id);
    const auto result = database_.exec(query);
    if (result.lastError().isValid())
        throw std::runtime_error("Error deleting category");
}

void SqlLayer::archiveCategory(int id) const {
    const auto query = QString(R"(UPDATE Categories SET Archived = "TRUE" WHERE id = %1)").arg(id);
    const auto result = database_.exec(query);
    if (result.lastError().isValid())
        throw std::runtime_error("Error archiving category");
}

void SqlLayer::unarchiveCategory(int id) const {
    const auto query = QString(R"(UPDATE Categories SET Archived = "FALSE" WHERE id = %1)").arg(id);
    const auto result = database_.exec(query);
    if (result.lastError().isValid())
        throw std::runtime_error("Error unarchiving category");
}

bool SqlLayer::isCategoryUsed(int id) const {
    // Check if any record in the log uses category indicated by supplied 'id'.
    const auto query = QString(R"(SELECT count(*) as Count FROM Records WHERE Category = %1)").arg(id);
    auto result = database_.exec(query);
    if (result.lastError().isValid())
        throw std::runtime_error("Error checking category usage");

    while (result.next())
        return result.value("Count").toInt() != 0;

    assert("Cannot be here");
    return false;
}

bool SqlLayer::isCategoryArchived(int id) const {
    const auto query = QString(R"(SELECT * FROM Categories WHERE Id = %1)").arg(id);
    auto result = database_.exec(query);
    if (result.lastError().isValid())
        throw std::runtime_error("Error checking category usage");

    while (result.next())
        return result.value("Archived").toBool() == true;

    assert("Cannot be here");
    return false;
}

int SqlLayer::addCategory(const Category& c) const {
    const auto query = QString(R"(INSERT INTO Categories(Name, Color, Archived) VALUES ("%1", "%2", "FALSE"))").arg(c.name_).arg(c.color_.name());
    const auto result = database_.exec(query);
    if (result.lastError().isValid())
        throw std::runtime_error("Error adding category");

    return result.lastInsertId().toInt();
}

void SqlLayer::updateCategory(int id, const QString& name, const QColor& color) const {
    const auto query = QString(R"(UPDATE Categories SET Name = "%1", Color = "%2" WHERE Id = %3)").arg(name).arg(color.name()).arg(id);
    const auto result = database_.exec(query);
    if (result.lastError().isValid())
        throw std::runtime_error("Error updating category");
}

QString toSqlTime(const std::chrono::time_point<std::chrono::system_clock>& timePoint) {
    const auto now = std::chrono::system_clock::to_time_t(timePoint);
    return QString::fromStdString(std::to_string(now));
}

std::chrono::time_point<std::chrono::system_clock> fromSqlTime(int timePoint) {
    return std::chrono::system_clock::from_time_t(timePoint);
}

int SqlLayer::addRecord(const Record& r) const {
    const auto query = QString(R"(INSERT INTO Records(Category, Status, StartTime, PlannedTime, PassedTime)
                                     VALUES ("%1", "%2", "%3", "%4", "%5"))")
        .arg(r.category_)
        .arg(r.status_)
        .arg(toSqlTime(r.startTime_))
        .arg(r.plannedTime_.count() / 1000)
        .arg(r.passedTime_.count() / 1000);

    const auto result = database_.exec(query);

    if (result.lastError().isValid())
        throw std::runtime_error("Error adding record");

    return result.lastInsertId().toInt();
}

std::vector<Record> SqlLayer::readRecords() const {
    static const auto query = QString("SELECT * FROM Records ORDER BY id");
    auto result = database_.exec(query);

    if (result.lastError().isValid())
        throw std::runtime_error("Error obtaining records");

    std::vector<Record> res;
    while (result.next()) {
        Record r;
        r.id_ = result.value("Id").toInt();
        r.category_ = result.value("Category").toInt();
        r.status_ = static_cast<Record::Status>(result.value("Status").toInt());
        r.startTime_ = fromSqlTime(result.value("StartTime").toInt());
        r.plannedTime_ = std::chrono::milliseconds{ result.value("PlannedTime").toInt() };
        r.passedTime_ = std::chrono::milliseconds{ result.value("PassedTime").toInt() };

        res.push_back(std::move(r));
    }

    return res;
}

void SqlLayer::finishRecord(int id) const {
    const auto query = QString(R"(UPDATE Records SET Status = %1, PassedTime = PlannedTime WHERE id = %2)").arg(Record::Status::Finished).arg(id);
    const auto result = database_.exec(query);
    if (result.lastError().isValid())
        throw std::runtime_error("Error finishing record");
}

void SqlLayer::interruptRecord(int id) const {
    const auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    const auto query = QString(R"(UPDATE Records SET Status = %1, PassedTime = %2 - StartTime WHERE id = %3)")
        .arg(Record::Status::Interrupted)
        .arg(now)
        .arg(id);
    const auto result = database_.exec(query);
    if (result.lastError().isValid())
        throw std::runtime_error("Error finishing record");
}

int SqlLayer::restingCategoryId() const {
    static const auto query = QString(R"(SELECT Id FROM Categories WHERE Role = "%1")").arg(restingCategoryRole);
    auto result = database_.exec(query);
    if (result.lastError().isValid())
        throw std::runtime_error("Error obtaining resting category id");

    while (result.next())
        return result.value("Id").toInt();

    assert("Cannot be here");
    return 0;
}

bool SqlLayer::isCategoryPersistent(int id) const {
    const auto query =
        QString(R"(SELECT
                     CASE WHEN Count(*) > 0 THEN TRUE ELSE FALSE END
                   AS IsPersistent
                   FROM Categories WHERE (Role = "%1" OR Role = "%2") AND Id = %3)")
        .arg(restingCategoryRole)
        .arg(defaultCategoryRole)
        .arg(id);

    auto result = database_.exec(query);
    if (result.lastError().isValid())
        throw std::runtime_error("Error getting whether category is persistent");

    while (result.next())
        return result.value("IsPersistent").toBool();

    assert("Cannot be here");
    return true;
}

std::vector<std::pair<SecondsSinceEpoch, int>> SqlLayer::getCompletedRecords(const std::vector<int>& categories,
                                                                             const QDateTime& from,
                                                                             const QDateTime& till) const {
    std::vector<std::pair<SecondsSinceEpoch, int>> res;

    if (categories.empty())
        return res;

    auto categoriesStr = QString();
    for (const auto c : categories)
        categoriesStr += QString("%1").arg(c) + ",";

    categoriesStr.remove(categoriesStr.size() - 1, 1);

    const auto query =
        QString(R"(SELECT
                      Count(*) as Count,
                      StartTime as Date
                  FROM Records
                  WHERE StartTime >= '%1' AND StartTime <='%2'
                  AND Category IN (%3)
                  GROUP BY strftime('%d-%m-%Y', StartTime, 'unixepoch')
                  ORDER BY Date)")
        .arg(from.toSecsSinceEpoch())
        .arg(till.toSecsSinceEpoch())
        .arg(categoriesStr);

    auto result = database_.exec(query);
    if (result.lastError().isValid())
        throw std::runtime_error("Error getting statistics data");

    while (result.next())
        res.emplace_back(result.value("Date").toLongLong(), result.value("Count").toInt());

    return res;
}

int SqlLayer::getCompletedRecordsCount(const std::vector<int>& categories,
                                       const QDateTime& from,
                                       const QDateTime& till,
                                       int recordStatus) const {
    if (categories.empty())
        return 0;

    auto categoriesStr = QString();
    for (const auto c : categories)
        categoriesStr += QString("%1").arg(c) + ",";

    categoriesStr.remove(categoriesStr.size() - 1, 1);

    const auto query =
        QString(R"(SELECT
                      Count(*) as Count
                  FROM Records
                  WHERE StartTime >= '%1' AND StartTime <='%2'
                  AND Category IN (%3)
                  AND Status = %4)")
        .arg(from.toSecsSinceEpoch())
        .arg(till.toSecsSinceEpoch())
        .arg(categoriesStr)
        .arg(recordStatus);

    int res{0};

    auto result = database_.exec(query);
    if (result.lastError().isValid())
        throw std::runtime_error("Error getting records count");

    while (result.next())
        res = result.value("Count").toInt();

    return res;
}