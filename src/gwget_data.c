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
 
#include <gnome.h>
#include <libgnomevfs/gnome-vfs.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include "gwget_data.h"
#include "wget-log.h"
#include "main_window.h"
#include "main_window_cb.h"
#include "utils.h"

#include <config.h>

static gint gwget_data_process_information (GwgetData *gwgetdata);
static void gwget_data_update_statistics_ui(GwgetData *gwgetdata);
static void convert_time2str(gchar *buffer, guint32 time);

static void
convert_time2str (gchar *buffer, guint32 time)
{
    sprintf (buffer, "%02d:%02d:%02d", (gint)(time / 3600), 
	     (gint)((time % 3600) / 60), (gint)(time % 60));
}



void
gwget_data_set_state (GwgetData *gwgetdata, DlState state) 
{
	gwgetdata->state=state;
	gwget_data_update_statistics_ui(gwgetdata);
}

void 
gwget_data_update_statistics (GwgetData *gwgetdata) 
{   
	guint32 cur_size;
	time_t cur_time, elapsed_time;
	struct stat file_stat;
	gchar buffer[20];
	guint32 retr_size, remain_size;
	time_t estimated;
	gdouble perc;
	gchar *title;
		
	
	/* Get time and size of the file being retrieved */
	if (stat (gwgetdata->local_filename, &file_stat) != -1) {
		cur_size = file_stat.st_size;
		cur_time = file_stat.st_ctime;
	} else {
		cur_size = 0;
		cur_time = 0;
	}
	
	gwgetdata->cur_size = cur_size;
	gtk_list_store_set(GTK_LIST_STORE(model),&gwgetdata->file_list,
				STATE_INT_COLUMN,gwgetdata->state,
				-1);
	
	/* Set the total and session time */
	if ( (gwgetdata->state != DL_NOT_STARTED) && 
	     (gwgetdata->state != DL_NOT_RUNNING) && 
	     (gwgetdata->state != DL_COMPLETED)) {
		elapsed_time = cur_time - gwgetdata->session_start_time;
		gwgetdata->total_time += elapsed_time - gwgetdata->session_elapsed;
		gwgetdata->session_elapsed = elapsed_time;
	} else {
		gwgetdata->session_elapsed = 0;
	}
	
	convert_time2str (buffer, gwgetdata->session_elapsed);
	
	gtk_list_store_set(GTK_LIST_STORE(model),&gwgetdata->file_list,
						ELAPSETIME_COLUMN,buffer,
						-1);
	convert_time2str (buffer, gwgetdata->total_time);
	
	gtk_list_store_set(GTK_LIST_STORE(model),&gwgetdata->file_list,
						TOTALTIME_COLUMN, buffer,
						-1);
	
	retr_size = gwgetdata->cur_size - gwgetdata->session_start_size;
	remain_size = gwgetdata->total_size - gwgetdata->cur_size;
	
	/* Total Size */
	if (gwgetdata->state == DL_NOT_STARTED)
		strcpy (buffer, "");
	else
		sprintf (buffer, "%d kB", (gwgetdata->total_size + 512) / 1024);
	
	gtk_list_store_set(GTK_LIST_STORE(model),&gwgetdata->file_list,
						TOTALSIZE_COLUMN,buffer,
						-1);
	
	
	/* Update retrieved information */
	if (gwgetdata->state == DL_NOT_STARTED || gwgetdata->state == DL_COMPLETED)
		strcpy (buffer, "");
	else
		sprintf (buffer, "%d kB", (gwgetdata->cur_size + 512) / 1024);
	gtk_list_store_set(GTK_LIST_STORE(model),&gwgetdata->file_list,
						CURRENTSIZE_COLUMN,buffer,
						-1);
	
	
	/* Update estimated and remain times */
	if ((gwgetdata->state != DL_NOT_STARTED) &&
            (gwgetdata->state != DL_NOT_RUNNING) && 
            (gwgetdata->state != DL_COMPLETED)) {
        	if (gwgetdata->total_size != 0 && retr_size != 0) { 
			estimated = (((gfloat) (gwgetdata->total_size - gwgetdata->session_start_size)
                  			* gwgetdata->session_elapsed) / retr_size)
                			+ ((gfloat) (gwgetdata->total_time
                             		- gwgetdata->session_elapsed)); 
		} else {
            		estimated = 0;
		}
	} else {
        	if ((gwgetdata->total_size != 0) && (gwgetdata->cur_size != 0) &&
            		(gwgetdata->state != DL_COMPLETED)) {
			estimated = ((gfloat) gwgetdata->total_size * gwgetdata->total_time) / gwgetdata->cur_size;
		} else {
            		estimated = 0;
		}
	}
	if (estimated == 0)
		strcpy (buffer, "");
	else
		convert_time2str (buffer, estimated);
	
	gtk_list_store_set(GTK_LIST_STORE(model),&gwgetdata->file_list,
						ESTIMATEDTIME_COLUMN,buffer,
						-1);
	
	/* Remain Time */
	if (estimated == 0) {
		strcpy (buffer, "");
	} else {
        	convert_time2str (buffer, estimated - gwgetdata->total_time);
	}
	gtk_list_store_set(GTK_LIST_STORE(model),&gwgetdata->file_list,
						REMAINTIME_COLUMN,buffer,
						-1);
	/* Percent column */
	sprintf(buffer,"%.1f%%", gwgetdata->total_size==0?0:((gfloat)gwgetdata->cur_size*100)/(gfloat)gwgetdata->total_size);
	perc = gwgetdata->total_size==0?0:((gdouble)gwgetdata->cur_size*100)/(gdouble)gwgetdata->total_size;
	gtk_list_store_set(GTK_LIST_STORE(model),&gwgetdata->file_list,
			PERCENTAGE_COLUMN,(gint)perc,-1);
	title = g_strdup_printf("%s %d %%", gwgetdata->filename, (gint)perc);
	/* Speed column */
	if ((gwgetdata->state != DL_NOT_STARTED) &&
		(gwgetdata->state != DL_NOT_RUNNING) &&
		(gwgetdata->state != DL_COMPLETED)) {
        		if (gwgetdata->session_elapsed != 0) {
            			if (retr_size == 0) {
					strcpy (buffer, _ ("stalled"));
				} else {
                			sprintf (buffer, "%.2f kB/s",
                         			((gfloat) retr_size / gwgetdata->session_elapsed) / 1024);
				}
			}
	} else {
		strcpy (buffer, "");
	}
	
	gtk_list_store_set(GTK_LIST_STORE(model),&gwgetdata->file_list,
			   SPEED_COLUMN,buffer,-1);
	
	/* Update the filename */
	gtk_list_store_set(GTK_LIST_STORE(model),
			   &gwgetdata->file_list,
			   FILENAME_COLUMN,
			   gwgetdata->filename,
			   -1);
		
	gwget_data_update_statistics_ui(gwgetdata);
	if (gwgetdata == gwget_data_get_selected()) {
		gtk_window_set_title(GTK_WINDOW(glade_xml_get_widget(xml, "main_window")), title);
	}


	check_download_in_progress();
}

