#pragma
#include <filesystem>

namespace PathUtils{
    bool IsParentPath(const std::filesystem::path parent, const std::filesystem::path child)
    {
        auto manipulatedChild  = std::filesystem::weakly_canonical(child);

        while (!manipulatedChild.empty())
        {
            if (manipulatedChild == parent)
                return true;
            manipulatedChild = child.parent_path();
        }
        return false;
    }
};