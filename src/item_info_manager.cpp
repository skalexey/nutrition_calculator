#include "item_info_manager.h"
#include <utils/string_utils.h>
#include <utils/io_utils.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cassert>

namespace
{
    // Helper functions from original item.cpp
    bool parse_aliases(const std::string_view& data, item_info::aliases_list_t& to)
    {
        auto aliases = utils::split(data, "=");
        std::transform(
            aliases.begin(),
            aliases.end(),
            std::inserter(to, to.end()),
            [&](auto&& v) {
                return std::string(v);
            });
        return true;
    }

    bool parse_nutrition(const std::string& data, std::vector<float>& nutrition)
    {
        assert(nutrition.empty());
        constexpr std::string_view delim("/");
        auto v = utils::split(data, "/");
        try
        {
            const auto nutrition_input_size = 5;
            auto size = v.size();
            if (size > nutrition_input_size)
            {
                std::cerr << "Error: Too many nutrition values provided. Expected at most 5 values.\n";
                return false;
            }
            nutrition.reserve(v.size() - 1);
            auto weight = 100.f;
            auto it_size = v.end();
            if (size == nutrition_input_size)
            {
                it_size = std::next(v.begin(), size - 1);
                weight = std::stof(std::string(*it_size));
                if (weight <= 0)
                {
                    std::cerr << "Error: Invalid weight value provided: " << weight << ". Weight must be greater than 0.\n";
                    return false;
                }
            }
            auto factor = 100.f / weight;
            for (auto it = v.begin(); it != it_size; ++it)
            {
                float value = std::stof(std::string(*it));
                if (value < 0)
                {
                    std::cerr << "Error: Negative nutrition value provided: " << value << ". Nutrition values must be non-negative.\n";
                    return false;
                }
                nutrition.push_back(value * factor);
            }
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

item_info_manager::item_info_manager(const std::filesystem::path& file_path)
    : m_file_path(file_path)
{
}

bool item_info_manager::load_all()
{
    m_items.clear();
    m_title_to_index.clear();
    
    std::ifstream file(m_file_path);
    if (!file.is_open())
    {
        // File doesn't exist yet, that's okay
        return true;
    }
    
    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty())
            continue;
            
        item_info_ptr info;
        if (parse_line(line, info))
        {
            m_items.push_back(info);
        }
        else
        {
            std::cerr << "Warning: Failed to parse line: " << line << std::endl;
        }
    }
    
    build_title_index();
    return true;
}

bool item_info_manager::save_all()
{
    std::ofstream file(m_file_path, std::ios::trunc);
    if (!file.is_open())
    {
        std::cerr << "Error: Cannot open file for writing: " << m_file_path << std::endl;
        return false;
    }
    
    for (const auto& item : m_items)
    {
        if (item)
        {
            file << serialize_item(*item) << "\n";
        }
    }
    
    return true;
}

bool item_info_manager::save_item_at_index(size_t index)
{
    if (index >= m_items.size() || !m_items[index])
        return false;
    
    std::string serialized = serialize_item(*m_items[index]);
    update_file_line(index, serialized);
    return true;
}

item_info_ptr item_info_manager::find_item(const std::string& title)
{
    auto it = m_title_to_index.find(title);
    if (it != m_title_to_index.end())
    {
        return m_items[it->second];
    }
    
    // Check aliases
    for (const auto& item : m_items)
    {
        if (item && item->aliases.find(title) != item->aliases.end())
        {
            return item;
        }
    }
    
    return nullptr;
}

item_info_ptr item_info_manager::get_item_at_index(size_t index)
{
    if (index >= m_items.size())
        return nullptr;
    return m_items[index];
}

void item_info_manager::add_item(const item_info_ptr& info)
{
    if (!info)
        return;
        
    // Check if item already exists
    auto existing = find_item(info->title);
    if (existing)
    {
        // Update existing item
        *existing = *info;
        save_all();
        return;
    }
        
    m_items.push_back(info);
    m_title_to_index[info->title] = m_items.size() - 1;
}

bool item_info_manager::edit_item(const std::string& title)
{
    auto item = find_item(title);
    if (!item)
    {
        std::cout << "Item '" << title << "' not found.\n";
        return false;
    }
    
    std::cout << "Editing item: " << item->title << "\n";
    std::cout << "Current nutrition: ";
    item->print_nutrition(100.0f);
    std::cout << "\n";
    
    // Re-enter nutrition
    std::cout << "Enter new nutrition data:\n";
    item->nutrition.clear();
    if (!item->enter_nutrition(std::cin))
    {
        std::cout << "Edit cancelled.\n";
        return false;
    }
    
    // Re-enter calories
    std::cout << "Enter new calorie data:\n";
    if (!item->enter_cal(std::cin))
    {
        std::cout << "Edit cancelled.\n";
        return false;
    }
    
    // Check calculated vs entered calories
    float calculated_cal = item->calc_calories();
    if (std::abs(calculated_cal - item->cal) > 5.0f)
    {
        std::cout << "Calculated calories (" << calculated_cal 
                  << ") differ from entered (" << item->cal 
                  << "). Use calculated value? (y/n): ";
        char answer;
        std::cin >> answer;
        if (answer == 'y' || answer == 'Y')
            item->cal = calculated_cal;
    }
    
    save_all();
    std::cout << "Item updated successfully.\n";
    return true;
}

void item_info_manager::validate_and_fix_items()
{
    std::vector<size_t> incomplete_items;
    
    for (size_t i = 0; i < m_items.size(); ++i)
    {
        if (m_items[i] && !is_item_complete(*m_items[i]))
        {
            incomplete_items.push_back(i);
        }
    }
    
    if (!incomplete_items.empty())
    {
        std::cout << "Found " << incomplete_items.size() << " incomplete items. Fixing them...\n";
        
        for (size_t index : incomplete_items)
        {
            auto& item = *m_items[index];
            std::cout << "\nIncomplete item: '" << item.title << "'\n";
            
            // Fix nutrition if missing
            if (item.nutrition.size() != 4)
            {
                std::cout << "Missing nutrition data. Please enter:\n";
                if (!item.enter_nutrition(std::cin))
                {
                    std::cout << "Skipped fixing nutrition for " << item.title << "\n";
                    continue;
                }
            }
            
            // Fix calories if missing
            if (item.cal == 0)
            {
                std::cout << "Missing calorie data. Please enter:\n";
                if (!item.enter_cal(std::cin))
                {
                    std::cout << "Skipped fixing calories for " << item.title << "\n";
                    continue;
                }
            }
            
            // Save the fixed item
            save_item_at_index(index);
            std::cout << "Fixed item: " << item.title << "\n";
        }
    }
}

std::vector<size_t> item_info_manager::get_items_needing_calorie_review() const
{
    std::vector<size_t> items_needing_review;
    
    for (size_t i = 0; i < m_items.size(); ++i)
    {
        if (m_items[i] && needs_calorie_review(*m_items[i]))
        {
            items_needing_review.push_back(i);
        }
    }
    
    return items_needing_review;
}

void item_info_manager::review_calories()
{
    auto items_needing_review = get_items_needing_calorie_review();
    
    if (items_needing_review.empty())
    {
        std::cout << "All items have correct calorie values.\n";
        return;
    }
    
    std::cout << "There are " << items_needing_review.size() 
              << " items with calories different from nutrition-based calculation. Would you like to review them? (y/n): ";
    
    char answer;
    std::cin >> answer;
    
    if (answer != 'y' && answer != 'Y')
        return;
    
    for (size_t index : items_needing_review)
    {
        auto& item = *m_items[index];
        float calculated_cal = item.calc_calories();
        
        std::cout << "\nItem: '" << item.title << "'\n";
        std::cout << "Stored calories: " << item.cal << "\n";
        std::cout << "Calculated from nutrition: " << calculated_cal << "\n";
        std::cout << "Replace stored value with calculated? (y/n): ";
        
        std::cin >> answer;
        if (answer == 'y' || answer == 'Y')
        {
            item.cal = calculated_cal;
            save_item_at_index(index);
            std::cout << "Updated calories for " << item.title << "\n";
        }
    }
}

void item_info_manager::build_title_index()
{
    m_title_to_index.clear();
    for (size_t i = 0; i < m_items.size(); ++i)
    {
        if (m_items[i])
        {
            m_title_to_index[m_items[i]->title] = i;
        }
    }
}

bool item_info_manager::parse_line(const std::string& line, item_info_ptr& info)
{
    auto v = utils::split(line, "\t");
    if (v.size() < 3)
        return false;
    
    info = std::make_shared<item_info>();
    
    // Parse title and aliases
    item_info::aliases_list_t aliases;
    parse_aliases(v[0], aliases);
    if (aliases.empty())
        return false;
    
    info->title = *aliases.begin();
    info->aliases = aliases;
    
    // Parse nutrition
    if (!parse_nutrition(std::string(v[1]), info->nutrition))
        return false;
    
    // Parse calories
    if (!parse_calories(std::string(v[2]), info->cal))
        return false;
    
    return true;
}

std::string item_info_manager::serialize_item(const item_info& info)
{
    std::ostringstream oss;
    
    // Write title (first alias becomes the main title)
    oss << info.title;
    
    // Write aliases if there are more than just the title
    if (info.aliases.size() > 1)
    {
        bool first = true;
        for (const auto& alias : info.aliases)
        {
            // Skip the main title since it's already written
            if (alias == info.title)
                continue;
                
            if (!first) oss << "=";
            else oss << "="; // First alias after title needs = too
            oss << alias;
            first = false;
        }
    }
    
    oss << "\t";
    
    // Write nutrition
    for (size_t i = 0; i < info.nutrition.size(); ++i)
    {
        if (i > 0) oss << "/";
        oss << info.nutrition[i];
    }
    
    oss << "\t" << info.cal;
    
    return oss.str();
}

void item_info_manager::update_file_line(size_t line_number, const std::string& content)
{
    // For now, we'll use the simpler approach of rewriting the entire file
    // This can be optimized later with actual line seeking if needed
    save_all();
}

bool item_info_manager::is_item_complete(const item_info& info) const
{
    // Check basic completeness
    if (info.title.empty() || info.nutrition.size() != 4)
        return false;
    
    // For calorie validation, we'll be more lenient
    // Only consider it incomplete if calories are significantly wrong
    // (this will be caught in calorie review instead)
    return true;
}

bool item_info_manager::needs_calorie_review(const item_info& info) const
{
    if (!is_item_complete(info))
        return false;
    
    float calculated_cal = info.calc_calories();
    float diff = std::abs(calculated_cal - info.cal);
    
    // Consider significant if difference is more than 5% or more than 10 calories
    return (diff > calculated_cal * 0.05f) && (diff > 10.0f);
}

// Static instance management
namespace {
    std::unique_ptr<item_info_manager> g_manager_instance = nullptr;
    std::filesystem::path g_default_path;
}

void item_info_manager::initialize()
{
    if (!g_manager_instance)
    {
        g_default_path = std::filesystem::temp_directory_path() / "nc_item_info.txt";
        g_manager_instance = std::make_unique<item_info_manager>(g_default_path);
        g_manager_instance->load_all();
        g_manager_instance->validate_and_fix_items();
        g_manager_instance->review_calories();
    }
}

item_info_manager& item_info_manager::get_instance()
{
    if (!g_manager_instance)
        initialize();
    return *g_manager_instance;
}

void item_info_manager::shutdown()
{
    if (g_manager_instance)
    {
        g_manager_instance->save_all();
        g_manager_instance.reset();
    }
}
