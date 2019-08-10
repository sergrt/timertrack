#include "stdafx.h"
#include "Category.h"

QIcon Category::createIcon() const {
    QPixmap pixmap(128, 128);
    pixmap.fill(color_);
    return QIcon(pixmap);
}
