#pragma once

namespace sa {

	class UUID {
	private:
		uint64_t m_uuid;
	public:
		UUID();
		UUID(uint64_t uuid);

		operator uint64_t() const { return m_uuid; }
	};
}

namespace std {
	template<>
	struct hash<sa::UUID> {
		size_t operator()(const sa::UUID& id) const {
			return hash<uint64_t>()((uint64_t)id);
		}
	};
}