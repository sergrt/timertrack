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
            str = str.right(str.size() - str.indexOf("h") - 1);
            res += std::chrono::milliseconds{ s.toInt() * 60 * 60 * 1000 };
        }
        if (str.indexOf("m") != -1) {
            const auto s = str.mid(0, str.indexOf("m"));
            str = str.right(str.size() - str.indexOf("m") - 1);
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
        const auto m = strToInterval(i);
        if (m)
            res[*m] = i;
    }
    return res;
}

std::vector<std::chrono::milliseconds> patternToIntervals(const QString& entries) {
    const auto items = entries.split(",");
    std::vector<std::chrono::milliseconds> res;
    for (const auto& i : items) {
        const auto m = strToInterval(i);
        if (m)
            res.push_back(*m);
    }
    return res;
}

QString intervalToStr(std::chrono::milliseconds interval) {
    // format is hh:mm:ss
    auto s = interval.count() / 1000;
    auto hoursPart = s / 60 / 60;
    s = s - hoursPart * 60 * 60;
    auto minutesPart = s / 60;
    s = s - minutesPart * 60;
    const auto secondsPart = s;

    // Case - 1 hour represented as 60:00
    if (hoursPart == 1 && minutesPart == 0 && secondsPart == 0) {
        minutesPart = 60;
        hoursPart = 0;
    }

    QString res;
    if (hoursPart != 0)
        res += QString("%1:").arg(hoursPart, 2, 10, QChar('0'));

    res += QString("%1:%2").arg(minutesPart, 2, 10, QChar('0')).arg(secondsPart, 2, 10, QChar('0'));

    return res;
}
