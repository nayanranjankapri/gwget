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
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include "gwget_data.h"
#include "wget-log.h"
#include "main_window.h"

#include <config.h>

static gint gwget_data_process_information (GwgetData *gwgetdata);
static void gwget_data_update_statistics_ui(GwgetData *gwgetdata);
static void convert_time2str(gchar *buffer, guint32 time);

/* Function to convert time to string */
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
	if (gwgetdata->state != DL_NOT_STARTED 
		&& gwgetdata->state != DL_NOT_RUNNING 
		&& gwgetdata->state != DL_COMPLETED) {
			elapsed_time = cur_time - gwgetdata->session_start_time;
			gwgetdata->total_time += elapsed_time - gwgetdata->session_elapsed;
			gwgetdata->session_elapsed = elapsed_time;
	}
	else
	{
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
    if (gwgetdata->state != DL_NOT_STARTED 
        && gwgetdata->state != DL_NOT_RUNNING 
        && gwgetdata->state != DL_COMPLETED) {
        if (gwgetdata->total_size != 0 && retr_size != 0)
		{ 
			estimated =
                (((gfloat) (gwgetdata->total_size
                            - gwgetdata->session_start_size)
                  * gwgetdata->session_elapsed) / retr_size)
                + ((gfloat) (gwgetdata->total_time
                             - gwgetdata->session_elapsed)); 
		}
        else
            estimated = 0;
    } else {
        if (gwgetdata->total_size != 0 && gwgetdata->cur_size != 0
            && gwgetdata->state != DL_COMPLETED)
            estimated =
                ((gfloat) gwgetdata->total_size * gwgetdata->total_time)
                / gwgetdata->cur_size;
        else
            estimated = 0;
    }
    if (estimated == 0)
        strcpy (buffer, "");
    else
        convert_time2str (buffer, estimated);
	
	gtk_list_store_set(GTK_LIST_STORE(model),&gwgetdata->file_list,
						ESTIMATEDTIME_COLUMN,buffer,
						-1);
	
	/* Remain Time */
	if (estimated == 0)
        strcpy (buffer, "");
    else
        convert_time2str (buffer, estimated - gwgetdata->total_time);
    gtk_list_store_set(GTK_LIST_STORE(model),&gwgetdata->file_list,
						REMAINTIME_COLUMN,buffer,
						-1);
	/* Percent column */
	sprintf(buffer,"%.1f%%", gwgetdata->total_size==0?0:((gfloat)gwgetdata->cur_size*100)/(gfloat)gwgetdata->total_size);
	perc = gwgetdata->total_size==0?0:((gdouble)gwgetdata->cur_size*100)/(gdouble)gwgetdata->total_size;
	gtk_list_store_set(GTK_LIST_STORE(model),&gwgetdata->file_list,
			PERCENTAGE_COLUMN,(gint)perc,-1);
	/* Speed column */
	if (gwgetdata->state != DL_NOT_STARTED 
        && gwgetdata->state != DL_NOT_RUNNING 
        && gwgetdata->state != DL_COMPLETED) {
        if (gwgetdata->session_elapsed != 0) {
            if (retr_size == 0)
                strcpy (buffer, _ ("stalled"));
            else
                sprintf (buffer, "%.2f kB/s",
                         ((gfloat) retr_size / gwgetdata->session_elapsed) 
                          / 1024);
        }
    } else {
        strcpy (buffer, "");
	}
	
	gtk_list_store_set(GTK_LIST_STORE(model),&gwgetdata->file_list,
			SPEED_COLUMN,buffer,-1);
	
	/* If is recursive, then update the filename */
	if (gwgetdata->recursive) {
		gtk_list_store_set(GTK_LIST_STORE(model),&gwgetdata->file_list,
			FILENAME_COLUMN,gwgetdata->filename,-1);
	}
	gwget_data_update_statistics_ui(gwgetdata);
	
	
}

