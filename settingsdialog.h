/*
 * File:    settingsdialog.h
 * Author:  Ian Cathcart
 * Date:    2020/08/05
 * Version: 1.3
 *
 * Purpose: Define the behaviour of the settings dialog window.
 *
 * Modification history:
 * Aug 7, 2020 (IC V1.1)
 *  (a) Rename background colour fields.
 *  (b) Add saveDone() signal to tell mainWindow that the user OK'd the dialog.
 * Aug 7, 2020 (IC V1.2)
 *  (a) Renamed functions to on_jpgBgColour_clicked() and
 *	on_otherImageBgColour_clicked().
 * Oct 15, 2020 (JD V1.3)
 *  (a) Added setOtherImageButtonStyle().
 */

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QtCore>

namespace Ui
{
    class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget * parent = nullptr);
    ~SettingsDialog();

private slots:
    void on_jpgBgColour_clicked();
    void on_otherImageBgColour_clicked();

public slots:
    void saveSettings();
    void loadSettings();

signals:
    void saveDone();

private:
    Ui::SettingsDialog * ui;
    void setOtherImageButtonStyle();
};

#endif // SETTINGSDIALOG_H
