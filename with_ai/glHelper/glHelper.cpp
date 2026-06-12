#include <chrono>
#include "glHelper.h"


bool GlHelper::running = true;
bool GlHelper::finished = false;
bool GlHelper::waitForClosing = false;
std::thread GlHelper::mainThread;
std::vector<GlHelper::Function> GlHelper::functions;


static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

void GlHelper::start()
{
	mainThread = std::thread([&] {
		glfwSetErrorCallback(glfw_error_callback);
		if (!glfwInit())
			exit(1);

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

		GLFWwindow* window = glfwCreateWindow(10, 50, "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);

		if (window == nullptr)
			exit(1);
		glfwMakeContextCurrent(window);
		glfwSwapInterval(1);

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImPlot::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;



		ImGui::StyleColorsDark();
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 330");
		while (!glfwWindowShouldClose(window) && running)
		{
			//--------------------------------------------------------
			//--------------------------------------------------------
			glfwPollEvents();
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
			//--------------------------------------------------------
			//--------------------------------------------------------

			if (waitForClosing)
			{
				running = false;
			}
			for (Function& func : functions)
			{
				if (func.is_open)
				{
					func.call(&func.is_open);
					if (waitForClosing)
					{
						running = true;
					}
				}
			}
			//--------------------------------------------------------
			//--------------------------------------------------------
			ImGui::Render();
			int display_w, display_h;
			glfwGetFramebufferSize(window, &display_w, &display_h);
			glViewport(0, 0, display_w, display_h);
			glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				GLFWwindow* backup_current_context = glfwGetCurrentContext();
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
				glfwMakeContextCurrent(backup_current_context);
			}
			glfwSwapBuffers(window);
			//--------------------------------------------------------
			//--------------------------------------------------------

		}
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImPlot::DestroyContext();
		ImGui::DestroyContext();

		glfwDestroyWindow(window);
		glfwTerminate();

		finished = true;
	});
}


void GlHelper::stopIfAllWindowsAreClosed()
{
	waitForClosing = true;
	while (!finished)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	mainThread.join();
}

void GlHelper::stop()
{
	running = false;
	mainThread.join();
}

