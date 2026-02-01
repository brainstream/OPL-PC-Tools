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

#include <OplPcTools/StringConverter.h>
#if QT_VERSION_MAJOR < 6
#   include <QTextCodec>
#else
#   include <QStringConverter>
#endif

using namespace OplPcTools;

QStringList TextEncoding::availableCodecs()
{
#if QT_VERSION_MAJOR < 6
    QStringList result;
    const QList<QByteArray> codecs = QTextCodec::availableCodecs();
    result.reserve(codecs.count());
    foreach(const QByteArray & codec, codecs)
        result.append(QString::fromLatin1(codec));
    return result;
#else
    return QStringConverter::availableCodecs();
#endif
}

class StringConverter::Private final
{
    Q_DISABLE_COPY(Private)

public:
    Private(Private && _private) = default;
    Private & operator = (Private && _private) = default;

#if QT_VERSION_MAJOR < 6
public:
    explicit Private(const QString & _codec) :
        m_codec_name(_codec),
        mp_codec(QTextCodec::codecForName(_codec.toLatin1()))
    {
    }

    QString codecName() const
    {
        return m_codec_name;
    }

    QString decode(const QByteArray & _bytes) const
    {
        return mp_codec ? mp_codec->toUnicode(_bytes) : QString::fromLatin1(_bytes);
    }

    QByteArray encode(const QString & _string) const
    {
        return mp_codec ? mp_codec->fromUnicode(_string) : _string.toLatin1();
    }

private:
    QString m_codec_name;
    const QTextCodec * mp_codec;
#else
public:
    explicit Private(const QString & _codec) :
        m_decoder(_codec),
        m_endcoder(_codec)
    {
    }

    QString codecName() const
    {
        return m_decoder.name();
    }

    QString decode(const QByteArray & _bytes) const
    {
        return m_decoder.decode(_bytes);
    }

    QByteArray encode(const QString & _string) const
    {
        return m_endcoder.encode(_string);
    }

private:
    mutable QStringDecoder m_decoder; // For some unknown reason, QStringDecoder::decode is not const
    mutable QStringEncoder m_endcoder; // For some unknown reason, QStringEncoder::encode is not const
#endif
};

StringConverter::StringConverter(const QString & _codec) :
    mp_private(new Private(_codec))
{
}

StringConverter::StringConverter(StringConverter && _converter) :
    mp_private(new Private(std::move(*_converter.mp_private)))
{
    _converter.mp_private = nullptr;
}

StringConverter::~StringConverter()
{
    delete mp_private;
}

StringConverter & StringConverter::operator = (StringConverter && _converter)
{
    if(this != &_converter)
    {
        *mp_private = std::move(*_converter.mp_private);
        _converter.mp_private = nullptr;
    }
    return *this;
}

QString StringConverter::codecName() const
{
    return mp_private->codecName();
}

QString StringConverter::decode(const QByteArray & _bytes) const
{
    return mp_private->decode(_bytes);
}

QByteArray StringConverter::encode(const QString & _string) const
{
    return mp_private->encode(_string);
}
