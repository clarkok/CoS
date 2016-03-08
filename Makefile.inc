PROJECT_PATH=/home/c/c-stack/SoC

UTILS_DIR=$(PROJECT_PATH)/utils

CC=clang-3.8 -O3 -emit-llvm -c -Wall -nodefaultlibs -Wno-gcc-compat -Wno-unused-function -Wno-incompatible-library-redeclaration -nostdlib -m32 -isystem $(PROJECT_PATH)/software/libc/ -I .
LK=llvm-link-3.8
ASW=llc-3.8 -O3 -relocation-model=static -disable-mips-delay-filler -march=mipsel 
ASWPIC=llc-3.8 -O3 -relocation-model=pic -disable-mips-delay-filler -march=mipsel 
AS=$(UTILS_DIR)/nt-asm/nt-asm
BIN2HEX=$(UTILS_DIR)/nt-asm/bin2hex.sh
BIN2COE=$(UTILS_DIR)/nt-asm/bin2coe.sh
DEP=$(CC) -MM -MF $(DEP_DIR)$$(addsuffix .d,$$(basename $$(subst /,-,$$<)))

UTILS_CC=clang-3.8 -Wall -m32 -g
TEST_CC=clang-3.8 -Wall -m32 -g -I .

BASENAME=$(basename $(SOURCES))
OBJECTS=$(addsuffix .bc, $(addprefix $(OBJ_DIR),$(subst /,-,$(BASENAME))))
BASE_PIC=$(basename $(PIC_SRCS))
OBJ_PIC=$(addsuffix .bc, $(addprefix $(OBJ_DIR),$(subst /,-,$(BASE_PIC))))

define COMPILE_MACRO
$(OBJ_DIR)$(subst /,-,$(basename $(1)).bc):$(SRC_DIR)$(1)
	$(CC) -o $$@ $$<
	@$(DEP) $$<
	@sed -e 's#\s*[-_a-zA-Z/]*\.o#$$@#g'\
		< $(DEP_DIR)$$(addsuffix .d,$$(basename $$(subst /,-,$$<))) \
	   	> $(DEP_DIR)$$(addsuffix .P,$$(basename $$(subst /,-,$$<)))
endef
