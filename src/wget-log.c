/*
 *  Copyright (C) 1999-2001 Bruno Pires Marinho
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
/*
 * For the log parsing to work, the wget process must be started with
 * the "C" locale, e.g., by setting the environment variable LC_ALL=C
 * programmatically prior to invoking wget.
 */

#define _FILE_OFFSET_BITS 64

#include <config.h>
#include <gnome.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include "gwget_data.h"
#include "wget-log.h"
#include "main_window.h"
#include "systray.h"
#include "utils.h"


/* Function to convert the wget notation of file size (55,449,600) */ 
static guint64
convert_wget_size (char *size)
{
	char *p = size;

	while (*p != ' ' && *p != '\0') {
		if (*p == ',') {
			while (*p != ' ' && *p != '\0')
				{
				*p = *(p+1);
				p++;
				}
			p = size;
		} else
			p++;
   }
   return atoll (size);
}

static void
show_error (GwgetData *gwgetdata, gchar *error_msg)
{
    gwget_data_set_state(gwgetdata,DL_ERROR);
	gwgetdata->error=TRUE;
	gwgetdata->error_msg=g_strdup(error_msg);
}

/* Get session start time and session file start size */
static void
set_start_size_and_time(GwgetData *gwgetdata)
{
	struct stat file_stat;
	if (stat (gwgetdata->local_filename, &file_stat) != -1) {
    	gwgetdata->session_start_time = file_stat.st_ctime;
    	gwgetdata->session_start_size = file_stat.st_size;
	} else {
    	gwgetdata->session_start_time = 0;
    	gwgetdata->session_start_size = 0;
	}
}

/* 
 * If gwgetdata->filename does not match the filesystem filename,
 * bad things can happen. We intecept the line that prints the 
 * filesystem filename and set gwgetdata->filename
 */
static void
update_filename(GwgetData *gwgetdata)
{
	char *sName = gwgetdata->line;
	int iL = strlen(sName);
	sName[iL-1] = 0; // Chop the final ' 

	/*
	 * Now sName contains the whole pathname. No filename can
	 * contain '/' so the following search for the last component
	 * is sane
	 */
	sName += iL - 2;
	while (*sName != '/' && sName != gwgetdata->line)
		sName--;
	if (*sName == '/')
		sName++;

	gwget_data_set_filename(gwgetdata,sName);
	set_start_size_and_time(gwgetdata);
	gwget_data_update_statistics(gwgetdata);
	gwget_remember_downloads();
}

