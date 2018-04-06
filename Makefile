build: build_fmm build_ubodt_gen
install: 
	@echo ----- Copy executables to home/bin ----- 
	cp dist/fmm $(HOME)/bin
	cp dist/ubodt_gen $(HOME)/bin
	@echo ----- Finish -----
initial:
	mkdir -p dist
	mkdir -p $(HOME)/bin
build_fmm:initial
	@echo ----- Start to build fmm -----
	g++ -Wall -std=gnu++11  -O3 app/fmm.cpp -o dist/fmm -lgdal -lboost_serialization
	@echo ----- fmm build success ----- 
build_install_fmm_debug:initial
	@echo ----- Start to build fmm_debug -----
	g++ -std=gnu++11 -DCPC_DEBUG_LEVEL='1' -O3 app/fmm.cpp -o dist/fmm_debug -lgdal -lboost_serialization
	@echo ----- fmm_debug build success ----- 
	@echo ----- Copy debug executables to home/bin ----- 
	cp dist/fmm_debug $(HOME)/bin
	@echo ----- Finish -----
build_ubodt_gen:initial
	@echo ----- Start to build ubodt_gen ----- 
	g++ -std=gnu++11  -O3 app/ubodt_gen.cpp -o dist/ubodt_gen -lgdal -lboost_serialization
	@echo ----- ubodt_gen build success ----- 
build_install_ubodt_gen_debug:initial
	@echo ----- Start to build ubodt_gen_debug ----- 
	g++ -std=gnu++11 -DDEBUG_LEVEL='2' -O3 app/ubodt_gen.cpp -o dist/ubodt_gen_debug -lgdal -lboost_serialization
	cp dist/ubodt_gen_debug $(HOME)/bin
	@echo ----- ubodt_gen build success ----- 
clean:
	rm dist/*
	rm $(HOME)/bin/fmm
	rm $(HOME)/bin/fmm_debug
	rm $(HOME)/bin/ubodt_gen
	rm $(HOME)/bin/ubodt_gen_debug
	rm $(HOME)/bin/ubodt_gen_opt
	