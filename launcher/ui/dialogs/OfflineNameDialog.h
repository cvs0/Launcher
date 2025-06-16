#pragma once
/*
 * Copyright 2024 Petr Mrázek
 *
 * This source is subject to the Microsoft Permissive License (MS-PL).
 * Please see the COPYING.md file for more information.
 */

#include <QDialog>
#include <QString>


class OfflineNameDialog : public QDialog
{
    Q_OBJECT

public:
    enum OfflineNameDialogCode {
        Rejected = QDialog::Rejected,
        Accepted = QDialog::Accepted,
        OnlineRequested,
    };

    OfflineNameDialog(QString current_name, QWidget * parent = nullptr);
    virtual ~OfflineNameDialog() = default;

    QString textValue() const;

public slots:
    void accept() override;

private slots:
    void textChanged(const QString& text);
    void retryOnline();

private:
    class QDialogButtonBox * m_buttonBox;
    class QLineEdit* m_lineEdit;
    bool m_acceptEnabled = false;
};
