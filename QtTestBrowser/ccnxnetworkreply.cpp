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

#include "ccnxnetworkreply.h"

#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QtNetwork>

#include <ccnximpl.h>


CCNxNetworkReply::CCNxNetworkReply(QNetworkAccessManager::Operation operation, const QNetworkRequest &request,
    QIODevice *device)
    : QNetworkReply()
{

	QString urlRaw = request.url().toString().toLatin1();
	
	QStringList urlList = urlRaw.split("?");

	if(urlList.size() == 2){	
        // for GET request with params
		QString url = urlList.at(0);
		QUrl request_url(url);
		outgoingData = urlList.at(1).toUtf8();
		qDebug() << "Outgoing Data:" << outgoingData;
		m_ccnxImpl = new CCNxImpl(request_url, outgoingData, true);
	}else{
		outgoingData = "";
        setUrl(request.url());

		if(device){
            // for POST request with params
			device->open(QIODevice::ReadOnly);
			outgoingData = device->readAll();
			qDebug() << "Outgoing Data:" << outgoingData;
			device->close();
			m_ccnxImpl = new CCNxImpl(request.url(), outgoingData, true);
		}else{
            if(getRequestType())
                m_ccnxImpl = new CCNxImpl(request.url(), outgoingData, false);
            else
                m_ccnxImpl = new CCNxImpl(request.url(), outgoingData, true);
		}		
	}	

	connect(m_ccnxImpl, SIGNAL(interestExpressed(QUrl)), this,
		SLOT(waitingForData(QUrl)));

	connect(m_ccnxImpl, SIGNAL(contentReadyRead(QByteArray, QString)), this,
		SLOT(receiveData(QByteArray, QString)));

	connect(m_ccnxImpl, SIGNAL(finished()), this,
		SLOT(allDataReady()));

	offset = 0;
    //setUrl(request.url());
	this->content = "";
	m_ccnxImpl->start();
	qDebug() << "Request start.";
}


CCNxNetworkReply::~CCNxNetworkReply()
{
	if(m_ccnxImpl != NULL){
		m_ccnxImpl->stop();
		m_ccnxImpl->wait();
	}

	content.clear();
}

// QIODevice methods
    
void CCNxNetworkReply::abort()//zhongzhi
{
	//qDebug() << "CCNxNetworkReply abort.";
	m_ccnxImpl->stop();
}


qint64 CCNxNetworkReply::bytesAvailable() const
{
	//qDebug() << "bytesAvailable:" << content.size() - offset;
    return content.size() - offset;//feng add bytesAvailable()函数返回reply保存的总的字节数减去offset后的值，
                                   //feng add offset值是我们已经向reader提供的字节数。
}


bool CCNxNetworkReply::isSequential() const
{
    return true;
}


qint64 CCNxNetworkReply::readData(char *data, qint64 maxSize)
{
	//qDebug() << url().toString() <<"offset:" << offset << "content.size():" << content.size() << "maxSize:" << maxSize;

	//qDebug() << "readData in reply:" << content;
		
    if (offset < content.size()) {
        qint64 number = qMin(maxSize, content.size() - offset);
        memcpy(data, content.constData() + offset, number);
	
        offset += number;//feng add offset变量更新以跟踪有多少字节已被读取
	//qDebug() << "readData:" << number << "left:" << content.size() - offset;
        return number;
    } else
        return 0;
}


QByteArray CCNxNetworkReply::readAll()
{
	qint64 number = qMin((qint64)content.size(), offset);
	//qDebug() << "readAll:" << content.size()-offset;
	if(number != 0)
		content = content.remove(0, number);

	return content;
}

void CCNxNetworkReply::waitingForData(QUrl url)
{

}


void CCNxNetworkReply::allDataReady()
{
	qDebug() << "All data ready, finished.";
	emit finished();
}


void CCNxNetworkReply::receiveData(QByteArray content, QString received_type)
{
	open(ReadOnly | Unbuffered);
	
    	this->content += content;

	QString type;
	if(received_type == NULL || QString::compare(received_type.trimmed(), "") == 0)
		type = getMIMETYPE();
	else
		type = received_type;

	//qDebug() << "Reply Content Type:" << type; 

	//setRawHeader(QString("Content-Type").toAscii(), type.toAscii());
	//setRawHeader(QString("Content-Length").toAscii(), QString::number(this->content.size()).toAscii());
	setAttribute(QNetworkRequest::HttpStatusCodeAttribute, QVariant("200"));
    	setHeader(QNetworkRequest::ContentTypeHeader, QVariant(type));
    	setHeader(QNetworkRequest::ContentLengthHeader, QVariant(this->content.size()));
    	emit readyRead();
}	

