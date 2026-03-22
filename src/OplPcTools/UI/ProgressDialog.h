/***********************************************************************************************
 * Copyright © 2017-2026 Sergey Smolyannikov aka brainstream                                   *
 *                                                                                             *
 * This file is part of the OPL PC Tools project, the graphical PC tools for Open PS2 Loader.  *
 *                                                                                             *
 * OPL PC Tools is free software: you can redistribute it and/or modify it under the terms of  *
 * the GNU General Public License as published by the Free Software Foundation,                *
 * either version 3 of the License, or (at your option) any later version.                     *
 *                                                                                             *
 * OPL PC Tools is distributed in the hope that it will be useful,  but WITHOUT ANY WARRANTY;  *
 * without even the implied warranty of  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  *
 * See the GNU General Public License for more details.                                        *
 *                                                                                             *
 * You should have received a copy of the GNU General Public License along with OPL PC Tools   *
 * If not, see <http://www.gnu.org/licenses/>.                                                 *
 *                                                                                             *
 ***********************************************************************************************/

#pragma once

#include "ui_ProgressDialog.h"

namespace OplPcTools {
namespace UI {

class ProgressDialog : public QDialog, private Ui::ProgressDialog
{
    Q_OBJECT

public:
    explicit ProgressDialog(QWidget * _parent = nullptr);
    void disableCancelation(bool _disable);
    bool isCancelationDisabled() const { return m_is_cancelation_disabled; }
    int progressValue() const { return mp_progress_bar->value(); }

public slots:
    void accept() override;
    void setProgressLabelText(const QString & _text);
    void setProgressRange(int _min_value, int _max_value);
    void setProgressMinimum(int _min_value);
    void setProgressMaximum(int _max_value);
    void setProgressValue(int _value);

signals:
    void canceled();

protected:
    void closeEvent(QCloseEvent * _event) override;

private:
    void repackLayout();

private:
    bool m_is_cancelation_disabled;
    bool m_is_one_shot_close_allowed;
};

} // namespace UI
} // namespace OplPcTools
