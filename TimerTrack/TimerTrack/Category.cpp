#include "stdafx.h"
#include "Category.h"

extern const QString restingCategoryRole;
extern const QString defaultCategoryRole;

QIcon Category::createIcon() const {
    static constexpr auto iconSize = 128;
    QPixmap pixmap(iconSize, iconSize);
    pixmap.fill(color_);
    return QIcon(pixmap);
}

Category::Role Category::roleFromStr(const QString& roleStr) {
    if (roleStr == restingCategoryRole)
        return Role::Resting;
    else if (roleStr == defaultCategoryRole)
        return Role::Default;

    return Role::Misc;
}