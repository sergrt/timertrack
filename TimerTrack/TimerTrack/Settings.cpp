#include "stdafx.h"
#include "Settings.h"
#include "Intervals.h"

const QString Settings::settingsFileName{ "settings.ini" };
/*
static const auto defaultStyle = QString(R"(
* {
    font-family: Arial;
    font-size: 16px;
}
QFrame {
    margin: 1px;
}
QLabel {
    font-family: Arial;
}
QToolButton{
    font-size: 10px;
    color: gray;
    background-color: rgba(255, 255, 255, 0%);
}
)");
*/
static const auto defaultStyle = QString(R"(
* {
    font-family: Arial;
    font-size: 16px;
}
QFrame {
    margin: 1px;
}
QLabel {
    font-family: Arial;
}
QToolButton{
    font-size: 10px;
    color: gray;
    background-color: rgba(255, 255, 255, 0%);
}
)");

namespace IniFile {
// Section names
static const QString Timer{ "Timer" };
static const QString ContextMenu{ "ContextMenu" };
static const QString Window{ "Window" };

// Section structure
struct Timer {
    static const QString TimerPattern;
    static const QString FinishActions;
    static const QString SoundFileName;
    static const QString DefaultCategoryId;
};

struct ContextMenu {
    static const QString ContextMenuEntries;
};

struct Window {
    static const QString Width;
    static const QString Height;
    static const QString PosX;
    static const QString PosY;
    static const QString StyleSheet;
    static const QString AlwaysOnTop;
};

// Data structures
struct FinishActions {
    static const QString Popup;
    static const QString Tooltip;
    static const QString Sound;
    static const QString Delimiter;
};

// Definitions
const QString Timer::TimerPattern{ "TimerPattern" };
const QString Timer::FinishActions{ "FinishActions" };
const QString Timer::SoundFileName{ "SoundFileName" };
const QString Timer::DefaultCategoryId{ "DefaultCategoryId" };

const QString ContextMenu::ContextMenuEntries{ "ContextMenuEntries" };

const QString Window::Width{ "Width" };
const QString Window::Height{ "Height" };
const QString Window::PosX{ "PosX" };
const QString Window::PosY{ "PosY" };
const QString Window::StyleSheet{ "StyleSheet" };
const QString Window::AlwaysOnTop{ "AlwaysOnTop" };

const QString FinishActions::Popup{ "Popup" };
const QString FinishActions::Tooltip{ "Tooltip" };
const QString FinishActions::Sound{ "Sound" };
const QString FinishActions::Delimiter{","};

}


Settings::FinishAction Settings::finishActionFromString(const QString& str) {
    if (str == IniFile::FinishActions::Popup)
        return FinishAction::Popup;
    else if (str == IniFile::FinishActions::Tooltip)
        return FinishAction::Tooltip;
    else if (str == IniFile::FinishActions::Sound)
        return FinishAction::Sound;

    throw std::runtime_error("Error converting FinishAction string value");
}

QString Settings::finishActionToString(FinishAction f) {
    if (f == FinishAction::Popup)
        return IniFile::FinishActions::Popup;
    else if (f == FinishAction::Tooltip)
        return IniFile::FinishActions::Tooltip;
    else if (f == FinishAction::Sound)
        return IniFile::FinishActions::Sound;

    throw std::runtime_error("Error converting string to FinishAction");
}

Settings::Settings() {
    load();
}

