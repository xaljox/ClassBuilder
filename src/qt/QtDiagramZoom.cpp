// qt/QtDiagramZoom.cpp -- see header. Last-activated diagram canvas registry.

#include "QtDiagramZoom.h"

#include <QPointer>
#include <QWidget>

namespace {
QPointer<QWidget>  g_canvas;            // weak: nulls itself when the view closes
DiagramZoomApplyFn g_apply = nullptr;
}

void Qt_SetActiveDiagramZoom(QWidget* canvas, DiagramZoomApplyFn apply)
{
    g_canvas = canvas;
    g_apply  = apply;
}

bool Qt_DiagramZoomAvailable()
{
    return g_canvas && g_apply;
}

void Qt_DiagramZoom(int op)
{
    if (g_canvas && g_apply)
        g_apply(g_canvas, op);
}
