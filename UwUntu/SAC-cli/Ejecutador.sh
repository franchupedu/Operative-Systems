gcc sac-cli.c hablar-servidor.c -DFUSE_USE_VERSION=27 -D_FILE_OFFSET_BITS=64 -lpthread -lfuse -lcommons -g -o sac-cli

