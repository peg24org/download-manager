#include "definitions.h"

using namespace std;

class URLInfo
{
	public:
	URLInfo(std::string url_param);
	addr_struct get_download_info();

	private:
	std::string url;
	addr_struct dl_info = {0};
};
