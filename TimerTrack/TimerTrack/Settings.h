#pragma once
#include <set>

class Settings final {
public:
    enum class FinishAction {
        Popup,
        Tooltip,
        Sound
    };
    static FinishAction finishActionFromString(const QString& str);
    static QString finishActionToString(FinishAction f);

    void load();
    void save() const;

    static bool validateTimerPattern(const QString& pattern);
    QString timerPattern() const;
    void setTimerPattern(const QString& pattern);

    std::set<FinishAction> finishActions() const;
    void updateFinishAction(FinishAction action, bool enabled);
    QString soundFileName() const;
    int defaultCategoryId() const;

private:
    static const QString settingsFileName;

    QString timersPattern_;
    std::set<FinishAction> finishActions_;
    QString soundFileName_;
    int defaultCategoryId_ = 0;
};
