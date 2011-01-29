
compile: ctest.o
	gcc -shared  -undefined dynamic_lookup ctest.o -o ctest.so 

ctest.o: ctest.c
	gcc -c ctest.c -I/System/Library/Frameworks/Python.framework/Versions/2.6/include/python2.6 -O3

clean:
	rm ctest.o
	rm ctest.so

test-generate:
	protoc test.proto -o test.pb2
	python generator.py test.pb2


