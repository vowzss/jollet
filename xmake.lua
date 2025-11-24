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
    --set_symbols("hidden")
    set_optimize("fastest")
end

-- dependencies
add_requires("utfcpp",      { alias = "utfcpp" })
add_requires("fast_float",  { alias = "fast_float" })
add_requires("robin-map",   { alias = "fast_map" })

add_requires("doctest",     {alias = "doctest"})

-- core target
target("serris_core")
    set_kind("static")
    set_basename("serris")

    -- dependencies
    add_packages("utfcpp")
    add_packages("fast_float")
    add_packages("fast_map", { public = true })
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
        local srcdir = path.join("$(projectdir)", "tests", "resources")
        local destdir = path.join(builddir, "resources")

        os.mkdir(destdir)
        os.cp(path.join(srcdir, "*"), destdir, {recursive = true})
    end)
target_end()