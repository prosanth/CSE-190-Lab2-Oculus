/************************************************************************************

Authors     :   Bradley Austin Davis <bdavis@saintandreas.org>
Copyright   :   Copyright Brad Davis. All Rights reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

************************************************************************************/


#include <iostream>
#include <memory>
#include <exception>
#include <algorithm>

#include <Windows.h>

#include "Window.h"
#include "Cube.h"
#include "Screen.h"


#define __STDC_FORMAT_MACROS 1

#define FAIL(X) throw std::runtime_error(X)

int renderMode = 0; //Button A (stereo-0, mono-1, left-2, right-3)
int whatToDisplay = 0; //Button X (cube-0, panorama-1, both-2)
int trackingMode = 0; //Button B (both-0, orientation-1, position-2, none-3)

int skyboxMode = 0;
int failure = -1;

bool lThumbPressed = false;
bool rThumbPressed = false;

//Oculus Touch input
bool aPressed = false;
bool bPressed = false;
bool xPressed = false;
bool yPressed = false;
bool rThumbDec = false;
bool rThumbInc = false;
bool lThumbMoveLeft = false;
bool lThumbMoveRight = false;
bool lThumbMoveForw = false;
bool lThumbMoveBack = false;
bool rThumbMoveUp = false;
bool rThumbMoveDown = false;
bool rTriggerPressed = false;



bool headViewpoint = true;

glm::vec3 rightHandPosition = glm::vec3(0.0f);

glm::mat4 leftPos;
glm::mat4 rightPos;

glm::mat4 leftPose;
glm::mat4 rightPose;

glm::vec3 eyeOffsetDelta;
glm::vec3 eyeOffsetDefault;

glm::mat4 originalHeadPose, newHeadPose = glm::mat4(1.0f);
glm::vec3 posVector = glm::vec3(0.0f, 0.0f, 0.0f);

GLuint FBfront, FBleft, FBbottom;
GLuint texFront, texLeft, texBottom;
GLuint newFB = 0;
GLuint texture;
GLuint depth;

glm::vec2 screenSize;

bool testSkybox = false;

//std::vector<Screen*> caveScreens;


///////////////////////////////////////////////////////////////////////////////
//
// GLM is a C++ math library meant to mirror the syntax of GLSL 
//

#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

// Import the most commonly used types into the default namespace
using glm::ivec3;
using glm::ivec2;
using glm::uvec2;
using glm::mat3;
using glm::mat4;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::quat;

///////////////////////////////////////////////////////////////////////////////
//
// GLEW gives cross platform access to OpenGL 3.x+ functionality.  
//

#include <GL/glew.h>

bool checkFramebufferStatus(GLenum target = GL_FRAMEBUFFER) {
	GLuint status = glCheckFramebufferStatus(target);
	switch (status) {
	case GL_FRAMEBUFFER_COMPLETE:
		return true;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
		std::cerr << "framebuffer incomplete attachment" << std::endl;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
		std::cerr << "framebuffer missing attachment" << std::endl;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
		std::cerr << "framebuffer incomplete draw buffer" << std::endl;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
		std::cerr << "framebuffer incomplete read buffer" << std::endl;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
		std::cerr << "framebuffer incomplete multisample" << std::endl;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
		std::cerr << "framebuffer incomplete layer targets" << std::endl;
		break;

	case GL_FRAMEBUFFER_UNSUPPORTED:
		std::cerr << "framebuffer unsupported internal format or image" << std::endl;
		break;

	default:
		std::cerr << "other framebuffer error" << std::endl;
		break;
	}

	return false;
}

bool checkGlError() {
	GLenum error = glGetError();
	if (!error) {
		return false;
	}
	else {
		switch (error) {
		case GL_INVALID_ENUM:
			std::cerr << ": An unacceptable value is specified for an enumerated argument.The offending command is ignored and has no other side effect than to set the error flag.";
			break;
		case GL_INVALID_VALUE:
			std::cerr << ": A numeric argument is out of range.The offending command is ignored and has no other side effect than to set the error flag";
			break;
		case GL_INVALID_OPERATION:
			std::cerr << ": The specified operation is not allowed in the current state.The offending command is ignored and has no other side effect than to set the error flag..";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			std::cerr << ": The framebuffer object is not complete.The offending command is ignored and has no other side effect than to set the error flag.";
			break;
		case GL_OUT_OF_MEMORY:
			std::cerr << ": There is not enough memory left to execute the command.The state of the GL is undefined, except for the state of the error flags, after this error is recorded.";
			break;
		case GL_STACK_UNDERFLOW:
			std::cerr << ": An attempt has been made to perform an operation that would cause an internal stack to underflow.";
			break;
		case GL_STACK_OVERFLOW:
			std::cerr << ": An attempt has been made to perform an operation that would cause an internal stack to overflow.";
			break;
		}
		return true;
	}
}

void glDebugCallbackHandler(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *msg, GLvoid* data) {
	OutputDebugStringA(msg);
	std::cout << "debug call: " << msg << std::endl;
}

//////////////////////////////////////////////////////////////////////
//
// GLFW provides cross platform window creation
//

#include <GLFW/glfw3.h>

namespace glfw {
	inline GLFWwindow * createWindow(const uvec2 & size, const ivec2 & position = ivec2(INT_MIN)) {
		GLFWwindow * window = glfwCreateWindow(size.x, size.y, "glfw", nullptr, nullptr);
		screenSize = glm::vec2(size.x, size.y);
		
		char buff[100];
		sprintf_s(buff, "screen size: %f %f \n", screenSize.x, screenSize.y);
		OutputDebugStringA(buff);

		if (!window) {
			FAIL("Unable to create rendering window");
		}
		if ((position.x > INT_MIN) && (position.y > INT_MIN)) {
			glfwSetWindowPos(window, position.x, position.y);
		}
		return window;
	}
}

