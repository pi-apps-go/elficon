/*
 *
 *  Copyright (c) 2008-2011 Erich E. Hoover
 *
 *  elfres - Adds resources into ELF files
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * To provide feedback, report bugs, or otherwise contact me:
 * ehoover at mines dot edu
 *
 */

#if GUI_ENABLED
	/* Handle GTK 3 */
	#include <gtk/gtk.h>
#endif /* GUI_ENABLED */

/* Obtaining command-line options */
#include <getopt.h>

/* Internationalization support */
#include <libr-i18n.h>

#include "elfres.h"

/* Reset if the GUI is called */
int gui_running = FALSE;

/* return application name */
extern const char *__progname;
#define progname() __progname

typedef enum {
	PARAM_PROGNAME      = 0,
	PARAM_OPTIONS       = 1,
	PARAM_ELF_FILE      = 2,
	PARAM_UUID          = 3,
	PARAM_RESOURCE_NAME = 3,
	PARAM_ICON_NAME     = 3,
	PARAM_ICON_SIZE     = 3,
	PARAM_RESOURCE_FILE = 4,
	PARAM_ICON_FILE     = 4,
} eParams;

typedef struct {
    char short_options[ELFICON_OPTIONS+1];
    struct option long_options[ELFICON_OPTIONS+1];
    char descriptions[ELFICON_OPTIONS][100];
} IconOptions;

IconOptions elficon_options = {
	"acfgrsv",
	{
		{"add-icon", 0, 0, 'a'},
		{"clear-icons", 0, 0, 'c'},
		{"find-icon", 0, 0, 'f'},
		{"get-uuid", 0, 0, 'g'},
		{"retrieve-icon", 0, 0, 'r'},
		{"set-uuid", 0, 0, 's'},
		{"version", 0, 0, 'v'},
		{NULL, 0, 0, 0}
    },
    {
        N_("add an icon to the ELF file"),
        N_("clear the file's ELF icon"),
        N_("find an ELF icon in the file by closest size"),
        N_("get the icon UUID for the file"),
        N_("retrieve an icon from the ELF file"),
        N_("set the icon UUID for the ELF file"),
        N_("display the current application revision"),
    }
};

typedef struct {
    char short_options[ELFRES_OPTIONS+1];
    struct option long_options[ELFRES_OPTIONS+1];
    char descriptions[ELFRES_OPTIONS][100];
} RcOptions;

RcOptions elfres_options = {
#if GUI_ENABLED
	"acilrv",
#else /* GUI_ENABLED=0 */
	"aclrv",
#endif /* GUI_ENABLED */
	{
		{"add", 0, 0, 'a'},
		{"clear", 0, 0, 'c'},
#if GUI_ENABLED
		{"interface", 0, 0, 'i'},
#endif /* GUI_ENABLED */
		{"list", 0, 0, 'l'},
		{"retrieve", 0, 0, 'r'},
		{"version", 0, 0, 'v'},
		{NULL, 0, 0, 0}
    },
    {
        N_("add a resource to the ELF file"),
        N_("clear an ELF file resource"),
#if GUI_ENABLED
        N_("open the elfres graphic interface"),
#endif /* GUI_ENABLED */
        N_("list libr-compatible resources"),
        N_("retrieve a resource from the ELF file"),
        N_("display the current application revision"),
    }
};

typedef enum {
	MODE_ADD_RESOURCE,
	MODE_CLEAR_RESOURCE,
	MODE_RETRIEVE_RESOURCE,
	MODE_LIST_RESOURCES,
	MODE_ADD_ICON,
	MODE_RETRIEVE_ICON,
	MODE_CLEAR_ICON,
	MODE_SET_UUID,
	MODE_GET_UUID,
	MODE_FIND_ICON,
	MODE_LAUNCH_GUI
} eMode;

/*
 * Return the size of the file represented by the file handle
 */
off_t file_size(FILE *file)
{
	struct stat file_stat;
	
	if(fstat(fileno(file), &file_stat) == ERROR)
		return ERROR;
	return file_stat.st_size;
}

/*
 * Output the application version
 */
void output_version(char *progname)
{
	printf(_("%s: Version %s\n"), progname, VERSION);
}

