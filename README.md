Usage
=====

    $ make
    $ dd if=/dev/zero of=X bs=256M count=1
    $ ./write-bench <X >Y   # Same as "cp X Y"

Notes
=====

* The benchmark sleeps 0.1 seconds between all three runs
* Best run is displayed in the first column
* The file is read sequentially once prior to the benchmark
* Benchmark only measures the time to copy data to stdout
    * In particular, depending on filesystem, the next `ftruncate(fd, 0)` may take very long
    * No `fsync()` or `fdatasync()` is performed

Some Results
============

Ext3+dmcrypt, SSD, filesize=256MB, kernel=3.13-1-amd64
------------------------------------------------------

    pipe+splice + advices + trunc                 371ms      385ms     2704ms
    sendfile + advices + trunc                    380ms      528ms     2910ms  (+ 2.4%)
    read+write 16bs + advices                     384ms      414ms      425ms  (+ 3.5%)
    pipe+splice                                   389ms      403ms     1631ms  (+ 4.9%)
    sendfile                                      390ms      410ms      663ms  (+ 5.1%)
    read+write 256bs                              393ms      634ms      806ms  (+ 5.9%)
    sendfile + advices                            401ms      408ms      446ms  (+ 8.1%)
    pipe+splice + advices                         403ms      416ms      417ms  (+ 8.6%)
    read+write 16bs                               405ms      537ms      896ms  (+ 9.2%)
    read+write 4bs                                413ms      427ms      638ms  (+ 11.3%)
    mmap+write + advices + trunc                  414ms      449ms      755ms  (+ 11.6%)
    mmap+write + advices                          432ms      458ms      740ms  (+ 16.4%)
    mmap+write                                    438ms      444ms      846ms  (+ 18.1%)
    read+write bs                                 469ms      538ms      901ms  (+ 26.4%)
    read+write 1k                                 639ms      672ms      866ms  (+ 72.2%)
    read+write 16bs + advices + trunc            1007ms     1500ms     4062ms  (+ 171.4%)
    sendfile + advices + falloc                  1381ms     1486ms     1742ms  (+ 272.2%)
    mmap+write + advices + falloc                1405ms     1426ms     1576ms  (+ 278.7%)
    read+write 16bs + advices + falloc           1446ms     1524ms     1683ms  (+ 289.8%)
    pipe+splice + advices + falloc               1863ms     1909ms     1950ms  (+ 402.2%)

XFS, SSD, filesize=1GB, kernel=2.6.32-5-amd64
---------------------------------------------

    sendfile + advices + falloc                   398ms      405ms      406ms
    pipe+splice + advices + falloc                407ms      409ms      409ms  (+ 2.3%)
    sendfile                                      445ms      448ms      459ms  (+ 11.8%)
    sendfile + advices                            447ms      448ms      450ms  (+ 12.3%)
    pipe+splice                                   459ms      460ms      462ms  (+ 15.3%)
    pipe+splice + advices                         460ms      460ms      467ms  (+ 15.6%)
    read+write 16bs + advices + falloc            464ms      465ms      466ms  (+ 16.6%)
    sendfile + advices + trunc                    468ms      471ms      476ms  (+ 17.6%)
    pipe+splice + advices + trunc                 471ms      472ms      476ms  (+ 18.3%)
    mmap+write + advices + falloc                 482ms      483ms      485ms  (+ 21.1%)
    read+write 256bs                              504ms      508ms      512ms  (+ 26.6%)
    read+write 16bs                               505ms      506ms      507ms  (+ 26.9%)
    read+write 16bs + advices                     511ms      515ms      528ms  (+ 28.4%)
    read+write 4bs                                524ms      538ms      541ms  (+ 31.7%)
    mmap+write + advices                          530ms      530ms      544ms  (+ 33.2%)
    mmap+write                                    530ms      533ms      533ms  (+ 33.2%)
    read+write 16bs + advices + trunc             537ms      540ms      543ms  (+ 34.9%)
    mmap+write + advices + trunc                  557ms      563ms      564ms  (+ 39.9%)
    read+write bs                                 630ms      638ms      638ms  (+ 58.3%)
    read+write 1k                                1138ms     1156ms     1168ms  (+ 185.9%)

