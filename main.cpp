// main.cpp : Defines the entry point for the application.
//

#include <functional>
#include <csignal>
#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <list>
#include <iostream>
#include <string_view>
#include <algorithm>
#include <iomanip>
#include <fstream>
#include <exception>
#include <cstdlib>
#include <http/authenticator.h>
#include <utils/dmb/auth.h>
#include <utils/filesystem.h>
#include <utils/io_utils.h>
#include <utils/file_utils.h>
#include <utils/datetime.h>
#include <utils/string_utils.h>
#include <utils/networking/sync_resources.h>
#include <utils/extern/user_input.h>
#include <utils/log.h>
#include "item.h"
#include <DMBCore.h>

LOG_TITLE("main");

#define COUT(msg) std::cout << msg
#define MSG(msg) COUT(msg << "\n")

namespace
{
	const fs::path items_fpath = fs::temp_directory_path().append("nc_item_info.txt").string();
	const fs::path input_fpath = fs::temp_directory_path().append("nc_input.txt").string();
	const fs::path identity_path = fs::temp_directory_path().append("nc_identity.json").string();
	const fs::path cfg_path = fs::temp_directory_path().append("nc_config.json").string();

	std::unique_ptr<dmb::Model> cfg_model_ptr;

	std::ofstream items_fo;
	std::ifstream items_fi;

	utils::networking::resources_list g_resources_list;
	
	const std::string default_host = "srv.vllibrary.net";
	const int default_port = 80;
	anp::tcp::endpoint_t g_ep = { default_host, default_port };
	
	const std::string empty_string;
}

// Used in utils/networking/sync_resources.h
void ask_user(
    const std::string& question
    , const utils::void_bool_cb& on_answer
    , const char* yes_btn_text_ptr
    , const char* no_btn_text_ptr
)
{
	//auto thread = std::thread([&] {
		on_answer(utils::input::ask_user(question));
	// });
	// thread.join();
}

int upload_changes(const utils::void_int_cb& cb, const utils::networking::resource_t& resource, int resource_index, bool async, bool force)
{
	LOG("upload_changes()");
	return utils::networking::upload_changes(
		g_ep
		, "/nc/h.php"
		, resource
		, resource_index
		, [=](int code) {
			utils::networking::on_upload_changes(code, resource, resource_index, async, cb);
		}
		, async
		, force
	);
}

void show_message(
	const std::string& message
	, const utils::void_cb& on_close
	, const char* ok_btn_text
)
{
	MSG(message);
}

using items_list_t = std::list<item>;

// Function declarations
vl::Object* get_cfg_data();
std::string get_host();
int get_port();
bool upload_file(const fs::path& local_path);
void load_items(items_list_t& to);
void store_item(const item& item);
bool enter_item(item& to);
int job();
int sync_resources();

std::string get_host()
{
	if (auto data_ptr = get_cfg_data())
		return (*data_ptr)["host"].as<vl::String>().Val();
	return empty_string;
}

