-- project metadata
set_project("serris")
set_version("1.0.0")
set_languages("cxx17")

-- build modes
add_rules("mode.debug", "mode.release")
set_defaultmode("debug")

-- build settings
if is_mode("debug") then
    set_symbols("debug")
    set_optimize("none")
else
    set_symbols("hidden")
    set_optimize("fastest")
end

-- dependencies
add_requires("conan::utfcpp/4.0.8", {alias = "utfcpp"})
add_requires("conan::fast_float/8.0.2", {alias = "fast_float"})
add_requires("conan::tsl-robin-map/1.4.0", {alias = "tsl-robin-map"})

add_requires("conan::doctest/2.4.12", {alias = "doctest"})

-- core target
target("serris_core")
    set_kind("shared")
    set_basename("serris")

    -- dependencies
    add_packages("utfcpp")
    add_packages("fast_float", "tsl-robin-map")

    -- headers
    add_headerfiles("include/serris/(**.h)")
    add_includedirs("include", {public = true})

    -- sources
    -- add_files("src/*.cpp")
    add_files("src/**/*.cpp")

    add_defines("SERRIS_EXPORTS")
target_end()

-- test project
target("serris_tests")
    set_kind("binary")

    -- link
    add_deps("serris_core")
    add_linkdirs("$(builddir)/$(os)/$(arch)/$(mode)")
    add_links("serris")
    
    -- dependencies
    add_packages("doctest")

    -- sources
    add_files("tests/*.cpp")

    -- package
    after_build(function (target)
        local builddir = target:targetdir()
        local srcdir = path.join("$(projectdir)", "resources")
        local destdir = path.join(builddir, "resources")

        os.mkdir(destdir)
        os.cp(path.join(srcdir, "*"), destdir, {recursive = true})
    end)
target_end()