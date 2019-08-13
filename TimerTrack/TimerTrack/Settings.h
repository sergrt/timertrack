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

    int width() const;
    int height() const;
    QString stylesheet() const;

    QPoint pos() const;
    void setPos(const QPoint& p);

    bool alwaysOnTop() const;

private:
    static const QString settingsFileName;

    QString timersPattern_;
    std::set<FinishAction> finishActions_;
    QString soundFileName_;
    int defaultCategoryId_ = 0;
    QString contextMenuEntries_;
    int width_ = 100;
    int height_ = 40;
    int posX_ = 0;
    int posY_ = 0;
    QString stylesheet_;
    bool alwaysOnTop_ = true;
    /*
    QString stylesheet_ = R"(
       * {
font-family: Arial;
font-size: 20pt;
}

QFrame{
    margin: 2px;
}
QLabel {
    font-family: Arial;
}
QToolButton {
    background-color: rgba( 255, 255, 255, 0% );
}
                        )";
    */
};
