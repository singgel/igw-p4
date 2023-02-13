BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj

SRCS-OBJS = $(patsubst %.c,%.o, $(SRCS-y))
CPPSRCS-OBJS = $(patsubst %.cpp,%.o, $(CPPSRCS-y))

OBJS = $(addprefix $(OBJ_DIR)/,$(SRCS-OBJS))   
CPPOBJS = $(addprefix $(OBJ_DIR)/,$(CPPSRCS-OBJS))

$(APP): $(OBJS) $(CPPOBJS)
	gcc $(CFLAGS) $(LDFLAGS) $^ -o $@

$(OBJ_DIR)/%.o : %.c
	@if [ ! -d $(OBJ_DIR) ]; then mkdir -p $(OBJ_DIR); fi;
	gcc -c $(CFLAGS) $< -o $@ 

$(OBJ_DIR)/%.o : %.cpp
	@if [ ! -d $(OBJ_DIR) ]; then mkdir -p $(OBJ_DIR); fi;
	g++ -c $(CXXFLAGS) $< -o $@

.PHONY:clean
clean:
	-@rm -f $(APP)
	-@rm -rf $(BUILD_DIR)
