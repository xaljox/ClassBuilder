// MainFrm.cpp : the MFC-free CMainFrame facade statics. See MainFrm.h.

#include "StdAfx.h"

#include "MainFrm.h"

#include <cstdio>

CMainFrame::StatusTextFn CMainFrame::s_statusTextFn = nullptr;
int CMainFrame::s_loadInProgress = 0;

namespace {
void setPane(int pane, const char* fmt, int v)
{
    if (!CMainFrame::s_statusTextFn)
        return;
    char buf[64];
    sprintf_s(buf, fmt, v);
    CMainFrame::s_statusTextFn(pane, buf);
}
void clearPane(int pane)
{
    if (CMainFrame::s_statusTextFn)
        CMainFrame::s_statusTextFn(pane, "");
}
}

void CMainFrame::SetCoordinateX(int x)   { setPane(2, "X=%d", x); clearPane(3); }
void CMainFrame::SetCoordinateY(int y)   { clearPane(2); setPane(3, "Y=%d", y); }
void CMainFrame::SetCoordinate(int x, int y)
{
    setPane(2, "X=%d", x);
    setPane(3, "Y=%d", y);
}
void CMainFrame::ResetCoordinate()       { clearPane(2); clearPane(3); }
void CMainFrame::SetWidth(int width)     { setPane(0, "Width=%d", width); }
void CMainFrame::ResetWidth()            { clearPane(0); }
void CMainFrame::SetHeight(int height)   { setPane(1, "Height=%d", height); }
void CMainFrame::ResetHeight()           { clearPane(1); }
