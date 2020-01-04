using BearBuildTool.Projects;
using System.IO;
using System;
public class bear_render_dx12 :Project
{
	public bear_render_dx12(string ProjectPath)
	{
        PCHFile = Path.Combine(ProjectPath, "source", "DX12PCH.cpp");
        PCHIncludeFile = "DX12PCH.h";
        AddSourceFiles(Path.Combine(ProjectPath, "source"), true);
        Projects.Private.Add("bear_graphics");
        if (BearBuildTool.Config.Global.DevVersion)
        {

            LibrariesStatic.Private.Add("d3dcompiler.lib");
        }
        Include.Private.Add(Path.Combine(ProjectPath,"include"));
        LibrariesStatic.Private.Add("d3d12.lib");
        LibrariesStatic.Private.Add("dxgi.lib");
        Defines.Private.Add("DX12");
    }
} 