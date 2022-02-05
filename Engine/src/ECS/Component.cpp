#include "pch.h"

#include "MetaComponent.h"
#include "Component.h"

namespace sa {

    void* Component::data() {
        return m_comp.data();
    }

    bool Component::isValid() {
        return m_comp.isValid();
    }

}