static void
gwget_data_update_statistics_ui(GwgetData *gwgetdata)
{
	gchar *tooltip_message;
	
	switch(gwgetdata->state) {
		case DL_WAITING: gwgetdata->state_str = g_strdup(_("Waiting"));
				 break;
		case DL_NOT_STARTED: gwgetdata->state_str = g_strdup(_("Not Started"));
								break;
		case DL_NOT_RUNNING: gwgetdata->state_str = g_strdup(_("Not Running"));
								break;
		case DL_NOT_CONNECTED: gwgetdata->state_str = g_strdup(_("Not Connected"));
								break;
		case DL_CONNECTED: gwgetdata->state_str = g_strdup(_("Connected"));
								break;
		case DL_RETRIEVING: gwgetdata->state_str = g_strdup(_("Retrieving"));
								break;
		case DL_COMPLETED: gwgetdata->state_str=g_strdup(_("Completed"));
				   gtk_list_store_set(GTK_LIST_STORE(model),&gwgetdata->file_list,
							      PERCENTAGE_COLUMN,100,-1);
				   if (gwget_pref.docked) {
					   tooltip_message=g_strdup_printf(_("Download Completed\n(%s)"), 
							   gwgetdata->url);
					   gtk_tooltips_set_tip (GTK_TOOLTIPS(tray_tooltip), GTK_WIDGET(tray_icon),tooltip_message,NULL);
					   egg_tray_icon_send_message(tray_icon,100000,tooltip_message,strlen(tooltip_message));
				   }
					   
				   break;
		case DL_ERROR: 	gwgetdata->state_str = g_strdup_printf("%s: %s",_("Error"),gwgetdata->error_msg);
				break;
	}
	
	
	
	gtk_list_store_set(GTK_LIST_STORE(model),&gwgetdata->file_list,
						STATE_COLUMN,gwgetdata->state_str,
						-1);
}


