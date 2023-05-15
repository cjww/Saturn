#pragma once
#include <structs.hpp>
#include <Resources/ResourceManager.hpp>
#include <Resources/Texture.hpp>
#include <Resources/Buffer.hpp>
#include <RenderContext.hpp>

#include <Tools/Logger.hpp>

#include "Assets/TextureAsset.h"

#include "Assets\MaterialShader.h"

#define MAX_TEXTURE_MAP_COUNT 4U

namespace sa {
	
	// same as the assimp aiTextureType enum
	enum class MaterialTextureType : int32_t {
        /** @brief Defines the purpose of a texture
         *
         *  This is a very difficult topic. Different 3D packages support different
         *  kinds of textures. For very common texture types, such as bumpmaps, the
         *  rendering results depend on implementation details in the rendering
         *  pipelines of these applications. Assimp loads all texture references from
         *  the model file and tries to determine which of the predefined texture
         *  types below is the best choice to match the original use of the texture
         *  as closely as possible.<br>
         *
         *  In content pipelines you'll usually define how textures have to be handled,
         *  and the artists working on models have to conform to this specification,
         *  regardless which 3D tool they're using.
         */
        /** Dummy value.
         *
         *  No texture, but the value to be used as 'texture semantic'
         *  (#aiMaterialProperty::mSemantic) for all material properties
         *  *not* related to textures.
         */
        NONE = 0,

		/** LEGACY API MATERIALS
		* Legacy refers to materials which
		* Were originally implemented in the specifications around 2000.
		* These must never be removed, as most engines support them.
		*/

		/** The texture is combined with the result of the diffuse
		*  lighting equation.
		*  OR
		*  PBR Specular/Glossiness
		*/
		DIFFUSE = 1,

		/** The texture is combined with the result of the specular
		*  lighting equation.
		*  OR
		*  PBR Specular/Glossiness
		*/
		SPECULAR = 2,

		/** The texture is combined with the result of the ambient
		*  lighting equation.
		*/
		AMBIENT = 3,

		/** The texture is added to the result of the lighting
		*  calculation. It isn't influenced by incoming light.
		*/
		EMISSIVE = 4,

		/** The texture is a height map.
		*
		*  By convention, higher gray-scale values stand for
		*  higher elevations from the base height.
		*/
		HEIGHT = 5,

		/** The texture is a (tangent space) normal-map.
		*
		*  Again, there are several conventions for tangent-space
		*  normal maps. Assimp does (intentionally) not
		*  distinguish here.
		*/
		NORMALS = 6,

		/** The texture defines the glossiness of the material.
		*
		*  The glossiness is in fact the exponent of the specular
		*  (phong) lighting equation. Usually there is a conversion
		*  function defined to map the linear color values in the
		*  texture to a suitable exponent. Have fun.
		*/
		SHININESS = 7,

		/** The texture defines per-pixel opacity.
		*
		*  Usually 'white' means opaque and 'black' means
		*  'transparency'. Or quite the opposite. Have fun.
		*/
		OPACITY = 8,

		/** Displacement texture
		*
		*  The exact purpose and format is application-dependent.
		*  Higher color values stand for higher vertex displacements.
		*/
		DISPLACEMENT = 9,

		/** Lightmap texture (aka Ambient Occlusion)
		*
		*  Both 'Lightmaps' and dedicated 'ambient occlusion maps' are
		*  covered by this material property. The texture contains a
		*  scaling value for the final color value of a pixel. Its
		*  intensity is not affected by incoming light.
		*/
		LIGHTMAP = 10,

		/** Reflection texture
		*
		* Contains the color of a perfect mirror reflection.
		* Rarely used, almost never for real-time applications.
		*/
		REFLECTION = 11,

		/** PBR Materials
		* PBR definitions from maya and other modelling packages now use this standard.
		* This was originally introduced around 2012.
		* Support for this is in game engines like Godot, Unreal or Unity3D.
		* Modelling packages which use this are very common now.
		*/

		BASE_COLOR = 12,
		NORMAL_CAMERA = 13,
		EMISSION_COLOR = 14,
		METALNESS = 15,
		DIFFUSE_ROUGHNESS = 16,
		AMBIENT_OCCLUSION = 17,

