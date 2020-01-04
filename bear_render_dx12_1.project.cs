using BearBuildTool.Projects;
using System.IO;
using System;
public class bear_render_dx12_1 :Project
{
	public bear_render_dx12_1(string ProjectPath)
	{
        PCHFile = Path.Combine(ProjectPath, "source", "DX12PCH.cpp");
        PCHIncludeFile = "DX12PCH.h";
        AddSourceFiles(Path.Combine(ProjectPath, "source"), true);
        Projects.Private.Add("bear_graphics");
        if (BearBuildTool.Config.Global.DevVersion)
        {
            if (BearBuildTool.Config.Global.Platform == BearBuildTool.Config.Platform.Win64)
            {
                Defines.Private.Add("RTX_SHADER_COMPILER");
                Projects.Private.Add("dxc");
            }

            LibrariesStatic.Private.Add("d3dcompiler.lib");
        }
        Include.Private.Add(Path.Combine(ProjectPath,"include"));
        LibrariesStatic.Private.Add("d3d12.lib");
        LibrariesStatic.Private.Add("dxgi.lib");
        Defines.Private.Add("DX12_1");
        Defines.Private.Add("RTX");
    }
} 