static gint
gwget_data_process_information (GwgetData *gwgetdata)
{
	pid_t child_pid;
	gint status;

	/* Process wget_log messages */
	wget_log_process (gwgetdata);

	/* Check that wget process is still running */
	child_pid = waitpid (gwgetdata->wget_pid, &status, WNOHANG | WUNTRACED);
 	if (child_pid == gwgetdata->wget_pid) {
		/*
		 * Use any information wget logged and we did not read
		 */
		wget_drain_remaining_log(gwgetdata);
		
		/* Wget process stopped so we must register its exit */
		close (gwgetdata->log_fd);
		g_free (gwgetdata->line);
		gwgetdata->line = NULL;

        	/* Set the appropriate state after stopping */
		if (gwgetdata->error) 
			gwget_data_set_state (gwgetdata, DL_ERROR);
		else if (WIFEXITED (status)) {
            		if (WEXITSTATUS (status) == 0) {
                		gwget_data_set_state (gwgetdata, DL_COMPLETED);
				start_first_waiting_download();
				if (gwget_pref.open_after_dl) {
					gwget_data_exec(gwgetdata);
				} 
	    		} else if (WEXITSTATUS (status) == 255) {
                		/* Only reaches here if wget is not found */
                		gwget_data_set_state (gwgetdata, DL_NOT_RUNNING);
                		g_warning ("couldn't find program wget to exec\n");
            		} else {
				if (gwgetdata->state!=DL_WAITING) {
                			gwget_data_set_state (gwgetdata, DL_NOT_RUNNING);
				}
			}
		} else {
			gwget_data_set_state (gwgetdata, DL_NOT_RUNNING);
		}
		gwget_data_update_statistics (gwgetdata);

        	/* Decrease the number of current downloads */
        	if (num_of_download > 0)
            		num_of_download--;

	        /* All done this download can be started again */
        	gwgetdata->log_tag = -1;

		return FALSE;
    }
    return TRUE;
}

void 
gwget_data_free(gpointer data) 
{
	g_free(data);
}

