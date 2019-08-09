#include "stdafx.h"
#include "Settings.h"

const QString Settings::settingsFileName{ "settings.ini" };

namespace IniFile {
// Section names
static const QString Timer{ "Timer" };

// Section structure
struct Timer {
    static const QString TimerPattern;
    static const QString FinishActions;
    static const QString SoundFileName;
    static const QString DefaultCategoryId;
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

const QString FinishActions::Popup{ "Popup" };
const QString FinishActions::Tooltip{ "Tooltip" };
const QString FinishActions::Sound{ "Sound" };
const QString FinishActions::Delimiter;

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

void Settings::load() {
    QSettings settings(settingsFileName, QSettings::Format::IniFormat);

    settings.beginGroup(IniFile::Timer);
    timersPattern_ = settings.value(IniFile::Timer::TimerPattern).toString();
    finishActions_.clear();
    const auto finishActionsStr = settings.value(IniFile::Timer::FinishActions).toString();
    const auto parts = finishActionsStr.split(IniFile::FinishActions::Delimiter);
    for (const auto& item : parts)
        finishActions_.insert(finishActionFromString(item));
    soundFileName_ = settings.value(IniFile::Timer::SoundFileName).toString();
    defaultCategoryId_ = settings.value(IniFile::Timer::DefaultCategoryId, QVariant::fromValue(0)).toInt();
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
}
