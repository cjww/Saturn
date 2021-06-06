
SHADER_DIR = ./shaders
VERT_SHADERS := $(wildcard $(SHADER_DIR)/*.vert)
FRAG_SHADERS := $(wildcard $(SHADER_DIR)/*.frag)
COMP_SHADERS := $(wildcard $(SHADER_DIR)/*.comp)
SHADERS := $(VERT_SHADERS:%.vert=%.spv) $(FRAG_SHADERS:%.frag=%.spv) $(COMP_SHADERS:%.comp=%.spv)

all: shaders

shaders: $(SHADERS)
	@echo "Done"

%.spv: %.vert
	@echo "compiling $<"
	glslangValidator -V $< -o $@ 

%.spv: %.frag
	@echo "compiling $<"
	glslangValidator -V $< -o $@ 

%.spv: %.comp
	@echo "compiling $<"
	glslangValidator -V $< -o $@ 

clean:
	rm *.spv
