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


#include "ccnximpl.h"
#include "ccnxnetworkreply.h"

#include <QUrl>
#include <QObject>
#include <QDebug>
#include <QFile>
#include <QIODevice>
#include <QPushButton>
#include <QTextCodec>
#include <QCoreApplication>
using namespace std;

//feng new add
#define localname  "Alice"
char *interestprefix[50];
int interestCount = 0;


//lvp add upload
long blocksize = 1024;
CCNTrieTree ccntrietree;

/*CCNxImpl :: CCNxImpl(const QUrl &name, QObject *parent) 
	: QObject(parent)
{
	setName(name);
	index = 0;
	ccn = ccn_create();
}*/

//feng new add
int CCNxImpl :: compareLocalname(char *prefix)
{
    int pre = 0, last = 0, count = 0, i = 0;
    while(count != 7)
    {
        if(prefix[i] == '/')
        {
                count ++;
                if(count == 6)pre = i + 1;
                if(count == 7)
                {
                        last = i - 1;
                        break;
                }
                //记录pre，last。
        }
        i++;
    }
    if(strlen(localname) == (last - pre + 1))
    {
        for(i = pre; i <= last; i++)
        {
            if(localname[i - pre] != prefix[i])
            {
                return 0;
            }
        }
        return 1;
    }
    else return 0;
}
int CCNxImpl :: lenOfLine(char *s)
{
    int count = 0;
    if(!s)return 0;
    while(s[count]!='\n')count++;
    return count;
}
int CCNxImpl :: check(char *s,char *t)
{
    char *res=strstr(s,t);
    if(res)
    {
        if(lenOfLine(res)==strlen(t))
        return 1;
        else return 0;
    }
    else return 0;
}

int CCNxImpl :: content_append(char *content,char *s)
{
    char *s1 = strtok(s,"\n");
    while(s1)
    {
        if(check(content,s1))
        {
            s1 = strtok(NULL,"\n");
            continue;
        }
        else
        {
            strcat((char *)content,s1);
            strcat((char *)content,(char *)"\n");
            s1 = strtok(NULL,"\n");
        }
    }
    return 1;
}

CCNxImpl :: CCNxImpl(const QUrl &name)
{
	setName(name);
	index = 0;
	header = false;
	stopped = false;
	ccn = ccn_create();
}


CCNxImpl :: CCNxImpl(const QUrl &name, QByteArray outgoingData, bool responseFromServer)
{
	this->outgoingData = outgoingData;
	this->responseFromServer = responseFromServer;

	//qDebug() <<  "CCNxImpl --> responseFromServer:" << responseFromServer;
	setName(name);
	index = 0;
	header = false;
	stopped = false;
	ccn = ccn_create();
}


CCNxImpl :: ~CCNxImpl()
{
	if(ccn != NULL)
		ccn_destroy(&ccn);

	ccn = NULL;

    contentBuf.clear();
	contentInfo.clear();
}


void CCNxImpl :: run()
{
	qDebug() << "-->new Thread running...";

	begin(true);
}


void CCNxImpl :: stop(){
	stopped = true;
}


struct ccn_charbuf* CCNxImpl :: 
make_template_normal(int allow_stale, int scope)
{
	struct ccn_charbuf *templ = ccn_charbuf_create();
	ccn_charbuf_append_tt(templ, CCN_DTAG_Interest, CCN_DTAG);
    ccn_charbuf_append_tt(templ, CCN_DTAG_Name, CCN_DTAG);
	ccn_charbuf_append_closer(templ); // </Name> 
	// XXX - use pubid if possible
	ccn_charbuf_append_tt(templ, CCN_DTAG_MaxSuffixComponents, CCN_DTAG);
	ccnb_append_number(templ, 1);
	ccn_charbuf_append_closer(templ); // </MaxSuffixComponents> 
	
	if (allow_stale) {
		ccn_charbuf_append_tt(templ, CCN_DTAG_AnswerOriginKind, CCN_DTAG);
		ccnb_append_number(templ, CCN_AOK_DEFAULT | CCN_AOK_STALE);
		qDebug() <<  "AnswerOriginKind:" << (CCN_AOK_DEFAULT | CCN_AOK_STALE); 
		ccn_charbuf_append_closer(templ); // </AnswerOriginKind> 
	}

    /*qDebug() <<  "CCNxImpl --> responseFromServer:" << (responseFromServer);
    if (responseFromServer){
		ccn_charbuf_append_tt(templ, CCN_DTAG_AnswerOriginKind, CCN_DTAG);
		ccnb_append_number(templ, 0);
		qDebug() <<  "AnswerOriginKind:" << 0; 
		ccn_charbuf_append_closer(templ); // </AnswerOriginKind> 
    }*/
		
   if (scope >= 0 && scope <= 2) {
      ccnb_tagged_putf(templ, CCN_DTAG_Scope, "%d", scope);
    	}

	ccn_charbuf_append_closer(templ); // </Interest> 
	return(templ);
}

//feng new add
struct ccn_charbuf *CCNxImpl ::
make_template(int allow_stale, int scope)
{
    struct ccn_charbuf *templ = ccn_charbuf_create();
    ccnb_element_begin(templ, CCN_DTAG_Interest);
    ccnb_element_begin(templ, CCN_DTAG_Name);
    ccnb_element_end(templ); /* </Name> */
    // XXX - use pubid if possible
    ccnb_element_begin(templ, CCN_DTAG_MaxSuffixComponents);
    ccnb_append_number(templ, 1);
    ccnb_element_end(templ); /* </MaxSuffixComponents> */
    if (allow_stale) {
        ccnb_element_begin(templ, CCN_DTAG_AnswerOriginKind);
        ccnb_append_number(templ, CCN_AOK_DEFAULT | CCN_AOK_STALE);
        ccnb_element_end(templ); /* </AnswerOriginKind> */
    }
    if (scope >= 0 && scope <= 2) {
        ccnb_tagged_putf(templ, CCN_DTAG_Scope, "%d", scope);
    }
    ccnb_element_end(templ); /* </Interest> */
    return(templ);
}
int CCNxImpl :: ccncat2(char *URI, char content_check[])
{
    int allow_stale = 0;
    int scope = -1;
    int pipeline = 4;
    int assumeFixed = 0; // variable only for now
    int count = 0;
    int res;
    struct ccn *ccn = NULL;
    struct ccn_fetch *fetch;
    struct ccn_charbuf *name = ccn_charbuf_create();
    struct ccn_charbuf *templ = NULL;
    unsigned char buf[600000];
    buf[0] = '\0';
    content_check[0] = '\0';
    res = ccn_name_from_uri(name, (const char *)URI);
    if (res < 0)
    {
        printf("bad ccn uri\n");
        exit(1);
    }
    ccn = ccn_create();
    if (ccn_connect(ccn, NULL) == -1)
    {
        perror("Could not connect to ccnd");
        exit(1);
    }
    templ = make_template(allow_stale, scope);
    fetch = ccn_fetch_new(ccn);
    struct ccn_fetch_stream *stream = ccn_fetch_open(fetch, name, (const char *)URI, templ, pipeline, CCN_V_HIGHEST, assumeFixed);
    while ((res = ccn_fetch_read(stream, buf, sizeof(buf))) != 0)
    {
        if (res > 0)
        {
            count = 0;
            fwrite(buf, res, 1, stdout);
            strcat((char*)content_check, (char *)buf);
            if(res<2000) break;
        }
        else if (res == CCN_FETCH_READ_NONE)
        {
            count++;
            if(count >= 4) break;
            fflush(stdout);
            if (ccn_run(ccn, 1000) < 0)
            {
                printf("error during ccn_run\n");
                exit(1);
            }
        }
        else if (res == CCN_FETCH_READ_END)
        {
            break;
        }
        else if (res == CCN_FETCH_READ_TIMEOUT)
        {
            ccn_reset_timeout(stream);
            fflush(stdout);
            if (ccn_run(ccn, 1000) < 0) {
                printf("error during ccn_run\n");
                exit(1);
            }
            break;
        }
        else
        {
            printf("fatal stream error: fetch error\n");
            exit(1);
        }
    }
    stream = ccn_fetch_close(stream);
    fflush(stdout);
    fetch = ccn_fetch_destroy(fetch);
    ccn_destroy(&ccn);
    ccn_charbuf_destroy(&name);
    ccn_charbuf_destroy(&templ);
    return 0;
}

char *CCNxImpl :: combinestring(char *a,char *b)
{
    if(a == NULL)
    {
        char *c = (char *)malloc(sizeof(char)*strlen(b)+1);
        strcpy(c,b);
        return c;
    }
    else
    {
        int num1 = strlen(a);
        int num2 = strlen(b);
        char * c = (char *)malloc((num1+num2)*sizeof(char)+1);
        c[0] = '\0';
        strcpy(c,a);
        strcat(c,b);
        return c;
    }
}
char *CCNxImpl :: URIStructure(char *responseURI,char *keyword)
{
    int i = 0;
    int count = 0;
    while(responseURI[i] != '\0')
    {
           if(responseURI[i] == '/')
           {
                  count ++;
                  if(count == 7) break;
           }
           i++;
    }
    count = i;
    int len = strlen(responseURI)-count;
    char* result = (char *)malloc(len * sizeof(char) + 1);
    for(i = 0; i < len - 1; i++)
    {
            if(responseURI[count + 1 + i] == '/')break;
            result[i] = responseURI[ count + 1 + i];
    }
    result[i] = '\0';
    const char * string = "ccnx:/bupt.edu.cn/C2Searcher/unicast/request/";
    char * string2 = combinestring((char*)string, keyword);
    char * string3 = combinestring(string2, (char*)"/");
    char * string4 = combinestring(string3, result);
    char * string5 = combinestring(string4, (char*)"/");
    result = combinestring(string5, (char*)localname);
    free(string2);
    string2 = NULL;
    free(string3);
    string3 = NULL;
    free(string4);
    string4 = NULL;
    free(string5);
    string5 = NULL;
    return result;
}

