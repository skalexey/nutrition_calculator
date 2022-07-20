// main.cpp : Defines the entry point for the application.
//

#include <functional>
#include <csignal>
#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <string_view>
#include <algorithm>
#include <iomanip>
#include <fstream>
#include <exception>
#include <cstdlib>
#include <filesystem>
#include <utils/string_utils.h>
#include <utils/io_utils.h>
#include <utils/file_utils.h>
#include <utils/datetime.h>
#include <utils/string_utils.h>
#include <utils/Log.h>
#include "downloader.h"
#include "uploader.h"
#include "item.h"
#include <DMBCore.h>

LOG_POSTFIX("\n");
LOG_PREFIX("[main]: ");

#define COUT(msg) std::cout << msg
#define MSG(msg) COUT(msg << "\n")

namespace fs = std::filesystem;

namespace
{
	const fs::path items_fpath = fs::temp_directory_path().append("item_info.txt").string();
	const fs::path input_fpath = fs::temp_directory_path().append("input.txt").string();
	const fs::path identity_path = fs::temp_directory_path().append("identity.json").string();

	std::unique_ptr<dmb::Model> identity_model_ptr;

	std::ofstream items_fo;
	std::ifstream items_fi;

	const std::string empty_string;

	const std::string host = "skalexey.ru";
	const int port = 80;
}

using items_list_t = std::vector<item>;

// Function declarations
bool get_identity(std::string* name = nullptr, std::string* pass = nullptr);
std::string h(const std::string& s);
bool ask_pass(std::string& s);
bool ask_name(std::string& s);
bool auth();
bool upload_file(const fs::path& local_path);
const std::string& get_user_token();
const std::string& get_user_name();
vl::Object* get_identity_cfg_data();
void load_items(items_list_t& to);
void store_item(const item& item);
bool enter_item(item& to);
int job();
int sync_resources();

// Definitions are all below
void load_items(items_list_t& to)
{
	while (!items_fi.eof())
		items_fi >> to.emplace_back();
}

void store_item(const item& item)
{
	items_fo << item;
}

bool enter_item(item& to)
{
	// Title
	std::string title;
	if (!item_info::enter_title(title, std::cin))
		return false;

	to.title = title;
	
	if (auto info = item_info::load(title))
	{
		to.set_info(info);
		std::cout << "Item info '" << title << "' found: ";
		to.info().print_nutrition(100.f);
	}
	std::cin >> to;

	return to;
}

int job()
{
	//items_fo.open(items_fname, std::ios::app | std::ios::binary);
	//items_fi.open(items_fname, std::ios::binary);

	items_list_t items;
	//load_items(items);

	auto finish_input = [&] {
		utils::input::close_input();
		auto cur_dt = utils::current_datetime("%02i-%02i-%02i-%03li");
		auto new_fname_input = fs::path(input_fpath.parent_path() / fs::path(utils::format_str("input-%s.txt", cur_dt.c_str())));
		auto new_fname_info = fs::path(items_fpath.parent_path() / utils::format_str("item_info-%s.txt", cur_dt.c_str()));
		utils::file::move_file(input_fpath.string(), new_fname_input.string());
		utils::file::copy_file(items_fpath.string(), new_fname_info.string());
		// Exit from the input loop
		return false;
	};

	utils::input::register_command("exit");
	utils::input::register_command("end", finish_input);
	utils::input::register_command("new", finish_input);
	utils::input::register_command("total");
	utils::input::register_command("remove_last", [&] {
		items.resize(items.size() - 1);
		utils::file::remove_last_line_f(*utils::input::get_file());
		return true;
	});
	utils::input::register_command("temp_dir", [] {
		MSG(fs::temp_directory_path().string());
		return true;
	});
	utils::input::register_command("sync", [] {
		sync_resources();
		return true;
	});

	while (utils::input::last_command != "exit")
	{
		while (
			utils::input::last_command != "end"
			&& utils::input::last_command != "exit"
			&& utils::input::last_command != "new"
			&& utils::input::last_command != "total"
		)
		{
			item item;

			if (!enter_item(item))
				continue;

			if (!utils::input::last_getline_valid)
				continue;
			item.print_nutrition();
			items.push_back(item);
			//store_item(item);
		}

		// Sort the items
		std::sort(items.begin(), items.end(), [](auto&& l, auto&& r) {
			float ln = 0.f, rn = 0.f;
			for (auto&& n : l.info().nutrition)
				ln += n;
			ln *= l.weight;
			for (auto&& n : r.info().nutrition)
				rn += n;
			rn *= r.weight;
			return ln > rn;
		});

		std::cout << std::setw(34) << "title |";
		std::cout << std::setw(10) << "w (g) |";
		std::cout << std::setw(10) << "p (g) |";
		std::cout << std::setw(10) << "f (g) |";
		std::cout << std::setw(10) << "c (g) |";
		std::cout << std::setw(10) << "fib (g) |";
		std::cout << std::setw(10) << "cal |";
		std::cout << "\n";

		item_info total_info;
		total_info.nutrition.resize(4);
		float total_weight = 0.f;

		for (auto item : items)
		{
			std::cout << std::setw(32) << item.info().title << " |";
			std::cout << std::setw(8) << item.weight << " |";
			int i = 0;
			for (auto n : item.info().nutrition)
			{
				float val = n * item.weight / 100.f;
				std::cout << std::setw(8) << val << " |";
				total_info.nutrition[i++] += val;
			}
			float val = item.info().cal * item.weight / 100.f;
			std::cout << std::setw(8) << val << " |";
			total_info.cal += val;
			total_weight += item.weight;
			std::cout << "\n";
		}

		std::cout << std::setw(34) << "Total: |";
		std::cout << std::setw(8) << total_weight << " |";
		for (auto n : total_info.nutrition)
			std::cout << std::setw(8) << n << " |";
		std::cout << std::setw(8) << total_info.cal << " |";
		std::cout << "\n";

		if (utils::input::last_command != "total")
			items.clear();

		if (utils::input::last_command != "exit")
			utils::input::reset_last_input();
	}
	return 0;
}