// A class to encapsulate using GLFW to handle input and render a scene
class GlfwApp {

protected:
	uvec2 windowSize;
	ivec2 windowPosition;
	GLFWwindow * window{ nullptr };
	unsigned int frame{ 0 };

public:
	GlfwApp() {
		// Initialize the GLFW system for creating and positioning windows
		if (!glfwInit()) {
			FAIL("Failed to initialize GLFW");
		}
		glfwSetErrorCallback(ErrorCallback);
	}

	virtual ~GlfwApp() {
		if (nullptr != window) {
			glfwDestroyWindow(window);
		}
		glfwTerminate();
	}

	virtual int run() {
		preCreate();

		window = createRenderingTarget(windowSize, windowPosition);

		if (!window) {
			std::cout << "Unable to create OpenGL window" << std::endl;
			return -1;
		}

		postCreate();

		initGl();

		while (!glfwWindowShouldClose(window)) {
			++frame;
			glfwPollEvents();
			update();
			draw();

			finishFrame();
		}

		shutdownGl();

		return 0;
	}


protected:
	virtual GLFWwindow * createRenderingTarget(uvec2 & size, ivec2 & pos) = 0;

	virtual void draw() = 0;

	void preCreate() {
		glfwWindowHint(GLFW_DEPTH_BITS, 16);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
	}


	void postCreate() {
		glfwSetWindowUserPointer(window, this);
		glfwSetKeyCallback(window, KeyCallback);
		glfwSetMouseButtonCallback(window, MouseButtonCallback);
		glfwMakeContextCurrent(window);

		// Initialize the OpenGL bindings
		// For some reason we have to set this experminetal flag to properly
		// init GLEW if we use a core context.
		glewExperimental = GL_TRUE;
		if (0 != glewInit()) {
			FAIL("Failed to initialize GLEW");
		}
		glGetError();

		if (GLEW_KHR_debug) {
			GLint v;
			glGetIntegerv(GL_CONTEXT_FLAGS, &v);
			if (v & GL_CONTEXT_FLAG_DEBUG_BIT) {
				//glDebugMessageCallback(glDebugCallbackHandler, this);
			}
		}
	}

	virtual void initGl() {
	}

	virtual void shutdownGl() {
	}

	virtual void finishFrame() {
		glfwSwapBuffers(window);
	}

	virtual void destroyWindow() {
		glfwSetKeyCallback(window, nullptr);
		glfwSetMouseButtonCallback(window, nullptr);
		glfwDestroyWindow(window);
	}

	virtual void onKey(int key, int scancode, int action, int mods) {
		if (GLFW_PRESS != action) {
			return;
		}

		switch (key) {
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, 1);
			return;
		}
	}

	virtual void update() {}

	virtual void onMouseButton(int button, int action, int mods) {}

protected:
	virtual void viewport(const ivec2 & pos, const uvec2 & size) {
		glViewport(pos.x, pos.y, size.x, size.y);
	}

private:

	static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		GlfwApp * instance = (GlfwApp *)glfwGetWindowUserPointer(window);
		instance->onKey(key, scancode, action, mods);
	}

	static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
		GlfwApp * instance = (GlfwApp *)glfwGetWindowUserPointer(window);
		instance->onMouseButton(button, action, mods);
	}

	static void ErrorCallback(int error, const char* description) {
		FAIL(description);
	}
};

//////////////////////////////////////////////////////////////////////
//
// The Oculus VR C API provides access to information about the HMD
//

#include <OVR_CAPI.h>
#include <OVR_CAPI_GL.h>

namespace ovr {

	// Convenience method for looping over each eye with a lambda
	template <typename Function>
	inline void for_each_eye(Function function) {
		for (ovrEyeType eye = ovrEyeType::ovrEye_Left;
			eye < ovrEyeType::ovrEye_Count;
			eye = static_cast<ovrEyeType>(eye + 1)) {
			function(eye);
		}
	}

	inline mat4 toGlm(const ovrMatrix4f & om) {
		return glm::transpose(glm::make_mat4(&om.M[0][0]));
	}

	inline mat4 toGlm(const ovrFovPort & fovport, float nearPlane = 0.01f, float farPlane = 10000.0f) {
		return toGlm(ovrMatrix4f_Projection(fovport, nearPlane, farPlane, true));
	}

	inline vec3 toGlm(const ovrVector3f & ov) {
		return glm::make_vec3(&ov.x);
	}

	inline vec2 toGlm(const ovrVector2f & ov) {
		return glm::make_vec2(&ov.x);
	}

	inline uvec2 toGlm(const ovrSizei & ov) {
		return uvec2(ov.w, ov.h);
	}

	inline quat toGlm(const ovrQuatf & oq) {
		return glm::make_quat(&oq.x);
	}

	inline mat4 toGlm(const ovrPosef & op) {
		mat4 orientation = glm::mat4_cast(toGlm(op.Orientation));
		mat4 translation = glm::translate(mat4(), ovr::toGlm(op.Position));
		return translation * orientation;
	}

	inline ovrMatrix4f fromGlm(const mat4 & m) {
		ovrMatrix4f result;
		mat4 transposed(glm::transpose(m));
		memcpy(result.M, &(transposed[0][0]), sizeof(float) * 16);
		return result;
	}

	inline ovrVector3f fromGlm(const vec3 & v) {
		ovrVector3f result;
		result.x = v.x;
		result.y = v.y;
		result.z = v.z;
		return result;
	}

	inline ovrVector2f fromGlm(const vec2 & v) {
		ovrVector2f result;
		result.x = v.x;
		result.y = v.y;
		return result;
	}

	inline ovrSizei fromGlm(const uvec2 & v) {
		ovrSizei result;
		result.w = v.x;
		result.h = v.y;
		return result;
	}

	inline ovrQuatf fromGlm(const quat & q) {
		ovrQuatf result;
		result.x = q.x;
		result.y = q.y;
		result.z = q.z;
		result.w = q.w;
		return result;
	}
}