enum ccn_upcall_res CCNxImpl :: incoming_interest(struct ccn_closure *selfp,
                  enum ccn_upcall_kind kind,
                  struct ccn_upcall_info *info)
{
    struct ccn_charbuf *flatname = ccn_charbuf_create();
    struct ccn_charbuf *uri = ccn_charbuf_create();
    int flag = 0;
    char *s = NULL;
    switch (kind) {
        case CCN_UPCALL_FINAL:
            return(CCN_UPCALL_RESULT_OK);
        case CCN_UPCALL_INTEREST:
            if(info->pi->answerfrom!=0)
                return(CCN_UPCALL_RESULT_OK);
            //struct ccn_charbuf *flatname = ccn_charbuf_create();
            //struct ccn_charbuf *uri = ccn_charbuf_create();
            ccn_flatname_append_from_ccnb(flatname, info->interest_ccnb, info->pi->offset[CCN_PI_E], 0, -1);
            ccn_uri_append_flatname(uri, flatname->buf, flatname->length, 0);
            s = ccn_charbuf_as_string(uri);
            flag = compareLocalname(s);
            if(flag == 1)
            {
                interestprefix[interestCount] = ccn_charbuf_as_string(uri);
                interestCount = (interestCount + 1) % 50;

            }
            return(CCN_UPCALL_RESULT_OK);
        case CCN_UPCALL_CONTENT:
            return(CCN_UPCALL_RESULT_OK);
        case CCN_UPCALL_INTEREST_TIMED_OUT:
            return(CCN_UPCALL_RESULT_OK);
        default:
            return(CCN_UPCALL_RESULT_ERR);
    }
}

void CCNxImpl :: interestexpress_withoutaction(struct ccn *h,char* namestring,int AnswerOringinKind)
{
    int res;
    int timeout_ms;
    struct ccn_charbuf *templ = ccn_charbuf_create();

    h = ccn_create();
    if (ccn_connect(h, NULL) == -1)
      printf("connect error\n");
    struct ccn_charbuf *name2 = NULL;
    name2 = ccn_charbuf_create();
    res = ccn_name_from_uri(name2, namestring);
        if (res < 0)
           printf("error\n");
    ccn_charbuf_append_tt(templ, CCN_DTAG_Interest, CCN_DTAG);
     ccn_charbuf_append(templ, name2->buf, name2->length);
     ccnb_tagged_putf(templ, CCN_DTAG_AnswerOriginKind, "%d", AnswerOringinKind);
     ccn_charbuf_append_closer(templ);
    ccn_put(h,templ->buf,templ->length);
     res = -1;
    timeout_ms = 50;
    res = ccn_run(h, timeout_ms);
    if (res != 0)
       printf("ccn_run error!\n");
    ccn_charbuf_destroy(&templ);
    ccn_charbuf_destroy(&name2);
}
char *CCNxImpl :: lowercase(char *s)
{
    char *result = (char*)malloc(sizeof(char)*strlen(s)+1);
    strcpy(result,s);
    result[strlen(s)] = '\0';
    int i = 0;
    while(result[i]!='\0')
    {
        if(s[i]>='A'&&s[i]<='Z')
        {
            result[i] = result[i] + 32;
        }
        i++;
    }
    return result;
}
char *CCNxImpl :: drawingkeywords(char *URIbroadcast)
{
    int i =0;
    int count = 0;
    int left=0,right=0;
    while(URIbroadcast[i]!='\0')
    {
           if(URIbroadcast[i]=='/')
           {
                  count ++;
                  if(count==5)left=i;
                  if(count==6)right=i;
           }
           i++;
    }
    char *result = (char *)malloc((right - left)*sizeof(char)+1);
    for(i=left+1;i<right;i++)
    {
             result[i-left-1]=URIbroadcast[i];
    }
    result[i-left-1]='\0';
    return result;
}
char *CCNxImpl :: FuzzySearch(char * keyword, int &length)
{

    struct ccn *h = NULL;
    int timeout_ms;
    int res;
    clock_t timeToEnd = 4;
    time_t starttime;
    time_t endtime;
    char content_check[200000];
    char content_send[500000];
    content_send[0]='\0';
    struct ccn_charbuf *name = ccn_charbuf_create();
    struct ccn_closure in_interest;
    in_interest.p = &incoming_interest;
    char* URI = (char*)"ccnx:/bupt.edu.cn/C2Searcher/broadcast/request/";

    char *name2_1 = combinestring(URI,keyword);
    char *name2_2 = combinestring(name2_1,(char*)"/");
    char *name2 = combinestring(name2_2,(char*)localname);
    free(name2_1);
    name2_1 = NULL;
    free(name2_2);
    name2_2 = NULL;
    h = ccn_create();
    if (ccn_connect(h, NULL) == -1)
      printf("connect error\n");
    res = ccn_name_from_uri(name, "ccnx:/bupt.edu.cn/");
    if (res < 0)
    {
        fprintf(stderr, "Can not convert name to URI\n");
        exit(1);
    }
    res = ccn_set_interest_filter(h, name, &in_interest);
    if (res < 0) {
        fprintf(stderr, "Failed to register interest (res == %d)\n", res);
        exit(1);
    }
    interestexpress_withoutaction(h,name2,32);
    free(name2);
    name2 = NULL;
    time(&starttime);
    interestCount = 0;
    for(;;)
    {
        res = -1;
        timeout_ms = 50;
        while(--interestCount!=-1)
        {
            char *keyword_2 = drawingkeywords(interestprefix[interestCount]);
            char *interestprefixtobesent=URIStructure(interestprefix[interestCount],keyword_2);
            ccncat2(interestprefixtobesent,content_check);
            free(interestprefixtobesent);
            interestprefixtobesent = NULL;
            if(content_check[0]!='\0')
            {
                if(content_send[0]=='\0')
                {
                   strcat((char*)content_send,(char*)content_check);
                }
                else
                {
                   content_append((char *)content_send,(char *)content_check);
                }
            }
            else break;
            time (&starttime);
        }
        interestCount=0;
        time(&endtime);
        if(endtime - starttime >= timeToEnd)break;

        res = ccn_run(h, timeout_ms);
            if (res != 0)
                printf("ccn_run error!\n");
    }
    ccn_destroy(&h);
    ccn_charbuf_destroy(&name);


    if(content_send[0]=='\0')return (char*)"No result;";
    else
    {
        char *ds = (char *)malloc(strlen(content_send)+1);
        ds[0] = '\0';
        length = strlen(content_send)+1;
        strcat(ds,content_send);
        return ds;
    }
}

//lvp add upload
ssize_t CCNxImpl::read_full(int fd, unsigned char *buf, size_t size){
    size_t i;
    ssize_t res = 0;
    for (i = 0; i < size; i += res) {
        res = read(fd, buf+i , size-i );
        if (res == -1) {
            if (errno == EAGAIN || errno == EINTR)
                res = 0;
            else
                return(res);
        }
        else if (res == 0)
            break;
    }
    return(i);

}

enum ccn_upcall_res
CCNxImpl::incoming_interest_up(struct ccn_closure *selfp,
                  enum ccn_upcall_kind kind,
                  struct ccn_upcall_info *info){
    struct ccn_charbuf *flatnamet=NULL;
    struct ccn_charbuf *urii=NULL;
    struct ccn_charbuf *cob;

    CCNTrieTree* ccnTrieTree=( CCNTrieTree* )selfp->data;

    flatnamet = ccn_charbuf_create();
    urii = ccn_charbuf_create();
    cob = ccn_charbuf_create();


    ccn_charbuf_reset(flatnamet);
    ccn_flatname_append_from_ccnb(flatnamet,info->interest_ccnb,info->pi->offset[CCN_PI_E],0,-1);
    ccn_charbuf_reset(urii);
    ccn_uri_append_flatname(urii,flatnamet->buf,flatnamet->length,0);

    string iname =ccn_charbuf_as_string(urii) ;


     cob->buf= ccnTrieTree->findPath(iname)->getCCNFileNode()->getBuf();
     cob->length=ccnTrieTree->findPath(iname)->getCCNFileNode()->getLength();
       int res;

       switch (kind) {
           case CCN_UPCALL_FINAL:
               break;
           case CCN_UPCALL_INTEREST:
               if (ccn_content_matches_interest(cob->buf, cob->length,
                       1, NULL,
                       info->interest_ccnb, info->pi->offset[CCN_PI_E],
                       info->pi)) {
                   res = ccn_put(info->h, cob->buf, cob->length);
                   if (res < 0) {
                              fprintf(stderr, "ccn_put failed (res == %d)\n", res);
                              //exit(1);
                              return (CCN_UPCALL_RESULT_ERR);
                          }

               }
               break;
           default:
               break;
       }
       return(CCN_UPCALL_RESULT_OK);

}

enum ccn_upcall_res
CCNxImpl::incoming_content_up(struct ccn_closure *selfp,
              enum ccn_upcall_kind kind,
              struct ccn_upcall_info *info){

        return(CCN_UPCALL_RESULT_ERR);

}

