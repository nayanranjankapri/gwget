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

#include <config.h>
#include <gnome.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include "gwget_data.h"
#include "wget-log.h"


/* Function to convert the wget notation of file size (55,449,600) */ 
static guint32 
convert_wget_size (char *size)
{
   char *p = size;

   while (*p != ' ') {
      if (*p == ',') {
	 while (*p != ' ')
	    *p++ = *(p+1);
	 p = size;
      } else
	 p++;
   }
   return atoi (size);
}

static void
show_error (GwgetData *gwgetdata, gchar *error_msg)
{
    /* gchar *message; */

    /* Before showing the error message disable auto-download. If we don't do
     * this auto download retries to download the file again and with an error.
     */
    /* gwget_data_set_use_auto_dl (gwgetdata, FALSE); */
    /* message = g_strconcat (error_msg, "\t", gwgetdata->url, NULL); */
    /* log_insert_entry (message); */
    /* g_print("%s\n",message);    */
    gwget_data_set_state(gwgetdata,DL_ERROR);
    gwgetdata->error=TRUE;
    gwgetdata->error_msg=g_strdup(error_msg);
    /* g_free (message); */
}


static int
wget_log_process_line (GwgetData *gwgetdata)
{
    gchar *p;
    struct stat file_stat;

	switch (gwgetdata->state) {
	case DL_NOT_CONNECTED:
	    /* First check to see if connected to the host correctly */
            /* Wget 1.8.1 says "connected." rather than "connected!" */
	    if (strstr (gwgetdata->line, "connected!") != NULL ||
                strstr (gwgetdata->line, "connected.") != NULL) {
		gwget_data_set_state (gwgetdata, DL_CONNECTED);
			break;
		}
	    /* Wget 1.8.1 adds an explicit "Resolving" status msg */
        if (strstr (gwgetdata->line, "Resolving") != NULL &&
                strstr (gwgetdata->line, "Host not found.") == NULL)
                break;

	    /* We are not connected to the host so we must find the problem */
	    if (strncmp (gwgetdata->line, "--", 2) == 0 ||
		strncmp (gwgetdata->line, "  ", 2) == 0 ||
		strncmp (gwgetdata->line, "Connecting to ", 14) == 0 ||
		strlen(gwgetdata->line) ==0)
		break;
	    else if (strncmp (gwgetdata->line, "socket: ", 8) == 0)
		show_error (gwgetdata,
                            _ ("Socket error"));
	    else if (strncmp (gwgetdata->line, "Connection to ", 14) == 0)
		show_error (gwgetdata,
                            _ ("Connection refused\n"));
	    else if (strstr (gwgetdata->line, "No route to host") != NULL)
		show_error (gwgetdata,
                            _ ("No route to host"));
	    else if (strncmp (gwgetdata->line, "connect: ", 9) == 0)
		show_error (gwgetdata,
                            _ ("Connection refused when downloading URL:\n"));
	    else if (strstr (gwgetdata->line, "Host not found.") != NULL)
		show_error (gwgetdata,
                            _ ("Host not found\n"));
	    else if (strstr (gwgetdata->line, "unsupported protocol") != NULL)
		show_error (gwgetdata,
			    _ ("Unsupported protocol - you need wget >= 1.7 "
			       "for https:\n"));
		else if (strstr (gwgetdata->line, "Refusing to truncate existing") != NULL)
			show_error(gwgetdata,
				_("Refusing to truncate existing file"));
	    else {
			show_error (gwgetdata,
                            _ ("Unknown error"));
			
		}
	    kill (gwgetdata->wget_pid, SIGKILL);
	    return 1;
	    break;

        case DL_CONNECTED:
	    /* File not found for FTP */
	    if (strncmp (gwgetdata->line, "No such file ", 13) == 0 ||
		strncmp (gwgetdata->line, "No such dir", 11) == 0) {
		show_error (gwgetdata,
                            _ ("File not found"));
		
		break;
	    }

	    /* File not found for HTTP or Proxy */
	    if (strstr (gwgetdata->line, "ERROR") != NULL) {
		show_error (gwgetdata, 
                            _ ("File not found"));
		break;
	    }

	    /* local file conflicts */
	    if (strstr (gwgetdata->line, "Continued download fail") != NULL) {
		show_error (gwgetdata,
			    _ ("Can't continue download, existing local file "
			       "conflicts with remote file"));
		break;
	    }

	    /* Get the leght of file being downloaded */
	    p = strstr (gwgetdata->line, "Length: ");
	    if (p != NULL) {
			p += 8;
            /* Only set the total size of we don't have one yet. */
            if (gwgetdata->total_size == 0) {
            	gwget_data_set_total_size (gwgetdata,
                                             convert_wget_size (p));
			}
			gwget_data_set_state (gwgetdata, DL_RETRIEVING);
			/* Get session start time and session file start size */
			if (stat (gwgetdata->local_filename, &file_stat) != -1) {
		    	gwgetdata->session_start_time = file_stat.st_ctime;
		    	gwgetdata->session_start_size = file_stat.st_size;
			} else {
		    	gwgetdata->session_start_time = 0;
		    	gwgetdata->session_start_size = 0;
			}
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
                    if (stat (gwgetdata->local_filename, &file_stat) != -1) {
                        gwgetdata->session_start_time = file_stat.st_ctime;
                        gwgetdata->session_start_size = file_stat.st_size;
                    } else {
                        gwgetdata->session_start_time = 0;
                        gwgetdata->session_start_size = 0;
                    }
                }
                gwgetdata->session_elapsed = 0;
        }
	    break;

        case DL_RETRIEVING:
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
				/* Get session start time and session file start size */
				if (stat (gwgetdata->local_filename, &file_stat) != -1) {
		    		gwgetdata->session_start_time = file_stat.st_ctime;
		    		gwgetdata->session_start_size = file_stat.st_size;
				} else {
		    		gwgetdata->session_start_time = 0;
		    		gwgetdata->session_start_size = 0;
				}
				gwgetdata->session_elapsed = 0;
			} else {
                /* We didn't get a length so, probably it's unspecified size
                   so get the start of download by trying to catch the
                   string "K ->" */
                p = strstr (gwgetdata->line, "K -> ");
                if (p != NULL) {
                    /* Unspecified size, so set total_size to 0 */
                    gwget_data_set_total_size (gwgetdata, 0);
                    if (stat (gwgetdata->local_filename, &file_stat) != -1) {
                        gwgetdata->session_start_time = file_stat.st_ctime;
                        gwgetdata->session_start_size = file_stat.st_size;
                    } else {
                        gwgetdata->session_start_time = 0;
                        gwgetdata->session_start_size = 0;
                    }
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
				printf("Linea: %s\n",tmp);
				gwgetdata->filename=g_strdup(tmp);
				
			}
	    }
		break;
        default:
	    break;
    }
    return 0;
}

void
wget_log_process (GwgetData *gwgetdata)
{
    gchar buffer[MAX_WGET_LINE_SIZE * 2 + 1];
    gint total_read, pos, res;

	
    do {
	total_read = read (gwgetdata->log_fd, buffer, MAX_WGET_LINE_SIZE * 2);
	pos = 0;
	while (pos < total_read) {
	    if (buffer[pos] != '\n') {
		if (gwgetdata->line_pos < MAX_WGET_LINE_SIZE)
		    gwgetdata->line[gwgetdata->line_pos++] = buffer[pos++];
		else
		    pos++;
	    } else {
		gwgetdata->line[gwgetdata->line_pos] = '\0';
			
		res = wget_log_process_line (gwgetdata);
		if (res == 1)
                    break;
		gwgetdata->line_pos = 0;
		pos++;
	    }
	}
	if (gwgetdata->state == DL_RETRIEVING)
	    gwget_data_update_statistics (gwgetdata);
    } while (total_read == MAX_WGET_LINE_SIZE * 2);
}
