SHELL=bash

SHADER_DIR = .
VERT_SHADERS := $(wildcard $(SHADER_DIR)/*.vert)
GEOM_SHADERS := $(wildcard $(SHADER_DIR)/*.geom)
FRAG_SHADERS := $(wildcard $(SHADER_DIR)/*.frag)
COMP_SHADERS := $(wildcard $(SHADER_DIR)/*.comp)
TESC_SHADERS := $(wildcard $(SHADER_DIR)/*.tesc)
TESE_SHADERS := $(wildcard $(SHADER_DIR)/*.tese)

SHADERS := $(VERT_SHADERS:%=%.spv) $(GEOM_SHADERS:%=%.spv) $(FRAG_SHADERS:%=%.spv) $(COMP_SHADERS:%=%.spv) $(TESC_SHADERS:%=%.spv) $(TESE_SHADERS:%=%.spv)

all: shaders

shaders: $(SHADERS)
	@echo "Done"

%.spv: %
	@echo "compiling $<"
	glslangValidator -V $< -o $@ 

clean:
	rm *.spv