class RiftManagerApp {
protected:
	ovrSession _session;
	ovrHmdDesc _hmdDesc;
	ovrGraphicsLuid _luid;

public:
	RiftManagerApp() {
		if (!OVR_SUCCESS(ovr_Create(&_session, &_luid))) {
			FAIL("Unable to create HMD session");
		}

		_hmdDesc = ovr_GetHmdDesc(_session);
	}

	~RiftManagerApp() {
		ovr_Destroy(_session);
		_session = nullptr;
	}
};

class RiftApp : public GlfwApp, public RiftManagerApp {
public:

private:
	GLuint _fbo{ 0 };
	GLuint _depthBuffer{ 0 };
	ovrTextureSwapChain _eyeTexture;

	GLuint _mirrorFbo{ 0 };
	ovrMirrorTexture _mirrorTexture;

	ovrEyeRenderDesc _eyeRenderDescs[2];

	mat4 _eyeProjections[2];

	ovrLayerEyeFov _sceneLayer;
	ovrViewScaleDesc _viewScaleDesc;

	uvec2 _renderTargetSize;
	uvec2 _mirrorSize;

	

public:

	RiftApp() {
		using namespace ovr;
		_viewScaleDesc.HmdSpaceToWorldScaleInMeters = 1.0f;

		memset(&_sceneLayer, 0, sizeof(ovrLayerEyeFov));
		_sceneLayer.Header.Type = ovrLayerType_EyeFov;
		_sceneLayer.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;

		ovr::for_each_eye([&](ovrEyeType eye) {
			
				ovrEyeRenderDesc& erd = _eyeRenderDescs[eye] = ovr_GetRenderDesc(_session, eye, _hmdDesc.DefaultEyeFov[eye]);
				ovrMatrix4f ovrPerspectiveProjection =
					ovrMatrix4f_Projection(erd.Fov, 0.01f, 1000.0f, ovrProjection_ClipRangeOpenGL);
				_eyeProjections[eye] = ovr::toGlm(ovrPerspectiveProjection);
				eyeOffsetDefault = ovr::toGlm(erd.HmdToEyeOffset);
				_viewScaleDesc.HmdToEyeOffset[eye] = ovr::fromGlm(ovr::toGlm(erd.HmdToEyeOffset) + eyeOffsetDelta);				

				ovrFovPort & fov = _sceneLayer.Fov[eye] = _eyeRenderDescs[eye].Fov;
				auto eyeSize = ovr_GetFovTextureSize(_session, eye, fov, 1.0f);
				_sceneLayer.Viewport[eye].Size = eyeSize;
				_sceneLayer.Viewport[eye].Pos = { (int)_renderTargetSize.x, 0 };

				_renderTargetSize.y = std::max(_renderTargetSize.y, (uint32_t)eyeSize.h);
				_renderTargetSize.x += eyeSize.w;
		});
		
		// Make the on screen window 1/4 the resolution of the render target
		_mirrorSize = _renderTargetSize;
		_mirrorSize /= 4;

		if (trackingMode != 3) {
			char buff[100];
			sprintf_s(buff, "Change proj\n");
			OutputDebugStringA(buff);

			leftPos = _eyeProjections[ovrEyeType::ovrEye_Left];
			rightPos = _eyeProjections[ovrEyeType::ovrEye_Right];
		}
	}

protected:
	GLFWwindow * createRenderingTarget(uvec2 & outSize, ivec2 & outPosition) override {
		return glfw::createWindow(_mirrorSize);
	}

