-- Project settings
set_project("jollet")
set_version("1.0.0")
set_languages("cxx17")

-- Build modes
add_rules("mode.debug", "mode.release")

-- Dependencies
add_requires("conan::doctest/2.4.12", {alias = "doctest"})

-- Library project
target("jollet")
    set_kind("headeronly")
    set_basename("jollet")

    -- Headers files
    add_includedirs("include", {public = true})
    add_headerfiles("include/(**.h)")
    
    -- Build modes
    if is_mode("debug") then
        set_symbols("debug")
        set_optimize("none")
    else
        set_symbols("hidden")
        set_optimize("fastest")
    end
target_end()

-- Tests project
target("jollet_tests")
    set_kind("binary")
    add_deps("jollet")

    add_files("tests/**.cpp")

    add_packages("doctest")

    set_rundir("$(projectdir)/tests")
target_end()