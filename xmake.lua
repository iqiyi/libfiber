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

-- samples: server
target("server")
    set_kind("binary")
    add_deps("fiber")
    add_files("samples/patch.c")
    add_files("samples/server/*.c")

-- benchmark: libco_server
target("libco_server")
    set_default(false)
    set_kind("binary")
    add_deps("fiber")
    add_files("benchmark/patch.cpp")
    add_files("benchmark/libco/*.cpp")

-- benchmark: libgo_server
target("libgo_server")
    set_default(false)
    set_kind("binary")
    add_deps("fiber")
    add_files("benchmark/patch.cpp")
    add_files("benchmark/libgo/*.cpp")

-- benchmark: libmill_server
target("libmill_server")
    set_default(false)
    set_kind("binary")
    add_deps("fiber")
    add_files("benchmark/patch.c")
    add_files("benchmark/libmill/*.c")

-- benchmark: tbox_server
target("tbox_server")
    set_default(false)
    set_kind("binary")
    add_deps("fiber")
    add_files("benchmark/patch.c")
    add_files("benchmark/tbox/*.c")
