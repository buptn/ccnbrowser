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


#ifndef ccnxdownloader_h
#define ccnxdownloader_h

#include <QObject>
#include <QUrl>
#include <QWidget>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QHash>

class CCNxDownloader : public QObject
{
    Q_OBJECT

public:
    CCNxDownloader(QWidget *parentWidget, QNetworkAccessManager *manager);

public slots:    
    QString chooseSaveFile(const QUrl &url);
    void startDownload(const QNetworkRequest &request);
    void saveFile(QNetworkReply *reply);
    void finishDownload();

private:
    QNetworkAccessManager *manager;
    QNetworkReply *reply;
    QHash<QString, QString> downloads;
    QString path;
    QWidget *parentWidget;
};

#endif
