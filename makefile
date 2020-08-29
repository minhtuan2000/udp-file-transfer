CLIENT_TARGET = client/client
SERVER_TARGET = server/server

CLIENT_OBJS = \
	common/url_utils.o \
	common/cli_utils.o \
	common/logger.o \
	client/requester.o \
	client/receiver.o \
	client/launcher.o \
	client/client.o

SERVER_OBJS = \
	common/url_utils.o \
	common/cli_utils.o \
	common/logger.o \
	server/sender.o \
	server/handler.o \
	server/launcher.o \
	server/server.o

REBUILDABLES = $(CLIENT_TARGET) $(CLIENT_OBJS) $(SERVER_TARGET) $(SERVER_OBJS)

all : $(CLIENT_TARGET) $(SERVER_TARGET)

clean: 
	rm -f $(REBUILDABLES)

$(CLIENT_TARGET) : $(CLIENT_OBJS)
	cc -g -Werror -std=gnu99 -pthread -o $@ $^ 

$(SERVER_TARGET) : $(SERVER_OBJS)
	cc -g -Werror -std=gnu99 -pthread -o $@ $^ 

%.o : %.c
	cc -g -Werror -std=gnu99 -o $@ -c $<

# Common dependencies
common/logger.o : common/logger.h
common/cli_utils.o : common/cli_utils.h common/logger.h
common/url_utils.o : common/url_utils.h common/cli_utils.h common/logger.h

COMMON_DEPENDENCIES = \
	common/logger.h \
	common/cli_utils.h \
	common/url_utils.h

# Client dependencies
client/requester.o : client/requester.h $(COMMON_DEPENDENCIES)
client/receiver.o : client/receiver.h $(COMMON_DEPENDENCIES)
client/launcher.o : client/launcher.h client/requester.h client/receiver.h $(COMMON_DEPENDENCIES)
client/client.o : client/client.h client/launcher.h $(COMMON_DEPENDENCIES)

# Server dependencies
server/sender.o : server/sender.h $(COMMON_DEPENDENCIES)
server/handler.o : server/handler.h server/sender.h $(COMMON_DEPENDENCIES)
server/launcher.o : server/launcher.h server/handler.h $(COMMON_DEPENDENCIES)
server/server.o : server/server.h server/launcher.h $(COMMON_DEPENDENCIES)
