
#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <optional>

// Debug �t���O
#ifdef NDEBUG
// Vulkan �́A�������̂��߂ɏ�ɃG���[�`�F�b�N������킯�ł͂Ȃ��B
// �Ƃ����Ă��A�����\������Ȃ��ƃf�o�b�O������Ȃ̂ŁA
// �J�����Ɏg�����̂Ƃ��āA�G���[�`�F�b�N���s���w(Layer)�ɒu����������悤�ɂ��Ă���
// ���́A�G���[�`�F�b�N�p�̃V�X�e�������ނ̂��Avalidation Layer
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

class MyApplication
{
private:
	constexpr static char APP_NAME[] = "Vulkan Application";

	GLFWwindow* window_;
	VkInstance instance_;
	VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT debugMessenger_;// �f�o�b�O���b�Z�[�W��`����I�u�W�F�N�g

public:
	MyApplication() : window_(nullptr) {}
	~MyApplication() {}

	void run()
	{
		// ������
		initializeWindow();
		initializeVulkan();

		// �ʏ폈��
		mainloop();

		// ��Еt��
		finalizeVulkan();
		finalizeWindow();
	}

private:
	// �\���E�B���h�E�̐ݒ�
	void initializeWindow()
	{
		const int WIDTH = 800;
		const int HEIGHT = 600;

		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);// OpenGL �̎�ނ̐ݒ�
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);// ���[�U�[�̓E�B���h�E�T�C�Y��ύX�ł��Ȃ�

		window_ = glfwCreateWindow(WIDTH, HEIGHT, APP_NAME, nullptr, nullptr);
	}

	void finalizeWindow()
	{
		glfwDestroyWindow(window_);
		glfwTerminate();
	}

	// �ʏ�̏���
	void mainloop()
	{
		while (!glfwWindowShouldClose(window_))
		{
			glfwPollEvents();
		}
	}

	// Vulkan�̐ݒ�
	void initializeVulkan()
	{
		createInstance(&instance_);
		initializeDebugMessenger(instance_, debugMessenger_);
		physicalDevice_ = pickPhysicalDevice(instance_);
	}

	void finalizeVulkan()
	{
		finalizeDebugMessenger(instance_, debugMessenger_);
		vkDestroyInstance(instance_, nullptr);
	}

	static void createInstance(VkInstance* dest)
	{
		// �A�v�P�[�V���������߂邽�߂̍\����
		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;			// �\���̂̎��
		appInfo.pApplicationName = APP_NAME;						// �A�v���P�[�V������
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);		// �J���҂����߂�o�[�W�����ԍ�
		appInfo.pEngineName = "My Engine";							// �Q�[���G���W����
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);			// �Q�[���G���W���̃o�[�W����
		appInfo.apiVersion = VK_API_VERSION_1_0;					// �g�p����API�̃o�[�W����

		// �V���������C���X�^���X�̐ݒ�̍\����
		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;	// �\���̂̎��
		createInfo.pApplicationInfo = &appInfo;						// VkApplicationInfo�̏��

		// valkan�̊g���@�\���擾���āA�������f�[�^�ɒǉ�
		std::vector<const char*> extensions = getRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		if (enableValidationLayers) {
			static const std::vector<const char*> validationLayers = {
				"VK_LAYER_KHRONOS_validation"
			};

			// ���؃��C���[�̊m�F
			if (!checkValidationLayerSupport(validationLayers)) {
				throw std::runtime_error("validation layers requested, but not available!");
			}

			// �C���X�^���X�ւ̐ݒ�
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();

			// �f�o�b�O���b�Z���W���[�����̌�Ɉ��������č쐬����
			VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = populateDebugMessengerCreateInfo();
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}

		// �C���X�^���X�̐���
		if (vkCreateInstance(&createInfo, nullptr, dest) != VK_SUCCESS) {
			throw std::runtime_error("failed to create instance!");
		}
	}

	static std::vector<const char*> getRequiredExtensions()
	{
		// �g���̌������o
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (enableValidationLayers) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

#ifdef _DEBUG
		// �L���ȃG�N�X�e���V�����̕\��
		std::cout << "available extensions:" << std::endl;
		for (const auto& extension : extensions) {
			std::cout << "\t" << extension << std::endl;
		}
#endif

		return extensions;
	}

	// ���؃��C���[�ɑΉ����Ă��邩�m�F
	static bool checkValidationLayerSupport(const std::vector<const char*>& validationLayers)
	{
		// ���C���[�̃v���p�e�B���擾
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);				// ���C���[���̎擾
		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());// �v���p�e�B���̂̎擾

		// �S�Ẵ��C���[�����؃��C���[�ɑΉ����Ă��邩�m�F
		for (const char* layerName : validationLayers) {
			if (![](const char* layerName, const auto& availableLayers) {
				// ���C���[�����؃��C���[�̂ǂꂩ�������Ă��邩�m�F
				for (const auto& layerProperties : availableLayers) {
					if (strcmp(layerName, layerProperties.layerName) == 0) { return true; }
				}
				return false;
				}(layerName, availableLayers)) {
				return false;
			}// �ǂ����̃��C���[��validationLayers�̃��C���[�ɑΉ����Ă��Ȃ��̂̓_��
		}

		return true;
	}

	/*** �f�o�C�X�̑I�� ***/
	static VkPhysicalDevice pickPhysicalDevice(const VkInstance& instance)
	{
		// �f�o�C�X���̎擾
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
		if (deviceCount == 0) throw std::runtime_error("failed to find GPUs with Vulkan support!");

		// �f�o�C�X�̎擾
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		// �K�؂ȃf�o�C�X��I�o(�ō����_�̃f�o�C�X���g�p����)
		VkPhysicalDevice best_device = VK_NULL_HANDLE;
		int best_score = 0;
		for (const auto& device : devices) {
			int score = rateDeviceSuitability(device);
			if (best_score < score) {
				best_device = device;
				best_score = score;
			}
		}

		// �g���镨���f�o�C�X���Ȃ���Α���
		if (best_device == VK_NULL_HANDLE) throw std::runtime_error("failed to find a suitable GPU!");

		return best_device;
	}

	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;

		bool isComplete() {
			return graphicsFamily.has_value();// ����́A�o�^����Ă����ok
		}
	};

	static int rateDeviceSuitability(const VkPhysicalDevice device)
	{
		// �f�o�C�X�Ɋւ�������擾
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		int score = 0;

		// �O�t��GPU�Ȃ獂�]��
		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 1000;

		// �ő�e�N�X�`���𑜓x�𐫔\�̕]���l�ɉ�����
		score += deviceProperties.limits.maxImageDimension2D;

		// �e�b�Z���[�V�����V�F�[�_�ɑΉ����Ă��Ȃ��Ɩ��O(�e�b�Z�Z���[�V�������g���ꍇ)
//		if (!deviceFeatures.tessellationShader) return 0;

		// Queue Family�̊m�F
		QueueFamilyIndices indices = findQueueFamilies(device);
		if (!indices.isComplete()) return 0;

#ifdef _DEBUG
		// �f�o�C�X���̕\��
		std::cout << "Physical Device: " << deviceProperties.deviceName
			<< " (score: " << score << ")" << std::endl;
#endif // _DEBUG
		return score;
	}

	static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device)
	{
		// �L���[�t�@�~���[�̐����擾
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
		// �L���[�t�@�~���[���擾
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

#ifdef _DEBUG
		std::cout << std::endl;
#endif // _DEBUG

		int i = 0;
		QueueFamilyIndices indices;
		for (const auto& queueFamily : queueFamilies) {

#ifdef _DEBUG
			std::cout << "queueFamily: " << queueFamily.queueCount << "quque(s)" << std::endl;
#endif // _DEBUG

			// �L���[�t�@�~���[�ɃL���[������A�O���t�B�b�N�X�L���[�Ƃ��Ďg���邩���ׂ�
			if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphicsFamily = i;
			}

			if (indices.isComplete()) {
				break;
			}

			i++;
		}

		return indices;
	}

	/*** debugMessenger �̏��� ***/
	// ������
	static void initializeDebugMessenger(VkInstance& instance, VkDebugUtilsMessengerEXT& debugMessenger)
	{
		if (!enableValidationLayers) return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo = populateDebugMessengerCreateInfo();// �������̍\�z

		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {// ����
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}

	// debugMessenger �̐������̍쐬
	static VkDebugUtilsMessengerCreateInfoEXT populateDebugMessengerCreateInfo()
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |	// �f�f���b�Z�[�W
//			VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |								// ���\�[�X�̍쐬�Ȃǂ̏�񃁃b�Z�[�W(���Ȃ�\�������)
VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |							// �K�������G���[�ł͂Ȃ����A�A�v���P�[�V�����̃o�O�ł���\������������Ɋւ��郁�b�Z�[�W
VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;								// �����ł���A�N���b�V���������N�����\���̂��铮��Ɋւ��郁�b�Z�[�W
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |			// �d�l�܂��̓p�t�H�[�}���X�Ƃ͖��֌W�̃C�x���g������
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |							// �d�l�Ɉᔽ����A�܂��͊ԈႢ�̉\�����������̂�����
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;							// Vulkan�̍œK�łȂ��g�p�̉\��
		createInfo.pfnUserCallback = [](
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData) -> VKAPI_ATTR VkBool32
		{
			std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

			return VK_FALSE;
		};

		return createInfo;
	}

	// debugMessenger �̐���
	static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		// vkCreateDebugUtilsMessengerEXT�ɑΉ����Ă��邩�m�F���Ď��s
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func == nullptr) return VK_ERROR_EXTENSION_NOT_PRESENT;
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}

	// �Еt��
	static void finalizeDebugMessenger(VkInstance& instance, VkDebugUtilsMessengerEXT& debugMessenger)
	{
		if (!enableValidationLayers) return;

		// vkCreateDebugUtilsMessengerEXT�ɑΉ����Ă��邩�m�F���Ď��s
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func == nullptr) return;
		func(instance, debugMessenger, nullptr);
	}
};