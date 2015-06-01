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

# include "ccnxdownloader.h"

#include <QObject>
#include <QUrl>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QWidget>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QHash>

CCNxDownloader::CCNxDownloader(QWidget *parentWidget, QNetworkAccessManager *manager)
    : QObject(parentWidget), manager(manager), parentWidget(parentWidget)
{
}

QString CCNxDownloader::chooseSaveFile(const QUrl &url)
{
    QString fileName = url.path().split("/").last();
    if (!path.isEmpty())
        fileName = QDir(path).filePath(fileName);

    return QFileDialog::getSaveFileName(parentWidget, tr("Save File"), fileName);
}

void CCNxDownloader::startDownload(const QNetworkRequest &request)
{
    downloads[request.url().toString()] = chooseSaveFile(request.url());

    QNetworkReply *reply = manager->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(finishDownload()));
}

void CCNxDownloader::saveFile(QNetworkReply *reply)
{
    QString newPath = downloads[reply->url().toString()];

    if (newPath.isEmpty())
        newPath = chooseSaveFile(reply->url());

    if (!newPath.isEmpty()) {
        QFile file(newPath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(reply->readAll());
            file.close();
            path = QDir(newPath).dirName();
            QMessageBox::information(parentWidget, tr("Download Completed"),
                                     tr("Saved '%1'.").arg(newPath));
        } else
            QMessageBox::warning(parentWidget, tr("Download Failed"),
                                 tr("Failed to save the file."));
    }
}

void CCNxDownloader::finishDownload()
{
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    saveFile(reply);
    downloads.remove(reply->url().toString());
    reply->deleteLater();
}
