# Description:
#   Redis (http://redis.io) is an open source, advanced key-value store.
#   This is the bazel BUILD file for Redis 5.0.4.

package(
    default_visibility = ["//visibility:public"],
    features = [
        "-layering_check",
        "-parse_headers",
    ],
)

licenses(["notice"])  # BSD

exports_files(["LICENSE"])

# Libraries for Redis deps.

cc_library(
    name = "hiredis_lib",
    srcs = ["deps/hiredis/" + filename for filename in [
        "async.c",
        "async.h",
        "dict.c",
        "dict.h",
        "fmacros.h",
        "hiredis.c",
        "hiredis.h",
        "net.c",
        "net.h",
        "read.c",
        "read.h",
        "sds.c",
        "sds.h",
        "sdsalloc.h",
    ]],
    copts = [
        "-std=c99",
        # The definition of macro "__redis_strerror_r" in "hiredis.h" with
        # "GNU_SOURCE" defined has a bug that causes bazel compilation errors
        # when used by "__redisSetErrorFromErrno()" in "net.c".
        # Also the behavior they are trying to correct for "GNU_SOURCE" case
        # does not apply to us even though we specifiy "GNU_SOURCE", so it still
        # results in  the correct functionality.
        "-U_GNU_SOURCE",
    ],
    textual_hdrs = ["deps/hiredis/dict.c"],
)

cc_library(
    name = "linenoise_lib",
    srcs = ["deps/linenoise/" + filename for filename in [
        "linenoise.c",
        "linenoise.h",
    ]],
)

# We use the version of Lua bundled with Redis for now, instead of depending
# on //third_party/lua. We want to make sure Redis users get a consistent
# experience, down to Lua language features. This reflects the intention of
# the Redis developers; see
# http://google3/third_party/redis/v3_2_0/deps/README.md?l=51.
cc_library(
    name = "lua_lib",
    srcs = ["deps/lua/src/" + filename for filename in [
        "fpconv.c",
        "fpconv.h",
        "lapi.c",
        "lapi.h",
        "lauxlib.c",
        "lauxlib.h",
        "lbaselib.c",
        "lcode.c",
        "lcode.h",
        "ldblib.c",
        "ldebug.c",
        "ldebug.h",
        "ldo.c",
        "ldo.h",
        "ldump.c",
        "lfunc.c",
        "lfunc.h",
        "lgc.c",
        "lgc.h",
        "linit.c",
        "liolib.c",
        "llex.c",
        "llex.h",
        "llimits.h",
        "lmathlib.c",
        "lmem.c",
        "lmem.h",
        "loadlib.c",
        "lobject.c",
        "lobject.h",
        "lopcodes.c",
        "lopcodes.h",
        "loslib.c",
        "lparser.c",
        "lparser.h",
        "lstate.c",
        "lstate.h",
        "lstring.c",
        "lstring.h",
        "lstrlib.c",
        "ltable.c",
        "ltable.h",
        "ltablib.c",
        "ltm.c",
        "ltm.h",
        "lua.h",
        "luaconf.h",
        "lualib.h",
        "lua_bit.c",
        "lua_cjson.c",
        "lua_cmsgpack.c",
        "lua_struct.c",
        "lundump.c",
        "lundump.h",
        "lvm.c",
        "lvm.h",
        "lzio.c",
        "lzio.h",
        "print.c",
        "strbuf.c",
        "strbuf.h",
    ]] + ["src/solarisfixes.h"],
    copts = [
        "-Wno-error=empty-body",
        "-Wno-error=implicit-function-declaration",
        "-Wno-error=unused-variable",
        # Needed to enable the cjson Lua module.
        "-DENABLE_CJSON_GLOBAL",
    ],
)

sh_binary(
    name = "mkreleasehdr",
    srcs = ["src/mkreleasehdr.sh"],
)

genrule(
    name = "generate_release_header",
    outs = ["release.h"],
    cmd = """$(location mkreleasehdr);
             mv release.h $@""",
    tools = [":mkreleasehdr"],
)

