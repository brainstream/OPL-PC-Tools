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

#include <OplPcTools/TextEncoding.h>
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

#if QT_VERSION_MAJOR < 6
    class TextDecoder::Private final
    {
    public:
        explicit Private(const QString & _codec) :
            m_codec_name(_codec)
        {
            QTextCodec * codec = QTextCodec::codecForName(_codec.toLatin1());
            mp_decoder = codec ? codec->makeDecoder() : nullptr;
        }

        Private(Private && _private) :
            m_codec_name(_private.m_codec_name),
            mp_decoder(_private.mp_decoder)
        {
            _private.mp_decoder = nullptr;
        }

        ~Private()
        {
            delete mp_decoder;
        }

        Private & operator = (Private && _private)
        {
            if(this == &_private)
                return *this;
            m_codec_name =_private.m_codec_name;
            mp_decoder = _private.mp_decoder;
            _private.mp_decoder = nullptr;
            return *this;
        }

        QString codecName() const
        {
            return m_codec_name;
        }

        QString decode(const QByteArray & _bytes) const
        {
            return mp_decoder ? mp_decoder->toUnicode(_bytes) : QString::fromLatin1(_bytes);
        }

    private:
        QString m_codec_name;
        mutable QTextDecoder * mp_decoder; // For some unknown reason, QTextDecoder::toUnicode is not const
    };
#else
    class TextDecoder::Private final
    {
    public:
        explicit Private(const QString & _codec) :
            m_decoder(_codec)
        {
        }

        Private(Private && _private) = default;
        Private & operator = (Private && _private) = default;

        QString codecName() const
        {
            return m_decoder.name();
        }

        QString decode(const QByteArray & _bytes) const
        {
            return m_decoder.decode(_bytes);
        }

    private:
        mutable QStringDecoder m_decoder; // For some unknown reason, QStringDecoder::decode is not const
    };
#endif

TextDecoder::TextDecoder(const QString & _codec) :
    mp_private(new Private(_codec))
{
}

TextDecoder::TextDecoder(TextDecoder && _decoder) :
    mp_private(new Private(std::move(*_decoder.mp_private)))
{
    _decoder.mp_private = nullptr;
}

TextDecoder::~TextDecoder()
{
    delete mp_private;
}

TextDecoder & TextDecoder::operator = (TextDecoder && _decoder)
{
    if(this != &_decoder)
    {
        *mp_private = std::move(*_decoder.mp_private);
        _decoder.mp_private = nullptr;
    }
    return *this;
}

QString TextDecoder::codecName() const
{
    return mp_private->codecName();
}

QString TextDecoder::decode(const QByteArray & _bytes) const
{
    return mp_private->decode(_bytes);
}
