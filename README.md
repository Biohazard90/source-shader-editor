SOURCE SHADER EDITOR
====================

This is the source code for the Source Shader Editor (SSE) for the Source Engine. 

The repository for the Source SDK 2013 can be found here:
http://github.com/ValveSoftware/source-sdk-2013
The SSE can be compiled for older engine version too, including 2007, 2006 and the
Alien Swarm SDK.

Documentation on the editor itself can be found here:
http://developer.valvesoftware.com/wiki/Category:SourceShaderEditor

USAGE:
--------------

1.) Clone this repository into a separate directory outside of your games source code.

2.) Edit the file src/shadereditor_platform.vpc to point to specify the location of
    your game, the source code of your game and the target Source Engine SDK version
    like this:

    TARGET_SRC_DIRECTORY -  Has to point to the root src/ directory, so for a single player mod on the
                            Source 2013 SDK it would point to "<ROOT>/sp/src/". This directory is used
                            to pull in SDK specific libraries and includes, like tier0, tier1 etc.

    TARGET_PLATFORM_X -     Specifies the target SDK version.
                            Possible options are currently: 2013, SWARM, 2007 and 2006 PORTAL2 is
                            experimental and cannot be shared as it would require references to closed
                            source.
                            These macros are used to add the preprocessor definition
                            SHADER_EDITOR_DLL_<VERSION> to all projects and toggle inclusion of files
                            in vpc scripts. Make sure you only enable the one you want to compile for!

    TARGET_GAME_DIRECTORY - Specifies the directory of your game. So for a Source Engine modification
                            it typically lies in SteamApps/sourcemods/<YOURGAME>/. This macro is used
                            to define the output directory of the binaries you build.

3.) Run src/createshadereditorprojects.bat to build projects and solution for
    Visual Studio.

4.) Open the solution and build it. The resulting binaries will be copied into the
    target directory you specified with TARGET_GAME_DIRECTORY.

5.) Add the directory from shadereditor from client/ to your games client library
    as described here:
    https://developer.valvesoftware.com/wiki/SourceShaderEditor/Installation#Compile

6.) Copy the contents of the directory game/ to your game root directory, the
    editor is relying on these resources to be present.

Editor internal shaders can be build by calling
src\materialsystem\procshader\buildeditorshaders.bat. Refer to the instructions inside
the batch file to set up your paths appropriately!

NOTES:
--------------

The libraries build by this project are shadereditor_<PLATFORM>.dll, so
shadereditor_2013.dll, for example, and game_shader_generic_eshader_<PLATFORM>.dll
respectively.

For the 2013 SDK, the game shader library is called game_shader_dx6.dll, because
Valve recently broke shader dll loading through the wildcard game_shader_generic*.dll.
