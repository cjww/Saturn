#pragma once
#include "EntityFactory.h"

enum class QueryOperator {
	AND,
	OR
};
class ComponentQuery {
private:
	ComponentMask m_componentMask;
	QueryOperator m_op;
public:
	ComponentQuery();
	ComponentQuery(ComponentMask componentMask, QueryOperator op = QueryOperator::AND);

	bool check(ComponentMask signature) const;

};