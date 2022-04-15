#include "VRRenderer.h"
#include "AftrFrameBufferObject.h"
#include "IndexedGeometryQuad.h"
#include "Camera.h"
#include "WorldContainer.h"
//#include "WorldContainer.h"
//#include "ModelMeshSkin.h"
//#include "Axes.h"
//#include "AftrUtilities.h"
//#include "AftrUtil_matrix.h"
//#include "CameraStandardEZNav.h"
//#include "Model.h"
//#include "WOLight.h"
//#include "SelectionQueryResult.h"

using namespace Aftr;

VRRenderer* VRRenderer::New( XrSession session, XrViewConfigurationView* viewconfig_views, XrSpace play_space)
{
	VRRenderer* r = new VRRenderer();
	r->onCreate(session, viewconfig_views, play_space);
	return r;
}

VRRenderer::VRRenderer()
{
	xrSession = XR_NULL_HANDLE;
}

VRRenderer::~VRRenderer() 
{

}

void VRRenderer::onCreate(XrSession session, XrViewConfigurationView* viewconfig_views, XrSpace play_space )
{
	std::cout << "BEGIN ON CREATE RENDERER" << std::endl;
	AftrGLRenderer::onCreate();

	this->viewconfig_views = viewconfig_views;
	this->xrSession = session;
	this->play_space = play_space;

	this->leftOrBottomStereoFBO = AftrFrameBufferObject::New(ManagerWindowing::getWindowWidth(), ManagerWindowing::getWindowHeight(), 1, true, false);
	this->rightOrTopStereoFBO = AftrFrameBufferObject::New(ManagerWindowing::getWindowWidth(), ManagerWindowing::getWindowHeight());

	//leftOrBottomStereoQuad = IndexedGeometryQuad::New(QuadOrientation::qoXY, .5, 1, Vector(1, 1, 1), true);
	//rightOrTopStereoQuad = IndexedGeometryQuad::New(QuadOrientation::qoXY, .5, 1, Vector(1, 1, 1), true);

	uint32_t view_count = 2;
	XrResult result;

	uint32_t preferred_format = GL_SRGB8_ALPHA8_EXT;

	uint32_t swapchain_format_count;
	result = xrEnumerateSwapchainFormats(session, 0, &swapchain_format_count, NULL);
	
	std::cout << "ENUMERATE SWAPCHAIN RESULT: " << ((result == XR_SUCCESS) ? "succeed" : "fail") << " " << result << std::endl;

	printf("Runtime supports %d swapchain formats\n", swapchain_format_count);
	int64_t* swapchain_formats = new int64_t[swapchain_format_count];

	result = xrEnumerateSwapchainFormats(session, swapchain_format_count, &swapchain_format_count,
		swapchain_formats);

	std::cout << "ENUMERATE SWAPCHAIN RESULT: " << ((result == XR_SUCCESS) ? "succeed" : "fail") << " " << result << std::endl;

	int64_t chosen_format = swapchain_formats[0];

	for (uint32_t i = 0; i < swapchain_format_count; i++) {
		printf("Supported GL format: %#lx\n", swapchain_formats[i]);
		if (swapchain_formats[i] == preferred_format) {
			chosen_format = swapchain_formats[i];
			printf("Using preferred swapchain format %#lx\n", chosen_format);
			break;
		}
	}
	if (chosen_format != preferred_format) {
		printf("Falling back to non preferred swapchain format %#lx\n", chosen_format);
	}

	delete swapchain_formats;

	swapchains = std::vector<XrSwapchain>(view_count);
	swapchain_lengths = std::vector<uint32_t>(view_count);

	for (uint32_t i = 0; i < view_count; i++) {
		XrSwapchainCreateInfo swapchain_create_info = {
			.type = XR_TYPE_SWAPCHAIN_CREATE_INFO,
			.next = NULL,
			.createFlags = 0,
			.usageFlags = XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT,
			//.usageFlags = 0,
			.format = chosen_format,
			//.sampleCount = 5,
			//.width = 5,
			//.height = 5,
			.sampleCount = viewconfig_views[i].recommendedSwapchainSampleCount,
			.width = viewconfig_views[i].recommendedImageRectWidth,
			.height = viewconfig_views[i].recommendedImageRectHeight,
			.faceCount = 1,
			.arraySize = 1,
			.mipCount = 1,
		};

		std::cout << "BEFORE CREATE SWAPCHAIN" << std::endl;
		std::cout << swapchain_create_info.sampleCount << " " << swapchain_create_info.width << " " << swapchain_create_info.height << std::endl;

		result = xrCreateSwapchain(session, &swapchain_create_info, &swapchains[i]);
		
		std::cout << "CREATE SWAPCHAIN RESULT: " << ((result == XR_SUCCESS) ? "succeed" : "fail") << " " << result << std::endl;


		// The runtime controls how many textures we have to be able to render to
		// (e.g. "triple buffering")
		result = xrEnumerateSwapchainImages(swapchains[i], 0, &swapchain_lengths[i], NULL);
		
		std::cout << "ENUMERATE SWAPCHAIN RESULT: " << ((result == XR_SUCCESS) ? "succeed" : "fail") << " " << result << std::endl;

		std::cout << "BEFORE HERE" << std::endl;
		images.push_back(new XrSwapchainImageOpenGLKHR[swapchain_lengths[i]]);
		for (uint32_t j = 0; j < swapchain_lengths[i]; j++) {
			images[i][j].type = XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_KHR;
			images[i][j].next = NULL;
		}
		std::cout << "AFTER HERE" << std::endl;
		result =
			xrEnumerateSwapchainImages(swapchains[i], swapchain_lengths[i], &swapchain_lengths[i],
				(XrSwapchainImageBaseHeader*)images[i]);
		
		std::cout << "ENUMERATE SWAPCHAIN RESULT: " << ((result == XR_SUCCESS) ? "succeed" : "fail") << " " << result << std::endl;
	}

	projection_views = new XrCompositionLayerProjectionView[view_count];
	for (uint32_t i = 0; i < view_count; i++) {
		projection_views[i].type = XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW;
		projection_views[i].next = NULL;
		projection_views[i].subImage.swapchain = swapchains[i];
		projection_views[i].subImage.imageArrayIndex = 0;
		projection_views[i].subImage.imageRect.offset.x = 0;
		projection_views[i].subImage.imageRect.offset.y = 0;
		projection_views[i].subImage.imageRect.extent.width =
			viewconfig_views[i].recommendedImageRectWidth;
		projection_views[i].subImage.imageRect.extent.height =
			viewconfig_views[i].recommendedImageRectHeight;

		// projection_views[i].{pose, fov} have to be filled every frame in frame loop
	};

	views = new XrView[view_count];
	for (uint32_t i = 0; i < view_count; i++) {
		views[i].type = XR_TYPE_VIEW;
		views[i].next = NULL;
	}

	leftFBO = AftrFrameBufferObject::New(viewconfig_views[0].recommendedImageRectWidth, viewconfig_views[0].recommendedImageRectHeight);
	rightFBO = AftrFrameBufferObject::New(viewconfig_views[1].recommendedImageRectWidth, viewconfig_views[1].recommendedImageRectHeight);

	std::cout << "END ON CREATE RENDERER\n\n\n";
}

