
SHADER_DIR = .
VERT_SHADERS := $(wildcard $(SHADER_DIR)/*.vert)
GEOM_SHADERS := $(wildcard $(SHADER_DIR)/*.geom)
FRAG_SHADERS := $(wildcard $(SHADER_DIR)/*.frag)
COMP_SHADERS := $(wildcard $(SHADER_DIR)/*.comp)
SHADERS := $(VERT_SHADERS:%=%.spv) $(GEOM_SHADERS:%=%.spv) $(FRAG_SHADERS:%=%.spv) $(COMP_SHADERS:%=%.spv)

all: shaders

shaders: $(SHADERS)
	@echo "Done"

%.spv: %
	@echo "compiling $<"
	glslangValidator -V $< -o $@ 

clean:
	rm *.spv
