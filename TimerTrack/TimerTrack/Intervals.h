#pragma once
#include <optional>
#include <chrono>

bool validateTimeEntries(const QString& sequence);
std::optional<std::chrono::milliseconds> strToInterval(QString str);
std::map<std::chrono::milliseconds, QString> entriesToIntervals(const QString& entries);
std::vector<std::chrono::milliseconds> sequenceToIntervals(const QString& entries);
QString intervalToStr(std::chrono::milliseconds interval);
