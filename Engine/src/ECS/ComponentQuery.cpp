#include "Saturn/ECS/ComponentQuery.h"

ComponentQuery::ComponentQuery()
	: m_components(0)
	, m_requiredComponents(0)
	, m_op(QueryOperator::AND)
{
}

ComponentQuery::ComponentQuery(ComponentMask componentMask, QueryOperator op)
	: m_components(componentMask)
	, m_requiredComponents(0)
	, m_op(op)
{

}

ComponentQuery::ComponentQuery(ComponentMask components, ComponentMask requiredComponents) 
	: m_components(components)
	, m_requiredComponents(requiredComponents)
	, m_op(QueryOperator::OR)
{
	
}

bool ComponentQuery::check(ComponentMask signature) const {
	if (m_requiredComponents != 0) {
		if ((signature & m_requiredComponents) != m_requiredComponents) {
			return false;
		}
	}
	switch (m_op) {
	case QueryOperator::AND:
		return (signature & m_components) == m_components;
	case QueryOperator::OR:
		return (signature & m_components) != 0;
	default:
		throw std::runtime_error("Invalid operator");
		break;
	}
	return false;
}
