# MapBuilder  

**MapBuilder** is a flexible command-line utility designed to automate the **map compilation pipeline** for Source Engine projects.  
It replaces the need to manually run tools like `vbsp`, `vvis`, and `vrad` by allowing you to define **compile presets** and execute them in sequence with a single command.  

Instead of hardcoding build steps, MapBuilder reads from a **scriptable configuration file** (`mapbuilder_settings.txt`) where you can:  
- Define multiple build **presets** (`Fast`, `Standard`, `Final`, etc.).  
- Configure which tools should run (`vbsp`, `vvis`, `vrad`, `vbspinfo`, `resourcecopy`, cubemap builders, or custom tools).  
- Pass flexible **build parameters** with placeholders like `%source`, `%filename`, `%gamedir`, `%mapbsp`, and `%mapdir`.  
- Overwrite default tool paths with `ToolName` to swap in custom compilers or external executables.  

This approach makes **MapBuilder** especially useful for:  
- Modders who want to integrate **custom tools** directly into the compile chain.  
- Teams or individuals who want to include **automated map builds in larger workflows** (e.g., continuous integration, automated testing, or packaging pipelines).  
- Anyone seeking a **Source 2–style resourcecompile workflow** within Source 1.  

By streamlining repetitive compile tasks, MapBuilder helps keep your workflow **fast, consistent, and customizable**.  
 
<img width="1242" height="455" alt="image" src="https://github.com/user-attachments/assets/9f0f732a-5fde-4678-96ee-e774e32b9b4a" />

---

## 📦 Installation  

To install **MapBuilder**, follow these steps:  

1. Start with a clean copy of the **Source SDK 2013** source code (works both for SP & MP).  
2. Drag and drop the contents of this repository into your SDK source directory.  
3. Overwrite any existing files when prompted.  
4. Regenerate the Visual Studio solution files (e.g., by running `createallprojects.bat`).  
5. Open the solution in Visual Studio and compile it. The `mapbuilder.exe` executable will be built and ready to use.  

---

## ✨ Features  

- 🔧 **Compile Presets** – Define multiple presets (`Fast`, `Standard`, `Final`) in `mapbuilder_system.txt` for different workflows.  
- 📜 **Scriptable Configuration** – Add or reorder tools, adjust parameters, and integrate custom steps without touching the source code.  
- 🔄 **Custom Builders** – Run external programs or override default tools (`vbsp`, `vvis`, `vrad`) via the `ToolName` key.  
- 🧩 **Dynamic Parameters** – Use placeholders like `%source`, `%filename`, `%gamedir`, `%mapbsp`, and `%mapdir` inside build parameters.  
- 🚦 **Error Handling** – Optionally continue building even if one step fails (`-ignoreerrors`).  
- 🗑 **Automatic Cleanup** – Removes leftover temporary files after a successful build.  
---

# MapBuilder Script System  

The **MapBuilder script system** is configured inside your game `scripts/tools/mapbuilder_system.txt`.  
It defines **compile presets** (`Fast`, `Standard`, `Final`, etc.) and the order in which each tool runs.  

Every builder command is wrapped inside a **KeyValue block**, and execution order follows the order they appear.  

---

## 📂 File Structure  

```txt
MapBuilderSystem
{
    // Default preset if none is specified at runtime
    DefaultMapBuilderSettings "Standard"

    // All presets are declared here
    CompilePresets
    {
        "Fast"
        {
            RunWorldBuilder { ... }
            RunVisibilityBuilder { ... }
            RunLightBuilder { ... }
            RunTransferFile { ... }
        }

        "Standard"
        {
            ...
        }

        "Final"
        {
            ...
        }
    }
}
```  

---

## 🛠 Supported Builders  

Each builder corresponds to a compile tool. By default, these tools are assumed to exist in the **`bin` directory** (the same location as `mapbuilder.exe`).  
You can override the executable with the **`ToolName`** KeyValue.  