void setCameraPose(Camera& cam, XrPosef xrpose)
{
	float scale = 2;
	cam.setPosition(scale * xrpose.position.x, scale * xrpose.position.y, scale * xrpose.position.z);
	
	Aftr::Mat4 temp = cam.getPose();
	
	float qw = xrpose.orientation.w;
	float qx = xrpose.orientation.x;
	float qy = xrpose.orientation.z; 
	float qz = xrpose.orientation.y;

	// , , 

	//temp[0] = 2 * (q0 * q0 + q1 * q1) - 1;
	//temp[1] = 2 * (q1 * q2 - q0 * q3);
	//temp[2] = 2 * (q1 * q3 + q0 * q2);

	//temp[4] = 2 * (q1 * q2 + q0 * q3);
	//temp[5] = 2 * (q0 * q0 + q2 * q2) - 1;
	//temp[6] = 2 * (q2 * q3 - q0 * q1);

	//temp[8] = 2 * (q1 * q3 - q0 * q2);
	//temp[9] = 2 * (q2 * q3 + q0 * q1);
	//temp[10] = 2 * (q0 * q0 + q3 * q3) - 1;

	temp[0] = 1.0f - 2.0f * qy * qy - 2.0f * qz * qz;
	temp[1] = 2.0f * qx * qy - 2.0f * qz * qw;
	temp[2] = 2.0f * qx * qz + 2.0f * qy * qw;

	temp[4] = 2.0f * qx * qy + 2.0f * qz * qw;
	temp[5] = 1.0f - 2.0f * qx * qx - 2.0f * qz * qz;
	temp[6] = 2.0f * qy * qz - 2.0f * qx * qw;

	temp[8] = 2.0f * qx * qz - 2.0f * qy * qw;
	temp[9] = 2.0f * qy * qz + 2.0f * qx * qw;
	temp[10] = 1.0f - 2.0f * qx * qx - 2.0f * qy * qy;


	std::cout << "before: " << cam.getPose() << std::endl;
	std::cout << "after: " << temp << std::endl;

	cam.setPose(temp);
}

