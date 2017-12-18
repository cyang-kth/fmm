initial:
	mkdir -p dist
	mkdir -p $(HOME)/bin
build_fmm:initial
	@echo ----- Start to build fmm -----
	g++ -std=gnu++11  -O3 app/fmm.cpp -o dist/fmm -lgdal
	@echo ----- fmm build success ----- 
build_fmm_debug:initial
	@echo ----- Start to build fmm_debug -----
	g++ -std=gnu++11 -DDEBUG_LEVEL='2' -O3 app/fmm.cpp -o dist/fmm_debug -lgdal
	@echo ----- fmm_debug build success ----- 
build_ubodt_gen:initial
	@echo ----- Start to build ubodt_gen ----- 
	g++ -std=gnu++11  -O3 app/ubodt_gen.cpp -o dist/ubodt_gen -lgdal
	@echo ----- ubodt_gen build success ----- 
build: build_fmm build_ubodt_gen
install: build
	@echo ----- Copy executables to home/bin ----- 
	cp dist/fmm $(HOME)/bin
	cp dist/ubodt_gen $(HOME)/bin
	@echo ----- Finish ----- 
clean:
	rm dist/*
	