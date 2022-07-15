#
# Main component makefile.
#
# This Makefile can be left empty. By default, it will take the sources in the
# src/ directory, compile them and link them into lib(subdirectory_name).a
# in the build directory. This behaviour is entirely configurable,
# please read the ESP-IDF documents if you need to do this.
#
# COMPONENT_EMBED_TXTFILES := ../images/anh.jpg
COMPONENT_EMBED_TXTFILES := ../html/index.html
COMPONENT_EMBED_TXTFILES :=  ${PROJECT_PATH}/server_certs/certs.pem