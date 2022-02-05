#pragma once


namespace sa {
	// Internal class
	class MetaComponent;
	
	// Represents an instance of a component of any type 
	class Component {
	private:
		MetaComponent m_comp;

	public:
		template<typename T>
		T* cast();
		void* data();
		bool isValid();
	};

	template<typename T>
	inline T* Component::cast() {
		return m_comp.cast<T>();
	}
}