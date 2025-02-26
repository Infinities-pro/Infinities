# python-infinity

# update python client

- Update "version" field of [project] chapter and client_version field of ConnectRequest message.
- build new python SDK
- upload to pypi.org
- install new python SDK

Please see [releases.yml](https://github.com/infiniflow/infinity/blob/main/.github/workflows/release.yml) for details.

# using

```python
import infinity
from infinity.common import LOCAL_HOST
from infinity.common import ConflictType

infinity_obj = infinity.connect(LOCAL_HOST)
db = infinity_obj.get_database("default_db")
db.drop_table("my_table", ConflictType.Ignore)
table = db.create_table("my_table", {"num": {"type": "integer"}, "body": {"type": "varchar"}, "vec": {"type": "vector,5,float"}}, ConflictType.Error)
table.insert([{"num": 1, "body": "undesirable, unnecessary, and harmful", "vec": [1.0] * 5}])
table.insert([{"num": 2, "body": "publisher=US National Office for Harmful Algal Blooms", "vec": [4.0] * 5}])
table.insert([{"num": 3, "body": "in the case of plants, growth and chemical", "vec": [7.0] * 5}])

res = table.output(["*"]).match_dense("vec", [3.0] * 5, "float", "ip", 2).to_pl()
print(res)

```


# For developer
```shell
pip install -e .
```
Build infinity-sdk 
```shell
pip install ./python/infinity_sdk 
```
Build the release version of infinity-embedded-sdk in the target location `cmake-build-release`
```shell
pip install . -v --config-settings=cmake.build-type="Release"  --config-settings=build-dir="cmake-build-release"
```
Build the debug version of infinity-embedded-sdk in the target location `cmake-build-debug`
```shell
pip install . -v --config-settings=cmake.build-type="Debug"  --config-settings=build-dir="cmake-build-debug"
```
Note: If you run with the release version and turn jemalloc compile flag on, you must set environment variable, for example
```shell
LD_PRELOAD=$(ldconfig -p | grep 'libjemalloc.so ' | awk '{print $4}') python3 example/simple_example.py
```
Note: If you run with the debug version, you must set the **libasan** environment variable, for example
```shell
LD_PRELOAD=$(find $(clang-18 -print-resource-dir) -name "libclang_rt.asan-x86_64.so") python3 example/simple_example.py
```
Note: When running with the debug version infinity_embedded-sdk, you may find some memory leaks caused by arrow. You can use `ASAN_OPTIONS=detect_leaks=0` to disable memory leak detection, for example
```shell
LD_PRELOAD=$(find $(clang-18 -print-resource-dir) -name "libclang_rt.asan-x86_64.so") ASAN_OPTIONS=detect_leaks=0 python3 example/simple_example.py
```

# run pysdk test
Run a local infinity test in project root directory
```shell
pytest --local-infinity python/test/cases/test_basic.py::TestInfinity::test_basic
```
Run a remote infinity test in project root directory
```shell
pytest python/test/cases/test_basic.py::TestInfinity::test_basic
```