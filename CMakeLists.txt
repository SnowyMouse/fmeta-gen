# SPDX-License-Identifier: GPL-3.0-only

# Require this
cmake_minimum_required(VERSION 3.12)

# Project
project(fmeta-gen
    VERSION 0.1.0
    DESCRIPTION "Generates fmeta files for the uncompressed file size"
    LANGUAGES C
)

# Patcher
add_executable(fmeta-gen
    main.c
)