bool upload_file(const fs::path& local_path)
{
	using namespace anp;
	uploader u;
	if (u.upload_file(host, port, local_path
		, utils::format_str(
			"/nc/h.php?u=%s&t=%s"
			, get_user_name().c_str()
			, get_user_token().c_str()
		)) == http_client::erc::no_error)
	{
		MSG("Uploaded '" << local_path << "'");
		return true;
	}
	else
		LOG_ERROR("Error while uploading '" << local_path << "'");
	return false;
}

int sync_resources()
{
	using namespace anp;

	downloader d;
	auto download = [&](const std::string& remote_path, const fs::path& local_path) -> bool {
		MSG("Download remote version of resource '" << local_path.filename() << "'...");
		if (d.download_file(host, port
			, utils::format_str(
				"/nc/s.php?p=%s&u=%s&t=%s"
				, remote_path.c_str()
				, get_user_name().c_str()
				, get_user_token().c_str(),
				get_user_token().c_str()
			), local_path) != http_client::erc::no_error)
		{
			if (d.errcode() == downloader::erc::uncommitted_changes)
			{
				std::stringstream ss;
				ss << "You have changes in '" << local_path << "'.\nWould you like to upload your file to the remote?";
				try
				{
					if (utils::input::ask_user(
						ss.str()))
					{
						if (!upload_file(local_path))
							return false;
					}
					else
					{
						if (utils::input::ask_user("Replace with the downloaded version?"))
							d.replace_with_download();
					}
				}
				catch (std::string s)
				{
					MSG("Emergency exit (" << s << ")");
					return false;
				}
				return true;
			}
			else if (d.errcode() == downloader::erc::parse_date_error)
				if (utils::input::ask_user("Replace with the downloaded version?"))
					d.replace_with_download();
			LOG_ERROR("Error while downloading resource '" << remote_path << "'" << " to '" << local_path << "': " << d.errcode());
			return false;
		}
		if (d.is_file_updated())
			MSG("Resource updated from the remote: '" << local_path.string() << "'");
		else
			MSG("Local resource is up to date: '" << local_path.string() << "'");
		return true;
	};

	if (!download("item_info.txt", items_fpath))
		return 1;

	if (!download("input.txt", input_fpath))
		return 2;
	
	return 0;
}

struct terminator
{
	~terminator() {
		LOG_DEBUG("~terminator()");
		sync_resources();
	}
};

