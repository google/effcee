workspace(name = "effcee")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "rules_cc",
    strip_prefix = "rules_cc-main",
    urls = ["https://github.com/bazelbuild/rules_cc/archive/main.zip"],
)

http_archive(
    name = "com_google_googletest",
    strip_prefix = "googletest-release-1.10.0",
    urls = ["https://github.com/google/googletest/archive/release-1.10.0.zip"],
    sha256 = "94c634d499558a76fa649edb13721dce6e98fb1e7018dfaeba3cd7a083945e91",
)

http_archive(
    name = "com_googlesource_code_re2",
    strip_prefix = "re2-2022-06-01",
    urls = ["https://github.com/google/re2/archive/2022-06-01.zip"],
    sha256 = "9f3b65f2e0c78253fcfdfce1754172b0f97ffdb643ee5fd67f0185acf91a3f28",
)
