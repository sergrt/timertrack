#pragma once
#include <optional>
#include <chrono>

bool validateTimeEntries(const QString& pattern);
std::optional<std::chrono::milliseconds> strToInterval(QString str);
std::map<std::chrono::milliseconds, QString> entriesToIntervals(const QString& entries);
std::vector<std::chrono::milliseconds> patternToIntervals(const QString& entries);
