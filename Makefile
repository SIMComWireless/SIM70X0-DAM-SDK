###设置默认OS
OS ?= LINUX
PROJ ?= app

###设置编译器等工具的路径
include ./tools/opt.mak
include ./src/$(PROJ)/cust.mak

###设置一些全局的变量
#empty         :=
#space         := $(empty) $(empty)
#INCLUDE_SIG   := $(sapce)-I$(sapce)
MAKE          := make
RM            := -rm -rf 
MV            := -mv
DAM_RO_BASE   := 0x40000000

###设置编译输出文件的路径
DIR_OBJ_PATH               := ./build
DIR_BIN_PATH               := ./bin
SENSOR_DAM_DEMO_LD_PATH    := ./src/build/cust_app.ld
DEMO_APP_SRC_PATH          := ./src/$(PROJ)
DEMO_APP_OUTPUT_PATH       := ./src/$(PROJ)/build
DAM_TARGET_NAME            := cust_app.bin
DAM_ELF_NAME               := $(DIR_BIN_PATH)/cust_app.elf
DAM_MAP_NAME               := $(DIR_BIN_PATH)/cust_app.map
DAM_TARGET_BIN             := $(DIR_BIN_PATH)/cust_app.bin
DAM_SRC_PATH               := ./src/$(PROJ)
DAM_LIB_PATH               := ./libs

###设置编译器
CC                            := $(LLVMTOOLCHAIN_PATH)/$(LLVMTOOLCHAIN)
LD                            := $(LLVMTOOLCHAIN_PATH)/$(LLVMLINKTOOL)
TOOLCHAIN_STANDARDS_INCLUDE   := $(LLVMTOOLCHAIN_PATH_STANDARDS)
TOOLCHAIN_LIBS_PATH           := $(LLVMLIB)
PREAMBLE_S_NAME               := txm_module_preamble_llvm.S
PREAMBLE_S_PP_NAME            := txm_module_preamble_llvm_pp.S
PREAMBLE_S_OBJ_NAME           := txm_module_preamble_llvm.o
LD_FLAGS                      := -d -o $(DAM_ELF_NAME) -target armv7m-none-musleabi -fuse-ld=qcld -lc++ -Wl,-mno-unaligned-access \
	-fuse-baremetal-sysroot -fno-use-baremetal-crt -Wl,-entry=$(DAM_RO_BASE) $(DEMO_APP_OUTPUT_PATH)/$(PREAMBLE_S_OBJ_NAME) \
	-Wl,-T$(SENSOR_DAM_DEMO_LD_PATH) -Wl,-Map=$(DAM_MAP_NAME) -Wl,-gc-sections 
DIR_SRC_S_FILE_PATH           := ./src/$(PROJ)/asm ./include/src



###设置头文件及.c文件的路径
DIR_INC         := $(TOOLCHAIN_STANDARDS_INCLUDE) ./include/qapi ./include/qmi \
           					./include/threadx_api ./include
DIR_SRC         := $(DIR_SRC_S_FILE_PATH) 

DIR_INC         += $(CUST_INCLUDE_PATH)
DIR_SRC         += $(CUST_SRC_PATH)

X_OUT           := ./bin
X_NAME          := $(patsubst %, $(X_OUT)/%, $(DAM_TARGET_NAME))
X_INCDIRS       := $(patsubst %, -I %, $(DIR_INC))
X_SRCDIRS       := $(patsubst %, %, $(DIR_SRC))
X_OBJDIRS       := $(patsubst %, $(DIR_OBJ_PATH)/%, $(X_SRCDIRS))

X_CFILES        := $(foreach dir, $(X_SRCDIRS), $(wildcard $(dir)/*.c))
X_SFILES        := $(foreach dir, $(X_SRCDIRS), $(wildcard $(dir)/*.S))
X_CDEPS         := $(patsubst %, $(DIR_OBJ_PATH)/%,$(X_CFILES:.c=.o))
X_SDEPS         := $(patsubst %, %, $(X_SFILES:.S=.o))

X_DEPS          := $(X_CDEPS) $(X_SDEPS)

X_COBJS         := $(patsubst %, %, $(X_CFILES:.c=.o))
X_SOBJS         := $(patsubst %, %, $(X_SFILES:.S=.o))
X_OBJS          := $(X_COBJS) $(X_SOBJS)

###设置编译选项
DAM_CPPFLAGS    := -DQAPI_TXM_MODULE -DTXM_MODULE -DTX_DAM_QC_CUSTOMIZATIONS -DTX_ENABLE_PROFILING -DTX_ENABLE_EVENT_TRACE \
                   -DTX_DISABLE_NOTIFY_CALLBACKS  -DFX_FILEX_PRESENT -DTX_ENABLE_IRQ_NESTING  -DTX3_CHANGES -DQAPI_TXM_MODULE
DAM_CFLAGS      := -marm -target armv7m-none-musleabi -mfloat-abi=softfp -mfpu=none -mcpu=cortex-a7 \
                   -mno-unaligned-access  -fms-extensions -Osize -fshort-enums -Wbuiltin-macro-redefined

DAM_CPPFLAGS    += $(CUST_DEF_FLAGS)

.PHONY: all clean
all:$(X_NAME)

$(X_NAME) : $(X_OBJS) 
	@echo "Start linking..."
	@$(RM) build.log
	@$(LD) $(LD_FLAGS) $(filter-out $(DEMO_APP_OUTPUT_PATH)/$(PREAMBLE_S_OBJ_NAME),$(foreach dir, $(DEMO_APP_OUTPUT_PATH), $(wildcard $(dir)/*.o))) $(DAM_LIB_PATH)/*.lib  >> build.log 2>&1
	@$(PYTHON_PATH) $(LLVMLINK_PATH)/llvm-elf-to-hex.py --bin $(DAM_ELF_NAME) --output $@ >>  build.log 2>&1
	@echo "linking complete"
	@echo "Demo application is built at" $(DIR_OBJ_PATH)/$(DAM_TARGET_NAME)

$(X_COBJS) : %.o : %.c
	@echo "CC $<"
	@$(CC) -c $(DAM_CPPFLAGS) $(DAM_CFLAGS) $(X_INCDIRS) $< -o $@ >> build.log 2>&1
	@$(MV) $@ $(DEMO_APP_OUTPUT_PATH)/

$(X_SOBJS) : %.o : %.S
	@echo "CC $<"
	@$(RM) $(PREAMBLE_S_PP_NAME)
	@$(CC) -E $(DAM_CPPFLAGS) $(DAM_CFLAGS) $< > $(PREAMBLE_S_PP_NAME) 
	@$(CC) -c $(DAM_CPPFLAGS) $(DAM_CFLAGS) $(PREAMBLE_S_PP_NAME) -o $@ >> build.log 2>&1
	@$(MV) $@ $(DEMO_APP_OUTPUT_PATH)/
	@$(RM) $(PREAMBLE_S_PP_NAME)


clean:
	$(RM) build.log
	$(RM) $(DEMO_APP_OUTPUT_PATH)/*.o
	$(RM) $(DAM_ELF_NAME) $(DAM_MAP_NAME) $(DAM_TARGET_BIN)

