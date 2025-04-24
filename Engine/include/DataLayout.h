#pragma once
#include <vector>

namespace sa {
    struct DataMember {
        int offset;
        int size;
        const char* name;
    };

    class DataLayout {
    private:
        std::unordered_map<const char*, DataMember> m_members;
        int m_size;
    public:
        DataLayout(DataMember* pMembers, int memberCount);
        const DataMember& getMemeber(const char* name) const;
        int getSize() const;
    };

    class DataLayoutFactory {
    private:
        std::vector<DataMember> m_members;
        int m_cursor = 0;
        DataLayoutFactory() = default;
    public:
        static DataLayoutFactory&& NewLayout();

        template<typename T>
        DataLayoutFactory& addMember(const char* name);
        DataLayoutFactory& addMember(const char* name, int size);

        DataLayoutFactory& addPadding(int padding);

        DataLayout create();
    };

    template<typename T>
    DataLayoutFactory& DataLayoutFactory::addMember(const char *name) {
        return addMember(name, sizeof(T));
    }

}