XFS, Hard Drive, filesize=1G, kernel=2.6.32-5-amd64
---------------------------------------------------

    pipe+splice + advices + falloc                580ms      597ms      706ms
    sendfile + advices + falloc                   591ms      600ms      710ms  (+ 1.9%)
    pipe+splice + advices                         624ms      699ms      766ms  (+ 7.6%)
    sendfile + advices                            627ms      658ms      676ms  (+ 8.1%)
    pipe+splice                                   650ms      748ms      887ms  (+ 12.1%)
    sendfile                                      653ms      658ms      693ms  (+ 12.6%)
    read+write 16bs + advices + falloc            683ms      688ms      689ms  (+ 17.8%)
    sendfile + advices + trunc                    685ms      686ms      825ms  (+ 18.1%)
    pipe+splice + advices + trunc                 691ms      720ms      723ms  (+ 19.1%)
    read+write 16bs                               718ms      742ms      753ms  (+ 23.8%)
    mmap+write + advices + falloc                 733ms      733ms      820ms  (+ 26.4%)
    read+write 256bs                              737ms      741ms      746ms  (+ 27.1%)
    read+write 16bs + advices                     746ms      754ms      758ms  (+ 28.6%)
    read+write 4bs                                759ms      774ms      817ms  (+ 30.9%)
    read+write 16bs + advices + trunc             774ms      775ms      780ms  (+ 33.4%)
    mmap+write                                    786ms      798ms      799ms  (+ 35.5%)
    mmap+write + advices + trunc                  814ms      831ms      948ms  (+ 40.3%)
    mmap+write + advices                          815ms      845ms      927ms  (+ 40.5%)
    read+write bs                                 856ms      889ms      905ms  (+ 47.6%)
    read+write 1k                                1482ms     1497ms     1534ms  (+ 155.5%)

tmpfs /dev/shm, filesize=256MB, kernel=3.13-1-amd64
---------------------------------------------------

    sendfile + advices + trunc                    132ms      133ms      133ms
    sendfile                                      132ms      132ms      133ms  (+ 0.0%)
    sendfile + advices                            133ms      134ms      134ms  (+ 0.8%)
    pipe+splice                                   133ms      134ms      134ms  (+ 0.8%)
    pipe+splice + advices + trunc                 134ms      134ms      134ms  (+ 1.5%)
    pipe+splice + advices                         134ms      134ms      135ms  (+ 1.5%)
    read+write 16bs + advices + trunc             135ms      136ms      137ms  (+ 2.3%)
    read+write 16bs + advices                     135ms      136ms      136ms  (+ 2.3%)
    read+write 16bs                               135ms      135ms      135ms  (+ 2.3%)
    read+write 256bs                              137ms      137ms      138ms  (+ 3.8%)
    read+write 4bs                                139ms      139ms      140ms  (+ 5.3%)
    sendfile + advices + falloc                   140ms      140ms      141ms  (+ 6.1%)
    pipe+splice + advices + falloc                142ms      142ms      143ms  (+ 7.6%)
    read+write 16bs + advices + falloc            145ms      145ms      145ms  (+ 9.8%)
    read+write bs                                 154ms      155ms      155ms  (+ 16.7%)
    mmap+write + advices                          169ms      170ms      170ms  (+ 28.0%)
    mmap+write                                    169ms      170ms      170ms  (+ 28.0%)
    mmap+write + advices + trunc                  170ms      170ms      174ms  (+ 28.8%)
    mmap+write + advices + falloc                 175ms      176ms      178ms  (+ 32.6%)
    read+write 1k                                 244ms      245ms      258ms  (+ 84.8%)

NFS, filesize=1G, kernel=2.6.32-5-amd64
---------------------------------------

    pipe+splice                                   448ms      508ms      564ms
    sendfile                                      485ms      496ms      501ms  (+ 8.3%)
    sendfile + advices                            495ms      508ms      569ms  (+ 10.5%)
    sendfile + advices + trunc                    497ms      524ms      526ms  (+ 10.9%)
    pipe+splice + advices                         512ms      528ms      542ms  (+ 14.3%)
    pipe+splice + advices + trunc                 515ms      563ms      734ms  (+ 15.0%)
    sendfile + advices + falloc                   546ms      596ms    18629ms  (+ 21.9%)
    read+write 16bs + advices + trunc             548ms      578ms      626ms  (+ 22.3%)
    read+write 4bs                                560ms      583ms      649ms  (+ 25.0%)
    read+write 16bs + advices                     561ms      597ms      599ms  (+ 25.2%)
    pipe+splice + advices + falloc                580ms      589ms      590ms  (+ 29.5%)
    read+write 16bs                               605ms      610ms      638ms  (+ 35.0%)
    mmap+write + advices                          610ms      637ms      807ms  (+ 36.2%)
    read+write 16bs + advices + falloc            614ms      652ms      652ms  (+ 37.1%)
    read+write bs                                 621ms      624ms      643ms  (+ 38.6%)
    mmap+write                                    637ms      640ms      657ms  (+ 42.2%)
    mmap+write + advices + trunc                  639ms      655ms      658ms  (+ 42.6%)
    mmap+write + advices + falloc                 667ms      672ms      699ms  (+ 48.9%)
    read+write 256bs                              686ms      688ms      713ms  (+ 53.1%)
    read+write 1k                                1638ms     1717ms    19303ms  (+ 265.6%)

(Whole benchmark very slow because of synces: `0.16s user 47.74s system 2% cpu 30:21.46 total`.)