void CCNxImpl::express_signal(char *keyname){
    string s(keyname);
    string t="/post_notification";
    t=t+s;
    char *name_full = (char *)t.data();
    cout<<name_full<<endl;


    struct ccn *h = NULL;
    struct ccn_closure in_content;
    int res;
    int timeout_ms;

    struct ccn_charbuf *templ = ccn_charbuf_create();

    h = ccn_create();
    if (ccn_connect(h, NULL) == -1) //连接到ccnd
        printf("connect error\n");

    in_content.p = &incoming_content_up;
    struct ccn_charbuf *name = NULL;
    name = ccn_charbuf_create();
    res = ccn_name_from_uri(name, name_full);
    //将CCNX-SCHEME URI转换为ccnb格式转换结果存储在name
    if (res < 0)
        printf("error\n");
    /*发送一次封装兴趣包，加入头字段信息的方法*/
    ccn_charbuf_append_tt(templ, CCN_DTAG_Interest, CCN_DTAG);
    ccn_charbuf_append(templ, name->buf, name->length);
    ccnb_tagged_putf(templ, CCN_DTAG_AnswerOriginKind, "%d", 1); //AnswerOringinKind=0表示不从CS中应答
    ccn_charbuf_append_closer(templ); /* </Interest> */
   /*h是ccnd处理句柄，name是请求数据包的URI，incontent.p是回调函数，temp1是兴趣包中除contentName以外的其他字段，如签名等信息*/

    ccn_express_interest(h, name, &in_content, templ);
    //兴趣包发送,ccnx://upload/keyname

    ccn_charbuf_destroy(&templ);
    ccn_charbuf_destroy(&name);

    res = -1;
    timeout_ms = 50;
    res = ccn_run(h, timeout_ms);
    if (res != 0) {

        printf("ccn_run error!\n");
    }

}

void CCNxImpl::setCaChe(char *keyname,char *address){
    struct ccn *ccn = NULL;
    struct ccn_charbuf *root = NULL;
    struct ccn_charbuf *name = NULL;
    struct ccn_charbuf *temp = NULL;
    struct ccn_charbuf *templ = NULL;
    struct ccn_charbuf *flatname=NULL;
    struct ccn_charbuf *uri=NULL;

    int timeout = -1;

    struct ccn_signing_params sp = CCN_SIGNING_PARAMS_INIT;

    int i;
    int status = 0;
    int res;
    ssize_t read_res;
     //  ssize_t call_res;
    unsigned char *buf = NULL;

    struct ccn_closure in_interest = {in_interest.p=&incoming_interest_up, in_interest.data=&ccntrietree,0,0};

    name = ccn_charbuf_create();
    res = ccn_name_from_uri(name, keyname);//change name form to ccnb form
    if (res < 0) {
          fprintf(stderr, "bad CCN URI: %s\n", keyname);
           exit(1);
       }
    ccn = ccn_create();
    if (ccn_connect(ccn, NULL) == -1) {
           perror("Could not connect to ccnd");
           exit(1);
       }


    root = name;
    name = ccn_charbuf_create();
    temp = ccn_charbuf_create();

    ccn_charbuf_append(name, root->buf, root->length);
    ccn_set_interest_filter(ccn, name, &in_interest);


    flatname = ccn_charbuf_create();
    uri= ccn_charbuf_create();

   freopen(address,"r",stdin);

   for (i = 0;; i++) {
           buf = (unsigned char *)calloc(1, blocksize);
           templ = ccn_charbuf_create();
          // ccn_charbuf_reset(templ);
           read_res = read_full(0, buf, blocksize);
           if (read_res < 0) {
               perror("read");
               read_res = 0;
               status = 1;
           }

   ccn_charbuf_reset(name);//
   ccn_charbuf_append(name, root->buf, root->length);//
   ccn_charbuf_reset(temp);//
   ccn_charbuf_putf(temp, "%d", i);//
   ccn_name_append(name, temp->buf, temp->length);
   ccn_charbuf_reset(temp);
   ccn_charbuf_append(temp, buf, read_res);


   ccn_charbuf_reset(templ);
   res = ccn_sign_content(ccn, templ, name, &sp, buf, read_res);


   if (res != 0) {
    fprintf(stderr, "Failed to sign ContentObject (res == %d)\n", res);
    exit(1);
    }
                   /* Put the keylocator in the first block only. */

   sp.type = CCN_CONTENT_DATA;
   //change name form
   ccn_charbuf_reset(flatname);
   ccn_flatname_append_from_ccnb(flatname,name->buf,name->length,0,-1);
   ccn_charbuf_reset(uri);
   ccn_uri_append_flatname(uri,flatname->buf,flatname->length,0);

            //insert data
   CCNFileNode* filenode = new CCNFileNode (templ->length);
   filenode->setCob(templ->buf);
   string cname =ccn_charbuf_as_string(uri) ;

   cout<<cname<<endl;


   ccntrietree.addPath(ccntrietree.getRoot(),cname,filenode);
   cout<<ccntrietree.findPath(cname)->getCCNFileNode()->getBuf()<<endl;


   if (read_res < blocksize)
       break;

   }


    res = ccn_run(ccn, timeout);
     if (in_interest.intdata == 0)
               exit(1);

    free(buf);
    buf = NULL;
    ccn_charbuf_destroy(&root);
    ccn_charbuf_destroy(&name);
    ccn_charbuf_destroy(&temp);
    ccn_destroy(&ccn);
    exit(status);

}



void CCNxImpl :: expressInterest(QUrl name){

	/*buttonTest = new QPushButton("Click Button to Get Resource:\n" + name.toString());
	connect(buttonTest, SIGNAL(clicked()), this, SLOT(receiveContent()));
	buttonTest->show();*/

	setName(name);

    //feng new add
   if(name.toString().startsWith("ccnx:/bupt.edu.cn/C2Searcher/broadcast/request/",Qt::CaseSensitive)){
       //int length = 11;
       //char *buf = (char*)"wang;you;feng;";
       char *keyword= outgoingData.data();
       keyword += 9;
       int length = strlen(keyword);
       char *buf = FuzzySearch(const_cast<char*>(keyword), length);

       if(!this->header){
           this->contentBuf.clear();

           // Test in local environment when response without headers
           this->contentBuf = "Type:"+getMIMETYPE().toUtf8()+"\r\n";
               for(int i=this->contentBuf.size(); *buf != '\0'; i++){
               this->contentBuf[i] = *buf++;
           }

           // used when server is running for reponse with headers
           /*for(int j = 0;*buf != '\0';){
               this->contentBuf[j] = *buf++;
           }*/

           this->header = true;

           //qDebug() << name.toString() <<"Header From Stream length :" << res ;
           this->receiveHeader();
       }else{
           this->contentBuf.clear();
           for(int j = 0;j < length;j++){
               this->contentBuf[j] = *buf++;
           }
           //qDebug() << name.toString() <<"Content From Stream length :" << res ;
           this->receiveContent();
       }
       expressNextInterest();
   }
   else if(name.toString().startsWith("ccnx:/upload/",Qt::CaseSensitive)){
       char *keyword = outgoingData.data();
       //ssskeyword +=9;
       string address(keyword);
       string s;
       cout<<address<<endl;
       int lastSlave = address.rfind("\\",address.length());
       s = address.substr(lastSlave+1,address.length());
       string temp="/";
       temp =temp+s;

       char *keyname = (char*)temp.data();
       string temp2="/upload";
       temp2 +=temp;
       char *real_address = (char*)temp2.data();

       //send upload interest
       express_signal(keyname);
       cout<<"hello"<<endl;
       //save data and send data
       setCaChe(keyname,real_address);
       cout<<"hello222"<<endl;

       expressNextInterest();



   }

   else{
       struct ccn_fetch *fetch = NULL;
       struct ccn_charbuf *ccn_name = NULL;
       struct ccn_charbuf *templ = NULL;
       const char *arg = NULL;
       //int dflag = 0;
       int allow_stale = 0;
       int scope = -1;
       int pipeline = 4;
       unsigned char buf[8192];
       //int i;
       int res;
       //int opt;
       int assumeFixed = 0; // variable only for now

       ccn_name = ccn_charbuf_create();
        QByteArray name_str = name.toString().toLatin1();
        const char *name_char = name_str.data();
        res = ccn_name_from_uri(ccn_name, name_char);

        if (responseFromServer){
            /*const char *params = "?";
            ccn_name_append(ccn_name, params, 1);
            const char *outgoingData_char = outgoingData.data();
            if(outgoingData != "")
                ccn_name_append(ccn_name, outgoingData_char, outgoingData.size());*/
        }

       templ = make_template_normal(allow_stale, scope);

       fetch = ccn_fetch_new(ccn);

       struct ccn_fetch_stream *stream = ccn_fetch_open(fetch, ccn_name, arg, templ, pipeline, CCN_V_HIGHEST, assumeFixed);
       if (NULL == stream) {
        return;
         }

        emit interestExpressed(name);
        while ((res = ccn_fetch_read(stream, buf, sizeof(buf))) != 0 && !stopped) {
            if (res > 0) {
                if(!this->header){
                    this->contentBuf.clear();

                    // Test in local environment when response without headers
                    /*this->contentBuf = "Type:"+getMIMETYPE().toUtf8()+"\r\n";
                    for(int i=this->contentBuf.size(), j=0; j<res; i++, j++){
                        this->contentBuf[i] = buf[j];
                    }*/

                    // used when server is running for reponse with headers
                    for(int j=0; j<res; j++){
                        this->contentBuf[j] = buf[j];
                    }

                    this->header = true;

                    qDebug() << name.toString() <<"Header From Stream length :" << res ;
                    this->receiveHeader();
                }else{
                    this->contentBuf.clear();
                    for(int i=0; i<res; i++){
                        this->contentBuf[i] = buf[i];
                    }
                    //qDebug() << name.toString() <<"Content From Stream length :" << res ;
                    this->receiveContent();
                }

                } else if (res == CCN_FETCH_READ_NONE) {
            //qDebug() << name.toString() << "CCN_FETCH_READ_NONE";
                    if (ccn_run(ccn, 1000) < 0) {
                        break;
                    }
                } else if (res == CCN_FETCH_READ_END) {
            //qDebug() << name.toString() << "CCN_FETCH_READ_END";
                    break;
                } else if (res == CCN_FETCH_READ_TIMEOUT) {
                   // qDebug()<< name.toString()  << "CCN_FETCH_READ_TIMEOUT";
                    ccn_reset_timeout(stream);
                    if (ccn_run(ccn, 1000) < 0) {
                break;
                    }
                } else {
                    // fatal stream error; shuld report this!
             //qDebug() << name.toString() << "CCN_FETCH_READ_OTHER";
            break;

                }
            }


       stream = ccn_fetch_close(stream);
       fetch = ccn_fetch_destroy(fetch);
       ccn_destroy(&ccn);
       ccn_charbuf_destroy(&ccn_name);

        /*this->receiveContent();*/

        expressNextInterest();
    }
}