- **`RunWorldBuilder`** → Runs **`vbsp.exe`**  
  - Default: `vbsp.exe`  
  - Example:  
    ```txt
    RunWorldBuilder
    {
        BuildParams "-game %gamedir %source"
    }
    ```  

- **`RunVisibilityBuilder`** → Runs **`vvis.exe`**  
  - Default: `vvis.exe`
  - Example:  
    ```txt
    RunVisibilityBuilder
    {
        BuildParams "-game %gamedir %source"
    }
    ```  

- **`RunLightBuilder`** → Runs **`vrad.exe`**  
  - Default: `vrad.exe`
    - Example:  
    ```txt
    RunLightBuilder
    {
        BuildParams "-game %gamedir %source"
    }
    ```  

- **`RunMapInfo`** → Runs **`vbspinfo.exe`**  
  - Default: `vbspinfo.exe`
    - Example:  
    ```txt
    RunMapInfo
    {
        BuildParams "-game %gamedir %source"
    }
    ```  

- **`RunTransferFile`** → Runs **`resourcecopy.exe`**  
  - Default: `resourcecopy.exe`
    - Example:  
    ```txt
    RunTransferFile
    {
       BuillParams "%mapbsp %gamedir/maps"
    }
    ```  

- **`RunCubemapBuilder`** → Runs the **game executable** (e.g., `hl2.exe`, `tf.exe`) to build cubemaps.  
  - This uses the **game installation base path of steam** (`%gamedir`).  
  - Example:  
    ```txt
    RunCubemapBuilder
    {
        ToolName "tf.exe"
        BuildParams "-buildcubemaps map %filename"
    }
    ```  

- **Custom processes** → You can define any additional builder by giving it a name (e.g., `RunCustomBuilder`).
- Supported KeyValues:
- `BinDir <boolean>`: If set to one this will search the tool in the same folder as `mapbuilder.exe` (In this case `game/bin/win64`)
- `BaseDir <boolean>`: If set to one this will search the name of the tool in the base path of your game. For example: `"C:\SteamLibrary\steamapps\common\Team Fortress 2\tf"`
- `ExternalPath <string>`: If provided with a path it will search the tool in that path. For example: `"C:\tooling\custom-game"`
- `ToolName <string>`: Name of the executable to be called. 
  - Example:  
    ```txt
    RunCustomBuilder
    {
		BinDir "1"
        ToolName "mytool.exe"
        BuildParams "%source %mapdir"
    }
    ```  

---

## 🔑 BuildParams Placeholders  

You can use placeholders to avoid hardcoding paths:  

- **`%filename`** → Map name without extension.  
  - Example: `test` from `C:\maps\test.vmf`.  

- **`%source`** → Full path to the `.vmf` map file.  
  - Example: `C:\maps\test.vmf`.  

- **`%gamedir`** → Path to the **gameinfo.txt** of the current mod/game.  
  - Example: `C:\Games\Team Fortress 2\tf`.  

- **`%mapdir`** → Directory where the `.bsp` will be copied.  
  - Example: `C:\Games\Team Fortress 2\tf\maps`.

- **`%mapbsp`** → Directory where the `.bsp` is.  
  - Example: `C:\Games\Team Fortress 2\tf\maps\test.bsp`.

Example expansion:  
```txt
RunLightBuilder
{
    BuildParams "-final -game %gamedir %source"
}
```  
Expands into:  
```sh
vrad.exe -final -game "C:\Games\Team Fortress 2\tf" "C:\maps\test.vmf"
```  

---

## 📜 Execution Order  

The **order of KeyValue imports matters**.  
Tools will run in the same order they are declared.  

For example:  
```txt
"Fast"
{
    RunWorldBuilder
    {
        BuildParams "-game %gamedir %source"
    }

    RunVisibilityBuilder
    {
        BuildParams "-fast -game %gamedir %source"
    }
}
```  

