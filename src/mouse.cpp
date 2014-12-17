#include "mouse.hpp"

#include <opencv2/highgui/highgui.hpp>

void onMouse(int event, int x, int y, int flags, void* param)
{
    MouseData* m = (MouseData*)param;
    m->event = event;
    m->x = x;
    m->y = y;

    switch (event)
    {
        case CV_EVENT_LBUTTONUP:
            m->l_up = true;
            break;
        case CV_EVENT_LBUTTONDOWN:
            m->l_down = true;
            break;
        case CV_EVENT_RBUTTONUP:
            m->r_up = true;
            break;
        case CV_EVENT_RBUTTONDOWN:
            m->r_down = true;
            break;
        case CV_EVENT_MOUSEMOVE:
            m->move = true;
            break;
        default:
            break;
    }
}