/*void CCNxImpl :: expressInterest(QUrl name)
{
	setName(name);
	//buttonTest = new QPushButton("Click Button to Get Resource:\n" + name.toString());
	//connect(buttonTest, SIGNAL(clicked()), this, SLOT(receiveContent()));
	//buttonTest->show();

	struct ccn_closure in_content = {0,0,0,0};
	int res = -1;
	int timeout_ms;
	struct ccn_charbuf *templ = ccn_charbuf_create();
   
	in_content.p = &(CCNxImpl::incoming_content);

	in_content.data = this;
	struct ccn_charbuf *ccn_name = NULL;
	ccn_name = ccn_charbuf_create();

	QByteArray name_str = name.toString().toLatin1();
	const char *name_char = name_str.data();
	res = ccn_name_from_uri(ccn_name, name_char); 
        if (res < 0){
		qDebug() << "CCN Name transfer error";
		return;
	}

	//ccn_name_append(ccn_name, "0", 1);
	ccn_charbuf_append_tt(templ, CCN_DTAG_Interest, CCN_DTAG);
	ccn_charbuf_append(templ, ccn_name->buf, ccn_name->length); 
	//ccnb_tagged_putf(templ, CCN_DTAG_AnswerOriginKind, "%d", 0);
	ccn_charbuf_append_closer(templ);
	ccn_express_interest(ccn, ccn_name, &in_content, templ);

	qDebug() << "****************";
	qDebug() << "interest express, name =" << name_char;
	qDebug() << "****************";
    
	emit interestExpressed(name);
	res = -1;
	timeout_ms = -1;
	res = ccn_run(ccn, timeout_ms);

	qDebug() << "****************";
	qDebug() << "interest express finished, name =" << name_char;
	qDebug() << "****************";

	if (res != 0){
		qDebug() << "CCN ccn_run error!";
		return;	
	}

	expressNextInterest();
}*/


enum ccn_upcall_res CCNxImpl :: 
	incoming_content(struct ccn_closure *selfp,
		          enum ccn_upcall_kind kind,
		          struct ccn_upcall_info *info)
{
		
	CCNxImpl* ccnximpl = (CCNxImpl*)selfp->data;			

	const unsigned char *ccnb = NULL;
	size_t ccnb_size = 0;
	const unsigned char *data = NULL;
	size_t size = 0;    
	int res;

	
	switch (kind) {
		case CCN_UPCALL_FINAL:
            qDebug() << "incoming_content CCN_UPCALL_FINAL";
			ccn_set_run_timeout(info->h, 0);
			return(CCN_UPCALL_RESULT_OK);
		case CCN_UPCALL_CONTENT:
			ccnb = info->content_ccnb;
			ccnb_size = info->pco->offset[CCN_PCO_E];	   
			res = ccn_content_get_value(ccnb, ccnb_size, info->pco, &data, &size);
		    	if (res < 0) 
				abort();
				
			for(int i=0; i<size; i++){
				ccnximpl->contentBuf[i] = data[i];
			}
			qDebug() << "incoming_content CCN_UPCALL_CONTENT, Size" << size;
			//ccnximpl->length = size;
			//ccnximpl->buffer = data;
			ccn_set_run_timeout(info->h, 0);
			ccnximpl->receiveContent();
			//printf("****************\n");
			//printf("In object, content receive, ccnb = %s, size=%d", int_exp->buffer, (int)int_exp->length);
			//printf("\n****************\n");			
		    	return(CCN_UPCALL_RESULT_OK);
		case CCN_UPCALL_INTEREST_TIMED_OUT:
			return(CCN_UPCALL_RESULT_OK);
		default:
			return(CCN_UPCALL_RESULT_ERR);
	}
}


void CCNxImpl :: begin(bool async)
{	
	if (ccn_connect(ccn, NULL) == -1){
		qDebug() << "CCN connect error";
		emit error("CCN connect error");
		return;
	}	
	
	/*if(async)
		connect(this,  SIGNAL(willExpressInterest(QUrl)), this, SLOT(expressInterest(QUrl)), Qt::QueuedConnection);			
	else
		connect(this,  SIGNAL(willExpressInterest(QUrl)), this, SLOT(expressInterest(QUrl)), Qt::AutoConnection);

	connect(this,  SIGNAL(willExpressNextInterest()), this, SLOT(expressNextInterest()), Qt::QueuedConnection);

	this->async = async;*/
    this->latencyTimer.start();

	expressInterest(this->name);
}


void CCNxImpl :: setName(const QUrl &name)
{
	this->name = name; 
}


void CCNxImpl :: receiveContent()
{	
	/*buttonTest->hide();*/

	/* test video and audio
	contentBuf = data();
	parseContent();
	*/

	//qDebug() << "Content Type:" << contentInfo["Type"].value<QString>(); 

    QByteArray content_o = contentInfo["content"].value<QByteArray>();
	if(contentInfo["Type"].value<QString>().contains("text/html")
		|| contentInfo["Type"].value<QString>().contains("audio") 
		|| contentInfo["Type"].value<QString>().contains("video")){			
		//qDebug() << "content_o Size:" << content_o.size();		
		contentInfo["content"] = content_o+contentBuf;

		/*if(contentInfo["Type"].value<QString>().contains("audio")){
			// adjust cache size for audio. 
			if(content_o .size() >= 204800){
				emit contentReadyRead(content_o, contentInfo["Type"].value<QString>());
				contentInfo["content"] = contentBuf;
			}
		}	

		if(contentInfo["Type"].value<QString>().contains("video")){
			// adjust cache size for video. 
			if(content_o .size() >= 409600){
				emit contentReadyRead(content_o, contentInfo["Type"].value<QString>());
				contentInfo["content"] = contentBuf;
			}
		}*/

	}
	else{
		contentInfo["content"] = contentBuf;
		emit contentReadyRead(contentInfo["content"].value<QByteArray>(), contentInfo["Type"].value<QString>());	
	}	

}


void CCNxImpl :: receiveHeader(){

	parseContent();

	qDebug() << "Content Type:" << contentInfo["Type"].value<QString>(); 
    qDebug() << "----------------------------\n" << name.toString() << "Latency Time:" << this->latencyTimer.elapsed() << "\n----------------------------";\
    this->downloadTimer.start();

	if(!contentInfo["Type"].value<QString>().contains("text/html")
		&& !contentInfo["Type"].value<QString>().contains("audio") 
		&& !contentInfo["Type"].value<QString>().contains("video"))		
		emit contentReadyRead(contentInfo["content"].value<QByteArray>(), contentInfo["Type"].value<QString>());

}


void CCNxImpl :: expressNextInterest(){

	if(namesList.size()==0){

		QString type = contentInfo["Type"].value<QString>();
		QByteArray content;
		int suffix_length = 0;
		if(type.contains("png") 
			|| type.contains("jpg") 
			|| type.contains("bmp") 
			|| type.contains("gif") 
			|| type.contains("jpeg")){

			suffix_length = 14;

		}else if(type.contains("text/html")){
			suffix_length = 512;
		}

		for(int i = 0; i<suffix_length; i++){	
			content[i] = i%2==0?'\r':'\n';
		}
		
		qDebug() << "In expressNextInterest() Content Type:" << type;
       // QByteArray content_o = contentInfo["content"].value<QByteArray>();
		if(type.contains("text/html")
			|| type.contains("audio") 
			|| type.contains("video")){			
            QByteArray content_o = contentInfo["content"].value<QByteArray>();
			contentInfo["content"] = content_o+content;
		}
		else{
            //contentInfo["content"] = content_o;
            contentInfo["content"] = content;
		}

        qDebug() << "----------------------------\n" << name.toString() << "Download Time:" << this->downloadTimer.elapsed() << "\n----------------------------";
		emit contentReadyRead(contentInfo["content"].value<QByteArray>(), contentInfo["Type"].value<QString>());
			
		emit finished();
			
	}else{
        expressInterest(namesList.at(0));
	}	
}


