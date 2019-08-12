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

    Settings();
    void load();
    void save() const;

    QString timerPattern() const;
    void setTimerPattern(const QString& pattern);

    std::set<FinishAction> finishActions() const;
    void updateFinishAction(FinishAction action, bool enabled);
    

    int defaultCategoryId() const;
    void setDefaultCategoryId(int id);

    QString contextMenuEntries() const;
    void setContextMenuEntries(const QString& entries);

    QString soundFileName() const;
    void setSoundFileName(const QString& fileName);

private:
    static const QString settingsFileName;

    QString timersPattern_;
    std::set<FinishAction> finishActions_;
    QString soundFileName_;
    int defaultCategoryId_ = 0;
    QString contextMenuEntries_;
};
