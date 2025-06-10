#ifndef __ELFRES_H
#define __ELFRES_H

/* Handle ELF Resources */
#include <libr.h>
#include <libr-icons.h>

/* Handle strings */
#include <string.h>

/* Obtain file information */
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>

/* Handle exit cases and errors */
#include <sysexits.h>

/* in elfres.c */
off_t file_size(FILE *file);

/* in elfres_gui.c */
int main_gui(int argc, char **argv);

#ifndef FALSE
	#define FALSE 0
#endif
#ifndef TRUE
	#define TRUE 1
#endif

extern int gui_running;

#define ERROR           -1
#define ERROR_BUF        1024
#define ELFICON_OPTIONS  7
#define ICON_SECTION     ".icon"
#define GUI_FN           __attribute__ ((visibility ("default")))
#define con_err(...)     fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n")
#define errorf(...)      gui_running ? gui_err(__VA_ARGS__) : con_err(__VA_ARGS__)

#if GUI_ENABLED
#define ELFRES_OPTIONS    6
#else /* GUI_ENABLED=0 */
#define ELFRES_OPTIONS    5
#endif /* GUI_ENABLED */

/* Functions shared by both GUI and console execution */
void gui_err(char *format, ...);
void get_resource(libr_file *handle, char *section_name, char *output_file);
int add_resource(libr_file *libr_handle, char *resource_name, char *input_file);

#endif /* __ELFRES_H */
