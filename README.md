CyPB
====

What is CyPB?
-------------

CyPB is fast and light Protocol Buffers decoder for Python. 

 * Lazy Decoding support. Message are decoded on the fly as attributes are accessed for the first time.
   It is big performance gain (~100x in some cases) if only some parts of message are accessed.

 * Fast. Full decoder is twice faster than Google's new C++ wrapper module in PB 2.4

 * No additional dependencies and libraries. 
   Automatically generated C code compiles to Python module.


Usage and example
-----------------

Compile your proto file

    protoc your_proto_file.proto -o your_proto_file.pb2

Generate C module
    
    python generator.py your_proto_file.pb2

Compile C module as Python extension
    
    make compile

Decode messages in Python

    from ctest import PBMsg
    from pb_types import MSG_Contact

    msg = "CiAKBUF0YXNoGghBdGFtdXJhZCINSGV6cmV0a3VsaXlldioUChJhdGFteXJhdEBnbWFpbC5jb20q\nFAoSYXRhbXVyYWRAY29ubmV4LmlvKhQKEmF0YW11cmFkQGdtYWlsLmNvbTIOCgwrOTkzNjc2NDI2\nNDIyDgoMKzk5MzEyMjcwMjAzYh50aGlzIGlzIG5vdGUgZmllbGQgZm9yIGNvbnRhY3Q=\n".decode("base64")

    contact = PBMsg(MSG_Contact, msg)

    # Display first name and list of phone numbers
    print contact.name.first
    for e in contact.phone:
        print " * ", e.display_number

Full Decode Benchmark
---------------------

For benchmarking, only full decode method is used instead of lazy decoder. 

Comparison were done on decoding same message 5000 times. 
See connexio.proto and run_google.py for test message and structure.

    Environment:
     * MacBook Pro 2.66Ghz Intel Core 2 Duo
     * Python version: 2.6.1
     * GCC 4.2.1

    Average running time on 10 runs:
     * Google's Python module (2.3): 0.711271 seconds.
     * Google's C++ implementation for Python (2.4): 0.093410
     * CyPB: 0.041584 seconds.


TODO
----

CyPB is in alpha stage and API is highly likely to change in the future. 
Below is the list of things to do, patches are welcome.

 * Support all data types: doubles, bytes, etc.
 * Handle unknown fields properly based on wire-type
 * Throw exception if invalid attribute is accessed
 * Merge Lazy and Full decoder into one API.
 * Encoder

About / License
---------------

CyPB is developed by [connex.io gmbh][connexio] and licensed under new BSD license. 

[connexio]: http://connex.io/


