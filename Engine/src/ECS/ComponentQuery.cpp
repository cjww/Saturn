#include "ComponentQuery.h"

ComponentQuery::ComponentQuery()
	: m_componentMask(0)
	, m_op(QueryOperator::AND)
{
}

ComponentQuery::ComponentQuery(ComponentMask componentMask, QueryOperator op)
	: m_componentMask(componentMask)
	, m_op(op)
{

}

bool ComponentQuery::check(ComponentMask signature) const {
	switch (m_op) {
	case QueryOperator::AND:
		return (signature & m_componentMask) == m_componentMask;
	case QueryOperator::OR:
		return (signature & m_componentMask) != 0;
	default:
		throw std::runtime_error("Invalid operator");
		break;
	}
	return false;
}
