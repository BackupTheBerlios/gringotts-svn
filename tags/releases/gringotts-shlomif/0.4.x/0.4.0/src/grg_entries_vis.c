/*  Gringotts - a small utility to safe-keep sensitive data
 *  (c) 2002, Germano Rizzo <mano78@users.sourceforge.net>
 *
 *  grg_entries_vis.c - generate and manage the widget for entry contents
 *  Authors: Germano Rizzo, Nicholas Pouillon
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <gtk/gtk.h>

#include "grg_defs.h"
#include "gringotts.h"
#include "grg_entries.h"
#include "grg_prefs.h"
#include "grg_widgets.h"

static GtkClipboard *clip = NULL;
static gboolean isThereAClip = FALSE;

static guchar *needle = NULL;

/**************
 * Sorry for the many commented pieces of code. It's work in progress...
 *************/

static int current_mode = SIMPLE_ENTRY;

static GtkTextBuffer *entryBuf = NULL;
#if 0
static GtkListStore *mdl = NULL;
#endif
static GtkWidget *simpleSheet = NULL/*, *structSheet = NULL*/;
static gulong simpleSigID = 0/*, structSigID = 0*/;

void entries_vis_init (void){
	/*GtkTreeViewColumn *c1, *c2, *c3;
	GtkCellRenderer *cr1, *cr2, *cr3;
*/
	clip = gtk_clipboard_get (GDK_NONE);
	if (clip) //why shouldn't it?
		isThereAClip = TRUE;

	entryBuf = gtk_text_buffer_new (NULL);
	simpleSheet = gtk_text_view_new_with_buffer (entryBuf);
	gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (simpleSheet), GTK_WRAP_WORD);
	simpleSigID = g_signal_connect (G_OBJECT (entryBuf), "changed",
		G_CALLBACK (meta_saveable), GINT_TO_POINTER (GRG_SAVE_ACTIVE));
/*
	mdl = gtk_list_store_new (3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	structSheet = gtk_tree_view_new_with_model (GTK_TREE_MODEL (mdl));
	g_object_unref (G_OBJECT (mdl));
	
	cr1=gtk_cell_renderer_text_new ();
	cr2=gtk_cell_renderer_text_new ();
	cr3=gtk_cell_renderer_text_new ();
	c1=gtk_tree_view_column_new_with_attributes ("URL", cr1, "text", 0, NULL);
	c2=gtk_tree_view_column_new_with_attributes ("UserID", cr2, "text", 1, NULL);
	c3=gtk_tree_view_column_new_with_attributes ("Password", cr3, "text", 2, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (structSheet), c1);
	gtk_tree_view_append_column (GTK_TREE_VIEW (structSheet), c2);
	gtk_tree_view_append_column (GTK_TREE_VIEW (structSheet), c3);*/
}

gboolean
has_needle (void) {
	return needle != NULL;
}

void
del_needle (void) {
	if (!has_needle())
		return;

	GRGAFREE (needle);
	needle = NULL;
}

void entries_vis_deinit (void){
	if (isThereAClip && grg_prefs_clip_clear_on_quit
	    && !grg_prefs_clip_clear_on_close)
		gtk_clipboard_clear (clip);

	del_needle ();
}

GtkWidget 
*get_updated_sheet (gboolean hasData){
	//if (current_mode == SIMPLE_ENTRY) {
		g_signal_handler_block (entryBuf, simpleSigID);
		gtk_text_buffer_set_text (entryBuf,
				  hasData ? grg_entries_get_Body () : "",
				  -1);
		g_signal_handler_unblock (entryBuf, simpleSigID);
		return simpleSheet;
	/*} else {
		GtkTreeIter iter;
		
		mdl = gtk_list_store_new (3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
		gtk_list_store_append(mdl, &iter);
		gtk_list_store_set(mdl, &iter, 0, "www.prosa.com", 1, "admin", 2, "nIitnPp", -1);
		gtk_tree_view_set_model (GTK_TREE_VIEW(structSheet), GTK_TREE_MODEL (mdl));
		return structSheet;
	}*/
}

void
clear_clipboard (void) {
	gtk_clipboard_clear (clip);
}

/**
 * sync_entry:
 *
 * Writes the information stored in the GtkTextBuffer to the
 * entry structure in memory. To be called whenever a page is
 * leaved, or saved.
 */
