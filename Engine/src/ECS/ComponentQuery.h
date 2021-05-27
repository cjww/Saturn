#pragma once
#include "EntityFactory.h"

enum class QueryOperator {
	AND,
	OR
};
class ComponentQuery {
private:
	ComponentMask m_requiredComponents;
	ComponentMask m_components;
	QueryOperator m_op;
public:
	ComponentQuery();
	ComponentQuery(ComponentMask components, QueryOperator op = QueryOperator::AND);
	ComponentQuery(ComponentMask components, ComponentMask requiredComponents);

	bool check(ComponentMask signature) const;

};