#pragma once
#include "Vector2.h"
#include "Debug.h"
#include "CollisionDetection.h"

namespace NCL {
	using namespace NCL::Maths;
	namespace CSC8503 {
		template<class T>
		class QuadTree;

		template<class T>
		struct QuadTreeEntry {
			Vector3 pos;
			Vector3 size;
			T object;

			QuadTreeEntry(T obj, Vector3 pos, Vector3 size) {
				object		= obj;
				this->pos	= pos;
				this->size	= size;
			}
		};

		template<class T>
		class QuadTreeNode	{
		public:
			typedef std::function<void(std::list<QuadTreeEntry<T>>&)> QuadTreeFunc;
		protected:
			friend class QuadTree<T>;

			QuadTreeNode() {}

			QuadTreeNode(Vector2 pos, Vector2 size) {
				children		= nullptr;
				this->position	= pos;
				this->size		= size;
			}

			~QuadTreeNode() {
				delete[] children;
			}

			void Insert(T& object, const Vector3& objectPos, const Vector3& objectSize, int depthLeft, int maxSize) {
				if (!CollisionDetection::AABBTest(objectPos,
					Vector3(position.x, 0, position.y),
					objectSize, Vector3(size.x, 1000.0f, size.y))) {
					return;
				}
				if (children) {	//not a leaf node , just descend the tree
					for (int i = 0; i < 4; ++i) {
						children[i].Insert(object, objectPos, objectSize, depthLeft - 1, maxSize);
					}
				}
				else { // currently a leaf node , can just expand
					contents.push_back(QuadTreeEntry<T>(object, objectPos, objectSize));
					if ((int)contents.size() > maxSize && depthLeft > 0) {
						if (!children) {
							Split();
							//we need to reinsert the contents so far!
							for (const auto& i : contents) {
								for (int j = 0; j < 4; ++j) {
									auto entry = i;
									children[j].Insert(entry.object, entry.pos, entry.size, depthLeft - 1, maxSize);
								}
							}
							contents.clear();
						}
					}
				}
			}

			void Split() {
				Vector2 halfSize = size / 2.0f;
				children = new QuadTreeNode<T>[4];
				children[0] = QuadTreeNode<T>(position + Vector2(-halfSize.x, halfSize.y), halfSize);
				children[1] = QuadTreeNode<T>(position + Vector2(halfSize.x, halfSize.y), halfSize);
				children[2] = QuadTreeNode<T>(position + Vector2(-halfSize.x, -halfSize.y), halfSize);
				children[3] = QuadTreeNode<T>(position + Vector2(halfSize.x, -halfSize.y), halfSize);
			}

			void DebugDraw(float dt) {
				if (children) {
					for (int i = 0; i < 4; i++) {
						children[i].DebugDraw(dt);
					}
					return;
				}

				Vector3 pos = Vector3(position.x, 0, position.y);

				Vector3 vert1 = pos + Vector3(size.x, 10, size.y);
				Vector3 vert2 = pos + Vector3(size.x, 10, -size.y);
				Vector3 vert3 = pos + Vector3(-size.x, 10, size.y);
				Vector3 vert4 = pos + Vector3(-size.x, 10, -size.y);

				Vector3 vert5 = pos + Vector3(size.x, -10, size.y);
				Vector3 vert6 = pos + Vector3(size.x, -10, -size.y);
				Vector3 vert7 = pos + Vector3(-size.x, -10, size.y);
				Vector3 vert8 = pos + Vector3(-size.x, -10, -size.y);

				Debug::DrawLine(vert1, vert2, Vector4(0, 0, 0, 1), dt);
				Debug::DrawLine(vert1, vert3, Vector4(0, 0, 0, 1), dt);
				Debug::DrawLine(vert1, vert5, Vector4(0, 0, 0, 1), dt);

				Debug::DrawLine(vert2, vert4, Vector4(0, 0, 0, 1), dt);
				Debug::DrawLine(vert2, vert6, Vector4(0, 0, 0, 1), dt);

				Debug::DrawLine(vert3, vert4, Vector4(0, 0, 0, 1), dt);
				Debug::DrawLine(vert3, vert7, Vector4(0, 0, 0, 1), dt);

				Debug::DrawLine(vert4, vert8, Vector4(0, 0, 0, 1), dt);

				Debug::DrawLine(vert5, vert6, Vector4(0, 0, 0, 1), dt);
				Debug::DrawLine(vert5, vert7, Vector4(0, 0, 0, 1), dt);

				Debug::DrawLine(vert6, vert8, Vector4(0, 0, 0, 1), dt);
				Debug::DrawLine(vert7, vert8, Vector4(0, 0, 0, 1), dt);
			}

			void OperateOnContents(QuadTreeFunc& func) {
				if (children) {
					for (int i = 0; i < 4; ++i) {
						children[i].OperateOnContents(func);
					}
				}
				else {
					if (!contents.empty()) {
						func(contents);
					}
				}
			}

		protected:
			std::list< QuadTreeEntry<T> >	contents;

			Vector2 position;
			Vector2 size;

			QuadTreeNode<T>* children;
		};
	}
}


namespace NCL {
	using namespace NCL::Maths;
	namespace CSC8503 {
		template<class T>
		class QuadTree
		{
		public:
			QuadTree(Vector2 size, int maxDepth = 6, int maxSize = 5){
				root = QuadTreeNode<T>(Vector2(), size);
				this->maxDepth	= maxDepth;
				this->maxSize	= maxSize;
			}
			~QuadTree() {
			}

			void Insert(T object, const Vector3& pos, const Vector3& size) {
				root.Insert(object, pos, size, maxDepth, maxSize);
			}

			void DebugDraw(float dt) {
				root.DebugDraw(dt);
			}

			void OperateOnContents(typename QuadTreeNode<T>::QuadTreeFunc  func) {
				root.OperateOnContents(func);
			}

		protected:
			QuadTreeNode<T> root;
			int maxDepth;
			int maxSize;
		};
	}
}