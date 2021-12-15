#pragma once

#include <unordered_map>
#include <functional>
#include <typeindex>

#include "Box.h"

using TypePair = std::pair<std::type_index, std::type_index>;

class CollisionEventManager : public b2ContactListener {
private:
	struct CompColorTrait {
		bool operator()(const TypePair& lhs, const TypePair& rhs) const {
			return lhs.first == rhs.first && lhs.second == rhs.second;
		}
	};
	struct HashColorTrait {
		size_t operator()(const TypePair& pair) const {
			std::hash<std::type_index> hasher;

			auto hashlhs = hasher(pair.first);
			auto hashrhs = hasher(pair.second);

			hashlhs ^= hashrhs + 0x9e3779b9 + (hashlhs << 6) + (hashlhs >> 2);

			return hashlhs;
		}
	};

public:
	template<class T, class U, class F>
	void Case(F f) {
		map[{std::type_index(typeid(T)), std::type_index(typeid(U))}] = f;
		map[{std::type_index(typeid(U)), std::type_index(typeid(T))}] = std::bind(
			f, std::placeholders::_2, std::placeholders::_1
		);
	}
	template<class T, class U>
	bool HasCase() const {
		return map.count({ std::type_index(typeid(T), std::type_index(typeid(U) }) > 0;
	}
	template<class T, class U>
	void ClearCase() {
		return map.erase({ std::type_index(typeid(T)), std::type_info(U)) });
		return map.erase({ std::type_index(typeid(U)), std::type_info(T)) });
	}
	template<class F>
	void Default(F f) {
		def = f;
	}
	void BeginContact(b2Contact* contact) override {
		b2Body* bodyPtrs[] = { contact->GetFixtureA()->GetBody(),contact->GetFixtureB()->GetBody() };
		if (bodyPtrs[0]->GetType() == b2BodyType::b2_dynamicBody &&
			bodyPtrs[1]->GetType() == b2BodyType::b2_dynamicBody) {
			Switch(
				*reinterpret_cast<Box*>(bodyPtrs[0]->GetUserData().pointer),
				*reinterpret_cast<Box*>(bodyPtrs[1]->GetUserData().pointer)
			);
		}
	}
private:
	void Switch(Box& a, Box& b) {
		auto i = map.find({
			std::type_index(typeid(a.GetColorTrait())),
			std::type_index(typeid(b.GetColorTrait()))
			});

		if (i != map.end()) {
			i->second(a, b);
		}
		else {
			def(a, b);
		}
	}

public:
	std::unordered_map<TypePair, std::function<void(Box&, Box&)>, HashColorTrait, CompColorTrait> map;
	std::function<void(Box&, Box&)> def = [](Box&, Box&) {};
};
