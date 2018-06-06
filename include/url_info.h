#include "definitions.h"

using namespace std;

class URLInfo
{
	std::string url;
	addr_struct dl_info;
	public:
	URLInfo(std::string url);
	addr_struct get_download_info();
};
