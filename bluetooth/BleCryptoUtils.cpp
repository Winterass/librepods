#include "BleCryptoUtils.h"

#include <openssl/aes.h>
#include <QByteArray>
#include <QDebug>
#include <QStringList>
#include <algorithm>
#include <cstring>

BleCryptoUtils::BleCryptoUtils(QObject *parent) : QObject(parent)
{
}

bool BleCryptoUtils::verifyRPA(const QString &address, const QByteArray &irk)
{
    if (address.isEmpty() || irk.isEmpty() || irk.size() != 16)
    {
        return false;
    }

    QStringList parts = address.split(QLatin1Char(':'));
    if (parts.size() != 6)
    {
        return false;
    }

    QByteArray rpa;
    bool ok;
    for (int i = parts.size() - 1; i >= 0; --i)
    {
        rpa.append(static_cast<char>(parts[i].toInt(&ok, 16)));
        if (!ok)
        {
            return false;
        }
    }

    if (rpa.size() != 6)
    {
        return false;
    }

    QByteArray prand = rpa.mid(3, 3);
    QByteArray hash = rpa.left(3);
    QByteArray computedHash = ah(irk, prand);

    return hash == computedHash;
}

bool BleCryptoUtils::isValidIrkRpa(const QByteArray &irk, const QString &rpa)
{
    return verifyRPA(rpa, irk);
}

QByteArray BleCryptoUtils::e(const QByteArray &key, const QByteArray &data)
{
    if (key.size() != 16 || data.size() != 16)
    {
        return QByteArray();
    }

    QByteArray reversedKey(key);
    std::reverse(reversedKey.begin(), reversedKey.end());

    QByteArray reversedData(data);
    std::reverse(reversedData.begin(), reversedData.end());

    AES_KEY aesKey;
    if (AES_set_encrypt_key(reinterpret_cast<const unsigned char *>(reversedKey.constData()), 128, &aesKey) != 0)
    {
        return QByteArray();
    }

    unsigned char out[16];
    AES_encrypt(reinterpret_cast<const unsigned char *>(reversedData.constData()), out, &aesKey);

    QByteArray result(reinterpret_cast<char *>(out), 16);
    std::reverse(result.begin(), result.end());

    return result;
}

QByteArray BleCryptoUtils::ah(const QByteArray &k, const QByteArray &r)
{
    if (r.size() < 3)
    {
        return QByteArray();
    }

    QByteArray rPadded(16, 0);
    rPadded.replace(0, 3, r.left(3));

    QByteArray encrypted = e(k, rPadded);
    if (encrypted.isEmpty())
    {
        return QByteArray();
    }

    return encrypted.left(3);
}

QByteArray BleCryptoUtils::decryptLastBytes(const QByteArray &data, const QByteArray &key)
{
    if (data.size() < 16 || key.size() != 16)
    {
        qDebug() << "Invalid input: data size < 16 or key size != 16";
        return QByteArray();
    }

    QByteArray block = data.right(16);

    AES_KEY aesKey;
    if (AES_set_decrypt_key(reinterpret_cast<const unsigned char *>(key.constData()), 128, &aesKey) != 0)
    {
        qDebug() << "Failed to set AES decryption key";
        return QByteArray();
    }

    unsigned char out[16];
    unsigned char iv[16];
    memset(iv, 0, 16);

    AES_cbc_encrypt(reinterpret_cast<const unsigned char *>(block.constData()), out, 16, &aesKey, iv, AES_DECRYPT);

    QByteArray result(reinterpret_cast<char *>(out), 16);

    return result;
}