/*
 * Use command-line arguments to set the appropriate data mode
 */
int handle_arguments(int argc, char **argv, eMode *mode)
{
	int required_params = 0;
	int i, c, index = 0;
	
	opterr = 0;  /* Prevent automatic getopt() error message */
	/* 
	 * Normal mode of operation is for general resources, if the application
	 * is named "elficon" then it is specifically for icons
	 */
	if(strncmp(progname(), "elficon", 7) == 0)
	{
		while ((c = getopt_long(argc, argv, elficon_options.short_options, elficon_options.long_options, &index)) != EOF)
		{
			switch(c)
			{
				case 'a':
					*mode = MODE_ADD_ICON;
					required_params = 5;
					break;
				case 'c':
					*mode = MODE_CLEAR_ICON;
					required_params = 3;
					break;
				case 'f':
					*mode = MODE_FIND_ICON;
					required_params = 5;
					break;
				case 'g':
					*mode = MODE_GET_UUID;
					required_params = 3;
					break;
				case 'r':
					*mode = MODE_RETRIEVE_ICON;
					required_params = 5;
					break;
				case 's':
					*mode = MODE_SET_UUID;
					required_params = 4;
					break;
				case 'v':
					output_version(argv[PARAM_PROGNAME]);
					return FALSE;
				default:
					goto print_icon_usage;
			}
			index++;
	    }
		if(argc != required_params)
			goto print_icon_usage;
	}
	else
	{
		while ((c = getopt_long(argc, argv, elfres_options.short_options, elfres_options.long_options, &index)) != EOF)
		{
			switch(c)
			{
				case 'a':
					*mode = MODE_ADD_RESOURCE;
					required_params = 5;
					break;
				case 'c':
	        		*mode = MODE_CLEAR_RESOURCE;
					required_params = 4;
					break;
				case 'i':
	        		*mode = MODE_LAUNCH_GUI;
					required_params = -1;
					break;
				case 'l':
	        		*mode = MODE_LIST_RESOURCES;
					required_params = 3;
					break;
				case 'r':
					*mode = MODE_RETRIEVE_RESOURCE;
					required_params = 5;
					break;
				case 'v':
					output_version(argv[PARAM_PROGNAME]);
					return FALSE;
				default:
					goto print_rc_usage;
			}
			index++;
	    } 
		if(required_params != -1 && argc != required_params)
			goto print_rc_usage;
	}
    return TRUE;
print_rc_usage:
	fprintf(stderr, _("usage: %s [options] elf-file-name section-name resource-file-name\n"), argv[PARAM_PROGNAME]);
	for(i=0;i<ELFRES_OPTIONS;i++)
	{
		fprintf(stderr, "\t-%c, --%s\t= %s\n", elfres_options.short_options[i],
			elfres_options.long_options[i].name, gettext(elfres_options.descriptions[i]));
	}
	return FALSE;
print_icon_usage:
	fprintf(stderr, _("usage: %s [-a|-r] elf-file-name icon-name svg-file-name\n"), argv[PARAM_PROGNAME]);
	fprintf(stderr, _("usage: %s [-c|-g] elf-file-name\n"), argv[PARAM_PROGNAME]);
	fprintf(stderr, _("usage: %s [-s] elf-file-name uuid\n"), argv[PARAM_PROGNAME]);
	for(i=0;i<ELFICON_OPTIONS;i++)
	{
		fprintf(stderr, "\t-%c, --%s\t= %s\n", elficon_options.short_options[i],
			elficon_options.long_options[i].name, gettext(elficon_options.descriptions[i]));
	}
	return FALSE;
}

/*
 * Add a resource to an ELF file
 */
