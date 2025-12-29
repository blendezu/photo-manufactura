/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "Halide", "index.html", [
    [ "Building Halide with CMake", "md_doc_2_building_halide_with_c_make.html", [
      [ "Installing CMake", "md_doc_2_building_halide_with_c_make.html#installing-cmake", [
        [ "Cross-platform", "md_doc_2_building_halide_with_c_make.html#cross-platform", null ],
        [ "Windows", "md_doc_2_building_halide_with_c_make.html#windows", null ],
        [ "macOS", "md_doc_2_building_halide_with_c_make.html#macos", null ],
        [ "Ubuntu Linux", "md_doc_2_building_halide_with_c_make.html#ubuntu-linux", null ],
        [ "Optional: Install Ninja", "md_doc_2_building_halide_with_c_make.html#optional-install-ninja", null ]
      ] ],
      [ "Dependencies", "md_doc_2_building_halide_with_c_make.html#dependencies", [
        [ "Summary", "md_doc_2_building_halide_with_c_make.html#summary", null ],
        [ "Installing dependencies", "md_doc_2_building_halide_with_c_make.html#installing-dependencies", [
          [ "vcpkg", "md_doc_2_building_halide_with_c_make.html#vcpkg", null ],
          [ "Windows", "md_doc_2_building_halide_with_c_make.html#windows-1", null ],
          [ "Homebrew", "md_doc_2_building_halide_with_c_make.html#homebrew", null ],
          [ "Ubuntu / Debian", "md_doc_2_building_halide_with_c_make.html#ubuntu--debian", null ],
          [ "Python", "md_doc_2_building_halide_with_c_make.html#python", null ]
        ] ]
      ] ],
      [ "Building Halide", "md_doc_2_building_halide_with_c_make.html#building-halide", [
        [ "Basic build", "md_doc_2_building_halide_with_c_make.html#basic-build", [
          [ "Windows", "md_doc_2_building_halide_with_c_make.html#windows-2", null ],
          [ "macOS and Linux", "md_doc_2_building_halide_with_c_make.html#macos-and-linux", null ]
        ] ],
        [ "CMake Presets", "md_doc_2_building_halide_with_c_make.html#cmake-presets", [
          [ "Common presets", "md_doc_2_building_halide_with_c_make.html#common-presets", null ],
          [ "Vcpkg presets", "md_doc_2_building_halide_with_c_make.html#vcpkg-presets", null ],
          [ "Sanitizer presets", "md_doc_2_building_halide_with_c_make.html#sanitizer-presets", null ]
        ] ],
        [ "Build options", "md_doc_2_building_halide_with_c_make.html#build-options", null ],
        [ "Installing", "md_doc_2_building_halide_with_c_make.html#installing", null ]
      ] ],
      [ "Building Halide with pip", "md_doc_2_building_halide_with_c_make.html#building-halide-with-pip", [
        [ "Using ccache with pip builds", "md_doc_2_building_halide_with_c_make.html#using-ccache-with-pip-builds", null ]
      ] ]
    ] ],
    [ "Contributing CMake code to Halide", "md_doc_2_code_style_c_make.html", [
      [ "General guidelines and best practices", "md_doc_2_code_style_c_make.html#general-guidelines-and-best-practices", [
        [ "Prohibited commands list", "md_doc_2_code_style_c_make.html#prohibited-commands-list", null ],
        [ "Prohibited variables list", "md_doc_2_code_style_c_make.html#prohibited-variables-list", null ]
      ] ],
      [ "Adding tests", "md_doc_2_code_style_c_make.html#adding-tests", null ],
      [ "Adding apps", "md_doc_2_code_style_c_make.html#adding-apps", null ]
    ] ],
    [ "Fuzz testing", "md_doc_2_fuzz_testing.html", [
      [ "Building fuzz tests", "md_doc_2_fuzz_testing.html#building-fuzz-tests", null ],
      [ "Using the fuzz-harnesses", "md_doc_2_fuzz_testing.html#using-the-fuzz-harnesses", null ],
      [ "Reproducing crashes", "md_doc_2_fuzz_testing.html#reproducing-crashes", null ],
      [ "Adding new fuzz tests", "md_doc_2_fuzz_testing.html#adding-new-fuzz-tests", null ],
      [ "Other useful materials", "md_doc_2_fuzz_testing.html#other-useful-materials", null ]
    ] ],
    [ "Using Halide from your CMake build", "md_doc_2_halide_c_make_package.html", [
      [ "A basic CMake project", "md_doc_2_halide_c_make_package.html#a-basic-cmake-project", [
        [ "JIT mode", "md_doc_2_halide_c_make_package.html#jit-mode", null ],
        [ "AOT mode", "md_doc_2_halide_c_make_package.html#aot-mode", [
          [ "Autoschedulers", "md_doc_2_halide_c_make_package.html#autoschedulers", null ],
          [ "RunGenMain", "md_doc_2_halide_c_make_package.html#rungenmain", null ]
        ] ]
      ] ],
      [ "Halide package documentation", "md_doc_2_halide_c_make_package.html#halide-package-documentation", [
        [ "Components", "md_doc_2_halide_c_make_package.html#components", null ],
        [ "Variables", "md_doc_2_halide_c_make_package.html#variables", null ],
        [ "Imported targets", "md_doc_2_halide_c_make_package.html#imported-targets", null ],
        [ "Functions", "md_doc_2_halide_c_make_package.html#functions", [
          [ "<span class=\"tt\">add_halide_generator</span>", "md_doc_2_halide_c_make_package.html#add_halide_generator", null ],
          [ "<span class=\"tt\">add_halide_library</span>", "md_doc_2_halide_c_make_package.html#add_halide_library", null ],
          [ "<span class=\"tt\">add_halide_python_extension_library</span>", "md_doc_2_halide_c_make_package.html#add_halide_python_extension_library", null ],
          [ "<span class=\"tt\">add_halide_runtime</span>", "md_doc_2_halide_c_make_package.html#add_halide_runtime", null ]
        ] ]
      ] ],
      [ "Cross compiling", "md_doc_2_halide_c_make_package.html#cross-compiling", [
        [ "Use <span class=\"tt\">add_halide_generator</span>", "md_doc_2_halide_c_make_package.html#use-add_halide_generator", null ],
        [ "Use a super-build", "md_doc_2_halide_c_make_package.html#use-a-super-build", null ],
        [ "Use <span class=\"tt\">ExternalProject</span> directly", "md_doc_2_halide_c_make_package.html#use-externalproject-directly", null ],
        [ "Use an emulator or run on device", "md_doc_2_halide_c_make_package.html#use-an-emulator-or-run-on-device", null ],
        [ "Bypass CMake", "md_doc_2_halide_c_make_package.html#bypass-cmake", null ]
      ] ]
    ] ],
    [ "Halide for Hexagon HVX", "md_doc_2_hexagon.html", [
      [ "1. Obtain and build trunk LLVM and Clang", "md_doc_2_hexagon.html#autotoc_md1-obtain-and-build-trunk-llvm-and-clang", null ],
      [ "2. Download and install the Hexagon SDK and Hexagon Tools", "md_doc_2_hexagon.html#autotoc_md2-download-and-install-the-hexagon-sdk-and-hexagon-tools", null ],
      [ "3. Build and run an example for Hexagon HVX", "md_doc_2_hexagon.html#autotoc_md3-build-and-run-an-example-for-hexagon-hvx", null ],
      [ "To build and run the blur example in Halide/apps/blur on Android:", "md_doc_2_hexagon.html#to-build-and-run-the-blur-example-in-halideappsblur-on-android", null ]
    ] ],
    [ "Halide Bindings for Python", "md_doc_2_python.html", [
      [ "Acquiring the Python bindings", "md_doc_2_python.html#acquiring-the-python-bindings", null ],
      [ "Building the Python bindings", "md_doc_2_python.html#building-the-python-bindings", [
        [ "Using CMake directly", "md_doc_2_python.html#using-cmake-directly", null ],
        [ "Using wheel infrastructure", "md_doc_2_python.html#using-wheel-infrastructure", null ]
      ] ],
      [ "Documentation and Examples", "md_doc_2_python.html#documentation-and-examples", null ],
      [ "Differences from C++ API", "md_doc_2_python.html#differences-from-c-api", null ],
      [ "Example of Simple Usage", "md_doc_2_python.html#example-of-simple-usage", null ],
      [ "Halide Generators In Python", "md_doc_2_python.html#halide-generators-in-python", [
        [ "Writing a Generator in Python", "md_doc_2_python.html#writing-a-generator-in-python", [
          [ "<span class=\"tt\">hl.generator(\"name\")</span>", "md_doc_2_python.html#hlgeneratorname", null ],
          [ "hl.GeneratorParam", "md_doc_2_python.html#hlgeneratorparam", null ],
          [ "hl.InputBuffer, hl.InputScalar", "md_doc_2_python.html#hlinputbuffer-hlinputscalar", null ],
          [ "hl.OutputBuffer, hl.OutputScalar", "md_doc_2_python.html#hloutputbuffer-hloutputscalar", null ],
          [ "Names", "md_doc_2_python.html#names", null ],
          [ "generate() method", "md_doc_2_python.html#generate-method", null ],
          [ "Types for Inputs and Outputs", "md_doc_2_python.html#types-for-inputs-and-outputs", null ]
        ] ],
        [ "Using a Generator for JIT compilation", "md_doc_2_python.html#using-a-generator-for-jit-compilation", null ],
        [ "Using a Generator for AOT compilation", "md_doc_2_python.html#using-a-generator-for-aot-compilation", null ],
        [ "Calling Generator-Produced code from Python", "md_doc_2_python.html#calling-generator-produced-code-from-python", null ],
        [ "Advanced Generator-Related Topics", "md_doc_2_python.html#advanced-generator-related-topics", [
          [ "Generator Aliases", "md_doc_2_python.html#generator-aliases", null ],
          [ "Dynamic Inputs and Outputs", "md_doc_2_python.html#dynamic-inputs-and-outputs", null ],
          [ "Calling a Generator Directly", "md_doc_2_python.html#calling-a-generator-directly", null ],
          [ "The Lifecycle Of A Generator", "md_doc_2_python.html#the-lifecycle-of-a-generator", null ],
          [ "Notable Differences Between C++ and Python Generators", "md_doc_2_python.html#notable-differences-between-c-and-python-generators", null ]
        ] ]
      ] ],
      [ "Keeping Up To Date", "md_doc_2_python.html#keeping-up-to-date", null ],
      [ "License", "md_doc_2_python.html#license", null ]
    ] ],
    [ "Running and Benchmarking Halide Generators", "md_doc_2_run_gen.html", [
      [ "Overview", "md_doc_2_run_gen.html#overview", null ],
      [ "Benchmarking", "md_doc_2_run_gen.html#benchmarking", null ],
      [ "Measuring Memory Usage", "md_doc_2_run_gen.html#measuring-memory-usage", null ],
      [ "Using RunGen in Make", "md_doc_2_run_gen.html#using-rungen-in-make", null ],
      [ "Known Issues &amp; Caveats", "md_doc_2_run_gen.html#known-issues--caveats", null ]
    ] ],
    [ "Testing", "md_doc_2_testing.html", [
      [ "Organization", "md_doc_2_testing.html#organization", null ],
      [ "Debugging with LLDB", "md_doc_2_testing.html#debugging-with-lldb", null ]
    ] ],
    [ "Vulkan Support for Halide", "md_doc_2_vulkan.html", [
      [ "Compiling Halide w/Vulkan Support", "md_doc_2_vulkan.html#compiling-halide-wvulkan-support", null ],
      [ "Vulkan Runtime Environment:", "md_doc_2_vulkan.html#vulkan-runtime-environment", [
        [ "Windows", "md_doc_2_vulkan.html#windows-3", null ],
        [ "Linux", "md_doc_2_vulkan.html#linux", null ],
        [ "Mac", "md_doc_2_vulkan.html#mac", null ]
      ] ],
      [ "Testing Your Vulkan Environment", "md_doc_2_vulkan.html#testing-your-vulkan-environment", null ],
      [ "Targetting Vulkan", "md_doc_2_vulkan.html#targetting-vulkan", null ],
      [ "Useful Runtime Environment Variables", "md_doc_2_vulkan.html#useful-runtime-environment-variables", null ],
      [ "Debug Environment Variables", "md_doc_2_vulkan.html#debug-environment-variables", null ],
      [ "Current Status", "md_doc_2_vulkan.html#current-status", null ],
      [ "Caveats:", "md_doc_2_vulkan.html#caveats", null ],
      [ "Known TODO:", "md_doc_2_vulkan.html#known-todo", null ]
    ] ],
    [ "WebAssembly Support for Halide", "md_doc_2_web_assembly.html", [
      [ "Additional Tooling Requirements:", "md_doc_2_web_assembly.html#additional-tooling-requirements", null ],
      [ "AOT Limitations", "md_doc_2_web_assembly.html#aot-limitations", null ],
      [ "JIT Limitations", "md_doc_2_web_assembly.html#jit-limitations", [
        [ "Using V8 as the interpreter", "md_doc_2_web_assembly.html#using-v8-as-the-interpreter", null ]
      ] ],
      [ "To Use Halide For WebAssembly:", "md_doc_2_web_assembly.html#to-use-halide-for-webassembly", [
        [ "Enabling wasm JIT", "md_doc_2_web_assembly.html#enabling-wasm-jit", null ],
        [ "Enabling wasm AOT", "md_doc_2_web_assembly.html#enabling-wasm-aot", null ]
      ] ],
      [ "Running benchmarks", "md_doc_2_web_assembly.html#running-benchmarks", null ],
      [ "Using Threads", "md_doc_2_web_assembly.html#using-threads", null ],
      [ "Known Limitations And Caveats", "md_doc_2_web_assembly.html#known-limitations-and-caveats", null ],
      [ "Known TODO:", "md_doc_2_web_assembly.html#known-todo-1", null ]
    ] ],
    [ "WebGPU support for Halide", "md_doc_2_web_g_p_u.html", [
      [ "Known limitations", "md_doc_2_web_g_p_u.html#known-limitations", null ],
      [ "Running with WebAssembly via Emscripten: <span class=\"tt\">HL_TARGET=wasm-32-wasmrt-webgpu</span>", "md_doc_2_web_g_p_u.html#running-with-webassembly-via-emscripten-hl_targetwasm-32-wasmrt-webgpu", null ],
      [ "Running natively: <span class=\"tt\">HL_TARGET=host-webgpu</span>", "md_doc_2_web_g_p_u.html#running-natively-hl_targethost-webgpu", null ],
      [ "Setting up Dawn", "md_doc_2_web_g_p_u.html#setting-up-dawn", null ],
      [ "Updating mini_webgpu.h", "md_doc_2_web_g_p_u.html#updating-mini_webgpuh", null ]
    ] ],
    [ "Topics", "topics.html", "topics" ],
    [ "Namespaces", "namespaces.html", [
      [ "Namespace List", "namespaces.html", "namespaces_dup" ],
      [ "Namespace Members", "namespacemembers.html", [
        [ "All", "namespacemembers.html", "namespacemembers_dup" ],
        [ "Functions", "namespacemembers_func.html", "namespacemembers_func" ],
        [ "Variables", "namespacemembers_vars.html", null ],
        [ "Typedefs", "namespacemembers_type.html", null ],
        [ "Enumerations", "namespacemembers_enum.html", null ],
        [ "Enumerator", "namespacemembers_eval.html", null ]
      ] ]
    ] ],
    [ "Classes", "annotated.html", [
      [ "Class List", "annotated.html", "annotated_dup" ],
      [ "Class Hierarchy", "hierarchy.html", "hierarchy" ],
      [ "Class Members", "functions.html", [
        [ "All", "functions.html", "functions_dup" ],
        [ "Functions", "functions_func.html", "functions_func" ],
        [ "Variables", "functions_vars.html", "functions_vars" ],
        [ "Typedefs", "functions_type.html", null ],
        [ "Enumerations", "functions_enum.html", null ],
        [ "Enumerator", "functions_eval.html", "functions_eval" ],
        [ "Related Symbols", "functions_rela.html", null ]
      ] ]
    ] ],
    [ "Files", "files.html", [
      [ "File List", "files.html", "files_dup" ],
      [ "File Members", "globals.html", [
        [ "All", "globals.html", "globals_dup" ],
        [ "Functions", "globals_func.html", "globals_func" ],
        [ "Variables", "globals_vars.html", null ],
        [ "Typedefs", "globals_type.html", "globals_type" ],
        [ "Enumerations", "globals_enum.html", null ],
        [ "Enumerator", "globals_eval.html", "globals_eval" ],
        [ "Macros", "globals_defs.html", "globals_defs" ]
      ] ]
    ] ],
    [ "Examples", "examples.html", "examples" ]
  ] ]
];

