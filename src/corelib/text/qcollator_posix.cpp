// Copyright (C) 2021 The Qt Company Ltd.
// Copyright (C) 2013 Aleix Pol Gonzalez <aleixpol@kde.org>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qcollator_p.h"
#include "qstringlist.h"
#include "qstring.h"
#include "qvarlengtharray.h"

#include <cstring>
#include <cwchar>

QT_BEGIN_NAMESPACE

void QCollatorPrivate::init()
{
    if (!isC()) {
        if (locale != QLocale::system().collation()) {
            qWarning("Only the C and system collation locales are supported "
                     "with the POSIX collation implementation");
        }
        if (caseSensitivity != Qt::CaseSensitive)
            qWarning("Case insensitive sorting unsupported in the posix collation implementation");
    }
    if (numericMode)
        qWarning("Numeric mode unsupported in the posix collation implementation");
    if (ignorePunctuation)
        qWarning("Ignoring punctuation unsupported in the posix collation implementation");
    dirty = false;
}

void QCollatorPrivate::cleanup()
{
}

static void stringToWCharArray(QVarLengthArray<wchar_t> &ret, QStringView string)
{
    ret.resize(string.length());
    int len = string.toWCharArray(ret.data());
    ret.resize(len+1);
    ret[len] = 0;
}

int QCollator::compare(QStringView s1, QStringView s2) const
{
    if (!s1.size())
        return s2.size() ? -1 : 0;
    if (!s2.size())
        return +1;

    if (d->isC())
        return s1.compare(s2, caseSensitivity());
    if (d->dirty)
        d->init();

    QVarLengthArray<wchar_t> array1, array2;
    stringToWCharArray(array1, s1);
    stringToWCharArray(array2, s2);
    return std::wcscoll(array1.constData(), array2.constData());
}

QCollatorSortKey QCollator::sortKey(const QString &string) const
{
    if (d->dirty)
        d->init();

    QVarLengthArray<wchar_t> original;
    stringToWCharArray(original, string);
    QList<wchar_t> result(original.size());
    if (d->isC()) {
        std::copy(original.cbegin(), original.cend(), result.begin());
    } else {
        size_t size = std::wcsxfrm(result.data(), original.constData(), string.size());
        if (size > uint(result.size())) {
            result.resize(size+1);
            size = std::wcsxfrm(result.data(), original.constData(), string.size());
        }
        result.resize(size+1);
        result[size] = 0;
    }
    return QCollatorSortKey(new QCollatorSortKeyPrivate(std::move(result)));
}

int QCollatorSortKey::compare(const QCollatorSortKey &otherKey) const
{
    return std::wcscmp(d->m_key.constData(), otherKey.d->m_key.constData());
}

QT_END_NAMESPACE
