add_executable(VulkanRendererTest
        main1.cpp
)

target_link_libraries(VulkanRendererTest
        PRIVATE
        VulkanRenderer
)