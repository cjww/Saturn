
SHADER_DIR = ./shaders
VERT_SHADERS := $(wildcard $(SHADER_DIR)/*.vert)
FRAG_SHADERS := $(wildcard $(SHADER_DIR)/*.frag)
COMP_SHADERS := $(wildcard $(SHADER_DIR)/*.comp)
GEOM_SHADERS := $(wildcard $(SHADER_DIR)/*.geom)

SHADERS := $(VERT_SHADERS:%.vert=%.vert.spv) $(FRAG_SHADERS:%.frag=%.frag.spv) $(COMP_SHADERS:%.comp=%.comp.spv) $(GEOM_SHADERS:%.geom=%.geom.spv)

all: shaders

shaders: $(SHADERS)
	@echo "Done"

%.vert.spv: %.vert
	@echo "compiling $<"
	glslangValidator -V $< -o $@ 

%.frag.spv: %.frag
	@echo "compiling $<"
	glslangValidator -V $< -o $@ 

%.comp.spv: %.comp
	@echo "compiling $<"
	glslangValidator -V $< -o $@ 

%.geom.spv: %.geom
	@echo "compiling $<"
	glslangValidator -V $< -o $@ 

clean:
	rm $(SHADER_DIR)/*.spv
