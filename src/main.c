/*
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
 

#include <config.h>

#include <gnome.h>
#include <gconf/gconf-client.h>
#include <locale.h>
#include "main_window_cb.h"
#include "main_window.h"


int main(int argc,char *argv[])
{
	
	GnomeProgram *gwget;
	

	bindtextdomain (GETTEXT_PACKAGE, GNOME_GWGET_LOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
	setlocale(LC_ALL, "");

	
	gwget = gnome_program_init("Gwget",VERSION,LIBGNOMEUI_MODULE,argc,argv,NULL);
		
	main_window();
	
	gtk_main();
		
	return (0);
}
