all:ubodt2binary binary2ubodt
ubodt2binary:
	mkdir -p bin
	g++ -w -O3 -g ubodt2binary.cpp -o bin/ubodt2binary -lboost_serialization
	cp bin/ubodt2binary ~/bin
binary2ubodt:
	mkdir -p bin
	g++ -w -O3 -g binary2ubodt.cpp -o bin/binary2ubodt -lboost_serialization
	cp bin/binary2ubodt ~/bin