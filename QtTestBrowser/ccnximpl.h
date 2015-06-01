/*
 * Copyright (C) 2013 peng yue 
 * Copyright (C) 2014 wang you feng (wangyoufeng3364@163.com)
 * Copyright (C) 2014 lv peng (lvpp9527@gmail.com)
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

#ifndef ccnximpl_h
#define ccnximpl_h

#include <QObject>
#include <QUrl>
#include <QMap>
#include <QTime>
#include <QVariant>
#include <QThread>
#include <QPushButton>

#include "ccntrietree.h"
#include <errno.h>
#include <string>

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/select.h>
#include <sys/time.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <ccn/ccn.h>
#include <ccn/ccn_private.h> /* for ccn_process_scheduled_operations() */
#include <ccn/charbuf.h>
#include <ccn/lned.h>
#include <ccn/uri.h>
#include <ccn/fetch.h>

#include <ccn/keystore.h>
#include <ccn/signing.h>

#include <ccn/flatname.h>

#ifdef __cplusplus
}
#endif



class Q_NETWORK_EXPORT CCNxImpl : public QThread
{
	Q_OBJECT

public:
	//explicit CCNxImpl(const QUrl &name, QObject *parent = 0);
	CCNxImpl(const QUrl &name);
	CCNxImpl(const QUrl &name, QByteArray outgoingData, bool responseFromServer);
	~CCNxImpl();

	enum State {
		Unconnected,
		Connecting,
		Waiting,
		Downloading
	};

	enum Error {
		NoError,
		UnknownError,
		NotConnected
	};

	void setName(const QUrl &name);
	void begin(bool async);
	QByteArray imageCoding();
	QByteArray htmlCoding();

	//static CONTENT_PACKET_PROCESS OnContentReceived;
	
public slots:
	/*void abort();*/
	void receiveContent();
	void expressInterest(QUrl name);
    struct ccn_charbuf* make_template_normal(int allow_stale, int scope);
	void expressNextInterest();

signals:

	void stateChanged(int);
	void contentReadyRead(QByteArray, QString);
	void interestExpressed(QUrl);
	void willExpressInterest(QUrl);
	void willExpressNextInterest();
	void error(QString);
	void finished();

private :

	QUrl name;
	QList<QUrl> namesList;
	QString Interest;
	QString content; 
	QPushButton *buttonTest;
	bool async;
	int index;
	State state;
	QByteArray contentBuf;
	QByteArray outgoingData;
	QMap<QString, QVariant> contentInfo;
    QTime latencyTimer;
    QTime downloadTimer;
	struct ccn *ccn;

	bool header;
	bool stopped;
	bool responseFromServer;

public :	
	void stop();
	void receiveInterest();
	void receiveHeader();
	void parseContent();
	QByteArray data();
	QString getMIMETYPE();
	void parseHeader(QString);
	void parseNames(QString);
	static enum ccn_upcall_res incoming_content(struct ccn_closure *selfp,
		          enum ccn_upcall_kind kind,
		          struct ccn_upcall_info *info);
    //feng new add
    static int compareLocalname(char *prefix);
    int lenOfLine(char *s);
    int check(char *s,char *t);
    int content_append(char *content,char *s);
    struct ccn_charbuf *
    make_template(int allow_stale, int scope);
    int ccncat2(char *URI, char content_check[]);
    char *combinestring(char *a,char *b);
    char *URIStructure(char *responseURI,char *keyword);
    static enum ccn_upcall_res
    incoming_interest(struct ccn_closure *selfp,
                      enum ccn_upcall_kind kind,
                      struct ccn_upcall_info *info);
    void interestexpress_withoutaction(struct ccn *h,char* namestring,int AnswerOringinKind);
    char *lowercase(char *s);
    char *drawingkeywords(char *URIbroadcast);
    char *FuzzySearch(char * keyword,int &length);
	void run();

    //upload
    ssize_t read_full(int fd, unsigned char *buf, size_t size);
    void setCaChe(char *keyname,char *address);
    static enum ccn_upcall_res
    incoming_interest_up(struct ccn_closure *selfp,
                      enum ccn_upcall_kind kind,
                      struct ccn_upcall_info *info);

    static enum ccn_upcall_res incoming_content_up(struct ccn_closure *selfp,
                  enum ccn_upcall_kind kind,
                  struct ccn_upcall_info *info);
    void express_signal(char *keyname);


};

#endif
