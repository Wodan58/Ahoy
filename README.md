Ahoy
----

Build|Linux|Coverity
---|---|---
status|[![GitHub CI build status](https://github.com/Wodan58/Ahoy/actions/workflows/c-cpp.yml/badge.svg)](https://github.com/Wodan58/Ahoy/actions/workflows/c-cpp.yml)|[![Coverity Scan Build Status](https://img.shields.io/coverity/scan/22424.svg)](https://scan.coverity.com/projects/wodan58-ahoy)

This is an assembly version of [42minjoy](https://github.com/Wodan58/42minjoy).
Linux, 64 bit. This exists for one reason only: how much smaller is an assembly
version of a C program?

The answer to this question: 18320 bytes for this version versus 47320 bytes
for 42minjoy. These numbers come from GCC 11.4 under Ubuntu 22.04 run from WSL.
