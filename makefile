CC ?= gcc

DEBUG?=
PC_DEBUG?=

#select platform version,default is PlatformV2
#using PLATFORM_VER = V2 to select platform,like:
# "make PLATFORM_VER=V2" or "make clean PLATFORM_VER=V2" and so on
PLATFORM_VER ?= V2

#OS detecting
ifeq ($(OS),Windows_NT)
    DETECTED_OS := Windows
    FILE_SUFFIX := .exe
    COMMON_ADD_DIR = $(shell if not exist "$(dir $@)" md "$(dir $@)")
    COMMON_CLEAN = $(shell if exist "$(BUILD_PATH_BASE)" rd /s /q "$(BUILD_PATH_BASE)")
    #like -L"D:/boost_1_65_1/stage/lib/" -L"D:/googletest/googletest/build/"
    LIB_PATH =
    #-lboost_system-mgw49-mt-1_65_1 \
	   -lboost_system-mgw49-mt-d-1_65_1 \
	   -lws2_32 -lgtest -lwsock32
    LIBS =
	#Like -I"D:/boost_1_65_1" -I"D:/googletest/googletest/include/"
    INCLUDE_PATH =

else
    DETECTED_OS := $(shell uname -s)
    FILE_SUFFIX := .elf
    COMMON_ADD_DIR = $(shell mkdir -p "$(dir $@)" )
    COMMON_CLEAN = $(shell rm -rf "$(BUILD_PATH_BASE)")
	# -lboost_system -lgtest -lpthread -lm
    LIBS =
endif

$(info The system is $(DETECTED_OS))

#Debug or Release ,default is Release
#using VERSION=Debug to used Debug mode,like:
# "make MODE=Debug" or "make clean MODE=Debug" and so on
MODE ?= Release
VER_FLAG =

CXXFLAGS = -std=c++11 -MMD
CFLAGS = -MMD

ifeq ($(DEBUG),1)
CXXFLAGS += -D_DEBUG_EN
CFLAGS += -D_DEBUG_EN
endif
ifeq ($(PC_DEBUG),1)
CXXFLAGS += -D_DEBUG_ON_PC
CFLAGS += -D_DEBUG_ON_PC
endif

DEBUG_FLAGS ?= -ggdb -Wall -O0
RELEASE_FLAGS ?= -Wall -Os

BUILD_PATH_BASE = build
BUILD_PATH =
SRC_PATH = src
OBJ_PATH = $(BUILD_PATH)/obj

#output config here
PRO_NAME := socket_manager_test
OUT_EXEC := $(PRO_NAME)$(FILE_SUFFIX)
# OUTPUT_BASE = $(BUILD_PATH)/$(OUT_EXEC)
OUTPUT_BASE = $(OUT_EXEC)

#output debug config
OUT_EXEC_DEBUG := $(patsubst %.out,%d.out,$(OUT_EXEC))
OUTPUT_DEBUG := $(BUILD_PATH_DEBUG)/$(OUT_EXEC_DEBUG)


#library include head like -I/usr/local/include
INCLUDE_PATH ?=

#library like -L/usr/local/lib
LIB_PATH ?=

#library like -lgtest or -l:gtest.a
LIBS = -lpthread -ldl -lstdc++ -lm

#all path config here
ifeq ($(OS),Windows_NT)
    LOCAL_DIRS := $(shell cd)
    DIRS :=$(LOCAL_DIRS)\$(SRC_PATH) $(shell dir "$(SRC_PATH)" /s/b/o:n/A:D)
    DIRS := $(subst $(LOCAL_DIRS)\,, $(DIRS))
    DIRS := $(subst \,/, $(DIRS))
else
    DIRS := $(shell find $(SRC_PATH) -maxdepth 8 -type d)
endif
DIRS := $(patsubst %, %/, $(DIRS))

ifeq ($(PC_DEBUG),1)
    IGNORE = src/dev/drivers/ src/dev/drivers/platform_1/ src/dev/drivers/platform_2/
    DIRS := $(filter-out $(IGNORE), $(DIRS))
else
    ifeq ($(PLATFORM_VER),V2)
        IGNORE = src/dev/drivers/platform_1/
    else
        IGNORE = src/dev/drivers/platform_2/
    endif
    DIRS := $(filter-out $(IGNORE), $(DIRS))
endif

# add source path to local include path & combine to INCLUDE_PATH
# LOCAL_INC = $(DIRS)
LOCAL_INC = src/include/
# INCLUDE_PATH += $(foreach i,$(LOCAL_INC),-I$i)
INCLUDE_PATH += -I$(LOCAL_INC)

FILES_CPP := $(foreach dir,$(DIRS),$(wildcard $(dir)*.cpp))
FILES_C := $(foreach dir,$(DIRS),$(wildcard $(dir)*.c))

OBJ = $(patsubst $(SRC_PATH)%.cpp,$(OBJ_PATH)%.o, $(FILES_CPP))
OBJ += $(patsubst $(SRC_PATH)%.c,$(OBJ_PATH)%.o, $(FILES_C))

# create depand files set
DEP = $(OBJ:%.o=%.d)

ifeq ($(MODE),Debug)
    VER_FLAG = $(DEBUG_FLAGS)
    BUILD_PATH = $(BUILD_PATH_BASE)/debug
    DEBUG_OUTPUT =$(basename $(OUTPUT_BASE))d$(suffix $(OUTPUT_BASE))
    OUTPUT = $(DEBUG_OUTPUT)
else
    VER_FLAG =$(RELEASE_FLAGS)
    BUILD_PATH = $(BUILD_PATH_BASE)/release
    OUTPUT = $(OUTPUT_BASE)
endif

DEFAULT: $(OUTPUT)
$(OUTPUT): $(OBJ)
	$(CC)  -o $@ $^ $(LIB_PATH) $(LIBS)
	$(info Link data to >>>>>>>>>> $@)

# include all .d files
-include $(DEP)

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.cpp
	$(COMMON_ADD_DIR)
	$(CC) $(CXXFLAGS) $(VER_FLAG) -c $< -o $@ $(INCLUDE_PATH)
#	$(info Compile obj >>>>>>>>>>> $@)

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c
	$(COMMON_ADD_DIR)
	$(CC) $(CFLAGS) $(VER_FLAG) -c $< -o $@ $(INCLUDE_PATH)
#	$(info Compile obj >>>>>>>>>>> $@)


.PHONY:clean run show
clean:
	$(COMMON_CLEAN)
	@echo clean data.
	@rm -rf *.elf

USERDEBUG?=
ARGS?=
run: $(OUTPUT)
	$(USERDEBUG) $(OUTPUT) $(ARGS)

show:
	echo BUILD_PATH: $(BUILD_PATH)
#	echo LOCAL_DIRS: $(LOCAL_DIRS)
	echo DIRS: $(DIRS)
#	echo FILES_CPP: $(FILES_CPP)
#	echo FILES_C: $(FILES_C)
#	echo OBJ: $(OBJ)
	echo OUTPUT: $(OUTPUT)
	echo BUILD_PATH :$(BUILD_PATH)
	echo OS: $(OS)
#	echo INCLUDE_PATH:$(INCLUDE_PATH)
