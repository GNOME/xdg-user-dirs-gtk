#include "config.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <libintl.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include "parse.h"

static XdgDirEntry *
find_dir_entry (XdgDirEntry *entries, const char *type)
{
  int i;

  for (i = 0; entries[i].type != NULL; i++)
    {
      if (strcmp (entries[i].type, type) == 0)
	return &entries[i];
    }
  return NULL;
}

static XdgDirEntry *
find_dir_entry_by_path (XdgDirEntry *entries, const char *path)
{
  int i;

  for (i = 0; entries[i].type != NULL; i++)
    {
      if (strcmp (entries[i].path, path) == 0)
	return &entries[i];
    }
  return NULL;
}

static void
update_locale (XdgDirEntry *old_entries)
{
  XdgDirEntry *new_entries, *entry;
  GtkWidget *dialog;
  int exit_status;
  int fd;
  char *filename;
  char *cmdline;
  int response;
  int i, j;
  GtkListStore *list_store;
  GtkTreeIter iter;
  GtkWidget *treeview, *check;
  GtkCellRenderer *cell;
  char *std_out, *std_err;

  fd = g_file_open_tmp ("dirs-XXXXXX", &filename, NULL);
  if (fd == -1)
    return;
  close (fd);
  
  cmdline = g_strdup_printf (XDG_USER_DIRS_UPDATE " --force --dummy-output %s", filename);
  if (!g_spawn_command_line_sync  (cmdline, &std_out, &std_err, &exit_status, NULL))
    {
      g_free (std_out);
      g_free (std_err);
      g_free (cmdline);
      g_unlink (filename);
      g_free (filename);
      return;
    }
  g_free (std_out);
  g_free (std_err);
  g_free (cmdline);

  if (!WIFEXITED(exit_status) || WEXITSTATUS(exit_status) != 0)
    return;
  
  new_entries = parse_xdg_dirs (filename);
  g_unlink (filename);
  g_free (filename);

  list_store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
  
  for (i = 0; old_entries[i].type != NULL; i++)
    {
      for (j = 0; new_entries[j].type != NULL; j++)
	{
	  if (strcmp (old_entries[i].type, new_entries[j].type) == 0)
	    break;
	}
      if (new_entries[j].type != NULL &&
	  strcmp (old_entries[i].path, new_entries[j].path) != 0)
	{
	  char *from, *to;
	  from = g_filename_display_name (old_entries[i].path);
	  to = g_filename_display_name (new_entries[j].path);

	  gtk_list_store_append (list_store, &iter);
	  gtk_list_store_set (list_store, &iter,
			      0, from, 1, to, -1);
	  
	  g_free (from);
	  g_free (to);
	}
    }
  for (j = 0; new_entries[j].type != NULL; j++)
    {
      for (i = 0; old_entries[i].type != NULL; i++)
	{
	  if (strcmp (old_entries[i].type, new_entries[j].type) == 0)
	    break;
	}
      if (old_entries[i].type == NULL)
	{
	  char *to;
	  to = g_filename_display_name (new_entries[j].path);

	  gtk_list_store_append (list_store, &iter);
	  gtk_list_store_set (list_store, &iter,
			      0, "-", 1, to, -1);

	  g_free (to);
	}
    }

  dialog = gtk_message_dialog_new (NULL, 0,
				   GTK_MESSAGE_QUESTION,
				   GTK_BUTTONS_NONE,
				   _("You have logged in in a new language. Do you want to update the standard folders in your home folder to the new language?"));
  gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
					    _("The update would change the following folders:"));

  gtk_dialog_add_buttons (GTK_DIALOG (dialog),
			  _("Keep old folders"), GTK_RESPONSE_NO,
			  _("Update names"), GTK_RESPONSE_YES,
			  NULL);

  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_NO);
  
  treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL (list_store));

  cell = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
					       -1, _("Current folder"),
					       cell,
					       "text", 0,
					       NULL);
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
					       -1, _("New folder"),
					       cell,
					       "text", 1,
					       NULL);

  gtk_widget_show (treeview);
  
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), treeview, FALSE, FALSE, 2);

  check = gtk_check_button_new_with_label (_("Don't ask me this again"));
  gtk_widget_show (check);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), check, FALSE, FALSE, 2);
  
  response =  gtk_dialog_run (GTK_DIALOG (dialog));

  if (response == GTK_RESPONSE_YES)
    {
      if (!g_spawn_command_line_sync (XDG_USER_DIRS_UPDATE " --force", NULL, NULL, &exit_status, NULL) ||
	  !WIFEXITED(exit_status) ||
	  WEXITSTATUS(exit_status) != 0)
	{
	  GtkWidget *error;

	  error = gtk_message_dialog_new (NULL, 0,
					  GTK_MESSAGE_ERROR,
					  GTK_BUTTONS_OK,
					  _("There was an error updating the folders"));
	  
	  gtk_dialog_run (GTK_DIALOG (error));
	  gtk_widget_destroy (error);
	}
      else
	{
	  /* Change succeeded, remove any leftover empty directories */
	  for (i = 0; old_entries[i].type != NULL; i++)
	    {
	      /* Never remove homedir */
	      if (strcmp (old_entries[i].path, g_get_home_dir ()) == 0)
		continue;
	      
	      /* If the old path is used by the new config, don't remove */
	      entry = find_dir_entry_by_path (new_entries, old_entries[i].path);
	      if (entry)
		continue;

	      /* Remove the dir, will fail if not empty */
	      g_rmdir (old_entries[i].path);
	    }
	}
    }

  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (check)))
    {
      char *file;
      
      file = g_build_filename (g_get_user_config_dir (),
			       "user-dirs.locale", NULL);
      g_unlink (file);
      g_free (file);
    }

  g_free (new_entries);

  gtk_widget_destroy (dialog);
}

