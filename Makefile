# Define your target executable names and source files
EXECUTABLES = getfacl setfacl fgetc fputc create_dir change_dir acl_creater simple_sudo
SOURCES = getfacl.c setfacl.c fgetc.c fputc.c create_dir.c change_dir.c acl_creater.c simple_sudo.c

# Build each executable separately
all: $(EXECUTABLES)

$(EXECUTABLES): $(SOURCES)
	gcc $@.c -o $@

# Clean targets
clean:
	rm -f $(EXECUTABLES)

# Define the setuid and executable permissions for each target
setuid_executable: $(EXECUTABLES)
	for exec in $(EXECUTABLES); do \
		chmod u+sx $$exec; \
	done
