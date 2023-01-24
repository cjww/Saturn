#pragma once

#include "Resources\ResourceManager.hpp"

#include "Resources\Texture.hpp"

#define SA_SUBPASS_EXTERNAL (~0U)

namespace sa {
	class RenderProgram;
	class Subpass;
	class VulkanCore;

	class RenderProgramFactory;

	enum SubpassAttachmentUsage {
		ColorTarget,
		DepthTarget,
		Input,
		Resolve,
	};

	typedef uint32_t AttachmentFlags;
	enum AttachmentFlagBits : AttachmentFlags {
		eClear = 1 << 0,
		eStore = 1 << 1,
		eSampled = 1 << 2,
		eLoad = 1 << 3
	};

	class RenderProgramFactory {
	public:

		struct AttachmentReference {
			uint32_t index;
			SubpassAttachmentUsage usage;
		};

		class SubpassFactory {
		private:
			Subpass* m_pSubpass;
			RenderProgramFactory* m_pProgramFactory;
			std::vector<AttachmentReference> m_attachmentReferences;
		public:
			SubpassFactory(RenderProgramFactory* pProgramFactory, Subpass* pSubpass);

			SubpassFactory& addAttachmentReference(uint32_t index, SubpassAttachmentUsage usage);

			RenderProgramFactory& endSubpass();

			std::vector<AttachmentReference> getAttachmentReferences() const;

		};
	private:
		RenderProgram* m_pProgram;
		ResourceID m_id;
		VulkanCore* m_pCore;

		std::vector<SubpassFactory> m_subpasses;

		void onSubpassEnd(SubpassFactory subpassFactory);

	public:

		RenderProgramFactory(VulkanCore* pCore);
		
		RenderProgramFactory& addColorAttachment(AttachmentFlags flags, uint32_t sampleCount = 1U);
		RenderProgramFactory& addColorAttachment(AttachmentFlags flags, const Texture2D& framebufferTexture);
		RenderProgramFactory& addColorAttachment(AttachmentFlags flags, Format format, uint32_t sampleCount = 1U);


		RenderProgramFactory& addSwapchainAttachment(ResourceID swapchain);
		RenderProgramFactory& addDepthAttachment(AttachmentFlags flags, uint32_t sampleCount = 1U);
		RenderProgramFactory& addDepthAttachment(AttachmentFlags flags, const Texture2D& framebufferTexture);
		RenderProgramFactory& addDepthAttachment(AttachmentFlags flags, Format format, uint32_t sampleCount = 1U);



		SubpassFactory beginSubpass();
		RenderProgramFactory& addColorDependency(uint32_t srcSubpass = SA_SUBPASS_EXTERNAL, uint32_t dstSubpass = SA_SUBPASS_EXTERNAL);
		

		ResourceID end();


	};
}