cc_library(
    name = "redis_lib",
    srcs = ["src/" + filename for filename in [
        "adlist.c",
        "adlist.h",
        "atomicvar.h",
        "quicklist.c",
        "quicklist.h",
        "ae.c",
        "ae.h",
        "anet.c",
        "anet.h",
        "config.h",
        "crc64.h",
        "debugmacro.h",
        "dict.c",
        "dict.h",
        "endianconv.h",
        "sds.h",
        "zmalloc.c",
        "latency.h",
        "listpack.h",
        "lzf.h",
        "lzfP.h",
        "lzf_c.c",
        "lzf_d.c",
        "pqsort.c",
        "pqsort.h",
        "zipmap.c",
        "zipmap.h",
        "sha1.c",
        "sha1.h",
        "ziplist.c",
        "rax.c",
        "rax.h",
        "rax_malloc.h",
        "release.c",
        "networking.c",
        "util.c",
        "object.c",
        "db.c",
        "redisassert.h",
        "replication.c",
        "rdb.c",
        "rdb.h",
        "t_string.c",
        "t_list.c",
        "t_set.c",
        "t_zset.c",
        "t_hash.c",
        "config.c",
        "aof.c",
        "pubsub.c",
        "multi.c",
        "debug.c",
        "sort.c",
        "stream.h",
        "intset.c",
        "intset.h",
        "syncio.c",
        "cluster.c",
        "cluster.h",
        "crc16.c",
        "endianconv.c",
        "slowlog.c",
        "slowlog.h",
        "scripting.c",
        "bio.c",
        "bio.h",
        "rio.c",
        "rio.h",
        "rand.c",
        "rand.h",
        "memtest.c",
        "crc64.c",
        "bitops.c",
        "sentinel.c",
        "notify.c",
        "setproctitle.c",
        "blocked.c",
        "solarisfixes.h",
        "hyperloglog.c",
        "latency.c",
        "server.h",
        "sparkline.c",
        "sparkline.h",
        "redis-check-rdb.c",
        "geo.c",
        "geo.h",
        "geohash.c",
        "geohash.h",
        "geohash_helper.c",
        "geohash_helper.h",
        "util.h",
        "version.h",
        "ziplist.h",
        "zmalloc.h",
    ]] + ["release.h"],
    copts = [
        "-std=c99",
        # "llroundl" is used in "src/hyperloglog.c", but the arguments are all
        # doubles, therefore we use llround instead for it since it's sufficent
        # and we don't currently support "llroundl".
        "-Dllroundl=llround",
    ],
    includes = [
        "deps/hiredis",
        "deps/linenoise",
        "deps/lua/src",
        "src",
    ],
    nocopts = "-Wframe-larger-than=16384",
    textual_hdrs = ["src/ae_select.c"],
    deps = [
        ":hiredis_lib",
        ":linenoise_lib",
        ":lua_lib",
        # Instead of using jemalloc (as stock Redis does), we simply
        # depend on the Google default (tcmalloc). If we need to precisely
        # mirror stock Redis's performance and fragmentation behavior,
        # we should consider adding a dependency on //third_party/jemalloc.
    ],
)

cc_library(
    name = "redis_main",
    srcs = [
        "deps/lua/src/lua.h",
        "deps/lua/src/luaconf.h",
        "src/ae.h",
        "src/anet.h",
        "src/asciilogo.h",
        "src/bio.h",
        "src/childinfo.c",
        "src/cluster.h",
        "src/crc64.h",
        "src/dict.h",
        "src/defrag.c",
        "src/endianconv.h",
        "src/evict.c",
        "src/expire.c",
        "src/fmacros.h",
        "src/intset.h",
        "src/latency.h",
        "src/lazyfree.c",
        "src/listpack.c",
        "src/listpack.h",
        "src/listpack_malloc.h",
        "src/localtime.c",
        "src/lolwut.c",
        "src/lolwut5.c",
        "src/module.c",
        "src/quicklist.h",
        "src/redismodule.h",
        "src/redis-check-aof.c",
        "src/rdb.h",
        "src/rio.h",
        "src/server.c",
        "src/server.h",
        "src/sha1.h",
        "src/siphash.c",
        "src/slowlog.h",
        "src/solarisfixes.h",
        "src/sparkline.h",
        "src/t_stream.c",
        "src/util.h",
        "src/ziplist.h",
        "src/zipmap.h",
    ],
    nocopts = "-Wframe-larger-than=16384",
    deps = [":redis_lib"],
)

cc_library(
    name = "redis_benchmark",
    srcs = ["src/redis-benchmark.c"],
    nocopts = "-Wframe-larger-than=16384",
    deps = [":redis_lib"],
)

cc_library(
    name = "redis_cli",
    srcs = [
        "deps/linenoise/linenoise.h",
        "src/help.h",
        "src/redis-cli.c",
    ],
    nocopts = "-Wframe-larger-than=16384",
    deps = [":redis_lib"],
)

cc_library(
    name = "redis_check_aof",
    srcs = ["src/redis-check-aof.c"],
    nocopts = "-Wframe-larger-than=16384",
    deps = [":redis_lib"],
)