int add_resource(libr_file *libr_handle, char *resource_name, char *input_file)
{
	char *buffer = NULL;
	FILE *handle = NULL;
	off_t size, len;
	int ret = FALSE;
	
	if((handle = fopen(input_file, "r")) == NULL)
	{
		errorf(_("open \"%s\" failed: %m"), input_file);
		return FALSE;
	}
	/* Get the size of the file */
	if((size = file_size(handle)) == ERROR)
	{
		errorf(_("failed to obtain file size: %m"));
		goto done_handle;
	}
	/* Allocate buffers for the uncompressed and compressed data */
	buffer = (char *) malloc(size);
	if(buffer == NULL)
	{
		errorf(_("failed to create buffer: %m"));
		goto done_handle;
	}
	/* Read the uncompressed data from the disk */
	if((len = fread(buffer, 1, size, handle)) <= 0)
	{
		errorf(_("failed to read input file: %m"));
		goto done_buffer;
	}
	if(len != size)
	{
		errorf(_("failed to read entire file."));
		goto done_buffer;
	}
	/* Compress the data */
	if(!libr_write(libr_handle, resource_name, buffer, size, LIBR_COMPRESSED, LIBR_OVERWRITE))
	{
		errorf(_("failed to write ELF resource: %s"), libr_errmsg());
		goto done_buffer;
	}
	
	ret = TRUE;
	/* Close remaining resources */
done_buffer:
	free(buffer);
done_handle:
	fclose(handle);
	return ret;
}

/*
 * Get a resource stored in an ELF file
 */
void get_resource(libr_file *handle, char *section_name, char *output_file)
{
	size_t buffer_size = 0;
	char *buffer = NULL;
	FILE *file = NULL;
	off_t len;
	
	/* Get the resource from the ELF binary */
	if(!libr_size(handle, section_name, &buffer_size))
	{
		errorf(_("failed to obtain ELF resource size: %s"), libr_errmsg());
		return;
	}
	/* Get the resource from the ELF file */
	buffer = (char *) malloc(buffer_size);
	if(!libr_read(handle, section_name, buffer))
	{
		errorf(_("failed to obtain ELF resource: %s"), libr_errmsg());
		goto fail;
	}
	/* Store the resource to the disk */
	if((file = fopen(output_file, "w")) == NULL)
	{
		errorf(_("open \"%s\" failed"), output_file);
		goto fail;
	}
	if((len = fwrite(buffer, 1, buffer_size, file)) <= 0)
	{
		errorf(_("failed to write output file: %m"));
		goto fail;
	}
	
fail:
	/* Close remaining resources */
	if(file != NULL)
		fclose(file);
	free(buffer);
}

/*
 * Clear the icon stored in an ELF file
 */
void clear_resource(libr_file *handle, char *resource_name)
{
	if(!libr_clear(handle, resource_name))
		errorf(_("failed to remove resource: %s"), libr_errmsg());
}

/* 
 * List all libr-compatible resources
 */
void list_resources(libr_file *handle)
{
	int i, res = libr_resources(handle);
	
	if(res == 0)
	{
		printf(_("The file contains no libr-compatible resources.\n"));
		return;
	}
	printf(_("%d resource(s):\n"), res);
	for(i=0;i<res;i++)
	{
		char *name = libr_list(handle, i);
		
		if(name == NULL)
		{
			printf(_("error!\n"));
			continue;
		}
		printf(_("resource %d: %s\n"), i, name);
		free(name);
	}
}

/*
 * Started from console
 */
