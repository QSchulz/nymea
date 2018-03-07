/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2017 Michael Zanetti <michael.zanetti@guh.io>            *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  nymea is free software: you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  nymea is distributed in the hope that it will be useful,               *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with nymea. If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "certificategenerator.h"

#include "openssl/ssl.h"

#include <QRegExp>
#include <QFileInfo>
#include <QDir>
#include <QUuid>
#include <QDebug>

namespace nymeaserver {

void CertificateGenerator::generate(const QString &certificateFilename, const QString &keyFilename)
{
    EVP_PKEY * pkey = nullptr;
    BIGNUM          *bne = NULL;
    RSA * rsa = nullptr;
    X509 * x509 = nullptr;
    X509_NAME * name = nullptr;
    BIO * bp_public = nullptr, * bp_private = nullptr;
    const char * buffer = nullptr;
    long size;

    bne = BN_new();
    BN_set_word(bne, RSA_F4);
    q_check_ptr(bne);

    rsa = RSA_new();
    RSA_generate_key_ex(rsa, 2048, bne, nullptr);
    q_check_ptr(rsa);

    pkey = EVP_PKEY_new();
    q_check_ptr(pkey);

    EVP_PKEY_assign_RSA(pkey, rsa);
    x509 = X509_new();
    q_check_ptr(x509);
    // Randomize serial number in case a previous one is stuck in a browser (Chromium
    // completely rejects reused serial numbers and doesn't even allow to bypass it by an exception)
    qsrand(QUuid::createUuid().toString().remove(QRegExp("[a-zA-Z{}-]")).left(5).toInt());
    ASN1_INTEGER_set(X509_get_serialNumber(x509), qrand());
    X509_gmtime_adj(X509_get_notBefore(x509), 0); // not before current time
    X509_gmtime_adj(X509_get_notAfter(x509), 31536000L*10); // not after 10 years from this point
    X509_set_pubkey(x509, pkey);
    name = X509_get_subject_name(x509);
    q_check_ptr(name);
    X509_NAME_add_entry_by_txt(name, "E", MBSTRING_ASC, (unsigned char *)"guh.io", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, (unsigned char *)"guh.io", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "OU", MBSTRING_ASC, (unsigned char *)"home", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "O", MBSTRING_ASC, (unsigned char *)"guh.io", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "L", MBSTRING_ASC, (unsigned char *)"Vienna", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "C", MBSTRING_ASC, (unsigned char *)"AT", -1, -1, 0);
    X509_set_issuer_name(x509, name);
    X509_sign(x509, pkey, EVP_sha256());
    bp_private = BIO_new(BIO_s_mem());
    q_check_ptr(bp_private);
    if(PEM_write_bio_PrivateKey(bp_private, pkey, nullptr, nullptr, 0, nullptr, nullptr) != 1)
    {
        BN_free(bne);
        EVP_PKEY_free(pkey);
        X509_free(x509);
        BIO_free_all(bp_private);
        qFatal("PEM_write_bio_PrivateKey");
        return;
    }
    bp_public = BIO_new(BIO_s_mem());
    q_check_ptr(bp_public);
    if(PEM_write_bio_X509(bp_public, x509) != 1)
    {

        BN_free(bne);
        EVP_PKEY_free(pkey);
        X509_free(x509);
        BIO_free_all(bp_public);
        BIO_free_all(bp_private);
        qFatal("PEM_write_bio_X509");
    }
    size = BIO_get_mem_data(bp_public, &buffer);
    q_check_ptr(buffer);
    QFileInfo certFi(certificateFilename);
    QDir dir;
    QFile certfile(certificateFilename);
    if (!dir.mkpath(certFi.absolutePath()) || !certfile.open(QFile::WriteOnly | QFile::Truncate) || certfile.write(buffer, size) != size) {
        qWarning() << "Error writing certificate file" << certificateFilename;
    }
    certfile.close();

    size = BIO_get_mem_data(bp_private, &buffer);
    q_check_ptr(buffer);
    QFileInfo keyFi(keyFilename);
    QFile keyFile(keyFilename);
    if (!dir.mkpath(keyFi.absolutePath()) || !keyFile.open(QFile::WriteOnly | QFile::Truncate) || keyFile.write(buffer, size) != size) {
        qWarning() << "Error writing key file" << keyFilename;
    }
    keyFile.close();

    BN_free(bne);
    EVP_PKEY_free(pkey); // this will also free the rsa key
    X509_free(x509);
    BIO_free_all(bp_public);
    BIO_free_all(bp_private);

}

}

