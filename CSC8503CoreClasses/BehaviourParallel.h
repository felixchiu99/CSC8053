#pragma once
#include "BehaviourNodeWithChildren.h"

class BehaviourParallel : public BehaviourNodeWithChildren {
public:
	BehaviourParallel(const std::string& nodeName) : BehaviourNodeWithChildren(nodeName) {}
	~BehaviourParallel() {}
	BehaviourState Execute(float dt) override {
		//std::cout << "Executing selector " << name << "\n";
		bool isOngoing = false;
		bool hasSuccess = false;
		for (auto& i : childNodes) {
			BehaviourState nodeState = i->Execute(dt);
			switch (nodeState) {
			case Failure: continue;
			case Success: hasSuccess = true; continue;
			case Ongoing:
			{
				currentState = nodeState;
				isOngoing = true;
			}
			}
		}
		if (isOngoing) {
			return Ongoing;
		}
		if (hasSuccess) {
			return Success;
		}
		return Failure;
	}
};