static void
gwget_data_update_statistics_ui(GwgetData *gwgetdata)
{
	gchar *state;
	
	state=g_new(char,20);
	
	switch(gwgetdata->state) {
		case DL_NOT_STARTED: strcpy(state,_("Not Started"));
								break;
		case DL_NOT_RUNNING: strcpy(state,_("Not Running"));
								break;
		case DL_NOT_CONNECTED: strcpy(state,_("Not Connected"));
								break;
		case DL_CONNECTED: strcpy(state,_("Connected"));
								break;
		case DL_RETRIEVING: strcpy(state,_("Retrieving"));
								break;
		case DL_COMPLETED: strcpy(state,_("Completed"));
						   gtk_list_store_set(GTK_LIST_STORE(model),&gwgetdata->file_list,
			PERCENTAGE_COLUMN,100,-1);
								break;
		case DL_ERROR: state = g_strdup_printf("%s: %s",_("Error"),gwgetdata->error_msg);
								break;
	}
	
	
	
	gtk_list_store_set(GTK_LIST_STORE(model),&gwgetdata->file_list,
						STATE_COLUMN,state,
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
	/* Wget process stopped so we must register its exit */
	close (gwgetdata->log_fd);
	g_free (gwgetdata->line);
	gwgetdata->line = NULL;

        /* Set the appropriate state after stopping */
	if (gwgetdata->error) 
		gwget_data_set_state (gwgetdata, DL_ERROR);
	else if (WIFEXITED (status)) {
            if (WEXITSTATUS (status) == 0)
                gwget_data_set_state (gwgetdata, DL_COMPLETED);
            else if (WEXITSTATUS (status) == 255) {
                /* Only reaches here if wget is not found */
                gwget_data_set_state (gwgetdata, DL_NOT_RUNNING);
                g_warning ("couldn't find program wget to exec\n");
            } else
                gwget_data_set_state (gwgetdata, DL_NOT_RUNNING);
	} 
	else  {
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
        /*  gchar *opt; */

            /* Set stderr of child process to one end of the pipe. The father
             * process reads child output throught the pipe */
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
			/* argv[7] = "1"; */
            /* argv[8] = "-T";                    Wget timeout */
            /* argv[9] = g_strdup_printf ("%d", gtm_pref.read_timeout); */
	    
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
			}
			if (gwgetdata->mirror) {
				argv[arg]="-m";
				arg++;
			}
			
			if (!gwgetdata->multimedia && !gwgetdata->mirror){
				if (gwget_pref.no_create_directories) {
					argv[arg]="-nd";
					arg++;
				}
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
	    
	    argv[arg] = NULL;
			
			/* Set Language to C. This must be done or we will not be able
             * to parse the wget output */
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
		gwgetdata->line = g_new (gchar, MAX_WGET_LINE_SIZE + 1);
		gwgetdata->line_pos = 0;
		gwgetdata->log_tag = gtk_timeout_add (1000, 
                             (GtkFunction) gwget_data_process_information,
                             gwgetdata);
		
	}
	
}

void
gwget_data_set_total_size (GwgetData *gwgetdata,guint32 total_size)
{
	gwgetdata->total_size = total_size;
}


GwgetData * 
gwget_data_create(gchar *url, gchar *dir)
{
	GwgetData *gwgetdata;
	gint length;
	gchar *filename,*filename2;
	
	
	gwgetdata = g_new0(GwgetData,1);
	
	gwgetdata->url = g_strdup(url);
	gwgetdata->log_tag = -1;
	
	/* Figure out a directory to use if none given */
    length = strlen (dir);
    if (length == 0) {
       /* if (!download_dirs_suggest_dir(file_data->url, &dir)) */
            dir = gwget_pref.download_dir;
    }
	
	/* Add a trailing '/' unless already present */
    length = strlen (dir);
    if (dir[length - 1] == '/')
        gwgetdata->dir = g_strdup (dir);
    else
        gwgetdata->dir = g_strconcat (dir, "/", NULL);
	
	/* Get the filename from the URL */
    filename = &gwgetdata->url[strlen (gwgetdata->url)];
    while (*filename != '/' && filename != gwgetdata->url)
        filename--;
    filename++;

    /* Figure out if the url it's from the form: http://www.domain.com */
    /* If it's in the form: http://www.domain.com/ or http://www.domain.com/directory/ */
    /* it's detected in the function on_boton_ok_clicked in nuevo_window.c file */
    filename2 = g_strdup_printf("http://%s",filename);
    if (!strcmp(filename2,gwgetdata->url)) {
	gwgetdata->filename=g_strdup(filename2);
    }
    else {
	 gwgetdata->filename = g_strdup (filename);
    }
    gwgetdata->local_filename = g_strconcat (gwgetdata->dir,
                                             gwgetdata->filename,
                                             NULL);
	
	gwgetdata->line = NULL;
    gwgetdata->session_start_time = 0;
    gwgetdata->session_start_size = 0;
    gwgetdata->session_elapsed = 0;
    gwgetdata->cur_size = 0;
    gwgetdata->state = DL_NOT_STARTED;
    gwgetdata->total_size = 0;
    gwgetdata->total_time = 0;
    gwgetdata->recursive=FALSE;
	gwgetdata->multimedia=FALSE;
	gwgetdata->mirror=FALSE;
    /* gwgetdata->file_list = NULL; */
	num_of_download++;
	return gwgetdata;
	
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
	/* 	for (i=0;i<g_list_length(downloads);i++) {
			gwgetdata=g_list_nth_data(downloads,i);
			if (&gwgetdata->file_list == &iter) {
				return gwgetdata;
			}
		}
		*/
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
            if (WIFEXITED (status) && (WEXITSTATUS (status) == 0))
                gwget_data_set_state (data, DL_COMPLETED);
            else
                gwget_data_set_state (data, DL_NOT_RUNNING);
            gwget_data_update_statistics (data);

            /* Decrease the number of current downloads */
            if (num_of_download > 0)
                num_of_download--;

            /* All done this download can be started again */
            data->log_tag = -1;
        }
    }
			
	
	
}