int get_port()
{
	if (auto data_ptr = get_cfg_data())
		return (*data_ptr)["port"].as<vl::Number>().Val();
	return 0;
}

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
	if (to.title.empty())
	{
		std::string title;
		if (!item_info::enter_title(title, std::cin))
			return false;
		to.title = title;
	}

	if (auto info = item_info::load(to.title))
	{
		to.set_info(info);
		std::cout << "Item info '" << to.title << "' found: ";
		to.info().print_nutrition(100.f);
	}
	std::cin >> to;
	if (!utils::input::last_getline_valid())
		return false;
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
		utils::file::move(input_fpath.string(), new_fname_input.string());
		utils::file::copy(items_fpath.string(), new_fname_info.string());
		upload_file(new_fname_info);
		upload_file(new_fname_input);
		// Exit from the input loop
		return false;
	};

	utils::input::register_command("exit");
	utils::input::register_command("end", finish_input);
	utils::input::register_command("new", finish_input);
	utils::input::register_command("total");
	utils::input::register_command("cancel");
	utils::input::register_command("edit", [&] {
		auto v = utils::input::last_getline_value();
		auto p = v.find(" ");
		if (p == std::string::npos)
			return true;
		auto what = v.substr(p + 1);
		if (!what.empty())
		{
			auto it = std::find_if(items.begin(), items.end(), [&](auto& item) {
				return item.title == what;
			});
			if (it != items.end())
			{
				if (enter_item(*it))
					MSG("Edit completed");
				else
					MSG("Edit interrupted");
			}
			else
				MSG("Item '" << what << "' not found");
		}
		return true;
	});
	utils::input::register_command("remove", [&] {
		auto v = utils::input::last_getline_value();
		auto p = v.find(" ");
		if (p == std::string::npos)
			return true;
		auto what = v.substr(p + 1);
		if (!what.empty())
		{
			auto it = std::find_if(items.begin(), items.end(), [&](auto& item) {
				return item.title == what;
				});
			if (it != items.end())
			{
				items.erase(it);
				MSG("Item '" << what << "' removed");
			}
			else
				MSG("Item '" << what << "' not found");
		}
		return true;
	});
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

	while (utils::input::last_command() != "exit")
	{
		while (
			utils::input::last_command() != "end"
			&& utils::input::last_command() != "exit"
			&& utils::input::last_command() != "new"
			&& utils::input::last_command() != "total"
		)
		{
			item item;

			if (!enter_item(item))
				continue;

			if (!utils::input::last_getline_valid())
				continue;
			item.print_nutrition();
			items.push_back(item);
			//store_item(item);
		}

		// Sort the items
		items.sort([](auto&& l, auto&& r) {
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

		if (utils::input::last_command() != "total")
			items.clear();

		if (utils::input::last_command() != "exit")
			utils::input::reset_last_input();
	}
	return 0;
}

bool upload_file(const fs::path& fpath)
{
	LOG_DEBUG("upload_file(" << fpath.string() << ")");
	return utils::http::upload_file(fpath.string(), g_ep, "/nc/h.php");
}

int sync_resources()
{
	return utils::networking::sync_resources(g_ep, "/nc/s.php", "/nc/h.php", g_resources_list
	, nullptr, false);
}

struct terminator
{
	~terminator() {
		LOG_DEBUG("~terminator()");
		sync_resources();
	}
};

vl::Object* get_cfg_data()
{
	if (!cfg_model_ptr)
		cfg_model_ptr = std::make_unique<dmb::Model>();

	if (!cfg_model_ptr->IsLoaded())
		if (!cfg_model_ptr->Load(cfg_path.string()))
			if (!cfg_model_ptr->Store(cfg_path.string(), { true }))
			{
				LOG_ERROR("Can't create config file");
				return nullptr;
			}
	return &cfg_model_ptr->GetContent().GetData();
}

bool check_config()
{
	auto content_data_ptr = get_cfg_data();
	if (!content_data_ptr)
	{
		LOG_ERROR("Can't load config file");
		return false;
	}
	auto& content_data = *content_data_ptr;
	bool need_to_store = false;

	if (!content_data.Has("host") || !content_data["host"].is<vl::String>() || content_data["host"].as<vl::String>().Val().empty())
	{
		content_data.Set("host", default_host);
		need_to_store = true;
	}
	if (!content_data.Has("port") || !content_data["port"].is<vl::Number>())
	{
		content_data.Set("port", default_port);
		need_to_store = true;
	}
	g_ep = { content_data["host"].as<vl::String>().Val(), content_data["port"].as<vl::Number>().Val<int>() };
	if (need_to_store)
		cfg_model_ptr->Store(cfg_path.string(), { true });
	return true;
}

int request_auth(const std::string& user_name, const std::string& token)
{
	using namespace anp;
	authenticator_ptr a = std::make_shared<authenticator>();
	auto host = get_host();
	auto port = get_port();
	return a->auth({ host, port }, "/nc/a.php", { user_name, token });
}

int main()
{
	using namespace anp;

	std::signal(SIGINT, [] (int sig) {
		LOG_DEBUG("SIGINT raised");
		sync_resources();
	});

	// Auto uploader on program finish
	std::unique_ptr<terminator> terminator_inst = std::make_unique<terminator>();

	std::cout << "Nutrition Calculator\n";

	if (!check_config())
	{
		MSG("Exit");
		return 0;
	}

	if (!get_identity())
	{
		MSG("No login information has been provided. Exit.");
		return 0;
	}

	if (auth() == 0)
	{
		utils::file::remove(identity_path);
		identity_model_ptr.reset(nullptr);
		if (!utils::input::ask_user("Authentication error. Continue in offline mode?"))
		{
			MSG("Exit");
			return 0;
		}
	}
	else
		MSG("\nHello, " << identity_model_ptr->GetContent().GetData()["user"]["name"].as<vl::String>().Val() << "!\n");

	g_resources_list = {
		{ "nc_input.txt", input_fpath },
		{ "nc_item_info.txt", items_fpath }
	};
	
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