void CCNxImpl :: parseContent(){

	QTextCodec *codec = QTextCodec::codecForName("KOI8-R");
	QString contentRaw = codec->toUnicode(contentBuf);
	QStringList contentList = contentRaw.split("\r\n");
	QByteArray content_o = contentInfo["content"].value<QByteArray>();
	//contentInfo.clear(); 

	if(contentList.size() < 2){
		qDebug() << "Packet parsing error.length:" << contentList.size();
 		emit error("Packet parsing error.");
	}else{
		QString header = contentList.at(0);
		//QString names = contentList.at(1);
		QByteArray content = contentBuf.remove(0, header.size()+2);

		//qDebug() << "-->Content Size:" << content.size();
	
		parseHeader(header);
		//parseNames(names);

		qDebug() << "Parsing first package finished, Header:" << header;
	
		if(contentInfo["Type"].value<QString>().contains("text/html")){			
			//qDebug() << "content_o Size:" << content_o.size();
			contentInfo["content"] = content_o+content;
		}
		else{
			contentInfo["content"] = content;	
		}	
	}
}


void CCNxImpl :: parseHeader(QString header){

	QList<QString> headerList = header.split("\n");

	if(headerList.size() == 0){
		qDebug() << "Packet header parsing error.";
 		emit error("Packet header parsing error.");
	}else{
		//contentInfo["status"] = headerList.at(0);

		QRegExp pair("[^:]*:[^:]*");
		QList<QString>::iterator it = headerList.begin(), itEnd = headerList.end();	
		for(; it != itEnd; it++){
			if(pair.exactMatch(*it)){
				 	
				contentInfo[(*it).split(":").at(0)] = (*it).split(":").at(1);
				//qDebug() << "header:" << (*it).split(":").at(0) << (*it).split(":").at(1);
			}
		}
	}

	QString type = contentInfo["Type"].value<QString>();
	if(type == NULL || QString::compare(type.trimmed(), "") == 0)
		contentInfo["Type"] = getMIMETYPE();	
}


void CCNxImpl :: parseNames(QString names){

	this->namesList.clear();

	QList<QString> namesList;
	if(QString::compare(names.trimmed(), ""))
		namesList = names.split("\n"); 

	QList<QString>::iterator it = namesList.begin(), itEnd = namesList.end();	
	for(; it != itEnd; it++){
		QUrl name(*it);
        this->namesList.append(name);
	}		
}


QByteArray CCNxImpl :: data(){

	const int PACKET_SIZE = 1024;

	QByteArray content;
	QString type = "";
	QString authority_name = name.authority();
	QString fragment_name = name.fragment();
	
	bool ok;
	qint64 fragment = 0;
	QRegExp number("\\d+");	
	if(number.exactMatch(fragment_name))
		fragment = fragment_name.toInt(&ok, 10);
					
	
	QFile file(QCoreApplication::applicationDirPath()+"/../../test_resource/"+authority_name);
	
	qDebug() << "ccnx Reply:"+QCoreApplication::applicationDirPath()+"/../../test_resource/"+authority_name << "Name" << name.toString();
	if(file.open(QIODevice::ReadOnly)){
		//qDebug() << "open and read File:"+QCoreApplication::applicationDirPath()+"/../../test_resource/"+authority_name;
		content = file.readAll();
		file.close();
		type = getMIMETYPE();
	}else{
		QString content_copy(
			"No Such file!");
		content = content_copy.toUtf8();
		type = "text/html; charset=UTF-8";
	}

	qDebug() << "File Size:" << content.size();

	int length = content.size();
	int suffix_length = 0;
	if(authority_name.endsWith("png") 
		|| authority_name.endsWith("jpg") 
		|| authority_name.endsWith("bmp") 
		|| authority_name.endsWith("gif") 
		|| authority_name.endsWith("jpeg")){

		suffix_length = 14;

	}else if(authority_name.endsWith("html")){
		suffix_length = 512;
	}

	for(int i = 0; i<suffix_length; i++){	
		content[length+i] = i%2==0?'\r':'\n';
	}

	qDebug() << "Total Size:" << content.size();

	qint64 start = PACKET_SIZE*fragment;
	qint64 end = PACKET_SIZE*(fragment+1);

	QString next_name = "";
	if(end > content.size()){
		end = content.size();		
	}else{
		next_name = name.scheme()+"://"+authority_name+"#"+QString::number((fragment+1), 10);
	}

	qDebug() << "Send Start:" << start << "End:" << end << "NEXT-Name:" << next_name;
	content.remove(end, content.size()-end);
	content.remove(0, start);

	QString header = "CCNx OK\nLength:"+QString::number(end-start, 10)+"\nType:"+type+"\r\n"+next_name+"\r\n";
	content = header.toUtf8()+content;

	return content;
}

QString CCNxImpl::getMIMETYPE()
{
	QString entire_name = name.toString();
    //qDebug() << "In CCNxImple::getMIMETYPE(), Entire Url:" << entire_name;
	QString base_name = name.authority();
    //qDebug() << "In CCNxImple::getMIMETYPE(), Base Url:" << base_name;

	if(base_name.endsWith(".html") || entire_name.endsWith(".html")){
		return "text/html; charset=UTF-8";
	}else if(base_name.endsWith(".xml") || entire_name.endsWith(".xml")){
		return "text/xml";
	}else if(base_name.endsWith(".xhtml") || entire_name.endsWith(".xhtml")){
		return "xhtml application/xhtml+xml";
	}else if(base_name.endsWith(".rtf") || entire_name.endsWith(".rtf")){
		return "appliction/rtf";
	}else if(base_name.endsWith(".pdf") || entire_name.endsWith(".pdf")){
		return "appliction/pdf";
	}else if(base_name.endsWith(".doc") || base_name.endsWith(".docx")){
		return "appliction/msword";
	}else if(base_name.endsWith(".png") || entire_name.endsWith(".png")){
		return "image/png";
	}else if(base_name.endsWith(".gif") || entire_name.endsWith(".gif")){
		return "image/gif";
	}else if(base_name.endsWith(".bmp") || entire_name.endsWith(".bmp")){
		return "image/bmp";
	}else if(base_name.endsWith(".jpg") || base_name.endsWith(".jpeg")  || entire_name.endsWith(".jpg")  || entire_name.endsWith(".jpeg")){
		return "image/jpeg";
	}else if(base_name.endsWith(".au")  || entire_name.endsWith(".au")){
		return "audio/basic";
	}else if(base_name.endsWith(".mid") || base_name.endsWith(".midi") || entire_name.endsWith(".mid") || entire_name.endsWith(".midi")){
		return "audio/midi,audio/x-mid";
	}else if(base_name.endsWith(".ra") || base_name.endsWith(".ram") || entire_name.endsWith(".ra") || entire_name.endsWith(".ram")){
		return "audio/x-pn-realaudio";
	}else if(base_name.endsWith(".mpg") || base_name.endsWith(".mpeg") 
			|| entire_name.endsWith(".mpg") || entire_name.endsWith(".mpeg")){
		return "video/mpeg";
	}else if(entire_name.endsWith(".mp4") || base_name.endsWith(".mp4")){
		return "video/mp4";
	}else if(base_name.endsWith(".mp3") || entire_name.endsWith(".mp3")){
		return "audio/mpeg";
	}else if(base_name.endsWith(".ogg") || base_name.contains(".ogg") || entire_name.endsWith(".ogg")){
		return "video/ogg";
	}else if(base_name.endsWith(".avi") || entire_name.endsWith(".avi")){
		return "video/x-msvideo";
	}else if(base_name.endsWith(".gz") || entire_name.endsWith(".gz")){
		return "appliction/x-gzip";
	}else if(base_name.endsWith(".tar") || entire_name.endsWith(".tar")){
		return "appliction/x-tar";
	}else if(base_name.endsWith(".txt") || entire_name.endsWith(".txt")){
		return "text/plain";
	}else if(base_name.endsWith(".js") || entire_name.endsWith(".js")){
		return "application/x-javascript";
	}else if(base_name.endsWith(".css") || entire_name.endsWith(".css")){
		return "text/css";
    }else if(base_name.endsWith(".jsp") || entire_name.endsWith(".jsp")){
        return "text/html; charset=UTF-8";
    }else{

       return "application/octet-stream";//application/octet-stream

	}
}


void CCNxImpl :: receiveInterest()
{

} 


/*void CCNxImpl :: abort()
{

}*/


QByteArray CCNxImpl :: htmlCoding()
{

	QByteArray html;

	QString base_name = name.toString();
		
	QFile file(QCoreApplication::applicationDirPath()+"/../../test_resource/"+base_name);
	
	if(file.open(QIODevice::ReadOnly)){
		html = file.readAll();
		file.close();
	}

	return html;
}