void VRRenderer::render(Camera& cam, WorldContainer& wList)
{
	printOpenGLErrors(808080, nullptr, AFTR_FILE_LINE_STR);

	AftrGLRenderer::render(cam, wList);
	//renderStereo(cam, wList);
	
	//float Z = 20.50f;//distance to projection plane
	//Z = this->stereoscopicProjectionPlaneDistance;

	//float A = this->eyeSeparationDistanceInCM;//.1f;//eye separation (in world units (usually meters))

	//float Near = cam.getCameraNearClippingPlaneDistance();
	//float Far = cam.getCameraFarClippingPlaneDistance();

	//float w = 2.0f * Z * tanf(cam.getCameraHorizontalFOVDeg() * Aftr::DEGtoRAD / 2.0f);
	//float h = 2.0f * Z * tanf(cam.getCameraVerticalFOVDeg() * Aftr::DEGtoRAD / 2.0f);

	//float L_l = -(Near * ((w / 2.0f - A / 2.0f) / Z));
	//float L_r = (Near * ((w / 2.0f + A / 2.0f) / Z));
	//float L_b = -(Near * ((h / 2.0f) / Z));
	//float L_t = (Near * ((h / 2.0f) / Z));

	//float R_l = -(Near * ((w / 2.0f + A / 2.0f) / Z));
	//float R_r = (Near * ((w / 2.0f - A / 2.0f) / Z));
	//float R_b = -(Near * ((h / 2.0f) / Z));
	//float R_t = (Near * ((h / 2.0f) / Z));

	//this->setUpStereoCamera(R_l, R_r, R_b, R_t, Near, Far, true, A, cam);
	//this->rightOrTopStereoFBO->bind();
	//this->renderScene(cam, wList);
	//this->rightOrTopStereoFBO->unbind();

	//setUpStereoCamera(L_l, L_r, L_b, L_t, Near, Far, false, A, cam);
	//this->leftOrBottomStereoFBO->bind();
	//this->renderScene(cam, wList);
	//this->leftOrBottomStereoFBO->unbind();

	//Vector eyeSeperationDirection = cam.getNormalDirection().crossProduct(cam.getLookDirection());
	//cam.moveRelative(eyeSeperationDirection * -A);

	//this->renderSplitHorizontally(cam);

	uint32_t view_count = 2;

	// --- Wait for our turn to do head-pose dependent computation and render a frame
	XrFrameState frame_state = { .type = XR_TYPE_FRAME_STATE, .next = NULL };
	XrFrameWaitInfo frame_wait_info = { .type = XR_TYPE_FRAME_WAIT_INFO, .next = NULL };
	XrResult result = xrWaitFrame( xrSession, &frame_wait_info, &frame_state);
	
	if (result != XR_SUCCESS)
	{
		std::cout << "FAILED TO WAIT FRAME" << std::endl;
	}

	// --- Create projection matrices and view matrices for each eye
	XrViewLocateInfo view_locate_info = { .type = XR_TYPE_VIEW_LOCATE_INFO,
										 .next = NULL,
										 .viewConfigurationType =
											 XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO,
										 .displayTime = frame_state.predictedDisplayTime,
										 .space = play_space };

	XrViewState view_state = { .type = XR_TYPE_VIEW_STATE, .next = NULL };
	result = xrLocateViews( xrSession, &view_locate_info, &view_state, view_count, &view_count, views);

	// --- Begin frame
	XrFrameBeginInfo frame_begin_info = { .type = XR_TYPE_FRAME_BEGIN_INFO, .next = NULL };

	result = xrBeginFrame(xrSession, &frame_begin_info);
	
	if (result != XR_SUCCESS)
	{
		std::cout << "FAILED TO BEGIN FRAME" << std::endl;
	}

	// render each eye and fill projection_views with the result
	for (uint32_t i = 0; i < view_count; i++) 
	{

		XrSwapchainImageAcquireInfo acquire_info = { .type = XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO,
													.next = NULL };
		uint32_t acquired_index;
		result = xrAcquireSwapchainImage(swapchains[i], &acquire_info, &acquired_index);
		
		if (result != XR_SUCCESS)
		{
			std::cout << "FAILED TO ACQUIRE SWAPCHAIN IMAGE" << std::endl;
		}

		XrSwapchainImageWaitInfo wait_info = {
			.type = XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO, .next = NULL, .timeout = 1000 };
		result = xrWaitSwapchainImage(swapchains[i], &wait_info);
		
		if (result != XR_SUCCESS)
		{
			std::cout << "FAILED TO WAIT INFO" << std::endl;
		}

		/*uint32_t depth_acquired_index = UINT32_MAX;
		if (depth.supported) {
			XrSwapchainImageAcquireInfo depth_acquire_info = {
				.type = XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO, .next = NULL };
			result = xrAcquireSwapchainImage(depth_swapchains[i], &depth_acquire_info,
				&depth_acquired_index);
			if (!xr_check(instance, result, "failed to acquire swapchain image!"))
				break;

			XrSwapchainImageWaitInfo depth_wait_info = {
				.type = XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO, .next = NULL, .timeout = 1000 };
			result = xrWaitSwapchainImage(depth_swapchains[i], &depth_wait_info);
			if (!xr_check(instance, result, "failed to wait for swapchain image!"))
				break;
		}*/

		projection_views[i].pose = views[i].pose;
		projection_views[i].fov = views[i].fov;

		//GLuint depth_image = depth.supported ? depth_images[i][depth_acquired_index].image : 0;

		int w = viewconfig_views[i].recommendedImageRectWidth;
		int h = viewconfig_views[i].recommendedImageRectHeight;

		// TODO: should not be necessary, but is for SteamVR 1.16.4 (but not 1.15.x)
		/*glXMakeCurrent(graphics_binding_gl.xDisplay, graphics_binding_gl.glxDrawable,
			graphics_binding_gl.glxContext);*/

		/*render_frame(w, h, gl_rendering.shader_program_id, gl_rendering.VAO,
			frame_state.predictedDisplayTime, i, hand_locations, projection_matrix,
			view_matrix, gl_rendering.framebuffers[i][acquired_index],
			images[i][acquired_index].image, depth.supported, depth_image);*/

		//images[i][acquired_index]

		if (i == 0)
		{
			leftFBO->bind();
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, images[i][acquired_index].image, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Set projection matrix
			//std::cout << "x: " << views[i].pose.position.x << " y: " << views[i].pose.position.y << " z: " << views[i].pose.position.z << std::endl;
			setCameraPose(cam, views[i].pose);

			glViewport(0, 0, viewconfig_views[i].recommendedImageRectWidth, viewconfig_views[i].recommendedImageRectHeight);
			wList.renderWorld(cam);
			leftFBO->unbind();
		}
		else
		{
			rightFBO->bind();
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, images[i][acquired_index].image, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glClearColor(.0f, 0.0f, 0.2f, 1.0f);

			// Set projection matrix
			setCameraPose(cam, views[i].pose);

			glViewport(0, 0, viewconfig_views[i].recommendedImageRectWidth, viewconfig_views[i].recommendedImageRectHeight);
			wList.renderWorld(cam);
			rightFBO->unbind();
		}


		XrSwapchainImageReleaseInfo release_info = { .type = XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO,
													.next = NULL };
		result = xrReleaseSwapchainImage(swapchains[i], &release_info);
		
		if (result != XR_SUCCESS)
		{
			std::cout << "FAILED TO RELEASE SWAPCHAIN INFO" << std::endl;
		}

		/*if (depth.supported) {
			XrSwapchainImageReleaseInfo depth_release_info = {
				.type = XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO, .next = NULL };
			result = xrReleaseSwapchainImage(depth_swapchains[i], &depth_release_info);
			if (!xr_check(instance, result, "failed to release swapchain image!"))
				break;
		}*/
	}

	XrCompositionLayerProjection projection_layer = {
		.type = XR_TYPE_COMPOSITION_LAYER_PROJECTION,
		.next = NULL,
		.layerFlags = 0,
		.space = play_space,
		.viewCount = view_count,
		.views = projection_views,
	};

	uint32_t submitted_layer_count = 1;
	const XrCompositionLayerBaseHeader* submitted_layers[1] = {
		(const XrCompositionLayerBaseHeader* const)&projection_layer };

	if ((view_state.viewStateFlags & XR_VIEW_STATE_ORIENTATION_VALID_BIT) == 0) {
		printf("submitting 0 layers because orientation is invalid\n");
		submitted_layer_count = 0;
	}

	if (!frame_state.shouldRender) {
		printf("submitting 0 layers because shouldRender = false\n");
		submitted_layer_count = 0;
	}

	XrFrameEndInfo frameEndInfo = { .type = XR_TYPE_FRAME_END_INFO,
									.next = NULL,
									.displayTime = frame_state.predictedDisplayTime,
									.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE,
									.layerCount = submitted_layer_count,
									.layers = submitted_layers
									};

	result = xrEndFrame(xrSession, &frameEndInfo);
	
	if (result != XR_SUCCESS)
	{
		std::cout << "FAILED TO END FRAME" << std::endl;
	}

	SDL_GL_SwapWindow(ManagerWindowing::getCurrentWindow());
}

