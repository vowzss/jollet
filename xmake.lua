-- project metadata
set_project("serris")
set_version("1.0.0")
set_languages("cxx17")

-- build modes
add_rules("mode.debug", "mode.release")

-- build settings
if is_mode("debug") then
    set_symbols("debug")
    set_optimize("none")
else
    set_symbols("hidden")
    set_optimize("fastest")
end

-- dependencies
add_requires("conan::doctest/2.4.12", {alias = "doctest"})

-- core target
target("serris")
    set_kind("shared")
    set_filename("serris")

    -- headers
    add_headerfiles("include/serris/(**.h)")
    add_includedirs("include", {public = true})

    -- sources
    add_files("src/*.cpp")
    add_files("src/**/*.cpp")
target_end()

-- test project
target("serris_tests")
    set_kind("binary")
    set_rundir("$(projectdir)/tests")

    -- link
    add_deps("serris")

    -- dependencies
    add_packages("doctest")

    -- sources
    add_files("tests/*.cpp")
target_end()