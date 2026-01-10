The dynamit library is implemented for C++ and Javascript:
        /dynamit_js: a javascript webgl2 implementation
           located in dynamit_js folder, runs directly in web browsers
		   does not have special requirements
		/dynamit_gl: a c++ implementation
    More detailed description of dynamit api located in /docs folder

NOTE: this project is being actively developed and is subject to changes

To use the C++ version of dynamit
1. Open 3rdparty folder
    Run extract_libs.cmd
2. From the folder dynamit_gl
    Open dynamit_gl.sln with VisualStudio 2022
        Alternatively can be opened with VisualStudio 2026
		Guaranteed to work, but additional steps might be required co configure the project
3. Solution contains projects
        dynamit_gl:   key dynamit library project
                      this library contains a copy of 3DCalculator and depends on it
        3DCalculator: math library, used by dynamit_gl
                      after build it is automatically exported entirely into dynamit_gl
        3DApi:        samples of usage, each sample includes enabler.h.
                      uncomment one single #define in enabler.h to enable particular sample.


4. dynamit_gl and 3DApi projects rely on libraries
        glew, glfw, glm, stb -- versions seen in 3rdparty folder
                             -- dynamic linkage is used
        Settings are applied by using following property file: gl_glew.props
    Alternatively, for middle/advanced programmers:
          glad can be used
		  static linkage can be manually configured
		  different property files created to replace or to add to current properties

