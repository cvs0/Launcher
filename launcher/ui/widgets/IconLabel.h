#pragma once
#include <QWidget>
#include <QIcon>

class QStyleOption;

/**
 * This is a trivial widget that paints a QIcon of the specified size.
 */
class IconLabel : public QWidget
{
    Q_OBJECT

public:
    explicit IconLabel(QWidget *parent, QIcon icon, QSize size);
    virtual ~IconLabel() = default;

    virtual QSize sizeHint() const;
    virtual void paintEvent(QPaintEvent *);

    void setIcon(QIcon icon);

private:
    QSize m_size;
    QIcon m_icon;
};

class IconLabel24 : public IconLabel
{
    Q_OBJECT
public:
    explicit IconLabel24(QWidget *parent) : IconLabel(parent, QIcon(), QSize(24, 24)) {}
    virtual ~IconLabel24() = default;
};

class IconLabel32 : public IconLabel
{
    Q_OBJECT
public:
    explicit IconLabel32(QWidget *parent) : IconLabel(parent, QIcon(), QSize(32, 32)) {}
    virtual ~IconLabel32() = default;
};
