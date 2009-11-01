## -*- makefile -*-

ERL := erl
ERLC := $(ERL)c

INCLUDE_DIRS := ../include
EBIN_DIRS := $(wildcard ../ebin)
ERLC_FLAGS := -W $(INCLUDE_DIRS:../%=-I ../%) $(EBIN_DIRS:%=-pa %)

EBIN_DIR := ../ebin
DOC_DIR  := ../doc
EMULATOR := beam

ERL_SOURCES  := $(wildcard *.erl)
ERL_HEADERS  := $(wildcard *.hrl) $(wildcard ../include/*.hrl)
ERL_OBJECTS  := $(ERL_SOURCES:%.erl=$(EBIN_DIR)/%.beam)
ERL_TEMPLATES := $(ERL_TEMPLATE:%.et=$(EBIN_DIR)/%.beam)
ERL_OBJECTS_LOCAL := $(ERL_SOURCES:%.erl=./%.$(EMULATOR))
APP_FILES := $(wildcard *.app)
EBIN_FILES = $(ERL_OBJECTS) $(APP_FILES:%.app=../ebin/%.app) $(ERL_TEMPLATES)
MODULES = $(ERL_SOURCES:%.erl=%)

../ebin/%.app: %.app
	cp $< $@

$(EBIN_DIR)/%.$(EMULATOR): %.erl
	$(ERLC) $(ERLC_FLAGS) -o $(EBIN_DIR) $<

./%.$(EMULATOR): %.erl
	$(ERLC) $(ERLC_FLAGS) -o . $<

