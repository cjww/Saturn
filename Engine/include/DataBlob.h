#pragma once
#include "DataLayout.h"
namespace sa {
    class DataBlob {
    private:
        void* m_data = nullptr;
        DataLayout& m_layout;
    public:
        explicit DataBlob(DataLayout &m_layout)
            : m_data(malloc(m_layout.getSize())),
              m_layout(m_layout) {
        }
        ~DataBlob() {
            free(m_data);
        }

        void setMember(const char* member, const void* data) const;
        template<typename T>
        void setMember(const char* member, T value) const;

        void* getMember(const char* member) const;
        template<typename T>
        T getMember(const char* member) const;
    };

    template<typename T>
    void DataBlob::setMember(const char *member, T value) const {
        setMember(member, value);
    }

    template<typename T>
    T DataBlob::getMember(const char *member) const {
        return reinterpret_cast<T>(member);
    }
}
