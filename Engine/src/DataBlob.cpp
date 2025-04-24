#include "DataBlob.h"
namespace sa {
    void DataBlob::setMember(const char *member, const void *data) const {
        DataMember memberData = m_layout.getMemeber(member);
        memcpy(static_cast<char*>(m_data) + memberData.offset, data, memberData.size);
    }

    void* DataBlob::getMember(const char* member) const {
        const DataMember memberData = m_layout.getMemeber(member);
        return static_cast<char*>(m_data) + memberData.offset;
    }
}