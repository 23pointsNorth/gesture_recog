#ifndef MOUSE_CPP
#define MOUSE_CPP

void onMouse(int event, int x, int y, int flags, void* param);

struct MouseData
{
	bool l_up;
	bool l_down;
	bool r_up;
	bool r_down;
	bool move;
	int x;
	int y;
	int event;

	MouseData() : 
		l_up(false), l_down(false), 
		r_up(false), r_down(false), 
		move(false), x(0), y(0), event(0)
	{};
};

#endif