// http_client.h

#pragma once

#include <memory>
#include <atomic>
#include <string>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <tcp/fwd.h>
#include <common/common.h>

namespace anp
{
	class http_client
	{
	public:
		struct headers_t;

		using http_response_cb = std::function<bool(
			const headers_t& headers,
			const std::vector<char>&	// Buffer
			, std::size_t				// Size
		)>;

		enum erc : int
		{
			unknown = -1,
			no_error = 0,
			connection_error,
			connection_process_error,
			receive_size_error,
			status_parse_error,
			count
		};

		struct header {
			std::string name;
			std::string value;
			std::string to_string() const;
			operator std::string() const;
		};

		struct headers_t
		{
			using data_t = std::vector<header>;
			headers_t() = default;
			headers_t(const data_t& headers) : data(headers) {}
			data_t data;
			std::string to_string() const;
			operator std::string() const;
			void add(const header& h);
		};

		http_client();
		~http_client();

		int query(
			const std::string& host,
			int port,
			const std::string& method,
			const std::string& query,
			const http_response_cb& on_receive = http_response_cb(),
			const headers_t& headers = headers_t(),
			const std::string& body = ""
		);

		void query_async(
			const std::string& host,
			int port,
			const std::string& method,
			const std::string& query,
			const http_response_cb& on_receive = http_response_cb(),
			const headers_t& headers = headers_t(),
			const std::string& body = ""
		);

		int request(
			const std::string& host,
			int port,
			const std::string& request,
			const http_response_cb& on_receive
		);

		void request_async(
			const std::string& host,
			int port,
			const std::string& request,
			const http_response_cb& on_receive
		);

		inline int errcode() {
			return m_error_code.load();
		}

		void wait();

		int notify(int ec);

	protected:
		std::string parse_header(const std::string& response, const std::string& header);

	protected:
		virtual inline void on_before_notify(int ec) {};
		virtual inline void on_notify(int ec) {};
		void reset();
		virtual inline void on_reset() {};

	private:
		std::atomic<int> m_error_code = erc::unknown;
		std::mutex m_cv_mtx;
		std::unique_lock<std::mutex> m_cv_ul;
		std::condition_variable m_cv;
		std::unique_ptr<tcp::client> m_client;
		headers_t m_headers;
		long long m_content_length = -1;
		bool m_is_header = true;
		int m_status;
		std::string m_last_part;
	};
}
