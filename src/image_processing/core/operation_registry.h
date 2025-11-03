#pragma one

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../core/operation_base.h"

class OperationRegistry {
   public:
    using OperationFactory = std::function<std::shared_ptr<ImageOperation>()>;

    // Categories for filters
    enum class Category { COLOR_EFFECTS, VINTAGE, BLACK_WHITE, BINARY };

    // filter information
    struct FilterInfo {
        std::string name;
        Category category;
        std::string description;
        std::string iconName;  // for UI-Icons
    };

    // single pattern
    static OperationRegistry& getInstance();

    // Filter register
    void registerFiler(const std::string& name, OperationFactory factory, Category category,
                       const std::string& description = "", const std::string& iconName = "");

    // create filter
    std::shared_ptr<ImageOperation> createFilter(const std::string& name);

    // list available filter
    std::vector<std::string> getFilterByCategory(Category category) const;

    // Filter information
    FilterInfo getFilterInfo(const std::string& name) const;

    static std::string categoryToString(Category category);

   private:
    OperationRegistry();
    void registerDefaultFilters();

    std::unordered_map<std::string, OperationFactory> factories;
    std::unordered_map<std::string, FilterInfo> filterInfos;
    std::unordered_map<Category, std::vector<std::string>> filtersByCategory;
};