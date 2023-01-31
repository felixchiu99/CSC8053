#pragma once
#include "BehaviourNodeWithChildren.h"

class BehaviourInverter : public BehaviourNodeWithChildren {
public:
	BehaviourInverter(const std::string& nodeName) : BehaviourNodeWithChildren(nodeName) {}
	~BehaviourInverter() {}
	BehaviourState Execute(float dt) override {
		//std::cout << "Executing selector " << name << "\n";
		bool isOngoing = false;
		bool hasSuccess = false;
		BehaviourState nodeState = childNode->Execute(dt);
		switch (nodeState) {
		case Failure: nodeState = Success;
		case Success: nodeState = Failure;
		case Ongoing:
		{
			currentState = nodeState;
		}
		}
		return nodeState;
	}
	void AddChild(BehaviourNode* n) {
		childNode = n;
	}
protected:
	BehaviourNode* childNode;

};
