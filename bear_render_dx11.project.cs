using BearBuildTool.Projects;
using System.IO;
using System;
public class bear_render_dx11 :Project
{
	public bear_render_dx11(string ProjectPath)
	{
        PCHFile = Path.Combine(ProjectPath, "source", "DX12PCH.cpp");
        PCHIncludeFile = "DX12PCH.h";
        AddSourceFiles(Path.Combine(ProjectPath, "source"), true);
        Projects.Private.Add("bear_graphics");
        Projects.Private.Add("dxc");
      	Include.Private.Add(Path.Combine(ProjectPath,"include"));
        LibrariesStatic.Private.Add("d3d12.lib");
        LibrariesStatic.Private.Add("dxgi.lib");
        LibrariesStatic.Private.Add("d3dcompiler.lib");
        Defines.Private.Add("DX11");
    }
} 