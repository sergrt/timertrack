#pragma once

struct Category {
    int id_ = 0;
    QString name_;
    QColor color_ = QColor(Qt::black);
    bool archived_ = false;

    enum class Role {
        Resting,
        Default,
        Misc
    };

    Role role_;
    static Role roleFromStr(const QString& roleStr);

    [[nodiscard]] QIcon createIcon() const;
};
