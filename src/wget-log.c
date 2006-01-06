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

#define _FILE_OFFSET_BITS 64

#include <config.h>
#include <gnome.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
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


static int
wget_log_process_line (GwgetData *gwgetdata)
{
	if (gwgetdata->line == NULL)
		return 0;

	gchar *p;
	struct stat file_stat;

	switch (gwgetdata->state) {
	case DL_NOT_CONNECTED:
		/* 
		 * If gwgetdata->filename does not match the filesystem filename,
		 * bad things can happen. We intecept the line that prints the 
		 * filesystem filename and set gwgetdata->filename
		 */
		if (strstr(gwgetdata->line,"           => `")) {
			int iL = strlen(gwgetdata->line);
			gwgetdata->line[iL-1] = 0; // Chop the final ' 

			char *sName = gwgetdata->line;
			/*
			 * Now sName contains the whole pathname. No filename can
			 * contain '/' so the following search for the last component
			 * is sane
			 */
			sName += strlen(sName) - 1;
			while (*sName != '/' && sName != gwgetdata->line)
				sName--;
			if (*sName == '/')
				sName++;

			//g_print("NAME: %s\n",sName);
			gwget_data_set_filename(gwgetdata,sName);
			gwget_data_update_statistics(gwgetdata);
			gwget_remember_downloads();
			break;
		}
		  
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
			show_error (gwgetdata, _ ("Socket error"));
	    else if (strncmp (gwgetdata->line, "Connection to ", 14) == 0)
			show_error (gwgetdata, _ ("Connection refused\n"));
	    else if (strstr (gwgetdata->line, "No route to host") != NULL)
			show_error (gwgetdata, _ ("No route to host"));
	    else if (strncmp (gwgetdata->line, "connect: ", 9) == 0)
				show_error (gwgetdata, _ ("Connection refused when downloading URL:\n"));
	    else if (strstr (gwgetdata->line, "Host not found.") != NULL)
			show_error (gwgetdata, _ ("Host not found\n"));
	    else if (strstr (gwgetdata->line, "unsupported protocol") != NULL)
		show_error (gwgetdata, _ ("Unsupported protocol - you need wget >= 1.7 "
			       "for https:\n"));
		else if (strstr (gwgetdata->line, "Refusing to truncate existing") != NULL)
			show_error(gwgetdata, _("Refusing to truncate existing file"));
	    else {
			show_error (gwgetdata, _ ("Unknown error"));
			
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
					gwget_data_set_filename_from_url(gwgetdata,tmp);
					gwgetdata->local_filename = g_strconcat (gwgetdata->dir, gwgetdata->filename, NULL);
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
				}
			}
		break;
		default:
		break;
	}
	return 0;
}

static void
wget_log_read_log_line(GwgetData *gwgetdata) {
	g_free(gwgetdata->line);
	gwgetdata->line = NULL;

	char c;
	int iRes = read(gwgetdata->log_fd,&c,1);

	if (iRes < 1) {
		/*
		 * No input available
		 */
		gwgetdata->line = NULL;
		return;
	}

	int iBlockCount = 1;
	gchar *buffer = g_malloc(sizeof(gchar)*(iBlockCount*BLOCK_SIZE));
	int iWritePos = 0;
		
	buffer[iWritePos++] = c;
	while (c != '\n') {
		iRes = read(gwgetdata->log_fd,&c,1);
		if (iRes < 1) {
			/*
			 * There is currently no more data to read. Return what we have.
			 */
			buffer[iWritePos++] = 0;
			break;
		}
		buffer[iWritePos++] = c;

		if (iWritePos == iBlockCount*BLOCK_SIZE && c != '\n') {
			/*
			 * The buffer is full , expanding
			 */
			iBlockCount++;
			buffer = g_realloc(buffer,
					   sizeof(gchar)*(iBlockCount*BLOCK_SIZE));
		}
	} 
	buffer[iWritePos-1] = 0;
	//g_print("LOG: %s\n",buffer);
	
	gwgetdata->line = buffer;
}

void
wget_drain_remaining_log(GwgetData *gwgetdata) 
{
	wget_log_read_log_line(gwgetdata);
	while (gwgetdata->line != NULL) {
		wget_log_process_line(gwgetdata);
		wget_log_read_log_line(gwgetdata);
	}
	gwget_data_update_statistics(gwgetdata);
}

void
wget_log_process (GwgetData *gwgetdata)
{
	/*
	 * Read and process two lines of log
	 */
	wget_log_read_log_line(gwgetdata);
	wget_log_process_line(gwgetdata);
	
	wget_log_read_log_line(gwgetdata);
	wget_log_process_line(gwgetdata);

	if (gwgetdata->state == DL_RETRIEVING)
		gwget_data_update_statistics(gwgetdata);
}