	void initGl() override {
		GlfwApp::initGl();

		// Disable the v-sync for buffer swap
		glfwSwapInterval(0);

		ovrTextureSwapChainDesc desc = {};
		desc.Type = ovrTexture_2D;
		desc.ArraySize = 1;
		desc.Width = _renderTargetSize.x;
		desc.Height = _renderTargetSize.y;
		desc.MipLevels = 1;
		desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
		desc.SampleCount = 1;
		desc.StaticImage = ovrFalse;
		ovrResult result = ovr_CreateTextureSwapChainGL(_session, &desc, &_eyeTexture);
		_sceneLayer.ColorTexture[0] = _eyeTexture;
		if (!OVR_SUCCESS(result)) {
			FAIL("Failed to create swap textures");
		}

		int length = 0;
		result = ovr_GetTextureSwapChainLength(_session, _eyeTexture, &length);
		if (!OVR_SUCCESS(result) || !length) {
			FAIL("Unable to count swap chain textures");
		}
		for (int i = 0; i < length; ++i) {
			GLuint chainTexId;
			ovr_GetTextureSwapChainBufferGL(_session, _eyeTexture, i, &chainTexId);
			glBindTexture(GL_TEXTURE_2D, chainTexId);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}
		glBindTexture(GL_TEXTURE_2D, 0);

		// Set up the framebuffer object
		glGenFramebuffers(1, &_fbo);
		glGenRenderbuffers(1, &_depthBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
		glBindRenderbuffer(GL_RENDERBUFFER, _depthBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, _renderTargetSize.x, _renderTargetSize.y);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

		ovrMirrorTextureDesc mirrorDesc;
		memset(&mirrorDesc, 0, sizeof(mirrorDesc));
		mirrorDesc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
		mirrorDesc.Width = _mirrorSize.x;
		mirrorDesc.Height = _mirrorSize.y;
		if (!OVR_SUCCESS(ovr_CreateMirrorTextureGL(_session, &mirrorDesc, &_mirrorTexture))) {
			FAIL("Could not create mirror texture");
		}
		glGenFramebuffers(1, &_mirrorFbo);

		glGenFramebuffers(1, &FBfront);
		glGenFramebuffers(1, &FBleft);
		glGenFramebuffers(1, &FBbottom);

		glGenTextures(1, &texFront);
		glGenTextures(1, &texLeft);
		glGenTextures(1, &texBottom);
	}

	void onKey(int key, int scancode, int action, int mods) override {
		if (GLFW_PRESS == action) switch (key) {
		case GLFW_KEY_R:
			ovr_RecenterTrackingOrigin(_session);
			return;
		}

		GlfwApp::onKey(key, scancode, action, mods);
	}

	void draw() final override {
		ovrPosef eyePoses[2];
		
		//Screen 

		ovr_GetEyePoses(_session, frame, true, _viewScaleDesc.HmdToEyeOffset, eyePoses, &_sceneLayer.SensorSampleTime);

		int curIndex;
		ovr_GetTextureSwapChainCurrentIndex(_session, _eyeTexture, &curIndex);
		GLuint curTexId;
		ovr_GetTextureSwapChainBufferGL(_session, _eyeTexture, curIndex, &curTexId);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, curTexId, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ovr::for_each_eye([&](ovrEyeType eye) {
			ovrEyeRenderDesc& erd = _eyeRenderDescs[eye] = ovr_GetRenderDesc(_session, eye, _hmdDesc.DefaultEyeFov[eye]);
			_viewScaleDesc.HmdToEyeOffset[eye] = erd.HmdToEyeOffset;

			GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
			glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
				return;
			}

			//front Framebuffer
			glBindFramebuffer(GL_FRAMEBUFFER, FBfront);

			glBindTexture(GL_TEXTURE_2D, texFront);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1000, 1000, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texFront, 0);

			glBindFramebuffer(GL_FRAMEBUFFER, FBfront);
			glViewport(0, 0, 1000, 1000); // Render on the whole framebuffer, complete from the lower left corner to the upper right
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			//draw to texture (need to change projections)
			glm::vec3 pa, pb, pc, pe;
			glm::vec3 vr, vu, vn;
			glm::vec3 va, vb, vc;
			if(rTriggerPressed) pe = rightHandPosition;
			else pe = ovr::toGlm(eyePoses[eye].Position);

			pa = glm::vec3(-1.2f, -1.2f, -1.2f);
			pb = glm::vec3(1.2f, -1.2f, -1.2f);
			pc = glm::vec3(-1.2f, 1.2f, -1.2f);
			
			//translate by these amounts
			/*left -sqrt(pow(1.2,2) / 2)
			right sqrt(pow(1.2, 2) / 2)
		    floor -1.2f, sqrt(pow(1.2, 2) / 2)*/

			//pa = glm::rotate(glm::mat4(1.0f), glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(pa, 1.0f);
			//pb = glm::rotate(glm::mat4(1.0f), glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(pb, 1.0f);
			//pc = glm::rotate(glm::mat4(1.0f), glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(pc, 1.0f);
			pa = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(pa, 1.0f);
			pb = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(pb, 1.0f);
			pc = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(pc, 1.0f);

			vr = glm::normalize(pb - pa);// / glm::length(pb - pa);
			vu = glm::normalize(pc - pa);// / glm::length(pc - pa);
			vn = glm::normalize(glm::cross(vr, vu));// / glm::length(glm::cross(vr, vu));

			va = pa - pe;
			vb = pb - pe;
			vc = pc - pe;

			float d = -(glm::dot(vn,va));
			float l, r, t, b;
			float n = 0.00000000000000001;

			l = glm::dot(vr, va)*n / d;
			r = glm::dot(vr, vb)*n / d;
			b = glm::dot(vu, va)*n / d;
			t = glm::dot(vu, vc)*n / d;

			glm::mat4 proj = glm::frustum(l, r, b, t, n, d);	
			glm::mat4 m = glm::mat4(glm::vec4(vr, 0.0f), glm::vec4(vu, 0.0f), glm::vec4(vn, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
			glm::mat4 mT = glm::transpose(m);

			glm::mat4 identity = mat4(1.0f);
			glm::mat4 T = glm::mat4(identity[0], identity[1], identity[2], glm::vec4(-pe, 1.0f));

			glm::mat4 pPrime = proj*mT*T;
			
			if (failure == 0 && eye == ovrEye_Left) {
			}
			else if (failure == 1 && eye == ovrEye_Right) {
			}
			else {
				if (eye == ovrEyeType::ovrEye_Left) {
					renderScene(pPrime, leftPose, eye, true);
				}
				else {
					renderScene(pPrime, rightPose, eye, true);
				}
			}

			//renderScene(_eyeProjections[eye], ovr::toGlm(eyePoses[eye]), eye, true);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			//left Framebuffer
			glBindFramebuffer(GL_FRAMEBUFFER, FBleft);

			glBindTexture(GL_TEXTURE_2D, texLeft);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1000, 1000, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texLeft, 0);

			glBindFramebuffer(GL_FRAMEBUFFER, FBleft);
			glViewport(0, 0, 1000, 1000); // Render on the whole framebuffer, complete from the lower left corner to the upper right
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			//draw to texture (need to change projections)
			pa = glm::vec3(-1.2f, -1.2f, -1.2f);
			pb = glm::vec3(1.2f, -1.2f, -1.2f);
			pc = glm::vec3(-1.2f, 1.2f, -1.2f);

			/*pa = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(pa, 1.0f);
			pb = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(pb, 1.0f);
			pc = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(pc, 1.0f);*/
			pa = glm::rotate(glm::mat4(1.0f), glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(pa, 1.0f);
			pb = glm::rotate(glm::mat4(1.0f), glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(pb, 1.0f);
			pc = glm::rotate(glm::mat4(1.0f), glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(pc, 1.0f);

			vr = glm::normalize(pb - pa);// / glm::length(pb - pa);
			vu = glm::normalize(pc - pa);// / glm::length(pc - pa);
			vn = glm::normalize(glm::cross(vr, vu));// / glm::length(glm::cross(vr, vu));

			va = pa - pe;
			vb = pb - pe;
			vc = pc - pe;

			char buff[100];
			sprintf_s(buff, "pa %f %f %f\n", pa.x, pa.y, pa.z);
			OutputDebugStringA(buff);

			sprintf_s(buff, "pb %f %f %f\n", pb.x, pb.y, pb.z);
			OutputDebugStringA(buff);

			sprintf_s(buff, "pc %f %f %f\n", pc.x, pc.y, pc.z);
			OutputDebugStringA(buff);

			sprintf_s(buff, "pe %f %f %f\n", pe.x, pe.y, pe.z);
			OutputDebugStringA(buff);

			sprintf_s(buff, "vr: %f %f %f\n", vr.x, vr.y, vr.z);
			OutputDebugStringA(buff);

			sprintf_s(buff, "vu: %f %f %f\n", vu.x, vu.y, vu.z);
			OutputDebugStringA(buff);

			sprintf_s(buff, "vn: %f %f %f\n", vn.x, vn.y, vn.z);
			OutputDebugStringA(buff);

			sprintf_s(buff, "va: %f %f %f\n", va.x, va.y, va.z);
			OutputDebugStringA(buff);

			sprintf_s(buff, "vb: %f %f %f\n", vb.x, vb.y, vb.z);
			OutputDebugStringA(buff);

			sprintf_s(buff, "vc: %f %f %f\n", vc.x, vc.y, vc.z);
			OutputDebugStringA(buff);

			d = -1 * (glm::dot(vn, va));		

			l = glm::dot(vr, va)*n / d;
			r = glm::dot(vr, vb)*n / d;
			b = glm::dot(vu, va)*n / d;
			t = glm::dot(vu, vc)*n / d;

			proj = glm::frustum(l, r, b, t, n, d);
			m = glm::mat4(glm::vec4(vr, 0.0f), glm::vec4(vu, 0.0f), glm::vec4(vn, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
			mT = glm::transpose(m);
			
			T = glm::mat4(identity[0], identity[1], identity[2], glm::vec4(-pe, 1.0f));

			pPrime = proj*mT*T;

			if (failure == 2 && eye == ovrEye_Left) {
			}
			else if (failure == 3 && eye == ovrEye_Right) {
			}
			else {
				if (eye == ovrEyeType::ovrEye_Left) {
					renderScene(pPrime, leftPose, eye, true);
				}
				else {
					renderScene(pPrime, rightPose, eye, true);
				}
			}
			//renderScene(_eyeProjections[eye], ovr::toGlm(eyePoses[eye]), eye, true);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			//bottom framebuffer
			glBindFramebuffer(GL_FRAMEBUFFER, FBbottom);

			glBindTexture(GL_TEXTURE_2D, texBottom);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1000, 1000, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texBottom, 0);

			glBindFramebuffer(GL_FRAMEBUFFER, FBbottom);
			glViewport(0, 0, 1000, 1000); // Render on the whole framebuffer, complete from the lower left corner to the upper right
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			//draw to texture (need to change projections)
			pa = glm::vec3(-1.2f, -1.2f, -1.2f);
			pb = glm::vec3(1.2f, -1.2f, -1.2f);
			pc = glm::vec3(-1.2f, 1.2f, -1.2f);

			//pa = glm::rotate(glm::mat4(1.0f), glm::radians(-45.0f - 90.0f), glm::vec3(0.0f, 0.0f, 1.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::vec4(pa, 1.0f);
			//pb = glm::rotate(glm::mat4(1.0f), glm::radians(-45.0f - 90.0f), glm::vec3(0.0f, 0.0f, 1.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::vec4(pb, 1.0f);
			//pc = glm::rotate(glm::mat4(1.0f), glm::radians(-45.0f - 90.0f), glm::vec3(0.0f, 0.0f, 1.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::vec4(pc, 1.0f);
						
			pa = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::vec4(pa, 1.0f);
			pb = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::vec4(pb, 1.0f);
			pc = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::vec4(pc, 1.0f);

			pa = glm::rotate(glm::mat4(1.0f), glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(pa, 1.0f);
			pb = glm::rotate(glm::mat4(1.0f), glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(pb, 1.0f);
			pc = glm::rotate(glm::mat4(1.0f), glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(pc, 1.0f);

			/*char buff[100];
			sprintf_s(buff, "pa %f %f %f\n", pa.x, pa.y, pa.z);
			OutputDebugStringA(buff);

			sprintf_s(buff, "pb %f %f %f\n", pb.x, pb.y, pb.z);
			OutputDebugStringA(buff);

			sprintf_s(buff, "pc %f %f %f\n", pc.x, pc.y, pc.z);
			OutputDebugStringA(buff);*/


			vr = glm::normalize(pb - pa);// / glm::length(pb - pa);
			vu = glm::normalize(pc - pa);// / glm::length(pc - pa);
			vn = glm::normalize(glm::cross(vr, vu));// / glm::length(glm::cross(vr, vu));

			va = pa - pe;
			vb = pb - pe;
			vc = pc - pe;

			d = -1 * (glm::dot(vn, va));

			l = glm::dot(vr, va)*n / d;
			r = glm::dot(vr, vb)*n / d;
			b = glm::dot(vu, va)*n / d;
			t = glm::dot(vu, vc)*n / d;

			proj = glm::frustum(l, r, b, t, n, d);
			m = glm::mat4(glm::vec4(vr, 0.0f), glm::vec4(vu, 0.0f), glm::vec4(vn, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
			mT = glm::transpose(m);

			T = glm::mat4(identity[0], identity[1], identity[2], glm::vec4(-pe, 1.0f));

			pPrime = proj*mT*T;

			if (failure == 4 && eye == ovrEye_Left) {
			}
			else if (failure == 5 && eye == ovrEye_Right) {
			}
			else {
				if (eye == ovrEyeType::ovrEye_Left) {
					renderScene(pPrime, leftPose, eye, true);
				}
				else {
					renderScene(pPrime, rightPose, eye, true);
				}
			}

			//renderScene(_eyeProjections[eye], ovr::toGlm(eyePoses[eye]), eye, true);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			//Render buffer
			GLuint rboId;
			glGenRenderbuffers(1, &rboId);
			glBindRenderbuffer(GL_RENDERBUFFER, rboId);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1000, 1000);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboId);

			//Go back to Oculus Framebuffer
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, curTexId, 0);

			//STEREO 
			const auto& vp = _sceneLayer.Viewport[eye];
			glViewport(vp.Pos.x, vp.Pos.y, vp.Size.w, vp.Size.h);
			_sceneLayer.RenderPose[eye] = eyePoses[eye];
			
			renderScene(_eyeProjections[eye], ovr::toGlm(eyePoses[eye]), eye, false);
			
		});

		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		ovr_CommitTextureSwapChain(_session, _eyeTexture);
		ovrLayerHeader* headerList = &_sceneLayer.Header;
		ovr_SubmitFrame(_session, frame, &_viewScaleDesc, &headerList, 1);

		GLuint mirrorTextureId;
		ovr_GetMirrorTextureBufferGL(_session, _mirrorTexture, &mirrorTextureId);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, _mirrorFbo);
		glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mirrorTextureId, 0);
		glBlitFramebuffer(0, 0, _mirrorSize.x, _mirrorSize.y, 0, _mirrorSize.y, _mirrorSize.x, 0, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

		//save last left and right eye poses
		if (trackingMode != 1) {
			leftPose = ovr::toGlm(eyePoses[ovrEye_Left]);
			rightPose = ovr::toGlm(eyePoses[ovrEye_Right]);
		}
		
	}

	virtual void renderScene(const glm::mat4 & projection, const glm::mat4 & headPose, ovrEyeType eye, bool renderTexture) = 0;
};

//////////////////////////////////////////////////////////////////////
//
// The remainder of this code is specific to the scene we want to 
// render.  I use oglplus to render an array of cubes, but your 
// application would perform whatever rendering you want
//


//////////////////////////////////////////////////////////////////////
//
// OGLplus is a set of wrapper classes for giving OpenGL a more object
// oriented interface
//
#define OGLPLUS_USE_GLCOREARB_H 0
#define OGLPLUS_USE_GLEW 1
#define OGLPLUS_USE_BOOST_CONFIG 0
#define OGLPLUS_NO_SITE_CONFIG 1
#define OGLPLUS_LOW_PROFILE 1

#pragma warning( disable : 4068 4244 4267 4065)
#include <oglplus/config/basic.hpp>
#include <oglplus/config/gl.hpp>
#include <oglplus/all.hpp>
#include <oglplus/interop/glm.hpp>
#include <oglplus/bound/texture.hpp>
#include <oglplus/bound/framebuffer.hpp>
#include <oglplus/bound/renderbuffer.hpp>
#include <oglplus/bound/buffer.hpp>
#include <oglplus/shapes/cube.hpp>
#include <oglplus/shapes/wrapper.hpp>
#pragma warning( default : 4068 4244 4267 4065)



namespace Attribute {
	enum {
		Position = 0,
		TexCoord0 = 1,
		Normal = 2,
		Color = 3,
		TexCoord1 = 4,
		InstanceTransform = 5,
	};
}

static const char * VERTEX_SHADER = R"SHADER(
#version 410 core

uniform mat4 ProjectionMatrix = mat4(1);
uniform mat4 CameraMatrix = mat4(1);
uniform mat4 Ah = mat4(1);

layout(location = 0) in vec4 Position;
layout(location = 2) in vec3 Normal;
layout(location = 5) in mat4 InstanceTransform;

out vec3 vertNormal;

void main(void) {
   mat4 ViewXfm = CameraMatrix * Ah;
   //mat4 ViewXfm = CameraMatrix;
   vertNormal = Normal;
   gl_Position = ProjectionMatrix * ViewXfm * Position;
}
)SHADER";

static const char * FRAGMENT_SHADER = R"SHADER(
#version 410 core

in vec3 vertNormal;
out vec4 fragColor;

void main(void) {
    vec3 color = vertNormal;
    if (!all(equal(color, abs(color)))) {
        color = vec3(1.0) - abs(color);
    }
    fragColor = vec4(color, 1.0);
}
)SHADER";

// a class for encapsulating building and rendering an RGB cube
struct ColorCubeScene {

	// Program
	oglplus::shapes::ShapeWrapper cube;
	oglplus::Program prog;
	oglplus::VertexArray vao;
	GLuint instanceCount;
	oglplus::Buffer instances;

	GLuint shaderProgram, texShader, skyboxShader;
	Cube * cube2;

	Cube * rightSkybox;
	Cube * leftSkybox;
	Cube * test;

	Screen * front;
	Screen * left;
	Screen * bottom;

	const unsigned int GRID_SIZE{ 5 };

public:
	ColorCubeScene() : cube({ "Position", "Normal" }, oglplus::shapes::Cube()) {
		using namespace oglplus;

		// Load the shader program
		shaderProgram = LoadShaders("./shader.vert", "./shader.frag");
		skyboxShader = LoadShaders("./skyboxshader.vert", "./skyboxshader.frag");
		texShader = LoadShaders("./textureShader.vert", "textureShader.frag");
		
		cube2 = new Cube(false, false, false);

		rightSkybox = new Cube(true, false, false);
		leftSkybox = new Cube(true, true, false);

		test = new Cube(true, false, true);

		front = new Screen(0);
		left = new Screen(1);
		bottom = new Screen(2);
	}

	void render(const mat4 & projection, const mat4 & modelview, ovrEyeType eye, bool renderTexture) {
		if (renderTexture) {
			glUseProgram(texShader);
			//Change size
			if (rThumbInc) cube2->changeSize(true, false, false);
			else if (rThumbDec) cube2->changeSize(false, true, false);

			if(rThumbPressed) cube2->changeSize(false, false, true);
			
			
			//Move cube
			if (lThumbMoveLeft)	cube2->move(1, 0, 0, false);
			else if (lThumbMoveRight) cube2->move(2, 0, 0, false);
			else if (lThumbMoveBack) cube2->move(0, 2, 0, false);
			else if (lThumbMoveForw) cube2->move(0, 1, 0, false);
			else if (rThumbMoveDown) cube2->move(0, 0, 2, false);
			else if (rThumbMoveUp) cube2->move(0, 0, 1, false);

			if (lThumbPressed) cube2->move(0, 0, 0, true);

			if (eye == ovrEyeType::ovrEye_Left) {
				leftSkybox->draw(texShader, modelview, projection);
			}
			else {
				rightSkybox->draw(texShader, modelview, projection);
			}

			cube2->draw(texShader, modelview, projection);
		}
		else {
			glUseProgram(skyboxShader);
			if (skyboxMode == 1) {
				test->draw(skyboxShader, modelview, projection);
			}

			if (aPressed == true) {
				front->debugOn = true;
				left->debugOn = true;
			}
			else {
				front->debugOn = false;
			}

			glm::vec3 eyePosition;
			if(!rTriggerPressed) eyePosition = glm::vec3(modelview[3]);
			else eyePosition = rightHandPosition;

			glUseProgram(shaderProgram);
			if (eye == ovrEyeType::ovrEye_Left) {
				front->draw(shaderProgram, modelview, projection, texFront, false, aPressed, eyePosition);
				left->draw(shaderProgram, modelview, projection, texLeft, false, aPressed, eyePosition);
				bottom->draw(shaderProgram, modelview, projection, texBottom, false, aPressed, eyePosition);
			}
			else {
				front->draw(shaderProgram, modelview, projection, texFront, true, aPressed, eyePosition);
				left->draw(shaderProgram, modelview, projection, texLeft, true, aPressed, eyePosition);
				bottom->draw(shaderProgram, modelview, projection, texBottom, true, aPressed, eyePosition);
			}

			char buff[100];
			if (eye == ovrEye_Left) {
				sprintf_s(buff, "l \n");
				//OutputDebugStringA(buff);
			}
			else {
				sprintf_s(buff, "r \n");
				//OutputDebugStringA(buff);
			}
		}
		/*if (whatToDisplay == 1 || whatToDisplay == 2) {
			if (testSkybox) {
				test->draw(shaderProgram, modelview, projection);
			}
			else {
				if (renderMode != 1) {	
					if (eye == ovrEyeType::ovrEye_Left) {
						leftSkybox->draw(shaderProgram, modelview, projection);
					}
					else {
						rightSkybox->draw(shaderProgram, modelview, projection);
					}
				}
				else {
					leftSkybox->draw(shaderProgram, modelview, projection);
				}
			}
		}

		if (whatToDisplay == 0 || whatToDisplay == 2) {
			if(lThumbInc) cube2->changeSize(true, false, false);
			else if (lThumbDec) cube2->changeSize(false, true, false);

			if (lThumbPressed) {
				cube2->changeSize(false, false, true);

				char buff[100];
				sprintf_s(buff, "l thumb pressed \n");
				OutputDebugStringA(buff);
			}
			
			cube2->draw(shaderProgram, modelview, projection);
		}*/
	}
};


// An example application that renders a simple cube
class ExampleApp : public RiftApp {
	std::shared_ptr<ColorCubeScene> cubeScene;

public:
	ExampleApp() { }

	GLuint lineShader;

protected:
	void initGl() override {
		RiftApp::initGl();
		glEnable(GL_DEPTH_TEST);
		ovr_RecenterTrackingOrigin(_session);
		cubeScene = std::shared_ptr<ColorCubeScene>(new ColorCubeScene());
	}

	void shutdownGl() override {
		cubeScene.reset();
	}


	void renderScene(const glm::mat4 & projection, const glm::mat4 & headPose, ovrEyeType eye, bool renderTexture) override {
		//OCULUS TOUCH 

		// determine whether left hand trigger pressed
		ovrInputState inputState;
		bool leftHandTriggerPressed = false;
		bool rightHandTriggerPressed = false;

		if (OVR_SUCCESS(ovr_GetInputState(_session, ovrControllerType_Touch, &inputState))) {

			/*if (inputState.IndexTrigger[ovrHand_Left] > 0.5f) {
				leftHandTriggerPressed = true;
			}
			if (inputState.IndexTrigger[ovrHand_Right] > 0.5f) {
				rightHandTriggerPressed = true;
			}*/

			//CHANGING CUBE SIZE (Right stick, left-right)
			if (inputState.Thumbstick[ovrHand_Right].x < -0.8f) {
				//decrease cube size
				rThumbDec = true;
				char buff[100];
				sprintf_s(buff, "r thumb dec \n");
				OutputDebugStringA(buff);
			}
			else if (inputState.Thumbstick[ovrHand_Right].x > 0.8f) {
				//increase cube size
				rThumbInc = true;
				char buff[100];
				sprintf_s(buff, "r thumb inc \n");
				OutputDebugStringA(buff);
			}			
			//MOVING CUBE (Left stick, forward-back, left-right, Right stick, up-down)
			else if (inputState.Thumbstick[ovrHand_Left].x < -0.8f) {
				//move cube left
				lThumbMoveLeft = true;
				char buff[100];
				sprintf_s(buff, "l thumb left \n");
				OutputDebugStringA(buff);
			}
			else if (inputState.Thumbstick[ovrHand_Left].x > 0.8f) {
				//move cube right
				lThumbMoveRight = true;
				char buff[100];
				sprintf_s(buff, "l thumb right \n");
				OutputDebugStringA(buff);
			}
			else if (inputState.Thumbstick[ovrHand_Left].y < -0.8f) {
				//move cube back
				lThumbMoveBack = true;
				char buff[100];
				sprintf_s(buff, "l thumb down \n");
				OutputDebugStringA(buff);
			}
			else if (inputState.Thumbstick[ovrHand_Left].y > 0.8f) {
				//move cube forward
				lThumbMoveForw = true;
				char buff[100];
				sprintf_s(buff, "l thumb up \n");
				OutputDebugStringA(buff);
			}
			else if (inputState.Thumbstick[ovrHand_Right].y < -0.8f) {
				//move cube down
				rThumbMoveDown = true;
				char buff[100];
				sprintf_s(buff, "r thumb down \n");
				OutputDebugStringA(buff);
			}
			else if (inputState.Thumbstick[ovrHand_Right].y > 0.8f) {
				//move cube up
				rThumbMoveUp = true;
				char buff[100];
				sprintf_s(buff, "r thumb up \n");
				OutputDebugStringA(buff);
			}
			else {
				rThumbDec = false;
				rThumbInc = false;
				lThumbMoveLeft = false;
				lThumbMoveRight = false;
				lThumbMoveForw = false;
				lThumbMoveBack = false;
				rThumbMoveUp = false;
				rThumbMoveDown = false;
			}

			if (inputState.IndexTrigger[ovrHand_Right] > 0.5f) {
				rTriggerPressed = true;
				char buff[100];
				sprintf_s(buff, "r index pressed \n");
				OutputDebugStringA(buff);
			}
			else {
				rTriggerPressed = false;
			}


			/*if (inputState.Thumbstick[ovrHand_Right].x < -0.8f) {
				//decrease cube size
				if(eyeOffsetDelta.x >= -5*eyeOffsetDefault.x)	eyeOffsetDelta.x = eyeOffsetDelta.x - 0.002;

				char buff[100];
				//sprintf_s(buff, "Offset: %f\n", eyeOffsetDelta.x);
				//OutputDebugStringA(buff);
			}
			else if (inputState.Thumbstick[ovrHand_Right].x > 0.8f) {
				//increase cube size
				if (eyeOffsetDelta.x <= 0.84 - eyeOffsetDefault.x) eyeOffsetDelta.x = eyeOffsetDelta.x + 0.002;

				char buff[100];
				//sprintf_s(buff, "Offset: %f\n", eyeOffsetDelta.x);
				//OutputDebugStringA(buff);
			}*/
			

			if (inputState.Buttons & ovrButton_A) {
				aPressed = true;				
			}
			else if (inputState.Buttons & ovrButton_B) {
				if (bPressed == false) {
					bPressed = true;

					trackingMode = (trackingMode + 1) % 2;

					char buff[100];
					sprintf_s(buff, "Tracking mode: %d\n", trackingMode);
					OutputDebugStringA(buff);
				}
			} 
			else if (inputState.Buttons & ovrButton_X) {
				if (xPressed == false) {
					xPressed = true;

					skyboxMode = (skyboxMode + 1) % 2;
				}
			}
			else if (inputState.Buttons & ovrButton_Y) {
				if (yPressed == false) {
					yPressed = true;

					if (failure != -1) {
						failure = -1;
					}
					else {
						failure = (rand() % 6);
					}

					char buff[100];
					sprintf_s(buff, "y pressed: %d\n", failure);
					OutputDebugStringA(buff);
				}
			}
			else if (inputState.Buttons & ovrButton_LThumb) {
				if (lThumbPressed == false) {

					lThumbPressed = true;

					char buff[100];
					sprintf_s(buff, "l thumb pressed \n");
					OutputDebugStringA(buff);

				}
			}
			else if (inputState.Buttons & ovrButton_RThumb) {
				if (rThumbPressed == false) {

					rThumbPressed = true;
					eyeOffsetDelta.x = 0;

					char buff[100];
					sprintf_s(buff, "r thumb pressed \n");
					OutputDebugStringA(buff);

				}
			}
			else {
				aPressed = false;
				xPressed = false; 
				bPressed = false;
				yPressed = false;
				lThumbPressed = false;
				rThumbPressed = false;
			}
		}

		double displayMidpointSeconds = ovr_GetPredictedDisplayTime(_session, 0);
		ovrTrackingState trackState = ovr_GetTrackingState(_session, displayMidpointSeconds, ovrTrue);

		ovrPosef leftHandPose = trackState.HandPoses[ovrHand_Left].ThePose;
		glm::vec3 leftHandPosition = glm::vec3(leftHandPose.Position.x, leftHandPose.Position.y, leftHandPose.Position.z);
		glm::quat leftHandOrient = glm::quat(leftHandPose.Orientation.x, leftHandPose.Orientation.y, leftHandPose.Orientation.z, leftHandPose.Orientation.w);

		ovrPosef rightHandPose = trackState.HandPoses[ovrHand_Right].ThePose;
		rightHandPosition = glm::vec3(rightHandPose.Position.x, rightHandPose.Position.y, rightHandPose.Position.z);
		glm::quat rightHandOrient = glm::quat(rightHandPose.Orientation.x, rightHandPose.Orientation.y, rightHandPose.Orientation.z, rightHandPose.Orientation.w);

		if (!rTriggerPressed) {
			cubeScene->render(projection, glm::inverse(headPose), eye, renderTexture);
		}
		else {
			if (renderTexture) { //rendering cube & skybox
				cubeScene->render(projection, glm::inverse(ovr::toGlm(rightHandPose)), eye, renderTexture);
			}
			else {
				cubeScene->render(projection, glm::inverse(headPose), eye, renderTexture);
			}
		}
	}
};

// Execute our example class
int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	int result = -1;
	try {
		if (!OVR_SUCCESS(ovr_Initialize(nullptr))) {
			FAIL("Failed to initialize the Oculus SDK");
		}
		result = ExampleApp().run();
	}
	catch (std::exception & error) {
		OutputDebugStringA(error.what());
		std::cerr << error.what() << std::endl;
	}
	ovr_Shutdown();
	return result;
}