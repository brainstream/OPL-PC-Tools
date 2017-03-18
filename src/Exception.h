#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <QException>

class Exception : public QException
{
public:
    explicit Exception(const QString & _message) :
        m_message(_message)
    {
    }

    const char * what() const noexcept override
    {
        return nullptr;
    }

    const QString & message() const noexcept
    {
        return m_message;
    }

private:
    QString m_message;
};

#define DECLARE_EXCEPTION(type_name)                    \
    class type_name : public Exception                  \
    {                                                   \
    public:                                             \
        explicit type_name(const QString & _message) :  \
            Exception(_message)                         \
        {                                               \
        }                                               \
    };

#endif // EXCEPTION_H
