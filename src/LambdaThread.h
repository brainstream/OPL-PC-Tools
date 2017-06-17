/***********************************************************************************************
 *                                                                                             *
 * This file is part of the qpcopl project, the graphical PC tools for Open PS2 Loader.        *
 *                                                                                             *
 * qpcopl is free software: you can redistribute it and/or modify it under the terms of        *
 * the GNU General Public License as published by the Free Software Foundation,                *
 * either version 3 of the License, or (at your option) any later version.                     *
 *                                                                                             *
 * qpcopl is distributed in the hope that it will be useful,  but WITHOUT ANY WARRANTY;        *
 * without even the implied warranty of  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  *
 * See the GNU General Public License for more details.                                        *
 *                                                                                             *
 * You should have received a copy of the GNU General Public License along with MailUnit.      *
 * If not, see <http://www.gnu.org/licenses/>.                                                 *
 *                                                                                             *
 ***********************************************************************************************/

#ifndef __QPCOPL_LAMBDATHREAD__
#define __QPCOPL_LAMBDATHREAD__

#include <functional>
#include <QThread>
#include "Exception.h"

class LambdaThread : public QThread
{
    Q_OBJECT

public:
    explicit LambdaThread(std::function<void()> _lambda, QObject * _parent = nullptr) :
        QThread(_parent),
        m_lambda(_lambda)
    {
    }


protected:
    void run() override
    {
        try
        {
            m_lambda();
        }
        catch(const Exception & ex)
        {
            emit exception(ex.message());
        }
        catch(const std::exception & err)
        {
            emit exception(QString::fromStdString(err.what()));
        }
        catch(...)
        {
            emit exception(tr("An unknown error has occurred"));
        }
    }

signals:
    void exception(QString _message);

private:
    std::function<void()> m_lambda;
};

#endif // __QPCOPL_LAMBDATHREAD__