int
main (int argc, char *argv[])
{
  XdgDirEntry *old_entries, *new_entries, *entry;
  GtkBookmark *bookmark;
  GList *bookmarks, *l;
  char *old_locale;
  char *locale, *dot;
  int i;
  gboolean modified_bookmarks;
  char *uri;
  
  setlocale (LC_ALL, "");
  
  bindtextdomain (GETTEXT_PACKAGE, GLIBLOCALEDIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);

  gtk_init (&argc, &argv);

  old_entries = parse_xdg_dirs (NULL);
  old_locale = parse_xdg_dirs_locale ();
  locale = g_strdup (setlocale (LC_MESSAGES, NULL));
  dot = strchr (locale, '.');
  if (dot)
    *dot = 0;

  if (old_locale && *old_locale != 0 &&
      strcmp (old_locale, locale) != 0)
    update_locale (old_entries);

  new_entries = parse_xdg_dirs (NULL);

  bookmarks = parse_gtk_bookmarks (NULL);

  modified_bookmarks = FALSE;
  if (bookmarks == NULL)
    {
      char *make_bm_for[] = {
	"DOCUMENTS",
	"MUSIC",
	"PICTURES",
	"VIDEOS",
	"DOWNLOAD",
	NULL};
      /* No previous bookmarks. Generate standard ones */

      for (i = 0; make_bm_for[i] != NULL; i++)
	{
	  entry = find_dir_entry (new_entries, make_bm_for[i]);
	  if (entry && strcmp (entry->path, g_get_home_dir ()) != 0)
	    {
	      uri = g_filename_to_uri (entry->path, NULL, NULL);
	      if (uri)
		{
		  modified_bookmarks = TRUE;
		  bookmark = g_new0 (GtkBookmark, 1);
		  bookmark->uri = uri;
		  bookmarks = g_list_append (bookmarks, bookmark);
		}
	    }
	}
    }
  else
    {
      /* Map old bookmarks that were moved */

      for (l = bookmarks; l != NULL; l = l->next)
	{
	  char *path;
	  
	  bookmark = l->data;

	  path = g_filename_from_uri (bookmark->uri, NULL, NULL);
	  if (path)
	    {
	      entry = find_dir_entry_by_path (old_entries, path);
	      if (entry)
		{
		  entry = find_dir_entry (new_entries, entry->type);
		  if (entry)
		    {
		      uri = g_filename_to_uri (entry->path, NULL, NULL);
		      if (uri)
			{
			  modified_bookmarks = TRUE;
			  g_free (bookmark->uri);
			  bookmark->uri = uri;
			}
		    }
		}
	      g_free (path);
	    }
	}
    }

  if (modified_bookmarks)
    save_gtk_bookmarks (NULL, bookmarks);
  
  g_free (new_entries);
  g_free (old_entries);
  
  return 0;
}
