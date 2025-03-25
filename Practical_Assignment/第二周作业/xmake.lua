rule("Imguiini")
    after_build(
        function(target)
            iniFile = path.join(target:scriptdir(), "imgui.ini")
            if os.isfile(iniFile) then
                os.cp(iniFile, target:targetdir())
            end
        end)
rule_end()

rule("ShaderCopy")
    set_extensions(".hlsl", ".hlsli")
    after_build(
        function(target)
            shaderFiles = path.join(target:scriptdir(), "/Shaders");
            if(os.exists(shaderFiles)) then
                os.cp(shaderFiles, target:targetdir())
            end
        end)
rule_end()

rule("ModelCopy")
    after_build(
        function(target)
            modelFiles = path.join(target:scriptdir(), "/Model")
            if(os.exists(modelFiles)) then
                os.cp(modelFiles, target:targetdir())
            end
        end)
rule_end()

rule("TextureCopy")
    after_build(
        function(target)
            modelFiles = path.join(target:scriptdir(), "/Texture")
            if(os.exists(modelFiles)) then
                os.cp(modelFiles, target:targetdir())
            end
        end)
rule_end()

set_project("DX11")

option("WIN7_SYSTEM_SUPPORT")
    set_default(false)
    set_description("Windows7 users need to select this option!")
option_end()

-- 需要定义 UNICODE 否则 TCHAR 无法转换为 wstring 
if is_os("windows") then 
    add_defines("UNICODE")
    add_defines("_UNICODE")
end

add_rules("mode.debug", "mode.release")

set_languages("c99", "cxx17")
set_toolchains("msvc")
set_encodings("utf-8")


if is_mode("debug") then 
    binDir = path.join(os.projectdir(), "Bin/Debug")
else 
    binDir = path.join(os.projectdir(), "Bin/Release")
end 


function add_dxsdk_options() 
    if has_config("WIN7_SYSTEM_SUPPORT") then
        add_defines("_WIN32_WINNT=0x601")
    end
    -- 添加系统库依赖
    add_syslinks("d3d11", "dxgi", "d3dcompiler", "dxguid", "d2d1","dwrite","winmm","user32","gdi32","ole32")
end

-- 添加需要的依赖包,同时禁用系统包
add_requires("assimp", {system = false})

includes("ImGui")


target("Project")
    set_kind("binary")
    set_targetdir(path.join(binDir, targetName))
    add_deps("ImGui")

    
    add_rules("Imguiini")
    add_rules("ShaderCopy")
    add_rules("ModelCopy")
    add_rules("TextureCopy")
    add_dxsdk_options()

    add_headerfiles("Src/**.h")
    add_files("Src/**.cpp")
    add_headerfiles("Shaders/**.hlsli", "Shaders/**.hlsl")
    
target_end()