// True if it is static request
bool CCNxNetworkReply::getRequestType()
{
    QString entire_url = url().toString();
    //qDebug() << "Entire Url:" << entire_url;
    QString base_url = url().authority();
    //qDebug() << "Base Url:" << base_url;

    if(base_url.endsWith(".html") || entire_url.endsWith(".html")){
        return true;
    }else if(base_url.endsWith(".xml") || entire_url.endsWith(".xml")){
        return true;
    }else if(base_url.endsWith(".xhtml") || entire_url.endsWith(".xhtml")){
        return true;
    }else if(base_url.endsWith(".rtf") || entire_url.endsWith(".rtf")){
        return true;
    }else if(base_url.endsWith(".pdf") || entire_url.endsWith(".pdf")){
        return true;
    }else if(base_url.endsWith(".doc") || base_url.endsWith(".docx")){
        return true;
    }else if(base_url.endsWith(".png") || entire_url.endsWith(".png")){
        return true;
    }else if(base_url.endsWith(".gif") || entire_url.endsWith(".gif")){
        return true;
    }else if(base_url.endsWith(".bmp") || entire_url.endsWith(".bmp")){
        return true;
    }else if(base_url.endsWith(".jpg") || base_url.endsWith(".jpeg")  || entire_url.endsWith(".jpg")  || entire_url.endsWith(".jpeg")){
        return true;
    }else if(base_url.endsWith(".au")  || entire_url.endsWith(".au")){
        return true;
    }else if(base_url.endsWith(".mid") || base_url.endsWith(".midi") || entire_url.endsWith(".mid") || entire_url.endsWith(".midi")){
        return true;
    }else if(base_url.endsWith(".ra") || base_url.endsWith(".ram") || entire_url.endsWith(".ra") || entire_url.endsWith(".ram")){
        return true;
    }else if(base_url.endsWith(".mpg") || base_url.endsWith(".mpeg")
            || entire_url.endsWith(".mpg") || entire_url.endsWith(".mpeg")){
        return true;
    }else if(entire_url.endsWith(".mp4") || base_url.endsWith(".mp4")){
        return true;
    }else if(base_url.endsWith(".mp3") || entire_url.endsWith(".mp3")){
        return true;
    }else if(base_url.endsWith(".ogg") || base_url.contains(".ogg") || entire_url.endsWith(".ogg")){
        return true;
    }else if(base_url.endsWith(".avi") || entire_url.endsWith(".avi")){
        return true;
    }else if(base_url.endsWith(".gz") || entire_url.endsWith(".gz")){
        return true;
    }else if(base_url.endsWith(".tar") || entire_url.endsWith(".tar")){
        return true;
    }else if(base_url.endsWith(".txt") || entire_url.endsWith(".txt")){
        return true;
    }else if(base_url.endsWith(".js") || entire_url.endsWith(".js")){
        return true;
    }else if(base_url.endsWith(".css") || entire_url.endsWith(".css")){
        return true;
    }else if(base_url.endsWith(".jsp") || entire_url.endsWith(".jsp")){
        return false;
    }else{

        return false;
    }
}

QString CCNxNetworkReply::getMIMETYPE()
{
	QString entire_url = url().toString();
	//qDebug() << "Entire Url:" << entire_url;
	QString base_url = url().authority();
	//qDebug() << "Base Url:" << base_url; 

	if(base_url.endsWith(".html") || entire_url.endsWith(".html")){
		return "text/html; charset=UTF-8";
	}else if(base_url.endsWith(".xml") || entire_url.endsWith(".xml")){
		return "text/xml";
	}else if(base_url.endsWith(".xhtml") || entire_url.endsWith(".xhtml")){
		return "xhtml application/xhtml+xml";
	}else if(base_url.endsWith(".rtf") || entire_url.endsWith(".rtf")){
		return "appliction/rtf";
	}else if(base_url.endsWith(".pdf") || entire_url.endsWith(".pdf")){
		return "appliction/pdf";
	}else if(base_url.endsWith(".doc") || base_url.endsWith(".docx")){
		return "appliction/msword";
	}else if(base_url.endsWith(".png") || entire_url.endsWith(".png")){
		return "image/png";
	}else if(base_url.endsWith(".gif") || entire_url.endsWith(".gif")){
		return "image/gif";
	}else if(base_url.endsWith(".bmp") || entire_url.endsWith(".bmp")){
		return "image/bmp";
	}else if(base_url.endsWith(".jpg") || base_url.endsWith(".jpeg")  || entire_url.endsWith(".jpg")  || entire_url.endsWith(".jpeg")){
		return "image/jpeg";
	}else if(base_url.endsWith(".au")  || entire_url.endsWith(".au")){
		return "audio/basic";
	}else if(base_url.endsWith(".mid") || base_url.endsWith(".midi") || entire_url.endsWith(".mid") || entire_url.endsWith(".midi")){
		return "audio/midi,audio/x-mid";
	}else if(base_url.endsWith(".ra") || base_url.endsWith(".ram") || entire_url.endsWith(".ra") || entire_url.endsWith(".ram")){
		return "audio/x-pn-realaudio";
	}else if(base_url.endsWith(".mpg") || base_url.endsWith(".mpeg")
			|| entire_url.endsWith(".mpg") || entire_url.endsWith(".mpeg")){
		return "video/mpeg";
	}else if(entire_url.endsWith(".mp4") || base_url.endsWith(".mp4")){
		return "video/mp4";
	}else if(base_url.endsWith(".mp3") || entire_url.endsWith(".mp3")){
		return "audio/mpeg";
	}else if(base_url.endsWith(".ogg") || base_url.contains(".ogg") || entire_url.endsWith(".ogg")){
		return "video/ogg";
	}else if(base_url.endsWith(".avi") || entire_url.endsWith(".avi")){
		return "video/x-msvideo";
	}else if(base_url.endsWith(".gz") || entire_url.endsWith(".gz")){
		return "appliction/x-gzip";
	}else if(base_url.endsWith(".tar") || entire_url.endsWith(".tar")){
		return "appliction/x-tar";
	}else if(base_url.endsWith(".txt") || entire_url.endsWith(".txt")){
		return "text/plain";
	}else if(base_url.endsWith(".js") || entire_url.endsWith(".js")){
		return "application/x-javascript";
	}else if(base_url.endsWith(".css") || entire_url.endsWith(".css")){
		return "text/css";
	}else{

        return "application/octet-stream";

	}	
}