int main_console(int argc, char **argv)
{
	char *section = NULL, *input_file = NULL, *output_file = NULL;
	libr_access_t access = LIBR_READ;
	libr_file *handle = NULL;
	eMode mode;
	
	/* Process command-line arguments */
	if(!handle_arguments(argc, argv, &mode))
		return EX_USAGE;
#if GUI_ENABLED
	if(mode == MODE_LAUNCH_GUI)
		return main_gui(argc, argv);
#endif /* GUI_ENABLED */
	
	/* Open the ELF file to be edited */
	if(mode == MODE_CLEAR_ICON || mode == MODE_CLEAR_RESOURCE
	 || mode == MODE_ADD_ICON || mode == MODE_ADD_RESOURCE
	 || mode == MODE_SET_UUID)
	{
		access = LIBR_READ_WRITE;
	}
	if((handle = libr_open(argv[PARAM_ELF_FILE], access)) == NULL)
	{
		errorf(_("failed to open file \"%s\": %s"), argv[PARAM_ELF_FILE], libr_errmsg());
		return EX_SOFTWARE;
	}
	
	/* Perform the requested user operation */
	switch(mode)
	{
		case MODE_FIND_ICON:
		{
			unsigned int icon_size;
			libr_icon *icon = NULL;
			
			sscanf(argv[PARAM_ICON_SIZE], "%d", &icon_size);
			icon = libr_icon_geticon_bysize(handle, icon_size);
			if(icon == NULL)
			{
				errorf(_("failed to obtain ELF icon: %s"), libr_errmsg());
				goto fail;
			}
			if(!libr_icon_save(icon, argv[PARAM_ICON_FILE]))
			{
				libr_icon_close(icon);
				errorf(_("failed to save the icon to a file: %s"), libr_errmsg());
				goto fail;
			}
			libr_icon_close(icon);
		}	break;
		case MODE_GET_UUID:
		{
			char uuid[UUIDSTR_LENGTH];
			
			if(!libr_icon_getuuid(handle, uuid))
			{
				errorf(_("Failed to get UUID: %s\n"), libr_errmsg());
				goto fail;
			}
			printf("%s\n", uuid);
		}	break;
		case MODE_SET_UUID:
			if(!libr_icon_setuuid(handle, argv[PARAM_UUID]))
			{
				errorf(_("Failed to set UUID: %s\n"), libr_errmsg());
				goto fail;
			}
			break;
		case MODE_LIST_RESOURCES:
			list_resources(handle);
			break;
		case MODE_CLEAR_ICON:
			section = ICON_SECTION;
			/* intentional fall-through */
		case MODE_CLEAR_RESOURCE:
			if(section == NULL)
				section = argv[PARAM_RESOURCE_NAME];
			clear_resource(handle, section);
			break;
		case MODE_ADD_ICON:
		{
			libr_icon *icon = NULL;
			
			icon = libr_icon_newicon_byfile(LIBR_SVG, 0, argv[PARAM_ICON_FILE]);
			if(icon == NULL)
			{
				errorf(_("failed to open icon file \"%s\": %s"), argv[PARAM_ICON_FILE], libr_errmsg());
				goto fail;
			}
			if(!libr_icon_write(handle, icon, argv[PARAM_ICON_NAME], LIBR_OVERWRITE))
			{
				libr_icon_close(icon);
				errorf(_("failed to add icon to ELF file: %s"), libr_errmsg());
				goto fail;
			}
			libr_icon_close(icon);
		}	break;
		case MODE_ADD_RESOURCE:
			if(section == NULL)
			{
				section = argv[PARAM_RESOURCE_NAME];
				input_file = argv[PARAM_RESOURCE_FILE];
			}
			add_resource(handle, section, input_file);
			break;
		case MODE_RETRIEVE_ICON:
		{
			libr_icon *icon = NULL;
			
			icon = libr_icon_geticon_byname(handle, argv[PARAM_ICON_NAME]);
			if(icon == NULL)
			{
				errorf(_("failed to get icon named \"%s\": %s"), argv[PARAM_ICON_NAME], libr_errmsg());
				goto fail;
			}
			if(!libr_icon_save(icon, argv[PARAM_ICON_FILE]))
			{
				libr_icon_close(icon);
				errorf(_("failed to save the icon to a file: %s"), libr_errmsg());
				goto fail;
			}
			libr_icon_close(icon);
		}	break;
		case MODE_RETRIEVE_RESOURCE:
			if(section == NULL)
			{
				section = argv[PARAM_RESOURCE_NAME];
				output_file = argv[PARAM_RESOURCE_FILE];
			}
			get_resource(handle, section, output_file);
			break;
		default:
			errorf(_("Unhandled operation code."));
			goto fail;
	}
	
fail:
	/* Close file handle and exit */
	libr_close(handle);
	return EX_OK;
}

/*
 * Main execution routine
 */
int main(int argc, char **argv)
{
	libr_i18n_autoload(PACKAGE);
#if GUI_ENABLED
	/* Check to see if the application was launched from the command-line
	 * NOTE: Also allow GUI applications to call the "console version if there are parameters.
	 */
	if(strncmp(progname(), "elficon", 7) == 0 || getenv("TERM") != NULL || argc != 1)
		return main_console(argc, argv);
	else
		return main_gui(argc, argv);
#else /* GUI_ENABLED=0 */
	return main_console(argc, argv);
#endif /* GUI_ENABLED */
}
