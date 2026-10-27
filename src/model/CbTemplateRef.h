// model/CbTemplateRef.h -- derive a template class's REFERENCE ("<T, N>") from
// its DECLARATION ("template<class T, int N = 4>"). Qt-free; shared by the Class
// and Extern Class dialogs and the set_class_template command, so all three
// derive it the same way.
#pragma once

#include <string>

// "template<class T, int N>"            -> "<T, N>"
// "template<UnsignedBits T>"            -> "<T>"   (C++20 constrained parameter)
// "template<class T, const int* P>"     -> "<T, P>"
// "template<typename... Ts>"            -> "<Ts...>"
// Defaults are dropped (BaseClass::StripTemplateDeclaration); each parameter's
// reference is its name, the last identifier. Not a template declaration -> "".
std::string Cb_DeriveTemplateReference(const std::string& declaration);