void Settings::load() {
    QSettings settings(settingsFileName, QSettings::Format::IniFormat);

    settings.beginGroup(IniFile::Timer);
    const auto timersPattern = settings.value(IniFile::Timer::TimerPattern).toString();
    if (validateTimeEntries(timersPattern))
        timersPattern_ = timersPattern;

    finishActions_.clear();
    const auto finishActionsStr = settings.value(IniFile::Timer::FinishActions).toString();
    if (!finishActionsStr.isEmpty()) {
        const auto parts = finishActionsStr.split(IniFile::FinishActions::Delimiter);
        for (const auto& item : parts)
            finishActions_.insert(finishActionFromString(item));
    }
    soundFileName_ = settings.value(IniFile::Timer::SoundFileName).toString();
    defaultCategoryId_ = settings.value(IniFile::Timer::DefaultCategoryId, QVariant::fromValue(0)).toInt();
    settings.endGroup();

    settings.beginGroup(IniFile::ContextMenu);
    const auto contextMenuEntries = settings.value(IniFile::ContextMenu::ContextMenuEntries).toString();
    if (validateTimeEntries(contextMenuEntries))
        contextMenuEntries_ = contextMenuEntries;
    settings.endGroup();

    settings.beginGroup(IniFile::Window);
    width_ = settings.value(IniFile::Window::Width).toInt();
    height_ = settings.value(IniFile::Window::Height).toInt();
    posX_ = settings.value(IniFile::Window::PosX).toInt();
    posY_ = settings.value(IniFile::Window::PosY).toInt();
    stylesheet_ = settings.value(IniFile::Window::StyleSheet).toString();
    alwaysOnTop_ = settings.value(IniFile::Window::AlwaysOnTop).toBool();
    settings.endGroup();
}

void Settings::save() const {
    QSettings settings(settingsFileName, QSettings::Format::IniFormat);

    settings.beginGroup(IniFile::Timer);
    settings.setValue(IniFile::Timer::TimerPattern, timersPattern_);
    QString finishActionsStr;
    for (const auto& f : finishActions_)
        finishActionsStr += finishActionToString(f) + IniFile::FinishActions::Delimiter;

    if (finishActionsStr.endsWith(IniFile::FinishActions::Delimiter))
        finishActionsStr.remove(finishActionsStr.size() - 1, 1);

    settings.setValue(IniFile::Timer::FinishActions, finishActionsStr);
    settings.setValue(IniFile::Timer::SoundFileName, soundFileName_);
    settings.setValue(IniFile::Timer::DefaultCategoryId, defaultCategoryId_);
    settings.endGroup();

    settings.beginGroup(IniFile::ContextMenu);
    settings.setValue(IniFile::ContextMenu::ContextMenuEntries, contextMenuEntries_);
    settings.endGroup();

    settings.beginGroup(IniFile::Window);
    settings.setValue(IniFile::Window::PosX, posX_);
    settings.setValue(IniFile::Window::PosY, posY_);
    settings.endGroup();
}

QString Settings::timerPattern() const {
    return timersPattern_;
}

void Settings::setTimerPattern(const QString& pattern) {
    timersPattern_ = pattern;
    save();
}

std::set<Settings::FinishAction> Settings::finishActions() const {
    return finishActions_;
}

void Settings::updateFinishAction(FinishAction action, bool enabled) {
    if (enabled)
        finishActions_.insert(action);
    else if (finishActions_.find(action) != finishActions_.end())
        finishActions_.erase(action);
    save();
}

QString Settings::soundFileName() const {
    return soundFileName_;
}

void Settings::setSoundFileName(const QString& fileName) {
    soundFileName_ = fileName;
    save();
}

int Settings::defaultCategoryId() const {
    return defaultCategoryId_;
}

void Settings::setDefaultCategoryId(int id) {
    defaultCategoryId_ = id;
    save();
}

QString Settings::contextMenuEntries() const {
    return contextMenuEntries_;
}

void Settings::setContextMenuEntries(const QString& entries) {
    contextMenuEntries_ = entries;
    save();
}

int Settings::width() const {
    return width_;
}

int Settings::height() const {
    return height_;
}

QString Settings::stylesheet() const {
    //return stylesheet_;
    return defaultStyle;
}

QPoint Settings::pos() const {
    return QPoint(posX_, posY_);
}

void Settings::setPos(const QPoint& p) {
    posX_ = p.x();
    posY_ = p.y();
    save();
}

bool Settings::alwaysOnTop() const {
    return alwaysOnTop_;
}
