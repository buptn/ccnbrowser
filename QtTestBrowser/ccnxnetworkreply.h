/*
 * Copyright (C) 2013 peng yue 
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ccnxnetworkreply_h
#define ccnxnetworkreply_h

#include <ccnximpl.h>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QtNetwork>


class CCNxNetworkReply : public QNetworkReply
{
    Q_OBJECT

public:
    CCNxNetworkReply(QNetworkAccessManager::Operation operation, const QNetworkRequest &request, QIODevice *device);
	~CCNxNetworkReply();
    void abort();
    qint64 bytesAvailable() const;
    bool isSequential() const;

protected:
    qint64 readData(char *data, qint64 maxSize);
    QByteArray readAll();

private slots:
	void waitingForData(QUrl url);
	void receiveData(QByteArray content, QString received_type);
	void allDataReady();

private:
    QString getMIMETYPE();
    bool getRequestType();
    QByteArray content;
    qint64 offset;
    QStringList units;
	 QByteArray outgoingData;
    CCNxImpl *m_ccnxImpl;
}; 

#endif
