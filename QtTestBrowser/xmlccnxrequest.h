#ifndef xmlccnxrequest_h
#define xmlccnxrequest_h

#include <QObject>
#include <QUrl>
#include <QMap>
#include <QVariant>
#include "ccnximpl.h"

class XMLCCNxRequest : public QObject
{
	Q_OBJECT

public:
	XMLCCNxRequest(QObject *parent = 0);
	~XMLCCNxRequest();

	enum State {
		Error = -1,
		Sent = 0,
		Success = 2
	};
	
	bool paramsIsValid(QString params);

public slots:
	void send(QString type, QString url, QString params, bool async, bool responseFromServer);
	void receive(QByteArray content, QString received_type);
	void sent_slot(QUrl);
	void finished();
	void stopRequest();	
	void error_slot(QString);

signals:
	void finish(QMap<QString, QVariant> ret);
	void error(QMap<QString, QVariant> ret);
	void sent(QMap<QString, QVariant> ret);

private:
	CCNxImpl *m_ccnxImpl;	
	QString data;
	QByteArray outgoingData;
};

#endif
