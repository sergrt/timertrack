# TimerTrack
## Overview
TimerTrack is a handy "pomodoro" timer with task categories. The idea is quite simple yet very useful - you can assign to every timer interval you set a specific category, and after a while it is possible to gather valuable statistics about how much time was dedicated to particular activities.

![MainWindow](../assets/assets/MainWindow.png)

There's a number of pomodoro timers in the wild, but most of them rather simple or overcomplicated. The goal of TimerTrack is to provide easiest ways to set timers and track categories.

## Features
- Simple unobtrusive timer buddy;
- Categories - either use them or use default;
- Supporting both single timers or timer sequences;
- Highly customizable UI with CSS-like syntax;
- Easy customization for the desired timer intervals;
- Several actions on timer stop - popup, tooltip, sound;
- Statistics about completed intervals - daily, weekly, or any other interval of interest;
- Handling of "interrupted" pomodoros;
- TimerTrack is completely free.

## Quick reference
TimerTrack application rests in the system tray, and is displayed on the screen as a (small) window with ticking clock. All operations are performed with context menu.

There are two modes - sequential timers or single timers. Sequential mode started with "Start sequence" menu item. To customize timers in the sequence there's special field in the settings. Sequence is a string of intervals, separated by comma (","). Each interval can be defined either as "<hours>h<minutes>m<seconds>s" string, or "hh:mm:ss" string. Hours (and minutes) can be omitted if not needed.
    
![SettingsTimer](../assets/assets/SettingsTimer.png)

Every odd interval in the sequence is threated as a category, specified by "Default category" combo box. Every even interval is considered as "Resting". So, defining "25:00,05:00,25:00,05:00,25:00,15:00" makes a orthodoxal pomodoro timer pattern.

Categories are customized with settings dialog. Two types of categories - "Resting" and "Default" cannot be deleted (but can be modified).

![SettingsCategories](../assets/assets/SettingsCategories.png)

If any category has been ever logged, there is no possibility to completely delete this category. Instead of being deleted, such category is became marked as "archived" (gray item in the category list), and no new operations can be performed on this category, either with timer sequence or single timer. You can always activate such category back, if needed.

Single interval can be started via context menu by choosing timer interval and category. Context menu items can be customized with settings dialog. Intervals can be defined with the same notation, as timer sequence.

![ContextMenu](../assets/assets/ContextMenu.png)

Statistics window can be spawn with context menu, and it is provide both chart and table presentation of the completed pomodoros. Currently chart display only completed pomodoros of the selected categories, and table provides both completed (columns ending with "+" sign) and interrupted (columns ending with "-" sign) timer intervals, as a number or as hours/minutes/seconds.

![StatisticsChart](../assets/assets/StatisticsChart.png)

## Customizing UI
All settings of the application are stored in the settings.ini file. Some of the settings are available to be changed with Settings dialog, and some are not (to prevent overwhelming interface with rarely modified options). For example, width and height of the main window can be changed only with altering settings.ini.
UI is customized with QSS, reference is here - https://doc.qt.io/qt-5/stylesheet-syntax.html. It is very similar to CSS. For example, stylesheet for the application can be like this:
```sh
* {
    font-family: Arial;
    font-size: 16px;
}
#timerTrackWindow {
    background-color: rgba(200, 200, 200, 100%);
}
#frame {
    margin: 0px;
}
#timerLabel {
    font-family: Arial;
}
#closeButton {
    font-size: 10px;
    color: gray;
    background-color: rgba(255, 255, 255, 0%);
}
```
where
- timerTrackWindow - main window;
- frame - frame inside main window;
- timerLabel - label with clock;
- closeButton - button which hides application.

Manipulating with the StyleSheet can help to provide almost any UI presentation.
