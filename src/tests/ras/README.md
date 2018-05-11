Reliability, Availability and Serviceability (RAS) tests
=================================
RAS tests cover PMDK features utilizing NVDIMMs RAS mechanisms. These tests
need to be run on machine with NVDIMM hardware described by
appropriate [config.xml](../../../etc/config/README.md) file.

### Unsafe shutdown tests ###
Unsafe shutdown tests (compiled into ```UNSAFE_SHUTDOWN``` binary) involve
machine power cycle during test. Because of that, these tests need to be run with use
of remote machine that can connect through ssh, without being prompted for
authentication, with root access user on test machine. ```US_TEST_CONTROLLER```
is utilized for this purpose. To run full scope of unsafe shutdown tests:

```
$ ./US_TEST_CONTROLLER user@address /path/on/remote/to/UNSAFE_SHUTDOWN
```

### Dependencies ###
* [ndctl](https://github.com/pmem/ndctl) - version 60.0 or greater
