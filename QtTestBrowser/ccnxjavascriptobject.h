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

#ifndef ccnxjavascriptobject_h
#define ccnxjavascriptobject_h

#include <QObject>
#include "xmlccnxrequest.h"
#include "ccnxwebpage.h"

class CCNxJavaScriptObject : public QObject
{
	Q_OBJECT

public:
	explicit CCNxJavaScriptObject(QObject *parent = 0);
	CCNxJavaScriptObject(CCNxWebPage *ccnxWebPage);
	~CCNxJavaScriptObject();
	void setPage(QWebPage* page);
	QWebPage* page();		

public slots:
	QVariant createXMLCCNxRequest();	

signals:
	void stopAllRequest();

private:
	QList<XMLCCNxRequest*> m_xmlCCNxRequestList; 
	CCNxWebPage* m_ccnxWebPage;
	QString m_str;
				
};

#endif
