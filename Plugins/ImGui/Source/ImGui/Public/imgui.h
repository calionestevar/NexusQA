#pragma once
#include "CoreMinimal.h"

namespace ImGui
{
    struct ImVec4 { float x, y, z, w; ImVec4(float a=0,float b=0,float c=0,float d=0):x(a),y(b),z(c),w(d){} };
    using ImGuiWindowFlags = int;
    static const ImGuiWindowFlags ImGuiWindowFlags_AlwaysAutoResize = 1;

    inline bool Begin(const TCHAR* Name, bool* p_open = nullptr, ImGuiWindowFlags Flags = 0) { return true; }
    inline void End() {}
    inline void TextColored(const ImVec4&, const TCHAR* Format, ...) {}
    inline void Text(const TCHAR* Format, ...) {}
    inline void Separator() {}
    inline void TextUnformatted(const TCHAR* Text) {}
}