		/** PBR Material Modifiers
		* Some modern renderers have further PBR modifiers that may be overlaid
		* on top of the 'base' PBR materials for additional realism.
		* These use multiple texture maps, so only the base type is directly defined
		*/

		/** Sheen
		* Generally used to simulate textiles that are covered in a layer of microfibers
		* eg velvet
		* https://github.com/KhronosGroup/glTF/tree/master/extensions/2.0/Khronos/KHR_materials_sheen
		*/
		SHEEN = 19,

		/** Clearcoat
		* Simulates a layer of 'polish' or 'laquer' layered on top of a PBR substrate
		* https://autodesk.github.io/standard-surface/#closures/coating
		* https://github.com/KhronosGroup/glTF/tree/master/extensions/2.0/Khronos/KHR_materials_clearcoat
		*/
		CLEARCOAT = 20,

		/** Transmission
		* Simulates transmission through the surface
		* May include further information such as wall thickness
		*/
		TRANSMISSION = 21,

		/** Unknown texture
		*
		*  A texture reference that does not match any of the definitions
		*  above is considered to be 'unknown'. It is still imported,
		*  but is excluded from any further post-processing.
		*/
		UNKNOWN = 18,
	};

	enum class TextureBlendOp {
		/** T = T1 * T2 */
		MULTIPLY = 0x0,

		/** T = T1 + T2 */
		ADD = 0x1,

		/** T = T1 - T2 */
		SUBTRACT = 0x2,

		/** T = T1 / T2 */
		DIVIDE = 0x3,

		/** T = (T1 + T2) - (T1 * T2) */
		SMOOTH_ADD = 0x4,

		/** T = T1 + (T2-0.5) */
		SIGNED_ADD = 0x5,
	};

	struct BlendedTexture {
		UUID textureAssetID;
		float blendFactor;
		TextureBlendOp blendOp;
	};

	class Material : public IAsset{
	private:
		std::unordered_map<MaterialTextureType, std::vector<UUID>> m_textures;
		std::unordered_map<MaterialTextureType, std::vector<std::pair<TextureBlendOp, float>>> m_blending;
		std::vector<Texture> m_allTextures;
		bool m_allTexturesLoaded;

		MaterialShader* m_pMaterialShader;

		void setTextures(const std::vector<BlendedTexture>& textures, MaterialTextureType type, uint32_t& count);

	public:
		struct alignas(16) Values { // sent to shader
			Color diffuseColor = { 1, 1, 1, 1 };
			Color specularColor = { 1, 1, 1, 1 };
			Color ambientColor = { 1, 1, 1, 1 };
			Color emissiveColor = { 1, 1, 1, 1 };

			uint32_t albedoMapFirst = 0;
			uint32_t albedoMapCount = 0;

			uint32_t normalMapFirst = 0;
			uint32_t normalMapCount = 0;

			uint32_t metalnessMapFirst = 0;
			uint32_t metalnessMapCount = 0;
			
			uint32_t emissiveMapFirst = 0;
			uint32_t emissiveMapCount = 0;

			uint32_t occlusionMapFirst = 0;
			uint32_t occlusionMapCount = 0;

			float opacity = 1.0f;
			float shininess = 1.0f;
			float metallic = 0.0f;
		} values;

		bool twoSided;


		static std::string TextureTypeToString(MaterialTextureType type);


		Material(const AssetHeader& header);

		void update();
	
		void setTextures(const std::vector<BlendedTexture>& textures, MaterialTextureType type);
		
		// Gathers all textures into an array, unless already gathered since last update
		const std::vector<Texture>& fetchTextures();
		
		std::unordered_map<MaterialTextureType, std::vector<UUID>>& getTextures();

		MaterialShader* getMaterialShader() const;
		void setMaterialShader(MaterialShader* pMaterialShader);

		virtual bool onLoad(std::ifstream& file, AssetLoadFlags flags) override;
		virtual bool onWrite(std::ofstream& file, AssetWriteFlags flags) override;
		virtual bool onUnload() override;


	};
}