// http.hpp
#ifndef HTTP_HPP
#define HTTP_HPP

#include <iostream>
#include <string>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <tchar.h>
#include <time.h>
#include <new>
#include <fstream>
#include "getcmdoutput.hpp"
#pragma comment (lib, "Ws2_32.lib")

namespace http{
//#pragma warning(disable:4996)
	//define error code
	typedef enum{_NO_ERROR,_UNKNOW_ERROR,_WSA_STARTUP_ERROR,_SCREATE_ERROR,_SBIND_ERROR,
		_SLISTEN_ERROR,_SACCEPT_ERROR,_SRECV_ERROR,_SSEND_ERROR,_FILE_ERROR,_PARAMETER_ERROR}_ERROR_CODE;
	// method = token
	typedef enum { _GET, _HEAD, _POST, _PUT, _DELETE, _CONNECT,_OPTIONS, _TRACE, _ERROR_METHOD}HTTP_METHOD;
	// request-line   = method SP request-target SP HTTP-version CRLF
	// HTTP-version  = HTTP-name "/" DIGIT "." DIGIT 
	// HTTP-name     = %x48.54.54.50 ; "HTTP", case-sensitive
	// request-target = origin-form/absolute-form/authority-form/asterisk-form
	// origin-form    = absolute-path [ "?" query ]
	// absolute-path = 1*( "/" segment ) 
	// absolute-form  = absolute-URI
	// authority-form = authority
	// asterisk-form  = "*"
	typedef struct{
		HTTP_METHOD _method;
		std::string _request_target;
		std::string _version;
	}REQUEST_LINE;
	// status-line = HTTP-version SP status-code SP reason-phrase CRLF
	typedef struct{
		std::string _version;
		std::string _status_Code;
		std::string _reason_Phrase;
	}STATUS_LINE;


	// header-field   = field-name ":" OWS field-value OWS
	//field-name     = token     
	//field-value    = *( field-content / obs-fold )
	//field-content  = field-vchar [ 1*( SP / HTAB ) field-vchar ]
	//field-vchar    = VCHAR / obs-text
    //obs-fold       = CRLF 1*( SP / HTAB )
	// OWS            = *( SP / HTAB )
	// RWS            = 1*( SP / HTAB )
 
