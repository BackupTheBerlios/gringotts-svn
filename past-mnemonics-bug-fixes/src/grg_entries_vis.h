/*  Gringotts - a small utility to safe-keep sensitive data
 *  (c) 2002, Germano Rizzo <mano78@users.sourceforge.net>
 *
 *  grg_entries_vis.h - header file for grg_entries_vis.c
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

#ifndef GRG_ENTRIES_VIS_H
#define GRG_ENTRIES_VIS_H

void entries_vis_init (void);
void entries_vis_deinit (void);

void sync_entry (void);
GtkWidget *get_updated_sheet (gboolean hasData);

void cucopa (gpointer callback_data, guint callback_action);
void clear_clipboard (void);

//Search operation
void find (gpointer callback_data, guint again, GtkWidget *parent);
void del_needle (void);
gboolean has_needle (void);

#endif
