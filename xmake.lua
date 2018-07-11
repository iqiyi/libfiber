-- for the windows platform (msvc)
if is_plat("windows") then 
    if is_mode("release") then
        add_cxflags("-MT") 
    elseif is_mode("debug") then
        add_cxflags("-MTd") 
    end
    add_ldflags("-nodefaultlib:\"msvcrt.lib\"")
    add_links("kernel32", "user32", "gdi32", "winspool", "comdlg32", "advapi32", "ws2_32")
end

-- define target: fiber
target("fiber")

    -- set kind: static/shared
    set_kind("$(kind)")

    -- add deps: acl
    -- add_deps("acl")

    -- add source files
    add_files("src/**.c")

    -- add include directories
    add_includedirs("src", "include")

    -- add headers
    add_headers("include/(**.h)")
    set_headerdir("$(buildir)/include/fiber")

    -- add flags
    add_cxflags("-std=gnu99")

    -- add defines
    add_defines("USE_JMP")

-- samples: client
target("client")
    set_kind("binary")
    add_deps("fiber")
    add_files("samples/patch.c")
    add_files("samples/client/*.c")
    if is_plat("windows") then
        add_files("samples/getopt.c")
    end

-- samples: server
target("server")
    set_kind("binary")
    add_deps("fiber")
    add_files("samples/patch.c")
    add_files("samples/server/*.c")
    if is_plat("windows") then
        add_files("samples/getopt.c")
    end

-- benchmark: libco_server
target("libco_server")
    set_default(false)
    set_kind("binary")
    add_deps("fiber")
    add_files("benchmark/patch.cpp")
    add_files("benchmark/libco/*.cpp")
    if is_plat("windows") then
        add_files("samples/getopt.c")
    end

-- benchmark: libgo_server
target("libgo_server")
    set_default(false)
    set_kind("binary")
    add_deps("fiber")
    add_files("benchmark/patch.cpp")
    add_files("benchmark/libgo/*.cpp")
    if is_plat("windows") then
        add_files("samples/getopt.c")
    end

-- benchmark: libmill_server
target("libmill_server")
    set_default(false)
    set_kind("binary")
    add_deps("fiber")
    add_files("benchmark/patch.cpp")
    add_files("benchmark/libmill/*.c")
    if is_plat("windows") then
        add_files("samples/getopt.c")
    end

-- benchmark: tbox_server
target("tbox_server")
    set_default(false)
    set_kind("binary")
    add_deps("fiber")
    add_files("benchmark/patch.cpp")
    add_files("benchmark/tbox/*.c")
    if is_mode("debug") then
        add_defines("__tb_debug__")
    end
    if is_plat("windows") then
        add_files("samples/getopt.c")
    end
    on_load(function (target)
        target:add(import("lib.detect.find_package")("tbox", {packagedirs = path.join(os.projectdir(), "package")}))
    end)