void
gwget_data_start_download(GwgetData *gwgetdata)
{
	pid_t pid;
	gint pipe_fd[2];
	gint number;

	/* Control the number of download allowed */
	number=count_download_in_progress();

	if (gwget_pref.limit_simultaneousdownloads==TRUE && number>=gwget_pref.max_simultaneousdownloads) {
		gwget_data_set_state(gwgetdata,DL_WAITING);
		return;
	}

	/* Put the not connected state before */
	gwget_data_set_state(gwgetdata,DL_NOT_CONNECTED);
	gwget_data_update_statistics_ui(gwgetdata);
	
	/* First check if we are not starting an already started download */
	if (!gwget_data_run (gwgetdata) && gwgetdata->state != DL_COMPLETED) {
        	if (pipe (pipe_fd) < 0) {
			g_warning ("couldn't create wget pipe");
            		return;
        	}
		pid = fork ();
		if (pid == 0) {
            		/* Child process */
            		gint arg;
            		gchar *argv[20];

			/* Set stderr of child process to one end of the pipe. The father */
			/* process reads child output throught the pipe */
			close (pipe_fd[0]);
			dup2 (pipe_fd[1], 2);
			
			/* Set common arguments */
			argv[0] = "wget";
			argv[1] = "-v";                   /* Verbose */
			argv[2] = "-P";                   /* Use directory prefix */
			argv[3] = gwgetdata->dir;         /* Directory prefix */
			argv[4] = gwgetdata->url;         /* Url to download */
			argv[5] = "-c";                   /* Continue download */
			argv[6] = "-t";                   /* Number of retries */
			argv[7] = g_strdup_printf ("%d", gwget_pref.num_retries); 
	    
			arg = 8;
			/* Preferences */
			if (gwgetdata->recursive) {
				/* recursive options */
				argv[arg]="-r";
				arg++;
				if (gwgetdata->multimedia) {
					argv[arg]="-l1";
					arg++;
					argv[arg]="-Ajpg";
					arg++;
					argv[arg]="-Agif";
					arg++;
					argv[arg]="-Ampg";
					arg++;
					argv[arg]="-Ampeg";
					arg++;
					argv[arg]="-Aavi";
					arg++;
					argv[arg]="-Apng";
					arg++;
					argv[arg]="-Awmv";
					arg++;
				}
				if (gwgetdata->mirror) {
					argv[arg]="-m";
					arg++;
				}
				
				if (gwget_pref.no_create_directories) {
						argv[arg]="-nd";
						arg++;
				}
				
				if (!gwgetdata->multimedia && !gwgetdata->mirror){
					
					if (gwget_pref.follow_relative) {
						argv[arg]="-L";
						arg++;
					}
					if (gwget_pref.convert_links) {
						argv[arg]="-k";
						arg++;
					}
					if (gwget_pref.dl_page_requisites) {
						argv[arg]="-p";
						arg++;
					}
				
					argv[arg]="-l";
					arg++;
					argv[arg]=g_strdup_printf("%d",gwget_pref.max_depth);
					arg++;
				}
			}
			
			if (gwget_pref.limit_speed) {
				argv[arg] = g_strdup_printf("--limit-rate=%dk",
						            gwget_pref.max_speed);
				arg++;
			}
			
			if ( strcmp (gwget_pref.network_mode, "manual") == 0 && 
			     gwget_pref.http_proxy ) 
			{
				setenv("http_proxy",g_strconcat(g_strdup_printf("%s",gwget_pref.http_proxy),":",g_strdup_printf("%d",gwget_pref.http_proxy_port),NULL),1);
		 	        argv[arg]="-Yon";
				arg++;
			}
			
			if ( strcmp (gwget_pref.network_mode, "default" ) == 0 && gwget_pref.gnome_http_proxy && gwget_pref.gnome_use_proxy) 
			{
				setenv("http_proxy",g_strconcat(g_strdup_printf("%s",gwget_pref.gnome_http_proxy),":",g_strdup_printf("%d",gwget_pref.gnome_http_proxy_port),NULL),1);
	            		argv[arg]="-Yon";
				arg++;
			}
			
			
			argv[arg] = NULL;
			
			/* Set Language to C. This must be done or we will not be able */
			/* to parse the wget output */
            		putenv ("LC_ALL=C");

			/* Everything ready run wget */
			execvp ("wget", argv);

			/* If we get here wget was not found so we return an error 255 */
			_exit (255);
		}
		if (pid < 0) {
			g_warning ("couldn't fork wget");
			return;
		}
		
		gtk_list_store_set(GTK_LIST_STORE(model),&gwgetdata->file_list,
				   PID_COLUMN,pid, -1); 
		
		close (pipe_fd[1]);
		gwgetdata->wget_pid = pid;
		
		gwgetdata->log_fd = pipe_fd[0];
		fcntl (gwgetdata->log_fd, F_SETFL, O_NONBLOCK);
		gwgetdata->log_tag = gtk_timeout_add (1000, 
                	             (GtkFunction) gwget_data_process_information,
                        	     gwgetdata);
	}
	
	/*
	 * Update download list in gconf
	 */
	gwget_remember_downloads();
}

