#ifndef VIRTUAL_HID_HEADER
#define VIRTUAL_HID_HEADER

#include <opencv2/opencv.hpp>

#include <X11/Xutil.h>
#include <X11/extensions/XTest.h>

class VirtualHID
{
	public:
		VirtualHID();
		~VirtualHID();

		//Monitor
		void TakeScreenshot(cv::Mat& image);
		
		//Keyboard
		void ClickKey(KeySym keysym);
		
		//Mouse
		void MouseDown(unsigned int button);
		void MouseUp(unsigned int button);
		void MouseClick(unsigned int button);
		void MoveMouse(int x, int y);
		void MoveMouseRelative(int x, int y);

	private:
		Display* display;
};

#endif
