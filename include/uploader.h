// uploader.h

#pragma once

#include <memory>
#include <string>
#include <filesystem>
#include "http_client.h"

namespace anp
{
	class uploader : public http_client
	{
		using base = http_client;

	public:
		enum erc : int
		{
			file_not_exists = http_client::erc::count,
			transfer_error,
			auth_error
		};

		int upload_file(
			const std::string& host,
			int port,
			const std::filesystem::path& target_path,
			const std::string& query = "/"
		);

	protected:
		void on_notify(int ec) override;
		void on_reset() override;
	};
}