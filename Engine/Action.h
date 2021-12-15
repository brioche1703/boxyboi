#pragma once

#include "Box.h"

class Action {
public:
	virtual void Do(std::vector<std::unique_ptr<Box>>& boxes, b2World& world) = 0;
};

class Split : public Action {
public:
	Split(Box& target)
		:
		pTarget(&target) 
	{}
	void Do(std::vector<std::unique_ptr<Box>>& boxes, b2World& world) override {
		if (pTarget->GetSize() >= 0.1f) {
			auto splits = pTarget->GetSplits(world);
			boxes.insert(boxes.end(),
				std::make_move_iterator(splits.begin()),
				std::make_move_iterator(splits.end())
			);
		}
	}

private:
	Box* pTarget;
};

class AssumeColor : public Action {
public:
	AssumeColor(Box& target, std::unique_ptr<Box::ColorTrait> pColorTrait) 
		:
		pColorTrait(std::move(pColorTrait)),
		pTarget(&target) 
	{}
	void Do(std::vector<std::unique_ptr<Box>>& boxes, b2World& world) override {
		pTarget->AssumeColorTrait(std::move(pColorTrait));
	}

private:
	std::unique_ptr<Box::ColorTrait> pColorTrait;
	Box* pTarget;
};