void
gwget_data_set_total_size (GwgetData *gwgetdata,guint32 total_size)
{
	g_return_if_fail (gwgetdata != NULL);
	gwgetdata->total_size = total_size;
}


GwgetData * 
gwget_data_create(gchar *url, gchar *dir)
{
	GwgetData *gwgetdata;
	GnomeVFSURI *localfile_uri;
	gint length;

	g_return_val_if_fail(url != NULL, NULL);
	g_return_val_if_fail(dir != NULL, NULL);
	g_return_val_if_fail(gwget_pref.download_dir != NULL, NULL);
		
	gwgetdata = g_new0(GwgetData,1);
	
	gwgetdata->url = g_strdup(url);
	gwgetdata->log_tag = -1;

	/* Figure out a directory to use if none given */
	length = strlen (dir);
	if (length == 0) {
		dir = gwget_pref.download_dir;
	}
	
	/* Add a trailing '/' unless already present */
	length = strlen (dir);
	if (dir[length - 1] == '/')
		gwgetdata->dir = g_strdup (dir);
	else
		gwgetdata->dir = g_strconcat (dir, "/", NULL);
	
	gwget_data_set_filename_from_url(gwgetdata,gwgetdata->url);
	
	gwget_data_set_filename(gwgetdata,gwgetdata->filename);
	
	localfile_uri = gnome_vfs_uri_new (gwgetdata->local_filename);
	if (gnome_vfs_uri_exists (localfile_uri) ) {
		GnomeVFSFileInfo *info;
		info = gnome_vfs_file_info_new();
		gnome_vfs_get_file_info (gwgetdata->local_filename, 
				         info, 
					 GNOME_VFS_FILE_INFO_DEFAULT);
		gwgetdata->cur_size = (guint32)info->size;
	} else {
		gwgetdata->cur_size = 0;
	}
	
	gwgetdata->line = NULL;
	gwgetdata->session_start_time = 0;
	gwgetdata->session_start_size = 0;
	gwgetdata->session_elapsed = 0;
	/* gwgetdata->cur_size = 0; */
	gwgetdata->state = DL_NOT_STARTED;
	gwgetdata->total_size = 0;
	gwgetdata->total_time = 0;
	gwgetdata->recursive=FALSE;
	gwgetdata->multimedia=FALSE;
	gwgetdata->mirror=FALSE;
	num_of_download++;
	return gwgetdata;
	
}

/* Keeps filename and local_filename in sync */
void
gwget_data_set_filename(GwgetData* gwgetdata,gchar *str) 
{
	gwgetdata->filename = g_strdup(str);
	gwgetdata->local_filename = g_strconcat (gwgetdata->dir, 
			                         str,
						 NULL);
}

/* Return the gwgetdata that is selected in the treeview */
GwgetData* gwget_data_get_selected(void)
{
	GtkWidget *treev;
	GtkTreeSelection *select;
	GtkTreeIter iter;
	GtkTreeModel *model;
	GwgetData *gwgetdata;
	gchar *url;
	
	treev=glade_xml_get_widget(xml,"treeview1");
	select=gtk_tree_view_get_selection(GTK_TREE_VIEW(treev));
	
	if (gtk_tree_selection_get_selected (select, &model, &iter)) 
	{
		gtk_tree_model_get (model, &iter, URL_COLUMN, &url, -1);
		gwgetdata=g_object_get_data(G_OBJECT(model),url);
		return gwgetdata;
	}
	return NULL;
}

