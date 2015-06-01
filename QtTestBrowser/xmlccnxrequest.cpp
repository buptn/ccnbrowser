#include "xmlccnxrequest.h"

#include <QUrl> 
#include <QDebug>
#include "ccnximpl.h"

XMLCCNxRequest :: XMLCCNxRequest(QObject *parent): QObject(parent){
	
}

XMLCCNxRequest :: ~XMLCCNxRequest(){

	if(m_ccnxImpl != NULL){
		m_ccnxImpl->stop();
		m_ccnxImpl->wait();
	}	
}

void XMLCCNxRequest :: send(QString type, QString url, QString params, bool async, bool responseFromServer){

	// validate type, url and params
	if(url.startsWith("/"))
		url += "ccnx:/";

	if(paramsIsValid(params)){

		QUrl request_url(url);
		outgoingData = params.toUtf8();
		qDebug() << "Outgoing Data:" << outgoingData;
		//qDebug() << "XMLCCNxRequest --> responseFromServer" << responseFromServer;
		if(async){
			
			// try to send interest async
			m_ccnxImpl = new CCNxImpl(request_url, outgoingData, responseFromServer);

			connect(m_ccnxImpl, SIGNAL(contentReadyRead(QByteArray, QString)), this,
				SLOT(receive(QByteArray, QString)));
			connect(m_ccnxImpl, SIGNAL(interestExpressed(QUrl)), this,
				SLOT(sent_slot(QUrl)));
			connect(m_ccnxImpl, SIGNAL(error(QString)), this,
				SLOT(error_slot(QString)));
			connect(m_ccnxImpl, SIGNAL(finished()), this,
				SLOT(finished()));

			m_ccnxImpl->start();

			/* async operation */
			
		}else{

			// try to send interest sync
			m_ccnxImpl = new CCNxImpl(request_url, outgoingData, responseFromServer);

			connect(m_ccnxImpl, SIGNAL(contentReadyRead(QByteArray, QString)), this,
				SLOT(receive(QByteArray, QString)));
			connect(m_ccnxImpl, SIGNAL(interestExpressed(QUrl)), this,
				SLOT(sent_slot(QUrl)));
			connect(m_ccnxImpl, SIGNAL(error(QString)), this,
				SLOT(error_slot(QString)));
			connect(m_ccnxImpl, SIGNAL(finished()), this,
				SLOT(finished()));

			m_ccnxImpl->start();	

			/* sync operation */							
		}			
	}
}


void XMLCCNxRequest :: receive(QByteArray content, QString received_type){
	qDebug() << "AJAX request receive response";

	QString data(content);
    this->data += data;
}


void XMLCCNxRequest :: sent_slot(QUrl url){
	qDebug() << "AJAX request sent:" << url.toString();

	QMap<QString, QVariant> response;
	response["statusCode"] = QVariant(Sent);
	response["data"] = QVariant("Interest Has Sent");
 
	emit sent(response);
}


void XMLCCNxRequest :: error_slot(QString errorInfo){

	qDebug() << "AJAX request error:" << errorInfo;
	QMap<QString, QVariant> response;
	response["statusCode"] = QVariant(Error);
	response["data"] = QVariant(errorInfo);
 
	emit error(response);
}


void XMLCCNxRequest :: finished(){

	QMap<QString, QVariant> response;
    //QString datanew;
	response["statusCode"] = QVariant(Success);
    /*for(int i = 0; i < data.length()/2;i++){
       datanew[i] = data[i];
    }*/
    response["data"] = QVariant(data);

	emit finish(response);
}

void XMLCCNxRequest :: stopRequest(){
	m_ccnxImpl->stop();
}


bool XMLCCNxRequest :: paramsIsValid(QString params){

	qDebug() << "Params:" << params << " " << QString::compare(params.trimmed(), "");
	if(QString::compare(params.trimmed(), "") == 0)
		return true;

	QRegExp equation("[^=]*=[^=]*");
 	QStringList paramsList = params.split("&");
	QList<QString>::iterator it = paramsList.begin(), itEnd = paramsList.end();	
	for(; it != itEnd; it++){
		if(!equation.exactMatch(*it)){	
			QMap<QString, QVariant> response;
			response["statusCode"] = QVariant(Error);
			response["data"] = QVariant("Please check your params.");	
			emit error(response);
			qDebug() << "AJAX Params Error.";
			return false;
		}
	}

	return true;
}


