#ifndef _DOWNLOADER_H
#define _DOWNLOADER_H

#include "thread.h"
#include "definitions.h"

using namespace std;


class Downloader:public Thread{
	private:
		void run();
		bool is_start_pos_written = false;

	protected:
		function<void(size_t,string,int)> init_callback_func;
		bool regex_search_string(const string& input, const string& pattern,
			string& output);

		virtual void downloader_trd() 	= 0;
		int 		index 		= 0;
		size_t		trd_len		= 0;	// file size in bytes
		size_t		pos		= 0;	//fp last position
		node_struct*	node_data;
		addr_struct	addr_data;

		void write_to_file(size_t pos, size_t len, char* buf);
		void write_log_file(size_t pos);
		void write_start_pos_log(size_t start_pos);
		int	sockfd = 0;

	public:
		Downloader(node_struct* node_data, const addr_struct, size_t pos,
				size_t trd_length, int index);
		~Downloader();
		void call_node_status_changed(int recieved_bytes, int err_flag = 0);
		int get_index();
		void set_index(int value);

		size_t get_trd_len();


		/**
		 * Check the size of file and redirection
		 *
		 * @param redirect_url: Will be filled with redirected url if
		 * 		redirection exist.
		 * @param size: Will be filled with size of file if exist.
		 *
		 * @return True if redirection detected
		 */
		virtual bool check_link(string& redirect_url, size_t& size) = 0;
		virtual void connect_to_server() = 0;
		virtual void disconnect() = 0;

		// "(HTTP\\/\\d\\.\\d\\s*)(\\d+\\s)([\\w|\\s]+\\n)";
		const static string HTTP_HEADER;
};

#endif