	//defined in: rfc7231
	typedef struct{
		std::string _all;
		std::string _Host; //GET,POST
		std::string _Accept; //GET,POST
		std::string _Accept_Encoding; //GET.POST
		std::string _Accept_Language; //GET,POST
		std::string _User_Agent; //GET,POST
		std::string _Connection; //GET,POST
		std::string _Cache_Control; //GET,POST
		//std::string _DNT; //GET,POST (IE11)
		std::string _Referer; //POST
		// Content-Length = 1*DIGIT
		std::string _Content_Type; //POST
		std::string _Content_Length; //POST
	}REQUEST_HEADER;
	typedef struct{
		std::string _Date;
		std::string _Server;
		std::string _X_Powered_By;
		std::string _Connection;
		std::string _Transfer_Encoding;
		std::string _Content_Type;
		std::string _Content_Length;
		std::string	_Accept_Ranges;
		std::string	_Cookie;
	}RESPONSE_HEADER;
	// HTTP-message   = start-line
	//					*( header-field CRLF )
	//					CRLF
	//					[ message-body ]
	// message-body = *OCTET
	typedef struct{
		REQUEST_LINE _request_line;
		REQUEST_HEADER _request_header;
		char* _message_body;
		std::string _post;
	}REQUEST_MESSAGE;
	typedef struct{
		STATUS_LINE _status_line;
		RESPONSE_HEADER _resopnse_header;
		std::string _message_body;
	}RESOPNSE_MESSAGE;
	class http_server{
	private:
		REQUEST_MESSAGE _request_message;
		RESOPNSE_MESSAGE _response_message;
		SOCKET _socket;
	public:
		std::string _port;
		_ERROR_CODE _error;
		int _extra_error;
	public:
		http_server(std::string port);
		http_server();
		virtual ~http_server();
		_ERROR_CODE Default();
		SOCKET Accept();
		_ERROR_CODE Send(SOCKET& socket);
		std::string query(std::string name);
		std::string get(std::string name);
		std::string post(std::string name);
		_ERROR_CODE setHtml(std::string html);
	private:
		_ERROR_CODE initialize(std::string path,std::string port);
		std::string recvLine(SOCKET& socket);
		std::string recvByte(SOCKET& socket,int byte=1);
		_ERROR_CODE init_http_request(SOCKET& socket);
		HTTP_METHOD stringToMethod(std::string str);
		std::string methodToString(HTTP_METHOD method);
		std::string getHeaderFidld(std::string str,std::string hack);
		//convert status code to reason phase
		std::string codeToReason(std::string code);
		//combine response message to string,
		std::string combineMessage();
		//to generate a GMT date format string
		std::string getNowDate();
		bool issetQuery(std::string name, int type = 0);
	};
}
using namespace http;
http_server::http_server():_port("6900"){
	http_server(this->_port);
}
http_server::http_server(std::string port){
	this->_port = port;
	this->_socket = INVALID_SOCKET;
	this->_error = _NO_ERROR;
	this->_extra_error = 0;
	_request_message._message_body = NULL;
	_request_message._post = "";
	_response_message._message_body = "";
}
http_server::~http_server(){
	shutdown(this->_socket, SD_BOTH);
	closesocket(this->_socket);
	WSACleanup();
}
_ERROR_CODE http_server::Default(){
	TCHAR* aaa=getcmdoutput(TEXT("ping.exe WWW.BAIDU.COM"));
	if(aaa) std::cout << aaa;
	if (_NO_ERROR == this->initialize("", "6900")){
		while (1){
			SOCKET s = this->Accept();
			if (_NO_ERROR != this->_error){
				closesocket(s);
				return this->_error;
			}
			if(_NO_ERROR != this->setHtml("")) return this->_error;
			if (_NO_ERROR != this->Send(s)) return this->_error;
		}
	}
	else
		return this->_error;
}
_ERROR_CODE http_server::initialize(std::string path, std::string port){
	this->_port = port;
	this->_error = _NO_ERROR;
	WSADATA _wsaData;
	this->_extra_error = WSAStartup(MAKEWORD(2, 2), &(_wsaData));
	if (this->_extra_error != NO_ERROR) {
		this->_error = _WSA_STARTUP_ERROR;
		return this->_error;
	}

	PADDRINFOA result = NULL;
	ADDRINFOA hints;
	SOCKADDR_IN sin;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	// Resolve the server address and port
	//GetAddrInfo
	this->_extra_error = getaddrinfo(NULL, this->_port.c_str(), &hints, &result);
	if (0 != this->_extra_error) {
		this->_extra_error = WSACleanup();
		this->_error=_WSA_STARTUP_ERROR;
		return this->_error;
	}
	this->_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (this->_socket == INVALID_SOCKET)
	{
		this->_extra_error = WSACleanup();
		this->_error = _SCREATE_ERROR;
		return this->_error;
	}
	// Setup the TCP listening socket
	this->_extra_error = bind(this->_socket, result->ai_addr, (int)result->ai_addrlen);
	if (this->_extra_error == SOCKET_ERROR) {
		this->_extra_error = WSAGetLastError();
		freeaddrinfo(result);
		closesocket(this->_socket);
		WSACleanup();
		this->_error = _SBIND_ERROR;
		return this->_error;
	}

	freeaddrinfo(result);

	this->_extra_error = listen(this->_socket, SOMAXCONN);
	if (this->_extra_error == SOCKET_ERROR) {
		this->_extra_error = WSAGetLastError();
		closesocket(this->_socket);
		WSACleanup();
		this->_error = _SLISTEN_ERROR;
		return this->_error;
	}
	return this->_error;
}
SOCKET http_server::Accept(){

	SOCKET clientSocket = accept(this->_socket, NULL, NULL);
	if (clientSocket == INVALID_SOCKET) {
		this->_extra_error = WSAGetLastError();
		WSACleanup();
		this->_error = _SACCEPT_ERROR;
		return INVALID_SOCKET;
	}
	this->_error = this->init_http_request(clientSocket);
	return clientSocket;	
}
_ERROR_CODE http_server::Send(SOCKET& socket){
	this->_response_message._status_line._version = this->_request_message._request_line._version;
	this->_response_message._status_line._status_Code = "200";


	if (SOCKET_ERROR == send(socket, combineMessage().c_str(), combineMessage().length() + 1, 0))
	{
		this->_extra_error = WSAGetLastError();
		closesocket(socket);
		this->_error = _SSEND_ERROR;
		return this->_error;
	}
	closesocket(socket);
	return this->_error;
}
_ERROR_CODE http_server::setHtml(std::string html){
	if (html.length()<=0)
		this->_response_message._message_body = "<html><body><form action=\"/\" method=\"post\"enctype=\"multipart/form-data\"><label for=\"file\"> \
	filename:</label><input type=\"file\" name=\"file\" id=\"file\" /><br /><input type=\"submit\" name=\"submit\" value=\"submit\" /></form>"
		"<form method=\"POST\" action=\"/handle_post_request\">"
		"Input 1: <input type=\"text\" name=\"input_1\" /> <br/>"
		"Input 2: <input type=\"text\" name=\"input_2\" /> <br/>"
		"<input type=\"submit\" value=\"post\"/>"
		"</form>"
		"</body></html>";
	else
		this->_response_message._message_body = html;
	this->_error= _NO_ERROR;
	return this->_error;
}
//function: recv a line from socket, "\r\n" included in the return string
std::string http_server::recvLine(SOCKET& socket){
	std::string ret="";
	while (1) {
		ret += recvByte(socket);
		if (this->_error != _NO_ERROR){
			return "";
		}
		if (ret.at(ret.length()-1) == '\n' && '\r' == ret.at(ret.length() - 2))
			return ret;		
	}
}
//function: recv any byte from socket
std::string http_server::recvByte(SOCKET& socket, int byte){	
	if (byte == 1)
	{
		char data = '\0';
		switch (recv(socket, &data, 1, 0)) {
		case 0: // not connected anymore;
		case SOCKET_ERROR:
				this->_extra_error = WSAGetLastError();// not connected anymore
				this->_error = _SRECV_ERROR;
				return "";
		default:
			return std::string(1,data);
		}
	}
	else if(byte>1) return recvByte(socket, byte - 1);
	else { this->_error = _PARAMETER_ERROR; return ""; }
}
//function: recv request message and initialize  _request_message;
_ERROR_CODE http_server::init_http_request(SOCKET& socket)
{
	//for initialize request line
	std::string tmp = recvLine(socket);
	if (this->_error != _NO_ERROR){this->_error = _SRECV_ERROR;return this->_error;}
	size_t firstpos = tmp.find(char(0x20));;
	size_t endpos = tmp.rfind(char(0x20));
	_request_message._request_line._method = this->stringToMethod(tmp.substr(0, firstpos));
	_request_message._request_line._request_target = tmp.substr(firstpos + 1, endpos - firstpos);
	_request_message._request_line._version = tmp.substr(endpos + 1, tmp.length() - endpos - 1 - 2);
	_request_message._request_header._all += tmp;
	//for initialize request header field
	tmp = recvLine(socket);
	if (this->_error != _NO_ERROR){ this->_error = _SRECV_ERROR; return this->_error; }
	_request_message._request_header._all += tmp;
	while (tmp != std::string("\r\n"))
	{
		size_t pos = 0;
		if (std::string::npos != tmp.find("Host"))
			_request_message._request_header._Host = getHeaderFidld(tmp, "Host");
		else if (std::string::npos != tmp.find("Accept"))
			_request_message._request_header._Accept = getHeaderFidld(tmp, "Accept");
		else if (std::string::npos != tmp.find("Accept-Encoding"))
			_request_message._request_header._Accept_Encoding = getHeaderFidld(tmp, "Accept-Encoding");
		else if (std::string::npos != tmp.find("Accept-Language"))
			_request_message._request_header._Accept_Language = getHeaderFidld(tmp, "Accept-Language");
		else if (std::string::npos != tmp.find("Connection"))
			_request_message._request_header._Connection = getHeaderFidld(tmp, "Connection");
		else if (std::string::npos != tmp.find("Cache-Control"))
			_request_message._request_header._Cache_Control = getHeaderFidld(tmp, "Cache-Control");
		else if (std::string::npos != tmp.find("User-Agent"))
			_request_message._request_header._User_Agent = getHeaderFidld(tmp, "User-Agent");
		else if (std::string::npos != tmp.find("Referer"))
			_request_message._request_header._Referer = getHeaderFidld(tmp, "Referer");
		else if (std::string::npos != tmp.find("Content-Type"))
			_request_message._request_header._Content_Type = getHeaderFidld(tmp, "Content-Type");
		else if (std::string::npos != tmp.find("Content-Length"))
			_request_message._request_header._Content_Length = getHeaderFidld(tmp, "Content-Length");
		else
			;
		tmp = recvLine(socket);
		_request_message._request_header._all += tmp;
	}
	std::cout << _request_message._request_header._all;
	
	//initialize request message body
	//Content-Type	multipart/form-data; boundary=-------------------------- - 7de411620276
	if (std::string::npos != _request_message._request_header._Content_Type.find("multipart/form-data")){
		int bound = _request_message._request_header._Content_Type.find("boundary");
		if (std::string::npos != bound){
			std::string strbound = _request_message._request_header._Content_Type.substr(bound + strlen("boundary="), _request_message._request_header._Content_Type.length() - bound - strlen("boundary="));
			recvLine(socket);//pass boundary line
			std::string disposition = recvLine(socket);
			std::string type = recvLine(socket);
			int pos = 0;
			std::string filename = "";
			if (std::string::npos != (pos = disposition.rfind("\\")))
				filename = disposition.substr(pos + 1, disposition.length() - pos - 1 - 3);
			else if(std::string::npos != (pos = disposition.rfind("/")))
				filename = disposition.substr(pos + 1, disposition.length() - pos - 1 - 2);
			else  if(std::string::npos != (pos = disposition.rfind("filename")))
				filename = disposition.substr(pos + strlen("filename=\"") + 1, disposition.length() - pos - strlen("filename=\"") - 1 - 3);

			recvLine(socket);//pass empty line
			char data = '\0';
			int iread = 0;
			std::string fr;
			bool found = false;
			while (!found){
				iread++;
				fr += this->recvByte(socket);
				if (this->_error != _NO_ERROR){
					this->_error = _SRECV_ERROR;
					return this->_error;
				}
				if (iread >= strbound.length()){
					for (int i = 0; i < strbound.length(); i++){
						if (fr.at(fr.length() - i - 1) != strbound.at(strbound.length() - i - 1))
							break;
						if (i == strbound.length()-1)
							found = true;
					}
				}
			}
			std::ofstream ofs(filename, std::ofstream::binary);
			ofs.write(fr.c_str(), fr.length() - strbound.length()-4);
			ofs.close();
		}
	}
	else if (std::stoi(_request_message._request_header._Content_Length) > 0){
		this->_request_message._post = this->recvByte(socket, std::stoi(_request_message._request_header._Content_Length));
		if (this->_error != _NO_ERROR){
			this->_error = _SRECV_ERROR;
			return this->_error;
		}
	}
	else //no post file and data
		;
	this->_error=_NO_ERROR;
	return this->_error;
}
HTTP_METHOD http_server::stringToMethod(std::string str){
	if (str == std::string("GET")) return  _GET;
	else if (str == std::string("HEAD")) return  _HEAD;
	else if (str == std::string("POST")) return  _POST;
	else if (str == std::string("PUT")) return  _PUT;
	else if (str == std::string("DELETE")) return  _DELETE;
	else if (str == std::string("CONNECT")) return  _CONNECT;
	else if (str == std::string("OPTIONS")) return  _OPTIONS;
	else if (str == std::string("TRACE")) return  _TRACE;
	else return  _ERROR_METHOD;
}
std::string http_server::methodToString(HTTP_METHOD method){
	switch (method){
	case _GET: return std::string("GET");
	case _HEAD: return std::string("HEAD");
	case _POST: return std::string("POST");
	case _PUT: return std::string("PUT");
	case _DELETE: return std::string("DELETE");
	case _CONNECT: return std::string("CONNECT");
	case _OPTIONS: return std::string("OPTIONS");
	case _TRACE: return std::string("TRACE");
	default: return std::string("ERROR_METHOD");
	}
}
std::string http_server::getHeaderFidld(std::string str,std::string hack){
	size_t pos = 0;
	if (std::string::npos != (pos = str.find(hack)))
	{
		std::string ret = str.substr(pos + hack.length() + 1, str.length() - pos - hack.length() - 1 - 2);
		if (std::string::npos != (pos = ret.find_first_not_of(" \t")))
			return ret.substr(pos, ret.length() - pos);
		else
			return ret;
	}
	return "";
}
std::string http_server::codeToReason(std::string code){
	switch (std::stoi(code))
	{
	case 100: return std::string("Continue");
	case 101: return "Switching Protocols";
	case 200: return "OK";
	case 201: return "Created";
	case 202: return "Accepted";
	case 203: return "Non-Authoritative Information";
	case 204: return "No Content";
	case 205: return "Reset Content";
	case 300: return "Multiple Choices";
	case 301: return "Moved Permanently";
	case 302: return "Found";
	case 303: return "See Other";
	case 305: return "Use Proxy";
	case 306: return "(Unused)";
	case 307: return "Temporary Redirect";
	case 400: return "Bad Request";
	case 402: return "Payment Required";
	case 403: return "Forbidden";
	case 404: return "Not Found";
	case 405: return "Method Not Allowed";
	case 406: return "Not Acceptable";
	case 408: return "Request Timeout";
	case 409: return "Conflict";
	case 410: return "Gone";
	case 411: return "Length Required";
	case 413: return "Payload Too Large";
	case 414: return "URI Too Long";
	case 415: return "Unsupported Media Type";
	case 417: return "Expectation Failed";
	case 426: return "Upgrade Required";
	case 500: return "Internal Server Error";
	case 501: return "Not Implemented";
	case 502: return "Bad Gateway";
	case 503: return "Service Unavailable";
	case 504: return "Gateway Timeout";
	case 505: return "HTTP Version Not Supported";
	default: return codeToReason("404");
	}
}
std::string http_server::combineMessage(){
	//combine status line
	std::string ret = "";
	ret = _response_message._status_line._version + " " + _response_message._status_line._status_Code;
	ret+=" " + codeToReason(_response_message._status_line._status_Code) + "\r\n";

	//combine request header field
	_response_message._resopnse_header._Content_Length = std::to_string(_response_message._message_body.length());
	_response_message._resopnse_header._Date = getNowDate();
	_response_message._resopnse_header._Server="http-server";
	_response_message._resopnse_header._X_Powered_By="";
	_response_message._resopnse_header._Connection="close";
	_response_message._resopnse_header._Transfer_Encoding="";
	_response_message._resopnse_header._Content_Type="";
	_response_message._resopnse_header._Accept_Ranges="";
	_response_message._resopnse_header._Cookie = "";

	ret += "Date: " + _response_message._resopnse_header._Date + "\r\n";
	ret += "Server: " + _response_message._resopnse_header._Server + "\r\n";
	ret += "X-Powered-By: " + _response_message._resopnse_header._X_Powered_By + "\r\n";
	ret += "Connection: " + _response_message._resopnse_header._Connection + "\r\n";
	ret += "Transfer-Encoding: " + _response_message._resopnse_header._Transfer_Encoding + "\r\n";
	ret += "Content-Type: " + _response_message._resopnse_header._Content_Type + "\r\n";
	ret += "Content-Length: " + _response_message._resopnse_header._Content_Length + "\r\n";
	ret += "Accept-Ranges: " + _response_message._resopnse_header._Accept_Ranges + "\r\n";
	if (_response_message._resopnse_header._Cookie != std::string(""))
		ret += "Cookie: " + _response_message._resopnse_header._Cookie + "\r\n";
	ret += "\r\n"; //end of header field 

	//combine request message body
	ret += _response_message._message_body;

	return ret;
}
// Sun, 06 Nov 1994 08:49:37 GMT    ; IMF-fixdate
std::string http_server::getNowDate(){
	time_t timer;
	time(&timer);
	struct tm * ptm;
	ptm = gmtime(&timer);
	std::string ret = "";
	switch (ptm->tm_wday){
	case 0: ret += "Sun";break;
	case 1: ret += "Mon";break;
	case 2: ret += "Tue";break;
	case 3: ret += "Wen";break;
	case 4: ret += "Thu";break;
	case 5: ret += "Fri";break;
	case 6: ret += "Sat";break;
	default: ret += "Sun";
	}
	ret += ", "; //add division 
	ret += ptm->tm_mday < 10 ? std::to_string(0) + std::to_string(ptm->tm_mday) : std::to_string(ptm->tm_mday);
	ret += " "; //add division 
	switch (ptm->tm_mon){
	case 0:		ret += "Jan";break;
	case 1:		ret += "Feb";break;
	case 2:		ret += "Mar";break;
	case 3:		ret += "Apr";break;
	case 4:		ret += "May"; break;
	case 5:		ret += "Jun"; break;
	case 6:		ret += "Jul"; break;
	case 7:		ret += "Aug"; break;
	case 8:		ret += "Sep";break;
	case 9:		ret += "Oct"; break;
	case 10:		ret += "Nov";break;
	case 11:		ret += "Dec";break;
	default:		ret += "Jan";
	}
	ret += " "; //add division 
	ret += std::to_string(ptm->tm_year);
	ret += " "; //add division 
	ret += ptm->tm_hour < 10 ? std::to_string(0) + std::to_string(ptm->tm_hour) : std::to_string(ptm->tm_hour);
	ret += ":"; //add division 
	ret += ptm->tm_min < 10 ? std::to_string(0) + std::to_string(ptm->tm_min) : std::to_string(ptm->tm_min);
	ret += ":"; //add division 
	ret += ptm->tm_sec < 10 ? std::to_string(0) + std::to_string(ptm->tm_sec) : std::to_string(ptm->tm_sec);
	ret += " GMT";
	return ret;
}
std::string http_server::query(std::string name){
	if (this->issetQuery(name, 1))
		return this->get(name);
	else
		return this->post(name);
}
std::string http_server::get(std::string name){
	if (this->issetQuery(name, 1)){
		int pos = this->_request_message._request_line._request_target.find(name + "=");
		int andpos = this->_request_message._request_line._request_target.find("&", pos);
		if (std::string::npos != andpos)
			return this->_request_message._request_line._request_target.substr(pos + name.length() + 1, andpos - pos - name.length() - 1);
		else
			return this->_request_message._request_line._request_target.substr(pos + name.length() + 1, _request_message._request_line._request_target.length() - pos - name.length() - 1);
	}
}
std::string http_server::post(std::string name){
	if (this->issetQuery(name, 2)){
		int pos = this->_request_message._post.find(name + "=");
		int andpos = this->_request_message._post.find("&", pos);
		if (std::string::npos != andpos)
			return this->_request_message._post.substr(pos + name.length() + 1, andpos - pos - name.length() - 1);
		else
			return this->_request_message._post.substr(pos + name.length() + 1, _request_message._post.length() - pos - name.length()- 1);
	}
	else
		return std::string("");
}
//function :
//parameter: type: 1 for GET, 2 for POST, else for BOTH.
bool http_server::issetQuery(std::string name, int type){
	switch (type){
	case 1:
		return (std::string::npos != _request_message._request_line._request_target.find(name + "="));
	case 2:
		return (std::string::npos != _request_message._post.find(name + "="));
	default:
		return (std::string::npos != _request_message._request_line._request_target.find(name + "=")) || (std::string::npos != _request_message._post.find(name + "="));
	}
}
#endif