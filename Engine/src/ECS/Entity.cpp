#include "Entity.h"

EntityWrapper::EntityWrapper(EntityID id)
    : id(id)
    , tags(0)
{

}

ComponentMask& EntityWrapper::getSignature() {
    return m_componentMask;
}
