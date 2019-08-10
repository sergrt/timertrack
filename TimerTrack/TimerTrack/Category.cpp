#include "stdafx.h"
#include "Category.h"

QIcon Category::createIcon() const {
    static constexpr auto iconSize = 128;
    QPixmap pixmap(iconSize, iconSize);
    pixmap.fill(color_);
    return QIcon(pixmap);
}
