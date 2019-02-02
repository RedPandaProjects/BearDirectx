using BearBuildTool.Projects;
using System.IO;
using System;
public class bear_directx10 :Project
{
	public bear_directx10(string ProjectPath)
	{
		PCHFile=Path.Combine(ProjectPath,"source","DXPCH.cpp");
		PCHIncludeFile="DXPCH.h";
		AddSourceFiles(Path.Combine(ProjectPath,"source"),true);
		Projects.Private.Add("bear_core");
		Projects.Private.Add("bear_graphics");
		Projects.Private.Add("directx");
		LibrariesStatic.Private.Add("dxgi.lib");
		LibrariesStatic.Private.Add("dxguid.lib");
		LibrariesStatic.Private.Add("d3d11.lib");
		LibrariesStatic.Private.Add("d3dx11.lib");
		Defines.Private.Add("DX10");
	}
} 