const std::string& get_user_name()
{
	if (auto data_ptr = get_identity_cfg_data())
		return (*data_ptr)["user"].AsObject().Get("name").AsString().Val();
	return empty_string;
}

const std::string& get_user_token()
{
	if (auto data_ptr = get_identity_cfg_data())
		return (*data_ptr)["user"].AsObject().Get("token").AsString().Val();
	return empty_string;
}

vl::Object* get_identity_cfg_data()
{
	if (!identity_model_ptr)
	{
		LOG_DEBUG("Identity model is not initialized");
		return nullptr;
	}
	if (!identity_model_ptr->IsLoaded())
	{
		LOG_DEBUG("Identity model is not loaded");
		return nullptr;
	}
	return &identity_model_ptr->GetContent().GetData();
}

bool request_auth(const std::string& name, const std::string& token)
{
	using namespace anp;
	http_client c;
	bool success = false;
	std::string response;
	c.query(host, port, "GET"
		, utils::format_str("/nc/a.php?u=%s&t=%s", name.c_str(), token.c_str()).c_str()
		, [=, &success, &response, &c](
			const std::vector<char>& data
			, std::size_t sz
			, int code
		)
		{
			LOG_VERBOSE("\nReceived " << sz << " bytes:");
			std::string s(data.begin(), data.begin() + sz);
			LOG_VERBOSE(s);
			response.insert(response.end(), data.begin(), data.end());
			if (s.find("Authenticated successfully") != std::string::npos)
				success = true;
			c.notify(http_client::erc::no_error);
			return true;
		}
	);
	return success;
}

bool auth()
{
	vl::Object& data = identity_model_ptr->GetContent().GetData();
	std::string user_name, token;
	if (!get_identity(&user_name, &token))
		return false;
	return request_auth(user_name, token);
}

bool ask_name(std::string& s)
{
	return utils::input::ask_line(s, "Enter your login name: ", " > ");
}

bool ask_pass(std::string& s)
{
	return utils::input::ask_line(s, "Enter password: ", " > ");
}

std::string h(const std::string& s)
{
	return std::to_string(std::hash<std::string>{}(s));
}

bool get_identity(std::string* user_name, std::string* user_pass)
{
	identity_model_ptr->Load(identity_path.string());
	auto data_ptr = get_identity_cfg_data();
	if (!data_ptr)
		return false;

	auto& data = *data_ptr;

	auto name = get_user_name();
	auto token = get_user_token();
	bool store = false;
	if (name.empty())
	{
		if (!ask_name(name))
			return false;
		data["user"].AsObject().Set("name", name);
		store = true;
	}
	if (user_name)
		user_name->swap(name);

	if (token.empty())
	{
		std::string pass;
		if (!ask_pass(pass))
			return false;
		token = h(pass);
		data["user"].AsObject().Set("token", token);
		store = true;
	}
	if (user_pass)
		user_pass->swap(token);
	
	if (store)
		identity_model_ptr->Store(identity_path.string(), { true });

	return true;
}

int main()
{
	identity_model_ptr = std::make_unique<dmb::Model>();
	using namespace anp;

	std::signal(SIGINT, [] (int sig) {
		LOG_DEBUG("SIGINT raised");
		sync_resources();
	});

	// Auto uploader on program finish
	std::unique_ptr<terminator> terminator_inst = std::make_unique<terminator>();

	std::cout << "Nutrition Calculator\n";

	if (!identity_model_ptr->Load(identity_path.string()))
	{
		if (!get_identity())
		{
			MSG("No login information has been provided. Exit.");
			return 0;
		}
	}

	if (!auth())
	{
		if (!utils::input::ask_user("Authentication error. Continue in offline mode?"))
		{
			MSG("Exit");
			return 0;
		}
	}

	MSG("Hello " << identity_model_ptr->GetContent().GetData()["user"]["name"].AsString().Val() << "!");

	auto ret = sync_resources();
	if (ret != 0)
		if (!utils::input::ask_user("Errors while syncing resources. Continue in offline mode?"))
			return ret;

	job();

	// Let it leave longer
	// TODO: think how to shorten its lifetime
	//identity_model_ptr.reset(nullptr);

	return 0;
}
	

// TODO:
//	* store and load items instead of input
//	* date in file names
//	* end <fname>
//	* from <fname> -load from a file
//	* edit_info <item_title>
//	*'category' command