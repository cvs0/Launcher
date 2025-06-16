/* Copyright 2024 Petr Mrázek
 *
 * This source is subject to the Microsoft Permissive License (MS-PL).
 * Please see the COPYING.md file for more information.
 */

#include "WrapLabel.h"

WrapLabel::WrapLabel(const QString& text, QWidget* parent) : QTextEdit(text, parent)
{
    PrivateInit();
}

WrapLabel::WrapLabel(QWidget* parent): QTextEdit(parent)
{
    PrivateInit();
}

void WrapLabel::PrivateInit()
{
    setStyleSheet(
        R"QSS(
        QTextEdit {
            background: palette(window);
            color: palette(window-text);
        }
        )QSS"
    );

    setReadOnly(true);
    setFrameShape(QFrame::Shape::NoFrame);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    connect(this, &QTextEdit::textChanged, this, &QTextEdit::updateGeometry);
}

QSize WrapLabel::minimumSizeHint() const
{
    QTextDocument *doc = document()->clone();
    doc->setTextWidth(viewport()->width());
    auto height = doc->size().height() + frameWidth() * 2;
    return QSize(50, height);
}


QSize WrapLabel::sizeHint() const
{
    return minimumSizeHint();
}

void WrapLabel::resizeEvent(QResizeEvent* event)
{
    QTextEdit::resizeEvent(event);
    updateGeometry();
}
