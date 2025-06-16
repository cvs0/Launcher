/* Copyright 2024 Petr Mrázek
 *
 * This source is subject to the Microsoft Permissive License (MS-PL).
 * Please see the COPYING.md file for more information.
 */

#pragma once

#include <QTextEdit>

/**
 * A replacement for text-based QLabel with multiple lines.
 *
 * The issue with multi-line QLabel is that it doesn't work with layouts and causes overlaps.
 * This fixes it by reusing some logic from QTextEdit instead.
 */
class WrapLabel: public QTextEdit
{
public:
    explicit WrapLabel(QWidget* parent = nullptr);
    explicit WrapLabel(const QString& text, QWidget* parent = nullptr);
    virtual ~WrapLabel() = default;

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;
    void resizeEvent(QResizeEvent * event) override;

private:
    void PrivateInit();
};
