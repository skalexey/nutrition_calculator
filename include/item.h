// item.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <unordered_set>
#include <memory>
#include <string>
#include <vector>
#include <istream>
#include <utility>

const std::string nutrition_protein = "Protein";
const std::string nutrition_fat = "Fat";
const std::string nutrition_carbs = "Carbs";
const std::string nutrition_fiber = "Fiber";

struct item_info;
using item_info_ptr = std::shared_ptr<item_info>;

struct item_info
{
	using aliases_list_t = std::unordered_set<std::string>;
    // Data
	std::string title;
	std::vector<float> nutrition;
	aliases_list_t aliases;
    float cal = 0;

    // Initializers
    item_info() = default;
    item_info(const std::string& title) : title(title) {}
    // Public interface
    void print_nutrition(float weight);
    
    const std::string& nutrition_title(int nutrition_index) const {
        switch (nutrition_index)
        {
            case 0: return nutrition_protein;
            case 1: return nutrition_fat;
            case 2: return nutrition_carbs;
            case 3: return nutrition_fiber;
        }
        static std::string empty_string;
        return empty_string;
    }

    bool enter_nutrition(std::istream& is);
    bool enter_cal(std::istream& is);

    // Static public interface
    static bool enter_title(std::string& to, std::istream& is);
    static item_info_ptr load(const std::string& item_title);

	// Operator bool
	operator bool() const {
		return !title.empty() && nutrition.size() == 4;
	}

    operator bool() {
        return std::as_const(*this).operator bool();
    }
};

std::istream& operator >> (std::istream& is, item_info& obj);

struct item
{
    // Data
	float weight = 0;
    
    std::string title;

    // Public interface
	item_info& info() {
		static item_info empty_info;
		return m_info_ptr ? *m_info_ptr : empty_info;
	}

    const item_info& get_info() const {
        static item_info empty_info;
        return m_info_ptr ? *m_info_ptr : empty_info;
    }

    void set_info(const item_info_ptr& ptr) {
        m_info_ptr = ptr;
    }

    void print_nutrition();

	// Operator bool
	operator bool() const {
		return m_info_ptr != nullptr && get_info();
	}

    operator bool() {
        return std::as_const(*this).operator bool();
    }

private:
	item_info_ptr m_info_ptr;
};

std::istream& operator >> (std::istream& is, item& obj);

