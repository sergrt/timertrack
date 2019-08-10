#include "stdafx.h"
#include "Intervals.h"
#include <QRegExp>

static const auto regExpHms = QRegExp(R"(^(?!$)(\d+h)?(\d+m)?(\d+s)?$)");
static const auto regExpColon = QRegExp(R"(^\d{1,2}(:\d{1,2})?(:\d{1,2})?$)");

bool validateTimeEntries(const QString& pattern) {
    const auto parts = pattern.split(",");
    for (const auto& p : parts) {
        if (regExpHms.indexIn(p) == -1 && regExpColon.indexIn(p) == -1)
            return false;
    }
    return true;
}

std::optional<std::chrono::milliseconds> strToInterval(QString str) {
    if (regExpHms.indexIn(str) != -1) {
        std::chrono::milliseconds res{ 0 };
        if (str.indexOf("h") != -1) {
            const auto s = str.mid(0, str.indexOf("h"));
            str = str.right(str.indexOf("h") + 1);
            res += std::chrono::milliseconds{ s.toInt() * 60 * 60 * 1000 };
        }
        if (str.indexOf("m") != -1) {
            const auto s = str.mid(0, str.indexOf("m"));
            str = str.right(str.indexOf("m") + 1);
            res += std::chrono::milliseconds{ s.toInt() * 60 * 1000 };
        }
        if (str.indexOf("s") != -1) {
            const auto s = str.mid(0, str.indexOf("s"));
            res += std::chrono::milliseconds{ s.toInt() * 1000 };
        }
        return res;
    } else if (regExpColon.indexIn(str) != -1) {
        std::chrono::milliseconds res{ 0 };
        auto parts = str.split(":");
        if (!parts.empty()) {
            res += std::chrono::milliseconds{ parts[parts.size() - 1].toInt() * 1000 };
            parts.pop_back();
        }
        if (!parts.empty()) {
            res += std::chrono::milliseconds{ parts[parts.size() - 1].toInt() * 60 * 1000 };
            parts.pop_back();
        }
        if (!parts.empty()) {
            res += std::chrono::milliseconds{ parts[parts.size() - 1].toInt() * 60 * 60 * 1000 };
            parts.pop_back();
        }
        return res;
    }

    return std::nullopt;
}

std::map<std::chrono::milliseconds, QString> entriesToIntervals(const QString& entries) {
    const auto items = entries.split(",");
    std::map<std::chrono::milliseconds, QString> res;
    for (const auto& i : items) {
        if (validateTimeEntries(i)) {
            const auto m = strToInterval(i);
            if (m)
                res[*m] = i;
        }
    }
    return res;
}