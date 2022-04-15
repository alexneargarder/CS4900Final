#pragma once

#include "AftrGLRenderer.h"

#define XR_USE_GRAPHICS_API_OPENGL
#define XR_USE_PLATFORM_WIN32
#include "windows.h"
#include "openxr.h"
#include "openxr_platform.h"


namespace Aftr
{

class VRRenderer : public AftrGLRenderer
{
public:

	static VRRenderer* New(XrSession session, XrViewConfigurationView* viewconfig_views, XrSpace play_space);
	virtual ~VRRenderer();

	virtual void render(Camera& cam, WorldContainer& wList);

protected:

	VRRenderer();
	virtual void onCreate(XrSession session, XrViewConfigurationView* viewconfig_views, XrSpace play_space);

	XrViewConfigurationView* viewconfig_views;
	XrView* views;

	std::vector<XrSwapchain> swapchains;
	std::vector<uint32_t> swapchain_lengths;
	std::vector<XrSwapchainImageOpenGLKHR*> images;
	XrCompositionLayerProjectionView* projection_views;
	XrSession xrSession;
	XrSpace play_space;

	AftrFrameBufferObject* leftFBO;
	AftrFrameBufferObject* rightFBO;
};

}