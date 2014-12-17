#include "virtualHID.hpp"


VirtualHID::VirtualHID()
{
	display = XOpenDisplay (NULL);
}

VirtualHID::~VirtualHID()
{
	XCloseDisplay (display);
}

void VirtualHID::TakeScreenshot(cv::Mat& image)
{
	Screen* screen = XDefaultScreenOfDisplay(display);
	cv::Size resolution = cv::Size(XWidthOfScreen(screen), XHeightOfScreen(screen));

	XImage* x_image = XGetImage (display, XDefaultRootWindow(display), 0, 0, resolution.width, resolution.height, AllPlanes, XYPixmap);
	unsigned long pixel;
	cv::Vec3b color;

	image = cv::Mat::zeros(resolution, CV_8UC3);
	for (int i = 0; i < resolution.height; ++i)
	{
		for (int j = 0; j < resolution.width; ++j)
		{
			pixel = XGetPixel(x_image, j, i);

			color[0] = (pixel & 0xFF);
			color[1] = (pixel & 0xFF00) >> 8;
			color[2] = (pixel & 0xFF0000) >> 16;

			image.at<cv::Vec3b>(i, j) = color;
		}
	}
	XFree (x_image);
}

void VirtualHID::ClickKey(KeySym keysym)
{
	KeyCode keycode = XKeysymToKeycode(display , keysym);
	XTestFakeKeyEvent(display , keycode , True  , CurrentTime); // key press event
    XTestFakeKeyEvent(display , keycode , False , CurrentTime); // key release event
    XFlush(display);
}

void VirtualHID::MouseUp(unsigned int button)
{
	XTestFakeButtonEvent(display, button, false, CurrentTime);
};

void VirtualHID::MouseClick(unsigned int button)
{
	XTestFakeButtonEvent(display, button, true, CurrentTime);
	XTestFakeButtonEvent(display, button, false, CurrentTime);
};

void VirtualHID::MoveMouse(int x, int y)
{
	XTestFakeMotionEvent(display, -1, x, y, CurrentTime);
	XSync(display, 0);
};

void VirtualHID::MoveMouseRelative(int x, int y)
{
	XTestFakeRelativeMotionEvent(display, x, y, CurrentTime);
};
