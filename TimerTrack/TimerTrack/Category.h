#pragma once
class Category {
public:

    int id_ = 0;
    QString name_;
    QColor color_ = QColor(Qt::black);
    bool archived_ = false;

    [[nodiscard]] QIcon createIcon() const;
};

