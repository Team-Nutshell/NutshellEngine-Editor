#pragma once
#include "ntshengn_system_module_interface.h"
#include "../resources/ntshengn_resources_window.h"
#include <vector>
#include <string>

namespace NtshEngn {

	struct Image;

	class WindowModuleInterface : public SystemModuleInterface {
	public:
		WindowModuleInterface() {}
		WindowModuleInterface(const std::string& name) : SystemModuleInterface(ModuleType::Window, name) {}
		virtual ~WindowModuleInterface() {}

		virtual void init() = 0;
		virtual void update(double dt) = 0;
		virtual void destroy() = 0;

		// Opens a new window and returns a unique identifier
		virtual WindowID openWindow(int width, int height, const std::string& title) = 0;
		// Returns true if the window with identifier windowID is open, else, returns false
		virtual bool isWindowOpen(WindowID windowID) = 0;
		// Flags the window to be closed at the next update
		virtual void closeWindow(WindowID windowID) = 0;

		// Returns the main window identifier
		virtual WindowID getMainWindowID() = 0;
		// Returns the number of opened windows
		virtual uint64_t windowCount() = 0;

		// Changes the size of the window with identifier windowID to width and height
		virtual void setWindowSize(WindowID windowID, int width, int height) = 0;
		// Returns the width of the window with identifier windowID
		virtual int getWindowWidth(WindowID windowID) = 0;
		// Returns the height of the window with identifier windowID
		virtual int getWindowHeight(WindowID windowID) = 0;

		// Changes the position of the window with identifier windowID to x and y
		virtual void setWindowPosition(WindowID windowID, int x, int y) = 0;
		// Returns the horizontal position of the window with identifier windowID
		virtual int getWindowPositionX(WindowID windowID) = 0;
		// Returns the vertical position of the window with identifier windowID
		virtual int getWindowPositionY(WindowID windowID) = 0;

		// If the fullscreen parameter is true, puts the window with identifier windowID in fullscreen, else, puts the window with identifier windowID in windowed
		virtual void setWindowFullscreen(WindowID windowID, bool fullscreen) = 0;
		// Returns true if the window with identifier windowID is in fullscreen mode, else, returns false
		virtual bool isWindowFullscreen(WindowID windowID) = 0;

		// If the borderless parameter is true, puts the window with identifier windowID in borderless, else, adds borders to the window with identifier windowID
		virtual void setWindowBorderless(WindowID windowID, bool borderless) = 0;
		// Returns true if the window with identifier windowID is in borderless mode, else, returns false
		virtual bool isWindowBorderless(WindowID windowID) = 0;

		// If the resizable parameter is true, makes the window with identifier windowID resizable, else, makes the window with identifier windowID not resizable
		virtual void setWindowResizable(WindowID windowID, bool resizable) = 0;
		// Returns true if the window with identifier windowID is resizable, else, returns false
		virtual bool isWindowResizable(WindowID windowID) = 0;

		// Sets the opacity of the window with identifier windowID
		virtual void setWindowOpacity(WindowID windowID, float opacity) = 0;
		// Returns the opacity of the window with identifier windowID
		virtual float getWindowOpacity(WindowID windowID) = 0;

		// Returns the path to the files dropped on the window with identifier windowID
		virtual std::vector<std::string> getWindowDroppedFiles(WindowID windowID) = 0;

		// Polls events of the windows
		virtual void pollEvents() = 0;

		// Sets the title of the window with identifier windowID
		virtual void setWindowTitle(WindowID windowID, const std::string& title) = 0;
		// Returns the title of the window with identifier windowID
		virtual std::string getWindowTitle(WindowID windowID) = 0;
		// Sets the icon of the window with identifier windowID
		virtual void setWindowIcon(WindowID windowID, const Image& image) = 0;

		// Returns the state of the keyboard key. None if the input is neutral, Pressed the first frame it is being pressed, Held from the second frame it is pressed, Released the frame it is being released
		virtual InputState getKeyState(WindowID windowID, InputKeyboardKey key) = 0;
		// Returns the state of the mouse button. None if the input is neutral, Pressed the first frame it is being pressed, Held from the second frame it is pressed, Released the frame it is being released
		virtual InputState getMouseButtonState(WindowID windowID, InputMouseButton mouseButton) = 0;

		// Sets the mouse cursor position
		virtual void setCursorPosition(WindowID windowID, int x, int y) = 0;
		// Returns the mouse cursor horizontal position
		virtual int getCursorPositionX(WindowID windowID) = 0;
		// Returns the mouse cursor vertical position
		virtual int getCursorPositionY(WindowID windowID) = 0;

		// Returns the horizontal mouse scroll offset between the last and current frame
		virtual float getMouseScrollOffsetX(WindowID windowID) = 0;
		// Returns the vertical mouse scroll offset between the last and current frame
		virtual float getMouseScrollOffsetY(WindowID windowID) = 0;

		// Sets the mouse cursor's visibility in the window with identifier windowID
		virtual void setCursorVisibility(WindowID windowID, bool visible) = 0;
		// Returns true if the mouse cursor is visible in the window with identifier windowID, else, returns false
		virtual bool isCursorVisible(WindowID windowID) = 0;

		// Returns the list of connected gamepads
		virtual std::vector<GamepadID> getConnectedGamepads() = 0;

		// Returns the state of the gamepad with identifier gamepadID
		virtual InputState getGamepadButtonState(GamepadID gamepadID, InputGamepadButton button) = 0;
		// Returns the value of the stick's horizontal axis, with -1.0 being left, 0.0 neutral and 1.0 right
		virtual float getGamepadStickAxisX(GamepadID gamepadID, InputGamepadStick stick) = 0;
		// Returns the value of the stick's vertical axis, with -1.0 being up, 0.0 neutral and 1.0 down
		virtual float getGamepadStickAxisY(GamepadID gamepadID, InputGamepadStick stick) = 0;
		// Returns the value of the stick's left trigger, with 0.0 being neutral and 1.0 being fully pressed
		virtual float getGamepadLeftTrigger(GamepadID gamepadID) = 0;
		// Returns the value of the stick's right trigger, with 0.0 being neutral and 1.0 being fully pressed
		virtual float getGamepadRightTrigger(GamepadID gamepadID) = 0;

		// Returns the name of the gamepad with identifier gamepadID
		virtual std::string getGamepadName(GamepadID gamepadID) = 0;

		// Returns the width of the main monitor
		virtual int getMonitorWidth() = 0;
		// Returns the height of the main monitor
		virtual int getMonitorHeight() = 0;
		// Returns the refresh rate of the main monitor
		virtual int getMonitorRefreshRate() = 0;
		// Returns the display scaling ratio of the main monitor
		virtual float getMonitorDisplayScaling() = 0;

		// Returns the native window handle of the window with identifier windowID
		virtual NativeWindowHandle getWindowNativeHandle(WindowID windowID) = 0;
		// Returns the native window additional information of the window with identifier windowID
		virtual NativeWindowAdditionalInformation getWindowNativeAdditionalInformation(WindowID windowID) = 0;
	};

}