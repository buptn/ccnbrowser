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

#include "ccnxnetworkaccessmanager.h"

#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkProxy>
#include <QMessageBox>
#include <QDebug>

CCNxNetworkAccessManager::CCNxNetworkAccessManager(QNetworkAccessManager *manager, QObject *parent)
    : QNetworkAccessManager(parent)
{
    setCache(manager->cache());
    setCookieJar(manager->cookieJar());
    setProxy(manager->proxy());
    setProxyFactory(manager->proxyFactory());
}

QNetworkReply *CCNxNetworkAccessManager::createRequest(
    QNetworkAccessManager::Operation operation, const QNetworkRequest &request,
    QIODevice *device)
{
	
    qDebug() << "create Request:"+request.url().toString();

    QVariant cacheLoad = request.attribute(QNetworkRequest::CacheLoadControlAttribute);
     if (cacheLoad.isValid()){
         //QNetworkRequest::CacheLoadControl lastCacheLoad = static_cast<QNetworkRequest::CacheLoadControl>(cacheLoad.toUInt());
         qDebug() << "[IsReload?] Request cache:"+cacheLoad.toString();
     }

    if (request.url().scheme() != "ccnx"){
        return QNetworkAccessManager::createRequest(operation, request, device);
    }

    if (operation == GetOperation || operation == PostOperation){
        return new CCNxNetworkReply(operation, request, device);
    }else{
        return QNetworkAccessManager::createRequest(operation, request, device);	
    }
}
