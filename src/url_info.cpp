#include <netdb.h>
#include <arpa/inet.h>

#include <string>
#include <regex>

#include "url_info.h"

URLInfo::URLInfo(std::string url_param) : url(url_param)
{
	smatch m;
	regex link_pattern("((http://|ftp://|https://)|())(.*?)(/|:(.+?)/)");

	if(regex_search(url, m, link_pattern)){
		dl_info.host_name = m[4];
		if(m[6].length() >0)
			dl_info.port = stoi(m[6]);
		dl_info.file_path_on_server = '/'+m.suffix().str();
	}

	regex file_name_pattern("(.*?/.+/)(.*)");

	if (regex_search(url, m, file_name_pattern))
		dl_info.file_name_on_server = m[2];

	regex http_pattern("http:|https:");
	regex ftp_pattern("ftp:");
	if (regex_search(url, m, http_pattern)){
		dl_info.protocol = kHttp;
		if(m[0].str()=="http:")
			dl_info.encrypted = false;
		else if(m[0].str()=="https:")
			dl_info.encrypted = true;
		if (dl_info.port ==0)
			dl_info.port = dl_info.encrypted ? 443 : 80;
	}
	else if (regex_search(url, m, ftp_pattern)){
		dl_info.protocol = kFtp;
		if (dl_info.port == 0)
			dl_info.port = 21;
	}

	struct hostent *server;
	server = gethostbyname(dl_info.host_name.c_str());

	if (!server){
		fprintf(stderr,"ERROR, no such host\n");
		exit(0);
	}

	dl_info.ip = string(inet_ntoa(*((struct in_addr*) server->h_addr)));

	regex encoding_character("%20");
	dl_info.file_name_on_server = regex_replace(dl_info.file_name_on_server,
			encoding_character, " ");
}

addr_struct URLInfo::get_download_info()
{
	return dl_info;
}
