#include "definitions.h"

using namespace std;

class URLInfo
{
	public:
	URLInfo(std::string url_param);
	struct addr_struct get_download_info();

	private:
	std::string url;
	struct addr_struct dl_info = {0};
};