var NAVTREEINDEX =
[
"_a_s_log_8h.html",
"_halide_runtime_8h.html#a1e478ab191f59290de68e17d3f624da1",
"_halide_runtime_8h.html#aba16e170cc9aa30b78ff703e101a448ca0e0b319ab036506ee80df45c1f5cdac0",
"_inline_reductions_8h_source.html",
"anderson2021_2_default_cost_model_8h.html",
"class_halide_1_1_compile_time_error_reporter.html#a2c0661870a3576a83777efdec5fc9cc6",
"class_halide_1_1_func.html#a7be85a5c88dec96e8b8ee8aa94816ff9",
"class_halide_1_1_func_tuple_element_ref.html#ad8ed697bea3d0f64c75469b0199bd9bc",
"class_halide_1_1_internal_1_1_autoscheduler_1_1_expr_branching.html",
"class_halide_1_1_internal_1_1_autoscheduler_1_1unordered__set.html#a9401ae48c5b80204409ee475f277cdb0",
"class_halide_1_1_internal_1_1_code_gen___c.html#a77286af3480e2ebe58a59dd2969adb36",
"class_halide_1_1_internal_1_1_code_gen___l_l_v_m.html#a056c5571524a7926b91716647a4e3d8b",
"class_halide_1_1_internal_1_1_code_gen___l_l_v_m.html#aab70d995e94fae105f05720babe3e61f",
"class_halide_1_1_internal_1_1_code_gen___posix.html#ae37d8e543796575154b0fbd858a50b04",
"class_halide_1_1_internal_1_1_elf_1_1_relocation.html#aa9b4324c9ad47ccb5b1e351a292f9b9f",
"class_halide_1_1_internal_1_1_func_schedule.html#a2fc8d895860be23b29d1aa0ad7354437",
"class_halide_1_1_internal_1_1_g_i_o_base.html#aa0f4fa2a2fc24351c4282b8a548d4603",
"class_halide_1_1_internal_1_1_generator_input___arithmetic.html#a6cbd1de37dada56d2076da49cfdf9e8f",
"class_halide_1_1_internal_1_1_generator_output___buffer.html#ad4b6e8c4d3c43ee7e6160999e38772b0",
"class_halide_1_1_internal_1_1_generator_param_base.html#ab2cdbefbb98ff4f2b809f8ff64a78ff6",
"class_halide_1_1_internal_1_1_i_r_graph_visitor.html#aa0f0b547bfec53c97a7c68e46ada238f",
"class_halide_1_1_internal_1_1_i_r_printer.html#a92ffb026251319615c47f9723bdd1883",
"class_halide_1_1_internal_1_1_object_instance_registry.html#a7b46432be45ba299cf8e057a6f3e355f",
"class_halide_1_1_internal_1_1_simplify.html#abfa5e9e0423e9399dda9e28f4f0e787d",
"class_halide_1_1_loop_level.html#a62ec62d4fe488a8633119d87fa19fb11",
"class_halide_1_1_parameter.html#a2d2fcffbf454c8a73986bf71f46ba8db",
"class_halide_1_1_py_torch_1_1_buffer.html#a2c5b03037f53363a00913ceef3bbf7aa",
"class_halide_1_1_r_dom.html#abf382bcc7a5f800de35d136d23a9e210",
"class_halide_1_1_runtime_1_1_buffer.html#ad0c19b380f0a928ea86df8616bde3b3c",
"class_halide_1_1_runtime_1_1_internal_1_1_memory_arena.html#a8d7029ae076b02d6d5b86466520010ac",
"class_halide_1_1_runtime_1_1_internal_1_1_synchronization_1_1fast__cond.html",
"class_halide_1_1_stage.html#ac638da06726dd7df2b0969126db5961f",
"functions_func_f.html",
"known__symbols_8h.html#a53e96bd2e0332094e653406d2451f50d",
"mini__cl_8h.html#a3e9bb4c252b675b9e4aad103fa9f0919",
"mini__cl_8h.html#acba80b97dbb2c22f707e8afee068bec3",
"mini__d3d12_8h.html#a2cf55f97f71b269d696bf2245e996aa8",
"mini__d3d12_8h.html#a6a9cbc7eff00c7ec769e4e6c3b2cb3bca2fcc5b49a6ab59bbbe20577309d22d98",
"mini__d3d12_8h.html#a9a1d8c36726ab1b8ff9022e92f5cffbe",
"mini__d3d12_8h.html#acd42770aecb025cfac170d4d3ace4544",
"mini__d3d12_8h.html#af5b7e37acc8533338422127bb23f5cb7",
"mini__webgpu_8h.html#a0e3a2414a79225161636aabf85871035a3eb9eb263a77626dbfa2554bc7c46ddd",
"mini__webgpu_8h.html#a3ec2d532b2ed6c57a4952a27f9a15174",
"mini__webgpu_8h.html#a7575408f6bc9f5866baa3b3b3f27a54d",
"mini__webgpu_8h.html#ab841cd33e4883f56e8f8893240b05941",
"namespace_halide.html#a02df159d6ed5236cc8e02d04bf72be4f",
"namespace_halide.html#a8e8cc876748afd9ec969b7d0d45d1008",
"namespace_halide_1_1_internal.html#a0396411abf88410baf6a6e6e41ea1419",
"namespace_halide_1_1_internal.html#a7114c2dcec2af79da28735c5b11c0916",
"namespace_halide_1_1_internal.html#ad90cfab6d9f71632c8e4b712c7c400e8",
"namespace_halide_1_1_internal_1_1_i_r_matcher.html#a2ca3aebf7980d62dafd0ca85581fe989",
"namespace_halide_1_1_runtime_1_1_internal.html#a86f1fdb22bd376c50f2e7babda95d12e",
"namespace_halide_1_1_runtime_1_1_internal_1_1_vulkan.html#a683285993b4c90c563c285568dcfe7f9",
"struct___s_e_c_u_r_i_t_y___a_t_t_r_i_b_u_t_e_s.html#a90d1d49f7d2d50b24cc74c5060ae6204",
"struct_d3_d12___r_e_s_o_u_r_c_e___d_e_s_c.html#a9037125128e3e4a735965bd6d9479a6c",
"struct_d3_d12___u_n_o_r_d_e_r_e_d___a_c_c_e_s_s___v_i_e_w___d_e_s_c.html#a9f0312075fbc3a19136ede101f214a28",
"struct_halide_1_1_extern_func_argument.html#acaf18f00d1d41e200ba8c7ac799a172baf2b30b30d6bb4a77d4b3b8d1d17350cc",
"struct_halide_1_1_internal_1_1_autoscheduler_1_1_anderson2021_params.html#a9d4f22c20278edd8234ad9ce14faa4da",
"struct_halide_1_1_internal_1_1_autoscheduler_1_1_function_d_a_g_1_1_node_1_1_region_computed_info.html#a1d1bb1dcb10d69cc527bf6ecc1e5ece6",
"struct_halide_1_1_internal_1_1_autoscheduler_1_1_loop_nest.html#a9891c9a1e72e40f6f019d6f55dad7fba",
"struct_halide_1_1_internal_1_1_autoscheduler_1_1_loop_nest_1_1_stage_schedule_state_1_1_func_var.html#a9df868628c6ae9da9c6186d494a59256",
"struct_halide_1_1_internal_1_1_autoscheduler_1_1_state.html#a30a728c59b08a8b8890782177c27160f",
"struct_halide_1_1_internal_1_1_autoscheduler_1_1_timer.html#a927bb363e2e754f5195fac8a1852c5d7",
"struct_halide_1_1_internal_1_1_call.html#a3643a3e5cf73435e07215fca17949974aae66666598d3bced4268916d6beb6a83",
"struct_halide_1_1_internal_1_1_constant_interval.html#a92022bae4ac1b8ba49568e297c6b22cb",
"struct_halide_1_1_internal_1_1_fused_pair.html",
"struct_halide_1_1_internal_1_1_i_r_matcher_1_1_is_u_int.html#a616afcd9fc4f5296dea146c078564991",
"struct_halide_1_1_internal_1_1_i_r_matcher_1_1_wild_const_int.html#a8ec2c97dd233ca7815c71c5eb2be9933",
"struct_halide_1_1_internal_1_1_let_stmt.html",
"struct_halide_1_1_internal_1_1_pipeline_features.html#a24dd570f5cbc57c750aca4c0d82f418dac33315685a0cba3ce53be378b3c7874b",
"struct_halide_1_1_internal_1_1_producer_consumer.html#a46c54dfc09aee38391dbe0bba6292f3c",
"struct_halide_1_1_internal_1_1_schedule_features.html#aacaae733178cf4dc0c266c4c812bcf6d",
"struct_halide_1_1_internal_1_1_solver_result.html#ab584af3b6ecefae73c20cef1bdf66dc2",
"struct_halide_1_1_j_i_t_extern.html#a6280a04b7678702ce855712fbf0ea6bf",
"struct_halide_1_1_runtime_1_1_internal_1_1_cuda_1_1_c_u_d_a___m_e_m_c_p_y3_d__st.html#ab5f2ae0be3f0bed7a85d1f1ab950dc32",
"struct_halide_1_1_runtime_1_1_internal_1_1_synchronization_1_1signal__parking__control.html#a02a4a4f6d30ec4ff7649bd5476f96fb3",
"struct_halide_1_1_target.html#a000e35902bb5bcb26cea9955c87c4894",
"struct_halide_1_1_target.html#a9398276edd5784185e1a08f8a4dfc43fad9b234dbac225381e9e9ea50ed83c75d",
"struct_halide_1_1bfloat16__t.html#a51b4bb9a8ef3373366f38314bc195926",
"struct_i_d3_d12_device_vtbl.html#a634856b73bd11847741714e2c2b6866d",
"struct_i_d3_d12_graphics_command_list_vtbl.html#aebc3e91d06b6d23daf15123d1b18c35a",
"struct_w_g_p_u_bind_group_entry.html#accfb78bbda8b9cad4cc45e84f1d158e1",
"struct_w_g_p_u_instance_features.html#a447a1948abbdcc253be91d2fb9d45058",
"struct_w_g_p_u_shared_texture_memory_d_x_g_i_shared_handle_descriptor.html",
"structhalide__filter__argument__t.html",
"vulkan__interface_8h.html#a43597394436598f2cae8c3e804b49683"
];

var SYNCONMSG = 'click to disable panel synchronization';
var SYNCOFFMSG = 'click to enable panel synchronization';
var LISTOFALLMEMBERS = 'List of all members';