void
sync_entry (void)
{
	static GtkTextIter s, e;

	gtk_text_buffer_get_bounds (entryBuf, &s, &e);
	grg_entries_set_Body (gtk_text_buffer_get_text
			      (entryBuf, &s, &e, FALSE));
}

/**
 * cucopa:
 * @callback_data: unused callback param
 * @callback_action: action to perform (GRG_CUT, GRG_COPY, GRG_PASTE)
 *
 * basic CUT/COPY/PASTE clipboard operation.
 */
void
cucopa (gpointer callback_data, guint callback_action)
{
	switch ((grg_clip_action) callback_action)
	{
	case GRG_CUT:
		gtk_text_buffer_cut_clipboard (entryBuf, clip, TRUE);
		return;

	case GRG_COPY:
		gtk_text_buffer_copy_clipboard (entryBuf, clip);
		return;

	case GRG_PASTE:
		gtk_text_buffer_paste_clipboard (entryBuf, clip, NULL, TRUE);
		return;

	default:
#ifdef MAINTAINER_MODE
		g_assert_not_reached ();
#endif
		break;
	}
}

/**
 * find:
 * @callback_data: TRUE if I have to continue a previous search
 *
 * Search operation.
 */
void
find (GtkWidget *widget, gpointer callback_data)
{
        guint again = GPOINTER_TO_UINT(callback_data);
	static gboolean only_current, case_sens;
	gint found, offset = 0;
	guchar *buf;
	GtkTextIter position;
	GtkTextMark *cursor, *endsel;
        GtkWidget *parent = gtk_widget_get_toplevel(widget);
	if (!again)
		if (!grg_find_dialog
		    (&needle, &only_current, &case_sens, GTK_WINDOW (parent)))
			return;

	buf = grg_entries_get_Body ();
	if (((current_mode == SIMPLE_ENTRY) && GTK_WIDGET_HAS_FOCUS (simpleSheet))/* ||
		((current_mode == STRUCT_ENTRY) && GTK_WIDGET_HAS_FOCUS (structSheet))*/)
	{
		cursor = gtk_text_buffer_get_mark (entryBuf, "insert");
		gtk_text_buffer_get_iter_at_mark (entryBuf, &position,
						  cursor);
		offset = gtk_text_iter_get_offset (&position);
	}

	while (TRUE)
	{
		found = grg_entries_find (needle, offset, only_current,
					  case_sens);

		if (found >= 0)
		{
			buf = grg_entries_get_Body ();

			g_signal_handler_block (entryBuf, simpleSigID);
			gtk_text_buffer_set_text (entryBuf, buf, -1);
			g_signal_handler_unblock (entryBuf, simpleSigID);

			//to avoid that searching again and again the same text finds
			//the same portion, we set the cursor AFTER the found text
			/* And this time really do it -- Shlomi Fish */
			cursor = gtk_text_buffer_get_mark (entryBuf,
							   "insert");
			gtk_text_buffer_get_iter_at_mark (entryBuf, &position,
							  cursor);
			endsel = gtk_text_buffer_get_mark (entryBuf,
							   "selection_bound");
			gtk_text_iter_set_offset (&position,
						  found +
						  g_utf8_strlen (needle, -1));
			gtk_text_buffer_move_mark (entryBuf, cursor,
						   &position);
			gtk_text_iter_set_offset (&position, found);
			gtk_text_buffer_move_mark (entryBuf, endsel,
						   &position);

			/*
			 * Make sure that the text-view window scrolls to
			 * view the current selection.
			 * */
			gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW (simpleSheet),
				gtk_text_buffer_get_mark (entryBuf,
				"insert"));

			/*
			 * Make sure that the sheet gets focus, this is so
			 * pressing "Find again" consecutively will yield
			 * a second result, as well, as let the user move the
			 * cursor immediately.
			 * */
			gtk_widget_grab_focus (GTK_WIDGET (simpleSheet));

			break;
		}
		else
		{
			if (only_current)
			{
				grg_msg (_
					 ("The text searched could not be found!"),
					 GTK_MESSAGE_ERROR, parent);
				break;
			}
			else
			{
				if (grg_ask_dialog
				    (_("Wrap around?"),
				     _("Text not found. Continue search from beginning?"),
				     FALSE, parent) == GRG_YES)
				{
					grg_entries_first ();
					/* Call update() now, because we changed the page and
					 * sync() may be called later, which will otherwise
					 * cause the first page to be over-rided with the
					 * info in the current page.
					 * */
					update();
					offset = 0;
					continue;
				}
				else
					break;
			}
		}
	}
}
