#pragma once
#include "TextureBase.h"
#include "ShaderBase.h"
#include <vector>

namespace NCL {
	using namespace NCL::Rendering;
	using namespace std;

	class MeshGeometry;
	namespace CSC8503 {
		class Transform;
		using namespace Maths;

		class RenderObject
		{
		public:
			RenderObject(Transform* parentTransform, MeshGeometry* mesh, TextureBase* tex, ShaderBase* shader);
			~RenderObject();

			void SetDefaultTexture(TextureBase* t) {
				texture = t;
			}

			TextureBase* GetDefaultTexture() const {
				return texture;
			}

			MeshGeometry*	GetMesh() const {
				return mesh;
			}

			Transform*		GetTransform() const {
				return transform;
			}

			ShaderBase*		GetShader() const {
				return shader;
			}

			void SetColour(const Vector4& c) {
				colour = c;
			}

			Vector4 GetColour() const {
				return colour;
			}

			const vector<Vector4>& GetVertexColours()	const { return vertColours; }

			void SetVertexColours(const vector<Vector4>& newColours) {
				vertColours = newColours;
			}

			void ClearVertexColours() {
				vertColours.clear();
			}
			
			vector <float> test;

		protected:
			MeshGeometry*	mesh;
			TextureBase*	texture;
			ShaderBase*		shader;
			Transform*		transform;
			Vector4			colour;

			vector<Vector4> vertColours;
		};
	}
}
