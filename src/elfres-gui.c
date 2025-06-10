/*
 *
 *  Copyright (c) 2008-2011 Erich Hoover
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

/* Handle GTK 3 and GtkBuilder */
#include <gtk/gtk.h>

/* Use libr GTK+ convenience routines */
#include <libr-gtk.h>

/* Internationalization support */
#include <libr-i18n.h>

#include "elfres.h"

/* Global GUI Resources */
GtkBuilder *builder = NULL;
libr_file *gui_handle = NULL;

/* Forward declarations */
GUI_FN void gui_open_file(GtkFileChooser *dialog, void *_m);

/*
 * Return the requested widget from GtkBuilder
 */
GtkWidget *get_widget(const char *name)
{
	if(builder)
		return (GtkWidget *) gtk_builder_get_object(builder, name);
	return NULL;
}

/*
 * Open a gui error dialog for problems
 * NOTE: call errorf() and it will choose to use this function if appropriate.
 */ 
void gui_err(char *format, ...)
{
	GtkLabel *errmsg = GTK_LABEL(get_widget("errmsg"));
	GtkWidget *errdialog = get_widget("errdialog");
	char buffer[ERROR_BUF];
	va_list args;
	
	va_start(args, format);
	vsnprintf(buffer, ERROR_BUF, format, args);
	va_end(args);
	gtk_label_set_text(errmsg, buffer);
	gtk_widget_show(errdialog);
}

/*
 * Allow the user to choose a resource to add to the ELF binary
 */
GUI_FN void gui_add_resource(GtkFileChooser *dialog, void *_m)
{
	GtkTreeView *resourcelist = GTK_TREE_VIEW(get_widget("resourcelist"));
	GtkListStore *lstore = GTK_LIST_STORE(gtk_tree_view_get_model(resourcelist));
	char *filename = gtk_file_chooser_get_filename(dialog);
	char *abbrvname = strdup(strrchr(filename, '/')+1);
	GtkTreeIter iter;
	
	/* Add the file to the binary */
	if(!add_resource(gui_handle, abbrvname, filename))
		return; /* Failed to add file */

	/* Add the file to the GUI list */
	gtk_list_store_append(lstore, &iter);
	gtk_list_store_set(lstore, &iter, 0, abbrvname, -1);
}

/*
 * Allow the user to extract a resource from an ELF binary
 */
GUI_FN void gui_save_resource(GtkFileChooser *dialog, void *_m)
{
	GtkTreeView *resourcelist = GTK_TREE_VIEW(get_widget("resourcelist"));
	GtkTreeModel *lstore = GTK_TREE_MODEL(gtk_tree_view_get_model(resourcelist));
	GtkTreeSelection *selection = gtk_tree_view_get_selection(resourcelist);
	char *filename = gtk_file_chooser_get_filename(dialog);
	char *abbrvname = strdup(strrchr(filename, '/')+1);
	char *resource = NULL;
	GtkTreeIter iter;
	
	if(!gtk_tree_selection_get_selected(selection, &lstore, NULL))
	{
		errorf(_("No resource selected."));
		return;
	}
	gtk_tree_selection_get_selected(selection, &lstore, &iter);
	gtk_tree_model_get(lstore, &iter, 0, &resource, -1);
	
	/* Save the file */
	get_resource(gui_handle, resource, filename);
}

/*
 * Remove the selected resource from the ELF binary
 */
GUI_FN void gui_remove_resource(GtkWidget *widget, void *_m)
{
	GtkTreeView *resourcelist = GTK_TREE_VIEW(get_widget("resourcelist"));
	GtkTreeModel *lstore = GTK_TREE_MODEL(gtk_tree_view_get_model(resourcelist));
	GtkTreeSelection *selection = gtk_tree_view_get_selection(resourcelist);
	char *resource = NULL;
	GtkTreeIter iter;
	
	if(!gtk_tree_selection_get_selected(selection, &lstore, NULL))
	{
		errorf(_("No resource selected."));
		return;
	}
	gtk_tree_selection_get_selected(selection, &lstore, &iter);
	gtk_tree_model_get(lstore, &iter, 0, &resource, -1);
	if(!libr_clear(gui_handle, resource))
	{
		errorf(_("failed to remove resource: %s"), libr_errmsg());
		return;
	}
	gtk_list_store_remove(GTK_LIST_STORE(lstore), &iter);
}