void gwget_data_stop_download(GwgetData *data)
{
	pid_t child_pid;
	gint status;
	
	
	if (gwget_data_run(data)) {
		/* Kill wget process */
		kill (data->wget_pid, SIGINT);
		
		/* Remove callback that communicates with wget */
		gtk_timeout_remove (data->log_tag);
		
		/* Wait the finish of wget */
		child_pid = waitpid (data->wget_pid, &status, WUNTRACED);
		if (child_pid == data->wget_pid) {
			/* Process the rest of the information that wget process has */
			gwget_data_process_information (data);
			
			/* Register wget exit */
			close (data->log_fd);
			g_free (data->line);
			data->line = NULL;
			
			/* Set the appropriate state after stopping */
			if (WIFEXITED (status) && (WEXITSTATUS (status) == 0)) {
				gwget_data_set_state (data, DL_COMPLETED);
				start_first_waiting_download();
				if (gwget_pref.open_after_dl) {
					gwget_data_exec(data);
				}
			} else {
				gwget_data_set_state (data, DL_NOT_RUNNING);
			}
			gwget_data_update_statistics (data);

			/* Decrease the number of current downloads */
			if (num_of_download > 0)
			num_of_download--;

			/* All done this download can be started again */
			data->log_tag = -1;
		}
	}
}

void 
gwget_data_set_filename_from_url(GwgetData *gwgetdata,gchar *url)
{
	gchar *filename,*filename2;
	
	/* Get the filename from the URL */
	filename = &url[strlen (url)];
	while (*filename != '/' && filename != url)
		filename--;
	filename++;

	/* 
	 * Figure out if the url it's from the form: http://www.domain.com         
	 * If it's in the form: http://www.domain.com/ or 
	 *                      http://www.domain.com/directory/ 
	 * it's detected in the function on_ok_button_clicked in new_window.c file 
	 */
	filename2 = g_strdup_printf("http://%s",filename);
	if (!strcmp(filename2,url)) {
		gwgetdata->filename = g_strdup(filename2);
	} else {
		gwgetdata->filename = g_strdup(filename);
	}
}

/* Add a gwgetdata to the main window */
void 
gwget_data_add_download(GwgetData *gwgetdata)
{
	gint response;
	gchar *reverse_filename;
	GtkWidget *radio, *recursive_window;
	
	if (check_url_already_exists(gwgetdata->url)) {
		run_dialog_information(_("Unable to add this download"),
				       _("This download is already added"));
		return;
	}

	/* if the url it's not a file drop a dialog to recurse into the url */
	reverse_filename = g_strdup(gwgetdata->filename);
	reverse_filename = g_strreverse(reverse_filename);
	if (!strcmp(gwgetdata->filename,"") || !strcmp(gwgetdata->filename,gwgetdata->url) ||
		!strncmp(reverse_filename,"lmth",4) || !strncmp(reverse_filename,"mth",3) || 
		!strncmp(reverse_filename,"php",3)  || !strncmp(reverse_filename,"asp",3)) {
		recursive_window=glade_xml_get_widget(xml,"dialog2");
		response=gtk_dialog_run(GTK_DIALOG(recursive_window));
		gtk_widget_hide(GTK_WIDGET(recursive_window));
		if (response==GTK_RESPONSE_OK) {
			radio=glade_xml_get_widget(xml,"radio_index");
			if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio))) {
				gwgetdata->recursive=FALSE;
			}
			radio=glade_xml_get_widget(xml,"radio_multimedia");
			if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio))) {
				gwgetdata->recursive=TRUE;
				gwgetdata->multimedia=TRUE;
			}
			radio=glade_xml_get_widget(xml,"radio_mirror");
			if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio))) {
				gwgetdata->recursive=TRUE;
				gwgetdata->mirror=TRUE;
			}
			radio=glade_xml_get_widget(xml,"radio_recursive");
			if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio))) {
				gwgetdata->recursive=TRUE;
			}
		}
	}
	downloads = g_list_append(downloads,gwgetdata);
	gwget_data_set_state(gwgetdata,DL_NOT_CONNECTED);
	new_download(gwgetdata);
}

void
gwget_data_exec (GwgetData *gwgetdata)
{
	gchar *uri;
	GError *err = NULL; 

	uri = gnome_vfs_make_uri_from_input_with_dirs (gwgetdata->local_filename,
							GNOME_VFS_MAKE_URI_DIR_CURRENT);
	if (!gnome_url_show (uri, &err)) {
		run_dialog(_("Error opening file"),_("Couldn't open the file"));
	}
}

void 
gwget_init_pref(Preferences *pref)
{
	pref->trayonly = FALSE;
	pref->docked   = FALSE;
}