static int
wget_log_process_line (GwgetData *gwgetdata)
{
	gchar *p;
	gint dots = 0;

	if ((gwgetdata->line == NULL) || (strlen (gwgetdata->line) == 0))
		return 0;
		
	switch (gwgetdata->state) {
	case DL_NOT_CONNECTED:
		if (strstr(gwgetdata->line,"           => `")) {		/* wget <1.11 */
		    update_filename(gwgetdata);
		    break;
    	}
   
		/* First check to see if we connected to the host correctly. */

		/* Wget 1.8.1 says "connected." rather than "connected!" */
		if (strstr (gwgetdata->line, "connected!") != NULL ||
			strstr (gwgetdata->line, "connected.") != NULL) {
				gwget_data_set_state (gwgetdata, DL_CONNECTED);
				break;
		}
		
		/* Second filter out other non-error messages that can precede "connected" */
		
		if (strncmp (gwgetdata->line, "--", 2) == 0 ||
			strncmp (gwgetdata->line, "  ", 2) == 0)
			break;

		if ((strncmp (gwgetdata->line, "Connecting to ", 14) == 0) &&
			(strstr (gwgetdata->line, "failed: ") == NULL))
			break;

		/* Wget 1.8.1 adds an explicit "Resolving" status msg */
		if ((strstr (gwgetdata->line, "Resolving") != NULL) &&
            (strstr (gwgetdata->line, "Host not found.") == NULL) &&
			(strstr (gwgetdata->line, "Name or service not known") == NULL))
                break;
			
		/* Wget, under certain circumstances, returns a list of resolved IP addresses
		 *	before attempting to connect, which can be ignored.
		 */
		
		/* Test for ipv4 address */
		dots = 0;
		for (p = gwgetdata->line; p[0]; p ++) {
			if (isdigit (p[0])) continue;
			else if (p[0] == '.') dots ++;
			else if ((p[0] == '\n') || (p[0] == ',')) break;
			else break;
		}
		if ((! p[0]) || (p[0] == '\n') || (p[0] == ',')) {
			if (dots == 3) break;
		}

		/* Test for ipv6 address */
		dots = 0;
		for (p = gwgetdata->line; p[0]; p ++) {
			if (isxdigit (p[0])) continue;
			else if (p[0] == ':') dots ++;
			else if ((p[0] == '\n') || (p[0] == ',')) break;
			else break;
		}
		if ((! p[0]) || (p[0] == '\n') || (p[0] == ',')) {
			if (dots > 1) break;
		}

		/* Failing the above checks, we _assume_ we have an error. The wget process will
		 * be killed after we create an error message.
		 */

		if (strncmp (gwgetdata->line, "socket: ", 8) == 0)
			show_error (gwgetdata, _ ("Socket error"));
			
	    else if (strncmp (gwgetdata->line, "Connection to ", 14) == 0)
			show_error (gwgetdata, _ ("Connection refused"));
			
	    else if (strstr (gwgetdata->line, "No route to host") != NULL)
			show_error (gwgetdata, _ ("No route to host"));
			
	    else if (strncmp (gwgetdata->line, "connect: ", 9) == 0)
			show_error (gwgetdata, _ ("Connection refused when downloading URL:"));
			
	    else if (strstr (gwgetdata->line, "Host not found.") != NULL)
			show_error (gwgetdata, _ ("Host not found"));
			
		else if (strstr (gwgetdata->line, "Name or service not known") != NULL)
			show_error (gwgetdata, _ ("Name or service not known"));
			
	    else if (strstr (gwgetdata->line, "unsupported protocol") != NULL)
			show_error (gwgetdata, _ ("Unsupported protocol"));
			
		else if (strstr (gwgetdata->line, "Refusing to truncate existing") != NULL)
			show_error(gwgetdata, _("Refusing to truncate existing file"));
			
		else if (strstr (gwgetdata->line, "unable to resolve"))
				show_error (gwgetdata, _ ("Unable to resolve host address"));
			
		else if ((p = strstr (gwgetdata->line, "failed: ")) != NULL) {
			/* 
			 * This is somewhat generic looking, but is informative
			 */
			show_error (gwgetdata, p + 8);
		}
			
		else {
			/*
			 * All other possible output may as well be reported, since we treat it
			 * as an error. We tag the message as unknown to make it more meaningful.
			 */
			p = g_strconcat (_ ("Unknown error "), gwgetdata->line, NULL);
			show_error (gwgetdata, p);
			g_free (p);
		}
		
		kill (gwgetdata->wget_pid, SIGKILL);
		return 1;
		break;

		case DL_CONNECTED:
		/* File not found for FTP */
			if (strncmp (gwgetdata->line, "No such file ", 13) == 0 ||
			strncmp (gwgetdata->line, "No such dir", 11) == 0) {
				show_error (gwgetdata, _ ("File not found"));
			break;
	    }

		/* File not found for HTTP or Proxy */
		if (strstr (gwgetdata->line, "ERROR") != NULL) {
			gwget_data_set_state (gwgetdata, DL_ERROR);
			show_error (gwgetdata, _ ("File not found"));
			break;
	    }

		/* local file conflicts */
		if (strstr (gwgetdata->line, "Continued download fail") != NULL) {
			show_error (gwgetdata,
				    _ ("Can't continue download, existing local file "
			    	   "conflicts with remote file"));
			break;
	    }

		/* Incorrect login or FTP with limited concurrent downloads */
		if (strstr (gwgetdata->line, "Login incorrect") != NULL) {
			if (check_server_already_exists(gwgetdata->url)==TRUE) {
				/* Login is correct but there is a limit in concurrents downloads */
				gwget_data_set_state (gwgetdata, DL_WAITING);			
			} else {
				gwget_data_set_state (gwgetdata, DL_ERROR);
			}
			break;
		}

		/* Get the leght of file being downloaded */
		p = strstr (gwgetdata->line, "Length: ");
		if (p != NULL) {
			p += 8;
            /* Only set the total size if we don't have one yet. */
            if (gwgetdata->total_size == 0 && !gwgetdata->recursive) { 
            	gwget_data_set_total_size (gwgetdata,convert_wget_size (p));
			}
			gwget_data_set_state (gwgetdata, DL_RETRIEVING);
			set_icon_newdownload();
			set_start_size_and_time(gwgetdata);
			gwgetdata->session_elapsed = 0;
	    } else {
				/* We didn't get a length so, probably it's unspecified size
                   so get the start of download by trying to catch the
                   string "K ->" */
				p = strstr (gwgetdata->line, "K -> ");
				if (p != NULL) {
					/* Unspecified size, so set total_size to 0 */
					gwget_data_set_total_size (gwgetdata, 0);
					gwget_data_set_state (gwgetdata, DL_RETRIEVING);
					set_start_size_and_time(gwgetdata);
				}
				gwgetdata->session_elapsed = 0;
        }
	    break;

		case DL_RETRIEVING:
			if (strstr(gwgetdata->line,"Saving to: `")) {		/* wget >=1.11 */
			    update_filename(gwgetdata);
			    break;
	    	}

			if (strncmp (gwgetdata->line, "Cannot write to ", 15) == 0) {
			show_error (gwgetdata,
					    _ ("Can't write to target directory"));
			kill (gwgetdata->wget_pid, SIGKILL);
			return 1;
			break;
			}
			
			if (gwgetdata->recursive) {
				/* Get the leght of file being downloaded */
				p = strstr (gwgetdata->line, "Length: ");
				if (p != NULL) {
					p += 8;
					gwget_data_set_total_size (gwgetdata,convert_wget_size (p));
					set_start_size_and_time(gwgetdata);
					gwgetdata->session_elapsed = 0;
				} else {
		                	/* We didn't get a length so, probably it's unspecified size
                			   so get the start of download by trying to catch the
			                   string "K ->" */
					p = strstr (gwgetdata->line, "K -> ");
					if (p != NULL) {
						/* Unspecified size, so set total_size to 0 */
						gwget_data_set_total_size (gwgetdata, 0);
						set_start_size_and_time(gwgetdata);
					}
					gwgetdata->session_elapsed = 0;
				}
				if (strstr (gwgetdata->line,"-- ") != NULL) {
					gchar *tmp=NULL;
					gint i,j;
					j=0;
					tmp = g_new(gchar,strlen(gwgetdata->line));
					for (i=14;i<strlen(gwgetdata->line);i++) {
						tmp[j]=gwgetdata->line[i];
						j++;
					}
					tmp[j]='\0';
					gwget_data_set_filename_from_url(gwgetdata,tmp);
					gwgetdata->local_filename = g_strconcat (gwgetdata->dir, gwgetdata->filename, NULL);
					g_free(tmp);
				}

				if (strstr (gwgetdata->line, "           =>") != NULL) {
					gchar *tmp=NULL;
					gint i,j;
					j=0;
					tmp = g_new(gchar,strlen(gwgetdata->line));
					for (i=15;i<strlen(gwgetdata->line);i++) {
						tmp[j]=gwgetdata->line[i];
						j++;
					}
					/* Remove the las ' character */
					tmp[j-1]='\0';
					gwgetdata->local_filename = g_strdup (tmp);
					gwgetdata->cur_size=0; 
					gwgetdata->total_size=0; 
					gwgetdata->session_start_time = 0;
					gwgetdata->session_start_size = 0;
					g_free(tmp);
				}
			}
		break;
		default:
		break;
	}
	return 0;
}

