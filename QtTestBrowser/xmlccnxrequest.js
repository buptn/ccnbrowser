/*
 * Please include the javascript library <xmlccnxrequest.js> when using the following methods.
 *
 * USAGE Normal:
 * 	var xmlCCnxrequest = new XMLCCNxRequest();
 * 
 * 	xmlCCNxRequest.success = function(data){
 * 		//processure
 * 	};
 * 	xmlCCNxRequest.error = function(data){
 * 		//processure
 * 	};	
 * 
 * 	xmlCCNxRequest.send("POST", "ccnx://bupt/get.jsp", "t="+Math.random(), true, true);
 * 	xmlCCNxRequest.post("ccnx://bupt/get.jsp", "t="+Math.random(), true, true);
 * 	xmlCCNxRequest.get("ccnx://bupt/get.jsp?t="+Math.random(), true, true);
 * 
 * USAGE AJAX:
 * 
 * 	AJAX_CCNx({		
 * 		url: "ccnx://get.jsp",
 * 		type: "GET",
 * 		params: "t="+Math.random(),
 * 		sent: function(data){
 * 			//processure				
 * 		},
 * 		success: function(data){
 * 			//processure
 * 		},
 * 		error: function(data){
 * 			//processure
 * 		}				
 * 	});
 *
 * Tips: The parameter <params> will be deglected when using GET method <send>.
 */


function XMLCCNxRequest(){
	
	this.xmlCCNxRequest = ccnxRequest.createXMLCCNxRequest();

	this.beforeSend = function(){};
	this.success = function(data){};
	this.error = function(data){};
	this.sent = function(data){};
	this.async = true;
	this.responseFromServer = true;
	this.url = "";
	this.type = "GET";
	this.dataType = "text";
	this.params = "";

	this.send = function(type, url, params, async, responseFromServer){

		this.xmlCCNxRequest.sent.connect(this.sent);
		this.xmlCCNxRequest.error.connect(this.error);
		this.xmlCCNxRequest.success.connect(this.success);
		this.type = type;
		this.url = url;
		this.params = params;
		this.async = async;	
		this.responseFromServer = responseFromServer;
		this.beforeSend();	
		this.xmlCCNxRequest.send(type, url, params, async, this.responseFromServer);

	};

	this.send_inner = function(){

		this.xmlCCNxRequest.sent.connect(this.sent);
		this.xmlCCNxRequest.error.connect(this.error);
		this.xmlCCNxRequest.finish.connect(this.success);	
		this.beforeSend();
		this.xmlCCNxRequest.send(this.type, this.url, this.params, this.async, this.responseFromServer);
	};

	this.get = function(url, async, responseFromServer){

		this.xmlCCNxRequest.sent.connect(this.sent);
		this.xmlCCNxRequest.error.connect(this.error);
		this.xmlCCNxRequest.success.connect(this.success);
		this.type = "GET";
		this.url = url;
		this.async = async;	
		this.responseFromServer = responseFromServer;
		this.beforeSend();	
		this.xmlCCNxRequest.send(type, url, params, async, this.responseFromServer);
	};

	this.post = function(url, params, async, responseFromServer){

		this.xmlCCNxRequest.sent.connect(this.sent);
		this.xmlCCNxRequest.error.connect(this.error);
		this.xmlCCNxRequest.success.connect(this.success);
		this.type = "POST";
		this.url = url;
		this.params = params;
		this.async = async;	
		this.responseFromServer = responseFromServer;
		this.beforeSend();	
		this.xmlCCNxRequest.send(type, url, params, async, this.responseFromServer);
	}
}


function AJAX_CCNx(request){

	var ccnx_ajax = new XMLCCNxRequest();

	for (e in request){
		switch(e){
			case "beforeSend":
				ccnx_ajax.beforeSend = request.beforeSend
				break

			case "success":
				ccnx_ajax.success = request.success
				break

			case "error":
				ccnx_ajax.error = request.error
				break

			case "sent":
				ccnx_ajax.sent = request.sent
				break

			case "async":
				ccnx_ajax.async = request.async
				break

			case "url":
				ccnx_ajax.url = request.url
				break

			case "type":
				ccnx_ajax.type = request.type
				break

			case "dataType":
				ccnx_ajax.dataType = request.dataType
				break

			case "params":
				ccnx_ajax.params = request.params
				break

			case "responseFromServer":
				ccnx_ajax.responseFromServer = request.responseFromServer
				break

			default:
				break

		}
	}

	ccnx_ajax.send_inner();	
} 