/*
 * Re-save the ELF binary with the requested changes
 */
GUI_FN void gui_apply_changes(GtkWidget *widget, void *_m)
{
	GtkFileChooser *dialog = GTK_FILE_CHOOSER(get_widget("openfile"));
	char *filename = gtk_file_chooser_get_filename(dialog);
	
	/* Write changes to disk */
	libr_close(gui_handle);
	/* Re-open handle */
	if((gui_handle = libr_open(filename, LIBR_READ_WRITE)) == NULL)
	{
		errorf(_("failed to re-open the executable (%s) for resources: %s"), filename, libr_errmsg());
		return;
	}
}

/*
 * A row was selected, enable the save and remove options
 */
GUI_FN void gui_row_selected(void *_m)
{
	GtkWidget *remove = get_widget("remove");
	GtkWidget *save = get_widget("save");

	gtk_widget_set_sensitive(save, TRUE);
	gtk_widget_set_sensitive(remove, TRUE);
}

/*
 * A row was selected, enable the save and remove options
 */
GUI_FN void gui_open_save_dialog(GtkFileChooser *dialog, void *_m)
{
	GtkTreeView *resourcelist = GTK_TREE_VIEW(get_widget("resourcelist"));
	GtkTreeModel *lstore = GTK_TREE_MODEL(gtk_tree_view_get_model(resourcelist));
	GtkTreeSelection *selection = gtk_tree_view_get_selection(resourcelist);
	char *resource = NULL;
	GtkTreeIter iter;
	
	if(!gtk_tree_selection_get_selected(selection, &lstore, NULL))
	{
		errorf(_("No resource selected."));
		return;
	}
	gtk_tree_selection_get_selected(selection, &lstore, &iter);
	gtk_tree_model_get(lstore, &iter, 0, &resource, -1);
	gtk_file_chooser_set_filename(dialog, resource);
	gtk_widget_show(GTK_WIDGET(dialog));
}

/*
 * Handle response from add resource dialog
 */
GUI_FN void on_addresource_response(GtkDialog *dialog, gint response_id, gpointer user_data)
{
	if(response_id == GTK_RESPONSE_ACCEPT) {
		gui_add_resource(GTK_FILE_CHOOSER(dialog), NULL);
	}
	gtk_widget_hide(GTK_WIDGET(dialog));
}

/*
 * Handle response from open file dialog
 */
GUI_FN void on_openfile_response(GtkDialog *dialog, gint response_id, gpointer user_data)
{
	if(response_id == GTK_RESPONSE_ACCEPT) {
		gui_open_file(GTK_FILE_CHOOSER(dialog), NULL);
	}
	gtk_widget_hide(GTK_WIDGET(dialog));
}

/*
 * Handle response from save resource dialog
 */
GUI_FN void on_saveresource_response(GtkDialog *dialog, gint response_id, gpointer user_data)
{
	if(response_id == GTK_RESPONSE_ACCEPT) {
		gui_save_resource(GTK_FILE_CHOOSER(dialog), NULL);
	}
	gtk_widget_hide(GTK_WIDGET(dialog));
}

/*
 * Allow the user to choose an ELF binary to edit
 */
GUI_FN void gui_open_file(GtkFileChooser *dialog, void *_m)
{
	GtkTreeView *resourcelist = GTK_TREE_VIEW(get_widget("resourcelist"));
	GtkLabel *activefile = GTK_LABEL(get_widget("activefile"));
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
	char *filename = gtk_file_chooser_get_filename(dialog);
	char *abbrvname = strdup(strrchr(filename, '/')+1);
	GtkWidget *getfile = get_widget("getfile");
	GtkWidget *apply = get_widget("apply");
	GtkWidget *add = get_widget("add");
	GtkListStore *lstore;
	int i, res;
	
	if((gui_handle = libr_open(filename, LIBR_READ_WRITE)) == NULL)
	{
		errorf(_("failed to open the executable (%s) for resources: %s"), filename, libr_errmsg());
		return;
	}
	res = libr_resources(gui_handle);
	
	lstore = gtk_list_store_new(1, G_TYPE_STRING);
	for(i=0;i<res;i++)
	{
		char *name = libr_list(gui_handle, i);
		GtkTreeIter iter;
		
		if(name == NULL)
		{
			printf("error!\n");
			continue;
		}
		gtk_list_store_append(lstore, &iter);
		gtk_list_store_set(lstore, &iter, 0, name, -1);
		free(name);
	}
	gtk_tree_view_insert_column_with_attributes(resourcelist, -1, "Resource", renderer, "text", 0, NULL);
	gtk_tree_view_set_model(resourcelist, GTK_TREE_MODEL(lstore));
	
	gtk_label_set_text(activefile, abbrvname);
	gtk_widget_set_sensitive(add, TRUE);
	gtk_widget_set_sensitive(apply, TRUE);
	gtk_widget_set_sensitive(getfile, FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(resourcelist), TRUE);
}

