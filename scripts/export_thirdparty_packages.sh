#!/bin/bash
pushd ./3rdparty

conan create ./volk morph/dependencies

popd
