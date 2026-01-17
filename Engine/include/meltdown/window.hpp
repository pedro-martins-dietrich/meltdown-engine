#pragma once

#include <memory>
#include <vector>

#include <meltdown/macros.hpp>
#include <meltdown/math.hpp>
#include <meltdown/structs.hpp>

namespace mtd
{
	class WindowHandler;

	/*
	* @brief Class for handling window usage.
	*/
	class MELTDOWN_API Window
	{
		public:
			Window(const WindowInfo& info);
			~Window();

			Window(const Window&) = delete;
			Window& operator=(const Window&) = delete;

			/*
			* @brief Gets the current window dimensions.
			*
			* @return The width and height of the window, as a 2D vector of unsigned integers.
			*/
			UIntVec2 getDimensions() const;
			/*
			* @brief Gets the current window aspect ratio (width / height).
			*
			* @return The aspect ratio as a float value.
			*/
			float getAspectRatio() const;

			friend class Engine;

		private:
			std::unique_ptr<WindowHandler> windowHandler;
	};
}
