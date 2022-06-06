// item.cpp : Defines the entry point for the application.
//
#include <iostream>
#include <ranges>
#include <string_view>
#include <algorithm>
#include <iomanip>
#include <fstream>
#include <exception>
#include <cstdlib>
#include <utils/io_utils.h>
#include <utils/string_utils.h>
#include "item.h"

namespace
{
	// Data
	const std::string item_info_fname = "item_info.txt";
	const std::string input_fname = "input.txt";
	std::ifstream fi_item_info;
	std::ofstream fo_item_info;

	// Functions
	bool parse_nutrition(const std::string& data, std::vector<float>& nutrition)
	{
		std::string_view parts(data);
		constexpr std::string_view delim("/");
		auto v = std::views::split(parts, delim);
		try
		{
			for (auto&& sub : v)
				nutrition.push_back(std::stof(std::string(sub)));
		}
		catch (const std::invalid_argument& e)
		{
			return false;
		}
		return true;
	}

	bool parse_calories(const std::string& data, float& to)
	{
		try
		{
			to = std::stof(data);
		}
		catch (const std::invalid_argument& e)
		{
			return false;
		}
		return true;
	}
}

// Begin of item
// Operator >>
std::istream& operator >> (std::istream& is, item& obj)
{
	// Title
	if (obj.title.empty())
		if (!item_info::enter_title(obj.title, is))
			return is;

	if (auto info = item_info::load(obj.title))
	{
		//std::cout << "Item info '" << title << "' found: ";
		//info->print_nutrition(100.f);
		obj.set_info(info);
	}

	// Weight
	std::cout << "\t" << "grams: ";
	while (!utils::input::input_t(obj.weight, is, input_fname));
	
	return is;
}

void item::print_nutrition()
{
	info().print_nutrition(weight);
}
// End of item

// Begin of item_info
void item_info::print_nutrition(float weight)
{
	int i = 0;
	for (auto&& n : nutrition)
		std::cout << nutrition_title(i++) << ": "
		 << n * weight / 100.f << "\t";
	std::cout << "Cal: " << cal * weight / 100.f;
	std::cout << "\n";
}

bool item_info::enter_title(std::string& to, std::istream& is)
{
	int trial = 0;
	std::cout << "New item title (or type 'exit' to finish): ";
	do {
		//if (trial > 0)
		//	std::cout << "\tInvalid name '" << to << "'. Enter again (or type 'exit' to finish): ";
		utils::input::input_line(to, is, input_fname);
		if (!utils::input::last_getline_valid)
			return false;
		//auto it = std::remove_if(to.begin(), to.end(), isspace);
		//if (it != to.end())
		//	to.erase(it, to.end());
		trial++;
	} while (to.empty());
	return true;
}

item_info_ptr item_info::load(const std::string& item_title)
{
	item_info_ptr ret(nullptr);
	if (!fi_item_info.is_open())
	{
		fo_item_info.open(item_info_fname, std::ios::app);
		fo_item_info.seekp(0, std::ios::end);
		fi_item_info.open(item_info_fname);
	}
	fi_item_info.clear();
	fi_item_info.seekg(0);
	while (true)
	{
		std::string line;
		getline(fi_item_info, line);
		if (line.empty())
			break;
		std::string_view parts(line);
		constexpr std::string_view delim("\t");
		auto v = std::views::split(parts, delim);
		// TODO: use streams
		int i = 0;
		for (auto&& p : v)
		{
			switch (i)
			{
				case 0: // Title
					if (std::string(p) != item_title)
						break;
					else
					{
						ret = std::make_shared<item_info>();
						ret->title = p;
					}
					break;

				case 1:	// Nutrition
					if (ret)
						parse_nutrition(std::string(p), ret->nutrition);
					break;

				case 2: // Callories
					if (ret)
						parse_calories(std::string(p), ret->cal);
					break;
			}
			i++;
		}
		if (ret)
			break;
	}
	return ret;
}

// Operator >>
std::istream& operator >> (std::istream& is, item_info& obj)
{
	// Title
	if (obj.title.empty())
		if (!item_info::enter_title(obj.title, is))
			return is;
	fo_item_info << obj.title << "\t";
	fo_item_info.flush();
	
	// Nutrition
	obj.enter_nutrition(is);
	fo_item_info << "\t";
	fo_item_info.flush();

	// Cal
	obj.enter_cal(is);
	fo_item_info << "\n";
	fo_item_info.flush();

	return is;
}

bool item_info::enter_nutrition(std::istream& is)
{
	int trial = 0;
	std::string pfcfib;
	while (nutrition.size() != 4)
	{
		nutrition.clear();
		pfcfib.clear();
		if (trial == 0)
			std::cout << "\t" << "p/f/c/fib per 100g: ";
		else
			std::cout << "\t" << "Wrong format\np/f/c/fib: ";
		do {
			if (!utils::input::input_line(pfcfib, is, input_fname))
				return false;
			auto it = std::remove_if(pfcfib.begin(), pfcfib.end(), isspace);
			if (it != pfcfib.end())
				pfcfib.erase(it, pfcfib.end());
		} while (pfcfib.empty());
		
		parse_nutrition(pfcfib, nutrition);
		trial++;
	}
	fo_item_info << pfcfib;
	fo_item_info.flush();
	return true;
}

bool item_info::enter_cal(std::istream& is)
{
	std::string s;
	std::cout << "\t" << "Callories: ";
	
	while (!utils::input::input_t(cal, is, input_fname));
	
	fo_item_info << cal;
	fo_item_info.flush();

	return true;
}
// End of item_info