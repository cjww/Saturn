#include "DataLayout.h"

namespace sa {
    DataLayout::DataLayout(DataMember *pMembers, int memberCount) {
        for (int i = 0; i < memberCount; i++) {
            m_members.insert(std::make_pair(pMembers[i].name, pMembers[i]));
        }
        DataMember lastMember = pMembers[memberCount - 1];
        m_size = lastMember.offset + lastMember.size;
    }

    const DataMember & DataLayout::getMemeber(const char *name) const{
        return m_members.at(name);
    }

    int DataLayout::getSize() const {
        return m_size;
    }

    DataLayoutFactory&& DataLayoutFactory::NewLayout() {
        return DataLayoutFactory();
    }

    DataLayoutFactory& DataLayoutFactory::addMember(const char *name, int size) {
        m_members.push_back(DataMember{
            .offset = m_cursor,
            .size = size,
            .name = name
        });
        m_cursor += size;
        return *this;
    }

    DataLayoutFactory& DataLayoutFactory::addPadding(int padding) {
        m_cursor += padding;
        return *this;
    }

    DataLayout DataLayoutFactory::create() {
        return DataLayout(m_members.data(), m_members.size());
    }
}