/*
 * Started from desktop
 */
int main_gui(int argc, char **argv)
{
	GtkWindow *window = NULL;
	GError *error = NULL;
	
	/* Initialize GTK 3 */
	gtk_init(&argc, &argv);
	
	/* Create GtkBuilder and load UI file */
	builder = gtk_builder_new();
	
	/* Load UI from embedded resource using libr */
	char *exe_path = argv[0];
	/* If argv[0] doesn't contain a path, try to find the full path */
	if(strchr(argv[0], '/') == NULL) {
		exe_path = "/usr/local/bin/elfres";  /* Use known install path */
	}
	libr_file *handle = libr_open(exe_path, LIBR_READ);
	if(handle)
	{
		size_t ui_size = 0;
		char *ui_data = libr_malloc(handle, ".ui", &ui_size);
		if(ui_data && ui_size > 0)
		{
			if(!gtk_builder_add_from_string(builder, ui_data, ui_size, &error))
			{
				errorf("failed to load UI from embedded resource: %s", 
					   error ? error->message : "unknown error");
				if(error) g_error_free(error);
				free(ui_data);
				libr_close(handle);
				return EX_SOFTWARE;
			}
			free(ui_data);
		}
		else
		{
			/* Fallback to file if embedded resource fails */
			if(!gtk_builder_add_from_file(builder, "elfres.ui", &error))
			{
				errorf("failed to load UI file and embedded resource: %s", 
					   error ? error->message : "UI file not found");
				if(error) g_error_free(error);
				libr_close(handle);
				return EX_SOFTWARE;
			}
		}
		libr_close(handle);
	}
	else
	{
		/* Fallback to file if can't open executable */
		if(!gtk_builder_add_from_file(builder, "elfres.ui", &error))
		{
			errorf("failed to load UI file and cannot open executable: %s", 
				   error ? error->message : "UI file not found");
			if(error) g_error_free(error);
			return EX_SOFTWARE;
		}
	}
	
	/* Get main window */
	window = GTK_WINDOW(gtk_builder_get_object(builder, "elfres"));
	if(!window)
	{
		errorf("failed to get main window from UI");
		return EX_SOFTWARE;
	}
	
	/* Connect signals */
	gtk_builder_connect_signals(builder, NULL);
	
	/* Set up file chooser dialog buttons properly for GTK 3 */
	GtkDialog *addresource = GTK_DIALOG(gtk_builder_get_object(builder, "addresource"));
	GtkDialog *openfile = GTK_DIALOG(gtk_builder_get_object(builder, "openfile"));
	GtkDialog *saveresource = GTK_DIALOG(gtk_builder_get_object(builder, "saveresource"));
	
	if(addresource) {
		gtk_dialog_add_button(addresource, "_Cancel", GTK_RESPONSE_CANCEL);
		gtk_dialog_add_button(addresource, "_Open", GTK_RESPONSE_ACCEPT);
	}
	
	if(openfile) {
		gtk_dialog_add_button(openfile, "_Cancel", GTK_RESPONSE_CANCEL);
		gtk_dialog_add_button(openfile, "_Open", GTK_RESPONSE_ACCEPT);
	}
	
	if(saveresource) {
		gtk_dialog_add_button(saveresource, "_Cancel", GTK_RESPONSE_CANCEL);
		gtk_dialog_add_button(saveresource, "_Save", GTK_RESPONSE_ACCEPT);
	}
	
	/* Set the window icon using the app icon */
	gtk_window_set_default_icon_name("elfres");
	
	/* Load embedded images for GUI elements */
	GtkImage *gear_image = GTK_IMAGE(gtk_builder_get_object(builder, "image2"));
	if(gear_image) {
		/* Reopen handle for gear image loading */
		libr_file *gear_handle = libr_open(exe_path, LIBR_READ);
		if(gear_handle) {
			size_t gear_size = 0;
			char *gear_data = libr_malloc(gear_handle, "gears.svg", &gear_size);
			if(gear_data && gear_size > 0) {
			GError *error = NULL;
			GdkPixbufLoader *loader = gdk_pixbuf_loader_new();
			if(loader) {
				if(gdk_pixbuf_loader_write(loader, (const guchar*)gear_data, gear_size, &error)) {
					if(gdk_pixbuf_loader_close(loader, &error)) {
						GdkPixbuf *pixbuf = gdk_pixbuf_loader_get_pixbuf(loader);
						if(pixbuf) {
							/* Scale to appropriate size */
							GdkPixbuf *scaled = gdk_pixbuf_scale_simple(pixbuf, 64, 64, GDK_INTERP_BILINEAR);
							if(scaled) {
								gtk_image_set_from_pixbuf(gear_image, scaled);
								g_object_unref(scaled);
							}
						}
					}
				}
				g_object_unref(loader);
				if(error) {
					g_error_free(error);
				}
			}
			free(gear_data);
			} else {
				/* Fallback to icon name if embedded resource not found */
				gtk_image_set_from_icon_name(gear_image, "preferences-system", GTK_ICON_SIZE_DIALOG);
			}
			libr_close(gear_handle);
		} else {
			/* Fallback to icon name if can't open handle */
			gtk_image_set_from_icon_name(gear_image, "preferences-system", GTK_ICON_SIZE_DIALOG);
		}
	}

	/* Show the window */
	gtk_widget_show_all(GTK_WIDGET(window));
	
	/* Check if a file was specified on the command line (for -i filename usage) */
	if(argc >= 2) {
		/* Skip the program name and any -i flag, look for the actual filename */
		char *filename = NULL;
		int i;
		
		/* Find the filename argument (skip options) */
		for(i = 1; i < argc; i++) {
			if(argv[i][0] != '-') {
				filename = argv[i];
				break;
			}
		}
		
		if(filename && strlen(filename) > 0) {
			/* Open the file directly by calling the file opening logic */
			GtkTreeView *resourcelist = GTK_TREE_VIEW(get_widget("resourcelist"));
			GtkLabel *activefile = GTK_LABEL(get_widget("activefile"));
			GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
			char *last_slash = strrchr(filename, '/');
			char *abbrvname = strdup(last_slash ? last_slash + 1 : filename);
			GtkWidget *getfile = get_widget("getfile");
			GtkWidget *apply = get_widget("apply");
			GtkWidget *add = get_widget("add");
			GtkListStore *lstore;
			int j, res;
			
			/* Try to open the file */
			if((gui_handle = libr_open(filename, LIBR_READ_WRITE)) != NULL)
			{
				res = libr_resources(gui_handle);
				
				lstore = gtk_list_store_new(1, G_TYPE_STRING);
				for(j=0;j<res;j++)
				{
					char *name = libr_list(gui_handle, j);
					GtkTreeIter iter;
					
					if(name == NULL)
					{
						printf("error!\n");
						continue;
					}
					gtk_list_store_append(lstore, &iter);
					gtk_list_store_set(lstore, &iter, 0, name, -1);
					free(name);
				}
				gtk_tree_view_insert_column_with_attributes(resourcelist, -1, "Resource", renderer, "text", 0, NULL);
				gtk_tree_view_set_model(resourcelist, GTK_TREE_MODEL(lstore));
				
				gtk_label_set_text(activefile, abbrvname);
				gtk_widget_set_sensitive(add, TRUE);
				gtk_widget_set_sensitive(apply, TRUE);
				gtk_widget_set_sensitive(getfile, FALSE);
				gtk_widget_set_sensitive(GTK_WIDGET(resourcelist), TRUE);
			}
			else
			{
				/* Show error if file couldn't be opened, but don't crash the app */
				errorf(_("failed to open the executable (%s) for resources: %s"), filename, libr_errmsg());
			}
			
			free(abbrvname);
		}
	}
	
	/* Wait for and respond to user operations */
	gui_running = TRUE;
	gtk_main();
	
	/* Cleanup */
	if(gui_handle)
		libr_close(gui_handle);
	if(builder)
		g_object_unref(builder);
		
	return EX_OK;
}

#endif /* GUI_ENABLED */