This will:  
1. Run **vbsp.exe** first.  
2. Then run **vvis.exe**.  

If reversed, MapBuilder will attempt to run **vvis.exe before vbsp.exe**, which will fail.  

---

## ✅ Example Preset  

You can also set the default preset, which will be run when none is specified in the command line:

```
DefaultMapBuilderSettings "TypeHereTheNameOfYourPreset"
```

If you need to run a preset different from the default, type in the console:

```
-preset <yourpresetname>
```

For example:

```
-preset "MyCustomFinalPreset"
```
```txt
"MyCustomFinalPreset"
{
    RunWorldBuilder
    {
        BuildParams "-game %gamedir %source"
    }

    RunVisibilityBuilder
    {
        BuildParams "-game %gamedir %source"
    }

    RunLightBuilder
    {
        BuildParams "-final -game %gamedir %source"
    }

    RunMapInfo
    {
        BuildParams "%source"
    }

    RunTransferFile
    {
        BuildParams "%mapbsp %mapdir"
    }

    RunCubemapBuilder
    {
        ToolName "tf.exe"
        BuildParams "-buildcubemaps map %filename"
    }
}
```  
This runs:  
1. `vbsp.exe`  
2. `vvis.exe`  
3. `vrad.exe -final`  
4. `vbspinfo.exe`  
5. `resourcecopy.exe`  
6. `tf.exe -buildcubemaps map test`  
---    
## ✅ Example Preset of a full `scripts/tools/mapbuilder_system.txt` file
```txt
MapBuilderSystem
{
	// Name of the preset found inside the CompilePreset KeyValue
	DefaultMapBuilderSettings "Standard"
	
	// Inside this keyvalue you declare you all the preset 
	CompilePresets
    {
		"Fast"
		{
			RunWorldBuilder 
			{
				BuildParams "-game %gamedir %source"
			}

			RunVisibilityBuilder 	
			{
				BuildParams "-fast -game %gamedir %source"
			}

			RunLightBuilder 		
			{
				BuildParams "-fast -fastambient -noextra -bounce 1 -bounce 1 -game %gamedir %source"
			}

			RunTrasferFile
			{
				BuillParams "%mapbsp %mapdir"
			}
		}

		"Standard"
		{
			RunWorldBuilder 
			{
				BuildParams "-game %gamedir %source"
			}

			RunVisibilityBuilder 	
			{
				BuildParams "-game %gamedir %source"
			}

			RunLightBuilder 		
			{
				BuildParams "-game %gamedir %source"
			}

			RunTrasferFile
			{
				BuildParams "%mapbsp %mapdir"
			}
		}

		"Final"
		{
			RunWorldBuilder 
			{
				BuildParams "-game %gamedir %source"
			}

			RunVisibilityBuilder 	
			{
				BuildParams "-game %gamedir %source"
			}

			RunLightBuilder 		
			{
				BuildParams "-final -extrasky 64 -bounce 250 -game %gamedir %source"
			}

			RunMapInfo
			{
				BuildParams "-worldtexturestats -treeinfo %mapbsp"
			}

			RunTrasferFile
			{
				BuildParams "%mapbsp %mapdir"
			}

			RunCubemapBuilder 		
			{
				ToolName 	"tf.exe" // For RunCubemapBuilder this paramet always needs to be set by the user. 
				BuildParams "-dev -novid -sw -console -buildcubeamps map %filename"
			}

			// Custom process, put here whatever you want...
			// Here is an example of a custom proccess
			//    RunCustomBuilder
			//    {
			//		  BaseBin 1
			//        ToolName "mytool.exe"
			//        BuildParams "%source %mapdir"
			//    }
		}
	}
}
```
---  

## Contact  

Steam: https://steamcommunity.com/profiles/76561199073832016/  
Twitter: https://x.com/47Z14  
Discord: `carlossuarez7285`  

---