QByteArray CCNxImpl :: imageCoding()
{

	QString base_name = name.toString();

	if(base_name.endsWith(".png")){

		/*const char png_Buf[] = {0x89,0x50,0x4e,0x47,0x0d,0x0a,0x1a,0x0a,0x00,0x00,0x00,0x0d,0x49,0x48,0x44,0x52,
					0x00,0x00,0x00,0x61,0x00,0x00,0x00,0x20,0x08,0x03,0x00,0x00,0x00,0x35,0xdc,0xe6,
					0x58,0x00,0x00,0x00,0x5d,0x50,0x4c,0x54,0x45,0xff,0xff,0xff,0xfb,0xfb,0xfe,0xef,
					0xf0,0xfd,0xe2,0xe3,0xfb,0xca,0xcc,0xf8,0xb3,0xb6,0xf4,0xf0,0x88,0x83,0x5d,0x64,
					0xe8,0x4e,0x56,0xe6,0x29,0x32,0xe1,0x8f,0x93,0xef,0x84,0x89,0xee,0x7d,0x83,0xed,
					0xed,0x6b,0x67,0x73,0x79,0xec,0xe9,0x51,0x48,0xf3,0x9c,0x9a,0xe8,0x43,0x3e,0x9a,
					0x9e,0xf1,0xe7,0x37,0x33,0x3d,0x45,0xe4,0x30,0x39,0xe2,0xe1,0x14,0x08,0xf7,0xc4,
					0xc1,0xe1,0x06,0x02,0xfb,0xe2,0xe0,0xf6,0xb5,0xb4,0xe3,0x15,0x11,0xf9,0xd0,0xcf,
					0xe4,0x22,0x1b,0xfd,0xf0,0xf0,0x0a,0x7e,0x5e,0x4d,0x00,0x00,0x00,0x01,0x74,0x52,
					0x4e,0x53,0x00,0x40,0xe6,0xd8,0x66,0x00,0x00,0x02,0x6c,0x49,0x44,0x41,0x54,0x78,
					0x5e,0xb5,0x95,0x87,0xce,0xea,0x30,0x0c,0x46,0x9d,0x39,0xba,0xf7,0x60,0xbd,0xff,
					0x63,0x5e,0xbb,0x49,0x08,0x70,0x0b,0xe2,0x97,0xca,0x91,0x40,0x51,0x91,0x72,0xea,
					0xcf,0x4e,0x80,0xef,0xe9,0x4c,0x25,0xfc,0x4a,0x70,0xf8,0x05,0xb9,0x73,0x6e,0x20,
					0x05,0x37,0xce,0x55,0x0c,0x0e,0x47,0x38,0xa2,0x02,0x60,0xc6,0x2f,0x0e,0x27,0x73,
					0xc4,0xc8,0xa0,0x73,0x1b,0x02,0x8e,0xa6,0x8a,0x1b,0x0f,0x7e,0xa1,0x7e,0x66,0x10,
					0xee,0x57,0x86,0x2e,0xa4,0x24,0xdc,0x0f,0x52,0x52,0xb9,0x62,0xa1,0xd3,0x05,0xb0,
					0xd1,0x11,0xf6,0xe8,0x7c,0x0c,0xdf,0x8a,0xb0,0xdc,0x4f,0xad,0x1b,0xe5,0x91,0x15,
					0x84,0x97,0x07,0x99,0x29,0x14,0x00,0x43,0xe3,0x20,0x81,0x2b,0x25,0x8e,0x6d,0x31,
					0x4f,0x4e,0x2e,0x19,0x48,0x0a,0x2b,0x83,0x23,0x88,0xd3,0xf9,0x12,0x8b,0x3d,0x70,
					0xa0,0xc6,0x3d,0x43,0x98,0x28,0x03,0x47,0x60,0x93,0x81,0x67,0xc6,0x64,0x9b,0x4a,
					0x3a,0x0f,0x3b,0xee,0x20,0x0c,0x0c,0x40,0xfa,0xc0,0x0a,0xf6,0x37,0x43,0xdb,0x07,
					0xea,0xe6,0xb6,0xf7,0xe3,0x85,0xdb,0x90,0xb8,0x20,0x01,0x91,0xdf,0x0d,0x23,0xbc,
					0x43,0x4f,0x93,0x06,0xcf,0x79,0xb9,0x73,0x3e,0x41,0x24,0xfd,0x38,0x03,0x2f,0x6c,
					0x25,0xfd,0x9d,0x1a,0x50,0xc0,0xef,0x7d,0x98,0xa7,0x27,0x34,0x4c,0xcf,0xcc,0x64,
					0x48,0xac,0x7b,0x86,0xf4,0xff,0x60,0x0a,0xdf,0x94,0x51,0x86,0x19,0x96,0x5f,0x1a,
					0x9a,0x0b,0x51,0xa2,0x41,0x7f,0xfe,0x03,0x52,0xf1,0x70,0x0c,0x82,0xe7,0x83,0x33,
					0x34,0xac,0xab,0x26,0x4a,0x14,0x94,0xb4,0x98,0x01,0x17,0x73,0x8b,0x1f,0x64,0x9a,
					0xda,0xf9,0x46,0x86,0x10,0x4e,0xbf,0x2c,0x25,0x40,0x53,0x7a,0x4e,0xb1,0x0f,0x00,
					0xe1,0x20,0x27,0x03,0x29,0x52,0xe2,0x44,0x4d,0x01,0x11,0x2b,0x1a,0xea,0xa7,0x1a,
					0x92,0x81,0x9d,0xb7,0x1a,0xca,0x25,0x70,0x5d,0x53,0x4a,0x9c,0x7a,0x60,0x2d,0x86,
					0x33,0x58,0xec,0x37,0x7e,0x0d,0x12,0x02,0x2f,0xa1,0xec,0x1a,0xca,0x06,0xd1,0x58,
					0xc2,0x72,0x01,0xd0,0xfd,0x06,0x2a,0x7a,0x16,0x0d,0x02,0xe3,0xcf,0x38,0x20,0x55,
					0x46,0xf7,0x54,0x87,0x5f,0xa3,0x4c,0x86,0x3a,0xd0,0x7b,0x83,0x86,0xda,0x0f,0x12,
					0xca,0x1e,0x67,0xe9,0xb9,0x0d,0x97,0x09,0x37,0x0f,0x06,0x86,0x82,0x1c,0x98,0x52,
					0x32,0x19,0x28,0xb3,0xbb,0x01,0x02,0x3a,0x19,0x7a,0x32,0x7e,0x34,0x50,0x57,0x4e,
					0xc1,0x40,0x67,0x8e,0x43,0x81,0x75,0x3c,0x19,0x46,0xf1,0xc1,0x10,0x68,0x00,0x7c,
					0x4a,0x84,0x46,0x05,0x3d,0x38,0xd5,0x3d,0x31,0x25,0x83,0xdd,0x8e,0xd6,0xf0,0x62,
					0x20,0xe1,0x7e,0x1f,0xf4,0x23,0x90,0x3a,0x4d,0x3d,0xbe,0x02,0xe8,0x25,0x12,0x0d,
					0xcc,0x5f,0xde,0xd5,0x7f,0x06,0xf3,0xce,0xf0,0xa8,0x7c,0x34,0xb4,0xb8,0x29,0xbb,
					0x2d,0xa1,0xa6,0x6b,0xaa,0x61,0x74,0xb4,0x29,0xcf,0x0a,0x05,0x55,0x0e,0x3c,0x2f,
					0xe4,0x8b,0x61,0x0e,0x94,0xc1,0x40,0x21,0xd5,0x27,0x7d,0x9d,0xa6,0x6b,0xf3,0x90,
					0x52,0x8d,0x82,0x33,0x5c,0x96,0xa5,0x7d,0xed,0x83,0xa1,0xd0,0x15,0x10,0x95,0x1f,
					0x21,0x6f,0x28,0xde,0xf6,0x01,0x8f,0xdc,0x86,0xbe,0xf9,0x3e,0x24,0x34,0x50,0x0d,
					0x6d,0x89,0x9c,0x93,0x41,0x39,0x62,0x28,0x10,0xeb,0x9c,0x25,0x06,0x32,0xa8,0xd7,
					0x1a,0xea,0x68,0x80,0xb5,0xdf,0x04,0xf8,0xec,0xd9,0x50,0xb3,0xbd,0x3e,0x50,0x11,
					0x7b,0x18,0xf6,0xf1,0xc4,0x61,0x46,0xc4,0xc3,0xed,0xdd,0x96,0xbe,0x1f,0xa7,0x32,
					0x72,0x89,0xb7,0x06,0xab,0xbc,0x60,0xb4,0xa6,0x30,0x36,0x0a,0xc4,0xce,0x99,0xae,
					0x81,0x0c,0x44,0x73,0x5b,0x35,0x2d,0xe0,0x4b,0x44,0x97,0x2b,0x19,0xb6,0xe4,0xaa,
					0x2b,0x8a,0x5c,0x32,0xf8,0x8e,0x7f,0x74,0xed,0x55,0x27,0x40,0x87,0xfa,0x74,0x00,
					0x00,0x00,0x00,0x49,0x45,0x4e,0x44,0xae,0x42,0x60,0x82};
		
		int png_length = 795;

		QByteArray png;
		for(int i=0; i<png_length; i++){
			png[i] = png_Buf[i];
		}*/

		QByteArray png;
		QFile file("/home/test/WebKit-r52686/WebKitBuild/test_resource/web.png");
		if(file.open(QIODevice::ReadOnly)){
			png = file.readAll();
			file.close();
		}

		return png;
	}else if(base_name.endsWith(".jpg")){

		/*const char jpg_Buf[] = {0xff,0xd8,0xff,0xe0,0x00,0x10,0x4a,0x46,0x49,0x46,0x00,0x01,0x01,0x00,0x00,0x01
					,0x00,0x01,0x00,0x00,0xff,0xdb,0x00,0x43,0x00,0x08,0x06,0x06,0x07,0x06,0x05,0x08
					,0x07,0x07,0x07,0x09,0x09,0x08,0x0a,0x0c,0x14,0x0d,0x0c,0x0b,0x0b,0x0c,0x19,0x12
					,0x13,0x0f,0x14,0x1d,0x1a,0x1f,0x1e,0x1d,0x1a,0x1c,0x1c,0x20,0x24,0x2e,0x27,0x20
					,0x22,0x2c,0x23,0x1c,0x1c,0x28,0x37,0x29,0x2c,0x30,0x31,0x34,0x34,0x34,0x1f,0x27
					,0x39,0x3d,0x38,0x32,0x3c,0x2e,0x33,0x34,0x32,0xff,0xdb,0x00,0x43,0x01,0x09,0x09
					,0x09,0x0c,0x0b,0x0c,0x18,0x0d,0x0d,0x18,0x32,0x21,0x1c,0x21,0x32,0x32,0x32,0x32
					,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32
					,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32
					,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,0xff,0xc0
					,0x00,0x11,0x08,0x00,0x20,0x00,0x61,0x03,0x01,0x22,0x00,0x02,0x11,0x01,0x03,0x11
					,0x01,0xff,0xc4,0x00,0x1f,0x00,0x00,0x01,0x05,0x01,0x01,0x01,0x01,0x01,0x01,0x00
					,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09
					,0x0a,0x0b,0xff,0xc4,0x00,0xb5,0x10,0x00,0x02,0x01,0x03,0x03,0x02,0x04,0x03,0x05
					,0x05,0x04,0x04,0x00,0x00,0x01,0x7d,0x01,0x02,0x03,0x00,0x04,0x11,0x05,0x12,0x21
					,0x31,0x41,0x06,0x13,0x51,0x61,0x07,0x22,0x71,0x14,0x32,0x81,0x91,0xa1,0x08,0x23
					,0x42,0xb1,0xc1,0x15,0x52,0xd1,0xf0,0x24,0x33,0x62,0x72,0x82,0x09,0x0a,0x16,0x17
					,0x18,0x19,0x1a,0x25,0x26,0x27,0x28,0x29,0x2a,0x34,0x35,0x36,0x37,0x38,0x39,0x3a
					,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a
					,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a
					,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8a,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99
					,0x9a,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7
					,0xb8,0xb9,0xba,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xd2,0xd3,0xd4,0xd5
					,0xd6,0xd7,0xd8,0xd9,0xda,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xf1
					,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa,0xff,0xc4,0x00,0x1f,0x01,0x00,0x03
					,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x01
					,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0xff,0xc4,0x00,0xb5,0x11,0x00
					,0x02,0x01,0x02,0x04,0x04,0x03,0x04,0x07,0x05,0x04,0x04,0x00,0x01,0x02,0x77,0x00
					,0x01,0x02,0x03,0x11,0x04,0x05,0x21,0x31,0x06,0x12,0x41,0x51,0x07,0x61,0x71,0x13
					,0x22,0x32,0x81,0x08,0x14,0x42,0x91,0xa1,0xb1,0xc1,0x09,0x23,0x33,0x52,0xf0,0x15
					,0x62,0x72,0xd1,0x0a,0x16,0x24,0x34,0xe1,0x25,0xf1,0x17,0x18,0x19,0x1a,0x26,0x27
					,0x28,0x29,0x2a,0x35,0x36,0x37,0x38,0x39,0x3a,0x43,0x44,0x45,0x46,0x47,0x48,0x49
					,0x4a,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x63,0x64,0x65,0x66,0x67,0x68,0x69
					,0x6a,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x82,0x83,0x84,0x85,0x86,0x87,0x88
					,0x89,0x8a,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0xa2,0xa3,0xa4,0xa5,0xa6
					,0xa7,0xa8,0xa9,0xaa,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xc2,0xc3,0xc4
					,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,0xe2
					,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9
					,0xfa,0xff,0xda,0x00,0x0c,0x03,0x01,0x00,0x02,0x11,0x03,0x11,0x00,0x3f,0x00,0xf7
					,0xfa,0x2b,0x37,0xfe,0x12,0x2d,0x13,0xfe,0x83,0x3a,0x7f,0xfe,0x05,0x27,0xf8,0xd5
					,0xcb,0x6b,0xab,0x7b,0xc8,0xbc,0xdb,0x5b,0x88,0xa7,0x8f,0xa6,0xf8,0x9c,0x30,0xfc
					,0xc5,0x25,0x24,0xf6,0x65,0xca,0x9c,0xe2,0xaf,0x24,0xd1,0x35,0x14,0x54,0x17,0x57
					,0xb6,0xb6,0x31,0x09,0x6e,0xee,0x61,0xb7,0x8c,0x9d,0xa1,0xe6,0x90,0x20,0x27,0xd3
					,0x27,0xbf,0x06,0x9e,0xc4,0xa4,0xdb,0xb2,0x1e,0x67,0x85,0x67,0x58,0x1a,0x54,0x12
					,0xb0,0xca,0xc6,0x58,0x6e,0x23,0xd4,0x0a,0xf0,0x0d,0x2f,0xc4,0x1a,0xdb,0xfc,0x53
					,0x8a,0x66,0xb8,0x9c,0xcf,0x2e,0xa1,0xe4,0x4b,0x0e,0xe3,0x8d,0x85,0xf6,0x94,0xc7
					,0xa0,0x1f,0x96,0x33,0x54,0x3c,0x63,0x71,0xa9,0xaf,0xc4,0x5b,0xf9,0x43,0x4a,0x2f
					,0x16,0xeb,0xfd,0x18,0xa1,0x3b,0xb6,0xe7,0xf7,0x7b,0x7f,0x0c,0x57,0xaf,0x6b,0x9a
					,0xc7,0x84,0xfc,0x25,0xa9,0xc5,0xa8,0xea,0x36,0x91,0x2e,0xb3,0x3a,0x06,0x26,0x08
					,0x43,0x4b,0xd3,0x05,0xbd,0x07,0x71,0x9c,0xe4,0xfb,0xd7,0xd1,0xd2,0xc3,0xfd,0x4a
					,0x29,0x28,0xfb,0x47,0x56,0x3a,0x25,0xd3,0xfe,0x06,0xbb,0xf9,0x1c,0xee,0x5c,0xef
					,0xb5,0x8f,0x33,0xf1,0x5f,0x8e,0xb5,0xff,0x00,0xf8,0x4d,0xee,0x9a,0xd2,0xfa,0x7b
					,0x78,0xec,0xae,0x1a,0x18,0x6d,0xd1,0x88,0x42,0x15,0xb1,0xf3,0x2f,0x46,0xce,0x39
					,0xcf,0xae,0x2b,0xdf,0x62,0x67,0x78,0x51,0xa4,0x5d,0x8e,0x54,0x16,0x5f,0x43,0xe9
					,0x5e,0x7f,0x3f,0xfc,0x22,0x1a,0xb6,0x9d,0xa8,0xf8,0xd3,0x4b,0xb4,0x86,0xe3,0x50
					,0xb4,0x89,0xe5,0xf9,0xf7,0x0d,0xb2,0x85,0xca,0x97,0x4c,0xe3,0x3e,0xf8,0xe6,0xb9
					,0xdf,0x87,0x3e,0x3d,0xd7,0x75,0x3f,0x16,0xc5,0xa6,0x6a,0x77,0x66,0xee,0x1b,0xb5
					,0x7c,0x6e,0x45,0x06,0x36,0x55,0x2d,0x91,0x80,0x38,0xe0,0x8c,0x54,0x62,0xa8,0x7d
					,0x6a,0x87,0x35,0x28,0x72,0xfb,0x25,0xad,0xf4,0x7f,0xd2,0xdf,0x5e,0xe1,0x19,0x72
					,0xcb,0x57,0x7b,0x9e,0xcb,0x45,0x14,0x57,0xcf,0x9b,0x85,0x14,0x8e,0xeb,0x1a,0x33
					,0xbb,0x05,0x55,0x19,0x2c,0x4e,0x00,0x15,0x0d,0xa5,0xed,0xad,0xfc,0x02,0x7b,0x3b
					,0x98,0x6e,0x21,0x24,0x8f,0x32,0x19,0x03,0xae,0x7e,0xa2,0x8b,0x8e,0xce,0xd7,0xe8
					,0x4f,0x45,0x14,0x50,0x23,0xe7,0x7f,0x0a,0xbf,0x82,0xd6,0xce,0xe4,0x78,0x9a,0x29
					,0xda,0xe7,0xce,0xfd,0xd1,0x8c,0xbe,0x36,0x60,0x7f,0x74,0xfa,0xe6,0xba,0x7f,0x87
					,0x42,0xd6,0x4f,0x88,0x9a,0x83,0xf8,0x7b,0xcf,0x8f,0x44,0x58,0x3e,0x64,0x99,0xb9
					,0x3d,0x31,0xc7,0xfb,0xd9,0xc7,0x7c,0x7d,0x6a,0x87,0x85,0xe4,0xf1,0x2f,0x86,0x2d
					,0x2e,0x2d,0xff,0x00,0xe1,0x0c,0x9e,0xf7,0xcd,0x97,0xcc,0xdf,0x34,0x0d,0x91,0xc6
					,0x31,0xd3,0xda,0xb6,0xfc,0x2b,0xa3,0xf8,0x87,0x51,0xf1,0xfb,0x78,0x8e,0xf3,0x4b
					,0x1a,0x45,0xba,0xc6,0xca,0x62,0xd9,0xb0,0x3e,0x53,0x68,0x1b,0x7a,0x9e,0x70,0x49
					,0x3e,0x95,0xe5,0xd2,0x4e,0xf1,0xd3,0x5b,0xf6,0xfd,0x4f,0xb9,0xc7,0xd4,0x4e,0x35
					,0xdb,0x97,0xba,0xe3,0xa5,0xe6,0x9a,0xbe,0x9b,0x47,0x74,0xff,0x00,0x23,0x7e,0x7d
					,0x5f,0xc7,0xf1,0xbc,0x9e,0x5f,0x86,0xac,0x1e,0x35,0x27,0x69,0xfb,0x50,0xcb,0x0e
					,0xdc,0x6e,0xae,0x6f,0xc4,0xde,0x33,0xb3,0xf1,0x0f,0xc3,0xe8,0x6f,0xe7,0xd2,0x22
					,0x95,0xe3,0xd4,0x16,0x09,0x6d,0xa6,0x91,0xc2,0xab,0xec,0x63,0xb8,0x32,0x15,0x27
					,0x8f,0xe6,0x7e,0xb5,0x99,0x6f,0xe1,0xfd,0x4d,0x26,0xba,0x3e,0x21,0xf0,0xe6,0xb5
					,0xa9,0xdf,0x34,0xc4,0x9b,0x98,0x2e,0xf0,0x8c,0x3d,0xb1,0xfe,0x7e,0x95,0x55,0xbc
					,0x2d,0xae,0xa7,0xc3,0xc9,0xec,0xff,0x00,0xb2,0x2f,0x05,0xc3,0x6a,0xc9,0x2a,0xc2
					,0x23,0x2c,0xdb,0x3c,0xa6,0x19,0xe3,0xb6,0x78,0xaa,0x94,0xea,0x34,0xd6,0xbb,0x3d
					,0xff,0x00,0xe1,0x8e,0x7a,0x38,0x5c,0x2c,0x27,0x19,0x37,0x14,0xd3,0x8d,0x9c,0x5d
					,0xbd,0x7e,0xdb,0x7f,0x82,0x3d,0x1f,0xc5,0x1e,0x21,0xd3,0xbc,0x3f,0x05,0x84,0xab
					,0xa7,0x43,0x77,0xad,0xdc,0xaa,0xc7,0x63,0x16,0xc0,0x5f,0x27,0x00,0x7c,0xdd,0x42
					,0xe4,0x8f,0xaf,0xe6,0x47,0x07,0x73,0xe1,0xa9,0x6f,0x21,0xf1,0x15,0xd6,0xb9,0xa8
					,0xe9,0xf7,0xda,0xdb,0x24,0x78,0x48,0xa4,0x2c,0xd6,0xac,0x5c,0x2f,0x3c,0x00,0x00
					,0x04,0x0c,0x0c,0xd6,0xa7,0xc4,0xad,0x0f,0x58,0x8f,0x52,0xd1,0xbc,0x47,0xa6,0xdb
					,0xc9,0x39,0xb2,0x8a,0x35,0x74,0x45,0x2c,0x63,0x64,0x6d,0xe1,0x8a,0x8e,0x71,0xcf
					,0x3f,0x4a,0xcc,0x5d,0x42,0xe2,0xfa,0x2d,0x4e,0xeb,0x4c,0xf0,0x66,0xad,0x16,0xa1
					,0xa9,0x05,0x32,0xc8,0x37,0x3c,0x3b,0x83,0x87,0xc8,0xca,0x8e,0xa4,0x7a,0xf7,0xaf
					,0xb6,0xc1,0x43,0x93,0x0f,0x09,0xd2,0x7a,0xbd,0xdd,0xd6,0x96,0x6b,0x4d,0x5e,0x8a
					,0xdd,0xb7,0x3e,0x22,0xa3,0xbc,0xda,0x66,0x46,0x8b,0xa5,0xea,0xbe,0x1b,0xd6,0x3c
					,0x41,0x08,0x96,0xda,0x7b,0x7d,0x3e,0xdc,0x8d,0x42,0xdd,0x99,0x82,0x5c,0xc4,0xc3
					,0x95,0x1c,0x75,0xc1,0xe0,0xf6,0xaf,0x43,0xf8,0x7b,0xe1,0xcf,0x0a,0x04,0x1e,0x20
					,0xd0,0xde,0xe6,0x59,0x18,0x18,0xf6,0xdc,0x38,0x26,0xdc,0x9c,0x6e,0x5c,0x00,0x39
					,0xf7,0xe7,0x83,0xd7,0x9a,0xe3,0xb5,0x8d,0x76,0x68,0xff,0x00,0xb6,0x99,0xfc,0x21
					,0xa9,0xd9,0xdf,0xea,0xf0,0x08,0xa6,0x79,0x24,0x62,0x80,0xe3,0xaa,0x82,0x9e,0xde
					,0xb5,0xd6,0xfc,0x23,0xd0,0x6f,0xf4,0x7d,0x0e,0xee,0x7b,0xf8,0x64,0x80,0xdd,0xca
					,0xad,0x1c,0x52,0x02,0x18,0x2a,0x8e,0xa4,0x76,0xce,0x7f,0x4a,0xbc,0xc2,0x73,0x78
					,0x69,0x54,0x9b,0xe5,0x6e,0xcb,0x46,0xbd,0xed,0x35,0xd9,0xbe,0xb7,0xf9,0x13,0x04
					,0xb9,0xac,0x8e,0x3e,0xe7,0xc3,0xf2,0x78,0x87,0x5e,0xd7,0xed,0xf4,0x9d,0x15,0x64
					,0x9a,0xd6,0xea,0x44,0x79,0x9a,0xef,0x6e,0x09,0x76,0x00,0xe0,0xf1,0xfc,0x27,0x8a
					,0x8e,0x3f,0x0c,0xdc,0x78,0x72,0xf3,0x49,0x5d,0x67,0x46,0x5d,0xf3,0xdd,0xa4,0x6b
					,0x32,0xdd,0xe7,0x27,0x70,0xfe,0x15,0xf4,0x15,0x71,0xb4,0x58,0x97,0xc5,0x1a,0xfc
					,0xda,0xcf,0x85,0xb5,0xbb,0xf8,0xa5,0xbd,0x91,0xad,0x9e,0xd6,0x17,0x0b,0x8d,0xee
					,0x49,0xc8,0xc6,0x41,0xca,0xe2,0x87,0xd0,0xe0,0x9b,0x5a,0xd1,0xce,0x8b,0xe1,0x3d
					,0x6e,0xc4,0xa5,0xe2,0x34,0xd2,0x5d,0x44,0xfb,0x76,0x86,0x1d,0xce,0x40,0xfa,0xd7
					,0xc0,0xf2,0x75,0xb6,0xbf,0xf0,0x7b,0x5b,0xf5,0x3f,0x43,0x58,0x86,0xad,0x0e,0x6f
					,0x76,0xdf,0xfb,0x6f,0xf3,0x73,0xdf,0x7f,0xee,0xff,0x00,0x99,0x8f,0xaf,0xe9,0x8d
					,0x2d,0xf6,0xbb,0xa8,0xed,0xb6,0xf2,0xe3,0xd6,0xa4,0x85,0xde,0x46,0x70,0xc3,0x73
					,0x31,0x1c,0x0e,0x36,0xf0,0x73,0xdf,0xa6,0x2a,0xbb,0x69,0x31,0x5e,0x6b,0x3a,0x6d
					,0x95,0x83,0x68,0xf3,0xc9,0x71,0x21,0x4d,0xb6,0xcf,0x72,0x53,0xb7,0xfa,0xcd,0xfc
					,0x81,0xd7,0xee,0xfb,0xfb,0x57,0x49,0xa8,0x68,0xba,0x84,0x97,0x3a,0xf6,0x2c,0x35
					,0xf8,0xee,0x1f,0x51,0x9e,0x5b,0x59,0x2d,0x20,0x3e,0x5b,0x82,0xdf,0xc5,0x92,0x32
					,0x0e,0x06,0x08,0xcf,0x7a,0xaf,0x69,0xa0,0x6b,0xf0,0xdf,0x46,0xfa,0xc6,0x97,0xad
					,0xfd,0x9c,0x20,0x22,0x2d,0x3e,0x52,0xe5,0xcf,0xfb,0x4c,0x5c,0xed,0xf7,0x1f,0xca
					,0xa5,0xd3,0x6e,0x5b,0x1a,0x53,0xc5,0x45,0x53,0xf8,0xf5,0x4b,0x45,0x7f,0x25,0xd3
					,0xaf,0xe0,0x7a,0xc7,0xf6,0x5c,0xdf,0xdc,0xb6,0xff,0x00,0xbe,0x9e,0x8a,0xd7,0xa2
					,0xbd,0x7e,0x44,0x7c,0x37,0xb7,0x99,0xff,0xd9};
		
		int jpg_length = 1849;

		QByteArray jpg;
		for(int i=0; i<jpg_length; i++){
			jpg[i] = jpg_Buf[i];
		}*/

		QByteArray jpg;
		QFile file("/home/test/WebKit-r52686/WebKitBuild/test_resource/web.jpg");
		if(file.open(QIODevice::ReadOnly)){
			jpg = file.readAll();
			file.close();
		}

		return jpg;

	}else if(base_name.endsWith(".bmp")){

		QByteArray bmp;
		QFile file("/home/test/WebKit-r52686/WebKitBuild/test_resource/web.bmp");
		if(file.open(QIODevice::ReadOnly)){
			bmp = file.readAll();
			file.close();
		}

		return bmp;	
	}else if(base_name.endsWith(".gif")){

		QByteArray gif;
		QFile file("/home/test/WebKit-r52686/WebKitBuild/test_resource/web.gif");
		if(file.open(QIODevice::ReadOnly)){
			gif = file.readAll();
			file.close();
		}

		return gif;	
	}else if(base_name.endsWith(".jpeg")){

		QByteArray jpeg;
		QFile file("/home/test/WebKit-r52686/WebKitBuild/test_resource/test_css_name.jpeg");
		if(file.open(QIODevice::ReadOnly)){
			jpeg = file.readAll();
			file.close();
		}
		return jpeg;	
	}else{

		return NULL;
	}
}
