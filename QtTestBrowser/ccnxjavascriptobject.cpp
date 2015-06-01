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

#include "ccnxjavascriptobject.h"

#include <QDebug>
#include "xmlccnxrequest.h"
#include "ccnxwebpage.h"

CCNxJavaScriptObject :: CCNxJavaScriptObject(QObject *parent) : QObject(parent){

	
}


CCNxJavaScriptObject :: CCNxJavaScriptObject(CCNxWebPage *ccnxWebPage){

	m_ccnxWebPage = ccnxWebPage;
}

CCNxJavaScriptObject :: ~CCNxJavaScriptObject(){
	emit stopAllRequest();
}

QVariant CCNxJavaScriptObject :: createXMLCCNxRequest(){

	XMLCCNxRequest* xmlCCNxRequest = new XMLCCNxRequest();
	connect(this, SIGNAL(stopAllRequest()), xmlCCNxRequest,
		SLOT(stopRequest()));

	qDebug() << "New a XMLCCNxRequest Object";
	return QVariant::fromValue(xmlCCNxRequest);
}
 

void  CCNxJavaScriptObject :: setPage(QWebPage* page){
	m_ccnxWebPage = (CCNxWebPage*)page;
}


QWebPage*  CCNxJavaScriptObject :: page(){

	return m_ccnxWebPage;
}