static gboolean
wget_log_read_log_line(GwgetData *gwgetdata) {
	char c;
	int res;

	res = read (gwgetdata->log_fd, &c, 1);
	if (res < 1) {
		/*
		 * No input available
		 */
		return FALSE;
	}

	if (! gwgetdata->line) {
		gwgetdata->line = g_malloc (sizeof (gchar) * BLOCK_SIZE);
		gwgetdata->line_pos = 0;
		gwgetdata->line_blocks = 1;
	}

	gwgetdata->line [gwgetdata->line_pos ++] = c;
	
	while (c != '\n') {
		res = read (gwgetdata->log_fd, &c, 1);
		if (res < 1) {
			/*
			 * There is currently no more data to read, so return FALSE - but 
			 * the line can still be completed later where it left off.
			 */ 
			return FALSE;
		}

		gwgetdata->line [gwgetdata->line_pos ++] = c;
		if ((gwgetdata->line_pos == gwgetdata->line_blocks * BLOCK_SIZE) && (c != '\n')) {
			/*
			 * The buffer is full, expanding
			 */
			gwgetdata->line_blocks ++;
			gwgetdata->line = g_realloc (gwgetdata->line,
					   sizeof (gchar) * gwgetdata->line_blocks * BLOCK_SIZE);
		}
	}
	
	gwgetdata->line [gwgetdata->line_pos -1] = 0;

	/* 
	 * We can just reuse gwgetdata->line by setting the line_pos to zero.
	 */
	gwgetdata->line_pos = 0;
	
	/*
	 * Result TRUE means gwgetdata->line contains a complete line.
	 */ 
	return TRUE;
}

void
wget_drain_remaining_log(GwgetData *gwgetdata) 
{
	wget_log_process (gwgetdata);
}

void
wget_log_process (GwgetData *gwgetdata)
{
	while (wget_log_read_log_line (gwgetdata))
		wget_log_process_line (gwgetdata);
	
	 if (gwgetdata->state == DL_RETRIEVING)
		gwget_data_update_statistics (gwgetdata);
}
