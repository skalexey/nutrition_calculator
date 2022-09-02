// downloader.h

#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <utils/filesystem.h>
#include "http_client.h"

namespace anp
{
	class downloader : public http_client
	{
		using base = http_client;

	public:
		enum erc : int
		{
			parse_size_error = http_client::erc::count,
			parse_date_error,
			receive_size_error,
			no_content_length,
			no_file,
			uncommitted_changes,
			no_date,
			file_error,
			backup_error,
			store_download_error,
			auth_error
		};

		int download_file(
			const std::string& host,
			int port,
			const std::string& query = "/",
			const std::filesystem::path& target_path = std::filesystem::path()
		);

		bool is_file_updated();
		// TODO: see how to return it to the protected
		bool replace_with_download();

	protected:
		void on_notify(int ec) override;
		void on_reset() override;

	private:
		bool backup_local_file();
		void create_download_file();
		bool restore_from_backup();
		bool remove_backup();

	private:
		long long m_content_length = -1;
		std::filesystem::path m_backup;
		std::filesystem::path m_target;
		std::filesystem::path m_download;
		std::chrono::system_clock::time_point m_tp, m_f_tp;
		bool m_is_file_updated = false;
	};
}