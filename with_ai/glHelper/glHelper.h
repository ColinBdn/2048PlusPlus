#pragma once

#include <thread>
#include <vector>
#include <iostream>
#include <functional>

#include "imgui.h"
#include "implot.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "glfw/glfw3.h"



/**
 * A helper class for managing ImGui/openGl functions and their execution.
 * Provides functionality to start and stop ImGui/openGl operations, add functions for execution,
 * remove functions, and handle closing conditions.
 */
class GlHelper
{
public:
	/**
	 * Initialize openGl and ImGui, must be called just one time at the start.
	*/
	static void start();

	/**
	 * Stops ImGui/openGl operations.
	*/
	static void stop();

	/**
	 * Stops ImGui/openGl operations if all windows are closed.
	*/
	static void stopIfAllWindowsAreClosed();

	/**
	 * Adds a function for execution.
	 *
	 * @tparam Args Variadic template for function arguments.
	 * @param func Pointer to a function (class member or not) with at least one boolean pointer (e.g., p_open*) for the window's open status.
	 *             Additional arguments can be provided as needed.
	 * @param name Unique identifier for the function being added.
	 * @param args Additional arguments to pass to the function.
	 */
	template<typename... Args>
	static void add(void (*func)(bool*, Args...), std::string name, Args&&... args)
	{
		for (Function& f : functions) //check si exist  déjà
		{
			if (f.name == name) return;
		}
		functions.push_back({ std::bind(func, std::placeholders::_1, std::forward<Args>(args)...), true, nullptr, name});
	}
	/**
	* Adds a member function of a class for execution.
	*
	* @tparam Class Type of the class containing the member function.
	* @tparam Args Variadic template for function arguments.
	* @param func Pointer to a member function of the specified class, with at least one boolean pointer (e.g., p_open*) for the window's open status.
	*             Additional arguments can be provided as needed.
	* @param class_ptr Pointer to an instance of the class containing the member function.
	* @param name Unique identifier for the function being added.
	* @param args Additional arguments to pass to the member function.
	*/
	template<typename Class, typename... Args>
	static void add(void (Class::* func)(bool*, Args...), Class* class_ptr, std::string name, Args&&... args)
	{
		for (Function& f : functions) //check si exist  déjà
		{
			if (f.class_ptr == reinterpret_cast<void*>(class_ptr) && f.name==name) return;
		}
		functions.push_back({ std::bind(func, class_ptr, std::placeholders::_1, std::forward<Args>(args)...), true, reinterpret_cast<void*>(class_ptr), name});
	}

	/**
	* Removes a function associated with a specific class pointer.
	*
	* @tparam T Type of the class.
	* @param name Name of the function.
	* @param class_ptr Pointer to the instance of the class.
	*/
	template<typename T>
	static void remove(std::string name, T* class_ptr)
	{
		for (int i=0; i<functions.size();i++)
		{
			if (functions[i].name == name && functions[i].class_ptr == reinterpret_cast<void*>(class_ptr))
			{
				functions.erase(functions.begin() + i);
			}
		}
	}
	static void remove(std::string name)
	{
		for (int i = 0; i < functions.size(); i++)
		{
			if (functions[i].name == name && functions[i].class_ptr == nullptr)
			{
				functions.erase(functions.begin() + i);
			}
		}
	}



public:
	static bool waitForClosing;
	static bool running;
	static bool finished;
	static std::thread mainThread;

	/**
	* Structure to hold function information.
	*/
	struct Function
	{
		std::function<void(bool*)> call;
		bool is_open;
		void* class_ptr;
		std::string name;
	};
	static std::vector<Function> functions;
};

